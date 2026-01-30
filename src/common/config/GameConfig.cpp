#include "rtype/common/GameConfig.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>

namespace rtype::config
{
namespace
{
std::string trim(const std::string& str)
{
    auto start = str.find_first_not_of(" \t\r\n");
    auto end = str.find_last_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";
    return str.substr(start, end - start + 1);
}

bool parseBool(const std::string& value)
{
    std::string lower = value;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower == "true" || lower == "1" || lower == "yes" || lower == "on";
}

Color parseColor(const std::string& value)
{
    Color color{200, 60, 60};
    std::istringstream ss(value);
    std::string r, g, b;
    
    if (std::getline(ss, r, ',') && std::getline(ss, g, ',') && std::getline(ss, b, ','))
    {
        color.r = static_cast<std::uint8_t>(std::stoi(trim(r)));
        color.g = static_cast<std::uint8_t>(std::stoi(trim(g)));
        color.b = static_cast<std::uint8_t>(std::stoi(trim(b)));
    }
    
    return color;
}

Team parseTeam(const std::string& value)
{
    std::string lower = value;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (lower == "player")
        return Team::Player;
    if (lower == "neutral")
        return Team::Neutral;
    if (lower == "monster" || lower == "enemy")
        return Team::Monster;

    // Fallback
    return Team::Monster;
}


ScrollDirection parseScrollDirection(const std::string& value)
{
    std::string lower = value;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower == "lefttoright" || lower == "left_to_right" || lower == "ltr" || lower == "left" || lower == "right")
        return ScrollDirection::LeftToRight;
    if (lower == "righttoleft" || lower == "right_to_left" || lower == "rtl")
        return ScrollDirection::RightToLeft;
    if (lower == "toptobottom" || lower == "top_to_bottom" || lower == "ttb" || lower == "down" || lower == "top" || lower == "bottom")
        return ScrollDirection::TopToBottom;
    if (lower == "bottomtotop" || lower == "bottom_to_top" || lower == "btt" || lower == "up")
        return ScrollDirection::BottomToTop;
    
    return ScrollDirection::LeftToRight;
}

PlayerDirection parsePlayerDirection(const std::string& value)
{
    std::string lower = value;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower == "lefttoright" || lower == "left_to_right" || lower == "ltr" || lower == "left" || lower == "right")
        return PlayerDirection::LeftToRight;
    if (lower == "righttoleft" || lower == "right_to_left" || lower == "rtl")
        return PlayerDirection::LeftToRight;
    if (lower == "toptobottom" || lower == "top_to_bottom" || lower == "ttb" || lower == "down" || lower == "top" || lower == "bottom")
        return PlayerDirection::TopToBottom;
    if (lower == "bottomtotop" || lower == "bottom_to_top" || lower == "btt" || lower == "up")
        return PlayerDirection::TopToBottom;
    if (lower == "all" || lower == "all" || lower == "all" || lower == "all")
        return PlayerDirection::All;
    
    return PlayerDirection::All;
}

std::vector<std::pair<float, float>> parsePositionList(const std::string& value)
{
    std::vector<std::pair<float, float>> positions;
    std::istringstream listStream(value);
    std::string pairStr;

    while (std::getline(listStream, pairStr, '/'))
    {
        std::istringstream pairStream(pairStr);
        std::string xStr, yStr;

        if (std::getline(pairStream, xStr, ',') &&
            std::getline(pairStream, yStr, ','))
        {
            std::cout << "adding posisition "<< xStr << yStr << std::endl;
            float x = std::stof(trim(xStr));
            float y = std::stof(trim(yStr));
            positions.emplace_back(x, y);
        }
    }

    return positions;
}

// Parse spawn/movement direction (supports additional keywords like MatchScroll, Opposite, Center, Static)
ScrollDirection parseSpawnDirection(const std::string& value, ScrollDirection scrollDir)
{
    std::string lower = value;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower == "matchscroll" || lower == "match" || lower == "scroll")
        return scrollDir;
    if (lower == "opposite" || lower == "reverse")
    {
        switch (scrollDir)
        {
        case ScrollDirection::LeftToRight: return ScrollDirection::RightToLeft;
        case ScrollDirection::RightToLeft: return ScrollDirection::LeftToRight;
        case ScrollDirection::TopToBottom: return ScrollDirection::BottomToTop;
        case ScrollDirection::BottomToTop: return ScrollDirection::TopToBottom;
        }
    }
    if (lower == "center" || lower == "static" || lower == "none")
        return scrollDir;

    return parseScrollDirection(value);
}

std::string scrollDirectionToString(ScrollDirection dir)
{
    switch (dir)
    {
    case ScrollDirection::LeftToRight: return "LeftToRight";
    case ScrollDirection::RightToLeft: return "RightToLeft";
    case ScrollDirection::TopToBottom: return "TopToBottom";
    case ScrollDirection::BottomToTop: return "BottomToTop";
    }
    return "LeftToRight";
}
}

bool GameConfig::loadFromFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "[config] Failed to open: " << path << '\n';
        return false;
    }
    
    std::string line;
    std::string currentSection;
    
    while (std::getline(file, line))
    {
        line = trim(line);

        if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;

        if (line[0] == '[' && line.back() == ']')
        {
            currentSection = line.substr(1, line.length() - 2);
            continue;
        }

        auto equalPos = line.find('=');
        if (equalPos == std::string::npos)
            continue;
        
        std::string key = trim(line.substr(0, equalPos));
        std::string value = line.substr(equalPos + 1);

        auto commentPos = value.find_first_of("#;");
        if (commentPos != std::string::npos)
            value = value.substr(0, commentPos);
        
        value = trim(value);

        if (currentSection == "Gameplay")
        {
            if (key == "ScrollDirection") gameplay.scrollDirection = parseScrollDirection(value);
            else if (key == "ScrollSpeed") gameplay.scrollSpeed = std::stof(value);
            else if (key == "PlayerSpeed") gameplay.playerSpeed = std::stof(value);
            else if (key == "PlayerStartHP") gameplay.playerStartHP = static_cast<std::uint8_t>(std::stoi(value));
            else if (key == "PlayerFireCooldown") gameplay.playerFireCooldown = std::stof(value);
            else if (key == "PlayerSpawnX") gameplay.playerSpawnX = std::stof(value);
            else if (key == "PlayerSpawnYBase") gameplay.playerSpawnYBase = std::stof(value);
            else if (key == "PlayerSpawnYSpacing") gameplay.playerSpawnYSpacing = std::stof(value);
            else if (key == "PlayerMovementDirection") gameplay.playerMovementDirection = parsePlayerDirection(value);
            else if (key == "FriendlyFire") gameplay.friendlyfire = parseBool(value);
            else if (key == "BulletSpeed") gameplay.bulletSpeed = std::stof(value);
            else if (key == "BulletLifetime") gameplay.bulletLifetime = std::stof(value);
            else if (key == "BulletSpawnOffsetX") gameplay.bulletSpawnOffsetX = std::stof(value);
            else if (key == "BulletSpawnOffsetY") gameplay.bulletSpawnOffsetY = std::stof(value);
            else if (key == "BulletDirection") gameplay.bulletDirection = parseScrollDirection(value);
            else if (key == "WeaponDamageBasic") gameplay.weaponDamageBasic = static_cast<std::uint8_t>(std::stoi(value));
            else if (key == "WeaponDamageLaser") gameplay.weaponDamageLaser = static_cast<std::uint8_t>(std::stoi(value));
            else if (key == "WeaponDamageMissile") gameplay.weaponDamageMissile = static_cast<std::uint8_t>(std::stoi(value));
            else if (key == "PowerUpsForLaser") {
                gameplay.powerUpsForLaser = std::stoi(value);
            }
            else if (key == "PowerUpsForRocket") {
                gameplay.powerUpsForRocket = std::stoi(value);
            }
            else if (key == "RocketFireCooldown") {
                gameplay.rocketFireCooldown = std::stof(value);
            }
            else if (key == "RocketDamageMultiplier") {
                gameplay.rocketDamageMultiplier = std::stof(value);
            }
            else if (key == "MonsterSpawnDelay") gameplay.monsterSpawnDelay = std::stof(value);
            else if (key == "MonsterHP") gameplay.monsterHP = static_cast<std::uint8_t>(std::stoi(value));
            else if (key == "MonsterSpawnSide") gameplay.monsterSpawnSide = parseSpawnDirection(value, gameplay.scrollDirection);
            else if (key == "MonsterMovement") gameplay.monsterMovement = parseSpawnDirection(value, gameplay.scrollDirection);
            else if (key.rfind("MonsterType", 0) == 0) addMonsterToConfig(key, value);
            else if (key == "PowerUpSpawnDelay") gameplay.powerUpSpawnDelay = std::stof(value);
            else if (key == "PowerUpsEnabled") gameplay.powerUpsEnabled = parseBool(value);
            else if (key == "PowerUpSpawnSide") gameplay.powerUpSpawnSide = parseSpawnDirection(value, gameplay.scrollDirection);
            else if (key == "PowerUpSpeedMultiplier") gameplay.powerUpSpeedMultiplier = std::stof(value);
            else if (key == "PowerUpSize") gameplay.powerUpSize = std::stof(value);
            else if (key == "PowerUpColor") gameplay.powerUpColor = parseColor(value);
            else if (key == "PowerUpOutlineColor") gameplay.powerUpOutlineColor = parseColor(value);
            else if (key == "PowerUpOutlineThickness") gameplay.powerUpOutlineThickness = std::stof(value);
            else if (key == "PowerUpsEnabled") gameplay.powerUpsEnabled = parseBool(value);
            else if (key == "PowerUpSpawnCenterX") gameplay.powerUpSpawnCenterX = std::stof(value);
            else if (key == "PowerUpSpawnCenterY") gameplay.powerUpSpawnCenterY = std::stof(value);
            else if (key == "PowerUpSpawnRandomRange") gameplay.powerUpSpawnRandomRange = std::stof(value);
            else if (key == "PowerUpSpawnMargin") gameplay.powerUpSpawnMargin = std::stof(value);
            else if (key == "PowerUpBoundaryMargin") gameplay.powerUpBoundaryMargin = std::stof(value);
            else if (key == "ShieldDuration") gameplay.shieldDuration = std::stoi(value);
            else if (key == "WeaponTypeMin") gameplay.weaponTypeMin = std::stoi(value);
            else if (key == "WeaponTypeMax") gameplay.weaponTypeMax = std::stoi(value);
            else if (key == "CollisionRadius") gameplay.collisionRadius = std::stof(value);
            else if (key == "WorldWidth") gameplay.worldWidth = std::stof(value);
            else if (key == "WorldHeight") gameplay.worldHeight = std::stof(value);
            else if (key == "NumberOfLevels") gameplay.numberOfLevels = std::stoi(value);
            else if (key == "MonsterPerLevel") gameplay.monsterPerLevel = std::stoi(value);
            else if (key == "BossMonsterType") gameplay.bossMonsterType = std::stoi(value);
            else if (key == "BossLevel") gameplay.bossLevel = std::stoi(value);
            else if (key == "Boss2MonsterType") gameplay.boss2MonsterType = std::stoi(value);
            else if (key == "Boss2Level") gameplay.boss2Level = std::stoi(value);
        }
        else if (currentSection == "Render")
        {
            if (key == "WindowWidth") render.windowWidth = std::stoul(value);
            else if (key == "WindowHeight") render.windowHeight = std::stoul(value);
            else if (key == "WindowTitle") render.windowTitle = value;
            else if (key == "TargetFPS") render.targetFPS = std::stoul(value);
            else if (key == "PlayerSize") gameRender.playerSize = std::stof(value);
            else if (key == "PlayerRotation") gameRender.playerRotation = std::stof(value);
            else if (key == "Player1Color") gameRender.player1Color = parseColor(value);
            else if (key == "Player2Color") gameRender.player2Color = parseColor(value);
            else if (key == "Player3Color") gameRender.player3Color = parseColor(value);
            else if (key == "Player4Color") gameRender.player4Color = parseColor(value);
            else if (key == "BulletSize") gameRender.bulletSize = std::stof(value);
            else if (key == "BulletColor") gameRender.bulletColor = parseColor(value);
            else if (key == "BackgroundColor") gameRender.backgroundColor = parseColor(value);
            else if (key == "StarCount") gameRender.starCount = std::stoul(value);
            else if (key == "StarSpeedMin") gameRender.starSpeedMin = std::stof(value);
            else if (key == "StarSpeedMax") gameRender.starSpeedMax = std::stof(value);
            else if (key == "StarSizeMin") gameRender.starSizeMin = std::stof(value);
            else if (key == "StarSizeMax") gameRender.starSizeMax = std::stof(value);
            else if (key == "TexturePack") render.texturePack = value;
        }
        else if (currentSection == "Network")
        {
            if (key == "DefaultPort") network.defaultPort = static_cast<std::uint16_t>(std::stoul(value));
            else if (key == "DefaultHost") network.defaultHost = value;
            else if (key == "MaxPlayers") network.maxPlayers = std::stoul(value);
            else if (key == "RxBufferSize") network.rxBufferSize = std::stoul(value);
            else if (key == "ServerTimeout" && std::stof(value) >= 1.0f) network.serverTimeout = std::stof(value);
            else if (key == "ClientTimeout" && std::stof(value) >= 1.0f) network.clientTimeout = std::stof(value);
        }
        else if (currentSection == "Audio")
        {
            if (key == "MasterVolume") audio.masterVolume = std::stof(value);
            else if (key == "SFXVolume") audio.sfxVolume = std::stof(value);
            else if (key == "Enabled") audio.enabled = parseBool(value);
        }
        else if (currentSection == "Font")
        {
            if (key == "FontFileName") font_config.font_filename = value;
        }
        else if (currentSection == "Systems")
        {
            if (key == "MovementSystem") systems.movementSystem = parseBool(value);
            else if (key == "FireCooldownSystem") systems.fireCooldownSystem = parseBool(value);
            else if (key == "ProjectileLifetimeSystem") systems.projectileLifetimeSystem = parseBool(value);
            else if (key == "CollisionSystem") systems.collisionSystem = parseBool(value);
            else if (key == "BoundarySystem") systems.boundarySystem = parseBool(value);
            else if (key == "CleanupSystem") systems.cleanupSystem = parseBool(value);
            else if (key == "MonsterSpawnerSystem") systems.monsterSpawnerSystem = parseBool(value);
            else if (key == "LevelSystem") systems.levelSystem = parseBool(value);
        }
        else if (currentSection == "SystemParameters")
        {
            if (key == "BoundaryMargin") systems.boundaryMargin = std::stof(value);
        }
        else if (currentSection == "Assets")
        {
            // Check if this is a monster sprite definition (MonsterTypeNSprites)
            if (key.rfind("MonsterType", 0) == 0 && key.find("Sprites") != std::string::npos)
            {
                // Extract monster type number from key (e.g., "MonsterType5Sprites" -> 5)
                std::string numStr = key.substr(11); // Skip "MonsterType"
                numStr = numStr.substr(0, numStr.find("Sprites"));
                try {
                    int typeIndex = std::stoi(numStr);
                    // If monster type exists, store sprite file with it
                    if (gameplay.MonstersType.find(typeIndex) != gameplay.MonstersType.end())
                    {
                        gameplay.MonstersType[typeIndex].spriteFile = value;
                    }
                    else
                    {
                        // Monster type not configured yet, store temporarily in assets
                        assets.textures[key] = value;
                    }
                } catch (...) {
                    assets.textures[key] = value;
                }
            }
            else
            {
                assets.textures[key] = value;
            }
        }
    }

    checkMonsterTypes();

    std::cout << "[config] Loaded configuration from: " << path << '\n';
    std::cout << "[config] Scroll direction: " << scrollDirectionToString(gameplay.scrollDirection) << '\n';
    return true;
}

void GameConfig::addMonsterToConfig(std::string key, std::string value)
{
    // Must start with "MonsterType"
    if (key.rfind("MonsterType", 0) != 0)
        return;

    // Parse monster index
    const size_t indexStart = 11; // strlen("MonsterType")
    size_t fieldPos = key.find_first_not_of("0123456789", indexStart);
    if (fieldPos == std::string::npos)
        return;

    int typeIndex = std::stoi(key.substr(indexStart, fieldPos - indexStart));
    // if (typeIndex < 0 || typeIndex >= gameplay.MonstersType.size())
    //     return;

    std::cout << "[config] Adding/updating monster type " << typeIndex << '\n';

    MonsterType& monsterType = gameplay.MonstersType[typeIndex];

    // Extract field name (THIS avoids substring collisions)
    std::string field = key.substr(fieldPos);

    if (field == "Size") {
        monsterType.size = std::stof(value);
    }
    else if (field == "CollisionSize") {
        monsterType.collisionSize = std::stof(value);
    }
    else if (field == "HP") {
        monsterType.HP = static_cast<std::uint8_t>(std::stoi(value));
        if (monsterType.hasShield) {
            monsterType.shieldHP = monsterType.HP / 2;
        }
    }
    else if (field == "Speed") {
        monsterType.speed = std::stof(value);
    }
    else if (field == "SpawnWeight") {
        monsterType.SpawnWeight = static_cast<std::uint8_t>(std::stoi(value));
    }
    else if (field == "Color") {
        monsterType.color = parseColor(value);
    }
    else if (field == "HasShield") {
        monsterType.hasShield = parseBool(value);
        if (monsterType.hasShield && monsterType.HP > 0) {
            monsterType.shieldHP = monsterType.HP / 2;
        }
    }
    else if (field == "DefaultPosition") {
        monsterType.defaultPositions = parsePositionList(value);
    } else if (field == "Team") {
        monsterType.team = parseTeam(value);
    } else if (field == "CanShoot") {
        monsterType.canShoot = parseBool(value);
    }
    else {return;
    }
}

void GameConfig::checkMonsterTypes()
{
    if (gameplay.MonstersType.empty())
    {
        std::cerr << "[config] No monster types defined in configuration!\n";
        return;
    }

    std::vector<int> invalidTypes;

    for (const auto& [typeIndex, monsterType] : gameplay.MonstersType)
    {
        // Allow SpawnWeight of 0 for boss types (they spawn via LevelSystem, not random rotation)
        bool isBossType = (typeIndex == gameplay.bossMonsterType || typeIndex == gameplay.boss2MonsterType);
        bool isInvalidSpawnWeight = (monsterType.SpawnWeight < 0);

        if (monsterType.size <= 0.0f || monsterType.HP == 0 ||
            (monsterType.color.r == 0 && monsterType.color.g == 0 && monsterType.color.b == 0) || monsterType.speed <= 0.0f)
        {
            std::cerr << "[config] Warning: Monster Type " << typeIndex << " has invalid parameter, erasing monster type" << typeIndex << '\n';
            invalidTypes.push_back(typeIndex);
        } else {
            std::cout << "[config] Monster Type " << typeIndex << ": Size=" << monsterType.size
                    //   << ", CollisionSize=" << monsterType.collisionSize
                      << ", HP=" << static_cast<int>(monsterType.HP)
                      << ", Speed=" << monsterType.speed
                      << ", SpawnWeight=" << static_cast<int>(monsterType.SpawnWeight)
                      << ", Color=(" << static_cast<int>(monsterType.color.r) << ","
                      << static_cast<int>(monsterType.color.g) << ","
                      << static_cast<int>(monsterType.color.b) << ")";
            if (!monsterType.spriteFile.empty())
            {
                std::cout << ", Sprites=" << monsterType.spriteFile;
            }
            std::cout << '\n';
        }
    }
    
    // Assign sprite files from assets that were parsed before monster configs
    for (auto &[typeIndex, monster] : gameplay.MonstersType)
    {
        if (monster.spriteFile.empty())
        {
            std::string spriteKey = "MonsterType" + std::to_string(typeIndex) + "Sprites";
            auto it = assets.textures.find(spriteKey);
            if (it != assets.textures.end())
            {
                monster.spriteFile = it->second;
                assets.textures.erase(it); // Remove from assets since it's now in monster config
                std::cout << "[config] Assigned sprite file " << monster.spriteFile 
                          << " to Monster Type " << typeIndex << '\n';
            }
        }
    }

    for (int typeIndex : invalidTypes) {
        gameplay.MonstersType.erase(typeIndex);
    }
}

bool GameConfig::saveToFile(const std::string& path) const
{
    std::ofstream file(path);
    if (!file.is_open())
    {
        std::cerr << "[config] Failed to create: " << path << '\n';
        return false;
    }
    
    file << "# R-Type Game Configuration\n";
    file << "# Generated automatically\n\n";
    
    file << "[Gameplay]\n";
    file << "ScrollDirection=" << scrollDirectionToString(gameplay.scrollDirection) << '\n';
    file << "ScrollSpeed=" << gameplay.scrollSpeed << '\n';
    file << "PlayerSpeed=" << gameplay.playerSpeed << '\n';
    file << "PlayerStartHP=" << static_cast<int>(gameplay.playerStartHP) << '\n';
    file << "PlayerFireCooldown=" << gameplay.playerFireCooldown << '\n';
    file << "PlayerSpawnX=" << gameplay.playerSpawnX << '\n';
    file << "PlayerSpawnYBase=" << gameplay.playerSpawnYBase << '\n';
    file << "PlayerSpawnYSpacing=" << gameplay.playerSpawnYSpacing << '\n';
    file << "FriendlyFire=" << gameplay.friendlyfire << '\n';
    file << "BulletSpeed=" << gameplay.bulletSpeed << '\n';
    file << "BulletLifetime=" << gameplay.bulletLifetime << '\n';
    file << "BulletSpawnOffsetX=" << gameplay.bulletSpawnOffsetX << '\n';
    file << "BulletSpawnOffsetY=" << gameplay.bulletSpawnOffsetY << '\n';
    file << "BulletDirection=" << scrollDirectionToString(gameplay.bulletDirection) << '\n';
    file << "WeaponDamageBasic=" << static_cast<int>(gameplay.weaponDamageBasic) << '\n';
    file << "WeaponDamageLaser=" << static_cast<int>(gameplay.weaponDamageLaser) << '\n';
    file << "WeaponDamageMissile=" << static_cast<int>(gameplay.weaponDamageMissile) << '\n';
    file << "PowerUpsForLaser=" << static_cast<int>(gameplay.powerUpsForLaser) << '\n';
    file << "PowerUpsForRocket=" << static_cast<int>(gameplay.powerUpsForRocket) << '\n';
    file << "RocketFireCooldown=" << gameplay.rocketFireCooldown << '\n';
    file << "RocketDamageMultiplier=" << gameplay.rocketDamageMultiplier << '\n';
    file << "MonsterSpawnDelay=" << gameplay.monsterSpawnDelay << '\n';
    file << "MonsterHP=" << static_cast<int>(gameplay.monsterHP) << '\n';
    file << "MonsterSpawnSide=" << scrollDirectionToString(gameplay.monsterSpawnSide) << '\n';
    file << "MonsterMovement=" << scrollDirectionToString(gameplay.monsterMovement) << '\n';
    if (!gameplay.MonstersType.empty()) {
        for (const auto& [typeIndex, monsterType] : gameplay.MonstersType) {
            file << "MonsterType" << typeIndex << "Size=" << monsterType.size << '\n';
            file << "MonsterType" << typeIndex << "CollisionSize=" << monsterType.collisionSize << '\n';
            file << "MonsterType" << typeIndex << "HP=" << static_cast<int>(monsterType.HP) << '\n';
            file << "MonsterType" << typeIndex << "Speed=" << monsterType.speed << '\n';
            file << "MonsterType" << typeIndex << "SpawnWeight=" << static_cast<int>(monsterType.SpawnWeight) << '\n';
            file << "MonsterType" << typeIndex << "Color=" << static_cast<int>(monsterType.color.r) << ","
                 << static_cast<int>(monsterType.color.g) << ","
                 << static_cast<int>(monsterType.color.b) << '\n';
        }
    }
    file << "PowerUpSpawnDelay=" << gameplay.powerUpSpawnDelay << '\n';
    file << "PowerUpsEnabled=" << (gameplay.powerUpsEnabled ? "true" : "false") << '\n';
    file << "PowerUpSpawnSide=" << scrollDirectionToString(gameplay.powerUpSpawnSide) << '\n';
    file << "PowerUpSpeedMultiplier=" << gameplay.powerUpSpeedMultiplier << '\n';
    file << "PowerUpSize=" << gameplay.powerUpSize << '\n';
    file << "PowerUpColor=" << static_cast<int>(gameplay.powerUpColor.r) << ","
         << static_cast<int>(gameplay.powerUpColor.g) << ","
         << static_cast<int>(gameplay.powerUpColor.b) << '\n';
    file << "PowerUpOutlineColor=" << static_cast<int>(gameplay.powerUpOutlineColor.r) << ","
         << static_cast<int>(gameplay.powerUpOutlineColor.g) << ","
         << static_cast<int>(gameplay.powerUpOutlineColor.b) << '\n';
    file << "PowerUpOutlineThickness=" << gameplay.powerUpOutlineThickness << '\n';
    file << "PowerUpSpawnCenterX=" << gameplay.powerUpSpawnCenterX << '\n';
    file << "PowerUpSpawnCenterY=" << gameplay.powerUpSpawnCenterY << '\n';
    file << "PowerUpSpawnRandomRange=" << gameplay.powerUpSpawnRandomRange << '\n';
    file << "PowerUpSpawnMargin=" << gameplay.powerUpSpawnMargin << '\n';
    file << "PowerUpBoundaryMargin=" << gameplay.powerUpBoundaryMargin << '\n';
    file << "WeaponTypeMin=" << gameplay.weaponTypeMin << '\n';
    file << "WeaponTypeMax=" << gameplay.weaponTypeMax << '\n';
    file << "CollisionRadius=" << gameplay.collisionRadius << '\n';
    file << "WorldWidth=" << gameplay.worldWidth << '\n';
    file << "WorldHeight=" << gameplay.worldHeight << '\n';
    file << "NumberOfLevels=" << gameplay.numberOfLevels << '\n';
    file << "MonsterPerLevel=" << gameplay.monsterPerLevel << '\n';
    file << '\n';
    
    file << "[Render]\n";
    file << "WindowWidth=" << render.windowWidth << '\n';
    file << "WindowHeight=" << render.windowHeight << '\n';
    file << "WindowTitle=" << render.windowTitle << '\n';
    file << "TargetFPS=" << render.targetFPS << '\n';
    file << "PlayerSize=" << gameRender.playerSize << '\n';
    file << "PlayerRotation=" << gameRender.playerRotation << '\n';
    file << "Player1Color=" << static_cast<int>(gameRender.player1Color.r) << ","
         << static_cast<int>(gameRender.player1Color.g) << ","
         << static_cast<int>(gameRender.player1Color.b) << '\n';
    file << "Player2Color=" << static_cast<int>(gameRender.player2Color.r) << ","
         << static_cast<int>(gameRender.player2Color.g) << ","
         << static_cast<int>(gameRender.player2Color.b) << '\n';
    file << "Player3Color=" << static_cast<int>(gameRender.player3Color.r) << ","
         << static_cast<int>(gameRender.player3Color.g) << ","
         << static_cast<int>(gameRender.player3Color.b) << '\n';
    file << "Player4Color=" << static_cast<int>(gameRender.player4Color.r) << ","
         << static_cast<int>(gameRender.player4Color.g) << ","
         << static_cast<int>(gameRender.player4Color.b) << '\n';
    file << "BulletSize=" << gameRender.bulletSize << '\n';
    file << "BulletColor=" << static_cast<int>(gameRender.bulletColor.r) << ","
         << static_cast<int>(gameRender.bulletColor.g) << ","
         << static_cast<int>(gameRender.bulletColor.b) << '\n';
    file << "BackgroundColor=" << static_cast<int>(gameRender.backgroundColor.r) << ","
         << static_cast<int>(gameRender.backgroundColor.g) << ","
         << static_cast<int>(gameRender.backgroundColor.b) << '\n';
    file << "StarCount=" << gameRender.starCount << '\n';
    file << "StarSpeedMin=" << gameRender.starSpeedMin << '\n';
    file << "StarSpeedMax=" << gameRender.starSpeedMax << '\n';
    file << "StarSizeMin=" << gameRender.starSizeMin << '\n';
    file << "StarSizeMax=" << gameRender.starSizeMax << '\n';
    file << "TexturePack=" << render.texturePack << '\n';
    file << '\n';
    
    file << "[Network]\n";
    file << "DefaultPort=" << network.defaultPort << '\n';
    file << "DefaultHost=" << network.defaultHost << '\n';
    file << "MaxPlayers=" << network.maxPlayers << '\n';
    file << "RxBufferSize=" << network.rxBufferSize << '\n';
    file << "ServerTimeout=" << network.serverTimeout << '\n';
    file << "ClientTimeout=" << network.clientTimeout << '\n';
    file << '\n';
    
    file << "[Audio]\n";
    file << "MasterVolume=" << audio.masterVolume << '\n';
    file << "SFXVolume=" << audio.sfxVolume << '\n';
    file << "Enabled=" << (audio.enabled ? "true" : "false") << '\n';
    
    std::cout << "[config] Saved configuration to: " << path << '\n';
    return true;
}

GameConfig GameConfig::getDefault()
{
    return GameConfig{};
}

void GameConfig::getScrollVelocity(float& vx, float& vy) const
{
    vx = 0.0f;
    vy = 0.0f;
    
    switch (gameplay.scrollDirection)
    {
    case ScrollDirection::LeftToRight:
        vx = -gameplay.scrollSpeed;
        break;
    case ScrollDirection::RightToLeft:
        vx = gameplay.scrollSpeed;
        break;
    case ScrollDirection::TopToBottom:
        vy = -gameplay.scrollSpeed;
        break;
    case ScrollDirection::BottomToTop:
        vy = gameplay.scrollSpeed;
        break;
    }
}

void GameConfig::getDirectionVelocity(ScrollDirection dir, float& vx, float& vy, float speed) const
{
    vx = 0.0f;
    vy = 0.0f;
    
    switch (dir)
    {
    case ScrollDirection::LeftToRight:
        vx = speed;
        break;
    case ScrollDirection::RightToLeft:
        vx = -speed;
        break;
    case ScrollDirection::TopToBottom:
        vy = speed;
        break;
    case ScrollDirection::BottomToTop:
        vy = -speed;
        break;
    }
}

void GameConfig::getSpawnPosition(float& x, float& y, float randomValue) const
{
    switch (gameplay.monsterSpawnSide)
    {
    case ScrollDirection::RightToLeft:
        x = gameplay.worldWidth + 40.0f;
        y = 40.0f + randomValue * (gameplay.worldHeight - 80.0f);
        break;
    case ScrollDirection::LeftToRight:
        x = -40.0f;
        y = 40.0f + randomValue * (gameplay.worldHeight - 80.0f);
        break;
    case ScrollDirection::BottomToTop:
        x = 40.0f + randomValue * (gameplay.worldWidth - 80.0f);
        y = gameplay.worldHeight + 40.0f;
        break;
    case ScrollDirection::TopToBottom:
        x = 40.0f + randomValue * (gameplay.worldWidth - 80.0f);
        y = -40.0f;
        break;
    }
}

bool GameConfig::isOffScreen(float x, float y) const
{
    constexpr float margin = 50.0f;
    
    switch (gameplay.scrollDirection)
    {
    case ScrollDirection::LeftToRight:
        return x < -margin;
    case ScrollDirection::RightToLeft:
        return x > gameplay.worldWidth + margin;
    case ScrollDirection::TopToBottom:
        return y < -margin;
    case ScrollDirection::BottomToTop:
        return y > gameplay.worldHeight + margin;
    }
    
    return false;
}
}
