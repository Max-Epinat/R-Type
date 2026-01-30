# API Reference

> **Related Documentation:**
> - [Developer Guide](developer_guide.md) — Code examples and contribution guide
> - [Protocol](protocol.md) — Network packet definitions
> - [Architecture](architecture.md) — System design overview
> - [Components](../include/rtype/common/Components.hpp) — ECS component definitions

This document summarizes the public headers in `include/rtype/**` and how they are intended to be consumed by tooling, gameplay code, or potential integrations. All namespaces follow the `rtype::*` convention.

## Shared / Common Layer
| Symbol | Location | Responsibility | Key Calls |
| --- | --- | --- | --- |
| `config::GameConfig` | [include/rtype/common/GameConfig.hpp](include/rtype/common/GameConfig.hpp) | Aggregates gameplay, render, network, and audio knobs loaded from `config/game.ini`. | `loadFromFile(path)`, `saveToFile(path)`, `getScrollVelocity(vx, vy)`, `getSpawnPosition(x, y, rand)`.
| `config::GameplayConfig` & friends | same | Plain structs consumed directly by game logic for deterministic parameters. | Field access; no behavior beyond defaults.
| `net::PacketHeader` | [include/rtype/common/Protocol.hpp](include/rtype/common/Protocol.hpp) | Standard header prepended to every UDP payload (type, size, sequence, timestamp). | Construct directly or via `serializePacket`.
| `net::BinaryWriter` / `net::BinaryReader` | same | Utility classes to push/pop primitive values in network byte order. | `writeU8`, `writeF32`, `readU16`, etc.
| `net::serialize*` / `net::deserialize*` | same | Strongly typed serializers/deserializers for each packet described in [docs/protocol.md](docs/protocol.md). | e.g., `serializePlayerInput`, `deserializePowerUpState`.
| Component structs (`Transform`, `Velocity`, `Health`, etc.) | [include/rtype/common/Components.hpp](include/rtype/common/Components.hpp) | Shared ECS data layout for both binaries. | Direct member access; server mutates, client reads.

## Engine (ECS)
| Symbol | Location | Responsibility | Key Calls |
| --- | --- | --- | --- |
| `engine::Registry` | [include/rtype/engine/Registry.hpp](include/rtype/engine/Registry.hpp) | Minimal ECS registry that stores components in sparse hash maps keyed by `EntityId`. | `createEntity()`, `destroyEntity(id)`, `emplace<Component>(id, args...)`, `get<Component>(id)`, `each<Component>(fn)`.
| `engine::ISystem` | same | Interface for extending the pipeline with custom systems. | Implement `update(deltaSeconds, Registry&)`.
| `engine::SystemPipeline` | same | Holds ordered systems and dispatches them each tick. | `addSystem(unique_ptr<ISystem>)`, `update(deltaSeconds, registry)`.

### Usage Notes
- Registries are owned per runtime (`GameLogicHandler` on the server, not constructed on the client except for potential local simulations).
- `each<Component>` iterates by value reference; avoid erasing components inside the callback—stage removal and call `destroyEntity` after iteration instead.

## Server Components
| Symbol | Location | Responsibility | Key Calls |
| --- | --- | --- | --- |
| `server::GameServer` | [include/rtype/server/GameServer.hpp](include/rtype/server/GameServer.hpp) | Entry point for the authoritative executable: socket lifecycle, receive queue, player admission, ticking, and broadcasting. | `start()`, `stop()`, private helpers `scheduleReceive()`, `handlePacket(...)`, `broadcastStates(...)`.
| `server::GameLogicHandler` | [include/rtype/server/GameLogicHandler.hpp](include/rtype/server/GameLogicHandler.hpp) | Owns the ECS registry and gameplay systems (movement, projectiles, spawn logic). | `spawnPlayer(playerId)`, `manageInputs(net::PlayerInput, entity)`, `shootProjectile(entity)`, `updateGame(dt)`, `markDestroy(id)`, `getEntityDestructionSet()`.
| `server::ClientHandler` | [include/rtype/server/ClientHandler.hpp](include/rtype/server/ClientHandler.hpp) | Tracks per-player metadata: UDP endpoint, assigned entity, last heartbeat. | `updateLastSeen(timestamp)`, `getEndpoint()`, `getEntityId()`.

### Typical Flow
1. `GameServer::scheduleReceive` pushes packets into `_rxQueue` (protected by `_rxMutex`).
2. `GameServer::updateGameLoop` hands `net::PlayerInput` events to `GameLogicHandler::manageInputs`.
3. After the tick, `broadcastStates` serializes snapshots with `rtype_common` helpers and uses the stored endpoints from `ClientHandler`.

## Client Components
| Symbol | Location | Responsibility | Key Calls |
| --- | --- | --- | --- |
| `client::GameClient` | [include/rtype/client/GameClient.hpp](include/rtype/client/GameClient.hpp) | Coordinates SFML window lifecycle, UDP socket, and replicated entity maps. | `run()`, `networkReceive()`, `handlePacket(...)`, `sendInput(net::PlayerInput&)`.
| `client::SFMLRender` | [include/rtype/client/SFMLRender.hpp](include/rtype/client/SFMLRender.hpp) | Loads textures/audio, draws the parallax starfield, and renders all remote entities. | `SetconfigAndWindow(config)`, `renderFrame(players, monsters, bullets, powerUps)`, `IsInputPressed(playerId)`, `LoadSounds()`, `buildStarfield()`.
| Remote DTOs (`RemotePlayer`, `RemoteMonster`, etc.) | [include/rtype/client/RemoteDisplay.hpp](include/rtype/client/RemoteDisplay.hpp) | Value types stored inside the replicated maps and consumed by the renderer. | Direct member access.

### Input / Rendering Loop
- `GameClient::run` continuously polls `SFMLRender::IsInputPressed`, forwards it via `sendInput`, then calls `renderFrame` using a snapshot of the replicated state (copy constructed under `_stateMutex`).
- `SFMLRender::renderFrame` orchestrates background drawing, entity sprites (with automatic sprite atlas loading), and HUD logging.
- Audio cues (e.g., `playShootSound`) are triggered locally to provide immediate feedback even before the server confirms the action.

## Extending the Codebase
- **New packet type**: add the struct + serialize/deserialize helpers in [include/rtype/common/Protocol.hpp](include/rtype/common/Protocol.hpp) and [src/common/protocol/Protocol.cpp](src/common/protocol/Protocol.cpp), bump the enum, and handle it in both `GameServer::handlePacket` and `GameClient::handlePacket`.
- **New component/system**: declare the component in [include/rtype/common/Components.hpp](include/rtype/common/Components.hpp), register behavior inside `GameLogicHandler`, and replicate state if the client needs it.
- **New render asset**: place the sprite under [src/assets](src/assets), register it in `SFMLRender::loadSpriteAssets`, and reference it via the sprite key from gameplay replication.

Cross-reference this file alongside [docs/architecture.md](docs/architecture.md) when onboarding new contributors or preparing for oral defenses.
