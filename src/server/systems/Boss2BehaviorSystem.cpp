/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** Boss2BehaviorSystem
*/

#include "rtype/server/systems/Boss2BehaviorSystem.hpp"
#include "rtype/common/Components.hpp"
#include <cmath>

namespace rtype::server
{

void Boss2BehaviorSystem::update(float deltaTime, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId>&)
{
    registry.view<Boss2Behavior, Transform>([&](EntityId id, Boss2Behavior &behavior, Transform &transform) {
        // Update oscillation timer for smooth movement when visible
        if (behavior.visible) {
            behavior.oscillationTimer += deltaTime * behavior.oscillationSpeed;
            float offset = std::sin(behavior.oscillationTimer) * behavior.oscillationAmplitude;
            transform.y = behavior.baseY + offset;
        }
        
        // Update visibility timer for teleport effect
        behavior.visibilityTimer += deltaTime;
        float currentDuration = behavior.visible ? behavior.visibleDuration : behavior.invisibleDuration;
        if (behavior.visibilityTimer >= currentDuration) {
            behavior.visibilityTimer = 0.0f;
            behavior.visible = !behavior.visible;
            
            // When becoming visible again, teleport to a new random Y position
            if (behavior.visible) {
                // Random Y between 15% and 85% of screen height
                float minY = _config.gameplay.worldHeight * 0.15f;
                float maxY = _config.gameplay.worldHeight * 0.85f;
                float randomRatio = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
                behavior.baseY = minY + randomRatio * (maxY - minY);
                transform.y = behavior.baseY;
                behavior.oscillationTimer = 0.0f; // Reset oscillation
            }
        }
    });
}

}