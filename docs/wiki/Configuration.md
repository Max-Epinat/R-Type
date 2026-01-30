# Configuration

All tunable parameters live in `config/game.ini`. Both binaries load the file at startup, so you can tweak gameplay, rendering, audio, and network settings without recompiling.

## File Resolution Order
1. `config/game.ini` (project root)
2. `../config/game.ini` (if running from `build/` or `bin/`)

## Sections
### [Gameplay]
Controls movement speeds, spawn logic, bullet behavior, world bounds, and power-up cadence.
- `ScrollDirection`: `LeftToRight`, `RightToLeft`, `TopToBottom`, `BottomToTop`.
- `PlayerSpeed`, `PlayerStartHP`, `PlayerFireCooldown`.
- `BulletSpeed`, `BulletLifetime`, `BulletDirection`.
- `MonsterSpawnDelay`, `MonsterType{0-3}Size`, HP, spawn weights.
- `PowerUpsEnabled`, spawn side, size, outline styling.

### [Render]
Window settings and color palette.
- `WindowWidth`, `WindowHeight`, `TargetFPS`.
- Player/bullet sizing and colors (`Player{1-4}Color`, `BulletColor`).
- Starfield density: `StarCount`, `StarSpeedMin/Max`, `StarSizeMin/Max`.
- `BackgroundColor` (RGB).

### [Network]
Socket defaults.
- `DefaultHost`, `DefaultPort` (client uses these to reach the server).
- `MaxPlayers`, `RxBufferSize`, `ServerTimeout` (seconds before timeout message).

### [Audio]
- `MasterVolume`, `SFXVolume`, `Enabled`.

## Usage Patterns
- **Preset tweak:** Copy the section you want to change, update values, and restart the binary.
- **Difficulty scaling:** Increase `MonsterSpawnDelay` for easier runs; crank up `ScrollSpeed` and monster HP for harder sessions.
- **Orientation swap:** Switch `ScrollDirection` to vertical (`TopToBottom`/`BottomToTop`) to turn the game into a shoot-'em-up scroller.

## Adding New Fields
1. Add the member to `config::GameplayConfig` (or the relevant struct).
2. Parse it in `GameConfig::loadFromFile` and emit it in `saveToFile`.
3. Reference the new field where needed (server logic, client rendering, etc.).
4. Update this page and the default `game.ini` so others discover the knob.

For ready-made examples (easy, hard, vertical) see `docs/configuration_guide.md` in the repo or mirror them into this wiki as needed.

## Related Documentation
- [configuration_guide.md](../configuration_guide.md) - Detailed configuration examples and presets
- [[Modules and API]] - GameConfig API reference
- [[Architecture]] - How configuration is loaded at runtime
- [developer_guide.md](../developer_guide.md) - Development and testing workflows
