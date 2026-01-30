/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** GameLogicHandler
*/

#include "rtype/server/GameLogicHandler.hpp"
#include "rtype/server/GameSystems.hpp"
// #include "rtype/server/PlayerInputSystem.hpp"
// #include "rtype/server/ShootingSystem.hpp"
// #include "rtype/server/WeaponDamageSystem.hpp"

#include "rtype/server/GameServer.hpp"
#include "rtype/common/GameConfig.hpp"
#include "rtype/server/ClientHandler.hpp"
#include "rtype/engine/SystemPipeline.hpp"
#include <iostream>
#include <algorithm>
#include <random>
#include <limits>
#include <array>
#include <cmath>

namespace
{
}

namespace rtype::server {

GameLogicHandler::GameLogicHandler(config::GameConfig config) 
    : _config(config), _entityFactory(_registry, _config)
{
    _currentLevel = 0;
    initializeSystems();
    
}

void GameLogicHandler::initializeSystems()
{
    std::cout << "[logic] Initializing ECS systems from configuration...\n";
    
    if (_config.systems.movementSystem) {
        _systemPipeline.addSystem(std::make_unique<MovementSystem>(_config));
        std::cout << "[logic] - MovementSystem loaded\n";
    }

    _systemPipeline.addSystem(std::make_unique<LaserBeamSystem>(_config));
    std::cout << "[logic] - LaserBeamSystem loaded\n";
    
    if (_config.systems.fireCooldownSystem) {
        _systemPipeline.addSystem(std::make_unique<FireCooldownSystem>(_config));
        std::cout << "[logic] - FireCooldownSystem loaded\n";
    }
    
    if (_config.systems.projectileLifetimeSystem) {
        _systemPipeline.addSystem(std::make_unique<ProjectileLifetimeSystem>(_config));
        std::cout << "[logic] - ProjectileLifetimeSystem loaded\n";
    }
    
    if (_config.systems.collisionSystem) {
        _systemPipeline.addSystem(std::make_unique<CollisionSystem>(_config));
        std::cout << "[logic] - CollisionSystem loaded\n";
    }
    
    if (_config.systems.boundarySystem) {
        _systemPipeline.addSystem(std::make_unique<BoundarySystem>(_config));
        std::cout << "[logic] - BoundarySystem loaded (margin: " << _config.systems.boundaryMargin << ")\n";
    }
    
    if (_config.systems.cleanupSystem) {
        _systemPipeline.addSystem(std::make_unique<CleanupSystem>(_config));
        std::cout << "[logic] - CleanupSystem loaded\n";
    }

    _systemPipeline.addSystem(std::make_unique<PlayerInputSystem>(_config));
    std::cout << "[logic] - PlayerInputSystem loaded\n";

    _systemPipeline.addSystem(std::make_unique<ShootingSystem>(_config));
    std::cout << "[logic] - ShootingSystem loaded\n";

    _systemPipeline.addSystem(std::make_unique<WeaponDamageSystem>(_config));
    std::cout << "[logic] - WeaponDamageSystem loaded\n";

    _systemPipeline.addSystem(std::make_unique<PowerUpSystem>(_config));
    std::cout << "[logic] - PowerUpSystem loaded\n";
    
    // Initialize LevelSystem (optional, requires spawner)
    if (_config.systems.levelSystem) {
            auto levelSystem = std::make_unique<LevelSystem>(
                _config
            );
            _systemPipeline.addSystem(std::move(levelSystem));
            std::cout << "[logic] - LevelSystem loaded\\n";
    }
    
    // Boss2BehaviorSystem - handles boss 2 oscillation and visibility
    _systemPipeline.addSystem(std::make_unique<Boss2BehaviorSystem>(_config));
    std::cout << "[logic] - Boss2BehaviorSystem loaded\\n";
    
    // ShieldFollowSystem - updates shield positions to follow parent monsters
    _systemPipeline.addSystem(std::make_unique<ShieldFollowSystem>(_config));
    std::cout << "[logic] - ShieldFollowSystem loaded\n";

    for (auto monsters : _config.gameplay.MonstersType) {
        for (auto positions : monsters.second.defaultPositions) {
            float vx, vy;
            _config.getDirectionVelocity(_config.gameplay.monsterMovement, vx,vy, monsters.second.speed);
            std::cout << "spawning monsters at " << positions.first << "/" << positions.second << std::endl;
            _entityFactory.spawnMonster(monsters.first, monsters.second.canShoot, monsters.second.team, positions.first, positions.second, vx, vy);
        }
    }
    
    std::cout << "[logic] System initialization complete\\n";
}

GameLogicHandler::~GameLogicHandler()
{
}

const engine::Registry &GameLogicHandler::getRegistry() const
{
    return this->_registry;
}

engine::Registry &GameLogicHandler::getRegistry()
{
    return this->_registry;
}

EntityId GameLogicHandler::spawnPlayer(PlayerId playerId)
{
    const float spawnX = _config.gameplay.playerSpawnX;
    const float spawnY = _config.gameplay.playerSpawnYBase + _config.gameplay.playerSpawnYSpacing * static_cast<float>(playerId);
    
    return _entityFactory.spawnPlayer(playerId, spawnX, spawnY);
}


void GameLogicHandler::manageInputs(const PlayerInputComponent input, const EntityId entity)
{
    if (!_registry.hasComponent<PlayerComponent>(entity))
        return;

    auto *inputComp = _registry.get<PlayerInputComponent>(entity);
    if (!inputComp)
        inputComp = &_registry.emplace<PlayerInputComponent>(entity);

    *inputComp = input;

}

void GameLogicHandler::markDestroy(EntityId id)
{
    toDestroySet.insert(id);
}


void GameLogicHandler::destroyEntity(rtype::EntityId id)
{
    _registry.destroyEntity(id);
}

void GameLogicHandler::updateGame(const float dt)
{
    toDestroySet.clear();

    _systemPipeline.update(dt, _currentLevel, _registry, toDestroySet);

    if (_currentLevel != _prevLevel) {
        _prevLevel = _currentLevel;
        _levelChanged = true;
    }
}

void GameLogicHandler::destroyEntityDestructionList()
{
    for (auto id : toDestroySet)
        destroyEntity(id);
}

const std::unordered_set<rtype::EntityId> &GameLogicHandler::getEntityDestructionSet()
{
    return this->toDestroySet;
}

int GameLogicHandler::getCurrentLevel() const
{
    return _currentLevel;
}

bool GameLogicHandler::hasLevelChanged()
{
    if (_levelChanged)
    {
        _levelChanged = false;
        return true;
    }
    return false;
}
}
