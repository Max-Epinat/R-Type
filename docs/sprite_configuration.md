# Sprite Configuration Guide

## Overview

R-Type uses JSON files to define sprite metadata for all visual assets. These files specify the location of sprites within texture files and control animation properties.

## File Location

Sprite JSON files are located in:
```
src/assets/textures/<texture-pack-name>/
```

For the default texture pack:
```
src/assets/textures/default/
```

## JSON File Structure

### Basic Template

```json
{
  "texture": "spritesheet-filename.gif",
  "sprites": {
    "sprite_name": {
      "frames": [
        { "x": 0, "y": 0, "width": 32, "height": 32 }
      ],
      "animate": false,
      "frameDuration": 0.0,
      "canRotate": true,
      "defaultDirection": "left"
    }
  }
}
```

### Root Object Fields

#### `texture` (required)
- **Type**: `string`
- **Description**: Filename of the texture/spritesheet image (usually a .gif or .png)
- **Example**: `"r-typesheet7.gif"`

#### `sprites` (required)
- **Type**: `object`
- **Description**: Container for all sprite definitions in this file
- Each key is a sprite identifier, each value is a sprite configuration object

## Sprite Configuration Object

### `frames` (required)
- **Type**: `array` of frame objects
- **Description**: Defines one or more rectangular regions in the texture
- **Single frame**: Non-animated sprites (e.g., static objects)
- **Multiple frames**: Animated sprites (e.g., enemies with movement cycles)

#### Frame Object Structure
```json
{ "x": 10, "y": 20, "width": 32, "height": 32 }
```

- **`x`** (required): Left edge position in pixels from texture origin (top-left is 0,0)
- **`y`** (required): Top edge position in pixels from texture origin
- **`width`** (required): Frame width in pixels
- **`height`** (required): Frame height in pixels

**Important**: Coordinates must be precise. Frames should not overlap or extend beyond texture boundaries.

### `animate` (optional)
- **Type**: `boolean`
- **Default**: `false`
- **Description**: Whether to cycle through frames automatically
- **Usage**:
  - `true`: Play animation continuously (enemies, explosions)
  - `false`: Display only the first frame (static objects)

### `frameDuration` (optional)
- **Type**: `float` (seconds)
- **Default**: `0.0`
- **Description**: Time to display each frame before advancing to next
- **Only used when** `animate` is `true`
- **Example values**:
  - `0.06`: Very fast animation (60 FPS = ~4 frames)
  - `0.12`: Medium speed animation
  - `0.3`: Slow animation

### `canRotate` (optional)
- **Type**: `boolean`
- **Default**: `true`
- **Description**: Whether the sprite can be rotated based on movement direction
- **Usage**:
  - `true`: Sprite rotates to face movement direction (most enemies)
  - `false`: Sprite maintains fixed orientation (some projectiles)

### `defaultDirection` (optional)
- **Type**: `string`
- **Default**: `"left"`
- **Description**: Initial facing direction of the sprite
- **Valid values**: `"left"`, `"right"`, `"up"`, `"down"`
- **Note**: This affects how rotation is applied when `canRotate` is true

## Complete Examples

### Example 1: Non-Animated Sprite
```json
{
  "texture": "r-typesheet24.gif",
  "sprites": {
    "powerup": {
      "frames": [
        { "x": 84, "y": 212, "width": 16, "height": 16 }
      ],
      "animate": false,
      "canRotate": false,
      "defaultDirection": "left"
    }
  }
}
```

### Example 2: Animated Enemy
```json
{
  "texture": "r-typesheet7.gif",
  "sprites": {
    "monster": {
      "frames": [
        { "x": 1, "y": 4, "width": 32, "height": 29 },
        { "x": 34, "y": 4, "width": 32, "height": 29 },
        { "x": 67, "y": 4, "width": 32, "height": 29 },
        { "x": 1, "y": 36, "width": 32, "height": 29 },
        { "x": 34, "y": 36, "width": 32, "height": 29 },
        { "x": 67, "y": 36, "width": 32, "height": 29 }
      ],
      "animate": true,
      "frameDuration": 0.12,
      "canRotate": true,
      "defaultDirection": "left"
    }
  }
}
```

### Example 3: Multiple Sprites in One File
```json
{
  "texture": "r-typesheet2.gif",
  "sprites": {
    "bullet_type1": {
      "frames": [
        { "x": 230, "y": 103, "width": 16, "height": 5 }
      ],
      "animate": false,
      "frameDuration": 0.0
    },
    "bullet_type2": {
      "frames": [
        { "x": 232, "y": 89, "width": 13, "height": 11 }
      ],
      "animate": false,
      "frameDuration": 0.0
    }
  }
}
```

### Example 4: Complex Animation (Explosion)
```json
{
  "texture": "r-typesheet1.gif",
  "sprites": {
    "explosion": {
      "frames": [
        { "x": 72,  "y": 300, "width": 32, "height": 32 },
        { "x": 104, "y": 300, "width": 32, "height": 32 },
        { "x": 136, "y": 300, "width": 32, "height": 32 },
        { "x": 168, "y": 300, "width": 32, "height": 32 },
        { "x": 200, "y": 300, "width": 32, "height": 32 },
        { "x": 232, "y": 300, "width": 32, "height": 32 },
        { "x": 264, "y": 300, "width": 32, "height": 32 },
        { "x": 296, "y": 300, "width": 32, "height": 32 }
      ],
      "animate": true,
      "frameDuration": 0.06,
      "canRotate": false,
      "defaultDirection": "left"
    }
  }
}
```

## Linking Sprites to Game Configuration

### For Monster Types

In `config/assets.ini`, link monster types to their JSON files:
```ini
[Assets]
MonsterType0Sprites=r-typesheet5.json
MonsterType1Sprites=r-typesheet7.json
MonsterType2Sprites=r-typesheet8.json
```

**Important**: 
- The sprite name inside the JSON must be `"monster"` (generic name)
- The system automatically associates it with the correct monster type based on the filename mapping

### For Core Game Elements

```ini
PlayerShipSprites=r-typesheet1-player.json
BulletSprites=r-typesheet2.json
EnemyBulletSprites=r-typesheet2-enemy.json
PowerUpSprites=r-typesheet24.json
ExplosionSprites=r-typesheet1-explosion.json
```

## Best Practices

### 1. Frame Coordinate Precision
- **Always measure carefully**: Use an image editor to determine exact pixel coordinates
- **Avoid overlaps**: Ensure frames don't extend into adjacent sprites
- **Test boundaries**: If animations appear corrupted, reduce width/height by 1-2 pixels

### 2. Frame Spacing
When frames are arranged horizontally:
```
Frame 1: x=1,  width=32  (ends at x=33)
Frame 2: x=34, width=32  (starts at x=34, ends at x=66)
Frame 3: x=67, width=32  (starts at x=67)
```
Ensure `x(n+1) ≥ x(n) + width(n)`

### 3. Animation Timing
- **Fast-moving projectiles**: 0.04 - 0.08 seconds
- **Enemy animations**: 0.08 - 0.15 seconds
- **Explosions**: 0.05 - 0.08 seconds
- **Slow effects**: 0.2 - 0.4 seconds

### 4. Rotation Settings
- **Enemies that move horizontally**: `canRotate: true`, `defaultDirection: "left"`
- **Vertical projectiles**: `canRotate: false` or adjust `defaultDirection` to `"up"`/`"down"`
- **Omnidirectional sprites**: `canRotate: true`

### 5. Performance
- Keep sprite dimensions reasonable (8x8 to 128x128 for most objects)
- Use power-of-two dimensions when possible (16, 32, 64) for better GPU performance
- Limit animation frames to 4-16 per sprite for smooth playback

## Troubleshooting

### Sprite Appears Corrupted or Cut Off
- **Cause**: Coordinates extend beyond frame boundaries
- **Solution**: Reduce `width` and `height` by 1-2 pixels, or adjust `x` and `y`

### Animation Doesn't Play
- **Check**: `animate` is set to `true`
- **Check**: `frameDuration` is > 0
- **Check**: Multiple frames are defined

### Sprite Doesn't Load
- **Check**: JSON syntax is valid (no trailing commas, proper quotes)
- **Check**: Texture file exists in same directory
- **Check**: Sprite is properly linked in `config/assets.ini`
- **Check**: Console logs for error messages

### Wrong Rotation/Direction
- **Adjust**: `defaultDirection` value
- **Adjust**: `canRotate` setting
- **Note**: Game logic may override rotation based on entity type

## Adding New Sprites

1. **Create/obtain spritesheet** (GIF or PNG format)
2. **Place in texture directory**: `src/assets/textures/default/`
3. **Create JSON file** with same base name: `mysheet.json`
4. **Define sprite metadata** following structure above
5. **Link in assets.ini** (if needed for game elements)
6. **Test in-game** and adjust coordinates as needed

## Schema Reference

```typescript
{
  texture: string,                    // Required: Texture filename
  sprites: {
    [spriteName: string]: {
      frames: Array<{               // Required: At least one frame
        x: number,                   // Required: Left position (pixels)
        y: number,                   // Required: Top position (pixels)
        width: number,               // Required: Frame width (pixels)
        height: number               // Required: Frame height (pixels)
      }>,
      animate?: boolean,             // Optional: Default false
      frameDuration?: number,        // Optional: Default 0.0 (seconds)
      canRotate?: boolean,           // Optional: Default true
      defaultDirection?: "left" | "right" | "up" | "down"  // Optional: Default "left"
    }
  }
}
```

## Related Documentation

- [Configuration Guide](configuration_guide.md) - Game configuration files
- [Asset Management](assets.md) - Asset loading and management
- [Monster Configuration](gameplay_guide.md#monsters) - Monster type setup
