# R-Type Configuration Guide

> **Related Documentation:**
> - [Gameplay Guide](gameplay_guide.md) — Understanding game mechanics
> - [Developer Guide](developer_guide.md) — Adding new configuration parameters
> - [Architecture](architecture.md) — How configuration is loaded and used

## Overview

The R-Type game engine now supports runtime configuration through an INI file. You can modify game parameters without recompiling by editing `config/game.ini`.

## Configuration File Location

The R-Type engine uses two configuration files:

1. **`config/game.ini`** - Game mechanics, gameplay parameters, rendering settings
2. **`config/systems.ini`** - ECS system toggles and parameters

The server and client will automatically search for them in:
1. `config/` (if running from project root)
2. `../config/` (if running from `build/Debug/`)

**Note:** The client must load both files. Make sure both are accessible.

## Configuration Sections

### systems.ini - ECS System Configuration

Controls which game systems are active. Located at `config/systems.ini`.

```ini
[Systems]
# Movement system - updates positions based on velocity
MovementSystem=true

# Fire cooldown system - manages weapon cooldowns
FireCooldownSystem=true

# Projectile lifetime system - ages bullets and removes old ones
ProjectileLifetimeSystem=true

# Collision system - handles projectile vs entity collisions
CollisionSystem=true

# Boundary system - removes entities outside world bounds
BoundarySystem=true

# Cleanup system - removes dead entities
CleanupSystem=true

# Monster spawner system - spawns monsters based on configuration
MonsterSpawnerSystem=true

# Level progression system - manages wave advancement
LevelSystem=true

[SystemParameters]
BoundaryMargin=100.0  # Distance outside bounds before entity destruction
```

**System Dependencies:**
- `LevelSystem` requires `MonsterSpawnerSystem` to function
- All other systems are independent
- Disabling a system removes its functionality completely

**Use Cases:**
- Disable `LevelSystem` for infinite spawning mode
- Disable `CollisionSystem` for invincibility testing
- Disable `MonsterSpawnerSystem` for peaceful mode

### game.ini - Gameplay Configuration

### [Gameplay]

Controls all gameplay mechanics and behavior:

```ini
[Gameplay]
ScrollDirection=LeftToRight  # Options: LeftToRight, RightToLeft, TopToBottom, BottomToTop
ScrollSpeed=90.0             # Speed of background scroll and monster movement

# Player settings
PlayerSpeed=220.0            # Player movement speed
PlayerStartHP=3              # Player starting health
PlayerFireCooldown=0.25      # Time in seconds between shots

# Player spawn positions
PlayerSpawnX=80.0            # X coordinate for player spawn
PlayerSpawnYBase=120.0       # Base Y coordinate for first player
PlayerSpawnYSpacing=80.0     # Vertical spacing between players in multiplayer

# Bullet settings
BulletSpeed=380.0            # Projectile speed
BulletLifetime=3.0           # Time in seconds before bullets are destroyed
BulletSpawnOffsetX=30.0      # Horizontal offset from player position
BulletSpawnOffsetY=0.0       # Vertical offset from player position
BulletDirection=LeftToRight  # Direction bullets travel

# Weapon damage by type
WeaponDamageBasic=1          # Damage for basic weapon (type 0)
WeaponDamageLaser=2          # Damage for laser weapon (type 1)
WeaponDamageMissile=3        # Base damage for rocket weapon (type 2) before multiplier
PowerUpsForLaser=5           # Power-ups required to unlock laser weapon
PowerUpsForRocket=10         # Power-ups required to unlock rocket shooter
RocketFireCooldown=0.6       # Cooldown (seconds) between rocket shots
RocketDamageMultiplier=3.0   # Rocket damage multiplier over basic weapon

# Monster settings
MonsterSpawnDelay=2.0        # Time in seconds between monster spawns
MonsterSpawnSide=RightToLeft # Where monsters spawn from
MonsterMovement=RightToLeft  # Direction monsters move

# Monster types (0-3 with different HP, size, and spawn weights)
MonsterType0HP=3             # Hit points for type 0 (weakest)
MonsterType1HP=5
MonsterType2HP=8
MonsterType3HP=10            # Hit points for type 3 (strongest)

MonsterType0Size=24.0        # Visual size in pixels
MonsterType1Size=28.0
MonsterType2Size=32.0
MonsterType3Size=40.0

#speed multipliers multiplied by scroll speed for each monster type
MonsterType0Speed=1.0        # Speed multiplier for type 0 monster
MonsterType1Speed=1.2        # Speed multiplier for type 1 monster
MonsterType2Speed=0.8        # Speed multiplier for type 2 monster
MonsterType3Speed=0.6        # Speed multiplier for type 3 monster

MonsterType0SpawnWeight=5    # Relative spawn probability
MonsterType1SpawnWeight=30   # Higher = more common
MonsterType2SpawnWeight=15
MonsterType3SpawnWeight=5

MonsterType0Color=200,60,60  # RGB color values
MonsterType1Color=220,120,40
MonsterType2Color=140,40,140
MonsterType3Color=255,20,20

# Power-up settings
PowerUpsEnabled=true         # Enable/disable power-up spawning
PowerUpSpawnDelay=10.0       # Time in seconds between power-up spawns
PowerUpSpawnSide=RightToLeft # Where power-ups spawn from
PowerUpSpeedMultiplier=0.25  # Speed relative to scroll speed
PowerUpSize=8.0              # Visual size in pixels
PowerUpColor=100,240,140     # RGB color
PowerUpOutlineColor=255,255,255
PowerUpOutlineThickness=2.0
Rocket shooter unlocks per-player once the threshold is reached; tune the above values to change progression pacing.

# Collision and world
CollisionRadius=20.0         # Collision detection radius in pixels
WorldWidth=1280.0            # Game world width
WorldHeight=720.0            # Game world height

# Level progression
NumberOfLevels=2             # Total number of levels
MonsterPerLevel=3            # Monsters spawned = this * level number
```

**Scroll Direction** changes the entire game orientation:
- `LeftToRight`: Classic R-Type (scroll right, monsters from right)
- `RightToLeft`: Reverse horizontal
- `TopToBottom`: Vertical scrolling downward
- `BottomToTop`: Vertical scrolling upward

**Monster/PowerUp Spawn Side**:
- `MatchScroll`: Spawns from scroll direction
- `Opposite`: Spawns from opposite side
- `LeftToRight`, `RightToLeft`, `TopToBottom`, `BottomToTop`: Explicit directions
- `Center`: Spawns from center (power-ups only)

**Monster Movement**:
- `MatchScroll`: Moves with scroll
- `Opposite`: Moves against scroll
- `Static`: Doesn't move
- Explicit directions: `LeftToRight`, `RightToLeft`, `TopToBottom`, `BottomToTop`

### [Render]

Controls visual rendering and window settings:

```ini
[Render]
WindowWidth=1280             # Window width in pixels
WindowHeight=720             # Window height in pixels
WindowTitle=R-Type           # Window title text
TargetFPS=60                 # Frame rate limiter

# Player rendering
PlayerSize=20.0              # Player sprite size
PlayerRotation=0.0           # Player sprite rotation in degrees
Player1Color=95,205,228      # RGB color for player 1 (cyan)
Player2Color=255,174,79      # RGB color for player 2 (orange)
Player3Color=140,122,230     # RGB color for player 3 (purple)
Player4Color=255,99,146      # RGB color for player 4 (pink)

# Bullet rendering
BulletSize=4.0               # Bullet sprite size
BulletColor=255,207,64       # RGB color (yellow)

# Background
BackgroundColor=6,10,26      # RGB color for background (dark blue)

# Starfield background (if enabled)
StarCount=100                # Number of background stars
StarSpeedMin=30.0            # Minimum star scroll speed
StarSpeedMax=90.0            # Maximum star scroll speed
StarSizeMin=1.0              # Minimum star size in pixels
StarSizeMax=2.6              # Maximum star size in pixels

TexturePack=default          # Texture pack to use (folder name in assets/textures/)
```

### [Network]

Network configuration:

```ini
[Network]
DefaultPort=5000             # Server UDP port
DefaultHost=127.0.0.1        # Default server address
MaxPacketSize=1024           # Maximum network packet size
SendBufferSize=65536         # Socket send buffer size
```

### [Audio]

Audio settings:

```ini
[Audio]
MusicVolume=50.0             # Background music volume (0-100)
SFXVolume=35.0               # Sound effects volume (0-100)
Enabled=true                 # Master audio on/off
```

## Examples

### Enable Power-Ups and Adjust Spawn Rate

Edit `config/game.ini`:

```ini
[Gameplay]
PowerUpsEnabled=true
PowerUpSpawnDelay=8.0        # Spawn power-ups more frequently
```

### Change to Vertical Scrolling

Edit `config/game.ini`:

```ini
[Gameplay]
ScrollDirection=TopToBottom
ScrollSpeed=60.0
MonsterSpawnSide=TopToBottom
MonsterMovement=TopToBottom
```

Run without recompiling:
```bash
cd build/Debug
./src/rtype_server &
./src/rtype_client
```

The game will now scroll vertically from top to bottom!

### Make Game Easier

```ini
[Gameplay]
MonsterSpawnDelay=4.0        # Spawn monsters less frequently
PlayerSpeed=300.0            # Move faster
MonsterType0HP=2             # Reduce monster health
MonsterType1HP=3
MonsterType2HP=5
MonsterType3HP=7
PlayerStartHP=5              # More health
PowerUpSpawnDelay=6.0        # More frequent weapon upgrades
```

### Make Game Harder

```ini
[Gameplay]
MonsterSpawnDelay=1.0        # Spawn monsters more frequently
PlayerSpeed=180.0            # Move slower
MonsterType0HP=5             # Increase monster health
MonsterType1HP=8
MonsterType2HP=12
MonsterType3HP=15
ScrollSpeed=120.0            # Faster scroll speed
PowerUpsEnabled=false        # Disable weapon upgrades
PlayerFireCooldown=0.4       # Slower fire rate
```

### Adjust Monster Variety

Change spawn weights to make certain monster types more common:

```ini
[Gameplay]
MonsterType0SpawnWeight=10   # Common weak monsters
MonsterType1SpawnWeight=50   # Very common medium monsters
MonsterType2SpawnWeight=5    # Rare strong monsters
MonsterType3SpawnWeight=1    # Very rare boss-type monsters
```

### Different Window Size

```ini
[Render]
WindowWidth=1920
WindowHeight=1080
```

## Testing Configuration Changes

1. Edit `config/game.ini` with your changes
2. Save the file
3. Run the game (no rebuild needed):
   ```bash
   cd build/Debug
   ./src/rtype_server &
   ./src/rtype_client
   ```
4. Observe the changes in real-time

## Configuration System Implementation

The configuration system is implemented in:
- `include/rtype/common/GameConfig.hpp` - Configuration structure
- `src/common/config/GameConfig.cpp` - INI parser and loader
- `config/game.ini` - Default configuration file

Both server and client load the configuration at startup. The server uses it for:
- Game physics (speeds, collision)
- Spawn logic and timing
- World boundaries
- Monster types and properties
- Weapon damage values
- Power-up spawning
- Level progression parameters

The client uses it for:
- Window size and FPS
- Starfield rendering
- Audio volumes
- Network connection
- Visual rendering (colors, sizes)

## Game Features

### Weapon System
- Three weapon types: Basic (0), Laser (1), Rocket (2)
- Three levels per weapon (damage multiplier)
- Power-ups change weapon type or level it up (max level 3)
- Picking up the same weapon type upgrades it; different types replace it

### Wave/Level Progression System

The game uses a modular system for spawning and progression:

**MonsterSpawnerSystem:**
- Spawns monsters at configured intervals (`MonsterSpawnDelay`)
- Uses weighted random selection based on monster spawn weights
- Can run standalone (continuous spawning) or controlled by LevelSystem
- Respects all monster configuration (type, position, velocity)

**LevelSystem:**
- Manages wave progression and difficulty scaling
- Waits for all monsters to be defeated before advancing
- Formula: `MonsterPerLevel × WaveNumber`
- Broadcasts wave changes to clients for display
- Controls MonsterSpawnerSystem to spawn the right amount

**Configuration Modes:**
- Both enabled: Wave-based progression with scaling difficulty
- Only MonsterSpawnerSystem: Continuous spawning, no wave tracking
- Only LevelSystem: Error (requires spawner)
- Both disabled: No monsters spawn

**Wave Example:**
- Wave 1: 3 monsters (if `MonsterPerLevel=3`)
- Wave 2: 6 monsters
- Wave 3: 9 monsters
- etc.

### Monster Types
- Four configurable monster types (0-3)
- Each type has: HP, size, color, and spawn weight
- Spawn weight determines rarity (higher = more common)
- Monsters are randomly selected based on weighted probabilities

### Collision Detection
- Radius-based collision using `CollisionRadius` setting
- Checks projectile vs monsters, projectile vs players, and players vs power-ups
- Entities marked for destruction are cleaned up at end of frame

## Default Configuration

If the configuration file is not found, the system uses default values (same as the provided `game.ini`). You'll see a warning message but the game will still run.

## Advanced: Adding New Configuration Options

To add a new configurable parameter:

1. Add the field to the appropriate struct in `GameConfig.hpp`:
   ```cpp
   struct GameplayConfig {
       // ... existing fields
       float newParameter{100.0f};
   };
   ```

2. Add parsing in `GameConfig.cpp` in `loadFromFile()`:
   ```cpp
   else if (key == "NewParameter")
       gameplay.newParameter = std::stof(value);
   ```

3. Add to `saveToFile()` (if applicable):
   ```cpp
   file << "NewParameter=" << gameplay.newParameter << "\n";
   ```

4. Add to `config/game.ini`:
   ```ini
   [Gameplay]
   NewParameter=100.0
   ```

5. Use in your code:
   ```cpp
   float value = _config.gameplay.newParameter;
   ```

6. Rebuild and test!

## Troubleshooting

### Configuration Not Loading
- Check file path: server looks for `config/game.ini` or `../config/game.ini`
- Verify INI syntax: `Key=Value` with section headers `[Section]`
- Check console for "[server] loaded config" or warning messages

### Changes Not Taking Effect
- Restart both server and client after editing config
- Verify the correct config file is being loaded (check working directory)
- Some visual settings require client restart, gameplay settings require server restart

### Invalid Values
- Config parser uses default values if parsing fails
- Check data types: integers for counts, floats for speeds/positions
- Boolean values: `true` or `false` (lowercase)
- Colors: `R,G,B` format with values 0-255
