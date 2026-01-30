#pragma once

#include "rtype/common/Types.hpp"
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace rtype::client
{

struct Vector2f
{
    float x{0.0f};
    float y{0.0f};
};

class RemotePlayer
{
public:
    Vector2f position{};
    std::uint8_t hp{3};
    bool alive{true};
    PlayerPowerUpType player_power_up_type{PlayerPowerUpType::Nothing};
};

class RemoteMonster
{
public:
    Vector2f position{};
    Vector2f velocity{};  // Added for sprite flipping based on direction
    std::uint8_t type{0};
    bool alive{true};
};

class RemoteBullet
{
public:
    Vector2f position{};
    std::uint8_t weaponType{0};
    bool active{true};
    bool fromPlayer{true};
};

class RemotePowerUp
{
public:
    Vector2f position{};
    std::uint8_t type{};
    std::uint8_t value{};
    bool active{true};
};

class RemoteShield
{
public:
    Vector2f position{};
    Vector2f velocity{};  // Added for sprite flipping based on direction
    std::uint8_t type{0};  // Monster type for color matching
    bool alive{true};
};

struct RemoteExplosionEvent
{
    Vector2f position{};
};

struct RemoteDisplay
{
    std::unordered_map<PlayerId, RemotePlayer> players;
    std::unordered_map<EntityId, RemoteMonster> monsters;
    std::unordered_map<EntityId, RemoteBullet> bullets;
    std::unordered_map<EntityId, RemotePowerUp> powerUps;
    std::unordered_map<EntityId, RemoteShield> shields;
    int currentLevel{1};
    mutable std::vector<RemoteExplosionEvent> explosionEvents;

    std::vector<RemoteExplosionEvent> consumeExplosionEvents() const
    {
        std::vector<RemoteExplosionEvent> events;
        events.swap(explosionEvents);
        return events;
    }
};

}
