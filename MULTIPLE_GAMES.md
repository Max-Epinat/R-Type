# Multiple Game Support

This engine supports multiple game configurations. By default, it runs the **R-Type** game, but it also includes a **Space Invaders** configuration.

## Switching Between Games

The engine loads its configuration from the `config/` folder at the root of the project. To switch between games, you need to swap which configuration folder is active.

### Playing Space Invaders

1. **Backup the current R-Type configuration:**
   ```bash
   mv config config_rtype_backup
   ```

2. **Activate the Space Invaders configuration:**
   ```bash
   mv config_spaceinvaders config
   ```

3. **Run the game** as normal using your preferred launch script

### Switching Back to R-Type

1. **Restore the R-Type configuration:**
   ```bash
   mv config config_spaceinvaders
   mv config_rtype_backup config
   ```

2. **Run the game** as normal

## Available Configurations

- **config_rtype/** - Original R-Type game configuration
- **config_spaceinvaders/** - Space Invaders game configuration
- **config/** - Active configuration (loaded by the engine)

## Configuration Files

Each configuration folder contains:
- `assets.ini` - Asset paths and sprite definitions
- `engine.ini` - Core engine settings (window size, networking, etc.)
- `game.ini` - Game-specific settings (scroll direction, player stats, etc.)
- `systems.ini` - System toggles (collision, spawning, powerups, etc.)

You can customize these files to create your own game variations!
