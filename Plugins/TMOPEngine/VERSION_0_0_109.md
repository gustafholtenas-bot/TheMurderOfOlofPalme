# TMOPEngine 0.0.109

## Emergency vehicle siren movement gating

- `bEmergencySirenEnabled` remains the emergency-response state and therefore
  keeps police and ambulance blue lights flashing while parked.
- The audible siren loop now starts only after the vehicle is moving faster
  than `SirenStartSpeedCmPerSecond` (default 25 cm/s).
- The audible loop fades out when speed falls below
  `SirenStopSpeedCmPerSecond` (default 8 cm/s).
- Separate start/stop thresholds prevent siren chatter caused by tiny vehicle
  position changes while stopped.

The two speed thresholds can be adjusted per vehicle audio component in the
editor under `TMOP | Audio | Vehicle | Siren`.
