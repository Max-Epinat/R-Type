/*
** EPITECH PROJECT, 2026
** rtype
** File description:
** ShootingSystem
*/

#include "rtype/server/systems/ShootingSystem.hpp"
#include "rtype/common/Components.hpp"
#include "rtype/common/GameConfig.hpp"
#include <algorithm>
#include <array>
#include <cmath>
namespace rtype::server {


void ShootingSystem::startLaserBeam(EntityId entity, WeaponComponent &weapon, PlayerComponent &, FireCooldown &cooldown, engine::Registry &registry)
{
    // auto *playerComp = _registry.get<PlayerComponent>(entity);
    auto *playerTransform = registry.get<Transform>(entity);
    if (!playerTransform)
        return;

    // Calculate bullet velocity to determine spawn offset direction
    float bulletVx = 0.0f, bulletVy = 0.0f;
    _config.getDirectionVelocity(this->_config.gameplay.bulletDirection, bulletVx, bulletVy, this->_config.gameplay.bulletSpeed);
    
    // Adjust spawn offset based on bullet direction
    float offsetX = this->_config.gameplay.bulletSpawnOffsetX;
    float offsetY = this->_config.gameplay.bulletSpawnOffsetY;
    
    if (bulletVx < 0.0f) {
        offsetX = -offsetX;  // Flip X offset for left-moving bullets
    }
    if (bulletVy < 0.0f) {
        offsetY = -offsetY;  // Flip Y offset for up-moving bullets
    }

    float startX = playerTransform->x + offsetX;
    float startY = playerTransform->y + offsetY;
    std::uint8_t damage = computeLaserDamage(weapon.weaponLevel);

    
    EntityFactory entityfactory(registry, _config);
    EntityId beamId = entityfactory.spawnBullet(entity, true, startX, startY,
                                                  0.0f, 0.0f, weapon.weaponType, damage);

    if (auto *projectile = registry.get<Projectile>(beamId)) {
        projectile->persistent = true;
        projectile->lifetime = 0.0f;
        projectile->damageTickTimer = 0.0f;
    }

    weapon.laserActive = true;
    weapon.activeLaserId = beamId;

    cooldown.timer = 0.0f;
}


void ShootingSystem::handleLaserFireInput(EntityId entity, WeaponComponent &weapon, PlayerComponent &player, FireCooldown &cooldown, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDestroySet)
{
    if (!weapon.laserActive)
        startLaserBeam(entity, weapon, player, cooldown, registry);
    // } else if (weapon.laserActive) {
    //     stopActiveLaser(weapon, registry, toDestroySet);
    // }
}

std::uint8_t ShootingSystem::computeLaserDamage(std::uint8_t weaponLevel) const
{
    std::uint8_t level = weaponLevel;
    if (level < 1)
        level = 1;
    std::uint32_t baseDamage = static_cast<std::uint32_t>(_config.gameplay.weaponDamageLaser) * level;
    std::uint32_t reduced = baseDamage / 3u;
    if (reduced < 1)
        reduced = 1;
    if (reduced > 255)
        reduced = 255;
    return static_cast<std::uint8_t>(reduced);
}

std::uint8_t ShootingSystem::computeRocketDamage(std::uint8_t weaponLevel) const
{
    const float multiplier = std::max(1.0f, _config.gameplay.rocketDamageMultiplier);
    const std::uint32_t level = std::max<std::uint32_t>(weaponLevel, 1u);
    const float referenceBase = static_cast<float>(_config.gameplay.weaponDamageBasic) * multiplier;
    const float configuredBase = (_config.gameplay.weaponDamageMissile > 0)
        ? static_cast<float>(_config.gameplay.weaponDamageMissile)
        : referenceBase;
    const float perLevelDamage = std::max(referenceBase, configuredBase);
    const float rawDamage = perLevelDamage * static_cast<float>(level);
    std::uint32_t damage = static_cast<std::uint32_t>(std::round(rawDamage));
    damage = std::clamp<std::uint32_t>(damage, 1u, 255u);
    return static_cast<std::uint8_t>(damage);
}

void ShootingSystem::shootProjectile(WeaponComponent &weapon, const EntityId entity, engine::Registry &registry)
{
    // auto *weapon = _registry.get<WeaponComponent>(entity);
    std::uint8_t damage = this->_config.gameplay.weaponDamageBasic;
    WeaponType weaponType = WeaponType::kWeaponBasicType;
    
    weaponType = weapon.weaponType;
    switch (weaponType)
    {
        case WeaponType::kWeaponBasicType: {
            const std::uint32_t level = std::max<std::uint32_t>(weapon.weaponLevel, 1u);
            const std::uint32_t scaled = static_cast<std::uint32_t>(this->_config.gameplay.weaponDamageBasic) * level;
            damage = static_cast<std::uint8_t>(std::clamp<std::uint32_t>(scaled, 1u, 255u));
            break;
        }
        case WeaponType::kWeaponLaserType:
            damage = computeLaserDamage(weapon.weaponLevel);
            break;
        case WeaponType::kWeaponRocketType:
            damage = computeRocketDamage(weapon.weaponLevel);
            break;
        default:
            weaponType = WeaponType::kWeaponBasicType;
            damage = this->_config.gameplay.weaponDamageBasic;
            break;
    }

    auto *playerTransform = registry.get<Transform>(entity);
    
    // Calculate bullet velocity first to determine spawn offset direction
    float bulletVx = 0.0f, bulletVy = 0.0f;
    _config.getDirectionVelocity(this->_config.gameplay.bulletDirection, bulletVx, bulletVy, this->_config.gameplay.bulletSpeed);
    
    // Adjust spawn offset based on bullet direction
    // If bullet goes left (negative vx), spawn it behind player (negative offset)
    // If bullet goes right (positive vx), spawn it in front of player (positive offset)
    float offsetX = this->_config.gameplay.bulletSpawnOffsetX;
    float offsetY = this->_config.gameplay.bulletSpawnOffsetY;
    
    if (bulletVx < 0.0f) {
        offsetX = -offsetX;  // Flip X offset for left-moving bullets
    }
    if (bulletVy < 0.0f) {
        offsetY = -offsetY;  // Flip Y offset for up-moving bullets
    }
    
    const float startX = playerTransform ? playerTransform->x + offsetX : 0.0f;
    const float startY = playerTransform ? playerTransform->y + offsetY : 0.0f;

    EntityFactory entityfactory(registry, _config);
    entityfactory.spawnBullet(entity, true, startX, startY, bulletVx, bulletVy, weaponType, damage);
}

void ShootingSystem::stopActiveLaser(WeaponComponent &weapon, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDestroySet)
{
    constexpr float kLaserReleaseFadeDuration = 0.18f;
    if (weapon.activeLaserId != 0) {
        bool scheduledFade = false;
        if (auto *projectile = registry.get<Projectile>(weapon.activeLaserId)) {
            projectile->persistent = false;
            projectile->damageTickTimer = 0.0f;
            float fadeLifetime = _config.gameplay.bulletLifetime - kLaserReleaseFadeDuration;
            if (fadeLifetime < 0.0f)
                fadeLifetime = 0.0f;
            projectile->lifetime = fadeLifetime;
            scheduledFade = true;
        }

        if (!scheduledFade || !_config.systems.projectileLifetimeSystem)
            toDestroySet.insert(weapon.activeLaserId);
    }
    weapon.activeLaserId = 0;
    weapon.laserActive = false;
}

void ShootingSystem::cycleEquippedWeapon(WeaponComponent &weapon)
{
    static constexpr std::array<WeaponType, 3> kWeaponOrder{ WeaponType::kWeaponBasicType, WeaponType::kWeaponLaserType, WeaponType::kWeaponRocketType };
    auto current = std::find(kWeaponOrder.begin(), kWeaponOrder.end(), weapon.weaponType);
    std::size_t startIndex = (current == kWeaponOrder.end()) ? 0u : static_cast<std::size_t>(current - kWeaponOrder.begin());

    for (std::size_t offset = 1; offset <= kWeaponOrder.size(); ++offset)
    {
        const std::size_t candidateIndex = (startIndex + offset) % kWeaponOrder.size();
        const WeaponType candidate = kWeaponOrder[candidateIndex];
        if (candidate == WeaponType::kWeaponLaserType && !weapon.laserUnlocked)
            continue;
        if (candidate == WeaponType::kWeaponRocketType && !weapon.rocketUnlocked)
            continue;

        weapon.weaponType = candidate;
        if (candidate != WeaponType::kWeaponLaserType)
            weapon.weaponLevel = 1;
        return;
    }
}



void ShootingSystem::update(float dt, int &currentLevel, engine::Registry &registry, std::unordered_set<rtype::EntityId> &toDestroySet)
{
    registry.view<PlayerInputComponent, WeaponComponent, FireCooldown>([&](
        EntityId entity,
        PlayerInputComponent &input,
        WeaponComponent &weapon,
        FireCooldown &cooldown) {
            if (input.swapWeapon)
            {
                stopActiveLaser(weapon, registry, toDestroySet);
                cycleEquippedWeapon(weapon);
            }
            if (!input.fire){
                if (weapon.laserActive)
                    stopActiveLaser(weapon, registry, toDestroySet);
                return;
            }

            if (weapon.weaponType == WeaponType::kWeaponRocketType) {
                cooldown.cooldownTime = _config.gameplay.rocketFireCooldown;
            } else {
                cooldown.cooldownTime = _config.gameplay.playerFireCooldown;
            }

            if (weapon.weaponType == WeaponType::kWeaponLaserType)
            {
                auto *player = registry.get<PlayerComponent>(entity);
                handleLaserFireInput(entity, weapon, *player, cooldown, registry, toDestroySet);
                return;
            }

            if (weapon.laserActive)
                stopActiveLaser(weapon, registry, toDestroySet);


            if (cooldown.timer <= 0.0f)
            {
                shootProjectile(weapon, entity, registry);
                cooldown.timer = cooldown.cooldownTime;
            }
        });
    registry.view<AutomaticShooting, WeaponComponent, FireCooldown>([&](
        EntityId entity,
        AutomaticShooting &shooting,
        WeaponComponent &weapon,
        FireCooldown &cooldown) {

            if (cooldown.timer <= 0.0f)
            {
                auto *monsterTransform = registry.get<Transform>(entity);
                EntityFactory entityfactory(registry, _config);
                
                // Adjust spawn offset based on shooting direction
                // Monsters shoot in the direction stored in shooting.dx/dy
                for (const auto &direction : shooting.shootingDirections) {
                    float offsetX = this->_config.gameplay.bulletSpawnOffsetX;
                    float offsetY = this->_config.gameplay.bulletSpawnOffsetY;
                    
                    if (direction.dx < 0.0f) {
                        offsetX = -offsetX;  // Flip X offset for left-shooting monsters
                    }
                    if (direction.dy < 0.0f) {
                        offsetY = -offsetY;  // Flip Y offset for up-shooting monsters
                    }
                    
                    const float startX = monsterTransform ? monsterTransform->x + offsetX : 0.0f;
                    const float startY = monsterTransform ? monsterTransform->y + offsetY : 0.0f;

                    entityfactory.spawnBullet(entity, false, startX, startY, direction.dx, direction.dy, WeaponType::kWeaponBasicType, 1);
                }
                cooldown.timer = cooldown.cooldownTime;
            }
        });
}
}