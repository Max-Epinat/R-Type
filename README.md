# R-Type — Multiplayer Network Game (C++)

Authoritative remake of the 1987 classic built with modern C++20, a lightweight ECS engine, and a UDP-first protocol. The workspace provides both a headless server simulation and an SFML client renderer that connect through a strict binary protocol.

## Highlights
- Client/server split with an authoritative simulation loop and thin presentation client.
- Binary UDP protocol with explicit headers, serialization helpers, and bounds checking.
- Custom ECS registry (`rtype_engine`) shared by both targets for deterministic gameplay code.
- Runtime configuration via [`config/game.ini`](config/game.ini) covering gameplay, rendering, audio, and network knobs.
- Portable build powered by CMake 3.20+, standalone Asio, and SFML 2.6 (for the client target).

## Project Layout
| Path | Description |
| --- | --- |
| [src/server](src/server) | Authoritative networking stack, player/session management, and gameplay loop. |
| [src/client](src/client) | SFML renderer, audio, and local input collection. |
| [src/common](src/common) | Shared protocol serializer and configuration loader. |
| [src/engine](src/engine) | Minimal ECS registry plus system pipeline. |
| [include/rtype](include/rtype) | Public headers for the four modules. |
| [docs](docs) | Architecture notes, configuration guide, protocol spec, accessibility checklist, API reference. |
| [config](config) | Runtime `.ini` files consumed by both binaries. |
| [bin](bin) | Default output directory for built artifacts and copied assets. |

## Prerequisites
- CMake 3.20+
- A C++20 compiler (GCC 12+, Clang 15+, or MSVC 19.36+ / Visual Studio 2022)
- [Asio](https://think-async.com/) (standalone) — automatically resolved via `find_package(asio)` or pkg-config fallback
- [SFML 2.6](https://www.sfml-dev.org/) components `system`, `window`, `graphics`, `audio`, `network` (client target only)
- Ninja/Make/MSBuild (any generator supported by CMake)

> **Platform-specific notes:**
> - **Linux**: Install `asio` and `sfml` through your package manager (`sudo apt install libasio-dev libsfml-dev`) before configuring.
> - **Windows**: Use Conan or vcpkg to install dependencies. The project includes a [conanfile.txt](conanfile.txt) for automated dependency resolution.

## Build & Run

### Linux / macOS
```bash
git clone <repo-url>
cd rtype
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

### Windows (Visual Studio)
```powershell
git clone <repo-url>
cd rtype

# Option 1: Use automated build script (recommended)
.\scripts\build.ps1

# Option 2: Manual build
# Install dependencies (both Debug and Release for complete setup)
conan install . --build=missing -s build_type=Debug
conan install . --build=missing -s build_type=Release

# Configure and build
cmake --preset conan-default
cmake --build --preset conan-release
```

**Note**: The build script (`build.ps1`) automatically installs both Debug and Release dependencies, preventing the common issue where debug builds fail due to missing debug DLLs.

Executable targets are emitted to [bin](bin):

- Launch the authoritative server:
  ```bash
  ./bin/rtype_server          # Linux
  .\bin\Release\rtype_server.exe  # Windows
  ```
- Launch a rendering client (same or different host):
  ```bash
  ./bin/rtype_client          # Linux
  .\bin\Release\rtype_client.exe  # Windows
  ```

The client reads the server endpoint from the configuration file and starts sending input frames immediately. Multiple clients can connect simultaneously as long as you stay under `Network.MaxPlayers` (default 4).

## Runtime Configuration
All gameplay, rendering, audio, and network parameters are loaded from [`config/game.ini`](config/game.ini). See [docs/configuration_guide.md](docs/configuration_guide.md) for every field plus ready-made presets (easy, hard, vertical scrolling, etc.). Edits are hot-reloaded at startup—no recompilation required.

Key groups:
- `[Gameplay]`: scroll direction, spawn cadence, weapon tuning, world bounds.
- `[Render]`: window size, starfield density, palette, bullet visuals.
- `[Network]`: host/port, socket buffer sizes, receive limits.
- `[Audio]`: per-channel volume and enable flags.

## Documentation Map
- [docs/building_and_running.md](docs/building_and_running.md) — comprehensive build instructions for all platforms with troubleshooting.
- [docs/architecture.md](docs/architecture.md) — diagrams, runtime loops, threading, deployment targets.
- [docs/developer_guide.md](docs/developer_guide.md) — ECS implementation details, system development, and codebase navigation.
- [docs/gameplay_guide.md](docs/gameplay_guide.md) — controls, weapons, power-ups, enemies, and gameplay mechanics.
- [docs/api_reference.md](docs/api_reference.md) — module-by-module class reference and integration points.
- [docs/protocol.md](docs/protocol.md) — binary packet catalog and serialization constraints.
- [docs/configuration_guide.md](docs/configuration_guide.md) — editable runtime settings and preset configurations.
- [docs/accessibility.md](docs/accessibility.md) — current compliance checklist and future actions.

## Roadmap (snapshot)
- Improve snapshot delta compression and sequence-based reconciliation.
- Expand monster behaviors and add scripted encounters.
- Introduce authoritative power-up lifecycle syncing.
- Harden client recovery (reconnect, assignment timeouts).
- Build automated soak tests for latency, jitter, and packet loss scenarios.

## Platform Support
The project builds and runs on:
- **Linux** (GCC 12+, Clang 15+)
- **Windows** (MSVC 19.36+ / Visual Studio 2022)

Recent compatibility improvements include MSVC-specific fixes for standard library compliance with random number generation APIs.

## Contributors

This project was developed by a team of 5 contributors, each specializing in different aspects of the engine and helping their teammates when needed:

### ECS Architecture
**Rares Dragomir**

Designed and implemented the Entity Component System (ECS) registry, system pipeline, and component management that powers the game's deterministic simulation.

### Network Protocol
**Raphael Grissonnanche, Rares Dragomir, Max Epinat**

Built the UDP-based networking layer with binary serialization, packet handling, client-server synchronization, and robust error recovery mechanisms.

### Multi-Platform Build
**Max Epinat, Rares Dragomir**

Configured the CMake build system, managed cross-platform compatibility (Linux/Windows), integrated Conan/vcpkg dependency resolution, and automated build scripts.

### Client & Graphical Assets
**Arnaud Jouan, Max Epinat, Raphael Grissonnanche**

Developed the SFML renderer, sprite management system, animation framework, visual effects, and created/integrated all graphical assets and sound effects.

### Configuration System
**Rares Dragomir, Max Epinat, Raphael Grissonnanche**

Designed the flexible INI-based configuration architecture, implemented hot-reloadable settings, and created multiple game presets including the Space Invaders variant.

### Gameplay Mechanics
**Ali Atmani, Max Epinat, Arnaud Jouan**

Implemented core gameplay features including player controls, enemy behaviors, weapon systems, power-ups, and level progression mechanics.

Contributions are welcome—feel free to open issues for gameplay ideas, protocol changes, or tooling enhancements.
