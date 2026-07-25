# TMOP Engine 0.0.66

## Safe cinema-seat stand-up placement

Standing up from a cinema seat now:

1. Treats the approach transform as the character's foot position.
2. Projects that point onto the nearest NavMesh surface.
3. Adds the character capsule half height.
4. Places the character with its capsule above the floor.

New per-seat settings:

- `Approach Is Foot Location` (default: enabled)
- `Project Approach To NavMesh` (default: enabled)
- `Approach Nav Projection Extent` (default: 150, 150, 300 cm)

For `SingleArmChair214`, keep `Approach Vertical Offset` at `-45` initially.
Move the seat component's forward direction toward the aisle. Increase
`Approach Distance` if the projected point is still too close to the chair.

If projection fails, the Output Log names the affected Seat ID.

Version 0.0.66 includes all fixes and features from version 0.0.65.
