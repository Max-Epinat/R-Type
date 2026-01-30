/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** FireCooldownSystem
*/

#include "rtype/server/systems/FireCooldownSystem.hpp"
#include "rtype/common/Components.hpp"

namespace rtype::server
{

// FireCooldownSystem
void FireCooldownSystem::update(float deltaTime, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &)
{
    registry.forEach<FireCooldown>([&](EntityId, FireCooldown &cooldown) {
        if (cooldown.timer > 0.0f) {
            cooldown.timer -= deltaTime;
            if (cooldown.timer < 0.0f) {
                cooldown.timer = 0.0f;
            }
        }
    });
}
}