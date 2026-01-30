/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** CollisionSystem
*/

#pragma once

#include "rtype/engine/ISystem.hpp"
#include "rtype/common/Components.hpp"

namespace rtype::server
{


/**
 * @brief Collision System - Handles projectile collisions with entities
 * Uses per-entity Collider components for accurate collision detection
 */
class CollisionSystem : public engine::ISystem
{
public:
    explicit CollisionSystem(const config::GameConfig &config)
        : ISystem(config) {}
    void update(float deltaTime, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDestroy) override;

private:
    inline bool circleVsCircle(const Transform& a, float ra, const Transform& b, float rb);
    inline bool beamVsCircle(const Transform &projectileTransform, float beamLength, float beamHalfHeight, Transform &monsterTransform, float monsterRadius);
    // std::unordered_set<EntityId> &_destroySet;
};
}