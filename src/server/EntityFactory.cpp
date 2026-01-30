/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** EntityFactory implementation
*/

#include "rtype/server/EntityFactory.hpp"
#include <iostream>
#include <vector>
#include <cmath>

namespace rtype::server
{

void EntityFactory::addTransformAndVelocity(EntityId entity, float x, float y, float vx, float vy)
{
    _registry.addComponent<Transform>(entity, x, y);
    _registry.addComponent<Velocity>(entity, vx, vy);
}

EntityId EntityFactory::spawnPlayer(PlayerId id, float x, float y)
{
    auto entity = _registry.createEntity();
    
    addTransformAndVelocity(entity, x, y, 0.0f, 0.0f);
    _registry.addComponent<PlayerComponent>(entity, id);
    _registry.addComponent<Health>(entity, _config.gameplay.playerStartHP, true);
    _registry.addComponent<FireCooldown>(entity, 0.0f);
    _registry.addComponent<WeaponComponent>(entity);
    _registry.addComponent<PlayerPowerUpStatus>(entity);

    // Player collision radius based on visual size
    const float playerRadius = _config.gameRender.playerSize * 0.5f;
    _registry.addComponent<Collider>(entity, playerRadius);
    _registry.addComponent<Hurtbox>(entity);
    if (!_config.gameplay.friendlyfire) {
        _registry.addComponent<TeamComponent>(entity, Team::Player);
    }
    return entity;
}


EntityId EntityFactory::spawnMonster(std::uint8_t type, bool canShoot, Team team, float x, float y, float vx, float vy)
{
    auto entity = _registry.createEntity();
    
    std::uint8_t hp = _config.gameplay.monsterHP;
    float size = 24.0f;
    bool hasShield = false;
    float collisionSize = 1.0;
    auto it = _config.gameplay.MonstersType.find(type);
    if (it != _config.gameplay.MonstersType.end()) {
        hp = it->second.HP;
        size = it->second.size;
        collisionSize = it->second.collisionSize;
        hasShield = it->second.hasShield;
    }
    
    addTransformAndVelocity(entity, x, y, vx, vy);
    _registry.addComponent<MonsterComponent>(entity, type);
    _registry.addComponent<Health>(entity, hp, true);
    
    const float monsterRadius = size * 0.5f * collisionSize;
    _registry.addComponent<Collider>(entity, monsterRadius);

    _registry.addComponent<WeaponComponent>(entity);
    constexpr std::uint8_t KAMIKAZE_TYPE = 5;
    if (type == KAMIKAZE_TYPE) {
        _registry.addComponent<Hitbox>(entity);
        std::cout << "spawning kamikaze" << std::endl;
    }
    if (canShoot)
        _registry.addComponent<FireCooldown>(entity, 0.0f, 2.0f);
    _registry.addComponent<Hurtbox>(entity);
    const float startX = x - this->_config.gameplay.bulletSpawnOffsetX;
    const float startY = y - this->_config.gameplay.bulletSpawnOffsetY ;

    float bulletVx = 0.0f, bulletVy = 0.0f;
    _config.getDirectionVelocity(this->_config.gameplay.bulletDirection, bulletVx, bulletVy, this->_config.gameplay.bulletSpeed);
    if (type == 6) {
        float fx = bulletVx;
        float fy = bulletVy;

        float lx = -fy;
        float ly = -fx;

        float speed = this->_config.gameplay.bulletSpeed;

        Direction back{-fx, -fy};

        Direction up{lx, ly};

        Direction diag{-fx + lx, -fy + ly};

        auto normalize = [speed](Direction &d){
            float len = std::sqrt(d.dx*d.dx + d.dy*d.dy);
            if (len > 0.0f) {
                d.dx = (d.dx / len) * speed;
                d.dy = (d.dy / len) * speed;
            }
        };

        normalize(back);
        normalize(up);
        normalize(diag);

        _registry.addComponent<AutomaticShooting>(entity, std::vector<Direction>{back, diag, up});
        _registry.addComponent<TeamComponent>(entity, team);
    } else {
        _registry.addComponent<AutomaticShooting>(entity, std::vector<Direction>({Direction{-bulletVx, -bulletVy}}));
        _registry.addComponent<TeamComponent>(entity, team);
    }

    // Spawn shield if configured
    if (hasShield) {
        // Position shield in front of monster (offset based on movement direction)
        float shieldOffsetX = size * 0.6f;  // Place shield 60% of monster size in front
        if (vx < 0) shieldOffsetX = -shieldOffsetX;  // Flip if moving left
        
        float shieldX = x + shieldOffsetX;
        float shieldY = y;
        
        spawnShield(entity, type, shieldX, shieldY, vx, vy);
        std::cout << "[EntityFactory] Spawned shield for monster type " << static_cast<int>(type) << std::endl;
    }
    
    return entity;
}

EntityId EntityFactory::spawnShield(EntityId parentMonster, std::uint8_t type, float x, float y, float vx, float vy)
{
    auto entity = _registry.createEntity();
    
    // Get monster configuration
    std::uint8_t shieldHP = 1;
    float size = 24.0f;
    config::Color color = {200, 200, 200};  // Default gray color
    
    auto it = _config.gameplay.MonstersType.find(type);
    if (it != _config.gameplay.MonstersType.end()) {
        shieldHP = it->second.shieldHP;
        size = it->second.size;
        color = it->second.color;
    }
    
    addTransformAndVelocity(entity, x, y, vx, vy);
    _registry.addComponent<Health>(entity, shieldHP, true);
    _registry.addComponent<ShieldComponent>(entity, parentMonster, 0.0f, 0.0f);
    
    // Shield collision radius (slightly smaller than monster to position in front)
    const float shieldRadius = size * 0.4f;
    _registry.addComponent<Collider>(entity, shieldRadius);
    _registry.addComponent<Hurtbox>(entity);
    auto *parentTeam = _registry.getComponent<TeamComponent>(parentMonster);
    if (parentTeam)
        _registry.addComponent<TeamComponent>(entity, parentTeam->team);
    return entity;
}

EntityId EntityFactory::spawnBullet(EntityId owner, bool fromPlayer, float x, float y, float vx, float vy,
                                    WeaponType weaponType, std::uint8_t damage)
{
    auto entity = _registry.createEntity();
    
    addTransformAndVelocity(entity, x, y, vx, vy);
    Projectile projectile{};
    projectile.owner = owner;
    projectile.fromPlayer = fromPlayer;
    projectile.lifetime = 0.0f;
    projectile.damage = damage;
    projectile.weaponType = weaponType;
    projectile.persistent = (weaponType == WeaponType::kWeaponLaserType);
    projectile.damageTickTimer = 0.0f;
    _registry.addComponent<Projectile>(entity, projectile);
    
    auto *ownerTeam = _registry.getComponent<TeamComponent>(owner);
    if (ownerTeam) {
        TeamComponent bulletTeam = *ownerTeam;
        _registry.addComponent<TeamComponent>(entity, bulletTeam.team);
    }
    // Bullet collision radius based on visual size
    if (weaponType == WeaponType::kWeaponLaserType) {
        float length = _config.gameplay.worldWidth + _config.systems.boundaryMargin;
        float beamHalfHeight = _config.gameRender.bulletSize * 1.5f;
        _registry.addComponent<BeamCollider>(entity, length, beamHalfHeight);
        _registry.addComponent<Hitbox>(entity, false);
    } else {
        float bulletRadius = _config.gameRender.bulletSize;
        if (weaponType == WeaponType::kWeaponRocketType)
            bulletRadius *= 2.0f;
        _registry.addComponent<Collider>(entity, bulletRadius);
        _registry.addComponent<Hitbox>(entity, true);
    }
    
    return entity;
}

EntityId EntityFactory::spawnPowerUp(std::uint8_t type, float x, float y, float vx, float vy)
{
    auto entity = _registry.createEntity();
    
    PowerUp powerUp{};
    powerUp.type = type;
    powerUp.value = 0;
    
    addTransformAndVelocity(entity, x, y, vx, vy);
    _registry.addComponent<PowerUp>(entity, powerUp);
    
    // PowerUp collision radius
    const float powerUpRadius = _config.gameplay.powerUpSize;
    _registry.addComponent<Collider>(entity, powerUpRadius);
    
    return entity;
}

} // namespace rtype::server
