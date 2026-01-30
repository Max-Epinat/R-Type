/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** SFMLRenderer - SFML implementation
*/

#include "rtype/client/SFMLRenderer.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <cmath>
#include <random>
#include <array>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <unordered_set>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace rtype::client
{

SFMLRenderer::SFMLRenderer(std::uint32_t width, std::uint32_t height, const std::string &title, const config::GameConfig &config)
    : _config(config)
    , _spriteManager(config)
    , _window(sf::VideoMode(width, height), title)
    , _gameWidth(static_cast<float>(width))
    , _gameHeight(static_cast<float>(height))
{
    std::cout << "[renderer] Creating SFML window: " << width << "x" << height << " - \"" << title << "\"\n";

    _fileSeparator = isWindowsOrLinuxPath();
    if (_fileSeparator == "\\")
        std::cout << "[renderer] Detected Windows file path format\n";
    else
        std::cout << "[renderer] Detected Linux/Unix file path format\n";
    
    if (!_window.isOpen()) {
        std::cerr << "[renderer] ERROR: Window failed to open!\n";
        throw std::runtime_error("Failed to create SFML window");
    }
    
    std::cout << "[renderer] Window opened successfully\n";
    
    _window.setVisible(true);
    _window.requestFocus();
    _window.setVerticalSyncEnabled(true);
    _window.setFramerateLimit(_config.render.targetFPS);

    _gameView.setSize(_gameWidth, _gameHeight);
    _gameView.setCenter(_gameWidth / 2.f, _gameHeight / 2.f);

    _menuView.setSize(static_cast<float>(width), static_cast<float>(height));
    _menuView.setCenter(static_cast<float>(width) / 2.f, static_cast<float>(height) / 2.f);
    
    std::cout << "[renderer] Window should now be visible on screen\n";

    setUpFont();

    // Initialize sound system
    if (_config.audio.enabled) {
        initializeSoundSystem();
    }

    
    buildStarfield();
    _spriteManager.loadSpriteAssets();
}

SFMLRenderer::SpriteTransform SFMLRenderer::calculateSpriteTransform(const std::string &defaultDir, float vx, float vy) const
{
    SpriteTransform transform;
    
    // Determine target direction based on velocity
    std::string targetDir = "left";  // Default
    float absVx = std::abs(vx);
    float absVy = std::abs(vy);
    
    if (absVx > absVy) {
        // Horizontal movement dominates
        targetDir = (vx > 0.0f) ? "right" : "left";
    } else if (absVy > 0.01f) {
        // Vertical movement dominates
        targetDir = (vy > 0.0f) ? "down" : "up";
    }
    
    // Calculate rotation and flip based on default -> target transformation
    if (defaultDir == "left") {
        if (targetDir == "right") {
            transform.flipX = true;
        } else if (targetDir == "down") {
            transform.rotation = -90.0f;  // Rotate counterclockwise to point down
        } else if (targetDir == "up") {
            transform.rotation = 90.0f;  // Rotate clockwise to point up
        }
    } else if (defaultDir == "right") {
        if (targetDir == "left") {
            transform.flipX = true;
        } else if (targetDir == "down") {
            transform.rotation = 90.0f;  // Rotate clockwise to point down
        } else if (targetDir == "up") {
            transform.rotation = -90.0f;  // Rotate counterclockwise to point up
        }
    } else if (defaultDir == "down") {
        if (targetDir == "up") {
            transform.flipY = true;
        } else if (targetDir == "left") {
            transform.rotation = 90.0f;  // Rotate clockwise to point left
        } else if (targetDir == "right") {
            transform.rotation = -90.0f;  // Rotate counterclockwise to point right
        }
    } else if (defaultDir == "up") {
        if (targetDir == "down") {
            transform.flipY = true;
        } else if (targetDir == "left") {
            transform.rotation = -90.0f;  // Rotate counterclockwise to point left
        } else if (targetDir == "right") {
            transform.rotation = 90.0f;  // Rotate clockwise to point right
        }
    }
    
    return transform;
}

void SFMLRenderer::buildStarfield()
{
    std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<float> posX(0.0f, static_cast<float>(_config.render.windowWidth));
    std::uniform_real_distribution<float> posY(0.0f, static_cast<float>(_config.render.windowHeight));
    std::uniform_real_distribution<float> speed(_config.gameRender.starSpeedMin, _config.gameRender.starSpeedMax);
    std::uniform_real_distribution<float> size(_config.gameRender.starSizeMin, _config.gameRender.starSizeMax);
    
    _stars.clear();
    for (std::size_t i = 0; i < _config.gameRender.starCount; ++i)
    {
        Star star{};
        star.pos = {posX(rng), posY(rng)};
        star.speed = speed(rng);
        star.size = size(rng);
        const sf::Uint8 shade = static_cast<sf::Uint8>(200 + (i % 55));
        star.color = sf::Color(shade, shade, 255);
        _stars.push_back(star);
    }
}

void SFMLRenderer::drawStarfield(float dt)
{
    const auto &bgColor = _config.gameRender.backgroundColor;
    _window.clear(sf::Color(bgColor.r, bgColor.g, bgColor.b));
    
    float scrollVx = 0.0f, scrollVy = 0.0f;
    _config.getScrollVelocity(scrollVx, scrollVy);
    
    for (auto &star : _stars)
    {
        star.pos.x -= scrollVx * star.speed / _config.gameplay.scrollSpeed * dt;
        star.pos.y -= scrollVy * star.speed / _config.gameplay.scrollSpeed * dt;
        
        if (_config.gameplay.scrollDirection == config::ScrollDirection::LeftToRight ||
            _config.gameplay.scrollDirection == config::ScrollDirection::RightToLeft)
        {
            if (star.pos.x < -2.0f)
                star.pos.x = static_cast<float>(_config.render.windowWidth) + 2.0f;
            else if (star.pos.x > static_cast<float>(_config.render.windowWidth) + 2.0f)
                star.pos.x = -2.0f;
        }
        else
        {
            if (star.pos.y < -2.0f)
                star.pos.y = static_cast<float>(_config.render.windowHeight) + 2.0f;
            else if (star.pos.y > static_cast<float>(_config.render.windowHeight) + 2.0f)
                star.pos.y = -2.0f;
        }
        
        sf::RectangleShape sprite({star.size, star.size});
        sprite.setPosition(star.pos);
        sprite.setFillColor(star.color);
        _window.draw(sprite);
    }
}

std::string SFMLRenderer::isWindowsOrLinuxPath() const
{
    #ifdef _WIN32
        return "\\";
    #else
        return "/";
    #endif
}

void SFMLRenderer::setUpFont()
{
    std::string fontpath = ".." + _fileSeparator + "src" + _fileSeparator + "assets" + _fileSeparator + "font" + _fileSeparator + _config.font_config.font_filename;
    std::cout << "[renderer] Trying font path: " << fontpath << '\n';
    if (!_font.loadFromFile(fontpath)) {
        std::cerr << "[renderer] Failed to load font, trying alternatives\n";

        fontpath = "src" + _fileSeparator + "assets" + _fileSeparator + "font" + _fileSeparator + _config.font_config.font_filename;
        std::cout << "[renderer] Trying font path: " << fontpath << '\n';
        if (!_font.loadFromFile(fontpath)) {
            std::cerr << "[renderer] Failed to load font from: " << fontpath << '\n';

            fontpath = "../.." + _fileSeparator + "src" + _fileSeparator + "assets" + _fileSeparator + "font" + _fileSeparator + _config.font_config.font_filename;
            std::cout << "[renderer] Trying font path: " << fontpath << '\n';
            if (!_font.loadFromFile(fontpath)) {
                std::cerr << "[renderer] Failed to load font from: " << fontpath << '\n';

                #ifdef _WIN32
                fontpath = "C:\\Windows\\Fonts\\arial.ttf";
                std::cout << "[renderer] Trying system font: " << fontpath << '\n';
                #else
                fontpath = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
                std::cout << "[renderer] Trying system font: " << fontpath << '\n';
                #endif
                
                if (!_font.loadFromFile(fontpath))
                    std::cerr << "[renderer] Warning: No font loaded\n";
                else
                    std::cout << "[renderer] Successfully loaded system font\n";
            } else {
                std::cout << "[renderer] Successfully loaded font\n";
            }
        } else {
            std::cout << "[renderer] Successfully loaded font\n";
        }
    } else {
        std::cout << "[renderer] Successfully loaded font\n";
    }
    return;
}

bool SFMLRenderer::isOpen() const
{
    return _window.isOpen();
}

bool SFMLRenderer::pollEvents()
{
    _mouseClicked = false;
    
    sf::Event event{};
    while (_window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            _window.close();
            return false;
        }
        else if (event.type == sf::Event::Resized)
        {
            // Both views keep their original size - viewport scaling handles the display
            // This ensures all content (game and menus) scale properly to any window size
            _menuView.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));
            _gameView.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));

            _window.setView(_menuView);
        }
        else if (event.type == sf::Event::TextEntered)
        {
            if (event.text.unicode < 128)
            {
                char c = static_cast<char>(event.text.unicode);
                if (c == 8)
                {
                    _textInputBuffer += '\b';
                }
                else if (c == 13)
                {
                    _textInputBuffer += '\r';
                }
                else if (c >= 32 && c < 127)
                {
                    _textInputBuffer += c;
                }
            }
        }
        else if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::Delete)
            {
                _textInputBuffer += '\b';
            }
        }
        else if (event.type == sf::Event::MouseButtonPressed)
        {
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                _mouseClicked = true;
                sf::Vector2f worldPos = _window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                _mousePosition = sf::Vector2i(static_cast<int>(worldPos.x), static_cast<int>(worldPos.y));
            }
        }
    }
    return true;
}

void SFMLRenderer::clear(const Color &color)
{
    _window.setView(_menuView);
    _window.clear(toSFMLColor(color));
}

void SFMLRenderer::display()
{
    _window.display();
}

float SFMLRenderer::getDeltaTime()
{
    return _lastDeltaTime;
}

void SFMLRenderer::drawRectangle(const Vector2 &position, const Vector2 &size, const Color &color)
{
    sf::RectangleShape rect({size.x, size.y});
    rect.setPosition(position.x, position.y);
    rect.setFillColor(toSFMLColor(color));
    _window.draw(rect);
}

void SFMLRenderer::drawCircle(const Vector2 &position, float radius, const Color &color)
{
    sf::CircleShape circle(radius);
    circle.setPosition(position.x, position.y);
    circle.setFillColor(toSFMLColor(color));
    _window.draw(circle);
}

void SFMLRenderer::drawText(const std::string &text, const Vector2 &position, 
                           std::uint32_t size, const Color &color)
{
    sf::Text sfText;
    sfText.setFont(_font);
    sfText.setString(text);
    sfText.setCharacterSize(size);
    sfText.setFillColor(toSFMLColor(color));
    sfText.setPosition(position.x, position.y);
    _window.draw(sfText);
}

Vector2 SFMLRenderer::getTextBounds(const std::string &text, std::uint32_t size)
{
    sf::Text sfText;
    sfText.setFont(_font);
    sfText.setString(text);
    sfText.setCharacterSize(size);
    sf::FloatRect bounds = sfText.getLocalBounds();
    return Vector2(bounds.width, bounds.height);
}

void SFMLRenderer::playSound(const std::string &soundId)
{
    if (!_config.audio.enabled)
        return;
    
    if (soundId == "shoot" || soundId == "player_shoot") {
        playShootSound();
    } else if (soundId == "enemy_shoot") {
        if (_enemyShootSound.getStatus() != sf::Sound::Playing)
            _enemyShootSound.play();
    } else if (soundId == "player_hit" || soundId == "damage") {
        std::cout << "[sound] Playing player hit sound\n";
        if (_playerHitSound.getStatus() != sf::Sound::Playing)
            _playerHitSound.play();
    } else if (soundId == "explosion") {
        std::cout << "[sound] Playing explosion sound\n";
        if (_explosionSound.getStatus() != sf::Sound::Playing)
            _explosionSound.play();
    } else if (soundId == "kamikaze_explosion") {
        std::cout << "[sound] Playing kamikaze explosion sound\n";
        _kamikazeExplosionSound.stop(); // Stop previous to play immediately
        _kamikazeExplosionSound.play();
    } else if (soundId == "powerup" || soundId == "power_up") {
        std::cout << "[sound] Playing powerup sound\n";
        _powerUpSound.stop(); // Stop previous to play immediately
        _powerUpSound.play();
    } else if (soundId == "boss_music") {
        playBossMusic();
    } else if (soundId == "background_music") {
        playBackgroundMusic();
    } else if (soundId == "stop_music") {
        stopAllMusic();
    }
}

void SFMLRenderer::playShootSound()
{
    if (!_config.audio.enabled)
        return;
    if (_playerShootBuffer.getSampleCount() == 0)
        return;
    if (_playerShootSound.getStatus() != sf::Sound::Playing)
        _playerShootSound.play();
}

float SFMLRenderer::getWidth() const
{
    return _window.getView().getSize().x;
}

float SFMLRenderer::getHeight() const
{
    return _window.getView().getSize().y;
}

net::PlayerInput SFMLRenderer::getPlayerInput()
{
    net::PlayerInput input{};
    if (!_window.hasFocus()) {
        _swapPressedLastFrame = false;
        return input;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        input.up = true;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        input.down = true;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        input.left = true;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        input.right = true;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
    {
        input.fire = true;
        if (_config.audio.enabled)
            playShootSound();
    }

    bool swapHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Q);
#if (SFML_VERSION_MAJOR > 2) || (SFML_VERSION_MAJOR == 2 && SFML_VERSION_MINOR >= 5)
    swapHeld = swapHeld || sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Q);
#endif
    input.swapWeapon = swapHeld && !_swapPressedLastFrame;
    _swapPressedLastFrame = swapHeld;
    return input;
}

void SFMLRenderer::render(const RemoteDisplay &display)
{
    // Apply game view for rendering (adapts to window size)
    applyGameView();
    
    const float dt = _clock.restart().asSeconds();
    _lastDeltaTime = dt;
    _deltaSeconds = dt;

    // Check for level changes and play appropriate music
    if (_config.audio.enabled && display.currentLevel != _lastLevel) {
        _lastLevel = display.currentLevel;
        
        // Check if this is a boss level (10 or 15 in the config)
        bool isBossLevel = (display.currentLevel == _config.gameplay.bossLevel || 
                           display.currentLevel == _config.gameplay.boss2Level);
        
        if (isBossLevel && !_isBossLevel) {
            playBossMusic();
        } else if (!isBossLevel && _isBossLevel) {
            playBackgroundMusic();
        }
    }
    
    // Start background music if none is playing
    if (_config.audio.enabled && _backgroundMusic.getStatus() != sf::Music::Playing && 
        _bossMusic.getStatus() != sf::Music::Playing &&
        display.currentLevel > 0) {
        playBackgroundMusic();
    }
    
    // Detect monster deaths and play explosion sounds
    for (const auto& [id, monster] : display.monsters) {
        if (!monster.alive && _previousMonsters.find(id) != _previousMonsters.end() && _previousMonsters[id]) {
            // Monster just died
            std::cout << "[sound] Monster " << id << " (type " << monster.type << ") died\n";
            // Check if it's a kamikaze (type 5)
            if (monster.type == 5) {
                std::cout << "[sound] -> This is a KAMIKAZE!\n";
                playSound("kamikaze_explosion");
            } else {
                playSound("explosion");
            }
        }
    }
    
    // Detect player damage - play sound immediately when HP drops
    for (const auto& [id, player] : display.players) {
        auto prevIt = _previousPlayerHealth.find(id);
        if (prevIt != _previousPlayerHealth.end()) {
            // Check if HP decreased at all
            if (player.hp < prevIt->second) {
                // Player took damage - play sound immediately
                std::cout << "[sound] DAMAGE DETECTED! Player " << id << " HP: " << prevIt->second << " -> " << player.hp << "\n";
                _playerHitSound.stop(); // Stop any playing instance to play immediately
                _playerHitSound.play();
            }
        }
        // Always update HP for next frame
        _previousPlayerHealth[id] = player.hp;
    }
    
    // Detect powerup collection - check both inactive and removed powerups
    for (const auto& prevId : _previousPowerUps) {
        auto currentIt = display.powerUps.find(prevId);
        // Check if powerup was removed or became inactive
        if (currentIt == display.powerUps.end() || !currentIt->second.active) {
            std::cout << "[sound] PowerUp " << prevId << " was COLLECTED!\n";
            _powerUpSound.stop();
            _powerUpSound.play();
        }
    }
    
    // Update previous powerup states
    _previousPowerUps.clear();
    for (const auto& [id, powerup] : display.powerUps) {
        if (powerup.active) {
            _previousPowerUps.insert(id);
        }
    }
    
    // Update previous monster states
    _previousMonsters.clear();
    for (const auto& [id, monster] : display.monsters) {
        _previousMonsters[id] = monster.alive;
    }

    drawStarfield(dt);
    updateProjectileEffects(display, dt);
    updateMonsterEffects(display);
    updateExplosions(dt);

    drawPlayers(display.players);
    drawMonsters(display.monsters);
    drawShields(display.shields);
    drawPowerUps(display.powerUps);
    drawBullets(display.bullets);
    drawExplosions();

    if (_config.systems.levelSystem)
        drawText("Level: " + std::to_string(display.currentLevel),
                 Vector2(20.0f, 20.0f), 40, Color(255, 255, 255));
}

void SFMLRenderer::spawnExplosion(const Vector2f &position)
{
    ExplosionInstance instance{};
    instance.position = {position.x, position.y};
    _explosions.push_back(instance);
    
    // Play explosion sound (kamikaze explosions use this function)
    std::cout << "[sound] Explosion spawned at (" << position.x << ", " << position.y << ")\n";
    _kamikazeExplosionSound.stop();
    _kamikazeExplosionSound.play();
}

void SFMLRenderer::updateExplosions(float dt)
{
    if (_explosions.empty())
        return;

    auto spriteRes = _spriteManager.getSprite("explosion");
    if (!spriteRes || spriteRes->frames.empty())
        return;

    const float frameDuration = spriteRes->frameDuration > 0.0f ? spriteRes->frameDuration : 0.05f;
    const std::size_t frameCount = spriteRes->frames.size();

    for (auto &explosion : _explosions)
    {
        if (explosion.finished)
            continue;

        explosion.frameTimer += dt;
        while (explosion.frameTimer >= frameDuration)
        {
            explosion.frameTimer -= frameDuration;
            if (explosion.currentFrame + 1 < frameCount)
            {
                ++explosion.currentFrame;
            }
            else
            {
                explosion.finished = true;
                break;
            }
        }
    }

    _explosions.erase(
        std::remove_if(
            _explosions.begin(),
            _explosions.end(),
            [](const ExplosionInstance &instance) { return instance.finished; }),
        _explosions.end());
}

void SFMLRenderer::drawExplosions()
{
    if (_explosions.empty())
        return;

    auto spriteRes = _spriteManager.getSprite("explosion");
    if (!spriteRes || spriteRes->frames.empty())
        return;

    for (const auto &explosion : _explosions)
    {
        if (explosion.finished)
            continue;

        const std::size_t frameIndex = std::min(explosion.currentFrame, spriteRes->frames.size() - 1);
        const sf::IntRect &frame = spriteRes->frames[frameIndex];
        sf::Sprite sprite(spriteRes->texture, frame);
        sprite.setOrigin(static_cast<float>(frame.width) / 2.0f,
                         static_cast<float>(frame.height) / 2.0f);
        sprite.setScale(2.5f, 2.5f);
        sprite.setPosition(explosion.position);
        _window.draw(sprite);
    }
}

sf::IntRect SFMLRenderer::pickPlayerFrame(const SpriteManager::SpriteResource &resource, SFMLRenderer::PlayerVisualState &visualState)
{
    if (resource.frames.empty())
        return resource.rect;

    const float tilt = std::clamp(visualState.tilt, -1.0f, 1.0f);
    const bool facingBackward = visualState.facingBackward;
    const float mild = 0.25f;
    const float strong = 0.6f;

    std::size_t frameIndex = facingBackward ? 9u : 0u;
    if (_config.gameplay.bulletDirection == config::ScrollDirection::LeftToRight)
    {
        if (tilt >= strong)
            frameIndex = 2u;
        else if (tilt >= mild)
            frameIndex = 1u;
        else if (tilt <= -strong)
            frameIndex = 4u;
        else if (tilt <= -mild)
            frameIndex = 3u;
        else
            frameIndex = 0u;
    }
    else
    {
        if (tilt >= strong)
            frameIndex = 5u;
        else if (tilt >= mild)
            frameIndex = 6u;
        else if (tilt <= -strong)
            frameIndex = 8u;
        else if (tilt <= -mild)
            frameIndex = 7u;
        else
            frameIndex = 9u;
    }

    if (frameIndex >= resource.frames.size())
        frameIndex = resource.frames.empty() ? 0u : resource.frames.size() - 1u;
    return resource.frames[frameIndex];
}

void SFMLRenderer::pruneMissingPlayerVisuals(const std::unordered_map<PlayerId, RemotePlayer> &players)
{
    if (players.empty())
    {
        _playerVisuals.clear();
        return;
    }

    for (auto it = _playerVisuals.begin(); it != _playerVisuals.end(); )
    {
        if (players.find(it->first) == players.end())
            it = _playerVisuals.erase(it);
        else
            ++it;
    }
}

void SFMLRenderer::drawPlayers(const std::unordered_map<PlayerId, RemotePlayer> &players)
{
    if (players.empty())
    {
        _playerVisuals.clear();
        return;
    }

    if (auto spriteRes = _spriteManager.getSprite("player_ship"))
    {
        const float playerSize = _config.gameRender.playerSize;
        const float targetWidth = playerSize * 1.6f;
        const float targetHeight = playerSize * 0.8f;
        const float orientationThreshold = std::max(_config.gameplay.playerSpeed * 0.2f, 25.0f);
        const float invDt = (_deltaSeconds > 0.00001f) ? 1.0f / _deltaSeconds : 0.0f;

        for (const auto &[id, player] : players)
        {
            if (!player.alive)
                continue;

            auto &visual = _playerVisuals[id];
            const sf::Vector2f currentPos = toSFVector(player.position);
            if (!visual.initialized)
            {
                visual.initialized = true;
                visual.lastPosition = currentPos;
                visual.facingBackward = false;
            }

            const sf::Vector2f delta = currentPos - visual.lastPosition;
            visual.lastPosition = currentPos;

            sf::Vector2f measuredVelocity{0.0f, 0.0f};
            if (invDt > 0.0f)
                measuredVelocity = delta * invDt;

            constexpr float smoothing = 0.25f;
            visual.velocity.x = visual.velocity.x + smoothing * (measuredVelocity.x - visual.velocity.x);
            visual.velocity.y = visual.velocity.y + smoothing * (measuredVelocity.y - visual.velocity.y);

            if (measuredVelocity.x > orientationThreshold)
                visual.facingBackward = false;
            else if (measuredVelocity.x < -orientationThreshold)
                visual.facingBackward = true;

            const float maxSpeed = std::max(_config.gameplay.playerSpeed, 1.0f);
            const float targetTilt = std::clamp(-visual.velocity.y / maxSpeed, -1.0f, 1.0f);
            visual.tilt = visual.tilt + 0.3f * (targetTilt - visual.tilt);

            const sf::IntRect activeRect = pickPlayerFrame(*spriteRes, visual);
            const float frameWidth = static_cast<float>(activeRect.width);
            const float frameHeight = static_cast<float>(activeRect.height);

            sf::Sprite sprite(spriteRes->texture, activeRect);
            if (frameWidth > 0.f && frameHeight > 0.f)
            {
                sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
                sprite.setScale(targetWidth / frameWidth, targetHeight / frameHeight);
            }
            sprite.setPosition(currentPos);
            sprite.setRotation(_config.gameRender.playerRotation);
            const auto color = toSFMLColor(colorForPlayer(id));
            sprite.setColor(color);

            if (player.player_power_up_type == PlayerPowerUpType::Shield) {
                auto shield_sprite_sheet = _spriteManager.getSprite("player_shield");

                //sf::IntRect frame = shield_sprite->frames[0];;
                sf::IntRect shield_frame = _spriteManager.getAnimatedFrame(*shield_sprite_sheet, _deltaSeconds);
                sf::Sprite shield_sprite(shield_sprite_sheet->texture, shield_frame);
                shield_sprite.setPosition(player.position.x, player.position.y);
                shield_sprite.setColor(color);

                float size = 24.0f;

                const float spriteWidth = static_cast<float>(shield_frame.width);
                const float spriteHeight = static_cast<float>(shield_frame.height);
                const float maxDim = static_cast<float>(std::max(shield_frame.width, shield_frame.height));
                if (maxDim > 0.0f)
                {
                    const float scale = (size * 2.0f) / maxDim;  // Make shield larger than half size
                    shield_sprite.setOrigin(spriteWidth / 2.0f, spriteHeight / 2.0f);

                    // Use the sprite's default direction to calculate transformation
                    // Only apply rotation/flip if canRotate is true
                    if (shield_sprite_sheet->canRotate)
                    {
                        auto transform = calculateSpriteTransform(shield_sprite_sheet->defaultDirection,
                                                                 measuredVelocity.x, measuredVelocity.y);

                        float scaleX = transform.flipX ? -scale : scale;
                        float scaleY = transform.flipY ? -scale : scale;

                        shield_sprite.setScale(scaleX, scaleY);
                        shield_sprite.setRotation(transform.rotation);
                    }
                    else
                    {
                        shield_sprite.setScale(scale, scale);
                        shield_sprite.setRotation(0.0f);
                    }
                }
                _window.draw(shield_sprite);
            }
            _window.draw(sprite);
        }

        pruneMissingPlayerVisuals(players);
        return;
    }

    pruneMissingPlayerVisuals(players);
    for (const auto &[id, player] : players)
    {
        if (!player.alive)
            continue;
        const float playerSize = _config.gameRender.playerSize;
        sf::RectangleShape ship({playerSize * 1.6f, playerSize * 0.8f});
        ship.setOrigin(playerSize * 0.8f, playerSize * 0.4f);
        ship.setPosition(player.position.x, player.position.y);
        ship.setRotation(_config.gameRender.playerRotation);
        ship.setFillColor(toSFMLColor(colorForPlayer(id)));
        _window.draw(ship);
    }
}

void SFMLRenderer::drawMonsters(const std::unordered_map<EntityId, RemoteMonster> &monsters)
{
    // Update boss 2 blink timer for fade effect
    _boss2BlinkTimer += _deltaSeconds;
    float cycleLength = _boss2VisibleDuration + _boss2InvisibleDuration;
    if (_boss2BlinkTimer >= cycleLength) {
        _boss2BlinkTimer -= cycleLength;
    }
    
    // Calculate fade alpha for boss 2 (smooth fade in/out)
    float boss2Alpha = 255.0f;
    float fadeDuration = 0.5f; // Time to fade in/out
    if (_boss2BlinkTimer < fadeDuration) {
        // Fading in at start of visible phase
        boss2Alpha = (_boss2BlinkTimer / fadeDuration) * 255.0f;
    } else if (_boss2BlinkTimer < _boss2VisibleDuration - fadeDuration) {
        // Fully visible
        boss2Alpha = 255.0f;
    } else if (_boss2BlinkTimer < _boss2VisibleDuration) {
        // Fading out before invisible phase
        float fadeProgress = (_boss2VisibleDuration - _boss2BlinkTimer) / fadeDuration;
        boss2Alpha = fadeProgress * 255.0f;
    } else {
        // Invisible phase (teleporting)
        boss2Alpha = 0.0f;
    }
    
    // Clean up animation states for dead/removed monsters
    for (auto it = _monsterAnimStates.begin(); it != _monsterAnimStates.end();)
    {
        if (monsters.find(it->first) == monsters.end())
            it = _monsterAnimStates.erase(it);
        else
            ++it;
    }
    
    for (const auto &entry : monsters)
    {
        const EntityId monsterId = entry.first;
        const auto &monster = entry.second;
        if (!monster.alive)
            continue;

        // Boss 2 (type 7) - skip drawing when fully invisible
        bool isBoss2 = (monster.type == static_cast<std::uint8_t>(_config.gameplay.boss2MonsterType));
        if (isBoss2 && boss2Alpha < 1.0f)
            continue;

        float size = 24.0f;
        sf::Color color(200, 60, 60);
        if (auto it = _config.gameplay.MonstersType.find(monster.type); it != _config.gameplay.MonstersType.end())
        {
            size = it->second.size;
            const auto &c = it->second.color;
            color = sf::Color(c.r, c.g, c.b);
        }
        
        // Apply fade alpha for boss 2
        if (isBoss2) {
            color.a = static_cast<std::uint8_t>(boss2Alpha);
        }

        // Look up sprite key for this monster type from the mapping
        SpriteManager::SpriteResource* spriteRes = nullptr;
        const std::string spriteKey = _spriteManager.getMonsterSpriteKey(monster.type);
        if (!spriteKey.empty())
        {
            spriteRes = _spriteManager.getSprite(spriteKey);
        }
        
        if (spriteRes)
        {
            sf::IntRect activeRect = spriteRes->rect;
            
            // Use per-entity animation state for independent animations
            if (spriteRes->animate && spriteRes->frames.size() > 1 && spriteRes->frameDuration > 0.0f)
            {
                // Get or create animation state for this monster
                auto &animState = _monsterAnimStates[monsterId];
                
                // Update animation timer
                animState.elapsed += _deltaSeconds;
                while (animState.elapsed >= spriteRes->frameDuration)
                {
                    animState.elapsed -= spriteRes->frameDuration;
                    animState.currentFrame = (animState.currentFrame + 1) % spriteRes->frames.size();
                }
                
                activeRect = spriteRes->frames[animState.currentFrame];
            }
            
            sf::Sprite sprite(spriteRes->texture, activeRect);
            const float spriteWidth = static_cast<float>(activeRect.width);
            const float spriteHeight = static_cast<float>(activeRect.height);
            const float maxDim = static_cast<float>(std::max(activeRect.width, activeRect.height));
            if (maxDim > 0.0f)
            {
                const float scale = size / maxDim;
                sprite.setOrigin(spriteWidth / 2.0f, spriteHeight / 2.0f);
                
                // Use the sprite's default direction to calculate transformation
                // Only apply rotation/flip if canRotate is true
                if (spriteRes->canRotate)
                {
                    auto transform = calculateSpriteTransform(spriteRes->defaultDirection, 
                                                             monster.velocity.x, monster.velocity.y);
                    
                    float scaleX = transform.flipX ? -scale : scale;
                    float scaleY = transform.flipY ? -scale : scale;
                    
                    sprite.setScale(scaleX, scaleY);
                    sprite.setRotation(transform.rotation);
                }
                else
                {
                    sprite.setScale(scale, scale);
                    sprite.setRotation(0.0f);
                }
            }
            sprite.setPosition(monster.position.x, monster.position.y);
            sprite.setColor(color);
            _window.draw(sprite);
            continue;
        }

        sf::RectangleShape enemy({size, size});
        enemy.setOrigin(size / 2.0f, size / 2.0f);
        enemy.setPosition(monster.position.x, monster.position.y);
        enemy.setFillColor(color);
        _window.draw(enemy);
    }
}

void SFMLRenderer::drawShields(const std::unordered_map<EntityId, RemoteShield> &shields)
{
    // Clean up animation states for dead/removed shields
    for (auto it = _shieldAnimStates.begin(); it != _shieldAnimStates.end();)
    {
        if (shields.find(it->first) == shields.end())
            it = _shieldAnimStates.erase(it);
        else
            ++it;
    }

    for (const auto &entry : shields)
    {
        const EntityId shieldId = entry.first;
        const auto &shield = entry.second;
        if (!shield.alive)
            continue;

        // Get shield configuration based on parent monster type
        float size = 24.0f;
        sf::Color color(200, 200, 200);  // Default gray
        
        auto it = _config.gameplay.MonstersType.find(shield.type);
        if (it != _config.gameplay.MonstersType.end())
        {
            size = it->second.size * 0.5f;  // Shield is half monster size
            color = sf::Color(it->second.color.r, it->second.color.g, it->second.color.b);
        }

        // Try to load shield sprite from assets
        auto spriteRes = _spriteManager.getSprite("shield");
        if (spriteRes)
        {
            // Get or create animation state for this shield
            auto &animState = _shieldAnimStates[shieldId];
            
            // Get the current animated frame
            sf::IntRect frame = _spriteManager.getAnimatedFrame(*spriteRes, _deltaSeconds);
            
            sf::Sprite sprite(spriteRes->texture, frame);
            const float spriteWidth = static_cast<float>(frame.width);
            const float spriteHeight = static_cast<float>(frame.height);
            const float maxDim = static_cast<float>(std::max(frame.width, frame.height));
            if (maxDim > 0.0f)
            {
                const float scale = (size * 2.0f) / maxDim;  // Make shield larger than half size
                sprite.setOrigin(spriteWidth / 2.0f, spriteHeight / 2.0f);
                
                // Use the sprite's default direction to calculate transformation
                // Only apply rotation/flip if canRotate is true
                if (spriteRes->canRotate)
                {
                    auto transform = calculateSpriteTransform(spriteRes->defaultDirection, 
                                                             shield.velocity.x, shield.velocity.y);
                    
                    float scaleX = transform.flipX ? -scale : scale;
                    float scaleY = transform.flipY ? -scale : scale;
                    
                    sprite.setScale(scaleX, scaleY);
                    sprite.setRotation(transform.rotation);
                }
                else
                {
                    sprite.setScale(scale, scale);
                    sprite.setRotation(0.0f);
                }
            }
            sprite.setPosition(shield.position.x, shield.position.y);
            sprite.setColor(color);
            _window.draw(sprite);
            continue;
        }

        // Fallback: Draw a rectangle if no sprite found
        sf::RectangleShape shieldRect({size * 1.5f, size * 1.8f});
        shieldRect.setOrigin((size * 1.5f) / 2.0f, (size * 1.8f) / 2.0f);
        shieldRect.setPosition(shield.position.x, shield.position.y);
        shieldRect.setFillColor(sf::Color(color.r, color.g, color.b, 180));  // Semi-transparent
        shieldRect.setOutlineThickness(2.0f);
        shieldRect.setOutlineColor(color);
        _window.draw(shieldRect);
    }
}

void SFMLRenderer::drawPowerUps(const std::unordered_map<EntityId, RemotePowerUp> &powerUps)
{
    for (const auto &entry : powerUps)
    {
        const auto &powerup = entry.second;
        if (!powerup.active)
            continue;

        if (auto spriteRes = _spriteManager.getSprite("powerup"))
        {
            sf::Sprite sprite(spriteRes->texture, spriteRes->rect);
            const float spriteWidth = static_cast<float>(spriteRes->rect.width);
            const float spriteHeight = static_cast<float>(spriteRes->rect.height);
            const float maxDim = static_cast<float>(std::max(spriteRes->rect.width, spriteRes->rect.height));
            if (maxDim > 0.0f)
            {
                const float targetSize = _config.gameplay.powerUpSize * 2.0f;
                const float scale = targetSize / maxDim;
                sprite.setOrigin(spriteWidth / 2.0f, spriteHeight / 2.0f);
                sprite.setScale(scale, scale);
            }
            sprite.setPosition(powerup.position.x, powerup.position.y);
            sprite.setColor(sf::Color(_config.gameplay.powerUpColor.r,
                                      _config.gameplay.powerUpColor.g,
                                      _config.gameplay.powerUpColor.b));
            _window.draw(sprite);
            continue;
        }

        sf::CircleShape circle(_config.gameplay.powerUpSize);
        circle.setPosition(powerup.position.x - _config.gameplay.powerUpSize,
                           powerup.position.y - _config.gameplay.powerUpSize);
        circle.setFillColor(sf::Color(_config.gameplay.powerUpColor.r,
                                      _config.gameplay.powerUpColor.g,
                                      _config.gameplay.powerUpColor.b));
        circle.setOutlineColor(sf::Color(_config.gameplay.powerUpOutlineColor.r,
                                         _config.gameplay.powerUpOutlineColor.g,
                                         _config.gameplay.powerUpOutlineColor.b));
        circle.setOutlineThickness(_config.gameplay.powerUpOutlineThickness);
        _window.draw(circle);
    }
}

void SFMLRenderer::drawBullets(const std::unordered_map<EntityId, RemoteBullet> &bullets)
{
    const auto spriteRes = _spriteManager.getSprite("bullet");
    const bool hasSprite = static_cast<bool>(spriteRes);
    const sf::IntRect activeRect = (hasSprite ? _spriteManager.getAnimatedFrame(*spriteRes, _deltaSeconds) : sf::IntRect());
    const float spriteWidth = hasSprite ? static_cast<float>(activeRect.width) : 0.0f;
    const float spriteHeight = hasSprite ? static_cast<float>(activeRect.height) : 0.0f;
    const float maxDim = hasSprite ? static_cast<float>(std::max(activeRect.width, activeRect.height)) : 0.0f;
    const bool hasScale = hasSprite && maxDim > 0.0f;
    const float spriteScale = hasScale ? (_config.gameRender.bulletSize * 2.0f) / maxDim : 1.0f;
    const sf::Color baseBulletColor(
        _config.gameRender.bulletColor.r,
        _config.gameRender.bulletColor.g,
        _config.gameRender.bulletColor.b
    );
    const sf::Color enemyBulletColor(255, 90, 70);  // Red/orange color for enemy bullets
    const sf::Color rocketColor(255, 140, 90);
    static constexpr float kRocketSpriteScale = 4.5f;
    static constexpr float kRocketRadiusScale = 4.0f;

    const auto drawFallbackBullet = [&](const RemoteBullet &bullet, const sf::Color &color, float radius) {
        sf::CircleShape b(radius);
        b.setOrigin(radius, radius);
        b.setPosition(bullet.position.x, bullet.position.y);
        b.setFillColor(color);
        _window.draw(b);
    };

    for (const auto &entry : bullets)
    {
        const auto &bullet = entry.second;
        if (!bullet.active)
            continue;

        const bool isLaser = bullet.weaponType == 1;
        const bool isRocket = bullet.weaponType == 2;
        const bool isEnemyBullet = !bullet.fromPlayer;
        
        if (isLaser)
        {
                drawLaserBeamAt(sf::Vector2f(bullet.position.x, bullet.position.y));
            continue;
        }

        // Select sprite based on bullet type
        SpriteManager::SpriteResource *currentSpriteRes = nullptr;
        sf::IntRect currentRect;
        
        if (isEnemyBullet && !isRocket) {
            // Choose enemy bullet sprite type based on bullet ID for variety
            int enemyBulletType = (entry.first % 5) + 1;  // Types 1-5
            std::string enemyBulletKey = "enemy_bullet_type" + std::to_string(enemyBulletType);
            currentSpriteRes = const_cast<SpriteManager::SpriteResource*>(_spriteManager.getSprite(enemyBulletKey));
            if (currentSpriteRes) {
                currentRect = _spriteManager.getAnimatedFrame(*currentSpriteRes, _deltaSeconds);
            }
        } else {
            currentSpriteRes = const_cast<SpriteManager::SpriteResource*>(spriteRes);
            currentRect = activeRect;
        }
        
        const bool hasCurrentSprite = currentSpriteRes != nullptr;

        if (hasCurrentSprite)
        {
            const float currentSpriteWidth = static_cast<float>(currentRect.width);
            const float currentSpriteHeight = static_cast<float>(currentRect.height);
            const float currentMaxDim = static_cast<float>(std::max(currentRect.width, currentRect.height));
            const bool hasCurrentScale = currentMaxDim > 0.0f;
            const float currentSpriteScale = hasCurrentScale ? (_config.gameRender.bulletSize * 2.0f) / currentMaxDim : 1.0f;
            
            sf::Sprite sprite(currentSpriteRes->texture, currentRect);
            if (hasCurrentScale)
            {
                sprite.setOrigin(currentSpriteWidth / 2.0f, currentSpriteHeight / 2.0f);
                float scale = currentSpriteScale;
                if (isRocket)
                    scale *= kRocketSpriteScale;
                sprite.setScale(scale, scale);
            }
            sprite.setPosition(bullet.position.x, bullet.position.y);
            
            // Choose color based on bullet source
            sf::Color bulletColor;
            if (isRocket)
                bulletColor = rocketColor;
            else if (isEnemyBullet)
                bulletColor = enemyBulletColor;
            else
                bulletColor = baseBulletColor;
                
            sprite.setColor(bulletColor);
            _window.draw(sprite);
            continue;
        }

        const float radius = _config.gameRender.bulletSize * (isRocket ? kRocketRadiusScale : 1.0f);
        const sf::Color &color = isRocket ? rocketColor : (isEnemyBullet ? enemyBulletColor : baseBulletColor);
        drawFallbackBullet(bullet, color, radius);
    }

    for (auto &fadeEntry : _laserFades) {
        float intensity = fadeEntry.second.remaining / _laserFadeDuration;
        if (intensity <= 0.0f)
            continue;
        if (intensity > 1.0f)
            intensity = 1.0f;
        drawLaserBeamAt(fadeEntry.second.position, intensity);
    }
}

void SFMLRenderer::updateProjectileEffects(const RemoteDisplay &display, float dt)
{
    auto it = _laserFades.begin();
    while (it != _laserFades.end()) {
        it->second.remaining -= dt;
        if (it->second.remaining <= 0.0f)
            it = _laserFades.erase(it);
        else
            ++it;
    }
    
    // Detect new enemy bullets
    for (const auto &[id, bullet] : display.bullets) {
        if (!bullet.fromPlayer && _lastBullets.find(id) == _lastBullets.end()) {
            // New enemy bullet spawned
            playSound("enemy_shoot");
        }
    }

    for (auto &entry : _lastBullets) {
        const bool bulletStillExists = display.bullets.find(entry.first) != display.bullets.end();
        const bool laserRemoved = (entry.second.weaponType == 1) && !bulletStillExists;
        if (laserRemoved && _laserFades.count(entry.first) == 0)
        {
            LaserFadeInstance fade{};
            fade.position = sf::Vector2f(entry.second.position.x, entry.second.position.y);
            fade.remaining = _laserFadeDuration;
            _laserFades.emplace(entry.first, fade);
        }

        const bool rocketRemoved = (entry.second.weaponType == 2) && !bulletStillExists;
        if (rocketRemoved)
            spawnExplosion(entry.second.position);
    }

    _lastBullets = display.bullets;
}

void SFMLRenderer::updateMonsterEffects(const RemoteDisplay &display)
{
    // Check for kamikaze (type 5) monsters that were removed - spawn explosion
    constexpr std::uint8_t KAMIKAZE_TYPE = 5;
    
    for (const auto &entry : _lastMonsters) {
        const bool monsterStillExists = display.monsters.find(entry.first) != display.monsters.end();
        const bool kamikazeRemoved = (entry.second.type == KAMIKAZE_TYPE) && !monsterStillExists;
        
        if (kamikazeRemoved) {
            spawnExplosion(entry.second.position);
        }
    }
    
    _lastMonsters = display.monsters;
}

void SFMLRenderer::drawLaserBeamAt(const sf::Vector2f &position, float intensity)
{
    if (intensity <= 0.0f)
        return;
    if (intensity > 1.0f)
        intensity = 1.0f;

    float baseHeight = _config.gameRender.bulletSize * 1.1f;
    if (baseHeight < 6.0f)
        baseHeight = 6.0f;
    float heightScale = 0.35f + (0.65f * intensity);
    float coreHeight = baseHeight * heightScale;

    float startX = position.x - 6.0f;
    if (startX < 0.0f)
        startX = 0.0f;
    float endX = static_cast<float>(_window.getSize().x) + 40.0f;
    float beamLength = endX - startX;
    if (beamLength < _config.gameRender.bulletSize * 10.0f)
        beamLength = _config.gameRender.bulletSize * 10.0f;
    if (beamLength <= 0.0f)
        return;

    sf::Uint8 glowAlpha = static_cast<sf::Uint8>(130.0f * intensity);
    sf::Uint8 coreAlpha = static_cast<sf::Uint8>(230.0f * intensity);
    sf::Uint8 outlineAlpha = static_cast<sf::Uint8>(220.0f * intensity);

    sf::RectangleShape glow({beamLength, coreHeight * 1.8f});
    glow.setOrigin(0.0f, glow.getSize().y / 2.0f);
    glow.setPosition(startX, position.y);
    glow.setFillColor(sf::Color(60, 200, 255, glowAlpha));
    _window.draw(glow);

    sf::RectangleShape core({beamLength, coreHeight});
    core.setOrigin(0.0f, coreHeight / 2.0f);
    core.setPosition(startX, position.y);
    core.setFillColor(sf::Color(150, 255, 255, coreAlpha));
    core.setOutlineColor(sf::Color(255, 255, 255, outlineAlpha));
    core.setOutlineThickness(1.2f * intensity);
    _window.draw(core);
}

sf::Vector2f SFMLRenderer::toSFVector(const Vector2f &vec) const
{
    return sf::Vector2f(vec.x, vec.y);
}

sf::Color SFMLRenderer::toSFMLColor(const Color &color) const
{
    return sf::Color(color.r, color.g, color.b, color.a);
}

Color SFMLRenderer::colorForPlayer(PlayerId id) const
{
    const std::array<const config::Color*, 4> colors = {
        &_config.gameRender.player1Color,
        &_config.gameRender.player2Color,
        &_config.gameRender.player3Color,
        &_config.gameRender.player4Color
    };
    const auto &c = *colors[id % colors.size()];
    return Color(c.r, c.g, c.b);
}

bool SFMLRenderer::isKeyPressed(int key)
{
    if (!_window.hasFocus())
        return false;
    return sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(key));
}

std::string SFMLRenderer::getTextInput()
{
    std::string result = _textInputBuffer;
    _textInputBuffer.clear();
    return result;
}

bool SFMLRenderer::wasMouseClicked() const
{
    return _mouseClicked;
}

Vector2 SFMLRenderer::getMousePosition() const
{
    return Vector2(static_cast<float>(_mousePosition.x), static_cast<float>(_mousePosition.y));
}

void SFMLRenderer::applyGameView()
{
    _window.setView(_gameView);
}

void SFMLRenderer::applyMenuView()
{
    _window.setView(_menuView);
}

void SFMLRenderer::initializeSoundSystem()
{
    std::cout << "[renderer] Initializing sound system...\n";
    
    // Try multiple possible sound paths
    std::vector<std::string> possiblePaths = {
        "src" + _fileSeparator + "assets" + _fileSeparator + "sound" + _fileSeparator,
        ".." + _fileSeparator + "src" + _fileSeparator + "assets" + _fileSeparator + "sound" + _fileSeparator,
        "../.." + _fileSeparator + "src" + _fileSeparator + "assets" + _fileSeparator + "sound" + _fileSeparator,
        "." + _fileSeparator + "src" + _fileSeparator + "assets" + _fileSeparator + "sound" + _fileSeparator
    };
    
    std::string soundPath;
    for (const auto& path : possiblePaths) {
        std::string testFile = path + "playerdamage.mp3";
        std::ifstream test(testFile);
        if (test.good()) {
            soundPath = path;
            std::cout << "[renderer] Found sound directory: " << soundPath << "\n";
            break;
        }
    }
    
    if (soundPath.empty()) {
        std::cerr << "[renderer] WARNING: Could not find sound directory, using procedural sounds\n";
        soundPath = possiblePaths[0]; // Use first as fallback
    }
    
    // Generate player shoot sound (high-pitched beep) - procedural
    generateSoundEffect(_playerShootBuffer, 880.0f, 0.05f, 3000.0f);
    _playerShootSound.setBuffer(_playerShootBuffer);
    _playerShootSound.setVolume(40.0f);
    
    // Generate enemy shoot sound (lower-pitched, more menacing) - procedural
    generateSoundEffect(_enemyShootBuffer, 440.0f, 0.08f, 2500.0f);
    _enemyShootSound.setBuffer(_enemyShootBuffer);
    _enemyShootSound.setVolume(35.0f);
    
    // Load player hit sound from file
    std::string playerDamagePath = soundPath + "playerdamage.mp3";
    std::cout << "[renderer] Trying to load: " << playerDamagePath << "\n";
    if (_playerHitBuffer.loadFromFile(playerDamagePath)) {
        _playerHitSound.setBuffer(_playerHitBuffer);
        _playerHitSound.setVolume(25.0f);
        std::cout << "[renderer] ✓ Loaded player damage sound\n";
    } else {
        std::cerr << "[renderer] ✗ Failed to load player damage sound from: " << playerDamagePath << "\n";
        std::cerr << "[renderer] Using procedural sound instead\n";
        generateSoundEffect(_playerHitBuffer, 200.0f, 0.15f, 4000.0f);
        _playerHitSound.setBuffer(_playerHitBuffer);
        _playerHitSound.setVolume(15.0f);
    }
    
    // Generate regular explosion sound - procedural
    generateSoundEffect(_explosionBuffer, 150.0f, 0.3f, 5000.0f);
    _explosionSound.setBuffer(_explosionBuffer);
    _explosionSound.setVolume(95.0f);
    
    // Load kamikaze explosion sound from file
    std::string kamikazePath = soundPath + "kamikaze explosion.mp3";
    std::cout << "[renderer] Trying to load: " << kamikazePath << "\n";
    if (_kamikazeExplosionBuffer.loadFromFile(kamikazePath)) {
        _kamikazeExplosionSound.setBuffer(_kamikazeExplosionBuffer);
        _kamikazeExplosionSound.setVolume(200.0f);
        std::cout << "[renderer] ✓ Loaded kamikaze explosion sound (" << _kamikazeExplosionBuffer.getSampleCount() << " samples)\n";
    } else {
        std::cerr << "[renderer] ✗ Failed to load kamikaze explosion sound from: " << kamikazePath << "\n";
        std::cerr << "[renderer] Using procedural sound instead\n";
        generateSoundEffect(_kamikazeExplosionBuffer, 180.0f, 0.25f, 6000.0f);
        _kamikazeExplosionSound.setBuffer(_kamikazeExplosionBuffer);
        _kamikazeExplosionSound.setVolume(200.0f);
    }
    
    // Load powerup sound from file
    std::string powerupPath = soundPath + "powerup.mp3";
    std::cout << "[renderer] Trying to load: " << powerupPath << "\n";
    if (_powerUpBuffer.loadFromFile(powerupPath)) {
        _powerUpSound.setBuffer(_powerUpBuffer);
        _powerUpSound.setVolume(75.0f);
        std::cout << "[renderer] ✓ Loaded powerup sound (" << _powerUpBuffer.getSampleCount() << " samples)\n";
    } else {
        std::cerr << "[renderer] ✗ Failed to load powerup sound from: " << powerupPath << "\n";
        std::cerr << "[renderer] Using procedural sound instead\n";
        generateSoundEffect(_powerUpBuffer, 1200.0f, 0.2f, 3500.0f);
        _powerUpSound.setBuffer(_powerUpBuffer);
        _powerUpSound.setVolume(75.0f);
    }
    
    // Load background music from file
    if (_backgroundMusic.openFromFile(soundPath + "background music.mp3")) {
        _backgroundMusic.setLoop(true);
        _backgroundMusic.setVolume(50.0f);
        std::cout << "[renderer] Loaded background music\n";
    } else {
        std::cerr << "[renderer] Failed to load background music\n";
    }
    
    // Load boss music from file
    if (_bossMusic.openFromFile(soundPath + "boss2damage.mp3")) {
        _bossMusic.setLoop(true);
        _bossMusic.setVolume(30.0f);
        std::cout << "[renderer] Loaded boss music\n";
    } else {
        std::cerr << "[renderer] Failed to load boss music\n";
    }
    
    std::cout << "[renderer] Sound system initialized\n";
}

void SFMLRenderer::generateSoundEffect(sf::SoundBuffer& buffer, float frequency, float duration, float amplitude)
{
    const unsigned int sampleRate = 44100;
    const std::size_t sampleCount = static_cast<std::size_t>(sampleRate * duration);
    std::vector<sf::Int16> samples(sampleCount);
    
    for (std::size_t i = 0; i < sampleCount; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(sampleRate);
        
        // Apply envelope (fade out)
        float envelope = 1.0f - (t / duration);
        envelope = std::max(0.0f, envelope);
        
        // Generate sine wave
        float value = std::sin(2.0f * 3.14159265f * frequency * t) * amplitude * envelope;
        
        // Add some harmonics for richer sound
        value += std::sin(4.0f * 3.14159265f * frequency * t) * amplitude * 0.3f * envelope;
        
        samples[i] = static_cast<sf::Int16>(std::clamp(value, -32767.0f, 32767.0f));
    }
    
    buffer.loadFromSamples(samples.data(), samples.size(), 1, sampleRate);
}

void SFMLRenderer::playBackgroundMusic()
{
    if (!_config.audio.enabled)
        return;
    
    if (_bossMusic.getStatus() == sf::Music::Playing) {
        _bossMusic.stop();
    }
    
    if (_backgroundMusic.getStatus() != sf::Music::Playing) {
        _backgroundMusic.play();
        _isBossLevel = false;
    }
}

void SFMLRenderer::playBossMusic()
{
    if (!_config.audio.enabled)
        return;
    
    if (_backgroundMusic.getStatus() == sf::Music::Playing) {
        _backgroundMusic.stop();
    }
    
    if (_bossMusic.getStatus() != sf::Music::Playing) {
        _bossMusic.play();
        _isBossLevel = true;
    }
}

void SFMLRenderer::stopAllMusic()
{
    _backgroundMusic.stop();
    _bossMusic.stop();
    _isBossLevel = false;
}

} // namespace rtype::client
