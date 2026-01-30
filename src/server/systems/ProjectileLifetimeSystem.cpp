/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** ProjectileLifetimeSystem
*/

#include "rtype/server/systems/ProjectileLifetimeSystem.hpp"
#include "rtype/common/Components.hpp"

namespace rtype::server
{


// ProjectileLifetimeSystem
ProjectileLifetimeSystem::ProjectileLifetimeSystem(const config::GameConfig &config)
    : ISystem(config), _maxLifetime(config.gameplay.bulletLifetime){}

void ProjectileLifetimeSystem::update(float deltaTime, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &destroySet)
{
    registry.forEach<Projectile>([&](EntityId id, Projectile &projectile) {
        if (projectile.persistent)
            return;
        projectile.lifetime += deltaTime;
        
        if (projectile.lifetime > _maxLifetime) {
            destroySet.insert(id);
        }
    });
}

}