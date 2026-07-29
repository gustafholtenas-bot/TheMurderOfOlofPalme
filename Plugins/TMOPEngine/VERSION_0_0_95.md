# TMOP Engine 0.0.95

## Person name labels

- Reduced the default overhead name size from 24 to 16.
- Moved names closer to the character, from 125 cm to 105 cm.
- Added automatic colors based on `DT_TMOP_People`:
  - standard people: white
  - Palme family (`EntityId` ending in `_PALME`): red
  - `SUSPECT`/`suspect`: green
  - `POLICE`/`POLIS`: blue
- Kept all colors and label dimensions editable on `ATMOPHistoricalAgent`.
