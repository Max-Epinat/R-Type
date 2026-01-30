/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** MovementSystem
*/

#ifndef MOVEMENTSYSTEM_HPP_
#define MOVEMENTSYSTEM_HPP_

#include "rtype/engine/ISystem.hpp"

namespace rtype::server
{

/**
 * @brief Movement System - Updates positions based on velocities
 */
class MovementSystem : public engine::ISystem
{
public:
    explicit MovementSystem(const config::GameConfig &config)
        : ISystem(config) {}
    void update(float deltaTime, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDestroy) override;
};

}

#endif /* !MOVEMENTSYSTEM_HPP_ */
