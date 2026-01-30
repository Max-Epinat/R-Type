/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** Boss2BehaviorSystem
*/

#pragma once

#include "rtype/engine/ISystem.hpp"

namespace rtype::server
{

/**
 * @brief Boss2 Behavior System - Handles boss 2 oscillation and visibility
 */
class Boss2BehaviorSystem : public engine::ISystem
{
public:
    explicit Boss2BehaviorSystem(const config::GameConfig &config)
        : ISystem(config) {}
    void update(float deltaTime, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDestroy) override;

private:
};

} // namespace rtype::server