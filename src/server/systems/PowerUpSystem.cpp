/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** PowerUpSystem
*/

#include "rtype/server/systems/PowerUpSystem.hpp"
#include <random>
#include <algorithm>
#include "rtype/server/EntityFactory.hpp"
#include <iostream>
#include <random>
namespace rtype::server {
void PowerUpSystem::update(float dt, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDestroy)
{
    _powerUpSpawnTimer += dt;
    if (_config.gameplay.powerUpsEnabled && _powerUpSpawnTimer >= _config.gameplay.powerUpSpawnDelay)
    {
        _powerUpSpawnTimer = 0.0f;
        std::uniform_real_distribution<float> distRandom(0.0f, 1.0f);
        std::uniform_int_distribution<int> power_up_types_random_plage(
            static_cast<int>(PowerUpTypes::MIN_VAL),
            static_cast<int>(PowerUpTypes::MAX_VAL)
        );
        const float randomValue = distRandom(_rng);

        float spawnX = _config.gameplay.worldWidth * _config.gameplay.powerUpSpawnCenterX + 
                       (randomValue - 0.5f) * _config.gameplay.powerUpSpawnRandomRange;
        float spawnY = _config.gameplay.worldHeight * _config.gameplay.powerUpSpawnCenterY + 
                       (randomValue - 0.5f) * _config.gameplay.powerUpSpawnRandomRange;

        spawnX = std::clamp(spawnX, _config.gameplay.powerUpSpawnMargin, 
                           _config.gameplay.worldWidth - _config.gameplay.powerUpSpawnMargin);
        spawnY = std::clamp(spawnY, _config.gameplay.powerUpSpawnMargin, 
                           _config.gameplay.worldHeight - _config.gameplay.powerUpSpawnMargin);

        const std::uint8_t power_up_types = static_cast<std::uint8_t>(power_up_types_random_plage(_rng));

        float powerUpVx, powerUpVy;
        _config.getDirectionVelocity(_config.gameplay.powerUpSpawnSide, powerUpVx, powerUpVy, _config.gameplay.scrollSpeed);

        EntityFactory entityFactory(registry, _config);
        entityFactory.spawnPowerUp(power_up_types, spawnX, spawnY,
                                   powerUpVx * _config.gameplay.powerUpSpeedMultiplier,
                                   powerUpVy * _config.gameplay.powerUpSpeedMultiplier);
        
        std::cout << "[server] Spawned powerup id_type : " << power_up_types << " at (" << spawnX << ", " << spawnY << ")" << std::endl;
    }

    registry.view<PowerUp, Transform>([&](EntityId powerUpId, PowerUp &powerUp, Transform &powerUpTransform) {

        const float powerMargin = _config.gameplay.powerUpBoundaryMargin;
        if (powerUpTransform.x < -powerMargin ||
            powerUpTransform.x > _config.gameplay.worldWidth + powerMargin ||
            powerUpTransform.y < -powerMargin ||
            powerUpTransform.y > _config.gameplay.worldHeight + powerMargin)
        {
            toDestroy.insert(powerUpId);
            return;
        }

        registry.each<PlayerComponent>([&](EntityId playerId, [[maybe_unused]] PlayerComponent &player) {
            if (toDestroy.count(powerUpId) > 0)
                return;
            
            auto *playerTransform = registry.get<Transform>(playerId);
            if (!playerTransform)
                return;
            
            auto *playerCollider = registry.get<Collider>(playerId);
            auto *powerUpCollider = registry.get<Collider>(powerUpId);
            const float playerRadius = playerCollider ? playerCollider->radius : 10.0f;
            const float powerUpRadius = powerUpCollider ? powerUpCollider->radius : 8.0f;
            const float combinedRadius = playerRadius + powerUpRadius;
            
            const float dx = playerTransform->x - powerUpTransform.x;
            const float dy = playerTransform->y - powerUpTransform.y;
            if ((dx * dx + dy * dy) < combinedRadius * combinedRadius)
            {
                switch (static_cast<PowerUpTypes>(powerUp.type)) {
                    case PowerUpTypes::WeaponUpgrade: {
                        auto *weapon = registry.get<WeaponComponent>(playerId);
                        if (!weapon)
                            weapon = &registry.emplace<WeaponComponent>(playerId, WeaponComponent{});

                        const bool hadLaser = weapon->laserUnlocked;

                        incrementPowerUpProgress(*weapon);

                        if (weapon->laserUnlocked && hadLaser && weapon->weaponType == WeaponType::kWeaponLaserType)
                        {
                            if (weapon->weaponLevel < 3)
                                weapon->weaponLevel++;
                        }
                        break;
                    }
                    case PowerUpTypes::Shield: {
                        auto *power_up_status = registry.get<PlayerPowerUpStatus>(playerId);
                        power_up_status->type = PlayerPowerUpType::Shield;
                        power_up_status->start_time = std::chrono::steady_clock::now();
                        break;
                    }
                    default:
                        break;
                }
                toDestroy.insert(powerUpId);
            }
        });
    });
}

void PowerUpSystem::incrementPowerUpProgress(WeaponComponent &weapon)
{
    if (weapon.powerUpsCollected < std::numeric_limits<std::uint16_t>::max())
        ++weapon.powerUpsCollected;

    const auto clampThreshold = [](std::uint16_t value) {
        return static_cast<std::uint16_t>(std::max<std::uint16_t>(value, 1));
    };

    const std::uint16_t laserThreshold = clampThreshold(_config.gameplay.powerUpsForLaser);
    if (!weapon.laserUnlocked && weapon.powerUpsCollected >= laserThreshold)
    {
        weapon.laserUnlocked = true;
        if (weapon.weaponType == WeaponType::kWeaponBasicType)
        {
            weapon.weaponType = WeaponType::kWeaponLaserType;
            weapon.weaponLevel = std::max<std::uint8_t>(weapon.weaponLevel, 1);
        }
    }

    const std::uint16_t rocketThreshold = clampThreshold(_config.gameplay.powerUpsForRocket);
    if (!weapon.rocketUnlocked && weapon.powerUpsCollected >= rocketThreshold)
    {
        weapon.rocketUnlocked = true;
        if (weapon.weaponType == WeaponType::kWeaponBasicType)
        {
            weapon.weaponType = WeaponType::kWeaponRocketType;
            weapon.weaponLevel = std::max<std::uint8_t>(weapon.weaponLevel, 1);
        }
    }
}
}