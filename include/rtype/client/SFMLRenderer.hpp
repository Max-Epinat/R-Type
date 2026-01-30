/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** SFMLRenderer - SFML implementation of rendering abstraction
*/

#pragma once

#include "rtype/client/IRender.hpp"
#include "rtype/client/RemoteDisplay.hpp"
#include "rtype/client/SpriteManager.hpp"
#include "rtype/common/GameConfig.hpp"
#include "rtype/common/Protocol.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <filesystem>

namespace rtype::client
{

/**
 * @brief SFML implementation of the rendering interface
 */
class SFMLRenderer : public IRender
{
public:
    SFMLRenderer(std::uint32_t width, std::uint32_t height, const std::string &title, const config::GameConfig &config);
    ~SFMLRenderer() override = default;
    
    bool isOpen() const override;
    bool pollEvents() override;
    void clear(const Color &color) override;
    void display() override;
    float getDeltaTime() override;
    void drawRectangle(const Vector2 &position, const Vector2 &size, const Color &color) override;
    void drawCircle(const Vector2 &position, float radius, const Color &color) override;
    void drawText(const std::string &text, const Vector2 &position, 
                 std::uint32_t size, const Color &color) override;
    Vector2 getTextBounds(const std::string &text, std::uint32_t size) override;
    void playSound(const std::string &soundId) override;
    float getWidth() const override;
    float getHeight() const override;
    void render(const RemoteDisplay &display) override;
    net::PlayerInput getPlayerInput() override;
    
    bool isKeyPressed(int key) override;
    std::string getTextInput() override;
    bool wasMouseClicked() const override;
    Vector2 getMousePosition() const override;
    
private:
    sf::Color toSFMLColor(const Color &color) const;
    Color colorForPlayer(PlayerId id) const;
    sf::Vector2f toSFVector(const Vector2f &vec) const;
    void buildStarfield();
    void drawStarfield(float dt);
    std::string isWindowsOrLinuxPath() const;
    void setUpFont();
    void applyGameView();    // Set view for game world (fixed size, scaled (change to match window aspect later))
    void applyMenuView();    // Set view for menus (matches window)
    void spawnExplosion(const Vector2f &position);
    void updateExplosions(float dt);
    void drawExplosions();
    void updateProjectileEffects(const RemoteDisplay &display, float dt);
    void updateMonsterEffects(const RemoteDisplay &display);
    void drawLaserBeamAt(const sf::Vector2f &position, float intensity = 1.0f);
    void pruneMissingPlayerVisuals(const std::unordered_map<PlayerId, RemotePlayer> &players);
    void drawPlayers(const std::unordered_map<PlayerId, RemotePlayer> &players);
    void drawMonsters(const std::unordered_map<EntityId, RemoteMonster> &monsters);
    void drawShields(const std::unordered_map<EntityId, RemoteShield> &shields);
    void drawPowerUps(const std::unordered_map<EntityId, RemotePowerUp> &powerUps);
    void drawBullets(const std::unordered_map<EntityId, RemoteBullet> &bullets);
    void playShootSound();
    
    // Helper to calculate sprite rotation and flip based on default direction and target direction
    struct SpriteTransform {
        float rotation{0.0f};
        bool flipX{false};
        bool flipY{false};
    };
    SpriteTransform calculateSpriteTransform(const std::string &defaultDir, float vx, float vy) const;
    
    const config::GameConfig &_config;
    SpriteManager _spriteManager;
    sf::RenderWindow _window;
    sf::Clock _clock;
    float _lastDeltaTime{0.0f};
    float _deltaSeconds{0.0f};
    sf::Font _font;
    std::string _fileSeparator;
    
    // Game view (fixed design resolution)
    sf::View _gameView;
    sf::View _menuView;
    float _gameWidth;
    float _gameHeight;
    
    // Sound system
    sf::SoundBuffer _playerShootBuffer;
    sf::SoundBuffer _enemyShootBuffer;
    sf::SoundBuffer _playerHitBuffer;
    sf::SoundBuffer _explosionBuffer;
    sf::SoundBuffer _kamikazeExplosionBuffer;
    sf::SoundBuffer _powerUpBuffer;
    
    sf::Sound _playerShootSound;
    sf::Sound _enemyShootSound;
    sf::Sound _playerHitSound;
    sf::Sound _explosionSound;
    sf::Sound _kamikazeExplosionSound;
    sf::Sound _powerUpSound;
    
    sf::Music _backgroundMusic;
    sf::Music _bossMusic;
    
    bool _isBossLevel{false};
    int _lastLevel{1};
    std::unordered_map<EntityId, bool> _previousMonsters;
    std::unordered_map<PlayerId, uint8_t> _previousPlayerHealth;
    std::unordered_set<EntityId> _previousPowerUps;
    
    void initializeSoundSystem();
    void generateSoundEffect(sf::SoundBuffer& buffer, float frequency, float duration, float amplitude = 3000.0f);
    void playBackgroundMusic();
    void playBossMusic();
    void stopAllMusic();
    
    // Starfield
    struct Star {
        sf::Vector2f pos;
        float speed;
        float size;
        sf::Color color;
    };
    std::vector<Star> _stars;
    
    // Particle system for effects
    struct Particle {
        sf::Vector2f pos;
        sf::Vector2f velocity;
        float lifetime;
        sf::Color color;
    };
    std::vector<Particle> _particles;
    
    // UI input handling
    std::string _textInputBuffer;
    bool _mouseClicked{false};
    sf::Vector2i _mousePosition;

    // Explosion animation
    struct ExplosionInstance {
        sf::Vector2f position;
        float frameTimer{0.0f};
        std::size_t currentFrame{0};
        bool finished{false};
    };
    std::vector<ExplosionInstance> _explosions;

    struct PlayerVisualState
    {
        sf::Vector2f lastPosition{};
        sf::Vector2f velocity{};
        float tilt{0.0f};
        bool initialized{false};
        bool facingBackward{false};
    };

    sf::IntRect pickPlayerFrame(const SpriteManager::SpriteResource &resource, PlayerVisualState &visualState);

    std::unordered_map<PlayerId, PlayerVisualState> _playerVisuals;
    bool _swapPressedLastFrame{false};
    struct LaserFadeInstance {
        sf::Vector2f position{};
        float remaining{0.0f};
    };
    std::unordered_map<EntityId, LaserFadeInstance> _laserFades;
    std::unordered_map<EntityId, RemoteBullet> _lastBullets;
    std::unordered_map<EntityId, RemoteMonster> _lastMonsters;
    static constexpr float _laserFadeDuration{0.2f};
    
    // Per-monster animation state
    struct MonsterAnimState {
        float elapsed{0.0f};
        std::size_t currentFrame{0};
    };
    std::unordered_map<EntityId, MonsterAnimState> _monsterAnimStates;
    std::unordered_map<EntityId, MonsterAnimState> _shieldAnimStates;
    
    // Boss 2 visibility blinking
    float _boss2BlinkTimer{0.0f};
    static constexpr float _boss2VisibleDuration{4.0f};   // Time visible on screen
    static constexpr float _boss2InvisibleDuration{2.0f}; // Time invisible (teleporting)
};

// Factory implementation
inline std::unique_ptr<IRender> RenderFactory::createRenderer(
    std::uint32_t width, 
    std::uint32_t height, 
    const std::string &title,
    const config::GameConfig &config)
{
    return std::make_unique<SFMLRenderer>(width, height, title, config);
}

} // namespace rtype::client
