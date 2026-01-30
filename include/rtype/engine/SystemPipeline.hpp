/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** SystemsPipeline
*/

#pragma once
#include <memory>
#include "rtype/engine/Registry.hpp"
#include "rtype/engine/ISystem.hpp"

namespace rtype::engine
{


/**
 * @brief Container for multiple systems that run in sequence
 * 
 * The SystemPipeline allows you to organize and run multiple systems
 * in a specific order each frame.
 */
class SystemPipeline
{
public:
    /**
     * @brief Add a system to the pipeline
     * @param system Unique pointer to the system (pipeline takes ownership)
     * 
     * Systems are executed in the order they are added.
     */
    void addSystem(std::unique_ptr<ISystem> system);
    
    /**
     * @brief Run all systems in the pipeline
     * @param deltaTime Time elapsed since last update
     * @param registry The ECS registry
     */
    void update(float deltaTime, int &currentLevel, Registry &registry, std::unordered_set<rtype::EntityId> &toDestroy);
    
    /**
     * @brief Get the number of systems in the pipeline
     */
    std::size_t systemCount() const;
    
    /**
     * @brief Clear all systems from the pipeline
     */
    void clear();

private:
    std::vector<std::unique_ptr<ISystem>> _systems;
};

}