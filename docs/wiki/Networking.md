# Networking

The multiplayer stack uses UDP exclusively, with a compact binary protocol guarded by explicit headers and sequence counters.

## Packet Layout
```
struct PacketHeader {
    uint16_t type;        // net::PacketType enum
    uint16_t payloadSize; // bytes after the header
    uint32_t sequence;    // monotonic per-sender
    uint32_t timestamp;   // ms since sender start
}
```
- All integers are serialized in network byte order.
- Floats are bitcast to `uint32_t` before serialization, then reconstructed on the receiver.

## Message Catalog

**Complete protocol specification** with all 30 packet types, byte layouts, and client implementation guide: [protocol.md](../protocol.md) (649 lines)

### Common Packets (excerpt)
| Type | Name | Payload |
| --- | --- | --- |
| 1 | `ConnectionRequest` | Player name (32 bytes max) |
| 2 | `PlayerInput` | `u8 player`, flags (`up/down/left/right/fire`) |
| 3 | `PlayerState` | Player id, position, HP, score, alive flag |
| 4 | `MonsterSpawn` | Entity id, position, monster type |
| 5 | `MonsterState` | Entity id, type, position, alive |
| 8 | `BulletFired` | Entity id, owner, position, velocity, source flag |
| 9 | `BulletState` | Entity id, position, active flag |
| 11 | `PlayerAssignment` | Player id assigned by server |
| 12 | `PowerUpState` | Entity id, type, value, position, active |
| ... | ... | See protocol.md for complete list including level progression, boss mechanics, etc. |

See `include/rtype/common/Protocol.hpp` for the complete list and helpers.

## Serialization Helpers
- `net::BinaryWriter` / `net::BinaryReader` handle primitive encoding.
- `serializePlayerInput`, `serializeMonsterState`, etc., build `[header][payload]` buffers.
- `deserializePayload` validates `payloadSize` before exposing the buffer to higher layers.

## Client → Server Flow
1. `GameClient::IsInputPressed` builds a `net::PlayerInput` every frame.
2. `sendInput` serializes the struct and calls `socket.send_to` targeting the server endpoint specified in `config/game.ini`.
3. Disconnect packets are sent when closing the client window to free the slot early.

## Server → Client Flow
1. When a client connects, the server replies with `PlayerAssignment` so the client can colorize its ship.
2. Each simulation tick, the server broadcasts the latest entity snapshots (`PlayerState`, `MonsterState`, `BulletState`, `PowerUpState`) plus spawn/death events.
3. Clients merge the data into replicated maps guarded by `_stateMutex` and immediately draw the new frame.

## Reliability Strategy
- UDP keeps latency low and avoids head-of-line blocking.
- Each sender increments `sequence` per packet; receivers can drop stale updates if they ever arrive out of order (future interpolation work can leverage this).
- Packets failing header/payload validation are discarded silently to prevent crashes or buffer overreads.

Future work: introduce delta compression, interpolation, and latency compensation once gameplay elements expand.

## Related Documentation
- [protocol.md](../protocol.md) - **Complete protocol specification** (649 lines) with all 30 packet types, byte layouts, serialization patterns, and client implementation guide sufficient to rebuild a client from scratch
- [[Architecture]] - System design and threading model
- [[Modules and API]] - API reference for Protocol.hpp helpers
- [developer_guide.md](../developer_guide.md) - Development setup and testing guidelines
