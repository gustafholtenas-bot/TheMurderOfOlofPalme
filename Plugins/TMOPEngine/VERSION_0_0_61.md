# TMOP Engine 0.0.61

## Lars Knubb: seated alignment

`TMOPCinemaSeat` now treats its component location as the seat cushion:

- `Seated Character Origin Height` defaults to 88 cm.
- `Reverse Seated Facing` defaults to enabled and turns the character 180 degrees.
- `Seated Local Offset` and `Seated Rotation Offset` remain available for
  per-seat fine adjustment.
- Seating explicitly selects the `Sitting` animation posture.
- Standing restores automatic posture derivation.

For `SingleArmChair214`, start with the new defaults. If the pelvis is slightly
above or below the cushion, adjust only `Seated Character Origin Height` in
small increments (normally 2–5 cm).

The shared Animation Blueprint must contain a sitting state that is entered
when `bIsSeated` is true (or `Posture == Sitting`) and must have a sitting
animation assigned.

## Vehicle route to an anchor

A person's `BeginDriving` timeline entry now has:

- `Vehicle Route Mode`
  - `Manual Lane Route`
  - `Automatic To Anchor`
  - `Manual Then Automatic`
- `Destination Anchor ID`
- the existing `Ordered Lane IDs`

For Lars:

1. `Action`: `BeginDriving`
2. `Target Entity ID`: `VEHICLE_LARS_KNUBBS_BIL`
3. `Vehicle Route Mode`: `Automatic To Anchor`
4. `Destination Anchor ID`: `exitsveavagenN`
5. Leave `Ordered Lane IDs` empty.
6. Use relative timing with the appropriate film `Shared Event ID`.

The vehicle selects the nearest start lane, the nearest lane to the destination
anchor, and a deterministic shortest path through allowed `Next Lanes`.

For a route that must begin with historically controlled lanes, select
`Manual Then Automatic`, enter those lanes in `Ordered Lane IDs`, and use the
same destination anchor. The calculated route begins after the last manual
lane.

All lane splines must have unique `Lane ID` values and valid directed
`Next Lanes` connections. Automatic routing deliberately does not jump between
unconnected nearby lanes.

## Upgrade

Replace the complete existing `Plugins/TMOPEngine` folder with this version.
Regenerate project files and build `Development Editor | Win64`.

`InputCore` remains included in `TMOPEngineEditor.Build.cs`.
