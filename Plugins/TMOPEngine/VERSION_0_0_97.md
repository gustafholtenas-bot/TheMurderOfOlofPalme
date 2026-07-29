# TMOP Engine 0.0.97

Runtime validation and diagnostics update:

- accepts archive-only historical vehicle rows without timelines when
  `bSpawnInSimulation` is false;
- preserves strict timeline validation for simulated vehicles;
- treats groups waiting for later-spawning agents as expected runtime state;
- reports actionable aerial spawn failure causes;
- includes both object paths in duplicate world-object and anchor diagnostics.

This release is cumulative with 0.0.96 and retains the observation system.
