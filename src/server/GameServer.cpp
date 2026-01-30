/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** GameServer implementation
*/

#include "rtype/server/GameServer.hpp"
#include "rtype/common/GameConfig.hpp"
#include "rtype/common/AsioNetwork.hpp"
#include "rtype/server/ClientHandler.hpp"
#include "rtype/server/RoomManager.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <random>
#include <span>
#include <string>
#include <vector>
#include <unordered_set>
#include <cstring>

namespace rtype::server
{

config::GameConfig GameServer::loadConfig()
{
    config::GameConfig cfg;
    bool loaded = false;
    
    if (cfg.loadFromFile("config/game.ini"))
    {
        loaded = true;
    }
    else if (cfg.loadFromFile("../config/game.ini"))
    {
        loaded = true;
    }
    else if (cfg.loadFromFile("../../config/game.ini"))
    {
        loaded = true;
    }
    
    if (loaded)
    {
        std::cout << "[server] loaded config\n";
        
        // Load additional config files
        if (cfg.loadFromFile("config/engine.ini"))
        {
            std::cout << "[server] loaded engine config\n";
        }
        else if (cfg.loadFromFile("../config/engine.ini"))
        {
            std::cout << "[server] loaded engine config\n";
        }
        else if (cfg.loadFromFile("../../config/engine.ini"))
        {
            std::cout << "[server] loaded engine config\n";
        }
        
        if (cfg.loadFromFile("config/systems.ini"))
        {
            std::cout << "[server] loaded systems config\n";
        }
        else if (cfg.loadFromFile("../config/systems.ini"))
        {
            std::cout << "[server] loaded systems config\n";
        }
        else if (cfg.loadFromFile("../../config/systems.ini"))
        {
            std::cout << "[server] loaded systems config\n";
        }
        else
        {
            std::cerr << "[server] systems config not found\n";
        }
    }
    else
    {
        std::cerr << "[server] config not found, using defaults\n";
    }
    return cfg;
}

GameServer::GameServer(std::uint16_t port)
    : _ioContext(network::NetworkFactory::createIOContext()),
      _socket(_ioContext->createUdpSocket(port)),
      _config{loadConfig()},
      _roomManager(std::make_unique<RoomManager>(_config))
{
}

GameServer::~GameServer()
{
    stop();
}

void GameServer::start()
{
    if (_running.exchange(true))
        return;

    scheduleReceive();

    _networkThread = std::thread([this]() {
        try
        {
            _ioContext->run();
        }
        catch (const std::exception &e)
        {
            std::cerr << "[server] network loop error: " << e.what() << '\n';
        }
    });

    _gameThread = std::thread([this]() { updateGameLoop(); });
    std::cout << "[server] listening on UDP port " << _socket->getLocalPort() << '\n';
}

void GameServer::stop()
{
    if (!_running.exchange(false))
        return;

    _ioContext->stop();

    if (_networkThread.joinable())
        _networkThread.join();
    if (_gameThread.joinable())
        _gameThread.join();
}

void GameServer::scheduleReceive()
{
    _socket->asyncReceive([this](const std::uint8_t* data, std::size_t size, std::unique_ptr<network::IEndpoint> sender) {
        if (!_running.load())
            return;
        
        if (size > 0)
        {
            PendingPacket packet;
            packet.data.assign(data, data + size);
            packet.sender = std::move(sender);
            {
                std::lock_guard lock(_rxMutex);
                _rxQueue.emplace_back(std::move(packet));
            }
        }
    });
}

void GameServer::handlePacket(const std::uint8_t* data, std::size_t size, std::unique_ptr<network::IEndpoint> sender)
{
    net::PacketHeader header{};
    std::vector<std::uint8_t> payload;
    if (!net::deserializePayload(data, size, header, payload))
        return;

    const std::string endpointKey = sender->getKey();

    switch (header.type)
    {
    case net::PacketType::CreateRoom: {
        net::CreateRoom createRoom{};
        if (net::deserializeCreateRoom(payload.data(), payload.size(), createRoom))
            handleCreateRoom(createRoom, std::move(sender));
        break;
    }
    case net::PacketType::JoinRoom: {
        net::JoinRoom joinRoom{};
        if (net::deserializeJoinRoom(payload.data(), payload.size(), joinRoom))
            handleJoinRoom(joinRoom, std::move(sender));
        break;
    }
    case net::PacketType::LeaveRoom: {
        net::LeaveRoom leaveRoom{};
        if (net::deserializeLeaveRoom(payload.data(), payload.size(), leaveRoom))
            handleLeaveRoom(leaveRoom, endpointKey);
        break;
    }
    case net::PacketType::StartGame: {
        net::StartGame startGame{};
        if (net::deserializeStartGame(payload.data(), payload.size(), startGame))
            handleStartGame(startGame, endpointKey);
        break;
    }
    case net::PacketType::RoomList: {
        handleRoomList(std::move(sender));
        break;
    }
    case net::PacketType::PlayerInput: {
        net::PlayerInput input{};
        if (net::deserializePlayerInput(payload.data(), payload.size(), input))
            handlePlayerInput(input, endpointKey);
        break;
    }
    case net::PacketType::Disconnect: {
        net::DisconnectNotice notice{};
        if (net::deserializeDisconnect(payload.data(), payload.size(), notice))
            handleDisconnect(notice, endpointKey);
        break;
    }
    case net::PacketType::SpectatorMode: {
        net::SpectatorMode spec{};
        if (net::deserializeSpectatorMode(payload.data(), payload.size(), spec))
            handleSpectatorMode(spec, endpointKey);
        break;
    }
    default:
        break;
    }
}

PlayerId GameServer::getOrCreatePlayer(const std::string& endpointKey, std::unique_ptr<network::IEndpoint> sender)
{
    auto it = _endpointToPlayer.find(endpointKey);
    if (it != _endpointToPlayer.end())
    {
        return it->second;
    }
    
    PlayerId playerId = _nextPlayerId++;
    _endpointToPlayer[endpointKey] = playerId;
    _playerEndpoints[playerId] = std::move(sender);
    
    net::PlayerAssignment assignment{playerId};
    auto assignmentPacket = net::serializePlayerAssignment(assignment, _sequence++, nowMilliseconds());
    flushSends(assignmentPacket, *_playerEndpoints[playerId]);
    
    std::cout << "[server] Assigned player ID " << static_cast<int>(playerId) << " to " << endpointKey << "\n";
    return playerId;
}

PlayerInputComponent GameServer::translateNetworkInput(const net::PlayerInput &netInput)
{
    PlayerInputComponent input{};
    input.left       = netInput.left;
    input.right      = netInput.right;
    input.up         = netInput.up;
    input.down       = netInput.down;
    input.fire       = netInput.fire;
    input.swapWeapon = netInput.swapWeapon;
    return input;
}

void GameServer::handleCreateRoom(const net::CreateRoom &createRoom, std::unique_ptr<network::IEndpoint> sender)
{
    const std::string endpointKey = sender->getKey();
    PlayerId playerId = getOrCreatePlayer(endpointKey, std::move(sender));
    
    std::string roomName(createRoom.roomName);
    RoomId roomId = _roomManager->createRoom(roomName, playerId);
    
    auto room = _roomManager->getRoom(roomId);
    if (!room)
    {
        std::cerr << "[server] Failed to create room\n";
        return;
    }
    
    auto endpoint = _playerEndpoints[playerId]->clone();
    if (!_roomManager->joinRoom(roomId, playerId, std::move(endpoint), nowMilliseconds()))
    {
        std::cerr << "[server] Host failed to join own room\n";
        return;
    }
    
    net::RoomCreated response{};
    response.roomId = roomId;
    std::strncpy(response.roomName, roomName.c_str(), 31);
    response.roomName[31] = '\0';
    response.hostId = playerId;
    response.playerId = playerId;
    
    auto packet = net::serializeRoomCreated(response, _sequence++, nowMilliseconds());
    flushSends(packet, *_playerEndpoints[playerId]);
    
    std::cout << "[server] Player " << static_cast<int>(playerId) << " created room " << roomId << " '" << roomName << "'\n";
}

void GameServer::handleJoinRoom(const net::JoinRoom &joinRoom, std::unique_ptr<network::IEndpoint> sender)
{
    const std::string endpointKey = sender->getKey();
    PlayerId playerId = getOrCreatePlayer(endpointKey, std::move(sender));
    
    auto room = _roomManager->getRoom(joinRoom.roomId);
    if (!room)
    {
        net::RoomError error{};
        error.errorCode = 1;
        std::strncpy(error.message, "Room not found", 63);
        error.message[63] = '\0';
        auto packet = net::serializeRoomError(error, _sequence++, nowMilliseconds());
        flushSends(packet, *_playerEndpoints[playerId]);
        std::cout << "[server] Player " << static_cast<int>(playerId) << " tried to join non-existent room " << joinRoom.roomId << "\n";
        return;
    }
    
    if (room->isFull())
    {
        net::RoomError error{};
        error.errorCode = 2;
        std::strncpy(error.message, "Room is full", 63);
        error.message[63] = '\0';
        auto packet = net::serializeRoomError(error, _sequence++, nowMilliseconds());
        flushSends(packet, *_playerEndpoints[playerId]);
        std::cout << "[server] Player " << static_cast<int>(playerId) << " tried to join full room " << joinRoom.roomId << "\n";
        return;
    }
    
    auto endpoint = _playerEndpoints[playerId]->clone();
    if (_roomManager->joinRoom(joinRoom.roomId, playerId, std::move(endpoint), nowMilliseconds()))
    {
        net::RoomJoined response{};
        response.roomId = joinRoom.roomId;
        std::strncpy(response.roomName, room->getName().c_str(), 31);
        response.roomName[31] = '\0';
        response.hostId = room->getHostId();
        response.playerCount = static_cast<std::uint8_t>(room->getPlayerCount());
        response.playerId = playerId;
        
        auto packet = net::serializeRoomJoined(response, _sequence++, nowMilliseconds());
        flushSends(packet, *_playerEndpoints[playerId]);
        
        // Notify other players in the room about the new player count
        for (auto& [pid, client] : room->getClients())
        {
            if (pid != playerId)
            {
                net::RoomJoined updateResponse{};
                updateResponse.roomId = joinRoom.roomId;
                std::strncpy(updateResponse.roomName, room->getName().c_str(), 31);
                updateResponse.roomName[31] = '\0';
                updateResponse.hostId = room->getHostId();
                updateResponse.playerCount = static_cast<std::uint8_t>(room->getPlayerCount());
                updateResponse.playerId = pid;
                
                auto updatePacket = net::serializeRoomJoined(updateResponse, _sequence++, nowMilliseconds());
                flushSends(updatePacket, client.getEndpoint());
            }
        }
        
        std::cout << "[server] Player " << static_cast<int>(playerId) << " joined room " << joinRoom.roomId << "\n";
    }
}

void GameServer::handleLeaveRoom(const net::LeaveRoom &leaveRoom, const std::string& endpointKey)
{
    auto it = _endpointToPlayer.find(endpointKey);
    if (it == _endpointToPlayer.end())
        return;
    
    PlayerId playerId = it->second;
    auto room = _roomManager->getRoomByPlayer(playerId);
    
    if (room)
    {
        PlayerId oldHostId = room->getHostId();
        bool wasHost = (oldHostId == playerId);
        
        _roomManager->leaveRoom(playerId);
        
        // If host left and room still has players, notify about new host
        if (wasHost && !room->isEmpty())
        {
            PlayerId newHostId = room->getHostId();
            std::cout << "[server] Notifying room " << room->getId() << " about new host: " << static_cast<int>(newHostId) << "\n";
            
            net::HostChanged hostChange{};
            hostChange.roomId = room->getId();
            hostChange.newHostId = newHostId;
            auto hostPacket = net::serializeHostChanged(hostChange, _sequence++, nowMilliseconds());
            
            for (const auto& [pid, client] : room->getClients())
            {
                flushSends(hostPacket, client.getEndpoint());
            }
        }
    }
    
    net::RoomLeft response{};
    response.roomId = leaveRoom.roomId;
    auto packet = net::serializeRoomLeft(response, _sequence++, nowMilliseconds());
    flushSends(packet, *_playerEndpoints[playerId]);
    
    std::cout << "[server] Player " << static_cast<int>(playerId) << " left room " << leaveRoom.roomId << "\n";
}

void GameServer::handleStartGame(const net::StartGame &startGame, const std::string& endpointKey)
{
    std::cout << "[server] Received StartGame request for room " << startGame.roomId 
              << " from endpoint " << endpointKey << "\n";
    
    auto it = _endpointToPlayer.find(endpointKey);
    if (it == _endpointToPlayer.end())
        return;
    
    PlayerId playerId = it->second;
    std::cout << "[server] Player " << static_cast<int>(playerId) << " wants to start room " << startGame.roomId << "\n";
    
    auto room = _roomManager->getRoom(startGame.roomId);
    
    if (!room)
        return;
    
    if (room->getHostId() != playerId)
    {
        net::RoomError error{};
        error.errorCode = 3;
        std::strncpy(error.message, "Only host can start game", 63);
        error.message[63] = '\0';
        auto packet = net::serializeRoomError(error, _sequence++, nowMilliseconds());
        flushSends(packet, *_playerEndpoints[playerId]);
        return;
    }
    
    room->startGame();
    
    net::GameStarted response{};
    response.roomId = startGame.roomId;
    auto packet = net::serializeGameStarted(response, _sequence++, nowMilliseconds());
    
    std::cout << "[server] Sending GameStarted to " << room->getClients().size() << " clients in room " << startGame.roomId << ":\n";
    for (auto& [pid, client] : room->getClients())
    {
        std::cout << "[server]   - Player " << static_cast<int>(pid) << " at " << client.getEndpoint().toString() << "\n";
        flushSends(packet, client.getEndpoint());
    }
    
    std::cout << "[server] Game started in room " << startGame.roomId << "\n";
}

void GameServer::handleRoomList(std::unique_ptr<network::IEndpoint> sender)
{
    auto rooms = _roomManager->listRooms();
    
    net::RoomListResponse response{};
    response.roomCount = static_cast<std::uint8_t>(std::min(rooms.size(), size_t(16)));
    
    for (size_t i = 0; i < response.roomCount; ++i)
    {
        auto& entry = response.rooms[i];
        entry.roomId = rooms[i].roomId;
        std::strncpy(entry.roomName, rooms[i].roomName.c_str(), 31);
        entry.roomName[31] = '\0';
        entry.hostId = rooms[i].hostId;
        entry.playerCount = static_cast<std::uint8_t>(rooms[i].playerCount);
        entry.maxPlayers = static_cast<std::uint8_t>(rooms[i].maxPlayers);
        entry.state = static_cast<std::uint8_t>(rooms[i].state);
    }
    
    auto packet = net::serializeRoomListResponse(response, _sequence++, nowMilliseconds());
    flushSends(packet, *sender);
    
    std::cout << "[server] Sent room list with " << static_cast<int>(response.roomCount) << " rooms\n";
}

void GameServer::handlePlayerInput(const net::PlayerInput &input, const std::string& endpointKey)
{
    auto it = _endpointToPlayer.find(endpointKey);
    if (it == _endpointToPlayer.end())
    {
        std::cout << "[server] PlayerInput from unknown endpoint: " << endpointKey << "\n";
        return;
    }
    
    PlayerId playerId = it->second;
    auto room = _roomManager->getRoomByPlayer(playerId);
    
    if (!room)
    {
        std::cout << "[server] Player " << static_cast<int>(playerId) << " not in any room\n";
        return;
    }
    
    auto& clients = room->getClients();
    auto clientIt = clients.find(playerId);
    if (clientIt == clients.end())
    {
        std::cout << "[server] Player " << static_cast<int>(playerId) << " not in room clients\n";
        return;
    }
    
    clientIt->second.updateLastSeen(nowMilliseconds());
    const EntityId entity = clientIt->second.getEntityId();

    // _gameLogicHandler.manageInputs(translateNetworkInput(input), entity);
    room->getGameLogic().manageInputs(translateNetworkInput(input), entity);
}

void GameServer::handleDisconnect(const net::DisconnectNotice &notice, const std::string& endpointKey)
{
    auto it = _endpointToPlayer.find(endpointKey);
    if (it == _endpointToPlayer.end())
        return;
    
    PlayerId playerId = it->second;
    auto room = _roomManager->getRoomByPlayer(playerId);
    
    if (room)
    {
        auto disconnectPacket = net::serializeDisconnect(notice, _sequence++, nowMilliseconds());
        for (auto &[id, client] : room->getClients())
        {
            if (id != playerId)
                flushSends(disconnectPacket, client.getEndpoint());
        }
        
        _roomManager->leaveRoom(playerId);
    }
    
    _playerEndpoints.erase(playerId);
    _endpointToPlayer.erase(endpointKey);
    
    std::cout << "[server] Player " << static_cast<int>(playerId) << " disconnected\n";
}

void GameServer::updateGameLoop()
{
    auto previous = std::chrono::steady_clock::now();

    while (_running.load())
    {
        auto now = std::chrono::steady_clock::now();
        const std::chrono::duration<float> delta = now - previous;
        previous = now;
        const float dt = delta.count();

        {
            std::vector<PendingPacket> queueCopy;
            {
                std::lock_guard lock(_rxMutex);
                queueCopy.swap(_rxQueue);
            }
            for (auto &pending : queueCopy)
                handlePacket(pending.data.data(), pending.data.size(), std::move(pending.sender));
        }

        _roomManager->updateAllRooms(dt);
        _roomManager->cleanupEmptyRooms();
        
        // Check for client timeouts
        checkClientTimeouts();

        const auto timestamp = nowMilliseconds();
        broadcastRoomStates(timestamp);

        const auto frameEnd = std::chrono::steady_clock::now();
        const float frameElapsed = std::chrono::duration<float>(frameEnd - now).count();
        constexpr float kTargetDelta = 1.0f / 60.0f;
        if (frameElapsed < kTargetDelta)
            std::this_thread::sleep_for(std::chrono::duration<float>(kTargetDelta - frameElapsed));
    }
}

void GameServer::broadcastRoomStates(Timestamp timestamp)
{
    auto rooms = _roomManager->listRooms();
    
    for (const auto& roomInfo : rooms)
    {
        auto room = _roomManager->getRoom(roomInfo.roomId);
        if (!room || room->getState() != RoomState::Playing)
            continue;
        
        const auto& registry = room->getGameLogic().getRegistry();
        const auto& toDestroy = room->getGameLogic().getEntityDestructionSet();
        const auto& clients = room->getClients();

        // Debug output every 60 frames per room (using timestamp instead of static counter)
        if ((timestamp / 16) % 60 == 0 && roomInfo.roomId == 1)  // Only room 1 for less spam
        {
            std::cout << "[server] Room " << roomInfo.roomId << " has " << clients.size() << " clients\n";
        }
        
        // Check for player deaths
        room->checkPlayerDeaths();
        
        if (room->getGameLogic().hasLevelChanged())
        {
            net::LevelBegin levelBegin{};
            levelBegin.levelNumber = static_cast<std::uint8_t>(room->getGameLogic().getCurrentLevel());
            auto levelPacket = net::serializeLevelBegin(levelBegin, _sequence++, timestamp);
            
            for (const auto& [playerId, client] : clients)
            {
                flushSends(levelPacket, client.getEndpoint());
            }
        }
        
        registry.each<PlayerComponent>([&](EntityId id, const PlayerComponent &player) {
            const auto *transform = registry.get<Transform>(id);
            const auto *health = registry.get<Health>(id);
            const auto *player_power_up = registry.get<PlayerPowerUpStatus>(id);
            if (!transform || !health)
                return;
            net::PlayerState state{};
            state.player = player.id;
            state.x = transform->x;
            state.y = transform->y;
            state.hp = health->hp;
            state.score = 0;
            state.alive = health->alive;
            state.powerUpType = player_power_up->type;
            auto packet = net::serializePlayerState(state, _sequence++, timestamp);
            for (const auto& [pid, client] : clients)
            {
                // Removed debug print to reduce spam with multiple rooms
                flushSends(packet, client.getEndpoint());
            }
            
            // Send PlayerDeath notification if player just died
            if (!health->alive)
            {
                net::PlayerDeath death{};
                death.player = player.id;
                auto deathPacket = net::serializePlayerDeath(death, _sequence++, timestamp);
                for (const auto& [pid, client] : clients)
                {
                    flushSends(deathPacket, client.getEndpoint());
                }
            }
        });
        
        // Check if all players are dead and notify
        if (room->areAllPlayersDead() && !room->hasNotifiedAllDead())
        {
            std::cout << "[server] All players dead in room " << roomInfo.roomId << "\n";
            room->setAllPlayersDeadNotified(true);
            
            net::AllPlayersDead allDead{};
            allDead.roomId = roomInfo.roomId;
            auto packet = net::serializeAllPlayersDead(allDead, _sequence++, timestamp);
            
            for (const auto& [pid, client] : clients)
            {
                flushSends(packet, client.getEndpoint());
            }
        }

        registry.each<MonsterComponent>([&](EntityId id, const MonsterComponent &monster) {
            const auto *transform = registry.get<const Transform>(id);
            const auto *velocity = registry.get<Velocity>(id);
            const auto *health = registry.get<Health>(id);
            if (!transform || !health)
                return;
            const bool marked = toDestroy.count(id) > 0;
            net::MonsterState state{};
            state.id = id;
            state.type = monster.type;
            state.x = transform->x;
            state.y = transform->y;
            state.vx = velocity ? velocity->vx : 0.0f;
            state.vy = velocity ? velocity->vy : 0.0f;
            state.alive = health->alive && !marked;
            auto packet = net::serializeMonsterState(state, _sequence++, timestamp);
            for (const auto& [pid, client] : clients)
                flushSends(packet, client.getEndpoint());
        });

        registry.each<ShieldComponent>([&](EntityId id, const ShieldComponent &shield) {
            const auto *transform = registry.get<const Transform>(id);
            const auto *velocity = registry.get<Velocity>(id);
            const auto *health = registry.get<Health>(id);
            if (!transform || !health)
                return;
            const bool marked = toDestroy.count(id) > 0;
            
            // Get shield type from parent monster
            std::uint8_t shieldType = 0;
            const auto *parentMonster = registry.get<MonsterComponent>(shield.parentMonster);
            if (parentMonster) {
                shieldType = parentMonster->type;
            }
            
            net::ShieldState state{};
            state.id = id;
            state.type = shieldType;
            state.x = transform->x;
            state.y = transform->y;
            state.vx = velocity ? velocity->vx : 0.0f;
            state.vy = velocity ? velocity->vy : 0.0f;
            state.alive = health->alive && !marked;
            auto packet = net::serializeShieldState(state, _sequence++, timestamp);
            for (const auto& [pid, client] : clients)
                flushSends(packet, client.getEndpoint());
        });

        registry.each<PowerUp>([&](EntityId id, const PowerUp &powerup) {
            const auto *transform = registry.get<Transform>(id);
            if (!transform)
                return;
            const bool marked = toDestroy.count(id) > 0;
            net::PowerUpState state{};
            state.id = id;
            state.type = powerup.type;
            state.value = powerup.value;
            state.x = transform->x;
            state.y = transform->y;
            state.active = !marked;
            auto packet = net::serializePowerUpState(state, _sequence++, timestamp);
            for (const auto& [pid, client] : clients)
                flushSends(packet, client.getEndpoint());
        });

        registry.each<Projectile>([&](EntityId id, [[maybe_unused]] const Projectile &projectile) {
            const auto *transform = registry.get<Transform>(id);
            if (!transform)
                return;
            const bool marked = toDestroy.count(id) > 0;
            net::BulletState bullet{};
            bullet.id = id;
            bullet.x = transform->x;
            bullet.y = transform->y;
            bullet.weaponType = static_cast<uint8_t>(projectile.weaponType);
            bullet.fromPlayer = projectile.fromPlayer;
            bullet.active = !marked;
            bullet.fromPlayer = projectile.fromPlayer;
            auto packet = net::serializeBulletState(bullet, _sequence++, timestamp);
            for (const auto& [pid, client] : clients)
                flushSends(packet, client.getEndpoint());
        });
        room->getGameLogic().destroyEntityDestructionList();
    }
}

void GameServer::flushSends(const std::vector<std::uint8_t> &data, const network::IEndpoint &target)
{
    _socket->sendTo(data, target);
}

Timestamp GameServer::nowMilliseconds() const
{
    using clock = std::chrono::steady_clock;
    const auto now = clock::now().time_since_epoch();
    return static_cast<Timestamp>(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
}

void GameServer::handleSpectatorMode(const net::SpectatorMode &spec, const std::string& endpointKey)
{
    auto it = _endpointToPlayer.find(endpointKey);
    if (it == _endpointToPlayer.end())
        return;
    
    PlayerId playerId = it->second;
    std::cout << "[server] Player " << static_cast<int>(playerId) 
              << (spec.enabled ? " enabled" : " disabled") << " spectator mode\n";
    
    // For now, spectator mode is just a client-side visual change
    // The dead player will continue to receive game state updates
    // but won't be able to control anything (already handled by manageInputs check)
}

void GameServer::checkClientTimeouts()
{
    const Timestamp now = nowMilliseconds();
    const Timestamp timeoutMs = static_cast<Timestamp>(_config.network.clientTimeout * 1000.0f);
    
    std::vector<PlayerId> timedOutPlayers;
    
    // Check all rooms for timed out clients
    auto rooms = _roomManager->listRooms();
    for (const auto& roomInfo : rooms)
    {
        auto room = _roomManager->getRoom(roomInfo.roomId);
        if (!room)
            continue;
        
        auto& clients = room->getClients();
        for (const auto& [playerId, client] : clients)
        {
            Timestamp lastSeen = client.getLastSeen();
            if (now - lastSeen > timeoutMs)
            {
                std::cout << "[server] Client " << static_cast<int>(playerId) 
                          << " timed out (no activity for " 
                          << (now - lastSeen) / 1000.0f << "s)\n";
                
                // Send disconnect notification to client (best effort)
                net::DisconnectNotice notice{};
                notice.player = playerId;
                auto packet = net::serializeDisconnect(notice, _sequence++, now);
                flushSends(packet, client.getEndpoint());
                
                timedOutPlayers.push_back(playerId);
            }
        }
    }
    
    // Remove timed out players from their rooms
    for (PlayerId playerId : timedOutPlayers)
    {
        // Find which room the player is in
        for (const auto& roomInfo : rooms)
        {
            auto room = _roomManager->getRoom(roomInfo.roomId);
            if (room && room->hasPlayer(playerId))
            {
                room->removePlayer(playerId);
                std::cout << "[server] Removed timed out player " 
                          << static_cast<int>(playerId) << " from room " 
                          << roomInfo.roomId << "\n";
                break;
            }
        }
    }
}

}
