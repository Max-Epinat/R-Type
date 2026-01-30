#include "rtype/client/GameClient.hpp"
#include "rtype/common/GameConfig.hpp"
#include "rtype/common/AsioNetwork.hpp"
#include "rtype/client/SFMLRenderer.hpp"

#include <SFML/Window/Keyboard.hpp>
#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <random>
#include <span>

namespace rtype::client
{
namespace
{
Timestamp nowMs()
{
    using clock = std::chrono::steady_clock;
    const auto now = clock::now().time_since_epoch();
    return static_cast<Timestamp>(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
}
}

GameClient::GameClient(const std::string &host, std::uint16_t port)
{
    bool loaded = false;
    if (_config.loadFromFile("config/game.ini"))
    {
        _config.loadFromFile("config/engine.ini");
        _config.loadFromFile("config/systems.ini");  // Also load systems config
        _config.loadFromFile("config/assets.ini");   // Also load assets config
        loaded = true;
    }
    else if (_config.loadFromFile("../config/game.ini"))
    {
        _config.loadFromFile("../config/engine.ini");
        _config.loadFromFile("../config/systems.ini");  // Also load systems config
        _config.loadFromFile("../config/assets.ini");   // Also load assets config
        loaded = true;
    }
    else if (_config.loadFromFile("../../config/game.ini"))
    {
        _config.loadFromFile("../../config/engine.ini");
        _config.loadFromFile("../../config/systems.ini");  // Also load systems config
        _config.loadFromFile("../../config/assets.ini");   // Also load assets config
        loaded = true;
    }
    
    if (loaded)
    {
        std::cout << "[client] loaded config\n";
    }
    else
    {
        std::cerr << "[client] config not found, using defaults\n";
    }
    
    std::cout << "[client] Initializing network...\n";
    _ioContext = network::NetworkFactory::createIOContext();
    _socket = _ioContext->createUdpSocket(0);
    _serverEndpoint = _ioContext->createEndpoint(host, port);
    std::cout << "[client] Connecting to server " << host << ":" << port << "\n";
    
    std::cout << "[client] Creating renderer window (" 
              << _config.render.windowWidth << "x" 
              << _config.render.windowHeight << ")...\n";
    _renderer = RenderFactory::createRenderer(
        static_cast<std::uint32_t>(_config.render.windowWidth),
        static_cast<std::uint32_t>(_config.render.windowHeight),
        _config.render.windowTitle,
        _config
    );
    std::cout << "[client] Window created successfully!\n";
}
GameClient::~GameClient()
{
    _running = false;
    _ioContext->stop();
    if (_networkThread.joinable())
        _networkThread.join();
}

void GameClient::run()
{
    std::cout << "[client] Starting main game loop...\n";
    std::cout << "[client] Window is " << (_renderer->isOpen() ? "OPEN" : "CLOSED") << "\n";
    
    _running = true;
    _lastPacketTime = std::chrono::steady_clock::now();
    _lastInputSentTime = std::chrono::steady_clock::now();
    if (_config.network.serverTimeout > 0 && _config.network.serverTimeout != 5.0f)
        _serverTimeOut = std::chrono::seconds(static_cast<int>(_config.network.serverTimeout));
    networkReceive();
    _networkThread = std::thread([this]() { _ioContext->run(); });

    std::cout << "[client] Entering render loop...\n";
    while (_renderer->isOpen())
    {
        if (!_renderer->pollEvents()) {
            _running = false;
            if (_myPlayerId != 0xFF) {
                const auto packet = net::serializeDisconnect({_myPlayerId}, _sequence++, nowMs());
                _socket->sendTo(packet, *_serverEndpoint);
            }
            break;
        }

        // Capture state at START of frame - prevents mid-frame changes from causing flicker
        MenuState currentState = _menuState;

        _renderer->clear(Color(0, 0, 0));
        
        if (currentState == MenuState::MainMenu)
        {
            // Debug: show state
            // std::cout << "[client] Rendering MainMenu\n";
            const float centerX = _renderer->getWidth() * 0.5f;
            const float padding = 20.0f;
            
            // Title
            _renderer->drawText("R-TYPE", Vector2(centerX - 90.0f, 120.0f), 80, Color(255, 255, 255));
            
            // Helper to draw a button with text-fitted hitbox
            auto drawButton = [&](const std::string &label, float y, const Color &bgColor, const Color &textColor) -> Vector2 {
                Vector2 textSize = _renderer->getTextBounds(label, 40);
                Vector2 btnSize(textSize.x + padding * 2.0f, textSize.y + padding * 2.0f);
                Vector2 btnPos(centerX - btnSize.x * 0.5f, y);
                
                _renderer->drawRectangle(btnPos, btnSize, bgColor);
                _renderer->drawText(label, Vector2(btnPos.x + padding, btnPos.y + padding), 40, textColor);
                
                // Return rect as (x, y, width, height) encoded in two Vector2
                return btnPos;  // position
            };
            
            Vector2 createPos = drawButton("Create Game", 240.0f, Color(30, 60, 30), Color(100, 200, 100));
            Vector2 createSize = _renderer->getTextBounds("Create Game", 40);
            createSize = Vector2(createSize.x + padding * 2.0f, createSize.y + padding * 2.0f);
            
            Vector2 joinPos = drawButton("Join Game", 340.0f, Color(30, 60, 30), Color(100, 200, 100));
            Vector2 joinSize = _renderer->getTextBounds("Join Game", 40);
            joinSize = Vector2(joinSize.x + padding * 2.0f, joinSize.y + padding * 2.0f);
            
            Vector2 exitPos = drawButton("Exit", 440.0f, Color(60, 30, 30), Color(200, 100, 100));
            Vector2 exitSize = _renderer->getTextBounds("Exit", 40);
            exitSize = Vector2(exitSize.x + padding * 2.0f, exitSize.y + padding * 2.0f);
            
            if (!_errorMessage.empty())
            {
                _renderer->drawText(_errorMessage, Vector2(centerX - 150.0f, 560.0f), 30, Color(255, 100, 100));
            }
            
            if (_renderer->wasMouseClicked())
            {
                Vector2 mousePos = _renderer->getMousePosition();
                std::cout << "[client] Mouse clicked at (" << mousePos.x << ", " << mousePos.y << ")\n";
                
                auto inside = [](const Vector2 &p, const Vector2 &pos, const Vector2 &size) {
                    return p.x >= pos.x && p.x <= pos.x + size.x &&
                           p.y >= pos.y && p.y <= pos.y + size.y;
                };
                
                if (inside(mousePos, createPos, createSize))
                {
                    _menuState = MenuState::CreateRoom;
                    _inputText.clear();
                    _errorMessage.clear();
                    std::cout << "[client] Create Game clicked\n";
                }
                else if (inside(mousePos, joinPos, joinSize))
                {
                    _menuState = MenuState::JoinRoom;
                    _inputText.clear();
                    _errorMessage.clear();
                    std::cout << "[client] Join Game clicked\n";
                }
                else if (inside(mousePos, exitPos, exitSize))
                {
                    _running = false;
                    std::cout << "[client] Exit clicked\n";
                    break;
                }
            }
        }
        else if (currentState == MenuState::CreateRoom)
        {
            _renderer->drawText("Create Room", Vector2(280, 80), 60, Color(255, 255, 255));
            _renderer->drawText("Room Name:", Vector2(200, 220), 40, Color(200, 200, 200));
            
            // Draw input box
            _renderer->drawRectangle(Vector2(190, 270), Vector2(420, 50), Color(40, 40, 40));
            _renderer->drawText(_inputText + "|", Vector2(200, 280), 35, Color(255, 255, 255));
            
            _renderer->drawText("Press ENTER to create", Vector2(220, 360), 30, Color(150, 150, 150));
            _renderer->drawText("Press DELETE/BACKSPACE to erase", Vector2(170, 400), 25, Color(150, 150, 150));
            _renderer->drawText("ESC to go back", Vector2(290, 450), 25, Color(150, 150, 150));
            
            std::string input = _renderer->getTextInput();
            for (char c : input)
            {
                if (c == '\r')
                {
                    if (!_inputText.empty())
                    {
                        sendCreateRoom(_inputText);
                        _inputText.clear();
                    }
                }
                else if (c == '\b')
                {
                    if (!_inputText.empty())
                        _inputText.pop_back();
                }
                else
                {
                    _inputText += c;
                }
            }
            
            // 36 = ESC
            if (_renderer->isKeyPressed(36))
            {
                _menuState = MenuState::MainMenu;
                _inputText.clear();
            }
        }
        else if (currentState == MenuState::JoinRoom)
        {
            _renderer->drawText("Join Room", Vector2(300, 80), 60, Color(255, 255, 255));
            _renderer->drawText("Room ID:", Vector2(200, 220), 40, Color(200, 200, 200));
            
            // Draw input box
            _renderer->drawRectangle(Vector2(190, 270), Vector2(420, 50), Color(40, 40, 40));
            _renderer->drawText(_inputText + "|", Vector2(200, 280), 35, Color(255, 255, 255));
            
            _renderer->drawText("Press ENTER to join", Vector2(230, 360), 30, Color(150, 150, 150));
            _renderer->drawText("Press DELETE/BACKSPACE to erase", Vector2(170, 400), 25, Color(150, 150, 150));
            _renderer->drawText("ESC to go back", Vector2(290, 450), 25, Color(150, 150, 150));
            
            if (!_errorMessage.empty())
            {
                _renderer->drawText(_errorMessage, Vector2(250, 520), 30, Color(255, 100, 100));
            }
            
            std::string input = _renderer->getTextInput();
            for (char c : input)
            {
                if (c == '\r')
                {
                    if (!_inputText.empty())
                    {
                        try {
                            std::uint32_t roomId = std::stoul(_inputText);
                            sendJoinRoom(roomId);
                            _inputText.clear();
                        } catch (...) {
                            _errorMessage = "Invalid room ID";
                        }
                    }
                }
                else if (c == '\b')
                {
                    if (!_inputText.empty())
                        _inputText.pop_back();
                }
                else
                {
                    _inputText += c;
                }
            }
            
            // ESC to go back
            if (_renderer->isKeyPressed(36))
            {
                _menuState = MenuState::MainMenu;
                _inputText.clear();
                _errorMessage.clear();
            }
        }
        else if (currentState == MenuState::Lobby)
        {
            // std::cout << "[client] Rendering Lobby\n";
            _renderer->drawText("Lobby: " + _currentRoom.roomName, Vector2(250, 100), 50, Color(255, 255, 255));
            _renderer->drawText("Players: " + std::to_string(_currentRoom.playerCount) + "/" + std::to_string(_currentRoom.maxPlayers), 
                              Vector2(300, 200), 40, Color(200, 200, 200));
            _renderer->drawText("Room ID: " + std::to_string(_currentRoom.roomId), Vector2(280, 270), 35, Color(180, 180, 180));
            
            if (_currentRoom.isHost)
            {
                _renderer->drawText("You are the host", Vector2(280, 340), 30, Color(100, 255, 100));
                _renderer->drawText("Press SPACE to start game", Vector2(220, 400), 35, Color(100, 200, 100));
            }
            else
            {
                _renderer->drawText("Waiting for host to start...", Vector2(200, 350), 35, Color(200, 200, 100));
            }
            
            _renderer->drawText("ESC to leave room", Vector2(270, 500), 25, Color(200, 100, 100));
            
            // Send keep-alive packet to prevent timeout while idle in lobby
            // Send a dummy input every 2 seconds to keep the connection alive
            auto timeSinceLastInput = std::chrono::steady_clock::now() - _lastInputSentTime;
            if (timeSinceLastInput > std::chrono::seconds(2))
            {
                net::PlayerInput keepAlive{};
                keepAlive.player = _myPlayerId;
                // All input fields are false (default), this is just a keep-alive
                sendInput(keepAlive);
                _lastInputSentTime = std::chrono::steady_clock::now();
            }
            
            bool spacePressed = _renderer->isKeyPressed(57);
            if (_currentRoom.isHost && spacePressed && !_startGameSent)
            {
                sendStartGame();
                _startGameSent = true;  // Only send once
            }
            else if (!spacePressed)
            {
                _startGameSent = false;  // Reset when key released
            }
            
            if (_renderer->isKeyPressed(36))
            {
                sendLeaveRoom();
                _menuState = MenuState::MainMenu;
            }
        }
        else if (currentState == MenuState::GameOver)
        {
            // Render the game in the background
            {
                std::lock_guard lock(_stateMutex);
                _renderer->render(_display);
            }
            
            const float centerX = _renderer->getWidth() * 0.5f;
            const float padding = 20.0f;
            
            // If spectating, just show the game with a simple message at the top
            if (_isSpectating)
            {
                // If all players died while spectating, exit spectate mode to show game over UI
                if (_allPlayersDead)
                {
                    _isSpectating = false;
                    _selectedGameOverOption = 0;  // Reset to first option
                    std::cout << "[client] All players dead - exiting spectate mode\n";
                    // Don't continue - fall through to show game over UI
                }
                else
                {
                    // Still spectating - show simple message at the top center
                    std::string message = "Press ESC to quit";
                    Vector2 textSize = _renderer->getTextBounds(message, 30);
                    _renderer->drawText(message, Vector2(centerX - textSize.x * 0.5f, 20.0f), 30, Color(255, 255, 255));
                    
                    // Check for server timeout while spectating
                    if (checkServerTimeout())
                    {
                        std::cerr << "[client] Server timeout while spectating\n";
                        _running = false;
                        // Don't continue - let it fall through to display and break from main loop
                    }
                    else
                    {
                        // Send keep-alive packets to prevent timeout while spectating
                        auto timeSinceLastInput = std::chrono::steady_clock::now() - _lastInputSentTime;
                        if (timeSinceLastInput > std::chrono::seconds(2))
                        {
                            net::PlayerInput keepAlive{};
                            keepAlive.player = _myPlayerId;
                            sendInput(keepAlive);
                            _lastInputSentTime = std::chrono::steady_clock::now();
                        }
                        
                        // Handle ESC to exit spectating
                        if (_renderer->isKeyPressed(36))  // ESC
                        {
                            sendLeaveRoom();
                            _menuState = MenuState::MainMenu;
                        }
                        
                        // Continue to next frame - don't show any other UI
                        _renderer->display();
                        continue;
                    }
                }
            }
            
            // Not spectating - show game over overlay
            _renderer->drawRectangle(Vector2(0, 0), 
                                    Vector2(_renderer->getWidth(), _renderer->getHeight()), 
                                    Color(0, 0, 0, 180));
            
            _renderer->drawText("GAME OVER", Vector2(centerX - 180.0f, 100.0f), 70, Color(255, 50, 50));
            
            if (_allPlayersDead)
            {
                _renderer->drawText("All players are dead!", 
                                  Vector2(centerX - 200.0f, 200.0f), 40, Color(255, 200, 100));
                
                // Simple Leave button for everyone (host or not)
                auto drawButton = [&](const std::string &label, float y, const Color &bgColor, const Color &textColor) -> std::pair<Vector2, Vector2> {
                    Vector2 textSize = _renderer->getTextBounds(label, 40);
                    Vector2 btnSize(textSize.x + padding * 2.0f, textSize.y + padding * 2.0f);
                    Vector2 btnPos(centerX - btnSize.x * 0.5f, y);
                    
                    _renderer->drawRectangle(btnPos, btnSize, bgColor);
                    _renderer->drawText(label, Vector2(btnPos.x + padding, btnPos.y + padding), 40, textColor);
                    
                    return {btnPos, btnSize};
                };
                
                auto [leavePos, leaveSize] = drawButton("Leave to Menu", 320.0f, Color(60, 30, 30), Color(200, 100, 100));
                
                _renderer->drawText("Click or press ESC to leave", 
                                  Vector2(centerX - 180.0f, 410.0f), 25, Color(180, 180, 180));
                
                // Handle mouse input
                if (_renderer->wasMouseClicked())
                {
                    Vector2 mousePos = _renderer->getMousePosition();
                    if (mousePos.y >= leavePos.y && mousePos.y <= leavePos.y + leaveSize.y &&
                        mousePos.x >= leavePos.x && mousePos.x <= leavePos.x + leaveSize.x)
                    {
                        sendLeaveRoom();
                        _menuState = MenuState::MainMenu;
                    }
                }
                
                // Handle keyboard input
                if (_renderer->isKeyPressed(36))  // ESC
                {
                    sendLeaveRoom();
                    _menuState = MenuState::MainMenu;
                }
        }
            else
            {
                // Some players still alive - offer spectate or leave
                _renderer->drawText("Other players are still fighting!", 
                                  Vector2(centerX - 240.0f, 200.0f), 40, Color(200, 200, 100));
                
                // Helper function to draw a button
                auto drawButton = [&](const std::string &label, float y, bool selected) -> std::pair<Vector2, Vector2> {
                    Vector2 textSize = _renderer->getTextBounds(label, 40);
                    Vector2 btnSize(textSize.x + padding * 2.0f, textSize.y + padding * 2.0f);
                    Vector2 btnPos(centerX - btnSize.x * 0.5f, y);
                    
                    Color bgColor = selected ? Color(50, 100, 50) : Color(30, 60, 30);
                    Color textColor = selected ? Color(150, 255, 150) : Color(100, 200, 100);
                    
                    _renderer->drawRectangle(btnPos, btnSize, bgColor);
                    _renderer->drawText(label, Vector2(btnPos.x + padding, btnPos.y + padding), 40, textColor);
                    
                    return {btnPos, btnSize};
                };
                
                // Option 0: Spectate
                auto [spectatePos, spectateSize] = drawButton("Spectate", 320.0f, _selectedGameOverOption == 0);
                
                // Option 1: Leave
                auto [leavePos, leaveSize] = drawButton("Leave to Menu", 400.0f, _selectedGameOverOption == 1);
                
                _renderer->drawText("Click or use UP/DOWN and ENTER", 
                                  Vector2(centerX - 220.0f, 480.0f), 25, Color(180, 180, 180));
                
                // Handle mouse input
                if (_renderer->wasMouseClicked())
                {
                    Vector2 mousePos = _renderer->getMousePosition();
                    
                    // Check Spectate button
                    if (mousePos.y >= spectatePos.y && mousePos.y <= spectatePos.y + spectateSize.y &&
                        mousePos.x >= spectatePos.x && mousePos.x <= spectatePos.x + spectateSize.x)
                    {
                        _isSpectating = true;
                        
                        // Clear all entity displays to remove stale sprites
                        // that died while we were in the GameOver menu
                        {
                            std::lock_guard lock(_stateMutex);
                            _display.monsters.clear();
                            _display.bullets.clear();
                            _display.shields.clear();
                            _display.powerUps.clear();
                        }
                        
                        sendSpectatorMode(true);
                    }
                    // Check Leave button
                    else if (mousePos.y >= leavePos.y && mousePos.y <= leavePos.y + leaveSize.y &&
                                 mousePos.x >= leavePos.x && mousePos.x <= leavePos.x + leaveSize.x)
                    {
                        sendLeaveRoom();
                        _menuState = MenuState::MainMenu;
                    }
                }
                
                // Handle keyboard input
                if (_renderer->isKeyPressed(17))  // W or UP
                {
                    _selectedGameOverOption = 0;
                }
                if (_renderer->isKeyPressed(31))  // S or DOWN
                {
                    _selectedGameOverOption = 1;
                }
                if (_renderer->isKeyPressed(28))  // ENTER
                {
                    if (_selectedGameOverOption == 0)
                    {
                        _isSpectating = true;
                        
                        // Clear all entity displays to remove stale sprites
                        // that died while we were in the GameOver menu
                        {
                            std::lock_guard lock(_stateMutex);
                            _display.monsters.clear();
                            _display.bullets.clear();
                            _display.shields.clear();
                            _display.powerUps.clear();
                        }
                        
                        sendSpectatorMode(true);
                    }
                    else
                    {
                        sendLeaveRoom();
                        _menuState = MenuState::MainMenu;
                    }
                }
            }
        }
        else if (currentState == MenuState::InGame)
        {
            // std::cout << "[client] Rendering InGame\n";
            if (checkServerTimeout())
            {
                std::cerr << "[client] Server timeout - no response for " 
                          << _serverTimeOut.count() << " seconds\n";
                _running = false;
                break;
            }

            net::PlayerInput input{};
            input = _renderer->getPlayerInput();
            input.player = _myPlayerId;
            if (input.fire)
                _renderer->playSound("shoot");
            
            sendInput(input);
            _lastInputSentTime = std::chrono::steady_clock::now();

            {
                std::lock_guard lock(_stateMutex);
                _renderer->render(_display);
            }
        }
        
        // Also render game in GameOver state when spectating or waiting
        if (currentState == MenuState::GameOver)
        {
            // Continue sending input if alive players exist (keeps connection alive)
            // and check for timeout
            if (checkServerTimeout())
            {
                std::cerr << "[client] Server timeout\n";
                _running = false;
                break;
            }
        }
        
        _renderer->display();
    }
}

bool GameClient::checkServerTimeout()
{
    return (std::chrono::steady_clock::now() - _lastPacketTime) > _serverTimeOut;
}

void GameClient::networkReceive()
{
    _socket->asyncReceive([this](const std::uint8_t* data, std::size_t size, std::unique_ptr<network::IEndpoint>) {
        if (!_running.load())
            return;
        if (size > 0)
        {
            handlePacket(data, size);
        }
    });
}

void GameClient::handlePacket(const std::uint8_t* data, std::size_t size)
{
    net::PacketHeader header{};
    std::vector<std::uint8_t> payload;
    _lastPacketTime = std::chrono::steady_clock::now();
    if (!net::deserializePayload(data, size, header, payload))
        return;

    switch (header.type)
    {
    case net::PacketType::PlayerAssignment: {
        net::PlayerAssignment assignment{};
        if (net::deserializePlayerAssignment(payload.data(), payload.size(), assignment))
        {
            _myPlayerId = assignment.playerId;
            std::cout << "[client] assigned player ID: " << static_cast<int>(_myPlayerId) << '\n';
        }
        break;
    }
    case net::PacketType::PlayerState: {
        // Only process game state packets when in game or spectating
        if (_menuState != MenuState::InGame && !(_menuState == MenuState::GameOver && _isSpectating))
            break;
        net::PlayerState state{};
        if (net::deserializePlayerState(payload.data(), payload.size(), state))
        {
            std::lock_guard lock(_stateMutex);
            auto &player = _display.players[state.player];
            player.position = {state.x, state.y};
            player.hp = state.hp;
            player.alive = state.alive;
            player.player_power_up_type = state.powerUpType;
        }
        break;
    }
    case net::PacketType::MonsterState: {
        // Only process game state packets when in game or spectating
        if (_menuState != MenuState::InGame && !(_menuState == MenuState::GameOver && _isSpectating))
            break;
        net::MonsterState state{};
        if (net::deserializeMonsterState(payload.data(), payload.size(), state))
        {
            std::lock_guard lock(_stateMutex);
            if (!state.alive)
            {
                _display.monsters.erase(state.id);
            }
            else
            {
                auto &monster = _display.monsters[state.id];
                monster.position = {state.x, state.y};
                monster.velocity = {state.vx, state.vy};
                monster.type = state.type;
                monster.alive = state.alive;
            }
        }
        break;
    }
    case net::PacketType::ShieldState: {
        // Only process game state packets when in game or spectating
        if (_menuState != MenuState::InGame && !(_menuState == MenuState::GameOver && _isSpectating))
            break;
        net::ShieldState state{};
        if (net::deserializeShieldState(payload.data(), payload.size(), state))
        {
            std::lock_guard lock(_stateMutex);
            if (!state.alive)
            {
                _display.shields.erase(state.id);
            }
            else
            {
                auto &shield = _display.shields[state.id];
                shield.position = {state.x, state.y};
                shield.velocity = {state.vx, state.vy};
                shield.type = state.type;
                shield.alive = state.alive;
            }
        }
        break;
    }
    case net::PacketType::ShieldDeath: {
        // Only process game state packets when in game or spectating
        if (_menuState != MenuState::InGame && !(_menuState == MenuState::GameOver && _isSpectating))
            break;
        net::ShieldDeath death{};
        if (net::deserializeShieldDeath(payload.data(), payload.size(), death))
        {
            std::lock_guard lock(_stateMutex);
            _display.shields.erase(death.id);
        }
        break;
    }
    case net::PacketType::BulletState: {
        // Only process game state packets when in game or spectating
        if (_menuState != MenuState::InGame && !(_menuState == MenuState::GameOver && _isSpectating))
            break;
        net::BulletState state{};
        if (net::deserializeBulletState(payload.data(), payload.size(), state))
        {
            std::lock_guard lock(_stateMutex);
            if (!state.active)
            {
                _display.bullets.erase(state.id);
            }
            else
            {
                auto &bullet = _display.bullets[state.id];
                bullet.position = {state.x, state.y};
                bullet.weaponType = state.weaponType;
                bullet.fromPlayer = state.fromPlayer;
                bullet.active = state.active;
                bullet.fromPlayer = state.fromPlayer;
            }
        }
        break;
    }
    case net::PacketType::PowerUpState: {
        // Only process game state packets when in game or spectating
        if (_menuState != MenuState::InGame && !(_menuState == MenuState::GameOver && _isSpectating))
            break;
        net::PowerUpState state{};
        if (net::deserializePowerUpState(payload.data(), payload.size(), state))
        {
            std::lock_guard lock(_stateMutex);
            if (!state.active)
            {
                _display.powerUps.erase(state.id);
            }
            else
            {
                auto &power = _display.powerUps[state.id];
                power.position = {state.x, state.y};
                power.type = state.type;
                power.value = state.value;
                power.active = state.active;
            }
        }
        break;
    }
    case net::PacketType::LevelBegin: {
        // Only process game state packets when in game or spectating
        if (_menuState != MenuState::InGame && !(_menuState == MenuState::GameOver && _isSpectating))
            break;
        net::LevelBegin level{};
        if (net::deserializeLevelBegin(payload.data(), payload.size(), level))
        {
            std::cout << "[client] Level " << static_cast<int>(level.levelNumber) << " started!\n";
            std::lock_guard lock(_stateMutex);
            _display.currentLevel = level.levelNumber;
        }
        break;
    }
    case net::PacketType::Disconnect: {
        net::DisconnectNotice notice{};
        if (net::deserializeDisconnect(payload.data(), payload.size(), notice))
        {
            std::lock_guard lock(_stateMutex);
            
            // If we are the one being disconnected, shut down
            if (notice.player == _myPlayerId)
            {
                std::cerr << "[client] Server disconnected us (likely timeout)\n";
                _running = false;
                return;
            }
            
            // Otherwise, just remove the other player from display
            _display.players.erase(notice.player);
            std::cout << "[client] Player " << static_cast<int>(notice.player) << " disconnected\n";
        }
        break;
    }
    case net::PacketType::RoomCreated: {
        net::RoomCreated created{};
        if (net::deserializeRoomCreated(payload.data(), payload.size(), created))
        {
            handleCreateRoom(created);
        }
        break;
    }
    case net::PacketType::RoomJoined: {
        net::RoomJoined joined{};
        if (net::deserializeRoomJoined(payload.data(), payload.size(), joined))
        {
            handleRoomJoined(joined);
        }
        break;
    }
    case net::PacketType::RoomLeft: {
        net::RoomLeft left{};
        if (net::deserializeRoomLeft(payload.data(), payload.size(), left))
        {
            handleRoomLeft(left);
        }
        break;
    }
    case net::PacketType::GameStarted: {
        net::GameStarted started{};
        if (net::deserializeGameStarted(payload.data(), payload.size(), started))
        {
            handleGameStarted(started);
        }
        break;
    }
    case net::PacketType::RoomError: {
        net::RoomError error{};
        if (net::deserializeRoomError(payload.data(), payload.size(), error))
        {
            handleRoomError(error);
        }
        break;
    }
    case net::PacketType::PlayerDeath: {
        net::PlayerDeath death{};
        if (net::deserializePlayerDeath(payload.data(), payload.size(), death))
        {
            handlePlayerDeath(death);
        }
        break;
    }
    case net::PacketType::AllPlayersDead: {
        net::AllPlayersDead msg{};
        if (net::deserializeAllPlayersDead(payload.data(), payload.size(), msg))
        {
            handleAllPlayersDead(msg);
        }
        break;
    }
    case net::PacketType::HostChanged: {
        net::HostChanged msg{};
        if (net::deserializeHostChanged(payload.data(), payload.size(), msg))
        {
            if (msg.roomId == _currentRoom.roomId)
            {
                bool wasHost = _currentRoom.isHost;
                _currentRoom.isHost = (msg.newHostId == _myPlayerId);
                std::cout << "[client] Host changed to player " << static_cast<int>(msg.newHostId) 
                          << " (I am " << (wasHost ? "no longer" : (_currentRoom.isHost ? "now" : "not")) << " host)\n";
            }
        }
        break;
    }
    default:
        break;
    }
}

void GameClient::sendInput(const net::PlayerInput &input)
{
    const auto packet = net::serializePlayerInput(input, _sequence++, nowMs());
    _socket->sendTo(packet, *_serverEndpoint);
}

void GameClient::handleCreateRoom(const net::RoomCreated &created)
{
    std::cout << "[client] Room created: " << created.roomName << " (ID: " << created.roomId << "), current state: " << static_cast<int>(_menuState) << "\n";
    
    // If already in game, ignore
    if (_menuState == MenuState::InGame)
    {
        std::cout << "[client] IGNORING RoomCreated - already in game\n";
        return;
    }
    
    _myPlayerId = created.playerId;
    std::cout << "[client] Assigned player ID: " << static_cast<int>(_myPlayerId) << "\n";
    
    // Reset timeout counter when creating a room
    _lastPacketTime = std::chrono::steady_clock::now();
    
    // Clear all display entities from any previous room
    {
        std::lock_guard lock(_stateMutex);
        _display.players.clear();
        _display.monsters.clear();
        _display.bullets.clear();
        _display.shields.clear();
        _display.powerUps.clear();
        _display.currentLevel = 1;
    }
    
    _currentRoom.roomId = created.roomId;
    _currentRoom.roomName = created.roomName;
    _currentRoom.isHost = true;
    _currentRoom.playerCount = 1;
    _currentRoom.maxPlayers = 4;
    _menuState = MenuState::Lobby;
}

void GameClient::handleRoomJoined(const net::RoomJoined &joined)
{
    std::cout << "[client] RoomJoined packet: room " << joined.roomId << ", " 
              << static_cast<int>(joined.playerCount) << " players, current state: " << static_cast<int>(_menuState) << "\n";
    
    // If already in game, ignore lobby updates entirely to prevent flicker
    if (_menuState == MenuState::InGame)
    {
        std::cout << "[client] IGNORING RoomJoined - already in game\n";
        return;
    }

    // Update room info (player count may have changed)
    _currentRoom.roomId = joined.roomId;
    _currentRoom.roomName = joined.roomName;
    _currentRoom.playerCount = joined.playerCount;
    _currentRoom.maxPlayers = 4;

    if (_myPlayerId == 0xFF || _myPlayerId != joined.playerId)
    {
        _myPlayerId = joined.playerId;
        std::cout << "[client] Assigned player ID: " << static_cast<int>(_myPlayerId) << "\n";
    }
    _currentRoom.isHost = (joined.hostId == _myPlayerId);
    
    // Reset timeout counter when joining a room
    _lastPacketTime = std::chrono::steady_clock::now();
    
    // Only transition to Lobby if we're not already there
    if (_menuState != MenuState::Lobby)
    {
        std::cout << "[client] Joined room: " << joined.roomName << " (ID: " << joined.roomId << ")\n";
        
        // Clear all display entities from any previous room
        std::lock_guard lock(_stateMutex);
        _display.players.clear();
        _display.monsters.clear();
        _display.bullets.clear();
        _display.shields.clear();
        _display.powerUps.clear();
        _display.currentLevel = 1;
        
        _menuState = MenuState::Lobby;
    }
}

void GameClient::handleRoomLeft(const net::RoomLeft &left)
{
    std::cout << "[client] Left room: " << left.roomId << "\n";
    
    // Clear all display entities from the old room
    std::lock_guard lock(_stateMutex);
    _display.players.clear();
    _display.monsters.clear();
    _display.bullets.clear();
    _display.shields.clear();
    _display.powerUps.clear();
    _display.currentLevel = 1;
    
    _menuState = MenuState::MainMenu;
}

void GameClient::handleGameStarted(const net::GameStarted &started)
{
    std::cout << "[client] Received GameStarted for room: " << started.roomId 
              << " (my room: " << _currentRoom.roomId << ", current state: " << static_cast<int>(_menuState) << ")\n";
    
    // Only transition to InGame if this is our room
    if (started.roomId != _currentRoom.roomId)
    {
        std::cout << "[client] Ignoring GameStarted - not my room\n";
        return;
    }
    
    _menuState = MenuState::InGame;
    _startGameSent = false; // reset guard
    _allPlayersDead = false; // reset game over state
    _isSpectating = false; // reset spectator mode
    
    // Reset timeout counter when game starts
    _lastPacketTime = std::chrono::steady_clock::now();
    
    // Clear all display state for fresh restart
    _display.players.clear();
    _display.monsters.clear();
    _display.bullets.clear();
    _display.powerUps.clear();
    
    std::cout << "[client] State changed to InGame (" << static_cast<int>(_menuState) << ") - display cleared for restart\n";
}

void GameClient::handleRoomError(const net::RoomError &error)
{
    std::cerr << "[client] Room error: " << error.message << "\n";
    _errorMessage = std::string(error.message);
}

void GameClient::sendCreateRoom(const std::string &roomName)
{
    net::CreateRoom createRoom{};
    std::strncpy(createRoom.roomName, roomName.c_str(), 31);
    createRoom.roomName[31] = '\0';
    auto packet = net::serializeCreateRoom(createRoom, _sequence++, nowMs());
    _socket->sendTo(packet, *_serverEndpoint);
    std::cout << "[client] Sent CreateRoom request: " << roomName << "\n";
}

void GameClient::sendJoinRoom(std::uint32_t roomId)
{
    net::JoinRoom joinRoom{};
    joinRoom.roomId = roomId;
    auto packet = net::serializeJoinRoom(joinRoom, _sequence++, nowMs());
    _socket->sendTo(packet, *_serverEndpoint);
    std::cout << "[client] Sent JoinRoom request: " << roomId << "\n";
}

void GameClient::sendLeaveRoom()
{
    net::LeaveRoom leaveRoom{};
    leaveRoom.roomId = _currentRoom.roomId;
    auto packet = net::serializeLeaveRoom(leaveRoom, _sequence++, nowMs());
    _socket->sendTo(packet, *_serverEndpoint);
    std::cout << "[client] Sent LeaveRoom request\n";
}

void GameClient::sendStartGame()
{
    net::StartGame startGame{};
    startGame.roomId = _currentRoom.roomId;
    auto packet = net::serializeStartGame(startGame, _sequence++, nowMs());
    _socket->sendTo(packet, *_serverEndpoint);
    std::cout << "[client] Sent StartGame request for room " << _currentRoom.roomId << "\n";
}

void GameClient::handlePlayerDeath(const net::PlayerDeath &death)
{
    std::cout << "[client] Player " << static_cast<int>(death.player) << " died\n";
    
    // If it's me who died, switch to game over screen
    if (death.player == _myPlayerId && _menuState == MenuState::InGame)
    {
        std::cout << "[client] I died! Showing game over screen\n";
        _menuState = MenuState::GameOver;
        _selectedGameOverOption = 0;
        _allPlayersDead = false;
    }
}

void GameClient::handleAllPlayersDead(const net::AllPlayersDead &msg)
{
    std::cout << "[client] All players dead in room " << msg.roomId << "\n";
    if (msg.roomId == _currentRoom.roomId)
    {
        _allPlayersDead = true;
    }
}

void GameClient::sendSpectatorMode(bool enabled)
{
    net::SpectatorMode spec{};
    spec.playerId = _myPlayerId;
    spec.enabled = enabled;
    auto packet = net::serializeSpectatorMode(spec, _sequence++, nowMs());
    _socket->sendTo(packet, *_serverEndpoint);
    std::cout << "[client] Sent SpectatorMode: " << (enabled ? "enabled" : "disabled") << "\n";
}}