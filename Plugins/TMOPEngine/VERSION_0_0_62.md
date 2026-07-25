# TMOP Engine 0.0.62

## Place historical vehicles at anchors

`Initial Placement` and `Spawn` entries in `DT_TMOP_HistoricalVehicles` now
support two placement modes:

- `World Transform` preserves the existing behaviour.
- `Anchor` reads position and rotation from a historical anchor.

For Lars Knubb's car:

1. Open `VEHICLE_LARS_KNUBBS_BIL`.
2. Expand its `Initial Placement` timeline entry.
3. Set `Placement Mode` to `Anchor`.
4. Set `Placement Anchor ID` to `LarsKnubbBil`.
5. Leave `Anchor Local Offset` at identity initially.

The vehicle inherits both location and rotation from the anchor. If its mesh
faces the wrong direction, change only the yaw in `Anchor Local Offset`, usually
to 180 degrees. Use its local location offset for small parking adjustments.

If an anchor cannot be found at runtime, the system logs an error and safely
falls back to the entry's existing `World Transform`.

Version 0.0.62 also contains all 0.0.61 automatic-routing and cinema-seat fixes.
