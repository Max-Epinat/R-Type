/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** EntityFactory - Generic entity creation from templates
*/

#pragma once

#include "rtype/common/Types.hpp"
#include "rtype/common/Components.hpp"
#include "rtype/common/GameConfig.hpp"
#include "rtype/engine/Registry.hpp"

namespace rtype::server
{

class EntityFactory
{
public:
    EntityFactory(engine::Registry &registry, const config::GameConfig &config)
        : _registry(registry), _config(config) {}

    EntityId spawnPlayer(PlayerId id, float x, float y);

    EntityId spawnMonster(std::uint8_t type, bool canShoot, Team team, float x, float y, float vx, float vy);

    EntityId spawnShield(EntityId parentMonster, std::uint8_t type, float x, float y, float vx, float vy);

    EntityId spawnBullet(EntityId owner, bool fromPlayer, float x, float y, float vx, float vy,
                                        WeaponType weaponType, std::uint8_t damage);

    EntityId spawnPowerUp(std::uint8_t type, float x, float y, float vx, float vy);

private:
    engine::Registry &_registry;
    const config::GameConfig &_config;

    void addTransformAndVelocity(EntityId entity, float x, float y, float vx, float vy);
};

} // namespace rtype::server
