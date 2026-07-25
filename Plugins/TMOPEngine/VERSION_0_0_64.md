# TMOP Engine 0.0.64

## Relative debug-time shortcuts

The normal number keys retain their existing absolute shortcuts:

- `1` = 23:00
- `2` = 23:05
- `3` = 23:10
- `4` = 23:15
- `5` = 23:20
- `6` = 23:25
- `7` = 23:30
- `8` = 23:35
- `9` = 23:40

The Shift shortcuts are now relative to the current simulation time:

- `Shift+1` = 30 seconds backwards
- `Shift+2` = 30 seconds forwards

They can be pressed repeatedly. A jump outside the configured scenario window
is rejected safely and reported in the Output Log.

Version 0.0.64 includes all fixes and features from version 0.0.63.
