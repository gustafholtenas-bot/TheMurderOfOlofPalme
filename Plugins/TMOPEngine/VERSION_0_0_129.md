# TMOPEngine 0.0.129

## Detailed typography and safe 3D labels

- Replaces broad typography roles with specific roles for every native TMOP UI.
- Adds a `Used By` description to every typography row.
- Shows the complete style reference directly on `TMOPTypographyDirector`.
- Adds editable shared menu, panel, button, status, accent and intro-card colours.
- Adds specific map and common Blueprint HUD styles.
- Keeps backward-compatible fallbacks to the old generic rows.
- Stops Composite Fonts, UI sizes and UI colours from automatically overwriting
  world-space person and vehicle labels.
- Restores each agent's label before optional world-text overrides are applied.
