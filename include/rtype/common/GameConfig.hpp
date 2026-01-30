#pragma once

#include "rtype/common/Types.hpp"
#include <string>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include "rtype/common/Components.hpp"
namespace rtype::config
{
enum class ScrollDirection
{
    LeftToRight,
    RightToLeft,
    TopToBottom,
    BottomToTop
};

struct Color { std::uint8_t r, g, b; };

enum class PlayerDirection
{
    LeftToRight,
    TopToBottom,
    All,
};

struct MonsterType
{
    float size;
    float collisionSize {1.0};
    std::uint8_t HP;
    float speed;
    std::uint8_t SpawnWeight;
    Color color;
    std::string spriteFile;  // JSON file containing sprite data
    bool hasShield{false};   // Whether this monster spawns with a shield
    std::uint8_t shieldHP{0}; // Shield HP (calculated as HP / 2 when hasShield is true)
    Team team{Team::Monster};

    bool canShoot{true};
    std::vector<std::pair<float, float>> defaultPositions {};
};

struct GameplayConfig
{
    ScrollDirection scrollDirection{ScrollDirection::LeftToRight};
    float scrollSpeed{90.0f};

    float playerSpeed{220.0f};
    std::uint8_t playerStartHP{3};
    float playerFireCooldown{0.25f};
    float playerSpawnX{80.0f};
    float playerSpawnYBase{120.0f};
    float playerSpawnYSpacing{80.0f};
    PlayerDirection playerMovementDirection{PlayerDirection::All};
    bool friendlyfire{false};

    float bulletSpeed{380.0f};
    float bulletLifetime{3.0f};
    float bulletSpawnOffsetX{30.0f};
    float bulletSpawnOffsetY{0.0f};
    ScrollDirection bulletDirection{ScrollDirection::LeftToRight};

    std::uint8_t weaponDamageBasic{1};
    std::uint8_t weaponDamageLaser{2};
    std::uint8_t weaponDamageMissile{3};
    std::uint8_t powerUpsForLaser{5};
    std::uint8_t powerUpsForRocket{10};
    float rocketFireCooldown{0.6f};
    float rocketDamageMultiplier{3.0f};

    float monsterSpawnDelay{2.0f};
    std::uint8_t monsterHP{1};
    ScrollDirection monsterSpawnSide{ScrollDirection::LeftToRight};
    ScrollDirection monsterMovement{ScrollDirection::LeftToRight};

    std::unordered_map<int, MonsterType> MonstersType;

    float powerUpSpawnDelay{10.0f};
    bool powerUpsEnabled{true};
    ScrollDirection powerUpSpawnSide{ScrollDirection::LeftToRight};
    float powerUpSpeedMultiplier{0.25f};
    float powerUpSize{8.0f};
    Color powerUpColor{100, 240, 140};
    Color powerUpOutlineColor{255, 255, 255};
    float powerUpOutlineThickness{2.0f};

    float powerUpSpawnCenterX{0.6f};
    float powerUpSpawnCenterY{0.5f};
    float powerUpSpawnRandomRange{120.0f};
    float powerUpSpawnMargin{80.0f};
    float powerUpBoundaryMargin{200.0f};
    int shieldDuration{5};

    int weaponTypeMin{0};
    int weaponTypeMax{2};
    
    float collisionRadius{20.0f};

    float worldWidth{1280.0f};
    float worldHeight{720.0f};

    int numberOfLevels{2};
    int monsterPerLevel{10};
    int bossMonsterType{6};
    int bossLevel{5};
    int boss2MonsterType{7};
    int boss2Level{15};
};

struct RenderConfig
{
    std::uint32_t windowWidth{1280};
    std::uint32_t windowHeight{720};
    std::string windowTitle{"R-Type"};
    std::uint32_t targetFPS{60};

    std::string texturePack{"default"};
};

struct GameRenderConfig
{
    float playerSize{20.0f};
    float playerRotation{0.0f};

    float bulletSize{4.0f};

    std::uint32_t starCount{100};
    float starSpeedMin{30.0f};
    float starSpeedMax{90.0f};
    float starSizeMin{1.0f};
    float starSizeMax{2.6f};

    Color backgroundColor{6, 10, 26};
    Color player1Color{95, 205, 228};
    Color player2Color{255, 174, 79};
    Color player3Color{140, 122, 230};
    Color player4Color{255, 99, 146};
    Color bulletColor{255, 207, 64};
};

struct NetworkConfig
{
    std::uint16_t defaultPort{5000};
    std::string defaultHost{"127.0.0.1"};
    std::size_t maxPlayers{4};
    std::size_t rxBufferSize{1024};
    float serverTimeout{5.0f};
    float clientTimeout{10.0f};
};

struct AudioConfig
{
    float masterVolume{100.0f};
    float sfxVolume{35.0f};
    bool enabled{true};
};

struct AssetConfig {
    std::unordered_map<std::string, std::string> textures;
};

struct FontConfig
{
    std::string font_filename;
};

struct SystemsConfig
{
    bool movementSystem{true};
    bool fireCooldownSystem{true};
    bool projectileLifetimeSystem{true};
    bool collisionSystem{true};
    bool boundarySystem{true};
    bool cleanupSystem{true};
    bool monsterSpawnerSystem{true};
    bool levelSystem{false};
    float boundaryMargin{100.0f};
};

class GameConfig
{
public:
    GameplayConfig gameplay;
    RenderConfig render;
    GameRenderConfig gameRender;
    NetworkConfig network;
    AudioConfig audio;
    FontConfig font_config;
    SystemsConfig systems;
    AssetConfig assets;

    std::unordered_map<std::string, float> floats;
    std::unordered_map<std::string, int> ints;
    std::unordered_map<std::string, bool> bools;
    std::unordered_map<std::string, std::string> strings;

    bool loadFromFile(const std::string& path);

    bool saveToFile(const std::string& path) const;

    static GameConfig getDefault();

    void getScrollVelocity(float& vx, float& vy) const;

    void getDirectionVelocity(ScrollDirection dir, float& vx, float& vy, float speed) const;

    void getSpawnPosition(float& x, float& y, float randomValue) const;

    bool isOffScreen(float x, float y) const;

private:
    void addMonsterToConfig(std::string key, std::string value);
    void checkMonsterTypes();
};
}
