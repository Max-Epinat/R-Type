/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** LevelSystem
*/

#include "rtype/server/systems/LevelSystem.hpp"
#include "rtype/common/Components.hpp"
#include <iostream>
#include "rtype/server/EntityFactory.hpp"

namespace rtype::server {

// MonsterSpawnerSystem
MonsterSpawnerSystem::MonsterSpawnerSystem(const config::GameConfig &config)
    : ISystem(config), _monstersToSpawn(0), _monstersSpawned(0), _spawnTimer(0.0f)
{
    // If no level system, start continuous spawning
    if (!config.systems.levelSystem) {
        startSpawning(999999);  // Effectively infinite spawning
        std::cout << "[MonsterSpawner] Starting continuous spawning (no LevelSystem)\n";
    }
}

void MonsterSpawnerSystem::update(float deltaTime, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &)
{
    // Check if spawning is complete
    if (_monstersSpawned >= _monstersToSpawn)
        return;
    
    // Update spawn timer
    _spawnTimer += deltaTime;
    
    // Spawn monsters at intervals from config
    if (_spawnTimer >= _config.gameplay.monsterSpawnDelay) {
        spawnMonster(registry);
        _spawnTimer = 0.0f;
        _monstersSpawned++;
    }
}

void MonsterSpawnerSystem::startSpawning(int monstersToSpawn)
{
    _monstersToSpawn = monstersToSpawn;
    _monstersSpawned = 0;
    _spawnTimer = 0.0f;
    
    std::cout << "[MonsterSpawner] Starting to spawn " << monstersToSpawn << " monsters\n";
}

void MonsterSpawnerSystem::spawnMonster(engine::Registry &registry)
{
    // Use config spawn position method
    std::uniform_real_distribution<float> distRandom(0.0f, 1.0f);
    const float randomValue = distRandom(_rng);
    float spawnX, spawnY;
    _config.getSpawnPosition(spawnX, spawnY, randomValue);
    
    // Calculate monster type based on weighted spawn chances from config
    int totalWeight = 0;
    for (const auto& [type, monsterConfig] : _config.gameplay.MonstersType) {
        totalWeight += monsterConfig.SpawnWeight;
    }
    
    std::uniform_int_distribution<int> typeDist(0, totalWeight - 1);
    int roll = typeDist(_rng);
    std::uint8_t monsterType = 0;
    
    int cumulative = 0;
    for (const auto& [type, monsterConfig] : _config.gameplay.MonstersType) {
        cumulative += monsterConfig.SpawnWeight;
        if (roll < cumulative) {
            monsterType = static_cast<std::uint8_t>(type);
            break;
        }
    }
    
    // Get velocity from config based on monster type and movement direction
    float monsterVx, monsterVy;
    float monsterSpeedMultiplier = _config.gameplay.MonstersType.find(monsterType)->second.speed;
    _config.getDirectionVelocity(_config.gameplay.monsterMovement, monsterVx, monsterVy, 
                                 _config.gameplay.scrollSpeed * monsterSpeedMultiplier);
    EntityFactory factory(registry, _config);
    factory.spawnMonster(monsterType, _config.gameplay.MonstersType.find(monsterType)->second.canShoot, _config.gameplay.MonstersType.find(monsterType)->second.team, spawnX, spawnY, monsterVx, monsterVy);
}

void MonsterSpawnerSystem::spawnBoss(std::uint8_t bossType, engine::Registry &registry)
{
    // Spawn boss on the right side of the screen, positioned lower
    float spawnX = _config.gameplay.worldWidth - 200.0f;  // Right side of screen
    float spawnY = _config.gameplay.worldHeight * 0.75f;  // Lower on screen
    
    // Boss is stationary - doesn't move (oscillation handled by Boss2BehaviorSystem for boss 2)
    float bossVx = 0.0f;
    float bossVy = 0.0f;
    
    std::cout << "[MonsterSpawner] Spawning BOSS (type " << static_cast<int>(bossType) << ") - STATIONARY on right side!\\n";
    EntityFactory factory(registry, _config);
    EntityId bossId = factory.spawnMonster(bossType, true, Team::Monster, spawnX, spawnY, bossVx, bossVy);
    
    // Add Boss2Behavior component for boss type 7 (second boss)
    if (bossType == static_cast<std::uint8_t>(_config.gameplay.boss2MonsterType)) {
        Boss2Behavior behavior{};
        behavior.baseY = spawnY;
        behavior.oscillationSpeed = 1.5f;        // Movement speed
        behavior.oscillationAmplitude = 120.0f;  // Smooth oscillation range
        behavior.visibleDuration = 4.0f;         // 4 seconds visible
        behavior.invisibleDuration = 2.0f;       // 2 seconds invisible
        behavior.visible = true;
        registry.addComponent<Boss2Behavior>(bossId, behavior);
        
        // Configure Boss2 to shoot slow big projectiles
        auto *cooldown = registry.get<FireCooldown>(bossId);
        if (cooldown) {
            cooldown->cooldownTime = 2.0f;  // Slow fire rate (every 2 seconds)
        }
        
        std::cout << "[MonsterSpawner] Added Boss2Behavior for oscillation and visibility\\n";
    }
}

// LevelSystem
LevelSystem::LevelSystem(
                         const config::GameConfig &config)
    : ISystem(config),  _spawner(config), _waveChanged(false)
{}

void LevelSystem::update(float deltaTime, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDelete)
{
    // if (currentLevel == 0) {
    //     startWave(1, currentLevel, registry);
    //     return;
    // }
    // Check if spawning is complete and all monsters are dead
    if (_spawner.isSpawningComplete() && isWaveComplete(registry)) {
        // Automatically start next wave if within level limit
        int nextWave = currentLevel + 1;
        if (nextWave <= _config.gameplay.numberOfLevels) {
            startWave(nextWave, currentLevel, registry);
        } else {
            std::cout << "[LevelSystem] All waves completed!\n";
        }
    }
    _spawner.update(deltaTime, currentLevel, registry, toDelete);
}

void LevelSystem::startWave(int waveNumber, int &currentLevel, engine::Registry &registry)
{
    currentLevel = waveNumber;
    std::cout << "updating wave num to " << waveNumber << std::endl;
    // _waveChanged = true;
    
    // Calculate monsters for this wave
    int monstersToSpawn = _config.gameplay.monsterPerLevel * waveNumber;
    
    // Check if this is a boss level
    bool isBossLevel = (waveNumber == _config.gameplay.bossLevel);
    bool isBoss2Level = (waveNumber == _config.gameplay.boss2Level);
    
    if (isBossLevel) {
        std::cout << "[LevelSystem] Starting BOSS Wave " << waveNumber << "!\n";
        // Spawn the boss
        _spawner.spawnBoss(static_cast<std::uint8_t>(_config.gameplay.bossMonsterType), registry);
        // Also spawn some regular monsters alongside the boss
        monstersToSpawn = _config.gameplay.monsterPerLevel;
    } else if (isBoss2Level) {
        std::cout << "[LevelSystem] Starting BOSS 2 Wave " << waveNumber << "!\n";
        // Spawn the second boss
        _spawner.spawnBoss(static_cast<std::uint8_t>(_config.gameplay.boss2MonsterType), registry);
        // Also spawn some regular monsters alongside the boss
        monstersToSpawn = _config.gameplay.monsterPerLevel * 2;
    } else {
        std::cout << "[LevelSystem] Starting Wave " << waveNumber 
                  << " with " << monstersToSpawn << " monsters\n";
    }
    
    // Tell spawner to start spawning
    _spawner.startSpawning(monstersToSpawn);
}

bool LevelSystem::isWaveComplete(engine::Registry &registry) const
{
    // Check if there are any living monsters
    bool hasLivingMonsters = false;
    registry.forEach<MonsterComponent>([&](EntityId id, const MonsterComponent &) {
        auto *health = registry.get<Health>(id);
        if (health && health->alive) {
            hasLivingMonsters = true;
        }
    });
    return !hasLivingMonsters;
}

}