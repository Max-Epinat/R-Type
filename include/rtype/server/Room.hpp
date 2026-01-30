/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** Room - Game room managing up to 4 players
*/

#pragma once

#include "rtype/common/Types.hpp"
#include "rtype/common/GameConfig.hpp"
#include "rtype/server/GameLogicHandler.hpp"
#include "rtype/server/ClientHandler.hpp"
#include "rtype/common/INetwork.hpp"

#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <atomic>

namespace rtype::server
{

using RoomId = std::uint32_t;

enum class RoomState : std::uint8_t
{
    Waiting,    // Room created, waiting for players
    Playing,    // Game in progress
    Finished    // Game finished
};

class Room
{
public:
    explicit Room(RoomId id, PlayerId hostId, const std::string& roomName, const config::GameConfig& config);
    ~Room() = default;

    RoomId getId() const { return _roomId; }
    const std::string& getName() const { return _roomName; }
    PlayerId getHostId() const { return _hostId; }
    RoomState getState() const { return _state; }
    std::size_t getPlayerCount() const { return _clients.size(); }
    bool isFull() const { return _clients.size() >= kMaxPlayersPerRoom; }
    bool isEmpty() const { return _clients.empty(); }
    
    bool addPlayer(PlayerId playerId, std::unique_ptr<network::IEndpoint> endpoint, Timestamp now);
    void removePlayer(PlayerId playerId);
    bool hasPlayer(PlayerId playerId) const;
    
    void startGame();
    void updateGame(float dt);
    
    GameLogicHandler& getGameLogic() { return _gameLogic; }
    const std::unordered_map<PlayerId, ClientHandler>& getClients() const { return _clients; }
    std::unordered_map<PlayerId, ClientHandler>& getClients() { return _clients; }
    
    std::vector<PlayerId> getPlayerIds() const;
    
    void checkPlayerDeaths();
    bool areAllPlayersDead() const;
    void resetDeathFlags() { _deadPlayers.clear(); _allPlayersDeadNotified = false; }
    bool hasNotifiedAllDead() const { return _allPlayersDeadNotified; }
    void setAllPlayersDeadNotified(bool notified) { _allPlayersDeadNotified = notified; }


    static constexpr std::size_t kMaxPlayersPerRoom = 4;

private:
    RoomId _roomId;
    std::string _roomName;
    PlayerId _hostId;
    RoomState _state;
    std::unordered_map<PlayerId, ClientHandler> _clients;
    GameLogicHandler _gameLogic;
    config::GameConfig _config;
    std::unordered_map<PlayerId, bool> _deadPlayers;  // Track which players have died
    bool _allPlayersDeadNotified{false};
};

} // namespace rtype::server
