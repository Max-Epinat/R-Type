/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** CleanupSystem
*/


#include "rtype/server/systems/CleanupSystem.hpp"
#include "rtype/common/Components.hpp"
namespace rtype::server
{

void CleanupSystem::update(float, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &destroySet)
{
    registry.forEach<Health>([&](EntityId id, const Health &health) {
        if (!health.alive || health.hp == 0) {
            destroySet.insert(id);
        }
    });
}
}