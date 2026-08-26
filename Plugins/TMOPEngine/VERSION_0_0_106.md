# TMOPEngine 0.0.106 – shared-event vehicle timelines

Historical vehicle timeline entries now support:

- `TimingMode`
- `SharedEventId`
- `EventOffsetSeconds`

Spawn, driving, stop, park, siren and despawn evaluation resolves these fields
through `UTMOPHistoricalEventSubsystem`. Police cars and ambulances can
therefore follow the same canonical arrival event as their occupants when the
time of `Palme_shot_1` changes.
