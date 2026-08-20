# Vehicle anchor ground snap

1. Select one or more `TMOPHistoricalAnchor` actors intended for parked vehicles.
2. Open **Tools > TMOP > Snap Selected Vehicle Anchors To Ground**.
3. Save the level.

The command traces vertically to the first surface that blocks the Visibility
channel. It preserves the anchor's heading, aligns pitch and roll to the surface
normal, and places the vehicle actor origin 60 cm above the surface to match the
default `TMOPConfiguredVehicle` collision half-height.

The operation supports multiple selected anchors and Unreal Undo/Redo. If an
anchor is not moved, verify that the road mesh blocks the Visibility trace
channel and that a surface exists below the anchor.
