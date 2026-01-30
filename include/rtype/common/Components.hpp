#pragma once

#include <chrono>
#include "Types.hpp"
#include <optional>
#include <vector>
namespace rtype
{
struct Transform
{
    float x{0.0f};
    float y{0.0f};
};

struct Velocity
{
    float vx{0.0f};
    float vy{0.0f};
};

struct Health
{
    std::uint8_t hp{3};
    bool alive{true};
};

struct PlayerPowerUpStatus
{
    PlayerPowerUpType type{PlayerPowerUpType::Nothing};
    std::chrono::steady_clock::time_point start_time{std::chrono::steady_clock::now()};
};

struct PlayerComponent
{
    PlayerId id{};
};

struct MonsterComponent
{
    std::uint8_t type{0};
};

enum class WeaponType {
    kWeaponBasicType = 0,
    kWeaponLaserType = 1,
    kWeaponRocketType = 2,
};

struct Projectile
{
    EntityId owner{};
    bool fromPlayer{true};
    float lifetime{0.0f};
    std::uint8_t damage{1};
    WeaponType weaponType = WeaponType::kWeaponBasicType;
    bool persistent{false};
    float damageTickTimer{0.0f};
};

struct PersistentLaser {
    float damageTickTimer{0.0f};
    float damageInterval{0.08f};
};

struct WeaponComponent
{
    WeaponType weaponType = WeaponType::kWeaponBasicType;
    std::uint8_t weaponLevel{1};
    bool laserActive{false};
    EntityId activeLaserId{0};
    bool laserUnlocked{false};
    bool rocketUnlocked{false};
    std::uint16_t powerUpsCollected{0};
};

struct PowerUp
{
    std::uint8_t type{0};
    std::uint8_t value{1};
};

struct FireCooldown
{
    float timer{0.0f};
    float cooldownTime{0.25f};
};

struct AutomaticShooting
{
    std::vector<Direction> shootingDirections;
};

enum class Team
{
    Player,
    Monster,
    Neutral,
};

struct TeamComponent
{
    Team team{Team::Player};
};

struct Hurtbox {
    std::optional<EntityId> collidedWith{};
};

struct Hitbox
{
    bool destroyOnHit = true;
};

struct Collider {
    float radius{10.0f};
};
struct BeamCollider
{
    float length;
    float halfHeight;
};

struct PlayerInputComponent
{
    bool left  = false;
    bool right = false;
    bool up    = false;
    bool down  = false;
    bool fire  = false;
    bool swapWeapon = false;
};



struct Boss2Behavior
{
    float oscillationTimer{0.0f};
    float oscillationSpeed{2.0f};        // How fast it oscillates
    float oscillationAmplitude{100.0f};  // How far it moves up/down
    float baseY{0.0f};                   // Original Y position
    float visibilityTimer{0.0f};
    float visibleDuration{4.0f};         // How long visible on screen
    float invisibleDuration{2.0f};       // How long invisible (teleporting)
    bool visible{true};
};

struct ShieldComponent
{
    EntityId parentMonster{0}; // The monster this shield protects
    float offsetX{0.0f};       // Offset from parent position
    float offsetY{0.0f};
};

}

