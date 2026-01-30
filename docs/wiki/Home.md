# R-Type Wiki

Welcome to the R-Type multiplayer remake documentation hub. This wiki mirrors the project docs so teammates and reviewers can navigate the system without cloning the repository.

## TL;DR
- **Tech stack:** C++20, standalone Asio, SFML 2.6, custom ECS engine.
- **Architecture:** Authoritative UDP server + thin SFML client driven by shared protocol/components.
- **Docs map:**
  - [[Building and Running]] — Comprehensive build guide
  - [[Gameplay Guide]] — Controls, weapons, strategy
  - [[Configuration]] — Customize gameplay, rendering, audio
  - [[Architecture]] — System design and module responsibilities
  - [[Networking]] — UDP protocol and packet specifications
  - [[Sound System]] — Audio implementation details
  - [[Modules and API]] — Class-level API reference
  - [[Contributing]] — Development environment and workflow
  - [[FAQ and Troubleshooting]] — Common issues and solutions
  - [[Accessibility]] — Accessibility features and guidelines

## Build & Run

### Linux/macOS
```bash
git clone <repo-url>
cd rtype
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

### Windows (Visual Studio 2022 + Conan)
**Prerequisites:**
- Visual Studio 2022 with C++20 support
- CMake 3.20+
- Conan 2.0+

```bash
# Install dependencies with Conan
conan install . --build=missing

# Configure and build
cmake --preset conan-default
cmake --build --preset conan-release
```

**Platform Notes:**
- **Windows/MSVC**: Use Conan for dependency management (SFML, Asio). Some C++11 standard requirements are stricter (e.g., `std::uniform_int_distribution` only accepts int types, not uint8_t).
- **Linux/macOS**: Install SFML and Asio via system package manager or build from source.

Artifacts land in `bin/`:
- Server: `./bin/rtype_server`
- Client: `./bin/rtype_client`

Update `config/game.ini` if the server runs on another host/port.

## Project Layout
| Path | Purpose |
| --- | --- |
| `src/server` | Authoritative gameplay, networking, and broadcast loop |
| `src/client` | SFML renderer, input/audio, client networking |
| `src/engine` | Minimal ECS registry + systems pipeline |
| `src/common` | Binary protocol + INI configuration loader |
| `include/rtype` | Public headers for each module |
| `config` | Runtime `.ini` files |
| `docs` | Source-of-truth documentation (mirrored here)

## Workflow Cheatsheet
1. Start `rtype_server` on the host machine.
2. Launch one or more `rtype_client` instances (same or remote host).
3. Watch the console for state snapshots every 60 frames.
4. Tune gameplay/render/audio/network knobs via `config/game.ini`.

Use the sidebar links (or the list above) to dive deeper into the codebase.
