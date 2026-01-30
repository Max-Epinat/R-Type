# FAQ and Troubleshooting

Common issues and solutions for building, running, and playing R-Type.

## Build Issues

### "CMake version too old"

**Problem**: CMake 3.20+ required, but you have older version.

**Solution (Ubuntu/Debian)**:
```bash
# Add Kitware repository
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc | sudo apt-key add -
sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main'
sudo apt update
sudo apt install cmake
```

**Solution (Windows)**: Download from [cmake.org/download](https://cmake.org/download/)

### "SFML not found" (Linux)

**Problem**: SFML library missing.

**Solution**:
```bash
# Ubuntu/Debian
sudo apt install libsfml-dev

# Fedora/RHEL
sudo dnf install SFML-devel

# Or use Conan
pip3 install conan
conan install . --build=missing
```

### "Asio not found" (Linux)

**Problem**: Asio library missing.

**Solution**:
```bash
# Ubuntu/Debian
sudo apt install libasio-dev

# Fedora/RHEL
sudo dnf install asio-devel

# Or use Conan
conan install . --build=missing
```

### MSVC compilation error: `uniform_int_distribution<uint8_t>`

**Problem**: MSVC enforces C++11 standard - `uniform_int_distribution` only accepts int types.

**Status**: Already fixed in current codebase.

**Solution**: Pull latest changes:
```bash
git pull
cmake --build build
```

If you see this in your own code, change:
```cpp
// Wrong
std::uniform_int_distribution<uint8_t> dist(0, 255);

// Correct
std::uniform_int_distribution<int> dist(0, 255);
uint8_t value = static_cast<uint8_t>(dist(rng));
```

### "conan: command not found" (Windows)

**Problem**: Conan installed but not in PATH.

**Solution 1** - Use Python launcher:
```powershell
py -m conan --version
py -m conan install . --build=missing
```

**Solution 2** - Add Scripts to PATH:
1. Note the path from pip install warning (e.g., `C:\Users\<You>\AppData\Local\Programs\Python\Python312\Scripts`)
2. Press `Win + X` → System → Advanced → Environment Variables
3. Edit "Path" → Add new entry with Scripts path
4. Restart PowerShell
5. Test: `conan --version`

### Build fails with Conan profile errors

**Problem**: Conan profile not configured.

**Solution**:
```bash
conan profile detect
conan install . --build=missing
```

## Runtime Issues

### Server: "bind failed: Address already in use"

**Problem**: Port 4242 already in use by another process.

**Solution 1** - Kill existing process:
```bash
# Linux/macOS
lsof -i :4242
kill <PID>

# Windows
netstat -ano | findstr :4242
taskkill /PID <PID> /F
```

**Solution 2** - Change port in `config/game.ini`:
```ini
[Network]
DefaultPort=5000  # Use different port
```

### Client: "Connection refused" / Can't connect

**Problem**: Can't reach server.

**Checklist**:
1. ✅ Server is running (check console for "Listening on UDP port...")
2. ✅ Correct host/port in `config/game.ini`:
   ```ini
   [Network]
   DefaultHost=localhost  # Or server IP
   DefaultPort=4242
   ```
3. ✅ Firewall allows UDP traffic on port 4242
4. ✅ Server and client on same network (if using remote IP)

**Test localhost first**:
```ini
[Network]
DefaultHost=localhost
DefaultPort=4242
```

### "Missing assets" / Blank screen

**Problem**: Assets not found or working directory incorrect.

**Solution**:
1. Verify assets exist in `src/assets/`
2. Run binaries from project root:
   ```bash
   # From rtype/ directory
   ./bin/rtype_client
   
   # NOT from bin/
   ```
3. Check CMake copied assets (automatic in CMakeLists.txt)

### Client window opens then crashes

**Problem**: Usually configuration or graphics driver issue.

**Solutions**:
1. Reset config to defaults (delete and regenerate `config/game.ini`)
2. Try lower resolution:
   ```ini
   [Render]
   WindowWidth=1280
   WindowHeight=720
   TargetFPS=30
   ```
3. Update graphics drivers
4. Check console output for error messages

## Gameplay Issues

### Controls not working

**Problem**: Game window not focused or key conflicts.

**Solutions**:
1. Click on game window to focus it
2. Close other applications that might intercept keys
3. Try alternative controls (arrows vs WASD)
4. Check if keyboard works in other applications

### Power-ups not spawning

**Problem**: Power-ups disabled or spawn delay too high.

**Solution** - Check `config/game.ini`:
```ini
[Gameplay]
PowerUpsEnabled=true
PowerUpSpawnDelay=10.0  # Seconds between spawns
```

Default is 10 seconds, so wait patiently after game starts.

### Low FPS / Lag

**Problem**: Performance issues.

**Solutions**:

1. **Reduce visual complexity** in `config/game.ini`:
   ```ini
   [Render]
   StarCount=50         # Default is 100
   TargetFPS=30         # Default is 60
   WindowWidth=1280
   WindowHeight=720
   ```

2. **Network optimization**:
   - Play on localhost (same machine)
   - Close bandwidth-heavy apps
   - Check ping to server

3. **System resources**:
   - Close other applications
   - Update drivers
   - Check CPU/GPU temperature (thermal throttling)

### Enemies not spawning

**Problem**: Spawn delay too high or already defeated.

**Solutions**:
1. Wait - check `MonsterSpawnDelay` in config (default 2.0s)
2. If level just advanced, enemies spawn after delay
3. Check server console for spawn messages

### Can't damage enemies

**Problem**: Bullets not colliding or enemy HP too high.

**Solutions**:
1. Aim directly at enemies (bullets are small)
2. Check server console for hit detection messages
3. Lower enemy HP in config for testing:
   ```ini
   [Gameplay]
   MonsterType0HP=1
   MonsterType1HP=1
   ```

## Network Issues

### High ping / Latency

**Problem**: Packets delayed.

**Solutions**:
1. Use localhost for testing
2. Check network quality: `ping <server-ip>`
3. Close background downloads
4. Use wired connection instead of WiFi
5. Ensure server isn't CPU-bound (check CPU usage)

### Disconnect after timeout

**Problem**: Client timeout (default 30 seconds).

**Solution** - Increase timeout in `config/game.ini`:
```ini
[Network]
ServerTimeout=60  # Seconds before timeout
```

### Multiple players see different game states

**Problem**: Network desync (rare with UDP broadcast).

**Solutions**:
1. Restart server and all clients
2. Ensure all clients use same config
3. Check for packet loss: `ping -c 100 <server-ip>`
4. Verify server broadcasts to all clients (check server logs)

## Configuration Issues

### Changes to config.ini not taking effect

**Problem**: Config cached or wrong file edited.

**Solutions**:
1. Restart server AND client after config changes
2. Verify editing correct file:
   - Should be `config/game.ini` in project root
   - NOT in build/ directory
3. Check file permissions (should be readable)
4. Verify syntax (INI format: `Key=Value`, no quotes)

### Invalid config values cause crash

**Problem**: Out-of-range values.

**Solutions**:
1. Delete `config/game.ini` - it will regenerate with defaults
2. Use reasonable values:
   ```ini
   [Gameplay]
   PlayerSpeed=50-1000      # Reasonable range
   MonsterSpawnDelay=0.1-10.0
   
   [Render]
   WindowWidth=800-3840
   WindowHeight=600-2160
   TargetFPS=30-144
   ```

## Platform-Specific

### Windows: "VCRUNTIME140.dll not found"

**Problem**: Visual C++ Redistributable missing.

**Solution**: Install [Visual C++ Redistributable](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist)

### Linux: "error while loading shared libraries: libsfml"

**Problem**: SFML libraries not in LD_LIBRARY_PATH.

**Solution**:
```bash
# Find SFML location
ldconfig -p | grep sfml

# If not found, install
sudo apt install libsfml-dev

# Or add to library path
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

### macOS: "command not found: brew"

**Problem**: Homebrew not installed.

**Solution**:
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

## Getting Help

If none of these solutions work:

1. **Check console output** - Server and client print detailed error messages
2. **Review logs** - Look for ERROR or WARNING messages
3. **Verify prerequisites** - Ensure all dependencies installed
4. **Try defaults** - Use default config to isolate issue
5. **Check documentation**:
   - [[Building and Running]] - Detailed build instructions
   - [[Contributing]] - Development environment setup
   - [developer_guide.md](../developer_guide.md) - Technical details

## Known Issues

- No player respawn (planned feature)
- No score display (planned feature)
- Simple geometric graphics (sprites planned)
- No enemy projectiles yet (planned feature)

See [[Gameplay Guide]] for current limitations and planned features.

## Related Documentation

- [[Quick Start]] - Fast path to get running
- [[Building and Running]] - Comprehensive build guide
- [[Configuration]] - All configuration options
- [[Gameplay Guide]] - Game mechanics and strategy
- [[Contributing]] - Report bugs or contribute fixes
