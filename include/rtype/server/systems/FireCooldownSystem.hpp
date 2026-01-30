#pragma once
#include "rtype/engine/ISystem.hpp"

namespace rtype::server
{


/**
 * @brief Fire Cooldown System - Manages shooting cooldowns
 */
class FireCooldownSystem : public engine::ISystem
{
public:
    explicit FireCooldownSystem(const config::GameConfig &config)
        : ISystem(config) {}
    void update(float deltaTime, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDestroy) override;
};
}