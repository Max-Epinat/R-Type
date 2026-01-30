# Accessibility

This document summarizes the accessibility guarantees implemented in the R-Type remake and highlights future improvements planned for better inclusivity.

## Current Measures

### Controls
- **Input method**: Single-key inputs using arrow keys and space bar, making it simple for accessibility devices
- **Polling strategy**: Lightweight input polling to support on-screen keyboards and alternative input devices
- **Future plan**: Expose configurable keybindings in a future release, stored in `controls.ini`

### Visual Design
- **Contrast**: Dark background (RGB 6,10,26) with bright ships and bullets for high visibility
- **Shape differentiation**: Critical information is not color-only:
  - Players: Rectangles
  - Monsters: Squares  
  - Bullets: Circles
  - Power-ups: Distinct shapes with outlines
- **Future plan**: Provide colorblind-friendly palettes (alternative player colors) switchable via configuration

### Audio
- **Optional SFX**: All sound effects are optional and can be disabled via `config/game.ini` (`Audio.Enabled=false`)
- **Volume levels**: Conservative defaults with adjustable master volume:
  - Explosion: 95%
  - Shoot: 35%
  - PowerUp collection: 75%
  - Hit: 55%
  - Background music: 50%
- **No audio spikes**: Sound playback gated through single buffer management
- **Configuration**: `MasterVolume` and `SFXVolume` settings in `game.ini`

### Motion
- **Scroll speed**: Starfield scroll speed capped by `Render.StarSpeedMax` and `Gameplay.ScrollSpeed` configuration
- **No camera shake**: Steady camera movement to reduce motion sickness risk
- **No flashing effects**: Conservative visual design without rapid flashing

### Feedback
- **Immediate response**: Client plays a short tone when firing, providing instant feedback even before server confirmation
- **Visual feedback**: Entity state changes are immediately reflected in rendering

## Future Recommendations

1. **Configurable Keybindings**
   - Add remapping support in `controls.ini`
   - Allow multiple input methods (keyboard, gamepad, custom devices)

2. **Enhanced Visual Options**
   - Colorblind-friendly palette presets
   - Adjustable sprite sizes
   - Optional high-contrast mode

3. **UI and Text**
   - When HUD text is added, ensure:
     - Adjustable font sizes
     - High contrast text
     - No rapid flashing text
     - Screen reader compatibility

4. **In-Game Options Menu**
   - Pause/options menu for runtime adjustment of:
     - Audio levels
     - Visual settings
     - Control preferences
   - Avoid requiring file editing for accessibility settings

5. **Seizure-Safe Design**
   - Document maximum flashes per second
   - Implement flash reduction options
   - Provide warnings if necessary

## Testing and Validation

When testing accessibility features:
- Verify settings in `config/game.ini` are respected
- Test with keyboard-only input
- Verify audio mute functionality
- Check visual contrast in different lighting conditions
- Test with screen magnification tools

## Contributing

Contributions improving accessibility are highly encouraged. When submitting accessibility improvements:
- Reference this document in your PR
- Test with actual accessibility tools when possible
- Document new settings in [configuration_guide.md](configuration_guide.md)
- Update this checklist with implemented features

## Related Documentation

- [configuration_guide.md](configuration_guide.md) - How to configure audio/visual settings
- [sound_system.md](sound_system.md) - Detailed audio system documentation
- [developer_guide.md](developer_guide.md) - How to add new accessibility features
- [docs/wiki/Accessibility.md](wiki/Accessibility.md) - Wiki mirror of this document
