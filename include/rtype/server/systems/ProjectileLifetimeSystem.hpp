/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** ProjectileLifetimeSystem
*/

#ifndef PROJECTILELIFETIMESYSTEM_HPP_
#define PROJECTILELIFETIMESYSTEM_HPP_

#include "rtype/engine/ISystem.hpp"

namespace rtype::server {

/**
 * @brief Projectile Lifetime System - Ages and removes old projectiles
 */
class ProjectileLifetimeSystem : public engine::ISystem
{
public:
    ProjectileLifetimeSystem(const config::GameConfig &config);
    void update(float deltaTime, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDestroy) override;

private:
    float _maxLifetime;
};
}


#endif /* !PROJECTILELIFETIMESYSTEM_HPP_ */
