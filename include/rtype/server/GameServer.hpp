#pragma once

#include "rtype/common/Components.hpp"
#include "rtype/common/Protocol.hpp"
#include "rtype/common/GameConfig.hpp"
#include "rtype/common/INetwork.hpp"
#include "rtype/server/GameLogicHandler.hpp"
#include "rtype/server/RoomManager.hpp"
#include "ClientHandler.hpp"

#include <atomic>
#include <array>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>

namespace rtype::server
{
class GameServer
{
public:
    explicit GameServer(std::uint16_t port);
    ~GameServer();

    void start();
    void stop();

private:
    void scheduleReceive();
    void handlePacket(const std::uint8_t* data, std::size_t size, std::unique_ptr<network::IEndpoint> sender);
    void handleCreateRoom(const net::CreateRoom &createRoom, std::unique_ptr<network::IEndpoint> sender);
    void handleJoinRoom(const net::JoinRoom &joinRoom, std::unique_ptr<network::IEndpoint> sender);
    void handleLeaveRoom(const net::LeaveRoom &leaveRoom, const std::string& endpointKey);
    void handleStartGame(const net::StartGame &startGame, const std::string& endpointKey);
    void handleRoomList(std::unique_ptr<network::IEndpoint> sender);
    void handlePlayerInput(const net::PlayerInput &input, const std::string& endpointKey);
    void handleDisconnect(const net::DisconnectNotice &notice, const std::string& endpointKey);
    void handleSpectatorMode(const net::SpectatorMode &spec, const std::string& endpointKey);
    
    void updateGameLoop();
    void broadcastRoomStates(Timestamp timestamp);
    void checkClientTimeouts();
    void flushSends(const std::vector<std::uint8_t> &data, const network::IEndpoint &target);
    PlayerInputComponent translateNetworkInput(const net::PlayerInput &input);
    Timestamp nowMilliseconds() const;
    
    PlayerId getOrCreatePlayer(const std::string& endpointKey, std::unique_ptr<network::IEndpoint> sender);
    config::GameConfig loadConfig();

    std::unique_ptr<network::IIOContext> _ioContext;
    std::unique_ptr<network::ISocket> _socket;

    std::thread _networkThread;
    std::thread _gameThread;
    std::atomic<bool> _running{false};

    std::unordered_map<std::string, PlayerId> _endpointToPlayer;
    std::unordered_map<PlayerId, std::unique_ptr<network::IEndpoint>> _playerEndpoints;
    
    config::GameConfig _config;
    std::unique_ptr<RoomManager> _roomManager;

    struct PendingPacket
    {
        std::vector<std::uint8_t> data;
        std::unique_ptr<network::IEndpoint> sender;
    };

    std::mutex _rxMutex;
    std::vector<PendingPacket> _rxQueue;

    SequenceNumber _sequence{1};
    PlayerId _nextPlayerId{0};
};
}

