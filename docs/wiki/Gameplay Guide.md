# Gameplay Guide

A classic horizontal shoot-em-up supporting up to 4 players cooperatively fighting through multiple levels.

## Controls

### Keyboard
- **Arrow Keys** / **WASD**: Move ship (up/down/left/right)
- **Space**: Fire weapon
- **Escape**: Quit game

**Notes:**
- Hold fire to continuously shoot (subject to cooldown)
- Ship stays within screen boundaries
- Diagonal movement supported

## Game Mechanics

### Your Ship
- **Starting Health**: 3 HP (configurable)
- **Speed**: Fast, responsive
- **Weapon**: Starts basic, upgradeable via power-ups
- **Position**: Players spawn at different vertical positions on the left

### Weapons

Three weapon types, each with 3 upgrade levels:

**1. Basic Weapon (Type 0)** - Yellow projectiles
- Level 1: 1 damage | Level 2: 2 damage | Level 3: 3 damage

**2. Laser Weapon (Type 1)** - Cyan projectiles
- Level 1: 2 damage | Level 2: 4 damage | Level 3: 6 damage

**3. Rocket Shooter (Type 2)** - Ember rockets (unlocked after 10 power-ups)
- Level 1-3: 3×-9× basic damage
- Fires slower but pierces tougher foes with raw damage

**Upgrading:**
- Collect power-ups (green glowing items) to upgrade
- Same weapon type → level up (max level 3)
- Different weapon type → replace current weapon at level 1

### Enemies

Four types with increasing difficulty:

| Type | Color | HP | Size | Description |
|------|-------|-----|------|-------------|
| 0 | Red | 3 | Small | Most common, early waves |
| 1 | Orange | 5 | Medium | Standard enemy |
| 2 | Purple | 8 | Large | Durable |
| 3 | Bright Red | 10 | Very Large | Rare, mini-boss difficulty |

**Behavior:**
- Spawn from right side
- Move left across screen
- Despawn at left edge
- No projectiles (future feature)

### Power-Ups

- **Appearance**: Small green squares with white outline
- **Spawn Rate**: Every 10 seconds (default)
- **Effect**: Changes/upgrades weapon
- **Rocket Unlock**: Collect 10 power-ups to unlock Rocket Shooter

### Levels

- **Progression**: Advance when all monsters defeated
- **Scaling**: More monsters each level
  - Formula: `MonsterPerLevel × LevelNumber`
  - Example: Level 1 = 3 monsters, Level 2 = 6 monsters (if MonsterPerLevel=3)
- **Difficulty**: Higher spawn rates for stronger enemy types in later levels

## Multiplayer

### Player Colors
- **Player 0**: Cyan (95, 205, 228)
- **Player 1**: Orange (255, 174, 79)
- **Player 2**: Purple (140, 122, 230)
- **Player 3**: Pink (255, 99, 146)

### Cooperative Play
- Shared objective: defeat all monsters
- Power-ups collectible by any player
- No friendly fire
- Work together to clear waves efficiently

## Strategy

### Survival Tips
- Keep moving for maximum maneuverability
- Stay in center area
- Watch fire cooldown
- Upgrade weapons ASAP

### Weapon Strategy
- **Basic**: Reliable all-around
- **Laser**: Melts clustered enemies
- **Rocket**: Devastating burst damage for tanky targets

### Multiplayer Tactics
- **Formation Flying**: Spread vertically to cover spawn heights
- **Focus Fire**: Multiple players on same target for quick kills
- **Power-up Rotation**: Take turns for even distribution
- **Screen Coverage**: Don't cluster - intercept enemies early

### Power-Up Management
- Let players with weak weapons collect first
- Max level players should let teammates grab them
- Don't let them scroll off screen
- Coordinate weapon type distribution

## Customization

Edit `config/game.ini` to customize gameplay:

### Easier
```ini
[Gameplay]
PlayerStartHP=5
MonsterSpawnDelay=3.0
PowerUpSpawnDelay=5.0
```

### Harder
```ini
[Gameplay]
PlayerStartHP=1
MonsterSpawnDelay=1.0
PowerUpsEnabled=false
PlayerFireCooldown=0.5
```

See [[Configuration]] for all options.

## Visual Indicators

- **Player Ships**: Colored circles (player color)
- **Bullets**: Small yellow circles
- **Enemies**: Colored circles by type (red/orange/purple/bright red)
- **Power-Ups**: Small green circles with white outline
- **Background**: Starfield scrolling right to left

## Performance Tips

If experiencing lag:

1. **Reduce Visual Complexity** in `config/game.ini`:
   - Lower `StarCount` (try 50 instead of 100)
   - Reduce `TargetFPS` (try 30 instead of 60)

2. **Network Optimization**:
   - Play on localhost for best performance
   - Close bandwidth-heavy applications

3. **System Resources**:
   - Close other applications
   - Update graphics drivers

## Known Limitations

**Current:**
- Enemies don't shoot back
- No player respawn
- No score display/HUD
- No audio
- Simple geometric shapes (no sprites)
- Limited to 4 players

**Planned:**
- Enemy projectiles and attack patterns
- Boss battles
- Player respawn
- Score tracking and display
- Sprite graphics
- Audio (SFX and music)
- More weapon types
- Shield/special abilities
- More game modes (versus, survival)

## Troubleshooting

### Can't connect to server
- Verify server is running
- Check `config/game.ini` network settings
- Ensure firewall allows UDP traffic

### Controls not working
- Click game window to focus
- Check for key binding conflicts

### Power-ups not spawning
- Verify `PowerUpsEnabled=true` in config
- Check `PowerUpSpawnDelay` setting
- Default spawn time is 10 seconds

## Related Documentation

- [[Building and Running]] - How to start server and client
- [[Configuration]] - Full configuration options
- [gameplay_guide.md](../gameplay_guide.md) - Extended gameplay documentation (308 lines)
- [configuration_guide.md](../configuration_guide.md) - Detailed configuration examples
