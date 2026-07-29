# TMOP Engine 0.0.98 — World Bake

This cumulative release replaces the limited Person Bake with a deterministic
World Bake for 23:00–23:45 time seeking.

Captured:

- resolved Shared Event times;
- active people, transforms, velocity, life/activity and movement targets;
- group membership, leader, formation, conversation and movement state;
- historical vehicles, lane progress, speed, routes and seat occupants;
- observation runtime state;
- every loaded light component's visibility, intensity and colour.

Derived schedule systems (timed props/findings, aircraft and scheduled lights)
are rebuilt after baked Shared Events have been applied.

The source signature covers the level, source DataTables, Shared Events,
anchors, lanes and configured scheduled systems. A stale bake automatically
falls back to the authored timeline catch-up.

Editor actions on `TMOPSimulationDebugDirector`:

- Bake Entire Simulation
- Cancel Bake
- Clear World Bake
- Validate World Bake
- Load World Bake

Bake writes use a temporary file and atomically replace the previous valid bake
only after serialization succeeds.
