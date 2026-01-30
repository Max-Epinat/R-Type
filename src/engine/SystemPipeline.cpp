/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** SystemPipeline
*/



#include "rtype/engine/SystemPipeline.hpp"

namespace rtype::engine {

// ========== System Pipeline ==========

void SystemPipeline::addSystem(std::unique_ptr<ISystem> system)
{
    _systems.emplace_back(std::move(system));
}

void SystemPipeline::update(float deltaTime, int &currentLevel, Registry &registry, std::unordered_set<rtype::EntityId> &toDestroySet)
{
    for (auto &system : _systems)
    {
        if (system)
            system->update(deltaTime, currentLevel, registry, toDestroySet);
    }
}

std::size_t SystemPipeline::systemCount() const
{
    return _systems.size();
}

void SystemPipeline::clear()
{
    _systems.clear();
}

}