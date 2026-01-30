/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** LevelSystem
*/

#pragma once
#include "rtype/engine/ISystem.hpp"
#include <random>

namespace rtype::server
{

/**
 * @brief Monster Spawner System - Spawns monsters based on wave configuration
 */
class MonsterSpawnerSystem : public engine::ISystem
{
public:
    MonsterSpawnerSystem(const config::GameConfig &config);
    
    void update(float deltaTime, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDestroy) override;
    
    // Control spawning
    void startSpawning(int monstersToSpawn);
    bool isSpawningComplete() const { return _monstersSpawned >= _monstersToSpawn; }
    void spawnBoss(std::uint8_t bossType, engine::Registry &registry);
private:
    void spawnMonster(engine::Registry &registry);
    
    std::mt19937 _rng{std::random_device{}()};
    
    int _monstersToSpawn;
    int _monstersSpawned;
    float _spawnTimer;
};

/**
 * @brief Level System - Manages wave progression and level tracking
 */
class LevelSystem : public engine::ISystem
{
public:
    LevelSystem(
                const config::GameConfig &config);
    
    void update(float deltaTime, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDestroy) override;
    
    // Query methods
    // int getCurrentWave() const { return _currentLevel; }
    // bool hasWaveChanged();
    
private:
    bool isWaveComplete(engine::Registry &registry) const;
    void startWave(int waveNumber, int &currentLevel, engine::Registry &registry);
    MonsterSpawnerSystem _spawner;
    
    // int _currentLevel{0};
    bool _waveChanged;
};
}