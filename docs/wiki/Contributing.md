# Contributing

Thanks for helping improve the R-Type remake! This page documents the workflow we use across engine, server, and client changes.

## Development Environment
- CMake 3.20+
- C++20 compiler:
  - **Linux**: GCC 12+ or Clang 15+
  - **macOS**: Clang 15+ (via Xcode Command Line Tools)
  - **Windows**: Visual Studio 2022 (MSVC 19.36+)
- Standalone Asio headers
- SFML 2.6 (client only)
- **Windows users**: Use Conan 2.0+ for dependency management
- Optional: `conan` or `vcpkg` if you prefer package managers over system packages

### Platform-Specific Setup

**Windows/MSVC:**
1. Install Visual Studio 2022 with C++ workload
2. Install Conan: `pip install conan`
3. Configure Conan profile: `conan profile detect`
4. Install dependencies: `conan install . --build=missing`
5. Build with presets:
   ```bash
   cmake --preset conan-default
   cmake --build --preset conan-debug
   ```

**Linux/macOS:**
```bash
# Install dependencies via package manager
# Ubuntu/Debian: sudo apt install libsfml-dev libasio-dev
# macOS: brew install sfml asio

# Standard CMake build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Known Platform Issues

**MSVC C++11 Standard Compliance:**
- `std::uniform_int_distribution` only accepts short, int, long, long long (not uint8_t per C++11 standard)
- ✅ **Correct**: `std::uniform_int_distribution<int>(0, 255)` with `static_cast<uint8_t>(result)`
- ❌ **Wrong**: `std::uniform_int_distribution<uint8_t>(0, 255)`
- **Files affected**: GameLogicHandler.cpp, PowerUpSystem.cpp (already fixed)
- See [developer_guide.md](../developer_guide.md) for complete Windows troubleshooting

## Branch & PR Flow
1. Create a feature branch from `main`.
2. Build both targets when touching shared code:
   ```bash
   cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
   cmake --build build --target rtype_server rtype_client -j
   ```
3. Run the server + at least one client locally to validate protocol changes.
4. Update documentation (README + wiki pages) whenever behavior, config fields, or packet types change.
5. Open a pull request that references any related issues.

## Coding Guidelines
- Stick to modern C++ (no raw new/delete; prefer RAII).
- Follow the warning set enforced by `-Wall -Wextra -Wpedantic`.
- Keep shared headers (`include/rtype/**`) free of SFML-specific code so the server stays headless.
- Use `config::GameConfig` instead of hardcoding gameplay constants.
- Document non-obvious logic with short comments—especially around networking, threading, and ECS destruction queues.

## Testing Checklist
- **Server:** Ensure no packets are dropped due to oversize `payloadSize`; watch console logs for `[server]` warnings.
- **Client:** Verify the console still prints entity counts every 60 frames and that starfield scrolling remains smooth.
- **Config:** If you add new `.ini` fields, bump the wiki + `docs/configuration_guide.md`.

## Documentation Duties
- Update the relevant wiki page (Architecture, Networking, Configuration, etc.) when touching that subsystem.
- Keep diagrams in sync—Mermaid blocks render directly on GitHub wikis.

Feel free to open issues for large refactors before diving in; planning upfront keeps the binary protocol stable for downstream clients.
