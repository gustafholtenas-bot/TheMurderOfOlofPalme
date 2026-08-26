# TMOPEngine 0.0.110

## Player vehicle handling and audio

- Normal player-driving top speed is now 45 km/h.
- Holding the regular player sprint input enables the previous 110 km/h top speed.
- Steering angle and body yaw rate are smoothed separately for a heavier turn-in.
- Ground height and pitch/roll alignment are interpolated.
- Low kerbs are retried using a vertical lift followed by a forward sweep.
- Vehicle camera supports mouse orbit, position lag and rotation lag.
- Camera orientation is updated every frame only for the player's active vehicle.
- Player takeover entry and exit now trigger the vehicle door audio cycle.
- Low/high collision audio hooks are selected by impact speed (25 km/h default split).

## Existing audio support audit

The vehicle audio profile already supports idle/driving engine loops, normal and
hard starts, skid, horn, door open/close, tyre burst and emergency siren. The
JSON sound library currently contains empty `Sound` references, so actual sound
assets must still be assigned in the project's DataTable/editor.

For collision audio, add sound-library rows/assets for:

- `VEHICLE_COLLISION_LOW`
- `VEHICLE_COLLISION_HIGH`
