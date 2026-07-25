# TMOP Engine 0.0.72

This release uses the runtime group timeline actions introduced in 0.0.71.

## Palme family groups

- `GROUP_OLOF_LISBET_PALME`
  - Leader: `OLOF_PALME`
  - Member: `LISBET_PALME`
  - Created at scenario start.
  - Dissolved at shared event `Palme_shot_1`.
- `GROUP_MARTEN_PALME_INGRID_KLERING`
  - Leader: `MARTEN_PALME`
  - Member: `INGRID_KLERING`
  - Created at scenario start and remains together through its current timeline.

At `Palme_shot_1`, Olof's timeline first executes `DissolveGroup`, clearing
runtime membership for both Olof and Lisbet. It then executes
`ChangeLifeState = Dead`. Lisbet's later entries consequently run independently.

Import both supplied JSON files after installing and rebuilding the plugin.
