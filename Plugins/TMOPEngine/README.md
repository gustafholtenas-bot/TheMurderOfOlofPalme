# TMOP Engine v0.0.2

## Sprint 1: Time

- `FTMOPTime`
- `UTMOPClockSubsystem`
- Real-time default loop from 23:13:00 to 23:23:00
- Blueprint events for second changes and loop restarts

## Sprint 2: World

- `UTMOPWorldSubsystem`
- Central weak-reference registry for agents, vehicles, items and other runtime objects
- Stable object IDs and object types
- Baseline world state
- Runtime causal world state
- Automatic runtime reset whenever the time loop restarts
- Blueprint events for registration, removal, state changes and loop reset
- Debug report generation

## Install or upgrade

Copy the included `Plugins` folder into the root of the Unreal project and allow it
to replace the existing `Plugins/TMOPEngine` files.

Final path:

`TheMurderOfOlofPalme/Plugins/TMOPEngine/TMOPEngine.uplugin`

Then regenerate Visual Studio project files and build:

`Development Editor | Win64`

## World-state example

In Blueprint:

1. Get Game Instance Subsystem: `TMOPWorldSubsystem`
2. Call `Set Baseline World State`
3. State key: `Palme.Alive`
4. Value: `Make Boolean(true)`
5. During play call `Set World State` with `false`
6. When the loop restarts, runtime state returns to the baseline value `true`
