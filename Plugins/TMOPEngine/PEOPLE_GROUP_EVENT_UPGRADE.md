# People, groups, shared events and anchor areas

This upgrade connects `DT_TMOP_People` to the already existing TMOP group and
historical-event systems. It does not introduce parallel directors.

## Required level actors

- one `TMOPPersonRegistryDirector`
- one `TMOPGroupDirector`
- one `TMOPHistoricalEventDirector`

The person director builds groups automatically from the people table when
`Create Groups From People Table` is enabled.

## Person group fields

Every member uses the same `SocialGroupId`, leader ID, formation and spacing.
Only the leader needs movement entries. Followers may contain only their
initial placement while `Follow Group Leader Schedule` is enabled.

Example groups:

- `GROUP_SUSANNE_ULRIKA`, leader `susanne_karlsson`
- `GROUP_GUNILLA_COMPANIONS`, leader `gunilla_dahlen`

## Shared event timing

On a timeline entry set:

- `Timing Mode = Relative`
- `Shared Event Id` to the ID registered by the historical event director
- `Event Offset Seconds` as required

For a `MoveToAnchor` entry, enable `Time Is Arrival`. The person director uses
the current NavMesh path length and movement profile to derive the departure
time. `Travel Speed Override` can replace the profile speed for one journey.

Suggested meeting event ID:

`SUSANNE_GUNILLA_KIOSK_MEETING_START`

## Anchor areas

`Placement Radius Cm = 0` preserves the old exact-point behaviour. A positive
radius chooses a deterministic point per entity or group. Enable projection to
NavMesh for outdoor gathering areas. The same stable entity/group always gets
the same point after restarting the simulation.
