# TMOP Engine 0.0.102

## Anchor display names

`TMOPHistoricalAnchor` now keeps `DisplayName` synchronized with its Entity ID
automatically during construction. A manually entered display name is no longer
required and an Entity ID rename updates the visible anchor label.

## Venue layout anchors

Place one `TMOPVenueLayoutImporter` in the level and press
`Import Or Update Venue Layout Anchors` in its Details panel. The default file is:

`Content/TMOP/Data/TMOP_VENUE_LAYOUT_ANCHORS.json`

The import is repeatable. Existing generated IDs are updated instead of duplicated.
Each layout is transformed relative to the corresponding `<Venue>_inside` anchor,
including its yaw rotation.

Per venue it creates:

- 20 table seats: four seats at each of five tables.
- 5 customer positions at the bar.
- 3 standing staff positions behind the bar.
- 4 musician positions on the stage.

`CafeMonCheri` and `ResturangBohemia` intentionally have no stage positions.

The positions are a reusable first-pass layout. Move individual generated anchors
after import if the actual room geometry requires local adjustment. Reimporting with
updates enabled restores the generated template positions.
