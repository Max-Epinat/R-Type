/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** MenuState - Client menu states and UI
*/

#pragma once

#include "rtype/common/Protocol.hpp"
#include <string>
#include <vector>

namespace rtype::client
{

enum class MenuState
{
    MainMenu,           // Main menu: Create/Join/Exit
    CreateRoom,         // Creating a room (enter name)
    JoinRoom,           // Join room (enter IP/see room list)
    Lobby,              // In lobby, waiting for game start
    InGame,             // Playing the game
    GameOver            // Player is dead (game over screen)
};

struct RoomInfo
{
    std::uint32_t roomId{};
    std::string roomName;
    std::uint8_t playerCount{};
    std::uint8_t maxPlayers{};
    bool isHost{false};
};

} // namespace rtype::client
