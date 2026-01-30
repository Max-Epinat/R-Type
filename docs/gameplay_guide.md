# Gameplay Guide

> **Related Documentation:**
> - [Configuration Guide](configuration_guide.md) — Customize gameplay parameters
> - [Protocol](protocol.md) — Network packet specifications for game entities
> - [README](../README.md) — Quick start and overview

## Game Overview

R-Type is a classic horizontal shoot-em-up where you pilot a spacecraft through waves of enemies. This multiplayer implementation supports up to 4 players cooperatively fighting through multiple levels.

## Controls

### Keyboard Controls
- **Arrow Keys** / **WASD**: Move your ship
  - ↑ / W: Move up
  - ↓ / S: Move down
  - ← / A: Move left
  - → / D: Move right
- **Space**: Fire weapon
- **Escape**: Quit game

### Controls Notes
- Hold fire to continuously shoot (subject to fire rate cooldown)
- Movement is smooth and responsive
- You can move in any diagonal direction
- Ship stays within screen boundaries

## Game Mechanics

### Your Ship
- **Starting Health**: 3 HP (configurable)
- **Speed**: Fast, responsive movement
- **Weapon**: Starts with basic weapon, upgradeable via power-ups
- **Position**: Each player spawns at a different vertical position on the left side

### Weapons

There are 3 weapon types, each with 3 upgrade levels:

1. **Basic Weapon (Type 0)** - Yellow projectiles
   - Level 1: 1 damage
   - Level 2: 2 damage  
   - Level 3: 3 damage

2. **Laser Weapon (Type 1)** - Cyan projectiles
   - Level 1: 2 damage
   - Level 2: 4 damage
   - Level 3: 6 damage

3. **Rocket Shooter (Type 2)** - Ember rockets (unlocked after 10 power-ups)
   - Level 1: ~3× basic damage
   - Level 2: ~6× basic damage
   - Level 3: ~9× basic damage
   - Fires slower than the other weapons but pierces tougher foes with raw damage

**Weapon Upgrading:**
- Collect power-ups (glowing green items) to upgrade
- Picking up the same weapon type levels it up (max level 3)
- Picking up a different weapon type replaces your current weapon and starts at level 1

### Enemies

Four types of enemies with different properties:

1. **Type 0** (Weakest) - Red
   - HP: 3
   - Size: Small
   - Most common in early waves

2. **Type 1** (Common) - Orange
   - HP: 5
   - Size: Medium
   - Standard enemy

3. **Type 2** (Strong) - Purple
   - HP: 8
   - Size: Large
   - Less common, more durable

4. **Type 3** (Rare) - Bright Red
   - HP: 10
   - Size: Very Large
   - Rare spawns, mini-boss difficulty

**Enemy Behavior:**
- Enemies spawn from the right side of the screen
- They move left across the screen
- They despawn when they reach the left edge
- No enemy projectiles in current version (future feature)

### Power-Ups

- **Appearance**: Small glowing green squares with white outline
- **Spawn Rate**: Every 10 seconds by default
- **Movement**: Scroll slowly across the screen
- **Collection**: Fly through them to collect
- **Effect**: Changes or upgrades your weapon
- **Progression**: Each pickup counts toward unlocking the Rocket Shooter; collect 10 to gain access
- **Types**: Randomly grants Basic or Laser upgrades; once unlocked you can swap to the Rocket Shooter

**Rocket Shooter**
- Unlocks per-player after collecting 10 power-ups
- Fires more slowly than the starter weapon but each rocket deals roughly triple damage
- Use the weapon swap key to cycle through Basic → Laser → Rocket once unlocked

### Levels

- **Level Progression**: Advance when all monsters are defeated
- **Difficulty Scaling**: Each level spawns more monsters
  - Formula: `MonsterPerLevel × LevelNumber`
  - Example: If MonsterPerLevel=3, level 1 has 3 monsters, level 2 has 6, level 3 has 9
- **Monster Variety**: Higher spawn rates for stronger enemy types in later waves
- **Notification**: Screen notification when new level begins

## Scoring

Currently, the game tracks:
- Monsters destroyed (not yet displayed in HUD)
- Player survival
- Level progression

*Note: Score display and leaderboard are planned features*

## Multiplayer

### Joining a Game

1. Ensure the server is running
2. Launch the client
3. You'll automatically connect and be assigned a player ID (0-3)
4. You spawn with a unique color:
   - Player 1: Cyan
   - Player 2: Orange
   - Player 3: Purple
   - Player 4: Pink

### Cooperative Play

- All players share the same objective: defeat all monsters
- Power-ups can be collected by any player
- Players can't damage each other
- Work together to clear waves efficiently
- Spread out to cover more of the screen
- Communicate to coordinate power-up collection

### Player Colors

Each player has a distinct color to easily identify themselves:
- **Player 0**: Cyan (95, 205, 228)
- **Player 1**: Orange (255, 174, 79)
- **Player 2**: Purple (140, 122, 230)
- **Player 3**: Pink (255, 99, 146)

## Tips and Strategy

### Survival Tips
- Keep moving - sitting still makes you an easy target (future enemy projectiles)
- Stay in the center area for maximum maneuverability
- Watch your fire cooldown - mashing fire won't help
- Upgrade your weapon when possible - higher levels make clearing waves faster

### Weapon Strategy
- **Basic**: Reliable, consistent damage. Good all-around choice.
- **Laser**: Sustained beam that melts clustered enemies once unlocked.
- **Rocket Shooter**: Slow rate of fire but devastating burst damage; shine against tanky targets.

### Power-Up Management
- Let players with weak weapons collect power-ups first
- If you're max level, let teammates get power-ups
- Don't let power-ups scroll off screen - someone should grab them
- Coordinate who takes which weapon type

### Multiplayer Tactics
- **Formation Flying**: Spread vertically to cover all enemy spawn heights
- **Focus Fire**: Multiple players shooting same target for quick kills
- **Power-up Rotation**: Take turns collecting to ensure even distribution
- **Screen Coverage**: Don't cluster - cover more area to intercept enemies early

### Level Strategy
- Early levels: Focus on practicing movement and aiming
- Mid levels: Start prioritizing weapon upgrades
- Late levels: Coordinate with team, focus fire on tough enemies

## Game Over Conditions

Currently, players respawn is not implemented. When you reach 0 HP:
- Your ship is marked as not alive
- You can no longer move or shoot
- Game continues for other players

*Note: Respawn mechanics and game over screen are planned features*

## Visual Indicators

- **Player Ships**: Colored circles (player color)
- **Bullets**: Small yellow circles
- **Enemies**: Colored circles (varies by type)
  - Red: Type 0
  - Orange: Type 1
  - Purple: Type 2
  - Bright Red: Type 3
- **Power-Ups**: Small green circles with white outline
- **Background**: Starfield scrolling right to left

## Performance Tips

If you experience lag or low FPS:

1. **Reduce Visual Complexity**:
   - Edit `config/game.ini`
   - Reduce `StarCount` (try 50 instead of 100)
   - Lower `TargetFPS` (try 30 if 60 is too demanding)

2. **Network Optimization**:
   - Play on localhost for best performance
   - Close bandwidth-heavy applications
   - Check ping to server

3. **System Resources**:
   - Close other applications
   - Update graphics drivers
   - Ensure adequate cooling (thermal throttling)

## Customization

Players can customize their experience by editing `config/game.ini`:

### Make Game Easier
```ini
[Gameplay]
PlayerStartHP=5
MonsterSpawnDelay=3.0
PowerUpSpawnDelay=5.0
```

### Make Game Harder  
```ini
[Gameplay]
PlayerStartHP=1
MonsterSpawnDelay=1.0
PowerUpsEnabled=false
PlayerFireCooldown=0.5
```

### Change Visual Style
```ini
[Render]
BackgroundColor=0,0,0        # Pure black
Player1Color=255,0,0         # Red
BulletColor=255,255,255      # White
```

See [Configuration Guide](configuration_guide.md) for all options.

## Known Limitations / Future Features

**Current Limitations:**
- Enemies don't shoot back (yet)
- No player respawn
- No score display or HUD
- No sound effects or music
- Simple geometric shapes for graphics
- No boss encounters
- Limited to 4 players

**Planned Features:**
- Enemy projectiles and attack patterns
- Boss battles at end of levels
- Player respawn system
- Score tracking and display
- Sprite graphics (replace geometric shapes)
- Audio (SFX and background music)
- More weapon types
- Shield/special ability system
- More game modes (versus, survival)
- Replay system

## Troubleshooting

### Can't connect to server
- Verify server is running: look for "listening on UDP port 5000"
- Check network configuration in config file
- Ensure firewall allows UDP traffic

### Controls not working
- Click on the game window to focus it
- Verify keyboard is working
- Check for conflicting key bindings from other software

### Low performance
- See Performance Tips section above
- Try running in release mode instead of debug
- Check system requirements

### Power-ups not spawning
- Check `PowerUpsEnabled=true` in config
- Verify `PowerUpSpawnDelay` is reasonable (not too high)
- Give the game time - default is 10 seconds between spawns

## Support

For issues, questions, or suggestions:
- Check documentation in `docs/` folder
- Review server console output for errors
- Check client console for connection issues
- See [Developer Guide](developer_guide.md) for technical details
