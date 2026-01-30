/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** IRender - Rendering abstraction layer
*/

#pragma once

#include "rtype/common/Types.hpp"
#include "rtype/client/RemoteDisplay.hpp"
#include "rtype/common/Protocol.hpp"
#include <cstdint>
#include <string>
#include <memory>

// Forward declaration
namespace rtype::config {
    struct GameConfig;
}

namespace rtype::client
{

/**
 * @brief Abstract color representation
 */
struct Color
{
    std::uint8_t r{0};
    std::uint8_t g{0};
    std::uint8_t b{0};
    std::uint8_t a{255};
    
    Color() = default;
    Color(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}
};

/**
 * @brief Abstract vector for positions
 */
struct Vector2
{
    float x{0.0f};
    float y{0.0f};
    
    Vector2() = default;
    Vector2(float px, float py) : x(px), y(py) {}
};

/**
 * @brief Abstract rendering interface
 * 
 * Hides graphics library implementation details
 */
class IRender
{
public:
    virtual ~IRender() = default;
    
    /**
     * @brief Check if render window is still open
     */
    virtual bool isOpen() const = 0;
    
    /**
     * @brief Poll events and check if window should close
     * @return true if should continue running
     */
    virtual bool pollEvents() = 0;
    
    /**
     * @brief Clear the screen with a color
     */
    virtual void clear(const Color &color = Color(0, 0, 0)) = 0;
    
    /**
     * @brief Display rendered content
     */
    virtual void display() = 0;
    
    /**
     * @brief Get elapsed time since last frame in seconds
     */
    virtual float getDeltaTime() = 0;
    
    /**
     * @brief Draw a rectangle
     */
    virtual void drawRectangle(const Vector2 &position, const Vector2 &size, const Color &color) = 0;
    
    /**
     * @brief Draw a circle
     */
    virtual void drawCircle(const Vector2 &position, float radius, const Color &color) = 0;
    
    /**
     * @brief Draw text
     */
    virtual void drawText(const std::string &text, const Vector2 &position, 
                         std::uint32_t size, const Color &color) = 0;
    
    /**
     * @brief Get the bounding box size of text (width, height)
     */
    virtual Vector2 getTextBounds(const std::string &text, std::uint32_t size) = 0;
    
    /**
     * @brief Play a sound effect
     */
    virtual void playSound(const std::string &soundId) = 0;
    
    /**
     * @brief Get window width
     */
    virtual float getWidth() const = 0;
    
    /**
     * @brief Get window height
     */
    virtual float getHeight() const = 0;
    
    /**
     * @brief High-level: Render complete game state
     */
    virtual void render(const RemoteDisplay &display) = 0;

    /**
     * @brief Get player input from renderer
     */
    virtual net::PlayerInput getPlayerInput() = 0;
    
    /**
     * @brief Check if a specific key is pressed
     */
    virtual bool isKeyPressed(int key) = 0;
    
    /**
     * @brief Get text input from user (for UI)
     */
    virtual std::string getTextInput() = 0;
    
    /**
     * @brief Check if mouse button was clicked
     */
    virtual bool wasMouseClicked() const = 0;
    
    /**
     * @brief Get mouse position
     */
    virtual Vector2 getMousePosition() const = 0;
};

/**
 * @brief Factory for creating renderers
 */
class RenderFactory
{
public:
    /**
     * @brief Create a renderer instance
     * @param width Window width
     * @param height Window height
     * @param title Window title
     * @param config Game configuration
     * @return Unique pointer to renderer
     */
    static std::unique_ptr<IRender> createRenderer(
        std::uint32_t width, 
        std::uint32_t height, 
        const std::string &title,
        const config::GameConfig &config
    );
};

} // namespace rtype::client
