/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** CleanupSystem
*/

#pragma once

#include "rtype/engine/ISystem.hpp"

namespace rtype::server
{

/**
 * @brief Cleanup System - Destroys dead entities
 */
class CleanupSystem : public engine::ISystem
{
public:
    explicit CleanupSystem(const config::GameConfig &config)
        : ISystem(config) {}
    void update(float, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDestroy) override;

private:
};
}