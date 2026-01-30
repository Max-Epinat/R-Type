/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** BoundarySystem
*/

#pragma once
#include "rtype/engine/ISystem.hpp"

namespace rtype::server
{

/**
 * @brief Boundary System - Clamps players to screen, destroys entities outside margins
 */
class BoundarySystem : public engine::ISystem
{
public:
    BoundarySystem(const config::GameConfig &config);
    void update(float, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDestroy) override;

private:
    float _worldWidth, _worldHeight, _margin;
};
}