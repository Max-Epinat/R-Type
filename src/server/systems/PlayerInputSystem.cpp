/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** PlayerInputSystem
*/

#include "rtype/server/systems/PlayerInputSystem.hpp"
#include "rtype/common/Components.hpp"

namespace rtype::server {


void PlayerInputSystem::update(float dt, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &)
{
    registry.view<PlayerInputComponent, Velocity, PlayerPowerUpStatus>([&](
        EntityId id,
        PlayerInputComponent &input,
        Velocity &vel,
        PlayerPowerUpStatus &powerUpStatus)
    {
        vel.vx = 0.f;
        vel.vy = 0.f;

        if (input.left)  vel.vx -= _config.gameplay.playerSpeed;
        if (input.right) vel.vx += _config.gameplay.playerSpeed;
        if (input.up)    vel.vy -= _config.gameplay.playerSpeed;
        if (input.down)  vel.vy += _config.gameplay.playerSpeed;

        if (_config.gameplay.playerMovementDirection == rtype::config::PlayerDirection::LeftToRight) {
            vel.vy = 0;
        }
        if (_config.gameplay.playerMovementDirection == rtype::config::PlayerDirection::TopToBottom) {
            vel.vx = 0;
        }

        if (powerUpStatus.type != PlayerPowerUpType::Nothing
            && std::chrono::steady_clock::now() - powerUpStatus.start_time > std::chrono::seconds((_config.gameplay.shieldDuration))) {
            powerUpStatus.type = PlayerPowerUpType::Nothing;
        }
    });
}
}