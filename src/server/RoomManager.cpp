/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** RoomManager implementation
*/

#include "rtype/server/RoomManager.hpp"
#include <iostream>
#include <algorithm>

namespace rtype::server
{

RoomManager::RoomManager(const config::GameConfig& config)
    : _config(config)
{
}

RoomId RoomManager::createRoom(const std::string& roomName, PlayerId hostId)
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    RoomId roomId = _nextRoomId++;
    auto room = std::make_shared<Room>(roomId, hostId, roomName, _config);
    _rooms.emplace(roomId, room);
    
    std::cout << "[room-manager] Created room " << roomId << " '" << roomName << "'\n";
    return roomId;
}

bool RoomManager::deleteRoom(RoomId roomId)
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    auto it = _rooms.find(roomId);
    if (it == _rooms.end())
        return false;
    
    for (PlayerId playerId : it->second->getPlayerIds())
    {
        _playerToRoom.erase(playerId);
    }
    
    _rooms.erase(it);
    std::cout << "[room-manager] Deleted room " << roomId << "\n";
    return true;
}

std::shared_ptr<Room> RoomManager::getRoom(RoomId roomId)
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    auto it = _rooms.find(roomId);
    if (it == _rooms.end())
        return nullptr;
    
    return it->second;
}

std::shared_ptr<Room> RoomManager::getRoomByPlayer(PlayerId playerId)
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    auto it = _playerToRoom.find(playerId);
    if (it == _playerToRoom.end())
        return nullptr;

    auto roomIt = _rooms.find(it->second);
    if (roomIt == _rooms.end())
        return nullptr;
    
    return roomIt->second;
}

bool RoomManager::joinRoom(RoomId roomId, PlayerId playerId, std::unique_ptr<network::IEndpoint> endpoint, Timestamp now)
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    auto existingRoom = _playerToRoom.find(playerId);
    if (existingRoom != _playerToRoom.end())
    {
        std::cout << "[room-manager] Player " << static_cast<int>(playerId) 
                  << " already in room " << existingRoom->second << "\n";
        return false;
    }
    
    auto it = _rooms.find(roomId);
    if (it == _rooms.end())
    {
        std::cout << "[room-manager] Room " << roomId << " not found\n";
        return false;
    }
    
    if (it->second->addPlayer(playerId, std::move(endpoint), now))
    {
        _playerToRoom[playerId] = roomId;
        return true;
    }
    
    return false;
}

void RoomManager::leaveRoom(PlayerId playerId)
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    auto it = _playerToRoom.find(playerId);
    if (it == _playerToRoom.end())
        return;
    
    RoomId roomId = it->second;
    auto roomIt = _rooms.find(roomId);
    if (roomIt != _rooms.end())
    {
        roomIt->second->removePlayer(playerId);
    }
    
    _playerToRoom.erase(it);
}

std::vector<RoomInfo> RoomManager::listRooms() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    std::vector<RoomInfo> rooms;
    rooms.reserve(_rooms.size());
    
    for (const auto& [id, room] : _rooms)
    {
        RoomInfo info;
        info.roomId = id;
        info.roomName = room->getName();
        info.hostId = room->getHostId();
        info.playerCount = room->getPlayerCount();
        info.maxPlayers = Room::kMaxPlayersPerRoom;
        info.state = room->getState();
        rooms.push_back(info);
    }
    
    return rooms;
}

void RoomManager::updateAllRooms(float dt)
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    for (auto& [id, room] : _rooms)
    {
        room->updateGame(dt);
    }
}

void RoomManager::cleanupEmptyRooms()
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    std::vector<RoomId> toDelete;
    for (const auto& [id, room] : _rooms)
    {
        if (room->isEmpty())
        {
            toDelete.push_back(id);
        }
    }
    
    for (RoomId id : toDelete)
    {
        _rooms.erase(id);
        std::cout << "[room-manager] Cleaned up empty room " << id << "\n";
    }
}

} // namespace rtype::server
