# TMOP Engine 0.0.104

This version fixes precision-timeline and historical-vehicle parking behavior.

- Manual lane routes now honor `DrivingDestinationAnchorId` and perform a
  smooth final approach from the last lane instead of driving past the target.
- Timed `Stop` and `Park` entries no longer teleport a vehicle over a large
  distance. A missed target is stopped in place and reported for validation.
- Critical person entries may opt into `bSupersedeActiveMovementWhenDue`.
  This cancels an older delayed navigation request without teleporting the
  person, allowing independently reconstructed shot-window actions to run.

The accompanying 08_26 replacement tables enable the new precision flag only
for the Page31 reconstruction and correct the affected groups, witnesses,
drivers, Morelius route, and ambulance destination handling.
