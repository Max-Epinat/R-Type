# Accessibility

This checklist summarizes the guarantees implemented so far and highlights future actions.

## Current Measures
- **Controls:** Single-key inputs (arrows + space) keep accessibility devices simple. Plan to expose remapping in a future release.
- **Visual contrast:** Dark background (RGB 6,10,26) with bright ships/bullets. Shapes differ per entity (rectangles for players, squares for monsters, circles for bullets) to avoid color-only cues.
- **Motion:** Starfield scroll speed capped by `Render.StarSpeedMax` and `Gameplay.ScrollSpeed`—no camera shake or flashing.
- **Audio:** Optional SFX generated locally with conservative volume; a mute toggle can be added by flipping `Audio.Enabled=false`.
- **Feedback:** Client plays a short tone when firing even before the server confirms the shot.

## Recommendations
1. Add configurable keybindings and store them in a `controls.ini` file.
2. Provide colorblind-friendly palettes (e.g., alt player colors) switchable via config.
3. Introduce on-screen HUD text with adjustable font sizes; avoid rapid flashing text.
4. Add an in-game pause/options menu so players can tweak audio/visual settings without editing files.
5. Document seizure-safe design (max flashes per second) once more visual effects are added.

Contributions improving accessibility are welcome—open an issue or PR referencing this page.

## Related Documentation
- [accessibility.md](../accessibility.md) - Extended accessibility documentation
- [[Configuration]] - How to adjust audio/visual settings
- [[Contributing]] - How to submit accessibility improvements
- [developer_guide.md](../developer_guide.md) - Development guidelines
