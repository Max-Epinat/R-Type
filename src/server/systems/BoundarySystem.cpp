/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** BoundarySystem
*/

#include "rtype/server/systems/BoundarySystem.hpp"
#include "rtype/common/Components.hpp"

namespace rtype::server
{


// BoundarySystem
BoundarySystem::BoundarySystem(const config::GameConfig &config)
    : ISystem(config), _worldWidth(config.gameplay.worldWidth), _worldHeight(config.gameplay.worldHeight), 
      _margin(config.systems.boundaryMargin){}
void BoundarySystem::update(float, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &_destroySet)
{
    registry.forEach<Transform>([&](EntityId id, Transform &transform) {
        bool isPlayer = registry.hasComponent<PlayerComponent>(id);
        
        if (isPlayer) {
            if (transform.x < 0.0f) transform.x = 0.0f;
            if (transform.x > _worldWidth) transform.x = _worldWidth;
            if (transform.y < 0.0f) transform.y = 0.0f;
            if (transform.y > _worldHeight) transform.y = _worldHeight;
        } else {
            if (transform.x < -_margin || transform.x > _worldWidth + _margin ||
                transform.y < -_margin || transform.y > _worldHeight + _margin) {
                _destroySet.insert(id);
            }
        }
    });
}
}