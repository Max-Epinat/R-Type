/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** ShieldFollowSystem
*/
#include "rtype/server/systems/ShieldFollowSystem.hpp"
#include "rtype/common/Components.hpp"

namespace rtype::server
{

    ShieldFollowSystem::ShieldFollowSystem(const config::GameConfig &config)
    : engine::ISystem(config)
{
}
void ShieldFollowSystem::update(float deltaTime, int &, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDestroy)
{
    // Update shield positions to follow parent monsters
    registry.forEach<ShieldComponent>([&](EntityId shieldId, ShieldComponent &shieldComp) {
        auto *shieldTransform = registry.get<Transform>(shieldId);
        if (!shieldTransform)
            return;
        
        // Check if parent monster is marked for destruction
        if (toDestroy.count(shieldComp.parentMonster) > 0) {
            toDestroy.insert(shieldId);
            return;
        }
        
        // Check if parent monster still exists and is alive
        auto *parentTransform = registry.get<Transform>(shieldComp.parentMonster);
        auto *parentHealth = registry.get<Health>(shieldComp.parentMonster);
        
        if (!parentTransform || !parentHealth || !parentHealth->alive) {
            // Parent is dead or doesn't exist, destroy shield
            toDestroy.insert(shieldId);
            return;
        }
        
        // Get monster configuration for size
        auto *monsterComp = registry.get<MonsterComponent>(shieldComp.parentMonster);
        if (!monsterComp)
            return;
        
        float size = 24.0f;
        auto it = _config.gameplay.MonstersType.find(monsterComp->type);
        if (it != _config.gameplay.MonstersType.end()) {
            size = it->second.size;
        }
        
        // Position shield in front of monster based on velocity direction
        auto *parentVelocity = registry.get<Velocity>(shieldComp.parentMonster);
        float shieldOffsetX = 0.0f;
        float shieldOffsetY = 0.0f;
        
        if (parentVelocity) {
            // Determine primary movement direction and position shield accordingly
            float absVx = std::abs(parentVelocity->vx);
            float absVy = std::abs(parentVelocity->vy);
            
            if (absVx > absVy) {
                // Horizontal movement dominates
                shieldOffsetX = size * 0.6f;
                if (parentVelocity->vx < 0) {
                    shieldOffsetX = -shieldOffsetX;  // Shield on left if moving left
                }
            } else {
                // Vertical movement dominates
                shieldOffsetY = size * 0.6f;
                if (parentVelocity->vy < 0) {
                    shieldOffsetY = -shieldOffsetY;  // Shield above if moving up
                }
            }
        }
        
        // Update shield position
        shieldTransform->x = parentTransform->x + shieldOffsetX + shieldComp.offsetX;
        shieldTransform->y = parentTransform->y + shieldOffsetY + shieldComp.offsetY;
        
        // Copy velocity from parent to keep shield moving with monster
        auto *shieldVelocity = registry.get<Velocity>(shieldId);
        if (shieldVelocity && parentVelocity) {
            shieldVelocity->vx = parentVelocity->vx;
            shieldVelocity->vy = parentVelocity->vy;
        }
        
        // Check if shield is destroyed
        auto *shieldHealth = registry.get<Health>(shieldId);
        if (shieldHealth && !shieldHealth->alive) {
            toDestroy.insert(shieldId);
        }
    });
}
}