/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** MovementSystem
*/

#include "rtype/server/systems/MovementSystem.hpp"
#include "rtype/common/Components.hpp"

namespace rtype::server
{

// MovementSystem
void MovementSystem::update(float deltaTime, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &)
{
    registry.forEach<Transform>([&](EntityId id, Transform &transform) {
        auto *velocity = registry.getComponent<Velocity>(id);
        if (velocity) {
            transform.x += velocity->vx * deltaTime;
            transform.y += velocity->vy * deltaTime;
        }
    });
}

}