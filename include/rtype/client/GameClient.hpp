#pragma once

#include "rtype/common/Protocol.hpp"
#include "rtype/common/GameConfig.hpp"
#include "rtype/common/INetwork.hpp"
#include "rtype/client/IRender.hpp"
#include "rtype/client/MenuState.hpp"
#include "RemoteDisplay.hpp"

#include <atomic>
#include <array>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <memory>
#include <chrono>

namespace rtype::client
{
class GameClient
{
public:
    GameClient(const std::string &host, std::uint16_t port);
    ~GameClient();

    void run();

private:
    void networkReceive();
    void handlePacket(const std::uint8_t* data, std::size_t size);
    void sendInput(const net::PlayerInput &input);
    bool checkServerTimeout();
    
    void handleCreateRoom(const net::RoomCreated &created);
    void handleRoomJoined(const net::RoomJoined &joined);
    void handleRoomLeft(const net::RoomLeft &left);
    void handleGameStarted(const net::GameStarted &started);
    void handleRoomError(const net::RoomError &error);
    void handlePlayerDeath(const net::PlayerDeath &death);
    void handleAllPlayersDead(const net::AllPlayersDead &msg);
    
    void sendCreateRoom(const std::string &roomName);
    void sendJoinRoom(std::uint32_t roomId);
    void sendLeaveRoom();
    void sendStartGame();
    void sendSpectatorMode(bool enabled);

    std::unique_ptr<network::IIOContext> _ioContext;
    std::unique_ptr<network::ISocket> _socket;
    std::unique_ptr<network::IEndpoint> _serverEndpoint;
    std::chrono::steady_clock::time_point _lastPacketTime;
    std::chrono::steady_clock::time_point _lastInputSentTime;
    std::chrono::seconds _serverTimeOut{5};

    std::thread _networkThread;
    std::atomic<bool> _running{false};

    std::mutex _stateMutex;
    RemoteDisplay _display;

    config::GameConfig _config;
    PlayerId _myPlayerId{0xFF};
    SequenceNumber _sequence{1};

    std::unique_ptr<IRender> _renderer;
    
    MenuState _menuState{MenuState::MainMenu};
    RoomInfo _currentRoom;
    std::string _errorMessage;
    std::string _inputText;
    bool _startGameSent{false};  // Prevent SPACE key spam
    bool _isSpectating{false};
    bool _allPlayersDead{false};
    int _selectedGameOverOption{0};  // 0=Spectate/Wait, 1=Leave
};
}
