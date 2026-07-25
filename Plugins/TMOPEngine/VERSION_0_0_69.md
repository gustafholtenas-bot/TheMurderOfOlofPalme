# TMOP Engine 0.0.69

## Person timeline: Relative to Previous Entry

Person timeline entries now support three useful timing modes:

- `Absolute`
- `Relative to Shared Event`
- `Relative to Previous Entry`

`Relative to Previous Entry` resolves from the execution time of the timeline
array item immediately above it, then adds `Offset Seconds`.

Example:

- `StandUp`: Absolute 23:08:00
- `MoveToAnchor`: Previous +5 seconds = 23:08:05
- `EnterVehicle`: Previous +30 seconds = 23:08:35
- `BeginDriving`: Previous +1 second = 23:08:36

Relative entries can be chained without limit. Runtime catch-up and debug time
seeking consume the same resolved chain in array order.

The visual People Editor displays entries as `Previous +N s`. The first
timeline item cannot use this mode and is marked as an error if configured that
way.

Version 0.0.69 includes both anchor generators and all previous fixes and
features.
