# Developer Guide

> **Related Documentation:**
> - [Building and Running](building_and_running.md) — Setup instructions for all platforms
> - [Protocol](protocol.md) — Network protocol specification
> - [Architecture](architecture.md) — System design and data flow
> - [API Reference](api_reference.md) — Complete API documentation
> - [Configuration Guide](configuration_guide.md) — Runtime settings and parameters

This guide helps developers understand the R-Type codebase and contribute effectively to the project.

## Table of Contents
1. [Getting Started](#getting-started)
2. [Project Architecture](#project-architecture-overview)
3. [Core Concepts](#core-concepts)
4. [Adding New Features](#adding-new-features)
5. [Common Development Tasks](#common-development-tasks)
6. [Testing](#testing)
7. [Platform-Specific Considerations](#platform-specific-considerations)
8. [Contribution Workflow](#contribution-workflow)

## Getting Started

### Prerequisites
Before contributing, ensure you have:
- Read the [README](../README.md) and [building_and_running.md](building_and_running.md)
- Successfully built the project on your platform
- Played the game to understand the mechanics
- Reviewed the [architecture.md](architecture.md) for system overview

### Development Environment Setup

#### Recommended Tools
- **IDE**: VS Code, CLion, or Visual Studio 2022
- **Debugger**: GDB (Linux), LLDB (macOS), or MSVC debugger (Windows)
- **Version Control**: Git with a GUI client (optional but helpful)
- **Documentation**: Doxygen (optional, for generating API docs)

#### Workspace Setup
```bash
# Clone and setup
git clone <repo-url>
cd rtype

# Linux/macOS - Install dependencies
sudo apt install libasio-dev libsfml-dev  # Ubuntu/Debian
brew install asio sfml                     # macOS

# Windows - Use Conan
conan install . --build=missing -s build_type=Debug

# Build debug version
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Run tests (if available)
cd build && ctest
```

### Understanding the Codebase

#### Module Structure
| Module | Location | Purpose | Key Files |
|--------|----------|---------|-----------|
| **rtype_engine** | `src/engine/`, `include/rtype/engine/` | ECS core | `Registry.cpp`, `SystemPipeline.cpp` |
| **rtype_common** | `src/common/`, `include/rtype/common/` | Shared code | `Protocol.cpp`, `GameConfig.cpp` |
| **rtype_server** | `src/server/` | Game server | `GameServer.cpp`, `GameLogicHandler.cpp` |
| **rtype_client** | `src/client/` | Game client | `GameClient.cpp`, `SFMLRenderer.cpp` |

#### Code Navigation Tips
- **Components**: All in `include/rtype/common/Components.hpp`
- **Network packets**: Defined in `include/rtype/common/Protocol.hpp`
- **Game systems**: Located in `src/server/systems/`
- **Configuration**: Loaded from `config/` directory via `GameConfig.cpp`

### Running in Debug Mode

```bash
# Server with verbose output
./bin/rtype_server --verbose

# Client with debug rendering
./bin/rtype_client --debug

# With debugger
gdb ./bin/rtype_server
(gdb) break GameServer::update
(gdb) run
```

## Project Architecture Overview

R-Type is built with a modular, component-based architecture:

```
rtype/
├── rtype_common    - Shared types, protocol, configuration
├── rtype_engine    - ECS (Entity Component System) core
├── rtype_server    - Authoritative game server
└── rtype_client    - SFML-based game client
```

## Core Concepts

### Entity Component System (ECS)

The game uses a minimal ECS implementation in `rtype::engine::Registry`.

**Components** are plain structs holding data:
```cpp
struct Transform {
    float x{0.0f};
    float y{0.0f};
};

struct Velocity {
    float vx{0.0f};
    float vy{0.0f};
};
```

**Entities** are unique IDs (`EntityId` = `std::uint32_t`)

**Systems** are functions that operate on entities with specific components:
```cpp
// Update all entities with Transform and Velocity
_registry.each<Velocity>([&](EntityId id, Velocity &vel) {
    if (auto *transform = _registry.get<Transform>(id)) {
        transform->x += vel.vx * deltaTime;
        transform->y += vel.vy * deltaTime;
    }
});
```

**Note on Iterator Safety**: The `each()` implementation safely handles nested iterations by collecting entity IDs before invoking callbacks. This prevents iterator invalidation when using nested `each()` calls (like in collision detection) or when calling `get<>()` on different component types during iteration.

### Registry API

```cpp
// Create entity
EntityId entity = registry.createEntity();

// Add components
registry.emplace<Transform>(entity, Transform{100.0f, 200.0f});
registry.emplace<Velocity>(entity, Velocity{50.0f, 0.0f});

// Get component (returns pointer, nullptr if not present)
Transform *transform = registry.get<Transform>(entity);

// Check if entity has component
bool hasHealth = registry.has<Health>(entity);

// Iterate all entities with component
registry.each<MonsterComponent>([](EntityId id, MonsterComponent &monster) {
    // Process each monster
});

// Destroy entity (removes all components)
registry.destroyEntity(entity);
```

## Adding New Components

### 1. Define the Component
Edit `include/rtype/common/Components.hpp`:
```cpp
struct ShieldComponent {
    float strength{100.0f};
    float regenRate{5.0f};
    bool active{true};
};
```

### 2. Use in Server Logic
In `GameLogicHandler`:
```cpp
// Spawn entity with shield
EntityId player = _registry.createEntity();
_registry.emplace<Transform>(player, Transform{x, y});
_registry.emplace<ShieldComponent>(player, ShieldComponent{});

// Update shields
void GameLogicHandler::updateShields(float dt) {
    _registry.each<ShieldComponent>([&](EntityId id, ShieldComponent &shield) {
        if (shield.active && shield.strength < 100.0f) {
            shield.strength += shield.regenRate * dt;
            shield.strength = std::min(shield.strength, 100.0f);
        }
    });
}
```

### 3. Serialize for Network (if needed)
If clients need to know about this component, add to protocol:

`include/rtype/common/Protocol.hpp`:
```cpp
struct ShieldState {
    EntityId id{};
    float strength{};
    bool active{};
};
```

`src/common/protocol/Protocol.cpp`:
```cpp
std::vector<std::uint8_t> serializeShieldState(
    const ShieldState &state, 
    SequenceNumber seq, 
    Timestamp ts) 
{
    BinaryWriter writer;
    writer.writeU32(state.id);
    writer.writeF32(state.strength);
    writer.writeU8(state.active ? 1 : 0);
    return serializePacket(PacketType::ShieldState, seq, ts, 
                          writer.data().data(), writer.data().size());
}
```

## Adding New Packet Types

### 1. Define Packet Type
`include/rtype/common/Protocol.hpp`:
```cpp
enum class PacketType : std::uint16_t {
    // ... existing types
    WeatherChange = 14,
};

struct WeatherChange {
    std::uint8_t weatherType{};  // 0=clear, 1=rain, 2=storm
    float intensity{1.0f};
};
```

### 2. Implement Serialization
`src/common/protocol/Protocol.cpp`:
```cpp
std::vector<std::uint8_t> serializeWeatherChange(
    const WeatherChange &weather,
    SequenceNumber sequence,
    Timestamp timestamp)
{
    BinaryWriter writer;
    writer.writeU8(weather.weatherType);
    writer.writeF32(weather.intensity);
    return serializePacket(PacketType::WeatherChange, sequence, timestamp,
                          writer.data().data(), writer.data().size());
}

bool deserializeWeatherChange(const std::uint8_t* payload, std::size_t size,
                              WeatherChange &out)
{
    BinaryReader reader(payload, size);
    return reader.readU8(out.weatherType) &&
           reader.readF32(out.intensity);
}
```

### 3. Send from Server
`src/server/GameServer.cpp`:
```cpp
// In updateGameLoop or wherever appropriate
if (shouldChangeWeather) {
    net::WeatherChange weather{};
    weather.weatherType = 2;  // Storm
    weather.intensity = 0.8f;
    
    auto packet = net::serializeWeatherChange(weather, _sequence++, timestamp);
    
    for (auto &[playerId, client] : _clients) {
        flushSends(packet, client.getEndpoint());
    }
}
```

### 4. Handle in Client
`src/client/GameClient.cpp`:
```cpp
void GameClient::handlePacket(const std::uint8_t* data, std::size_t size) {
    // ... existing handling
    
    if (header.type == net::PacketType::WeatherChange) {
        net::WeatherChange weather{};
        if (net::deserializeWeatherChange(payload.data(), payload.size(), weather)) {
            std::lock_guard lock(_mutex);
            _currentWeather = weather;
        }
    }
}
```

## Server Architecture

### Threading Model

**Network Thread** (`_networkThread`):
- Runs `asio::io_context`
- Receives UDP packets asynchronously
- Enqueues packets into `_rxQueue` (mutex-protected)
- Never touches game state directly

**Game Thread** (`_gameThread`):
- Runs at fixed 60 FPS
- Dequeues and processes packets
- Updates game logic via `GameLogicHandler`
- Broadcasts state updates
- Only thread that modifies `_registry`

### Game Loop Structure
```cpp
void GameServer::updateGameLoop() {
    while (_running) {
        // 1. Process input packets
        processPacketQueue();
        
        // 2. Update game state
        _gameLogicHandler.updateGame(deltaTime);
        
        // 3. Check for level changes
        if (_gameLogicHandler.hasLevelChanged()) {
            broadcastLevelBegin();
        }
        
        // 4. Broadcast state
        broadcastStates(timestamp);
        
        // 5. Clean up destroyed entities
        _gameLogicHandler.destroyEntityDestructionList();
        
        // 6. Sleep to maintain 60 FPS
        sleepUntilNextFrame();
    }
}
```

### Adding Server Features

To add a new gameplay mechanic:

1. **Add to GameLogicHandler** (`src/server/GameLogicHandler.cpp`):
```cpp
void GameLogicHandler::updateBossMechanics(float dt) {
    _registry.each<BossComponent>([&](EntityId id, BossComponent &boss) {
        boss.phaseTimer += dt;
        
        if (boss.phaseTimer >= boss.phaseDuration) {
            boss.currentPhase++;
            boss.phaseTimer = 0.0f;
            
            // Change attack pattern based on phase
            switch (boss.currentPhase) {
                case 1: setupPhase1Attacks(id); break;
                case 2: setupPhase2Attacks(id); break;
                // etc.
            }
        }
    });
}
```

2. **Call from updateGame**:
```cpp
void GameLogicHandler::updateGame(float dt) {
    // ... existing updates
    updateBossMechanics(dt);
    // ... rest
}
```

## Client Architecture

### Main Loop Structure
```cpp
while (window.isOpen()) {
    // 1. Handle SFML events
    handleEvents();
    
    // 2. Send player input
    sendInput();
    
    // 3. Render frame
    renderFrame();
}
```

### Rendering System

`RemoteDisplay` manages all remote entities received from server:
```cpp
class RemoteDisplay {
    std::unordered_map<EntityId, RemotePlayer> _players;
    std::unordered_map<EntityId, RemoteMonster> _monsters;
    std::unordered_map<EntityId, RemoteBullet> _bullets;
    std::unordered_map<EntityId, RemotePowerUp> _powerUps;
};
```

`SFMLRender` converts entities to SFML drawables:
```cpp
void SFMLRender::render(sf::RenderWindow &window, 
                       const RemoteDisplay &display,
                       const GameConfig &config);
```

### Adding Client-Side Effects

For effects that don't need server synchronization:

```cpp
class ParticleSystem {
public:
    void spawnExplosion(float x, float y) {
        for (int i = 0; i < 20; i++) {
            Particle p;
            p.x = x;
            p.y = y;
            // Random velocity
            p.vx = randomFloat(-100, 100);
            p.vy = randomFloat(-100, 100);
            p.lifetime = 0.5f;
            _particles.push_back(p);
        }
    }
    
    void update(float dt) {
        for (auto &p : _particles) {
            p.x += p.vx * dt;
            p.y += p.vy * dt;
            p.lifetime -= dt;
        }
        // Remove dead particles
        _particles.erase(
            std::remove_if(_particles.begin(), _particles.end(),
                          [](const Particle &p) { return p.lifetime <= 0; }),
            _particles.end());
    }
    
    void draw(sf::RenderWindow &window) {
        for (const auto &p : _particles) {
            sf::CircleShape shape(2.0f);
            shape.setPosition(p.x, p.y);
            shape.setFillColor(sf::Color::Yellow);
            window.draw(shape);
        }
    }
    
private:
    struct Particle {
        float x, y, vx, vy, lifetime;
    };
    std::vector<Particle> _particles;
};
```

## Configuration System

### Adding Configuration Options

1. **Define in GameConfig.hpp**:
```cpp
struct GameplayConfig {
    // ... existing fields
    float bossSpawnInterval{120.0f};
    std::uint8_t maxBossesPerLevel{1};
};
```

2. **Parse in GameConfig.cpp**:
```cpp
if (section == "Gameplay") {
    if (key == "BossSpawnInterval")
        gameplay.bossSpawnInterval = std::stof(value);
    else if (key == "MaxBossesPerLevel")
        gameplay.maxBossesPerLevel = static_cast<std::uint8_t>(std::stoi(value));
}
```

### Enemy Explosion Animation

- The client now spawns an explosion animation when the server reports a monster death (`MonsterState.alive == false`).
- Frames are read from the second-to-last row of `src/assets/sprites/r-typesheet1.gif` and scaled up for readability.
- The renderer drains pending explosion events from `RemoteDisplay::consumeExplosionEvents()` each frame and advances the sprite-sheet animation on the client only (no extra network traffic).

3. **Add to config/game.ini**:
```ini
[Gameplay]
BossSpawnInterval=120.0
MaxBossesPerLevel=1
```

4. **Use in code**:
```cpp
if (_bossSpawnTimer >= _config.gameplay.bossSpawnInterval) {
    if (countBosses() < _config.gameplay.maxBossesPerLevel) {
        spawnBoss();
    }
}
```

## Best Practices

### Performance
- Keep component data simple (POD types when possible)
- Avoid allocations in hot loops
- Use `each()` for batch processing, not individual `get()` calls
- Profile with `perf` or `valgrind --tool=callgrind`

### Network
- Keep packets small (< 1KB)
- Batch related updates into single packets when possible
- Use delta compression for frequently updated values
- Validate all deserialized data

### Code Organization
- Components in `common/Components.hpp` (shared)
- Protocol in `common/Protocol.hpp` and `Protocol.cpp`
- Server logic in `server/GameLogicHandler.cpp`
- Client rendering in `client/SFMLRender.cpp`
- Configuration in `common/GameConfig.hpp` and `.cpp`

### Error Handling
- Server: log errors but continue running
- Client: reconnect logic for disconnect
- Network: silently drop malformed packets
- Config: use defaults if file missing or values invalid

## Testing

### Unit Testing Components
```cpp
TEST(RegistryTest, CreateAndDestroyEntity) {
    engine::Registry registry;
    EntityId e = registry.createEntity();
    EXPECT_NE(e, 0);
    registry.destroyEntity(e);
}

TEST(RegistryTest, ComponentLifecycle) {
    engine::Registry registry;
    EntityId e = registry.createEntity();
    
    registry.emplace<Transform>(e, Transform{10.0f, 20.0f});
    EXPECT_TRUE(registry.has<Transform>(e));
    
    auto *t = registry.get<Transform>(e);
    ASSERT_NE(t, nullptr);
    EXPECT_FLOAT_EQ(t->x, 10.0f);
    
    registry.destroyEntity(e);
    EXPECT_FALSE(registry.has<Transform>(e));
}
```

### Integration Testing
```bash
# Start server
./rtype_server &
SERVER_PID=$!

# Give it time to start
sleep 2

# Run client test
./rtype_client --test-mode

# Cleanup
kill $SERVER_PID
```

## Common Development Tasks

### Adding a New Enemy Type

1. **Define the enemy type constant** in `include/rtype/common/Components.hpp`:
```cpp
namespace MonsterTypes {
    constexpr std::uint8_t BASIC = 0;
    constexpr std::uint8_t FAST = 1;
    constexpr std::uint8_t TANK = 2;
    constexpr std::uint8_t BOSS = 3;
    constexpr std::uint8_t YOUR_NEW_TYPE = 4;  // Add here
}
```

2. **Add spawn logic** in `src/server/EntityFactory.cpp`:
```cpp
EntityId EntityFactory::spawnMonster(std::uint8_t type, float x, float y) {
    // ... existing code
    
    if (type == MonsterTypes::YOUR_NEW_TYPE) {
        monster.type = type;
        monster.hp = 50;  // Custom health
        monster.scoreValue = 500;
        // ... set other properties
    }
}
```

3. **Implement behavior** in `src/server/systems/` (create new system or extend existing):
```cpp
// src/server/systems/YourNewTypeSystem.cpp
void YourNewTypeSystem::update(float dt, Registry& registry) {
    registry.each<MonsterComponent, Transform, Velocity>(
        [&](EntityId id, MonsterComponent& monster, Transform& t, Velocity& v) {
            if (monster.type == MonsterTypes::YOUR_NEW_TYPE) {
                // Custom behavior logic
            }
        });
}
```

4. **Add visual assets** for the client in `src/assets/textures/`:
   - Create sprite file: `enemy_your_type.png`
   - Update `SpriteManager.cpp` to load it

5. **Update client rendering** in `src/client/SFMLRenderer.cpp`:
```cpp
void SFMLRenderer::renderMonster(const MonsterState& monster) {
    Sprite* sprite = nullptr;
    if (monster.type == MonsterTypes::YOUR_NEW_TYPE) {
        sprite = _spriteManager.getSprite("enemy_your_type");
    }
    // ... render sprite
}
```

### Adding a New Configuration Parameter

1. **Add to config struct** in `include/rtype/common/GameConfig.hpp`:
```cpp
struct GameplayConfig {
    // ... existing fields
    float yourNewParameter{42.0f};  // Add with default
};
```

2. **Add parsing** in `src/common/config/GameConfig.cpp`:
```cpp
void GameConfig::loadFromFile(const std::string& path) {
    // ... existing code
    
    if (section == "Gameplay") {
        if (key == "YourNewParameter") {
            gameplay.yourNewParameter = std::stof(value);
        }
    }
}
```

3. **Add to config file** `config/game.ini`:
```ini
[Gameplay]
YourNewParameter=42.0
```

4. **Use in your code**:
```cpp
float value = _config.gameplay.yourNewParameter;
```

### Debugging Network Issues

#### Enable Packet Logging
Add to server or client:
```cpp
void logPacket(const PacketHeader& header, const std::string& direction) {
    std::cout << "[" << direction << "] Type: " << static_cast<int>(header.type)
              << " Size: " << header.payloadSize
              << " Seq: " << header.sequence
              << " Time: " << header.timestamp << std::endl;
}
```

#### Wireshark Filter for R-Type Traffic
```
udp.port == 12345  # Your server port
```

#### Common Issues
| Symptom | Likely Cause | Solution |
|---------|--------------|----------|
| Entities not appearing | Missed spawn packet | Add packet loss detection |
| Position jumps | Packet reordering | Check sequence numbers |
| Client disconnects | Server not responding | Check firewall/NAT |
| Deserialization fails | Endianness mismatch | Use htons/ntohs correctly |

### Profiling Performance

#### Linux
```bash
# Compile with debug symbols
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo

# Profile with perf
perf record -g ./bin/rtype_server
perf report

# Or use valgrind
valgrind --tool=callgrind ./bin/rtype_server
kcachegrind callgrind.out.*
```

#### Windows
Use Visual Studio Performance Profiler:
1. Debug → Performance Profiler
2. Select "CPU Usage"
3. Start profiling
4. Let game run for 30-60 seconds
5. Stop and analyze hotspots

## Platform-Specific Considerations

### Windows (MSVC) Compatibility

#### Known Issues and Solutions

**1. std::uniform_int_distribution with uint8_t**
```cpp
// ❌ WRONG - Doesn't compile on MSVC
std::uniform_int_distribution<uint8_t> dist(0, 10);

// ✅ CORRECT - Use int and cast
std::uniform_int_distribution<int> dist(0, 10);
uint8_t value = static_cast<uint8_t>(dist(rng));
```

**Reason**: MSVC strictly follows C++11 standard which only allows `short`, `int`, `long`, `long long` and their unsigned variants for `uniform_int_distribution`.

**2. WIN32_WINNT Warnings**
If you see warnings about `_WIN32_WINNT`, add to `CMakeLists.txt`:
```cmake
if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0601)  # Windows 7 target
endif()
```

**3. Network Header Order**
On Windows with Asio, include order matters:
```cpp
// ✅ CORRECT order
#include <asio.hpp>
// ... other includes
```

### Linux-Specific

#### Socket Limits
```bash
# Increase UDP buffer size if dropping packets
sudo sysctl -w net.core.rmem_max=26214400
sudo sysctl -w net.core.wmem_max=26214400
```

### macOS-Specific

#### Homebrew Libraries
```bash
# Link to Homebrew SFML
export PKG_CONFIG_PATH="/opt/homebrew/lib/pkgconfig:$PKG_CONFIG_PATH"
```

### Cross-Platform Best Practices

**1. Use portable types**
```cpp
// ✅ Use these
#include <cstdint>
std::uint8_t, std::uint16_t, std::uint32_t
float  // IEEE 754 is portable

// ❌ Avoid platform-specific
unsigned char  // size varies
long  // 32-bit on Windows, 64-bit on Linux
```

**2. Filesystem paths**
```cpp
// ✅ Use std::filesystem (C++17)
#include <filesystem>
auto path = std::filesystem::path("config") / "game.ini";

// ❌ Avoid hardcoded separators
auto path = "config\\game.ini";  // Windows only
```

**3. Endianness**
```cpp
// ✅ Always use network byte order for network code
#include <arpa/inet.h>  // htons, htonl, ntohs, ntohl

uint16_t hostValue = 1234;
uint16_t networkValue = htons(hostValue);  // Portable
```

## Contribution Workflow

### Step 1: Fork and Clone
```bash
# Fork on GitHub, then clone
git clone https://github.com/YOUR_USERNAME/rtype.git
cd rtype
git remote add upstream https://github.com/ORIGINAL/rtype.git
```

### Step 2: Create Feature Branch
```bash
git checkout -b feature/your-feature-name
# or
git checkout -b fix/bug-description
```

### Step 3: Make Changes
- Write clear, documented code
- Follow existing code style
- Add comments for complex logic
- Update relevant documentation

### Step 4: Test Thoroughly
```bash
# Build on all supported platforms (if possible)
cmake --build build

# Run server and client
./bin/rtype_server &
./bin/rtype_client

# Test edge cases
# - Multiple clients
# - Packet loss simulation
# - High latency conditions
```

### Step 5: Commit with Clear Messages
```bash
git add <files>
git commit -m "feat: add new enemy type with shield behavior

- Implemented ShieldedEnemy with regenerating shields
- Added shield visual indicators
- Updated protocol to sync shield state
- Closes #123"
```

**Commit Message Format**:
- `feat:` - New feature
- `fix:` - Bug fix
- `docs:` - Documentation only
- `refactor:` - Code refactoring
- `perf:` - Performance improvement
- `test:` - Adding tests
- `build:` - Build system changes

### Step 6: Push and Create Pull Request
```bash
git push origin feature/your-feature-name
```

Then on GitHub:
1. Click "Compare & pull request"
2. Fill in description with:
   - What changed
   - Why it changed
   - How to test
   - Screenshots/videos if applicable
3. Link related issues
4. Request review

### Step 7: Address Review Feedback
```bash
# Make requested changes
git add <files>
git commit -m "refactor: address review comments"
git push origin feature/your-feature-name
```

### Keeping Your Fork Updated
```bash
git fetch upstream
git checkout main
git merge upstream/main
git push origin main
```

## Code Style Guidelines

### Naming Conventions
- **Classes/Structs**: `PascalCase` (e.g., `GameServer`, `PlayerState`)
- **Functions/Methods**: `camelCase` (e.g., `updatePosition`, `spawnEntity`)
- **Variables**: `camelCase` (e.g., `playerHealth`, `enemyCount`)
- **Constants**: `UPPER_SNAKE_CASE` (e.g., `MAX_PLAYERS`, `DEFAULT_PORT`)
- **Private members**: `_camelCase` with underscore prefix (e.g., `_registry`, `_config`)

### Formatting
- **Indentation**: 4 spaces (no tabs)
- **Braces**: K&R style (opening brace on same line)
```cpp
void function() {
    if (condition) {
        // code
    } else {
        // code
    }
}
```
- **Line length**: Aim for 120 characters max
- **Include order**: System headers, then third-party, then project headers

### Documentation
```cpp
/**
 * @brief Spawns a new enemy entity in the game world
 * 
 * Creates an enemy with the specified type at the given position.
 * The enemy will be assigned a unique ID and added to the registry.
 * 
 * @param type Enemy type constant (see MonsterTypes)
 * @param x Initial X position in world coordinates
 * @param y Initial Y position in world coordinates
 * @return EntityId The unique identifier of the spawned enemy
 * 
 * @note This should only be called from the server's game logic thread
 */
EntityId spawnEnemy(uint8_t type, float x, float y);
```

## Getting Help

### Documentation Resources
- [README.md](../README.md) - Project overview
- [architecture.md](architecture.md) - System architecture
- [protocol.md](protocol.md) - Network protocol details
- [api_reference.md](api_reference.md) - Complete API documentation
- [building_and_running.md](building_and_running.md) - Build instructions

### Community
- **GitHub Issues**: Bug reports and feature requests
- **GitHub Discussions**: Questions and general discussion
- **Pull Requests**: Code review and collaboration

### Common Questions

**Q: Where do I start contributing?**
A: Look for issues labeled `good-first-issue` or `help-wanted` on GitHub.

**Q: How do I test network code locally?**
A: Run server and client on localhost. Use `tc` (Linux) or Clumsy (Windows) to simulate packet loss.

**Q: Can I add new dependencies?**
A: Prefer header-only libraries. If adding compiled dependency, ensure it's available via Conan/vcpkg and discuss in issue first.

**Q: How do I debug the ECS?**
A: Add logging in `each()` callbacks. Use debugger watch on `_registry._components` to inspect component storage.

**Q: The build fails on Windows but works on Linux. Help?**
A: See [Platform-Specific Considerations](#platform-specific-considerations). Common issue is MSVC standard library differences.

---

**Last Updated**: January 2026  
**Maintainers**: See [CONTRIBUTORS.md](../CONTRIBUTORS.md)

## Common Development Tasks

### Adding Assets (Fonts, Sprites, Sounds)

When adding new assets to the game, you **must** account for cross-platform path handling. The `SFMLRenderer` class uses the `_fileSeparator` member variable to handle path separators correctly across Windows and Linux.

**Critical**: Always use the `_fileSeparator` variable when constructing file paths in the SFML renderer to ensure compatibility across platforms.

#### Example: Adding a New Font

```cpp
// In SFMLRenderer.cpp constructor
std::string fontPath = "src/assets/font" + _fileSeparator + "MyNewFont.ttf";
if (!_font.loadFromFile(fontPath)) {
    std::cerr << "[renderer] Failed to load font from: " << fontPath << '\n';
}
```

#### Example: Adding Sound Effects

```cpp
std::string soundPath = "src/assets/sound" + _fileSeparator + "explosion.wav";
if (!_explosionBuffer.loadFromFile(soundPath)) {
    std::cerr << "[renderer] Failed to load sound from: " << soundPath << '\n';
}
_explosionSound.setBuffer(_explosionBuffer);
```

**Asset Directory Structure**:
```
src/assets/
├── font/
│   ├── DejaVuSans.ttf
│   └── (add custom fonts here)
├── sprites/
│   ├── player.png
│   ├── enemies.png
│   └── (add sprite assets here)
└── sound/
    ├── shoot.wav
    ├── explosion.wav
    └── (add audio files here)
```

**Best Practices**:
- Always construct paths using `_fileSeparator` (becomes `"/"` on Linux, `"\"` on Windows)
- Use relative paths starting from the project root
- Ensure assets exist in both development and compiled binary directories
- Always add error handling when loading assets (check return values)
- Test path loading on both Windows and Linux before committing

### Adding a New Monster Type
1. Update config with new monster type properties
2. Parse in `GameConfig.cpp`
3. Modify spawn logic in `GameLogicHandler::spawnMonsters()`
4. Update client rendering in `SFMLRender.cpp` for visual differences

### Adding Player Abilities
1. Add component (e.g., `DashAbility`)
2. Handle input in `GameLogicHandler::manageInputs()`
3. Implement logic in update loop
4. Add cooldown/energy system
5. Sync state to clients if visual feedback needed

### Debugging Network Issues
```cpp
// Add detailed logging
std::cout << "[server] Sending packet type=" << static_cast<int>(type)
          << " size=" << data.size() 
          << " to " << endpoint << "\n";

// Verify packet contents
std::cout << "Payload: ";
for (auto byte : data) {
    printf("%02x ", byte);
}
std::cout << "\n";
```

## Further Reading

- [Protocol Documentation](protocol.md)
- [Architecture Overview](architecture.md)
- [Configuration Guide](configuration_guide.md)
- [Building and Running](building_and_running.md)
