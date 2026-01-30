/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** RoomManager - Manages multiple game rooms
*/

#pragma once

#include "rtype/server/Room.hpp"
#include "rtype/common/Types.hpp"

#include <unordered_map>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

namespace rtype::server
{

struct RoomInfo
{
    RoomId roomId;
    std::string roomName;
    PlayerId hostId;
    std::size_t playerCount;
    std::size_t maxPlayers;
    RoomState state;
};

class RoomManager
{
public:
    explicit RoomManager(const config::GameConfig& config);
    ~RoomManager() = default;

    RoomId createRoom(const std::string& roomName, PlayerId hostId);
    bool deleteRoom(RoomId roomId);
    
    std::shared_ptr<Room> getRoom(RoomId roomId);
    std::shared_ptr<Room> getRoomByPlayer(PlayerId playerId);
    
    bool joinRoom(RoomId roomId, PlayerId playerId, std::unique_ptr<network::IEndpoint> endpoint, Timestamp now);
    void leaveRoom(PlayerId playerId);
    
    std::vector<RoomInfo> listRooms() const;
    void updateAllRooms(float dt);
    
    void cleanupEmptyRooms();

private:
    std::unordered_map<RoomId, std::shared_ptr<Room>> _rooms;
    std::unordered_map<PlayerId, RoomId> _playerToRoom;
    RoomId _nextRoomId{1};
    config::GameConfig _config;
    mutable std::mutex _mutex;
};

} // namespace rtype::server
