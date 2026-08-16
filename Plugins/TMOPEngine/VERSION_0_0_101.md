# TMOP Engine 0.0.101 — full-day documentation timeline

Person timelines can now document the whole of 1986-02-28 without changing the
standard 23:00 simulation epoch.

## Timeline usage

- `Simulation`: existing behaviour; the entry affects the running simulation.
- `DocumentationOnly`: retained for chronology, source work and alibis, but
  never spawns or moves a person.
- `DocumentationAndSimulation`: visible as documentation and executable at
  runtime when its required world references exist.

Existing imported data remains backwards compatible because an omitted `Usage`
field defaults to `Simulation`.

Documentation entries may appear before the first simulation entry. The first
runtime-enabled entry—not necessarily `Timeline[0]`—must be `InitialPlacement`
or `Spawn`.

## Planned anchors

Set `AnchorReferenceMode` to `PlannedFuture` when a timeline needs a canonical
location ID that has not been built in Unreal yet. Keep the canonical ID in
`TargetAnchorId`, and optionally fill in `PlannedAnchorDisplayName` and
`PlannedAnchorNotes`.

Initial reserved IDs:

| TargetAnchorId | PlannedAnchorDisplayName | Purpose |
|---|---|---|
| `POLISHUSET` | Polishuset | Off-map police location and later world anchor. |
| `LARMCENTRALEN` | Larmcentralen | Off-map alarm/emergency-control location and later world anchor. |

A missing planned anchor never blocks the rest of a person's runtime timeline.
If an Unreal historical anchor with exactly the same ID is later added, entries
marked `Simulation` or `DocumentationAndSimulation` can execute automatically.
Entries marked `DocumentationOnly` remain non-physical even after the anchor is
created.

## Example JSON entry

```json
{
  "EntryId": "PERSON_06_LOCATION",
  "Action": "InitialPlacement",
  "Usage": "DocumentationOnly",
  "Time": { "Hour": 6, "Minute": 0, "Second": 0 },
  "TimingMode": "Absolute",
  "LocationType": "Anchor",
  "TargetAnchorId": "POLISHUSET",
  "AnchorReferenceMode": "PlannedFuture",
  "PlannedAnchorDisplayName": "Polishuset",
  "PlannedAnchorNotes": "Dokumentationspunkt utanför nuvarande spelområde.",
  "Confidence": "Documented",
  "SourceReference": "Fyll i källa",
  "Notes": "Fyll i vad uppgiften styrker och eventuell tidsosäkerhet."
}
```
