# TMOP Engine 0.0.67

## Exit-anchor generator

Select one or more general `TMOPHistoricalAnchor` exit actors in the level,
then run:

`Tools > Generate Exit Anchors From Selection`

For a selected parent such as `ExitAdolfFredriksKyrkogataW`, the tool creates:

- `ExitAdolfFredriksKyrkogataW_Sidewalk1`
- `ExitAdolfFredriksKyrkogataW_Sidewalk2`
- `EnterAdolfFredriksKyrkogataW_Car`
- `ExitAdolfFredriksKyrkogataW_Car`

Every generated actor:

- starts at the parent transform,
- uses `Map Exit` as its category,
- stores the original anchor in `Parent Anchor ID`,
- is enabled for routing,
- is grouped below the parent in the World Outliner.

Existing matching IDs are skipped rather than duplicated. The complete action
supports Undo.

After generation, move the two sidewalk anchors to their respective NavMesh
surfaces and move the car anchors to the centres of the incoming and outgoing
traffic lanes.

Version 0.0.67 includes all fixes and features from version 0.0.66.
