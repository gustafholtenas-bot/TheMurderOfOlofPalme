# TMOP Engine 0.0.70

## Searchable timeline references

The visual TMOP People Editor now includes searchable autocomplete selectors
above the selected timeline entry's normal property panel.

Available selectors:

- `Target Anchor`
- `Target Person / Vehicle`
- `Target Seat`
- `Shared Event`

Type any part of an ID or display name to filter the list. For example,
`kamma` can match intersection corners and exits containing Kammakargatan,
while `lars` matches people or vehicles whose ID or display label contains
Lars.

Reference sources:

- Anchors are read from all loaded `TMOPHistoricalAnchor` actors in the editor
  world.
- People are read from `DT_TMOP_People`.
- Vehicles are read from `DT_TMOP_HistoricalVehicles`.
- Cinema and vehicle seats are read from loaded actor components.
- Shared events are read from `DT_TMOP_HistoricalEvents`.

The list shows useful descriptions and categories, but the timeline continues
to store only stable `FName` IDs. The original free-text properties remain
available as a fallback for IDs that have not been created or loaded yet.

Reference lists refresh whenever another timeline entry is selected.

Version 0.0.70 includes Relative to Previous Entry, both anchor generators,
and all previous fixes and features.
