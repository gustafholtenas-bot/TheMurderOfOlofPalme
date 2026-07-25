# TMOP Engine 0.0.74

Historical vehicles now have camera-facing world-space name labels matching
historical people.

- The label uses `DisplayName` from `DT_TMOP_HistoricalVehicles`.
- If `DisplayName` is empty, it displays `VehicleId`.
- `bShowNameLabel`, height, world size and colour can be adjusted on the
  vehicle actor.
- Labels work for both director-spawned and reused placed vehicles.

This release also includes the per-vehicle body colour support from 0.0.73.
