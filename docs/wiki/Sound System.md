# Sound System

The R-Type sound system provides audio feedback for game events using SFML's audio capabilities, combining file-based audio and procedural sound generation.

## Audio Files

Loaded from `src/assets/sound/`:

- **background music.mp3** - Normal gameplay music
- **boss2damage.mp3** - Boss battle music
- **kamikaze explosion.mp3** - Kamikaze enemy explosion SFX
- **playerdamage.mp3** - Player damage SFX
- **powerup.mp3** - Power-up collection SFX

## Sound Effects

### Player Shoot
- **Trigger**: Player fires weapon
- **Type**: Procedurally generated
- **Characteristics**: High-pitched beep (880Hz, 50ms)
- **Volume**: 40%

### Enemy Shoot
- **Trigger**: Enemy fires projectile
- **Type**: Procedurally generated
- **Characteristics**: Menacing tone (440Hz, 80ms)
- **Volume**: 35%

### Player Hit
- **Trigger**: Player takes damage
- **Source**: playerdamage.mp3 (25%) or procedural fallback (200Hz, 150ms, 15%)

### Explosion
- **Trigger**: Regular enemy destroyed
- **Type**: Procedurally generated
- **Characteristics**: Low rumble with decay (150Hz, 300ms)
- **Volume**: 95%

### Kamikaze Explosion
- **Trigger**: Kamikaze enemy (type 5) explodes
- **Source**: kamikaze explosion.mp3 (200%) or procedural fallback (180Hz, 250ms)

### Power-Up Collection
- **Trigger**: Player collects power-up
- **Source**: powerup.mp3 (75%) or procedural fallback (1200Hz, 200ms)

## Background Music

### Normal Music
- **File**: background music.mp3
- **Volume**: 50%
- **Behavior**: Loops continuously, stops on boss levels

### Boss Music
- **File**: boss2damage.mp3
- **Volume**: 30%
- **Behavior**: Plays on boss levels (10, 15), more intense

## Technical Details

### Procedural Sound Generation

Uses sine waves with:
- **Sample rate**: 44,100 Hz
- **Envelope**: Decay for natural sound
- **Harmonics**: Richer audio texture

**Parameters:**
- Player shoot: 880Hz, 50ms, 3000 amplitude
- Enemy shoot: 440Hz, 80ms, 2500 amplitude
- Explosion: 150Hz, 300ms, 5000 amplitude
- Fallback hit: 200Hz, 150ms, 4000 amplitude
- Fallback kamikaze: 180Hz, 250ms, 6000 amplitude
- Fallback powerup: 1200Hz, 200ms, 3500 amplitude

### Fallback System

If audio files can't be loaded, system uses procedural generation automatically. No crashes, guaranteed audio feedback.

### Automatic Triggers

Sound system detects and plays:
- Level changes → music switching
- Monster deaths → explosion sounds
- Player damage → hit sound
- Enemy firing → enemy shoot sound
- Power-up collection → powerup sound

## Sound System API

```cpp
// Play specific sounds
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

## Adding New Sounds

1. Place audio file (MP3/WAV/OGG/FLAC) in `src/assets/sound/`
2. Add `sf::SoundBuffer` and `sf::Sound` member to `SFMLRenderer.hpp`
3. Load file in `initializeSoundSystem()` in `SFMLRenderer.cpp`
4. Add case in `playSound()` function
5. Trigger with `playSound("your_sound_id")`

## Volume Configuration

Adjust in `SFMLRenderer.cpp`:

```cpp
void SFMLRenderer::initializeSoundSystem() {
    _playerShootSound.setVolume(40.0f);      // Player weapon
    _enemyShootSound.setVolume(35.0f);       // Enemy weapon
    _playerHitSound.setVolume(25.0f);        // Player damage (file)
    _explosionSound.setVolume(95.0f);        // Regular explosions
    _kamikazeExplosionSound.setVolume(200.0f); // Kamikaze (boosted)
    _powerUpSound.setVolume(75.0f);          // Power-up
    _backgroundMusic.setVolume(50.0f);       // Background music
    _bossMusic.setVolume(30.0f);             // Boss music
}
```

**Volume range**: 0-100 (can exceed 100 for boosting)

## Enable/Disable

Controlled by `config.audio.enabled` setting. When disabled, no sounds play.

Edit `config/game.ini`:
```ini
[Audio]
Enabled=true
MasterVolume=100
SFXVolume=80
```

## Related Documentation

- [[Accessibility]] - Audio accessibility features
- [[Configuration]] - Audio configuration options
- [sound_system.md](../sound_system.md) - Extended sound system documentation
- [developer_guide.md](../developer_guide.md) - Adding new features
