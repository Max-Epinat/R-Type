/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** LaserBeamSystem
*/


#pragma once

#include "rtype/engine/ISystem.hpp"
#include "rtype/common/Components.hpp"


namespace rtype::server
{

/**
 * @brief Laser Beam System - Updates laser beam position to track player
 */
class LaserBeamSystem : public engine::ISystem
{
public:
    explicit LaserBeamSystem(const config::GameConfig &config)
        : ISystem(config) {}
    void update(float, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDestroy) override;

private:
    void stopLaser(engine::Registry &registry, WeaponComponent &weapon, std::unordered_set<rtype::EntityId> &toDestroySet);
};
}