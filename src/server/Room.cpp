/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** Room implementation
*/

#include "rtype/server/Room.hpp"
#include <iostream>

namespace rtype::server
{

Room::Room(RoomId id, PlayerId hostId, const std::string& roomName, const config::GameConfig& config)
    : _roomId(id)
    , _roomName(roomName)
    , _hostId(hostId)
    , _state(RoomState::Waiting)
    , _gameLogic(config)
    , _config(config)
{
    std::cout << "[room:" << _roomId << "] Created room '" << _roomName 
              << "' with host player " << static_cast<int>(hostId) << "\n";
}

bool Room::addPlayer(PlayerId playerId, std::unique_ptr<network::IEndpoint> endpoint, Timestamp now)
{
    if (isFull())
    {
        std::cout << "[room:" << _roomId << "] Cannot add player " << static_cast<int>(playerId) 
                  << " - room is full\n";
        return false;
    }
    
    if (_state != RoomState::Waiting)
    {
        std::cout << "[room:" << _roomId << "] Cannot add player " << static_cast<int>(playerId) 
                  << " - game already started\n";
        return false;
    }
    
    EntityId entity = _gameLogic.spawnPlayer(playerId);
    _clients.emplace(playerId, ClientHandler{playerId, std::move(endpoint), now, entity});
    
    std::cout << "[room:" << _roomId << "] Player " << static_cast<int>(playerId) 
              << " joined (" << _clients.size() << "/" << kMaxPlayersPerRoom << ")\n";
    
    return true;
}

void Room::removePlayer(PlayerId playerId)
{
    auto it = _clients.find(playerId);
    if (it != _clients.end())
    {
        EntityId entity = it->second.getEntityId();
        _gameLogic.destroyEntity(entity);
        _clients.erase(it);
        
        std::cout << "[room:" << _roomId << "] Player " << static_cast<int>(playerId) 
                  << " left (" << _clients.size() << "/" << kMaxPlayersPerRoom << ")\n";
        
        // Check if host left and assign new host
        if (playerId == _hostId && !_clients.empty())
        {
            PlayerId oldHost = _hostId;
            _hostId = _clients.begin()->first;
            std::cout << "[room:" << _roomId << "] Host changed from " << static_cast<int>(oldHost) 
                      << " to player " << static_cast<int>(_hostId) << "\n";
            // Note: The caller (GameServer) should notify clients about the host change
        }
    }
}

bool Room::hasPlayer(PlayerId playerId) const
{
    return _clients.find(playerId) != _clients.end();
}

void Room::startGame()
{
    if (_state != RoomState::Waiting)
    {
        std::cout << "[room:" << _roomId << "] Cannot start - game already started\n";
        return;
    }
    
    if (_clients.empty())
    {
        std::cout << "[room:" << _roomId << "] Cannot start - no players\n";
        return;
    }
    
    _state = RoomState::Playing;
    std::cout << "[room:" << _roomId << "] Game started with " << _clients.size() << " players\n";
}

void Room::updateGame(float dt)
{
    // Only update game if in playing state AND not all players dead
    if (_state == RoomState::Playing && !areAllPlayersDead())
    {
        _gameLogic.updateGame(dt);
    }
}

std::vector<PlayerId> Room::getPlayerIds() const
{
    std::vector<PlayerId> playerIds;
    playerIds.reserve(_clients.size());
    for (const auto& [id, _] : _clients)
    {
        playerIds.push_back(id);
    }
    return playerIds;
}

void Room::checkPlayerDeaths()
{
    const auto& registry = _gameLogic.getRegistry();
    
    for (const auto& [playerId, client] : _clients)
    {
        EntityId entityId = client.getEntityId();
        
        // Check if this player's entity is alive
        const auto* playerComp = registry.getComponent<PlayerComponent>(entityId);
        const auto* health = registry.getComponent<Health>(entityId);
        
        if (playerComp && health)
        {
            // If player just died (was not marked as dead before but is now)
            if (!health->alive && _deadPlayers.find(playerId) == _deadPlayers.end())
            {
                _deadPlayers[playerId] = true;
                std::cout << "[room:" << _roomId << "] Player " << static_cast<int>(playerId) << " died\n";
            }
        }
    }
}

bool Room::areAllPlayersDead() const
{
    if (_clients.empty())
        return false;
    
    const auto& registry = _gameLogic.getRegistry();
    
    for (const auto& [playerId, client] : _clients)
    {
        EntityId entityId = client.getEntityId();
        const auto* playerComp = registry.getComponent<PlayerComponent>(entityId);
        const auto* health = registry.getComponent<Health>(entityId);
        
        if (playerComp && health && health->alive)
        {
            return false;  // At least one player is still alive
        }
    }
    
    return true;  // All players are dead
}

} // namespace rtype::server
