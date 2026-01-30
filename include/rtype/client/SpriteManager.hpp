/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** SpriteManager - Manages sprite loading and asset management
*/

#pragma once

#include "rtype/common/GameConfig.hpp"
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>
#include <filesystem>
#include <vector>

namespace rtype::client
{

/**
 * @brief Manages sprite loading, storage, and retrieval
 * 
 * Handles JSON-based sprite configuration, asset path resolution,
 * and sprite animation state management.
 */
class SpriteManager
{
public:
    struct SpriteResource
    {
        sf::Texture texture;
        sf::IntRect rect{0, 0, 0, 0};
        std::vector<sf::IntRect> frames;
        bool animate{false};
        float frameDuration{0.0f};
        float elapsed{0.0f};
        std::size_t currentFrame{0};
        std::string defaultDirection{"left"};  // Default sprite direction: "left", "right", "up", "down"
        bool canRotate{true};  // Whether this sprite can be rotated/flipped based on direction
    };

    explicit SpriteManager(const config::GameConfig &config);
    
    /**
     * @brief Load all sprite assets from configuration
     */
    void loadSpriteAssets();
    
    /**
     * @brief Get a sprite resource by key
     * @param key The sprite identifier
     * @return Pointer to sprite resource or nullptr if not found
     */
    SpriteResource* getSprite(const std::string &key);
    
    /**
     * @brief Get the current animated frame for a sprite
     * @param resource The sprite resource to animate
     * @param deltaTime Time since last frame
     * @return The current frame rectangle
     */
    sf::IntRect getAnimatedFrame(SpriteResource &resource, float deltaTime);
    
    /**
     * @brief Get the sprite key for a specific monster type
     * @param monsterType The monster type index
     * @return The sprite key or empty string if not found
     */
    std::string getMonsterSpriteKey(int monsterType) const;
    
    /**
     * @brief Get the total number of loaded sprites
     */
    std::size_t getSpriteCount() const { return _sprites.size(); }

private:
    void loadSprite(const std::string &key, const std::string &relativePath,
                    sf::IntRect rect = {}, std::vector<sf::IntRect> frames = {},
                    float frameDurationSeconds = 0.0f, bool animateFrames = false);
    
    void loadSpriteFromJson(const std::string &jsonPath, bool useUniquePrefix = false);
    
    std::filesystem::path resolveAssetPath(const std::string &relativePath) const;
    
    std::string getFileSeparator() const;

    const config::GameConfig &_config;
    std::unordered_map<std::string, SpriteResource> _sprites;
    std::unordered_map<int, std::string> _monsterTypeToSpriteKey;
    std::string _fileSeparator;
};

} // namespace rtype::client
