# TMOP Engine 0.0.68

## Intersection-corner generator

Select one or more general intersection `TMOPHistoricalAnchor` actors and run:

`Tools > Generate Intersection Corners From Selection`

For `kammakargXSveav`, the tool creates:

- `kammakargXSveav_NW`
- `kammakargXSveav_NE`
- `kammakargXSveav_SW`
- `kammakargXSveav_SE`

Each generated anchor:

- starts at the parent transform,
- uses `Street Corner` as its category,
- stores the intersection in `Parent Anchor ID`,
- prefers sidewalk routing,
- projects placement to NavMesh,
- is grouped below the parent in the World Outliner.

Existing matching IDs are skipped. The operation supports Undo.

Move each generated anchor to the corresponding geographical pavement corner.
The compass suffix describes the real-world corner, not the anchor actor's
local rotation.

Version 0.0.68 includes the exit generator and all previous fixes and features.
