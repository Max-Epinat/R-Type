# Modules and API

This page condenses the public headers from `include/rtype/**`. Use it when onboarding teammates or preparing for oral defenses.

## Shared Layer (`rtype_common`)
| Symbol | Role | Highlights |
| --- | --- | --- |
| `config::GameConfig` | Runtime settings loaded from `config/game.ini`. | `loadFromFile`, `getScrollVelocity`, `getSpawnPosition`, `isOffScreen`. |
| `config::GameplayConfig/RenderConfig/...` | Plain structs used directly by gameplay/rendering code. | Field access only. |
| `net::PacketHeader` & `net::PacketType` | Binary protocol metadata. | `deserializeHeader`, `serializePacket`. |
| `net::BinaryWriter` / `net::BinaryReader` | Encode/decode primitives in network byte order. | `writeU8/U16/U32/F32`, `readU8/U16/U32/F32`. |
| `net::serialize*` helpers | Strongly typed payload builders. | `serializePlayerInput`, `serializeMonsterSpawn`, etc. |
| Component structs | `Transform`, `Velocity`, `Health`, `PlayerComponent`, `MonsterComponent`, `Projectile`, `PowerUp`, `FireCooldown`. |

## Engine (`rtype_engine`)
| Symbol | Role | Highlights |
| --- | --- | --- |
| `engine::Registry` | Minimal ECS registry storing per-component unordered_maps keyed by `EntityId`. | `createEntity`, `destroyEntity`, `emplace<Component>`, `get<Component>`, `each<Component>`. |
| `engine::ISystem` | Interface for deterministic systems. | Implement `update(deltaSeconds, Registry&)`. |
| `engine::SystemPipeline` | Ordered list of systems executed per tick. | `addSystem`, `update`. |

## Server (`rtype_server`)
| Symbol | Role | Highlights |
| --- | --- | --- |
| `server::GameServer` | Orchestrates networking, matchmaking, tick loop, and broadcasting. | `start`, `stop`, handles `_rxQueue`, `_clients`, `_sequence`. |
| `server::GameLogicHandler` | Owns the registry, spawns players/monsters, resolves collisions, handles projectiles. | `spawnPlayer`, `manageInputs`, `managePlayerMovement`, `shootProjectile`, `updateGame`, `markDestroy`. |
| `server::ClientHandler` | Tracks endpoint ↔ player/entity mapping. | `updateLastSeen`, `getEndpoint`, `getEntityId`. |

## Client (`rtype_client`)
| Symbol | Role | Highlights |
| --- | --- | --- |
| `client::GameClient` | Main loop: loads config, manages SFML window, spawns network thread, mirrors server state. | `run`, `networkReceive`, `handlePacket`, `sendInput`. |
| `client::SFMLRender` | Rendering/audio helper with sprite atlas loading, starfield builder, and HUD logging. | `SetconfigAndWindow`, `renderFrame`, `DrawPlayers/Monsters/PowerUps/Bullets`, `LoadSounds`, `buildStarfield`, `IsInputPressed`. |
| `client::Remote*` DTOs | `RemotePlayer`, `RemoteMonster`, `RemoteBullet`, `RemotePowerUp`. | Simple structs stored in replicated maps. |

## Extension Recipes
- **New Packet:** Add struct + serializer/deserializer in `Protocol.hpp/.cpp`, increment `PacketType`, handle it on both server (`handlePacket`) and client (`handlePacket`).
- **New Component/System:** Declare the component in `Components.hpp`, add logic inside `GameLogicHandler`, and (if needed) replicate the component to clients via a new packet.
- **New Sprite:** Drop asset under `src/assets/sprites`, register it in `SFMLRender::loadSpriteAssets`, and reference it via the sprite key when rendering.

Cross-link: [[Architecture]] for diagrams, [[Networking]] for the protocol table, [[Configuration]] for runtime knobs.

## Related Documentation
- [api_reference.md](../api_reference.md) - Complete API reference
- [protocol.md](../protocol.md) - Complete protocol specification (649 lines) with all packet types and implementation guide
- [[Architecture]] - System design and module interactions
- [[Networking]] - Protocol overview and reliability strategy
- [developer_guide.md](../developer_guide.md) - How to extend the system with new components/packets
