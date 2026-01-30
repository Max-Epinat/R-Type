/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** WeaponDamageSystem
*/

#include "rtype/server/systems/WeaponDamageSystem.hpp"
#include "rtype/common/Components.hpp"

namespace rtype::server {
void WeaponDamageSystem::dealDamage(std::uint8_t damage, Health &health)
{
    if (damage < 1)
        damage = 1;
    health.hp = std::max(health.hp - damage, 0);
    if (health.hp == 0)
        health.alive = false;
}



void WeaponDamageSystem::update(float deltaTime, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDestroy)
{
    registry.forEach<Hurtbox>([&](EntityId id, Hurtbox &hurtbox){
        if (!hurtbox.collidedWith || *hurtbox.collidedWith == id)
            return;
        EntityId projectileId = *(hurtbox.collidedWith);
        if (toDestroy.count(projectileId) > 0)
            return;
        auto *projectile = registry.getComponent<Projectile>(projectileId);
        if (!projectile)
            return;

        if (projectile->weaponType == WeaponType::kWeaponLaserType) {
            float laserDamageInterval = 0.08f;

            projectile->damageTickTimer += deltaTime;
            if (projectile->damageTickTimer < laserDamageInterval || !projectile->persistent)
                return;

            auto *health = registry.get<Health>(id);
            if (!health || !health->alive)
                return;

            dealDamage(projectile->damage, *health);

            projectile->damageTickTimer = 0.0f;

            if (!health->alive)
                toDestroy.insert(id);
        } else {
            if (toDestroy.count(id) > 0)
                return;

            auto *health = registry.get<Health>(id);
            if (!health || !health->alive)
                return;

            if (registry.hasComponent<PlayerPowerUpStatus>(id)) {
                auto *player_power_up_status = registry.get<PlayerPowerUpStatus>(id);
                if (player_power_up_status->type == PlayerPowerUpType::Shield) {
                    hurtbox.collidedWith = std::nullopt;
                    return;
                }
            }
            dealDamage(projectile->damage, *health);

            if (!health->alive)
                toDestroy.insert(id);
            }
    hurtbox.collidedWith = std::nullopt;
    });
}



}