# TMOP Engine 0.0.103

Unified corrective build containing the venue-layout anchors, newspaper reader,
vehicle-anchor ground-snap editor tool, and the latest transit/player updates.

Fixes:

- Renames the venue JSON helper to `ReadVenueLayoutNumber` to prevent Unreal
  Unity Build collisions with another translation unit's `ReadNumber` helper.
- Ships the matching `TMOPItemDefinition.h` and `TMOPPlayerCharacter.h` from
  the same source snapshot, preventing header mismatches caused by installing
  the earlier feature packages on top of one another.
