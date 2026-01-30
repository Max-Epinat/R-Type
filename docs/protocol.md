# UDP Binary Protocol

> **Related Documentation:**
> - [Developer Guide](developer_guide.md) — ECS implementation and system development
> - [Architecture](architecture.md) — Network architecture and client-server communication
> - [Building and Running](building_and_running.md) — Setup instructions for development

This document provides a complete specification of the R-Type network protocol. It should enable any developer to create a compatible client or server implementation from scratch.

## Table of Contents
1. [Overview](#overview)
2. [Packet Structure](#packet-structure)
3. [Message Types Reference](#message-types-reference)
4. [Connection Flow](#connection-flow)
5. [Client Implementation Guide](#client-implementation-guide)
6. [Server Expectations](#server-expectations)
7. [Error Handling](#error-handling)

## Overview

R-Type uses a custom UDP binary protocol for all client-server communication. The protocol is designed for:
- **Low latency**: UDP with minimal header overhead
- **Reliability by design**: Idempotent state updates, no ACKs required for most packets
- **Simplicity**: Fixed-size headers, straightforward serialization
- **Forward compatibility**: Unknown packet types are safely ignored

All communication is **server-authoritative**. The client sends inputs, and the server broadcasts the authoritative game state.

## Packet Structure

### Packet Header
Every packet begins with a 12-byte header:

| Field | Type | Size | Description |
|-------|------|------|-------------|
| `type` | `u16` | 2 bytes | Packet type identifier (see [Message Types](#message-types-reference)) |
| `payloadSize` | `u16` | 2 bytes | Size of payload in bytes (excludes header) |
| `sequence` | `u32` | 4 bytes | Monotonic sequence number from sender |
| `timestamp` | `u32` | 4 bytes | Sender timestamp in milliseconds |

**Total header size: 12 bytes**

#### Endianness
- All integers use **network byte order** (big-endian)
- Floats are serialized as `u32` bitcasts in network byte order
- Safety: payload is only read if `payload_size` matches the remaining bytes; malformed packets are dropped silently.

- Safety: payload is only read if `payload_size` matches the remaining bytes; malformed packets are dropped silently.

#### Sequence Numbers
- Increments by 1 for each packet sent by that endpoint
- Used for detecting packet loss and out-of-order delivery
- Each endpoint (client/server) maintains its own sequence counter
- Current implementation doesn't enforce strict ordering but provides the infrastructure

#### Timestamps
- Monotonic milliseconds since sender startup
- Used for latency calculation and interpolation
- Not synchronized between client and server

### Payload Structure
Following the 12-byte header is the payload, structured according to the packet type. All payload fields are tightly packed with no padding.

## Message Types Reference

### Complete Type List
| Type | Value | Name | Direction | Payload Size | Description |
|------|-------|------|-----------|--------------|-------------|
| 1 | 0x0001 | `Handshake` | Bidirectional | 0 | Reserved for future authentication |
| 2 | 0x0002 | `PlayerInput` | Client→Server | 7 | Player control inputs |
| 3 | 0x0003 | `PlayerState` | Server→Client | 15 | Player position, health, score |
| 4 | 0x0004 | `MonsterSpawn` | Server→Client | 13 | New enemy entity created |
| 5 | 0x0005 | `MonsterState` | Server→Client | 22 | Enemy position and velocity |
| 6 | 0x0006 | `MonsterDeath` | Server→Client | 5 | Enemy destroyed |
| 7 | 0x0007 | `PlayerDeath` | Server→Client | 1 | Player killed |
| 8 | 0x0008 | `BulletFired` | Server→Client | 22 | New projectile created |
| 9 | 0x0009 | `BulletState` | Server→Client | 12 | Projectile position update |
| 10 | 0x000A | `Disconnect` | Bidirectional | 1 | Player disconnecting |
| 11 | 0x000B | `PlayerAssignment` | Server→Client | 1 | Assign player ID on connect |
| 12 | 0x000C | `PowerUpState` | Server→Client | 15 | Power-up position and type |
| 13 | 0x000D | `LevelBegin` | Server→Client | 1 | New level started |
| 14 | 0x000E | `CreateRoom` | Client→Server | 32 | Create multiplayer lobby |
| 15 | 0x000F | `JoinRoom` | Client→Server | 4 | Join existing room |
| 16 | 0x0010 | `LeaveRoom` | Client→Server | 4 | Leave current room |
| 17 | 0x0011 | `StartGame` | Client→Server | 4 | Host starts game |
| 18 | 0x0012 | `RoomList` | Client→Server | 0 | Request room list |
| 19 | 0x0013 | `RoomCreated` | Server→Client | 38 | Room creation confirmed |
| 20 | 0x0014 | `RoomJoined` | Server→Client | 39 | Successfully joined room |
| 21 | 0x0015 | `RoomLeft` | Server→Client | 4 | Left room confirmation |
| 22 | 0x0016 | `GameStarted` | Server→Client | 4 | Game starting |
| 23 | 0x0017 | `RoomListResponse` | Server→Client | Variable | Available rooms |
| 24 | 0x0018 | `RoomError` | Server→Client | 65 | Room operation failed |
| 25 | 0x0019 | `AllPlayersDead` | Server→Client | 4 | Game over condition |
| 26 | 0x001A | `SpectatorMode` | Server→Client | 2 | Player entered spectator mode |
| 27 | 0x001B | `HostChanged` | Server→Client | 5 | New room host assigned |
| 28 | 0x001C | `ShieldSpawn` | Server→Client | 13 | Shield entity created |
| 29 | 0x001D | `ShieldState` | Server→Client | 22 | Shield position update |
| 30 | 0x001E | `ShieldDeath` | Server→Client | 4 | Shield destroyed |

### Detailed Payload Specifications

#### PlayerInput (Type 2) — Client→Server
Sent by client every frame (typically 60 Hz) containing all input state.

| Field | Type | Size | Description |
|-------|------|------|-------------|
| `player` | `u8` | 1 | Player ID (assigned by server) |
| `up` | `u8` | 1 | Move up (0=no, 1=yes) |
| `down` | `u8` | 1 | Move down (0=no, 1=yes) |
| `left` | `u8` | 1 | Move left (0=no, 1=yes) |
| `right` | `u8` | 1 | Move right (0=no, 1=yes) |
| `fire` | `u8` | 1 | Fire weapon (0=no, 1=yes) |
| `swapWeapon` | `u8` | 1 | Swap weapon (0=no, 1=yes) |

**Total: 7 bytes**

#### PlayerAssignment (Type 11) — Server→Client
First packet received after connecting. Assigns the client a player ID (0-3).

| Field | Type | Size | Description |
|-------|------|------|-------------|
| `playerId` | `u8` | 1 | Assigned player ID |

**Total: 1 byte**

**Client must store this ID and use it in all PlayerInput packets.**

#### PlayerState (Type 3) — Server→Client
Broadcast every server tick (~60 Hz) for each active player.

| Field | Type | Size | Description |
|-------|------|------|-------------|
| `player` | `u8` | 1 | Player ID |
| `x` | `f32` | 4 | X position in world space |
| `y` | `f32` | 4 | Y position in world space |
| `hp` | `u8` | 1 | Current health points |
| `score` | `u16` | 2 | Current score |
| `alive` | `u8` | 1 | Alive status (0=dead, 1=alive) |
| `powerUpType` | `u8` | 1 | Current weapon type (0-2) |

**Total: 15 bytes**

#### MonsterSpawn (Type 4) — Server→Client
Sent once when a new enemy appears.

| Field | Type | Size | Description |
|-------|------|------|-------------|
| `id` | `u32` | 4 | Unique entity ID |
| `x` | `f32` | 4 | Initial X position |
| `y` | `f32` | 4 | Initial Y position |
| `monsterType` | `u8` | 1 | Enemy type (0=basic, 1=fast, 2=tank, 3=boss, etc.) |

**Total: 13 bytes**

**Client should create visual entity with this ID.**

#### MonsterState (Type 5) — Server→Client
Broadcast every tick for each living enemy.

| Field | Type | Size | Description |
|-------|------|------|-------------|
| `id` | `u32` | 4 | Entity ID |
| `type` | `u8` | 1 | Enemy type |
| `x` | `f32` | 4 | X position |
| `y` | `f32` | 4 | Y position |
| `vx` | `f32` | 4 | X velocity (for sprite flipping) |
| `vy` | `f32` | 4 | Y velocity |
| `alive` | `u8` | 1 | Alive status |

**Total: 22 bytes**

#### MonsterDeath (Type 6) — Server→Client
Sent when an enemy is destroyed.

| Field | Type | Size | Description |
|-------|------|------|-------------|
| `id` | `u32` | 4 | Entity ID to remove |
| `killer` | `u8` | 1 | Player ID who killed it |

**Total: 5 bytes**

**Client should remove entity and play death animation.**

#### PlayerDeath (Type 7) — Server→Client
Sent when a player dies.

| Field | Type | Size | Description |
|-------|------|------|-------------|
| `player` | `u8` | 1 | Player ID who died |

**Total: 1 byte**

#### BulletFired (Type 8) — Server→Client
Sent when any projectile is created.

| Field | Type | Size | Description |
|-------|------|------|-------------|
| `id` | `u32` | 4 | Unique bullet ID |
| `owner` | `u8` | 1 | Player ID who fired |
| `x` | `f32` | 4 | Initial X position |
| `y` | `f32` | 4 | Initial Y position |
| `vx` | `f32` | 4 | X velocity |
| `vy` | `f32` | 4 | Y velocity |
| `fromPlayer` | `u8` | 1 | 1 if player bullet, 0 if enemy |

**Total: 22 bytes**

#### BulletState (Type 9) — Server→Client
Broadcast for active projectiles each tick.

| Field | Type | Size | Description |
|-------|------|------|-------------|
| `id` | `u32` | 4 | Bullet ID |
| `x` | `f32` | 4 | X position |
| `y` | `f32` | 4 | Y position |
| `weaponType` | `u8` | 1 | Weapon type for visuals |
| `fromPlayer` | `u8` | 1 | Source indicator |
| `active` | `u8` | 1 | Active status |

**Total: 12 bytes**

#### PowerUpState (Type 12) — Server→Client
Broadcast for active power-ups.

| Field | Type | Size | Description |
|-------|------|------|-------------|
| `id` | `u32` | 4 | Power-up entity ID |
| `type` | `u8` | 1 | Power-up category |
| `value` | `u8` | 1 | Specific weapon/upgrade ID |
| `x` | `f32` | 4 | X position |
| `y` | `f32` | 4 | Y position |
| `active` | `u8` | 1 | Active status |

**Total: 15 bytes**

#### LevelBegin (Type 13) — Server→Client
Broadcast when a new level starts (all enemies defeated).

| Field | Type | Size | Description |
|-------|------|------|-------------|
| `levelNumber` | `u8` | 1 | New level number |

**Total: 1 byte**

#### Shield Packets (Types 28-30)
Shields use the same structure as monsters with separate packet types:
- **ShieldSpawn (28)**: 13 bytes, same layout as MonsterSpawn
- **ShieldState (29)**: 22 bytes, same layout as MonsterState  
- **ShieldDeath (30)**: 4 bytes, just entity ID

### Room/Lobby Packets (Types 14-27)
Used for multiplayer lobby management. See [include/rtype/common/Protocol.hpp](../include/rtype/common/Protocol.hpp) for detailed structures.

## Connection Flow

### Typical Client Session

```
1. Client connects UDP socket to server:port
2. Client immediately starts sending PlayerInput packets
3. Server responds with PlayerAssignment containing player ID
4. Server starts broadcasting game state:
   - PlayerState (all players)
   - MonsterSpawn/State/Death
   - BulletFired/State
   - PowerUpState
   - LevelBegin (when applicable)
5. Client continues sending PlayerInput at ~60 Hz
6. On disconnect: client sends Disconnect packet
```

### Minimal Client Loop

```cpp
// Pseudocode for a minimal client implementation

void clientLoop() {
    UDPSocket socket;
    socket.connect(serverAddress, serverPort);
    
    uint8_t myPlayerId = 0xFF;  // Invalid until assigned
    uint32_t sequenceNum = 0;
    
    while (running) {
        // 1. Send input to server
        PlayerInput input = gatherInput(myPlayerId);
        auto packet = serializePlayerInput(input, sequenceNum++, getTimestamp());
        socket.send(packet);
        
        // 2. Receive all available packets
        while (socket.hasData()) {
            auto data = socket.receive();
            PacketHeader header = deserializeHeader(data);
            
            switch (header.type) {
                case PlayerAssignment:
                    deserializePlayerAssignment(data, assignment);
                    myPlayerId = assignment.playerId;
                    break;
                    
                case PlayerState:
                    deserializePlayerState(data, state);
                    updatePlayerVisual(state);
                    break;
                    
                case MonsterSpawn:
                    deserializeMonsterSpawn(data, spawn);
                    createEnemyEntity(spawn);
                    break;
                    
                case MonsterState:
                    deserializeMonsterState(data, state);
                    updateEnemyVisual(state);
                    break;
                    
                case MonsterDeath:
                    deserializeMonsterDeath(data, death);
                    removeEnemyEntity(death.id);
                    break;
                    
                case BulletFired:
                    deserializeBulletFired(data, bullet);
                    createBulletEntity(bullet);
                    break;
                    
                // ... handle other packet types
            }
        }
        
        // 3. Render current game state
        render();
        
        // Maintain ~60 FPS
        sleep(16ms);
    }
}
```

## Server Expectations

### What the Server Does
- **Authoritative simulation**: All game logic runs on server
- **Broadcast state**: Sends complete game state every tick (~16ms)
- **Process inputs**: Reads PlayerInput, applies to simulation
- **Entity management**: Creates/destroys all entities
- **Collision detection**: Server-side only
- **Score tracking**: Server maintains all scores

### What the Client Should Do
- **Send inputs continuously**: Even if nothing changed
- **Trust server state**: Don't predict beyond interpolation
- **Handle packet loss gracefully**: Missing packets are normal
- **Create/destroy entities on command**: Follow spawn/death packets
- **Visual only**: All rendering, sounds, effects are client-side

- **Visual only**: All rendering, sounds, effects are client-side

### Tick Rate
- Server runs at **60 ticks per second** (16.67ms per tick)
- Client should send inputs at similar rate for responsiveness
- State broadcasts happen every tick

### Network Tolerance
- **No packet acknowledgment required**
- **Idempotent updates**: Each state packet is complete, not delta-based
- **Out-of-order OK**: Use sequence numbers to detect stale packets
- **Packet loss OK**: Next state packet will arrive shortly

## Client Implementation Guide

This section provides step-by-step guidance for implementing a compatible R-Type client.

### Phase 1: Network Layer

#### Required Components
1. **UDP Socket**: Non-blocking UDP socket for communication
2. **Serialization**: Implement BinaryWriter/BinaryReader (see [Protocol.hpp](../include/rtype/common/Protocol.hpp))
3. **Packet parser**: Header deserialization and type dispatch

#### Example Serialization (C++)
```cpp
// Serialize u16 to network byte order
void BinaryWriter::writeU16(uint16_t value) {
    uint16_t netValue = htons(value);  // Host to network short
    uint8_t* bytes = reinterpret_cast<uint8_t*>(&netValue);
    _buffer.push_back(bytes[0]);
    _buffer.push_back(bytes[1]);
}

// Serialize float as u32 bitcast
void BinaryWriter::writeF32(float value) {
    uint32_t bits;
    memcpy(&bits, &value, sizeof(float));
    writeU32(bits);
}

// Deserialize u16 from network byte order
bool BinaryReader::readU16(uint16_t& value) {
    if (_offset + 2 > _size) return false;
    uint16_t netValue;
    memcpy(&netValue, _data + _offset, 2);
    value = ntohs(netValue);  // Network to host short
    _offset += 2;
    return true;
}
```

#### Packet Deserialization Pattern
```cpp
PacketHeader deserializeHeader(const uint8_t* data, size_t size, bool& ok) {
    PacketHeader header;
    ok = false;
    
    if (size < 12) return header;  // Header is 12 bytes
    
    BinaryReader reader(data, size);
    if (!reader.readU16(reinterpret_cast<uint16_t&>(header.type))) return header;
    if (!reader.readU16(header.payloadSize)) return header;
    if (!reader.readU32(header.sequence)) return header;
    if (!reader.readU32(header.timestamp)) return header;
    
    ok = true;
    return header;
}

bool deserializePlayerState(const uint8_t* payload, size_t size, PlayerState& out) {
    if (size < 15) return false;  // Expected size
    
    BinaryReader reader(payload, size);
    if (!reader.readU8(out.player)) return false;
    if (!reader.readF32(out.x)) return false;
    if (!reader.readF32(out.y)) return false;
    if (!reader.readU8(out.hp)) return false;
    if (!reader.readU16(out.score)) return false;
    
    uint8_t aliveByte;
    if (!reader.readU8(aliveByte)) return false;
    out.alive = (aliveByte != 0);
    
    if (!reader.readU8(reinterpret_cast<uint8_t&>(out.powerUpType))) return false;
    
    return true;
}
```

### Phase 2: Entity Management

#### Entity Registry
Create a simple entity manager to store game objects:

```cpp
struct Entity {
    uint32_t id;
    float x, y;
    float vx, vy;
    uint8_t type;
    bool alive;
};

class EntityManager {
    std::unordered_map<uint32_t, Entity> entities;
    
public:
    void spawn(uint32_t id, float x, float y, uint8_t type) {
        entities[id] = {id, x, y, 0, 0, type, true};
    }
    
    void update(uint32_t id, float x, float y, float vx, float vy) {
        if (entities.contains(id)) {
            entities[id].x = x;
            entities[id].y = y;
            entities[id].vx = vx;
            entities[id].vy = vy;
        }
    }
    
    void remove(uint32_t id) {
        entities.erase(id);
    }
    
    Entity* get(uint32_t id) {
        auto it = entities.find(id);
        return it != entities.end() ? &it->second : nullptr;
    }
};
```

#### Player State Management
```cpp
struct Player {
    uint8_t id;
    float x, y;
    uint8_t hp;
    uint16_t score;
    bool alive;
    uint8_t weaponType;
};

std::array<Player, 4> players;  // Support up to 4 players
```

### Phase 3: Rendering

#### Sprite Management
You'll need sprites for:
- **Player ships** (4 colors/skins)
- **Enemies** (multiple types)
- **Bullets** (different weapon types)
- **Power-ups**
- **Shields**
- **Backgrounds/starfield**

#### Recommended Sprite Sizes
- Player: 32×32 or 48×48
- Enemies: 32×32 to 64×64
- Bullets: 8×8 to 16×16
- Power-ups: 24×24

See [gameplay_guide.md](gameplay_guide.md) for entity type descriptions.

#### Rendering Loop
```cpp
void render() {
    // 1. Draw background/starfield (scrolling)
    renderBackground();
    
    // 2. Draw all entities
    for (auto& [id, entity] : entities) {
        if (entity.alive) {
            renderSprite(getSpriteForType(entity.type), entity.x, entity.y);
        }
    }
    
    // 3. Draw bullets
    for (auto& [id, bullet] : bullets) {
        if (bullet.active) {
            renderBullet(bullet.weaponType, bullet.x, bullet.y);
        }
    }
    
    // 4. Draw players
    for (auto& player : players) {
        if (player.alive) {
            renderPlayer(player.id, player.x, player.y);
        }
    }
    
    // 5. Draw UI (health, score)
    renderUI();
}
```

### Phase 4: Input Handling

#### Input Gathering
```cpp
PlayerInput gatherInput(uint8_t playerId) {
    PlayerInput input;
    input.player = playerId;
    input.up = isKeyPressed(KEY_UP) || isKeyPressed(KEY_W);
    input.down = isKeyPressed(KEY_DOWN) || isKeyPressed(KEY_S);
    input.left = isKeyPressed(KEY_LEFT) || isKeyPressed(KEY_A);
    input.right = isKeyPressed(KEY_RIGHT) || isKeyPressed(KEY_D);
    input.fire = isKeyPressed(KEY_SPACE);
    input.swapWeapon = isKeyPressed(KEY_TAB);
    return input;
}
```

#### Input Serialization
```cpp
std::vector<uint8_t> serializePlayerInput(const PlayerInput& input, 
                                          uint32_t sequence, 
                                          uint32_t timestamp) {
    BinaryWriter payload;
    payload.writeU8(input.player);
    payload.writeU8(input.up ? 1 : 0);
    payload.writeU8(input.down ? 1 : 0);
    payload.writeU8(input.left ? 1 : 0);
    payload.writeU8(input.right ? 1 : 0);
    payload.writeU8(input.fire ? 1 : 0);
    payload.writeU8(input.swapWeapon ? 1 : 0);
    
    return serializePacket(PacketType::PlayerInput, sequence, timestamp,
                          payload.data().data(), payload.data().size());
}

std::vector<uint8_t> serializePacket(PacketType type, uint32_t sequence,
                                     uint32_t timestamp,
                                     const uint8_t* payload, size_t payloadSize) {
    BinaryWriter writer;
    writer.writeU16(static_cast<uint16_t>(type));
    writer.writeU16(static_cast<uint16_t>(payloadSize));
    writer.writeU32(sequence);
    writer.writeU32(timestamp);
    writer.writeBytes(payload, payloadSize);
    return writer.moveData();
}
```

### Phase 5: Audio (Optional)

Add sound effects for:
- Player fire
- Enemy death
- Player hit/death
- Power-up collection
- Level complete

See [sound_system.md](sound_system.md) for audio system details.

### Testing Your Client

#### Minimal Viable Client Checklist
- [ ] Connect to server via UDP
- [ ] Receive PlayerAssignment and store ID
- [ ] Send PlayerInput packets continuously
- [ ] Receive and parse PlayerState packets
- [ ] Render player positions
- [ ] Receive MonsterSpawn/State/Death
- [ ] Render enemies
- [ ] Handle BulletFired/BulletState
- [ ] Render projectiles
- [ ] Display player health and score

#### Advanced Features
- [ ] PowerUp visualization
- [ ] Level transitions (LevelBegin packets)
- [ ] Multiplayer (multiple PlayerState packets)
- [ ] Room/lobby system (Types 14-27)
- [ ] Shield entities (Types 28-30)
- [ ] Interpolation for smooth movement
- [ ] Particle effects and animations

## Serialization Helpers

### Reference Implementation
Complete serialization code is in:
- **Header**: [include/rtype/common/Protocol.hpp](../include/rtype/common/Protocol.hpp)
- **Implementation**: [src/common/protocol/Protocol.cpp](../src/common/protocol/Protocol.cpp)

The reference implementation provides:
- `BinaryWriter` and `BinaryReader` classes
- Serialize/deserialize functions for every packet type
- Bounds checking and error handling
- Network byte order conversion

You can use these directly if implementing in C++, or use them as reference for other languages.

## Error Handling

### Packet Validation
All packet processing follows this validation chain:

1. **Size check**: Packet must be at least 12 bytes (header size)
2. **Header parsing**: Extract type, payloadSize, sequence, timestamp
3. **Payload size validation**: Remaining bytes must match payloadSize field
4. **Type validation**: Unknown types are ignored (forward compatibility)
5. **Payload parsing**: Type-specific deserialization with bounds checking

**Any failure at any step causes the packet to be silently dropped.**

### Client-Side Error Handling

#### Network Errors
```cpp
// Handle socket errors gracefully
if (socket.hasError()) {
    if (socket.wouldBlock()) {
        // No data available, continue
    } else if (socket.connectionReset()) {
        // Server disconnected, show error
        showError("Lost connection to server");
        return;
    }
}
```

#### Missing Player Assignment
```cpp
// Wait for assignment before processing game state
if (myPlayerId == 0xFF) {
    // Not assigned yet, only process PlayerAssignment packets
    if (header.type == PacketType::PlayerAssignment) {
        // Process assignment
    }
    // Ignore all other packets until assigned
    return;
}
```

#### Entity Desync
```cpp
// Handle updates for non-existent entities
void updateMonster(uint32_t id, float x, float y) {
    auto* monster = entityManager.get(id);
    if (!monster) {
        // Monster not in our registry, might have missed spawn packet
        // Create placeholder or log warning
        logWarning("Received update for unknown monster " + std::to_string(id));
        return;
    }
    monster->x = x;
    monster->y = y;
}
```

### Server-Side Error Handling

#### Malformed Input
- Invalid player IDs are ignored
- Out-of-range movement values are clamped
- Unknown input flags are ignored

#### Client Timeout
- Server should track last received packet time per client
- After ~5 seconds without packets, consider client disconnected
- Broadcast Disconnect packet to other clients

### Common Issues and Solutions

| Issue | Cause | Solution |
|-------|-------|----------|
| "No PlayerAssignment received" | UDP packet loss | Client should timeout after 5s and reconnect |
| "Entities jumping around" | Packet loss | Implement interpolation between state updates |
| "Player ID mismatch" | Sending before assignment | Wait for PlayerAssignment before sending inputs |
| "Unknown entity updates" | Missed spawn packet | Create placeholder or ignore updates |
| "Deserialization fails" | Endianness mismatch | Always use network byte order (htons/htonl) |

## Security Notes

### Current Limitations
- **No authentication**: Any client can connect
- **No encryption**: All data sent in plain text
- **No rate limiting**: Client can spam packets
- **No validation**: Server trusts client input timing

### Potential Improvements
These could be added in future versions:
- Simple authentication token in Handshake packet
- Rate limiting (drop packets from clients sending >100/sec)
- Replay detection using sequence numbers
- Input validation (velocity bounds, fire rate limits)
- DDoS protection (IP-based connection limits)

### Current Mitigations
- **Bounds checking**: All deserializers validate sizes
- **Server authority**: Client inputs are suggestions, server decides outcomes
- **Type safety**: Unknown packet types are ignored safely
- **Fixed allocations**: No dynamic allocation based on packet content

## Future Protocol Extensions

The protocol is designed for extension without breaking compatibility:

### Adding New Packet Types
1. Add new PacketType enum value (use next available number)
2. Define payload structure in Protocol.hpp
3. Implement serialize/deserialize functions
4. Old clients will ignore unknown types

### Adding Fields to Existing Packets
**Breaking change** — requires protocol version bump. Alternatives:
- Create new packet type with extended fields
- Use reserved/unused bits for new flags
- Increase payloadSize and make old fields optional

### Protocol Versioning (Future)
Consider adding version field to Handshake packet:
```cpp
struct Handshake {
    uint16_t protocolVersion;  // e.g., 1
    uint16_t clientVersion;    // e.g., 100 for "1.0.0"
};
```

Server could reject incompatible clients or negotiate features.

## Additional Resources

- **Component Definitions**: See [include/rtype/common/Components.hpp](../include/rtype/common/Components.hpp) for ECS component structures
- **Entity Types**: See [gameplay_guide.md](gameplay_guide.md) for monster/weapon/power-up type IDs
- **Configuration**: See [configuration_guide.md](configuration_guide.md) for server settings (tick rate, spawn rates, etc.)
- **Architecture**: See [architecture.md](architecture.md) for system design and data flow diagrams

---

**Document Version**: 2.0  
**Protocol Version**: 1 (implicit, no version field yet)  
**Last Updated**: January 2026
