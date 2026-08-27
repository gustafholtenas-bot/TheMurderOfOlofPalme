# TMOPEngine 0.0.125

## Central typography

- Adds `TMOPTypographyDirector` and `DT_TMOP_Typography` support.
- Central rows control font asset, typeface, size, colour, shadow and outline.
- Native agent information, dialog/radio, target prompt, speech bubble,
  quick inventory, newspaper reader and pause-menu text use named styles.
- Spawned agents' world-space name labels use the `AgentName` style.
- Blueprint `UTextBlock` widgets are discovered and styled automatically.
- Blueprint code can call `Apply Typography Style` for an explicit style ID.
- `Widget Name Style Overrides` handles project-specific Blueprint names.
