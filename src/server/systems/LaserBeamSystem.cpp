/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** LaserBeamSystem
*/

#include "rtype/server/systems/LaserBeamSystem.hpp"


namespace rtype::server {
void LaserBeamSystem::update(float, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDestroySet)
{
    registry.forEach<PlayerComponent>([&](EntityId playerId, PlayerComponent &) {
        auto *weapon = registry.get<WeaponComponent>(playerId);
        if (!weapon || !weapon->laserActive)
            return;

        auto *playerTransform = registry.get<Transform>(playerId);
        auto *health = registry.get<Health>(playerId);
        if (!playerTransform || (health && !health->alive)) {
            stopLaser(registry, *weapon, toDestroySet);
            return;
        }

        if (weapon->activeLaserId == 0) {
            weapon->laserActive = false;
            return;
        }

        auto *beamTransform = registry.get<Transform>(weapon->activeLaserId);
        auto *beamVelocity = registry.get<Velocity>(weapon->activeLaserId);
        auto *projectile = registry.get<Projectile>(weapon->activeLaserId);
        if (!beamTransform || !projectile) {
            stopLaser(registry, *weapon, toDestroySet);
            return;
        }

        beamTransform->x = playerTransform->x + _config.gameplay.bulletSpawnOffsetX;
        beamTransform->y = playerTransform->y + _config.gameplay.bulletSpawnOffsetY;
        if (beamVelocity) {
            beamVelocity->vx = 0.0f;
            beamVelocity->vy = 0.0f;
        }
    });
}

void LaserBeamSystem::stopLaser(engine::Registry &registry, WeaponComponent &weapon, std::unordered_set<rtype::EntityId> &toDestroySet)
{
    if (weapon.activeLaserId != 0 && registry.entityExists(weapon.activeLaserId))
        toDestroySet.insert(weapon.activeLaserId);
    weapon.activeLaserId = 0;
    weapon.laserActive = false;
}

}