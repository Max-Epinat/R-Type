/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** SpriteManager - Implementation
*/

#include "rtype/client/SpriteManager.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <array>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace rtype::client
{

SpriteManager::SpriteManager(const config::GameConfig &config)
    : _config(config)
    , _fileSeparator(getFileSeparator())
{
}

std::string SpriteManager::getFileSeparator() const
{
    #ifdef _WIN32
        return "\\";
    #else
        return "/";
    #endif
}

std::filesystem::path SpriteManager::resolveAssetPath(const std::string &relativePath) const
{
    const std::array<std::string, 10> searchRoots = {
        "src" + _fileSeparator + "assets",
        ".." + _fileSeparator + "src" + _fileSeparator + "assets",
        "../.." + _fileSeparator + "src" + _fileSeparator + "assets",
        "assets",
        ".." + _fileSeparator + "assets",
        "../.." + _fileSeparator + "assets",
        "bin" + _fileSeparator + "assets",
        ".." + _fileSeparator + "bin" + _fileSeparator + "assets",
        "../.." + _fileSeparator + "bin" + _fileSeparator + "assets",
        "../../.." + _fileSeparator + "bin" + _fileSeparator + "assets"
    };

    // First try with texture pack path
    const std::string texturePackPath = "textures" + _fileSeparator + _config.render.texturePack + _fileSeparator + relativePath;
    for (const auto &root : searchRoots)
    {
        const std::string candidate = root + _fileSeparator + texturePackPath;
        if (fs::exists(candidate))
            return fs::path(candidate);
    }

    // Fallback to direct path (for non-texture assets like fonts)
    for (const auto &root : searchRoots)
    {
        const std::string candidate = root + _fileSeparator + relativePath;
        if (fs::exists(candidate))
            return fs::path(candidate);
    }
    return fs::path(relativePath);
}

void SpriteManager::loadSprite(
    const std::string &key,
    const std::string &relativePath,
    sf::IntRect rect,
    std::vector<sf::IntRect> frames,
    float frameDurationSeconds,
    bool animateFrames)
{
    const auto assetPath = resolveAssetPath(relativePath);
    if (!fs::exists(assetPath))
    {
        std::cerr << "[sprite-manager] missing sprite asset '" << relativePath << "'\n";
        return;
    }

    SpriteResource resource{};
    if (!resource.texture.loadFromFile(assetPath.string()))
    {
        std::cerr << "[sprite-manager] failed to load sprite '" << key << "' from " << assetPath << "\n";
        return;
    }

    resource.texture.setSmooth(true);
    const auto texSize = resource.texture.getSize();
    if (rect.width <= 0 || rect.height <= 0)
    {
        rect.left = 0;
        rect.top = 0;
        rect.width = static_cast<int>(texSize.x);
        rect.height = static_cast<int>(texSize.y);
    }
    resource.rect = rect;

    if (frames.empty())
        resource.frames = {resource.rect};
    else
    {
        resource.frames = std::move(frames);
        resource.rect = resource.frames.front();
    }

    const bool hasAnimationFrames = animateFrames && resource.frames.size() > 1 && frameDurationSeconds > 0.0f;
    resource.animate = hasAnimationFrames;
    resource.frameDuration = hasAnimationFrames ? frameDurationSeconds : 0.0f;
    resource.elapsed = 0.0f;
    resource.currentFrame = 0;
    resource.defaultDirection = "left";  // Default value, will be overridden by JSON if specified
    _sprites[key] = std::move(resource);
    std::cout << "[sprite-manager] loaded sprite '" << key << "' from " << assetPath << "\n";
}

void SpriteManager::loadSpriteFromJson(const std::string &jsonPath, bool useUniquePrefix)
{
    const auto jsonFullPath = resolveAssetPath(jsonPath);
    if (!fs::exists(jsonFullPath))
    {
        std::cerr << "[sprite-manager] JSON file not found: " << jsonPath << "\n";
        return;
    }

    std::ifstream file(jsonFullPath);
    if (!file.is_open())
    {
        std::cerr << "[sprite-manager] Failed to open JSON file: " << jsonFullPath << "\n";
        return;
    }

    try
    {
        json data = json::parse(file);
        
        if (!data.contains("texture") || !data.contains("sprites"))
        {
            std::cerr << "[sprite-manager] Invalid JSON format in: " << jsonPath << "\n";
            return;
        }

        const std::string texturePath = data["texture"].get<std::string>();
        const auto &sprites = data["sprites"];
        
        // Create unique prefix from JSON filename (only used if useUniquePrefix is true)
        std::string jsonPrefix;
        if (useUniquePrefix) {
            jsonPrefix = jsonPath;
            size_t lastSlash = jsonPrefix.find_last_of("/\\");
            if (lastSlash != std::string::npos) {
                jsonPrefix = jsonPrefix.substr(lastSlash + 1);
            }
            size_t dotPos = jsonPrefix.find('.');
            if (dotPos != std::string::npos) {
                jsonPrefix = jsonPrefix.substr(0, dotPos);
            }
        }

        for (auto it = sprites.begin(); it != sprites.end(); ++it)
        {
            const std::string &spriteKey = it.key();
            const auto &spriteData = it.value();

            if (!spriteData.contains("frames") || spriteData["frames"].empty())
            {
                std::cerr << "[sprite-manager] No frames defined for sprite '" << spriteKey << "'\n";
                continue;
            }

            std::vector<sf::IntRect> frames;
            for (const auto &frame : spriteData["frames"])
            {
                const int x = frame["x"].get<int>();
                const int y = frame["y"].get<int>();
                const int width = frame["width"].get<int>();
                const int height = frame["height"].get<int>();
                frames.emplace_back(x, y, width, height);
            }

            const bool animate = spriteData.value("animate", false);
            const float frameDuration = spriteData.value("frameDuration", 0.0f);
            const std::string direction = spriteData.value("defaultDirection", "left");
            const bool canRotate = spriteData.value("canRotate", true);

            if (!frames.empty())
            {
                // Create unique key only if requested (for monster sprites)
                const std::string finalKey = useUniquePrefix ? (jsonPrefix + "_" + spriteKey) : spriteKey;
                loadSprite(finalKey, texturePath, frames.front(), frames, frameDuration, animate);
                
                // Set the direction and rotation settings after loading
                if (_sprites.find(finalKey) != _sprites.end())
                {
                    _sprites[finalKey].defaultDirection = direction;
                    _sprites[finalKey].canRotate = canRotate;
                }
                
                std::cout << "[sprite-manager] Loaded sprite '" << finalKey << "' (dir: " << direction 
                         << ", rotate: " << (canRotate ? "yes" : "no") << ") from JSON: " << jsonPath << "\n";
            }
        }
    }
    catch (const json::exception &e)
    {
        std::cerr << "[sprite-manager] JSON parsing error in " << jsonPath << ": " << e.what() << "\n";
    }
}

void SpriteManager::loadSpriteAssets()
{
    std::cout << "[sprite-manager] Loading sprite assets from configuration...\n";
    
    // Load core game sprites from config (non-monster sprites)
    for (const auto &[key, jsonFile] : _config.assets.textures)
    {
        if (!jsonFile.empty())
        {
            std::cout << "[sprite-manager] Loading " << key << " from " << jsonFile << "\n";
            loadSpriteFromJson(jsonFile);
        }
    }
    
    // Track which monster sprite files we've already loaded (to avoid loading same file twice)
    std::unordered_set<std::string> loadedMonsterSprites;
    
    // Load monster sprites and build mappings
    for (const auto &[typeIndex, monster] : _config.gameplay.MonstersType)
    {
        if (!monster.spriteFile.empty())
        {
            // Load the sprite file if we haven't loaded it yet
            if (loadedMonsterSprites.find(monster.spriteFile) == loadedMonsterSprites.end())
            {
                std::cout << "[sprite-manager] Loading monster sprites from " << monster.spriteFile << "\n";
                loadSpriteFromJson(monster.spriteFile, true);  // Use unique prefix for monsters
                loadedMonsterSprites.insert(monster.spriteFile);
            }
            
            // Build unique sprite key based on JSON filename
            std::string jsonPrefix = monster.spriteFile;
            size_t lastSlash = jsonPrefix.find_last_of("/\\");
            if (lastSlash != std::string::npos) {
                jsonPrefix = jsonPrefix.substr(lastSlash + 1);
            }
            size_t dotPos = jsonPrefix.find('.');
            if (dotPos != std::string::npos) {
                jsonPrefix = jsonPrefix.substr(0, dotPos);
            }
            
            // Find the sprite key that was loaded from this monster's JSON file
            const auto jsonFullPath = resolveAssetPath(monster.spriteFile);
            if (std::filesystem::exists(jsonFullPath))
            {
                try {
                    std::ifstream file(jsonFullPath);
                    if (file.is_open())
                    {
                        json data = json::parse(file);
                        if (data.contains("sprites"))
                        {
                            // Get the first sprite key from this JSON and prefix it
                            for (const auto& item : data["sprites"].items())
                            {
                                const std::string uniqueKey = jsonPrefix + "_" + item.key();
                                _monsterTypeToSpriteKey[typeIndex] = uniqueKey;
                                std::cout << "[sprite-manager] Monster type " << typeIndex 
                                          << " mapped to sprite '" << uniqueKey << "' from " 
                                          << monster.spriteFile << "\n";
                                break;
                            }
                        }
                    }
                } catch (...) {
                    std::cerr << "[sprite-manager] Failed to map monster type " << typeIndex << "\n";
                }
            }
        }
        else
        {
            std::cout << "[sprite-manager] No sprite file configured for monster type " << typeIndex 
                      << ", will use fallback rendering\n";
        }
    }
    
    std::cout << "[sprite-manager] Sprite loading complete. Loaded " << _sprites.size() << " sprite resources.\n";
}

SpriteManager::SpriteResource* SpriteManager::getSprite(const std::string &key)
{
    const auto it = _sprites.find(key);
    if (it == _sprites.end())
        return nullptr;
    return &it->second;
}

sf::IntRect SpriteManager::getAnimatedFrame(SpriteResource &resource, float deltaTime)
{
    if (!resource.animate || resource.frames.empty())
        return resource.rect;

    if (resource.frameDuration <= 0.0f)
        return resource.frames[resource.currentFrame];

    resource.elapsed += deltaTime;
    while (resource.elapsed >= resource.frameDuration)
    {
        resource.elapsed -= resource.frameDuration;
        resource.currentFrame = (resource.currentFrame + 1) % resource.frames.size();
    }
    return resource.frames[resource.currentFrame];
}

std::string SpriteManager::getMonsterSpriteKey(int monsterType) const
{
    auto it = _monsterTypeToSpriteKey.find(monsterType);
    if (it != _monsterTypeToSpriteKey.end())
        return it->second;
    return "";
}

} // namespace rtype::client
