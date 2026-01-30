#pragma once

#include <unordered_set>
#include "rtype/engine/Registry.hpp"
#include "rtype/common/GameConfig.hpp"


namespace rtype::engine
{

// ========== System Abstractions ==========

/**
 * @brief Base class for all systems in the ECS
 */
class ISystem
{
public:
    ISystem(const config::GameConfig &config) : _config(config) {}
    virtual ~ISystem() = default;

    /**
     * @brief Update this system
     * @param deltaTime Time elapsed since last update (in seconds)
     * @param currentLevel A reference to the current level of the game.
     * @param registry The ECS registry containing all entities and components
     * @param toDestroy The list of entities that will be destroyed by the engine
     */
    virtual void update(float deltaTime, int &currentLevel, Registry &registry, std::unordered_set<rtype::EntityId> &toDestroy) = 0;

protected:
    const config::GameConfig &_config;
    

};

}