# R-Type Game Engine - Complete Architecture Documentation

**Version:** 1.0  
**Date:** January 2026  
**Authors:** EPITECH Project Team

---

## Table of Contents

1. [Introduction](#introduction)
2. [Architectural Overview](#architectural-overview)
3. [Entity-Component-System (ECS) Explained](#entity-component-system-ecs-explained)
4. [Core Modules](#core-modules)
5. [Dependency Abstraction](#dependency-abstraction)
6. [Entity Factory Pattern](#entity-factory-pattern)
7. [Configuration System](#configuration-system)
8. [Network Architecture](#network-architecture)
9. [Game Flow](#game-flow)
10. [How to Extend](#how-to-extend)

---

## Introduction

### What is R-Type?

R-Type is a multiplayer space shooter game built with a custom game engine. The engine is designed following modern software architecture principles:

- **Entity-Component-System (ECS)** for game logic
- **Dependency Abstraction** for library independence
- **Configuration-Driven** for flexibility without recompilation
- **Modular Design** for easy maintenance and extension

### Design Goals

1. **Separation of Concerns**: Data, logic, and presentation are separated
2. **Flexibility**: Change game behavior through configuration files
3. **Maintainability**: Clean, modular code that's easy to understand
4. **Testability**: Components can be tested in isolation
5. **Performance**: Data-oriented design for cache efficiency

---

## Architectural Overview

### High-Level Architecture

```
┌───────────────────────────────────────────────────────────┐
│                      R-Type Game                          │
├───────────────────────────────────────────────────────────┤
│                                                           │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │   CLIENT     │  │   SERVER     │  │   COMMON     │     │
│  │              │  │              │  │              │     │
│  │ ┌──────────┐ │  │ ┌──────────┐ │  │ ┌──────────┐ │     │
│  │ │Rendering │ │  │ │Game Logic│ │  │ │   ECS    │ │     │
│  │ │          │ │  │ │          │ │  │ │  Engine  │ │     │
│  │ │ IRender  │ │  │ │  Logic   │ │  │ │          │ │     │
│  │ │   ↓      │ │  │ │ Handler  │ │  │ │ Registry │ │     │
│  │ │  SFML    │ │  │ │          │ │  │ │ Systems  │ │     │
│  │ └──────────┘ │  │ └──────────┘ │  │ └──────────┘ │     │
│  │              │  │              │  │              │     │
│  │ ┌──────────┐ │  │ ┌──────────┐ │  │ ┌──────────┐ │     │
│  │ │ Network  │ │  │ │ Network  │ │  │ │ Network  │ │     │
│  │ │          │ │  │ │          │ │  │ │          │ │     │
│  │ │INetwork  │ │  │ │INetwork  │ │  │ │INetwork  │ │     │
│  │ │   ↓      │ │  │ │   ↓      │ │  │ │  (Base)  │ │     │
│  │ │  ASIO    │ │  │ │  ASIO    │ │  │ └──────────┘ │     │
│  │ └──────────┘ │  │ └──────────┘ │  │              │     │
│  └──────────────┘  └──────────────┘  │ ┌──────────┐ │     │
│                                      │ │  Config  │ │     │
│                                      │ │          │ │     │
│                                      │ │GameConfig│ │     │
│                                      │ │Protocol  │ │     │
│                                      │ └──────────┘ │     │
│                                      └──────────────┘     │
└───────────────────────────────────────────────────────────┘
```

### Module Dependencies

```
┌────────────────┐
│     Client     │
└────────┬───────┘
         │ uses
         ↓
┌────────────────┐     ┌────────────────┐
│     Common     │────→│     Engine     │
└────────────────┘     └────────────────┘
         ↑
         │ uses
         │
┌────────┴───────┐
│     Server     │
└────────────────┘
```

**Key Principle**: Server and Client never depend on each other, only on Common.

---

## Entity-Component-System (ECS) Explained

### What is ECS?

ECS is an architectural pattern that separates **data** from **logic** and uses **composition** instead of inheritance.

### The Three Pillars

#### 1. **Entities** - Just IDs

```cpp
// An entity is just a number
using EntityId = std::uint32_t;

EntityId player = 42;      // This is an entity!
EntityId monster = 43;     // This is also an entity!
EntityId bullet = 44;      // And this too!
```

**Think of entities as:** Database row IDs. They identify "things" in the game but contain no data themselves.

#### 2. **Components** - Pure Data

```cpp
// Components are simple data structures with NO logic
struct Transform {
    float x;  // Position X
    float y;  // Position Y
};

struct Velocity {
    float vx;  // Speed X
    float vy;  // Speed Y
};

struct Health {
    std::uint8_t hp;      // Health points
    bool alive;           // Is alive?
};

struct PlayerComponent {
    PlayerId id;  // Which player (1-4)
};
```

**Key Rules:**
- ✅ **DO**: Store data (position, health, speed, etc.)
- ❌ **DON'T**: Have methods (no `move()`, `takeDamage()`, etc.)
- ❌ **DON'T**: Reference other entities
- ❌ **DON'T**: Contain logic

**Think of components as:** Database columns. Each component is one piece of information.

#### 3. **Systems** - Pure Logic

```cpp
// Systems contain ALL the game logic
class MovementSystem : public ISystem {
public:
    void update(Registry& registry, float deltaTime) override {
        // Find ALL entities that have both Transform AND Velocity
        auto entities = registry.getEntitiesWithComponents<Transform, Velocity>();
        
        // Update each one
        for (auto entity : entities) {
            auto& transform = registry.getComponent<Transform>(entity);
            auto& velocity = registry.getComponent<Velocity>(entity);
            
            // Logic: move the entity
            transform.x += velocity.vx * deltaTime;
            transform.y += velocity.vy * deltaTime;
        }
    }
};
```

**Key Rules:**
- ✅ **DO**: Contain game logic
- ✅ **DO**: Iterate over entities with specific components
- ✅ **DO**: Modify component data
- ❌ **DON'T**: Store state (systems are stateless)
- ❌ **DON'T**: Know about specific entities

**Think of systems as:** SQL queries that find and update rows.

### The Registry - The Manager

The Registry manages the relationship between entities and components:

```cpp
class Registry {
public:
    // Create a new entity (get a new ID)
    EntityId createEntity();
    
    // Attach a component to an entity
    template<typename Component>
    Component& addComponent(EntityId entity, Component component);
    
    // Get a component from an entity
    template<typename Component>
    Component* get(EntityId entity);
    
    // Check if entity has a component
    template<typename Component>
    bool hasComponent(EntityId entity);
    
    // Find all entities with specific components
    template<typename... Components>
    std::vector<EntityId> getEntitiesWithComponents();
};
```

**Think of the Registry as:** A database that stores which entities have which components.

### ECS Example: Creating a Player

```cpp
// 1. Create entity (get an ID)
EntityId player = registry.createEntity();  // player = 42

// 2. Add components (attach data to the ID)
registry.addComponent<Transform>(player, Transform{100.0f, 200.0f});
registry.addComponent<Velocity>(player, Velocity{0.0f, 0.0f});
registry.addComponent<Health>(player, Health{3, true});
registry.addComponent<PlayerComponent>(player, PlayerComponent{1});

// Now entity 42 has: Transform, Velocity, Health, PlayerComponent
```

### ECS in Action: Game Loop

```cpp
void gameLoop(float deltaTime) {
    // Each system processes entities with specific components
    
    // 1. MovementSystem: Updates position for ALL entities with Transform+Velocity
    //    (players, monsters, bullets, powerups - doesn't care which!)
    movementSystem.update(registry, deltaTime);
    
    // 2. CollisionSystem: Checks collisions for entities with Transform
    collisionSystem.update(registry, deltaTime);
    
    // 3. HealthSystem: Updates health for entities with Health component
    healthSystem.update(registry, deltaTime);
    
    // 4. CleanupSystem: Removes dead entities
    cleanupSystem.update(registry, deltaTime);
}
```

### Why ECS?

#### Traditional OOP Approach (BAD for games):

```cpp
// Every entity type is a class
class Player : public GameObject {
    float x, y;           // Position
    float vx, vy;         // Velocity
    int health;           // Health
    
    void update() {
        x += vx * dt;     // Movement logic
        y += vy * dt;
        checkCollision(); // Collision logic
        updateHealth();   // Health logic
    }
};

class Monster : public GameObject {
    // Duplicate the same position, velocity, health code!
    float x, y;
    float vx, vy;
    int health;
    
    void update() {
        // Duplicate the same movement logic!
        x += vx * dt;
        y += vy * dt;
        // ... more duplication
    }
};
```

**Problems:**
- ❌ Code duplication (movement logic in every class)
- ❌ Rigid hierarchy (hard to add new entity types)
- ❌ Poor cache performance (objects scattered in memory)

#### ECS Approach (GOOD for games):

```cpp
// Components are shared data
Transform playerTransform{100, 200};
Velocity playerVelocity{50, 0};

Transform monsterTransform{500, 300};
Velocity monsterVelocity{-30, 0};

// ONE system handles ALL movement
class MovementSystem {
    void update(Registry& registry, float dt) {
        // Works for players, monsters, bullets - anything with Transform+Velocity!
        for (auto [entity, transform, velocity] : registry.view<Transform, Velocity>()) {
            transform.x += velocity.vx * dt;
            transform.y += velocity.vy * dt;
        }
    }
};
```

**Benefits:**
- ✅ No code duplication (one system handles all entities)
- ✅ Flexible composition (combine components freely)
- ✅ Cache-friendly (components stored contiguously)
- ✅ Easy to extend (add new components/systems)

### ECS Data Flow Diagram

```
Game Loop:
    │
    ├─→ [MovementSystem]
    │       ↓
    │   Find all entities with <Transform, Velocity>
    │       ↓
    │   Entity 42 (Player): Transform{100,200} + Velocity{50,0}
    │   Entity 43 (Monster): Transform{500,300} + Velocity{-30,0}
    │   Entity 44 (Bullet): Transform{120,205} + Velocity{200,0}
    │       ↓
    │   Update all: position += velocity * deltaTime
    │
    ├─→ [CollisionSystem]
    │       ↓
    │   Find all entities with <Transform, Projectile>
    │   Find all entities with <Transform, Health>
    │       ↓
    │   Check if any projectile hits any damageable entity
    │       ↓
    │   Reduce Health.hp if collision detected
    │
    ├─→ [HealthSystem]
    │       ↓
    │   Find all entities with <Health>
    │       ↓
    │   If Health.hp == 0, mark entity for destruction
    │
    └─→ [CleanupSystem]
            ↓
        Find all marked entities
            ↓
        Remove them from registry
```

---

## Core Modules

### 1. ECS Engine (`include/rtype/engine/`)

#### Registry (`Registry.hpp`)

The core of the ECS system. Manages entities and components.

**Key Responsibilities:**
- Create/destroy entities
- Add/remove/get components
- Query entities by component combinations
- Maintain component storage

**Implementation Details:**
```cpp
template<typename Component>
class ComponentStorage {
    std::unordered_map<EntityId, Component> components;
    
    Component& add(EntityId entity, Component comp) {
        return components[entity] = comp;
    }
    
    Component* get(EntityId entity) {
        auto it = components.find(entity);
        return it != components.end() ? &it->second : nullptr;
    }
};
```

**Why this design?**
- Fast component lookup (O(1) hash map)
- Separate storage per component type (cache-friendly)
- Type-safe (templates ensure correct types)

#### System Interface (`ISystem.hpp`)

Base class for all systems.

```cpp
class ISystem {
public:
    virtual ~ISystem() = default;
    virtual void update(Registry& registry, float deltaTime) = 0;
};
```

**Why this design?**
- Polymorphic (can store different systems in one container)
- Simple interface (just one method to implement)
- Flexible (each system decides what to process)

#### System Pipeline (`SystemPipeline.hpp`)

Orchestrates system execution.

```cpp
class SystemPipeline {
    std::vector<std::unique_ptr<ISystem>> systems;
    
public:
    void addSystem(std::unique_ptr<ISystem> system) {
        systems.push_back(std::move(system));
    }
    
    void update(float deltaTime, Registry& registry) {
        for (auto& system : systems) {
            system->update(registry, deltaTime);
        }
    }
};
```

**Why this design?**
- Ordered execution (systems run in the order they're added)
- Easy to add/remove systems
- Configuration-driven (can load systems from config)

### 2. Game Systems (`include/rtype/server/GameSystems.hpp`)

#### MovementSystem

Updates position for all entities with Transform and Velocity.

```cpp
class MovementSystem : public ISystem {
    void update(Registry& registry, float deltaTime) override {
        auto entities = registry.getEntitiesWithComponents<Transform, Velocity>();
        for (auto entity : entities) {
            auto& transform = registry.getComponent<Transform>(entity);
            auto& velocity = registry.getComponent<Velocity>(entity);
            
            transform.x += velocity.vx * deltaTime;
            transform.y += velocity.vy * deltaTime;
        }
    }
};
```

**Processes:** Players, monsters, bullets, powerups - anything that moves!

#### CollisionSystem

Detects and handles collisions between projectiles and damageable entities.

```cpp
class CollisionSystem : public ISystem {
    void update(Registry& registry, float deltaTime) override {
        // Get all projectiles
        auto projectiles = registry.getEntitiesWithComponents<Transform, Projectile>();
        
        // Get all damageable entities (with Health)
        auto targets = registry.getEntitiesWithComponents<Transform, Health>();
        
        // Check each projectile against each target
        for (auto proj : projectiles) {
            for (auto target : targets) {
                if (collides(proj, target)) {
                    applyDamage(target, proj);
                    markDestroy(proj);
                }
            }
        }
    }
};
```

**Why this works for all entities:**
- Doesn't care if target is player or monster
- Just checks: "Does it have Transform + Health?"
- Generic collision logic works for everything

#### BoundarySystem

Removes entities that go outside world bounds. Special handling for players (clamps them).

```cpp
class BoundarySystem : public ISystem {
    void update(Registry& registry, float deltaTime) override {
        for (auto entity : registry.getEntitiesWithComponents<Transform>()) {
            auto& transform = registry.getComponent<Transform>(entity);
            
            if (registry.hasComponent<PlayerComponent>(entity)) {
                // Clamp players to screen
                transform.x = std::clamp(transform.x, 0.0f, worldWidth);
                transform.y = std::clamp(transform.y, 0.0f, worldHeight);
            } else {
                // Destroy non-players outside bounds
                if (outOfBounds(transform)) {
                    markDestroy(entity);
                }
            }
        }
    }
};
```

#### MonsterSpawnerSystem

Spawns monsters at configured intervals based on quotas. Works independently or controlled by LevelSystem.

```cpp
class MonsterSpawnerSystem : public ISystem {
public:
    void update(Registry& registry, float deltaTime) override {
        if (_monstersSpawned >= _monstersToSpawn)
            return;  // Quota reached
        
        _spawnTimer += deltaTime;
        if (_spawnTimer >= _config.monsterSpawnDelay) {
            spawnMonster();  // Uses config for type, position, velocity
            _spawnTimer = 0.0f;
            _monstersSpawned++;
        }
    }
    
    void startSpawning(int count) {
        _monstersToSpawn = count;
        _monstersSpawned = 0;
    }
};
```

**Features:**
- Weighted random monster type selection
- Config-driven spawn positions and movement
- Interval-based spawning
- Can run standalone (infinite spawning) or controlled by LevelSystem

#### LevelSystem

Manages wave progression and difficulty scaling. Controls MonsterSpawnerSystem.

```cpp
class LevelSystem : public ISystem {
public:
    void update(Registry& registry, float deltaTime) override {
        // Check if current wave is complete
        if (_spawner.isSpawningComplete() && isWaveComplete()) {
            if (_currentWave < maxWaves) {
                startWave(_currentWave + 1);
            }
        }
    }
    
private:
    void startWave(int waveNumber) {
        _currentWave = waveNumber;
        int monstersToSpawn = _config.monsterPerLevel * waveNumber;
        _spawner.startSpawning(monstersToSpawn);  // Tell spawner to spawn
    }
    
    bool isWaveComplete() const {
        // Check if all spawned monsters are dead
        return !hasLivingMonsters(registry);
    }
};
```

**Wave Progression:**
1. Spawner spawns all monsters for current wave
2. LevelSystem waits until all monsters are defeated
3. Automatically starts next wave with more monsters
4. Formula: `MonsterPerLevel × WaveNumber`

**Configuration Modes:**

| MonsterSpawnerSystem | LevelSystem | Behavior |
|---------------------|-------------|----------|
| true | true | Wave-based progression with scaling difficulty |
| true | false | Continuous spawning, no wave tracking |
| false | true | Error - LevelSystem requires spawner |
| false | false | No monsters spawn |

#### CleanupSystem
    float worldWidth, worldHeight, margin;
    
    void update(Registry& registry, float deltaTime) override {
        auto entities = registry.getEntitiesWithComponents<Transform>();
        
        for (auto entity : entities) {
            auto& transform = registry.getComponent<Transform>(entity);
            
            // Players: clamp to screen
            if (registry.hasComponent<PlayerComponent>(entity)) {
                transform.x = std::clamp(transform.x, 0.0f, worldWidth);
                transform.y = std::clamp(transform.y, 0.0f, worldHeight);
            }
            // Others: destroy if outside bounds
            else if (transform.x < -margin || transform.x > worldWidth + margin ||
                     transform.y < -margin || transform.y > worldHeight + margin) {
                markDestroy(entity);
            }
        }
    }
};
```

**Why component checks work:**
- `hasComponent<PlayerComponent>()` identifies players
- Generic code handles special cases
- No need for class hierarchies

#### Other Systems

- **FireCooldownSystem**: Manages weapon cooldowns
- **ProjectileLifetimeSystem**: Ages bullets and removes old ones
- **CleanupSystem**: Removes destroyed entities

### 3. Components (`include/rtype/common/Components.hpp`)

All components are pure data structures:

```cpp
struct Transform {
    float x{0.0f};
    float y{0.0f};
};

struct Velocity {
    float vx{0.0f};
    float vy{0.0f};
};

struct Health {
    std::uint8_t hp{3};
    bool alive{true};
};

struct PlayerComponent {
    PlayerId id{};
};

struct MonsterComponent {
    std::uint8_t type{0};
};

struct Projectile {
    EntityId owner{};
    bool fromPlayer{true};
    float lifetime{0.0f};
    std::uint8_t damage{1};
    std::uint8_t weaponType{0};
    bool persistent{false};
    float damageTickTimer{0.0f};
};

struct PowerUp {
    std::uint8_t type{0};   // Type (basic/laser/rocket)
    std::uint8_t value{1};  // Weapon type to grant
};

struct FireCooldown {
    float timer{0.0f};
    float cooldownTime{0.25f};
};

struct WeaponComponent {
    std::uint8_t weaponType{0};
    std::uint8_t weaponLevel{1};
    bool laserActive{false};
    EntityId activeLaserId{0};
    bool laserUnlocked{false};
    bool rocketUnlocked{false};
    std::uint16_t powerUpsCollected{0};
};
```

**Design Principles:**
- ✅ Default member initializers
- ✅ Aggregate initialization (no constructors needed)
- ✅ POD types when possible (plain old data)
- ❌ No methods
- ❌ No inheritance

---

## Dependency Abstraction

### Why Abstraction?

**Problem:** If we use SFML and ASIO directly in game code:
```cpp
// BAD: Game code depends on SFML
class GameClient {
    sf::RenderWindow window;  // ← Tight coupling to SFML!
    asio::ip::udp::socket socket;  // ← Tight coupling to ASIO!
    
    void render() {
        window.draw(sprite);  // ← Can't switch to SDL/Raylib
    }
};
```

**Problems:**
- ❌ Can't replace SFML with SDL2/Raylib without rewriting game code
- ❌ Can't replace ASIO with ENet/ZeroMQ
- ❌ Hard to test (need real window/network)
- ❌ Platform-specific code leaks into game logic

**Solution:** Abstract interfaces!

### Network Abstraction

#### Interface (`include/rtype/common/INetwork.hpp`)

```cpp
// Abstract endpoint (no ASIO types!)
class IEndpoint {
public:
    virtual ~IEndpoint() = default;
    virtual std::string getKey() const = 0;
    virtual std::unique_ptr<IEndpoint> clone() const = 0;
};

// Abstract socket (no ASIO types!)
class ISocket {
public:
    virtual ~ISocket() = default;
    
    virtual void sendTo(std::span<const std::uint8_t> data, 
                       const IEndpoint& endpoint) = 0;
    
    virtual void asyncReceive(
        std::function<void(std::span<const std::uint8_t>, 
                          std::unique_ptr<IEndpoint>)> handler) = 0;
    
    virtual void close() = 0;
};

// Abstract IO context (no ASIO types!)
class IIOContext {
public:
    virtual ~IIOContext() = default;
    
    virtual void run() = 0;
    virtual void stop() = 0;
    virtual std::unique_ptr<ISocket> createUdpSocket(std::uint16_t port) = 0;
};
```

**Key Points:**
- ✅ No ASIO types in interface
- ✅ Uses standard C++ types (`std::span`, `std::function`, `std::string`)
- ✅ Virtual destructors for polymorphism
- ✅ Factory method pattern (`createUdpSocket`)

#### Implementation (`include/rtype/common/AsioNetwork.hpp`)

```cpp
// ASIO implementation (hidden from game code!)
class AsioEndpoint : public IEndpoint {
private:
    asio::ip::udp::endpoint endpoint;  // ← ASIO type hidden!
    
public:
    AsioEndpoint(asio::ip::udp::endpoint ep) : endpoint(ep) {}
    
    std::string getKey() const override {
        return endpoint.address().to_string() + ":" + 
               std::to_string(endpoint.port());
    }
};

class AsioUdpSocket : public ISocket {
private:
    asio::ip::udp::socket socket;  // ← ASIO type hidden!
    
public:
    void sendTo(std::span<const std::uint8_t> data, 
               const IEndpoint& endpoint) override {
        auto& asioEp = static_cast<const AsioEndpoint&>(endpoint);
        socket.send_to(asio::buffer(data.data(), data.size()), 
                      asioEp.getEndpoint());
    }
};
```

**Key Points:**
- ✅ ASIO types are **private** (hidden)
- ✅ Implements abstract interface
- ✅ Can be replaced with different implementation

#### Factory (`include/rtype/common/NetworkFactory.hpp`)

```cpp
class NetworkFactory {
public:
    static std::unique_ptr<IIOContext> createIOContext() {
        return std::make_unique<AsioIOContext>();
    }
};
```

**Usage in Game Code:**
```cpp
// Game code uses interfaces only!
class GameServer {
    std::unique_ptr<IIOContext> ioContext;
    std::unique_ptr<ISocket> socket;
    
public:
    GameServer() {
        ioContext = NetworkFactory::createIOContext();
        socket = ioContext->createUdpSocket(5000);
        
        // No ASIO types anywhere!
        socket->asyncReceive([](auto data, auto endpoint) {
            // Handle received data
        });
    }
};
```

**Benefits:**
- ✅ Can swap ASIO for ENet: just change factory
- ✅ Can mock for testing: create `MockSocket` implementing `ISocket`
- ✅ Game code is library-agnostic

### Rendering Abstraction

#### Interface (`include/rtype/client/IRender.hpp`)

```cpp
// Custom types (not SFML!)
struct Vector2 {
    float x, y;
};

struct Color {
    std::uint8_t r, g, b, a;
};

// Abstract rendering interface
class IRender {
public:
    virtual ~IRender() = default;
    
    virtual bool isOpen() const = 0;
    virtual void clear() = 0;
    virtual void display() = 0;
    
    virtual void drawCircle(const Vector2& pos, float radius, 
                           const Color& color) = 0;
    
    virtual void drawRectangle(const Vector2& pos, const Vector2& size, 
                              const Color& color, float rotation = 0.0f) = 0;
    
    virtual void drawText(const std::string& text, const Vector2& pos, 
                         std::uint32_t size, const Color& color) = 0;
    
    virtual bool pollEvent(/* ... */) = 0;
};
```

**Key Points:**
- ✅ No SFML types (`sf::Vector2f`, `sf::Color`)
- ✅ Custom types (`Vector2`, `Color`)
- ✅ Minimal interface (only what we need)

#### Implementation (`include/rtype/client/SFMLRenderer.hpp`)

```cpp
class SFMLRenderer : public IRender {
private:
    sf::RenderWindow window;  // ← SFML type hidden!
    
public:
    void drawCircle(const Vector2& pos, float radius, 
                   const Color& color) override {
        sf::CircleShape circle(radius);
        circle.setPosition(pos.x, pos.y);
        circle.setFillColor(sf::Color(color.r, color.g, color.b, color.a));
        window.draw(circle);
    }
    
    // Convert our types to SFML types internally
};
```

**Usage in Game Code:**
```cpp
class GameClient {
    std::unique_ptr<IRender> renderer;
    
public:
    GameClient() {
        // Factory creates renderer
        renderer = RenderFactory::createRenderer(1280, 720, "R-Type", config);
        
        // Use abstract interface (no SFML types!)
        renderer->drawCircle({100, 200}, 20, {255, 0, 0, 255});
    }
};
```

**Benefits:**
- ✅ Can swap SFML for SDL2/Raylib: just change factory
- ✅ Can create headless renderer for tests
- ✅ Platform-independent game code

### Dependency Graph

```
Game Code (Server/Client)
        │
        │ uses
        ↓
  Abstract Interfaces (INetwork, IRender)
        │
        │ implements
        ↓
  Concrete Implementations (ASIO, SFML)
        │
        │ uses
        ↓
  External Libraries (asio, sfml)
```

**Key Principle:** Game code depends on abstractions, not implementations.

---

## Entity Factory Pattern

### Why Factory?

**Problem:** Creating entities manually is repetitive:
```cpp
// Create player - lots of boilerplate!
EntityId player = registry.createEntity();
registry.addComponent<Transform>(player, 100, 200);
registry.addComponent<Velocity>(player, 0, 0);
registry.addComponent<PlayerComponent>(player, 1);
registry.addComponent<Health>(player, config.playerStartHP, true);
registry.addComponent<FireCooldown>(player, 0.0f);

// Create monster - repeat similar code!
EntityId monster = registry.createEntity();
registry.addComponent<Transform>(monster, 500, 300);
registry.addComponent<Velocity>(monster, -30, 0);
registry.addComponent<MonsterComponent>(monster, 0);
auto hp = config.MonstersType[0].HP;
registry.addComponent<Health>(monster, hp, true);

// Create bullet - more repetition!
EntityId bullet = registry.createEntity();
// ... etc
```

**Problems:**
- ❌ Code duplication
- ❌ Easy to forget components
- ❌ Hardcoded values scattered everywhere
- ❌ Hard to maintain

**Solution:** Entity Factory!

### Factory Implementation (`include/rtype/server/EntityFactory.hpp`)

```cpp
class EntityFactory {
public:
    EntityFactory(Registry& registry, const GameConfig& config)
        : _registry(registry), _config(config) {}
    
    // Generic spawn methods
    EntityId spawnPlayer(PlayerId id, float x, float y);
    EntityId spawnMonster(std::uint8_t type, float x, float y, float vx, float vy);
    EntityId spawnBullet(PlayerId owner, float x, float y, float vx, float vy);
    EntityId spawnPowerUp(std::uint8_t type, float x, float y, float vx, float vy);
    
private:
    Registry& _registry;
    const GameConfig& _config;
    
    // Helper method
    void addTransformAndVelocity(EntityId entity, float x, float y, 
                                float vx, float vy);
};
```

### Factory Methods (`src/server/EntityFactory.cpp`)

```cpp
EntityId EntityFactory::spawnPlayer(PlayerId id, float x, float y) {
    auto entity = _registry.createEntity();
    
    // All values from config!
    addTransformAndVelocity(entity, x, y, 0.0f, 0.0f);
    _registry.addComponent<PlayerComponent>(entity, id);
    _registry.addComponent<Health>(entity, _config.gameplay.playerStartHP, true);
    _registry.addComponent<FireCooldown>(entity, 0.0f);
    
    return entity;
}

EntityId EntityFactory::spawnMonster(std::uint8_t type, float x, float y, 
                                     float vx, float vy) {
    auto entity = _registry.createEntity();
    
    // Get monster stats from config
    std::uint8_t hp = _config.gameplay.monsterHP;
    auto it = _config.gameplay.MonstersType.find(type);
    if (it != _config.gameplay.MonstersType.end()) {
        hp = it->second.HP;
    }
    
    addTransformAndVelocity(entity, x, y, vx, vy);
    _registry.addComponent<MonsterComponent>(entity, type);
    _registry.addComponent<Health>(entity, hp, true);
    
    return entity;
}

EntityId EntityFactory::spawnBullet(PlayerId owner, float x, float y, 
                                    float vx, float vy) {
    auto entity = _registry.createEntity();
    
    addTransformAndVelocity(entity, x, y, vx, vy);
    _registry.addComponent<Projectile>(entity, owner, true, 0.0f, 
                                      _config.gameplay.weaponDamageBasic);
    
    return entity;
}
```

### Usage

**Before (manual creation):**
```cpp
void spawnPlayer(PlayerId id, float x, float y) {
    EntityId player = registry.createEntity();
    registry.addComponent<Transform>(player, x, y);
    registry.addComponent<Velocity>(player, 0, 0);
    registry.addComponent<PlayerComponent>(player, id);
    registry.addComponent<Health>(player, config.playerStartHP, true);
    registry.addComponent<FireCooldown>(player, 0.0f);
}
```

**After (factory):**
```cpp
void spawnPlayer(PlayerId id, float x, float y) {
    _entityFactory.spawnPlayer(id, x, y);
}
```

### Benefits

✅ **No Code Duplication**: One implementation per entity type  
✅ **Configuration-Driven**: All values from `game.ini`  
✅ **Easy to Maintain**: Change entity structure in one place  
✅ **Type Safety**: Factory ensures proper component initialization  
✅ **Testability**: Can mock factory for tests  

---

## Configuration System

### Why Configuration?

**Problem:** Hardcoded values in code:
```cpp
// BAD: Can't change without recompiling
const int PLAYER_HP = 3;
const float BULLET_SPEED = 380.0f;
const float MONSTER_SPAWN_DELAY = 2.0f;
```

**Solution:** Configuration files!

### Configuration Structure

#### Game Configuration (`config/game.ini`)

```ini
[Gameplay]
# Player settings
PlayerSpeed=220.0
PlayerStartHP=3
PlayerFireCooldown=0.25

# Bullet settings
BulletSpeed=380.0
BulletLifetime=3.0
BulletDirection=LeftToRight

# Monster settings
MonsterSpawnDelay=2.0
MonsterType0Size=24.0
MonsterType0HP=3
MonsterType0Color=200,60,60

# PowerUp settings
PowerUpSpawnDelay=10.0
PowerUpsEnabled=true
PowerUpSpawnCenterX=0.6
PowerUpSpawnCenterY=0.5

# World
WorldWidth=1280.0
WorldHeight=720.0

[Render]
WindowWidth=1280
WindowHeight=720
TargetFPS=60

PlayerSize=20.0
Player1Color=95,205,228
BulletColor=255,207,64
BackgroundColor=6,10,26

StarCount=100
StarSpeedMin=30.0
StarSpeedMax=90.0

[Network]
DefaultPort=5000
DefaultHost=127.0.0.1
MaxPlayers=4
```

#### Systems Configuration (`config/systems.ini`)

```ini
[Systems]
# Enable/disable systems without recompiling!
MovementSystem=true
FireCooldownSystem=true
ProjectileLifetimeSystem=true
CollisionSystem=true
BoundarySystem=true
CleanupSystem=true

[SystemParameters]
BoundaryMargin=100.0
```

### Configuration Loading

#### Config Structure (`include/rtype/common/GameConfig.hpp`)

```cpp
struct GameplayConfig {
    float playerSpeed{220.0f};
    std::uint8_t playerStartHP{3};
    float playerFireCooldown{0.25f};
    
    float bulletSpeed{380.0f};
    float bulletLifetime{3.0f};
    
    float monsterSpawnDelay{2.0f};
    std::unordered_map<int, MonsterType> MonstersType;
    
    float powerUpSpawnDelay{10.0f};
    bool powerUpsEnabled{true};
    float powerUpSpawnCenterX{0.6f};
    float powerUpSpawnCenterY{0.5f};
    
    float worldWidth{1280.0f};
    float worldHeight{720.0f};
    
    // ... all other config values
};

struct RenderConfig {
    std::uint32_t windowWidth{1280};
    std::uint32_t windowHeight{720};
    std::uint32_t targetFPS{60};
    
    float playerSize{20.0f};
    Color player1Color{95, 205, 228};
    Color bulletColor{255, 207, 64};
    
    std::uint32_t starCount{100};
    float starSpeedMin{30.0f};
    float starSpeedMax{90.0f};
    
    // ... all other render values
};

struct GameConfig {
    GameplayConfig gameplay;
    RenderConfig render;
    NetworkConfig network;
    AudioConfig audio;
    SystemsConfig systems;
    
    // Load from file
    static GameConfig loadFromFile(const std::string& filename);
};
```

#### Parser (`src/common/config/GameConfig.cpp`)

```cpp
GameConfig GameConfig::loadFromFile(const std::string& filename) {
    GameConfig config;
    std::ifstream file(filename);
    std::string line, section;
    
    while (std::getline(file, line)) {
        // Parse [Section]
        if (line.starts_with('[')) {
            section = line.substr(1, line.find(']') - 1);
            continue;
        }
        
        // Parse Key=Value
        auto pos = line.find('=');
        if (pos == std::string::npos) continue;
        
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        // Assign to appropriate config section
        if (section == "Gameplay") {
            if (key == "PlayerSpeed") 
                config.gameplay.playerSpeed = std::stof(value);
            else if (key == "PlayerStartHP") 
                config.gameplay.playerStartHP = std::stoi(value);
            // ... etc for all keys
        }
        else if (section == "Render") {
            // ... parse render config
        }
    }
    
    return config;
}
```

### Dynamic System Loading

```cpp
void GameLogicHandler::initializeSystems() {
    std::cout << "[logic] Initializing ECS systems from configuration...\n";
    
    // Load systems based on config
    if (_config.systems.movementSystem) {
        _systemPipeline.addSystem(std::make_unique<MovementSystem>());
        std::cout << "[logic] - MovementSystem loaded\n";
    }
    
    if (_config.systems.collisionSystem) {
        _systemPipeline.addSystem(std::make_unique<CollisionSystem>(
            _registry, toDestroySet, _config.gameplay.collisionRadius
        ));
        std::cout << "[logic] - CollisionSystem loaded\n";
    }
    
    if (_config.systems.boundarySystem) {
        _systemPipeline.addSystem(std::make_unique<BoundarySystem>(
            _config.gameplay.worldWidth,
            _config.gameplay.worldHeight,
            _config.systems.boundaryMargin,  // From systems.ini!
            toDestroySet
        ));
        std::cout << "[logic] - BoundarySystem loaded\n";
    }
    
    // ... etc for all systems
}
```

### Benefits

✅ **No Recompilation**: Change values without rebuilding  
✅ **Easy Balancing**: Tweak gameplay without coding  
✅ **Easy Testing**: Test different configurations quickly  
✅ **Runtime Flexibility**: Enable/disable systems on the fly  

---

## Network Architecture

### Protocol Design

#### Binary Protocol

We use a compact binary protocol for efficiency:

```cpp
struct PacketHeader {
    std::uint8_t type;     // Packet type (PlayerInput, EntityState, etc.)
    std::uint32_t seq;     // Sequence number
    std::uint32_t timestamp; // Timestamp
};
```

#### Packet Types

```cpp
enum class PacketType : std::uint8_t {
    PlayerInput = 1,      // Client → Server: input
    EntityState = 2,      // Server → Client: entity updates
    PlayerJoin = 3,       // Client → Server: join request
    PlayerAssignment = 4, // Server → Client: player ID
    LevelStart = 5,       // Server → Client: level started
    Disconnect = 6        // Either → Other: disconnect notice
};
```

#### Example: Player Input Packet

```cpp
struct PlayerInput {
    PacketHeader header;
    std::uint8_t playerId;
    bool moveUp, moveDown, moveLeft, moveRight;
    bool shoot;
    
    // Serialize to bytes
    std::vector<std::uint8_t> serialize() const;
    
    // Deserialize from bytes
    static PlayerInput deserialize(std::span<const std::uint8_t> data);
};
```

### Client-Server Flow

```
Client:                          Server:
  │                                 │
  ├──[PlayerJoin]────────────────>  │
  │                                 ├─ Create player entity
  │                                 ├─ Assign player ID
  │  <────────────[PlayerAssignment]┤
  │                                 │
  ├──[PlayerInput]─────────────────>│
  │  (move right, shoot)            ├─ Process input
  │                                 ├─ Update game state
  │                                 ├─ Run ECS systems
  │  <────────────[EntityState]─────┤
  │  (all entities positions)       │
  ├─ Update local state             │
  ├─ Render entities                │
  │                                 │
  ├──[PlayerInput]─────────────────>│
  │  (move up)                      ├─ Process input
  │  <────────────[EntityState]─────┤
  │                                 │
```

### Server-Side Network Handling

```cpp
class GameServer {
    void handleIncomingPacket(std::span<const std::uint8_t> data, 
                             std::unique_ptr<IEndpoint> endpoint) {
        auto type = static_cast<PacketType>(data[0]);
        
        switch (type) {
            case PacketType::PlayerJoin: {
                auto playerId = assignPlayer(endpoint);
                sendPlayerAssignment(playerId, endpoint);
                break;
            }
            
            case PacketType::PlayerInput: {
                auto input = PlayerInput::deserialize(data);
                gameLogic.processInput(input);
                break;
            }
            
            case PacketType::Disconnect: {
                removePlayer(endpoint);
                break;
            }
        }
    }
    
    void broadcastGameState() {
        auto state = gameLogic.getEntityStates();
        auto packet = EntityState::serialize(state);
        
        for (auto& [endpoint, playerId] : clients) {
            socket->sendTo(packet, *endpoint);
        }
    }
};
```

### Client-Side Network Handling

```cpp
class GameClient {
    void handleIncomingPacket(std::span<const std::uint8_t> data) {
        auto type = static_cast<PacketType>(data[0]);
        
        switch (type) {
            case PacketType::PlayerAssignment: {
                auto assignment = PlayerAssignment::deserialize(data);
                _myPlayerId = assignment.playerId;
                std::cout << "[client] Assigned player ID: " 
                         << (int)_myPlayerId << '\n';
                break;
            }
            
            case PacketType::EntityState: {
                auto state = EntityState::deserialize(data);
                updateEntities(state);
                break;
            }
            
            case PacketType::LevelStart: {
                auto level = LevelStart::deserialize(data);
                std::cout << "[client] Level " << (int)level.levelNumber 
                         << " started!\n";
                break;
            }
        }
    }
    
    void sendInput() {
        PlayerInput input;
        input.playerId = _myPlayerId;
        input.moveUp = isKeyPressed(Up);
        input.moveDown = isKeyPressed(Down);
        input.moveLeft = isKeyPressed(Left);
        input.moveRight = isKeyPressed(Right);
        input.shoot = isKeyPressed(Space);
        
        auto packet = input.serialize();
        _socket->sendTo(packet, *_serverEndpoint);
    }
};
```

---

## Game Flow

### Server Startup

```
1. Load Configuration
   ├─ Load game.ini
   ├─ Load systems.ini
   └─ Parse monster types, player settings, etc.

2. Initialize ECS
   ├─ Create Registry
   ├─ Create EntityFactory
   └─ Load Systems (from config!)
       ├─ MovementSystem
       ├─ CollisionSystem
       ├─ BoundarySystem
       └─ CleanupSystem

3. Initialize Network
   ├─ Create IOContext (ASIO abstraction)
   ├─ Create UDP Socket (port 5000)
   └─ Start async receive

4. Start Game Loop
   └─ Run at 60 FPS
```

### Game Loop (Server)

```cpp
void gameLoop() {
    auto lastTime = std::chrono::steady_clock::now();
    
    while (running) {
        auto currentTime = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(
            currentTime - lastTime).count();
        lastTime = currentTime;
        
        // 1. Process network (receive inputs)
        ioContext->poll();
        
        // 2. Update game state
        gameLogic.update(deltaTime);
        // This runs:
        //   - MovementSystem (update positions)
        //   - FireCooldownSystem (update cooldowns)
        //   - ProjectileLifetimeSystem (age bullets)
        //   - CollisionSystem (detect collisions)
        //   - BoundarySystem (remove out-of-bounds entities)
        //   - CleanupSystem (remove dead entities)
        //   - Monster spawning
        //   - PowerUp spawning
        
        // 3. Broadcast state to all clients
        broadcastGameState();
        
        // 4. Sleep to maintain 60 FPS
        std::this_thread::sleep_for(16ms);
    }
}
```

### Client Startup

```
1. Load Configuration
   └─ Load game.ini (render settings)

2. Initialize Rendering
   ├─ Create Renderer (SFML abstraction)
   ├─ Open window (1280x720)
   └─ Build starfield background

3. Initialize Network
   ├─ Create IOContext
   ├─ Create UDP Socket
   └─ Connect to server (127.0.0.1:5000)

4. Send Join Request
   └─ Wait for PlayerAssignment

5. Start Game Loop
   └─ Run at 60 FPS
```

### Game Loop (Client)

```cpp
void gameLoop() {
    while (renderer->isOpen()) {
        // 1. Handle window events
        while (renderer->pollEvent(event)) {
            if (event.type == Close) {
                renderer->close();
            }
        }
        
        // 2. Handle input
        sendPlayerInput();
        
        // 3. Process network (receive state updates)
        ioContext->poll();
        
        // 4. Render
        renderer->clear();
        
        // Draw background
        drawStarfield();
        
        // Draw all entities
        for (auto& [id, entity] : entities) {
            if (entity.isPlayer) {
                drawPlayer(entity);
            } else if (entity.isMonster) {
                drawMonster(entity);
            } else if (entity.isBullet) {
                drawBullet(entity);
            } else if (entity.isPowerUp) {
                drawPowerUp(entity);
            }
        }
        
        // Draw UI
        drawHUD();
        
        renderer->display();
        
        // 5. Sleep to maintain 60 FPS
        std::this_thread::sleep_for(16ms);
    }
}
```

### Entity Lifecycle

```
Creation:
    ├─ EntityFactory.spawnX() called
    ├─ Registry.createEntity() → returns ID
    ├─ Components added to entity
    └─ Entity is now active

Update (each frame):
    ├─ Systems iterate over entity
    ├─ Systems read/modify components
    └─ Entity behavior emerges from components

Destruction:
    ├─ System marks entity for destruction
    │   (e.g., health = 0, out of bounds)
    ├─ Added to destruction queue
    ├─ CleanupSystem removes entity
    └─ Registry.destroyEntity() called
```

---

## How to Extend

### Adding a New Component

1. **Define component** (`include/rtype/common/Components.hpp`):
```cpp
struct Shield {
    std::uint8_t strength{5};  // Shield HP
    float rechargeRate{0.5f};  // HP per second
    float rechargeDelay{3.0f}; // Seconds before recharge
    float timeSinceDamage{0.0f};
};
```

2. **Use in systems** (automatically works with existing systems!):
```cpp
// Shields automatically move with MovementSystem (if entity has Transform+Velocity)
// Shields automatically get collision detection (if entity has Transform)
```

3. **Create specialized system** if needed:
```cpp
class ShieldRechargeSystem : public ISystem {
    void update(Registry& registry, float deltaTime) override {
        auto entities = registry.getEntitiesWithComponents<Shield>();
        
        for (auto entity : entities) {
            auto& shield = registry.getComponent<Shield>(entity);
            
            shield.timeSinceDamage += deltaTime;
            
            if (shield.timeSinceDamage >= shield.rechargeDelay) {
                shield.strength = std::min(shield.strength + 
                    static_cast<std::uint8_t>(shield.rechargeRate * deltaTime),
                    static_cast<std::uint8_t>(5));
            }
        }
    }
};
```

4. **Add to EntityFactory**:
```cpp
EntityId EntityFactory::spawnShieldedPlayer(PlayerId id, float x, float y) {
    auto entity = spawnPlayer(id, x, y);  // Create normal player
    _registry.addComponent<Shield>(entity, 5, 0.5f, 3.0f, 0.0f);  // Add shield
    return entity;
}
```

### Adding a New System

1. **Create system class** (`include/rtype/server/GameSystems.hpp`):
```cpp
class GravitySystem : public ISystem {
public:
    GravitySystem(float gravity = 9.8f) : _gravity(gravity) {}
    
    void update(Registry& registry, float deltaTime) override {
        auto entities = registry.getEntitiesWithComponents<Velocity>();
        
        for (auto entity : entities) {
            auto& velocity = registry.getComponent<Velocity>(entity);
            
            // Apply gravity
            velocity.vy += _gravity * deltaTime;
        }
    }
    
private:
    float _gravity;
};
```

2. **Add to config** (`config/systems.ini`):
```ini
[Systems]
GravitySystem=true

[SystemParameters]
Gravity=9.8
```

3. **Load in GameLogicHandler**:
```cpp
if (_config.systems.gravitySystem) {
    _systemPipeline.addSystem(std::make_unique<GravitySystem>(
        _config.systems.gravity
    ));
}
```

### Adding a New Entity Type

1. **Add component** (if needed):
```cpp
struct BossComponent {
    std::uint8_t phase{1};
    std::string attackPattern{"spiral"};
};
```

2. **Add to EntityFactory**:
```cpp
EntityId EntityFactory::spawnBoss(std::uint8_t type, float x, float y) {
    auto entity = _registry.createEntity();
    
    // Get boss stats from config
    auto hp = _config.gameplay.bossTypes[type].HP;
    
    addTransformAndVelocity(entity, x, y, 0.0f, 0.0f);
    _registry.addComponent<BossComponent>(entity, 1, "spiral");
    _registry.addComponent<Health>(entity, hp, true);
    
    return entity;
}
```

3. **Add to config** (`config/game.ini`):
```ini
[Gameplay]
BossType0HP=100
BossType0Size=80.0
BossType0Color=255,0,0
```

4. **Systems automatically handle it!**
   - MovementSystem moves boss (has Transform+Velocity)
   - CollisionSystem detects bullet hits (has Transform+Health)
   - BoundarySystem keeps boss in bounds (has Transform)

### Adding a New Rendering Effect

1. **Add to RenderConfig**:
```cpp
struct RenderConfig {
    // ... existing fields ...
    
    bool particleEffects{true};
    std::uint32_t maxParticles{1000};
};
```

2. **Add to Renderer**:
```cpp
class SFMLRenderer : public IRender {
public:
    void drawExplosion(const Vector2& pos, float radius, 
                      const Color& color) override {
        if (!_config.render.particleEffects) return;
        
        // Spawn particles
        for (int i = 0; i < 20; i++) {
            // Create particle
        }
    }
};
```

3. **Add to config**:
```ini
[Render]
ParticleEffects=true
MaxParticles=1000
```

---

## Performance Considerations

### ECS Performance

**Why ECS is Fast:**

1. **Cache-Friendly**: Components stored contiguously in memory
```cpp
// Traditional OOP: objects scattered
Player* players[100];  // Pointers to different memory locations
// ❌ Cache misses when iterating

// ECS: components contiguous
std::vector<Transform> transforms;  // All transforms together
std::vector<Velocity> velocities;   // All velocities together
// ✅ Cache-friendly iteration
```

2. **Data-Oriented**: Systems process arrays of data
```cpp
// Process 1000 entities
for (auto entity : entities) {
    transform.x += velocity.vx * dt;  // Simple array access
    transform.y += velocity.vy * dt;  // CPU can vectorize this!
}
```

3. **Skip Irrelevant Entities**: Only process entities with needed components
```cpp
// Only iterate entities with Transform+Velocity
// Skips entities without movement automatically
auto moving = registry.getEntitiesWithComponents<Transform, Velocity>();
```

### Network Optimization

1. **Binary Protocol**: Compact representation (not JSON/XML)
2. **Delta Compression**: Only send changed entities (future optimization)
3. **Prediction**: Client predicts movement (future optimization)

### Rendering Optimization

1. **Batch Drawing**: Group similar entities
2. **Frustum Culling**: Don't render off-screen entities (future optimization)
3. **Sprite Sheets**: Reduce texture switches (future optimization)

---

## Testing Strategy

### Unit Testing Components

```cpp
TEST(ComponentTest, TransformInitialization) {
    Transform t{100.0f, 200.0f};
    EXPECT_EQ(t.x, 100.0f);
    EXPECT_EQ(t.y, 200.0f);
}
```

### Unit Testing Systems

```cpp
TEST(MovementSystemTest, UpdatesPosition) {
    Registry registry;
    MovementSystem movement;
    
    // Create entity
    auto entity = registry.createEntity();
    registry.addComponent<Transform>(entity, 0.0f, 0.0f);
    registry.addComponent<Velocity>(entity, 10.0f, 0.0f);
    
    // Update system
    movement.update(registry, 1.0f);  // 1 second
    
    // Check result
    auto& transform = registry.getComponent<Transform>(entity);
    EXPECT_EQ(transform.x, 10.0f);  // Moved 10 pixels
}
```

### Integration Testing

```cpp
TEST(GameLogicTest, BulletHitsMonster) {
    GameLogicHandler logic(config);
    
    // Spawn monster
    auto monster = logic.spawnMonster(0, 100, 100);
    auto monsterHealth = logic.getHealth(monster);
    EXPECT_EQ(monsterHealth, 3);  // Initial HP
    
    // Spawn bullet near monster
    auto bullet = logic.spawnBullet(1, 98, 100, 10, 0);
    
    // Update collision system
    logic.update(0.1f);
    
    // Check monster took damage
    monsterHealth = logic.getHealth(monster);
    EXPECT_EQ(monsterHealth, 2);  // Lost 1 HP
}
```

### Mock Testing (Network)

```cpp
class MockSocket : public ISocket {
public:
    std::vector<std::vector<uint8_t>> sentPackets;
    
    void sendTo(std::span<const uint8_t> data, 
               const IEndpoint& endpoint) override {
        sentPackets.push_back(std::vector(data.begin(), data.end()));
    }
};

TEST(ServerTest, BroadcastsGameState) {
    auto mockSocket = std::make_unique<MockSocket>();
    GameServer server(std::move(mockSocket));
    
    server.broadcastGameState();
    
    EXPECT_EQ(mockSocket->sentPackets.size(), 4);  // 4 clients
}
```

---

## Conclusion

### What We Built

✅ **True ECS Architecture**: Entities = IDs, Components = Data, Systems = Logic  
✅ **Fully Abstracted Dependencies**: No ASIO/SFML in game code  
✅ **Generic Entity Factory**: Config-driven, no duplication  
✅ **100% Configurable**: All parameters in .ini files  
✅ **Modular Systems**: Enable/disable without recompiling  
✅ **Clean Architecture**: Separation of concerns, testable, maintainable  

### Why This Design?

1. **Flexibility**: Change behavior through config, not code
2. **Maintainability**: Clean separation makes debugging easy
3. **Extensibility**: Add new components/systems without breaking existing code
4. **Testability**: Mock dependencies, test in isolation
5. **Performance**: Data-oriented design for cache efficiency
6. **Professional Quality**: Industry-standard patterns and practices

### Key Takeaways

**ECS is about:**
- Composition over inheritance
- Data separate from logic
- Systems that work on any matching entities

**Abstraction is about:**
- Depending on interfaces, not implementations
- Being able to swap libraries easily
- Testable, mockable code

**Configuration is about:**
- Flexibility without recompilation
- Easy balancing and tweaking
- Runtime behavior changes

---

## Appendix: File Structure

```
RTYPE/
├── include/
│   └── rtype/
│       ├── engine/           # ECS Core
│       │   ├── Registry.hpp
│       │   ├── ISystem.hpp
│       │   └── SystemPipeline.hpp
│       ├── common/           # Shared Code
│       │   ├── Types.hpp
│       │   ├── Components.hpp
│       │   ├── GameConfig.hpp
│       │   ├── Protocol.hpp
│       │   ├── INetwork.hpp
│       │   └── AsioNetwork.hpp
│       ├── server/           # Server-Specific
│       │   ├── GameServer.hpp
│       │   ├── GameLogicHandler.hpp
│       │   ├── GameSystems.hpp
│       │   ├── EntityFactory.hpp
│       │   └── ClientHandler.hpp
│       └── client/           # Client-Specific
│           ├── GameClient.hpp
│           ├── IRender.hpp
│           └── SFMLRender.hpp
├── src/
│   ├── engine/
│   │   └── Registry.cpp
│   ├── common/
│   │   ├── config/
│   │   │   └── GameConfig.cpp
│   │   └── protocol/
│   │       └── Protocol.cpp
│   ├── server/
│   │   ├── main.cpp
│   │   ├── GameServer.cpp
│   │   ├── GameLogicHandler.cpp
│   │   ├── EntityFactory.cpp
│   │   └── ClientHandler.cpp
│   └── client/
│       ├── main.cpp
│       ├── GameClient.cpp
│       └── SFMLRenderer.cpp
├── config/
│   ├── game.ini
│   └── systems.ini
├── docs/
│   └── ARCHITECTURE.md (this file)
└── CMakeLists.txt
```

---

**End of Documentation**

For questions or contributions, please contact the development team.
