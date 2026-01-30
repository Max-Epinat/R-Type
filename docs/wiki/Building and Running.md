# Building and Running

This guide covers building and running the R-Type project on all supported platforms.

## Prerequisites

### Required Tools
- **C++ Compiler**: 
  - Linux: GCC 12+ or Clang 15+
  - macOS: Clang 15+ (via Xcode)
  - Windows: Visual Studio 2022 (MSVC 19.36+)
- **CMake**: 3.20+
- **Conan**: 2.0+ (Windows recommended, optional on Linux/macOS)

### Platform-Specific Setup

#### Linux (Ubuntu/Debian)
```bash
# Install build tools
sudo apt update
sudo apt install build-essential cmake git python3-pip

# Option 1: System packages (recommended)
sudo apt install libasio-dev libsfml-dev

# Option 2: Conan package manager
pip3 install conan
```

#### macOS
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install dependencies via Homebrew
brew install cmake sfml asio

# Or use Conan
pip3 install conan
```

#### Windows
**Required:**
- **Visual Studio 2022** with "Desktop development with C++"
- **CMake 3.20+** (add to PATH during install)
- **Python 3.7+** (check "Add Python to PATH")
- **Conan 2.0+**: `pip install conan`

**Verify installation:**
```powershell
cmake --version
python --version
conan --version
```

**Common Issues:**

If `conan` command not found after installation:
```powershell
# Solution 1: Use py launcher
py -m conan --version

# Solution 2: Add Python Scripts to PATH
# The path is shown in pip install warning
# Example: C:\Users\<You>\AppData\Local\Programs\Python\Python312\Scripts
# Add to Environment Variables → Path
```

See [building_and_running.md](../building_and_running.md) for detailed Windows troubleshooting.

## Quick Build

### Linux/macOS (System Packages)
```bash
git clone <repo-url>
cd rtype
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

### Linux/macOS (Conan)
```bash
git clone <repo-url>
cd rtype
conan install . --build=missing
cmake --preset conan-default
cmake --build --preset conan-release
```

### Windows (Conan)
```powershell
git clone <repo-url>
cd rtype

# Use the automated build script (installs both Debug and Release dependencies)
.\scripts\build.ps1

# Or manual approach:
# Install dependencies (both Debug and Release for complete setup)
conan install . --build=missing -s build_type=Debug
conan install . --build=missing -s build_type=Release

# Build Release
cmake --preset conan-default
cmake --build --preset conan-release
```

**Important**: The build script automatically installs both Debug and Release dependencies to prevent the common issue where debug builds fail due to missing debug DLLs (e.g., `sfml-graphics-d-2.dll`).

Binaries appear in `bin/`:
- `rtype_server` (or `rtype_server.exe`)
- `rtype_client` (or `rtype_client.exe`)

## Running the Game

### 1. Start the Server
```bash
# Linux/macOS
./bin/rtype_server

# Windows
.\bin\rtype_server.exe
```

The server listens on `0.0.0.0:4242` by default (configured in `config/game.ini`).

### 2. Start Client(s)
```bash
# Linux/macOS
./bin/rtype_client

# Windows
.\bin\rtype_client.exe
```

The client connects to `localhost:4242` by default. To connect to a remote server, edit `config/game.ini`:

```ini
[Network]
DefaultHost = <server-ip>
DefaultPort = 4242
```

### Multiple Players
Launch up to 4 client instances (same machine or different machines on the network). Each player is automatically assigned a unique color and position.

## Configuration

Edit `config/game.ini` to customize:
- **Network settings**: Host, port, timeouts
- **Gameplay**: Player speed, monster spawn rates, difficulty
- **Rendering**: Window size, FPS, colors
- **Audio**: Volume levels, enable/disable

See [[Configuration]] for all available options.

## Build Options

### Debug Build
```bash
# Linux/macOS
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Windows (Conan) - use build script or manual
.\scripts\build.ps1 -BuildType Debug

# Or manual:
cmake --preset conan-debug
cmake --build --preset conan-debug
```

**Note for Windows**: Debug builds require debug versions of dependencies. The `build.ps1` script automatically handles this by installing both Debug and Release dependencies.

### Custom CMake Options
```bash
# Specify compiler
cmake -DCMAKE_CXX_COMPILER=clang++ ...

# Verbose build
cmake --build build --verbose

# Parallel build (Linux/macOS)
cmake --build build -j$(nproc)
```

## Troubleshooting

### Common Build Errors

**"SFML not found"** (Linux)
```bash
sudo apt install libsfml-dev
# Or use Conan: conan install . --build=missing
```

**"Asio not found"** (Linux)
```bash
sudo apt install libasio-dev
# Or use Conan: conan install . --build=missing
```

**MSVC compilation error with `uniform_int_distribution`**
- Already fixed in codebase
- If you see this, make sure you've pulled latest changes
- See [[Contributing]] for details on the fix

**"CMake version too old"**
```bash
# Ubuntu: Use Kitware APT repository
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc | sudo apt-key add -
sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main'
sudo apt update
sudo apt install cmake
```

### Common Runtime Errors

**"Server: bind failed: Address already in use"**
- Another process is using port 4242
- Change port in `config/game.ini` or kill the existing process:
```bash
# Linux/macOS
lsof -i :4242
kill <PID>

# Windows
netstat -ano | findstr :4242
taskkill /PID <PID> /F
```

**"Client: Connection refused"**
- Ensure server is running
- Check host/port in `config/game.ini`
- Verify firewall settings

**"Missing assets" / blank screen**
- Assets are in `src/assets/` and copied to build directory
- Ensure working directory is project root when running binaries

## Related Documentation

- [[Home]] - Wiki home and overview
- [[Configuration]] - Game configuration options
- [[Contributing]] - Development environment setup
- [building_and_running.md](../building_and_running.md) - Extended build documentation (606 lines)
- [developer_guide.md](../developer_guide.md) - Development workflow
