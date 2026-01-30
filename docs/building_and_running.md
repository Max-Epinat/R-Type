# Building and Running R-Type

> **Related Documentation:**
> - [Developer Guide](developer_guide.md) — Development workflow and contribution guide
> - [README](../README.md) — Project overview
> - [Architecture](architecture.md) — Understanding the codebase structure

This guide covers building and running the R-Type project on Linux and Windows.

## Prerequisites

### Required Tools
- **C++ Compiler**: GCC 11+ or Clang 14+ (C++20 support required)
- **CMake**: Version 3.28 or higher
- **Conan**: Package manager version 2.0+
- **Git**: For cloning the repository

### Platform-Specific Requirements

#### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install build-essential cmake git python3-pip
sudo apt install libasio-dev libsfml-dev  # Optional: system libraries
pip3 install conan
```

#### Windows
**Required:**
- **Visual Studio 2019 or later** (with C++ development tools) or **Visual Studio Build Tools**
  - Download from: https://visualstudio.microsoft.com/downloads/
  - During installation, select "Desktop development with C++"
- **CMake 3.28+**
  - Download from: https://cmake.org/download/
  - During installation, select "Add CMake to system PATH"
- **Python 3.7+**
  - Download from: https://www.python.org/downloads/
  - **IMPORTANT**: During installation, check "Add Python to PATH"
- **Conan 2.0+**
  ```powershell
  pip install conan
  ```

**Note**: The automated build script (build.ps1) will automatically create the Conan profile if needed.

**Verification:**
```powershell
# Open PowerShell (close and reopen after Python installation) and verify
cmake --version
python --version
py --version
conan --version
```

**Troubleshooting Python/pip:**

If `pip` or `python` are not recognized:

1. **Check if Python is installed:**
   ```powershell
   # Try these commands
   python --version
   py --version
   python3 --version
   ```
   
2. **If Python is not installed:**
   - Go to https://www.python.org/downloads/
   - Download Python 3.12 or later
   - Run installer
   - **CRITICAL**: Check ☑ "Add Python to PATH"
   - Click "Install Now"
   - Restart PowerShell after installation

3. **If Python is installed but not in PATH:**
   
   **Option A: Add to PATH manually**
   - Press `Win + X` → System → Advanced system settings → Environment Variables
   - Under "User variables", select "Path" → Edit
   - Click "New" and add these paths (adjust Python version):
     ```
     C:\Users\<YourUsername>\AppData\Local\Programs\Python\Python312
     C:\Users\<YourUsername>\AppData\Local\Programs\Python\Python312\Scripts
     ```
   - Click OK, restart PowerShell
   
   **Option B: Use py launcher**
   - Windows Python installations include `py` launcher
   - Use `py` instead of `python`:
     ```powershell
     py --version
     py -m pip install conan
     ```
   
   **Option C: Reinstall Python with PATH option**
   - Uninstall Python from Windows Settings → Apps
   - Reinstall from https://www.python.org/downloads/
   - **Check** ☑ "Add Python to PATH" during installation

4. **Verify Conan installation:**
   ```powershell
   # If pip worked, verify Conan
   conan --version
   
   # If not found, check installation location
   py -m pip show conan
   ```

5. **If `conan` command is not recognized after installation:**

   This happens when Python Scripts folder is not in PATH. You'll see:
   ```
   WARNING: The script conan.exe is installed in '...\Scripts' which is not on PATH
   ```
   
   **Solution A: Add Scripts to PATH (Permanent fix)**
   
   1. Note the path from the warning message (example: `C:\Users\maxep\AppData\Local\Python\pythoncore-3.14-64\Scripts`)
   2. Press `Win + X` → **System** → **Advanced system settings**
   3. Click **Environment Variables**
   4. Under "User variables", select **Path** → **Edit**
   5. Click **New** and paste the Scripts path from step 1
   6. Click **OK** on all dialogs
   7. **Close and reopen PowerShell**
   8. Test: `conan --version`
   
   **Solution B: Use py -m conan (Works immediately, no PATH needed)**
   
   Instead of `conan`, use `py -m conan`:
   ```powershell
   # Check version
   py -m conan --version
   
   # Use in all commands
   py -m conan install ...
   py -m conan profile detect
   ```
   
   **Solution C: Create an alias (PowerShell session only)**
   ```powershell
   # Add to your PowerShell profile
   Set-Alias conan "py -m conan"
   
   # Now you can use:
   conan --version
   ```

## Building the Project

### 1. Clone the Repository
```bash
git clone <repository-url>
cd r-type
```

### 2. Build Instructions

#### Linux Build

**Install Dependencies with Conan:**
```bash
# Create build directory
mkdir -p build/Debug
cd build/Debug

# Install dependencies and generate build files
conan install ../.. --output-folder=. --build=missing -s build_type=Debug
```

**Configure with CMake:**
```bash
# From build/Debug directory
cmake ../.. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
```

**Build:**
```bash
# Build all targets
cmake --build .

# Or use ninja if available
ninja
```

The build produces:
- `src/rtype_server` - The game server executable
- `src/rtype_client` - The game client executable

#### Windows Build

**Option 1: Using PowerShell Build Script (Recommended)**

1. Open **Developer PowerShell for VS** (or regular PowerShell with Visual Studio in PATH)
2. Navigate to the project directory:
   ```powershell
   cd r-type
   ```
3. **Important**: Enable script execution (first time only):
   ```powershell
   # Check current policy
   Get-ExecutionPolicy
   
   # If it shows "Restricted", you need to change it
   # Option A: Allow for current user (recommended)
   Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
   
   # Option B: Bypass for this session only
   Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process
   
   # Option C: Run script with bypass (no policy change)
   PowerShell -ExecutionPolicy Bypass -File .\build.ps1 -BuildType Release
   ```

4. Run the build script:
   ```powershell
   .\build.ps1 -BuildType Release
   ```

The script will:
- ✓ Check prerequisites (CMake, Conan, compiler)
- ✓ Create build directory
- ✓ Install dependencies with Conan
- ✓ Configure with CMake
- ✓ Build the project

**Option 2: Manual Build**

1. Open **Developer PowerShell for VS 2019** (or later)
2. Install dependencies:
   ```powershell
   mkdir build\Debug
   cd build\Debug
   conan install ..\..\ --output-folder=. --build=missing -s build_type=Debug
   
   # Or if 'conan' is not in PATH:
   py -m conan install ..\..\ --output-folder=. --build=missing -s build_type=Debug
   ```
3. Configure with CMake:
   ```powershell
   cmake ..\..\ -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
   ```
4. Build:
   ```powershell
   cmake --build . --config Debug
   ```

The build produces:
- `build\Debug\src\Debug\rtype_server.exe` - The game server
- `build\Debug\src\Debug\rtype_client.exe` - The game client

## Running the Game

### Quick Start

#### Linux
Use the provided launch script:
```bash
# From project root
cd scripts
./launch.sh
```

#### Windows
Use the PowerShell launch script:

```powershell
.\launch.ps1 -BuildType Release
```

Both scripts:
1. Check if binaries exist
2. Start the server in the background
3. Wait 2 seconds for server initialization
4. Launch the client
5. Clean up server process on exit

### Manual Launch

#### Start the Server

**Linux:**
```bash
# From build/Debug directory
./src/rtype_server

# Or from project root
./bin/rtype_server
```

**Windows:**
```powershell
# From build\Debug\src\Debug directory
.\rtype_server.exe

# Or from project root
.\build\Debug\src\Debug\rtype_server.exe
```

The server:
- Listens on UDP port 5000 by default (configurable in `config/game.ini`)
- Loads `config/game.ini` for game settings
- Runs at 60 FPS update rate
- Logs connection and game events to console

#### Start the Client(s)

**Linux:**
```bash
# From build/Debug directory
./src/rtype_client

# Or from project root
./bin/rtype_client
```

**Windows:**
```powershell
# From build\Debug\src\Debug directory
.\rtype_client.exe

# Or from project root
.\build\Debug\src\Debug\rtype_client.exe
```

The client:
- Connects to localhost:5000 by default
- Sends player input every frame
- Renders at configured FPS (default 60)
- Supports up to 4 players with different colors

### Multiple Clients
To test multiplayer, run multiple client instances:

**Linux:**
```bash
# Terminal 1
./bin/rtype_server

# Terminal 2-4
./bin/rtype_client
```

**Windows:**
```powershell
# PowerShell 1
.\build\Debug\src\Debug\rtype_server.exe

# PowerShell 2-4
.\build\Debug\src\Debug\rtype_client.exe
```

Each client will be assigned a unique player ID and spawn position.

## Configuration

Edit `config/game.ini` to customize:
- Network settings (port, host)
- Gameplay parameters (speeds, spawn rates, difficulty)
- Visual settings (window size, colors)
- Audio settings

See [Configuration Guide](configuration_guide.md) for details.

## Troubleshooting

### Build Issues

**Problem**: CMake can't find Conan toolchain
```
Solution: Ensure you ran conan install with --output-folder pointing to build directory
```

**Problem**: Missing dependencies
```bash
# Rebuild all dependencies
conan install ../.. --output-folder=. --build=missing
```

**Problem**: Compiler errors about C++20 features
```
Linux: Update your compiler to GCC 11+ or Clang 14+
  Check with: g++ --version
  
Windows: Ensure Visual Studio 2019+ is installed
  Check with: cl.exe (in Developer Command Prompt)
  Update Visual Studio if needed
```

**Problem** (Windows): "'conan' is not recognized" after successful installation

This means Conan is installed but the Scripts folder is not in PATH.

**Quick fix (works immediately):**
```powershell
# Use py -m conan instead of conan
py -m conan --version

# In all build commands, replace 'conan' with 'py -m conan'
py -m conan install ..\..\ --output-folder=. --build=missing -s build_type=Debug
```

**Permanent fix (add to PATH):**
1. Check where Conan is installed:
   ```powershell
   py -m pip show conan
   ```
2. Look for the installation warning that showed the Scripts path
3. Add that Scripts folder to PATH (see Prerequisites → Troubleshooting Python/pip → Step 5)
4. Restart PowerShell
5. Test: `conan --version`

**Alternative: Modify build script**

If you can't modify PATH, edit `scripts\build.ps1`:
- Replace `& conan` with `& py -m conan`

**Problem** (Windows): "CMake Error: CMake was unable to find a build program"
```
Solution: Run from Developer PowerShell for VS 2019 (or later)
OR ensure Visual Studio Build Tools are properly installed
```

**Problem** (Windows): "error MSB8020: The build tools for v142 cannot be found"
```
Solution: Install Visual Studio 2019 Build Tools or update to VS 2022
```

**Problem** (Windows): "L'exécution de scripts est désactivée" / "Execution of scripts is disabled"

This is Windows PowerShell execution policy blocking scripts.

**Solution A: Change policy for current user (permanent, recommended)**
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```
Then run the script normally: `.\build.ps1`

**Solution B: Bypass for current session only**
```powershell
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process
```
This only affects the current PowerShell window.

**Solution C: Run with bypass (no policy change needed)**
```powershell
PowerShell -ExecutionPolicy Bypass -File .\build.ps1
```

**Solution D: Use manual build instead**
Follow the "Option 2: Manual Build" instructions above.

### Runtime Issues

**Problem**: Server won't start - "Address already in use"

**Linux:**
```bash
# Find and kill process using port 5000
lsof -i :5000
kill <PID>
```

**Windows:**
```powershell
# Find process using port 5000
netstat -ano | findstr :5000

# Kill process (use PID from above)
Stop-Process -Id <PID> -Force
```

**Problem**: Client can't connect to server
- Verify server is running: `ps aux | grep rtype_server`
- Check firewall settings allow UDP port 5000
- Verify network configuration in `config/game.ini`

**Problem**: Config file not found
```
Server looks for config/game.ini relative to:
1. Current working directory
2. One directory up (../config/game.ini)

Run from project root or build/Debug directory
```

**Problem**: Client window doesn't appear

**Linux:**
- Check SFML installation: `ldconfig -p | grep sfml`
- Verify display is available: `echo $DISPLAY`
- Check render settings in config file

**Windows:**
- Ensure all DLLs are present in executable directory
- Check Windows Defender/Firewall isn't blocking
- Verify graphics drivers are up to date
- Check render settings in config file

**Problem** (Windows): "Missing DLL" errors (sfml-*.dll, etc.)
```
Solution: Conan places DLLs in the build directory automatically
If missing, rebuild with:
  cd scripts
  .\build.ps1
  
Or manually copy DLLs from:
  build\Debug\ to build\Debug\src\Debug\
```

### Performance Issues

**Problem**: Low FPS or stuttering
- Reduce `StarCount` in config
- Lower `TargetFPS` if hardware limited
- Close other applications
- Check CPU usage with `top` or `htop`

**Problem**: Network lag
- Check network latency: `ping localhost` (should be <1ms)
- Reduce `MonsterSpawnDelay` to spawn fewer monsters
- Monitor bandwidth with `iftop` or `nethogs`

## Development Build

For development with debug symbols:
```bash
mkdir -p build/Debug
cd build/Debug
conan install ../.. --output-folder=. --build=missing -s build_type=Debug
cmake ../.. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

For optimized release build:
```bash
mkdir -p build/Release
cd build/Release
conan install ../.. --output-folder=. --build=missing -s build_type=Release
cmake ../.. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## Testing Network Conditions

### Simulate Latency (Linux)
```bash
# Add 100ms latency to loopback
sudo tc qdisc add dev lo root netem delay 100ms

# Remove latency simulation
sudo tc qdisc del dev lo root
```

### Simulate Packet Loss
```bash
# Add 10% packet loss
sudo tc qdisc add dev lo root netem loss 10%

# Remove packet loss
sudo tc qdisc del dev lo root
```

## Debugging

### Server Debugging
```bash
# Run with GDB
gdb ./src/rtype_server
(gdb) run

# Or with Valgrind for memory issues
valgrind --leak-check=full ./src/rtype_server
```

### Client Debugging
```bash
gdb ./src/rtype_client
(gdb) run
```

### Network Debugging
```bash
# Monitor UDP traffic on port 5000
sudo tcpdump -i lo udp port 5000 -X

# Or use Wireshark for detailed packet inspection
```

## Clean Build

To start fresh:
```bash
# Remove build directory
rm -rf build/

# Remove Conan cache (optional, if dependency issues)
conan remove "*" -c

# Rebuild from scratch
mkdir -p build/Debug
cd build/Debug
conan install ../.. --output-folder=. --build=missing
cmake ../.. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
cmake --build .
```
