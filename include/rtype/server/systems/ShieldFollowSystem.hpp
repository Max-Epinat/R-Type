/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** ShieldFollowSystem
*/

#pragma once

#include "rtype/engine/ISystem.hpp"
namespace rtype::server {

class ShieldFollowSystem : public engine::ISystem
{
public:
    ShieldFollowSystem(const config::GameConfig &config);
    void update(float deltaTime, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDestroy) override;

private:
};

}