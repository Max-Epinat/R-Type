/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** PlayerInputSystem
*/

#pragma once
#include "rtype/engine/Registry.hpp"
#include "rtype/engine/ISystem.hpp"
#include "rtype/common/GameConfig.hpp"

namespace rtype::server {
class PlayerInputSystem : public engine::ISystem {
    public:
        explicit PlayerInputSystem(const config::GameConfig &config)
        : ISystem(config) {}
        void update(float dt, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDestroy);
    private:
};
}