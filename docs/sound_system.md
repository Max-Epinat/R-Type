# Sound System Documentation

> **Related Documentation:**
> - [Architecture](architecture.md) — System design overview
> - [Developer Guide](developer_guide.md) — Adding new features
> - [Configuration Guide](configuration_guide.md) — Audio settings

## Overview

The R-Type game features a modular sound system that provides audio feedback for various game events. The system uses SFML's audio capabilities, loading audio files from disk for music and key sound effects, while using procedural generation for weapon sounds.

## Audio Files

The following audio files are loaded from `src/assets/sound/`:

- **background music.mp3** - Background music during normal gameplay
- **boss2damage.mp3** - Music that plays during boss battles
- **kamikaze explosion.mp3** - Sound effect for kamikaze enemy explosions
- **playerdamage.mp3** - Sound effect when the player takes damage
- **powerup.mp3** - Sound effect when collecting power-ups

## Sound Effects

### 1. Player Shoot Sound
- **Trigger**: When the player fires a weapon
- **Source**: Procedurally generated
- **Characteristics**: High-pitched beep (880Hz)
- **Duration**: 50ms
- **Volume**: 40%

### 2. Enemy Shoot Sound
- **Trigger**: When an enemy fires a projectile
- **Source**: Procedurally generated
- **Characteristics**: Lower-pitched, menacing tone (440Hz)
- **Duration**: 80ms
- **Volume**: 35%

### 3. Player Hit Sound
- **Trigger**: When the player takes damage
- **Source**: Audio file (playerdamage.mp3)
- **Volume**: 25% (from file) or 15% (procedurally generated fallback)
- **Fallback**: Procedurally generated if file not found (200Hz, 150ms)

### 4. Explosion Sound
- **Trigger**: When a regular enemy is destroyed
- **Source**: Procedurally generated
- **Characteristics**: Low rumble with decay (150Hz)
- **Duration**: 300ms
- **Volume**: 95%

### 5. Kamikaze Explosion Sound
- **Trigger**: When a kamikaze enemy (type 5) explodes
- **Source**: Audio file (kamikaze explosion.mp3)
- **Volume**: 200% (louder, more impactful)
- **Fallback**: Procedurally generated if file not found (180Hz, 250ms, 6000 amplitude)

### 6. Power-Up Collection Sound
- **Trigger**: When the player collects a power-up
- **Source**: Audio file (powerup.mp3)
- **Volume**: 75%
- **Fallback**: Procedurally generated if file not found (1200Hz, 200ms)

## Background Music

### Normal Background Music
- **Trigger**: Automatically starts when gameplay begins
- **Source**: Audio file (background music.mp3)
- **Characteristics**: Loops continuously
- **Volume**: 50%
- **Behavior**: Stops when entering boss levels

### Boss Battle Music
- **Trigger**: Automatically starts on boss levels (levels 10 and 15)
- **Source**: Audio file (boss2damage.mp3)
- **Characteristics**: More intense, loops continuously
- **Volume**: 30%
- **Behavior**: Stops when boss level is completed

## Technical Implementation

### Sound Loading
The system attempts to load audio files from the `src/assets/sound/` directory. If a file cannot be loaded, it falls back to procedural generation for sound effects.

### Sound Generation
Procedural sound effects are generated using sine waves with:
- Customizable frequency
- Envelope with decay for natural-sounding effects
- Harmonics for richer audio texture
- Sample rate: 44,100 Hz
- Amplitude control for volume variation

**Procedural Sound Parameters:**
- Player shoot: 880Hz, 50ms, 3000 amplitude
- Enemy shoot: 440Hz, 80ms, 2500 amplitude
- Explosion: 150Hz, 300ms, 5000 amplitude
- Fallback hit sound: 200Hz, 150ms, 4000 amplitude
- Fallback kamikaze: 180Hz, 250ms, 6000 amplitude
- Fallback powerup: 1200Hz, 200ms, 3500 amplitude

### Sound System API

```cpp
// Play specific sound effects
playSound("player_shoot");
playSound("enemy_shoot");
playSound("player_hit");
playSound("explosion");
playSound("kamikaze_explosion");
playSound("powerup");

// Control music
playSound("background_music");
playSound("boss_music");
playSound("stop_music");
```

### Automatic Triggers

The sound system automatically detects and plays sounds for:
- **Level changes**: Switches between normal and boss music
- **Monster deaths**: Plays appropriate explosion sounds
- **Player damage**: Plays hit sound when HP decreases
- **Enemy firing**: Plays enemy shoot sound for new enemy bullets
- **Kamikaze explosions**: Special explosion sound for kamikaze enemies
- **Power-up collection**: Plays sound when power-ups are collected

## Adding New Sounds

To add new sound effects:

1. Place the audio file (MP3, WAV, OGG, or FLAC) in `src/assets/sound/`
2. Add a `sf::SoundBuffer` and `sf::Sound` member to `SFMLRenderer.hpp`
3. Load the file in `initializeSoundSystem()` in `SFMLRenderer.cpp`
4. Add a case in the `playSound()` function to handle the new sound
5. Trigger the sound by calling `playSound("your_sound_id")` at appropriate locations

## Configuration

Sound parameters can be adjusted in the `initializeSoundSystem()` function in `SFMLRenderer.cpp`:

```cpp
void SFMLRenderer::initializeSoundSystem() {
    // Adjust volume levels (0-100, can go above 100 for boosting)
    _playerShootSound.setVolume(40.0f);      // Player weapon sounds
    _enemyShootSound.setVolume(35.0f);       // Enemy weapon sounds
    _playerHitSound.setVolume(25.0f);        // Player damage (from file)
    _explosionSound.setVolume(95.0f);        // Regular explosions
    _kamikazeExplosionSound.setVolume(200.0f); // Kamikaze explosions (boosted)
    _powerUpSound.setVolume(75.0f);          // Power-up collection
    _backgroundMusic.setVolume(50.0f);       // Background music
    _bossMusic.setVolume(30.0f);             // Boss battle music
    // ... etc
}
```

**Audio Enable/Disable:**
The sound system respects the `_config.audio.enabled` setting. When disabled, no sounds will play.
