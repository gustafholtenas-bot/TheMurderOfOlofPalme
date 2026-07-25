# TMOP Engine 0.0.73

## Historical vehicle paint colour

`FTMOPHistoricalVehicleRow` now exposes:

- `Override Body Color`
- `Body Color`

The historical vehicle director copies these fields to both automatically
spawned and reused placed `ATMOPConfiguredVehicle` actors.

Each `UTMOPVehicleModelData` also exposes `BodyColorParameterName`, which
defaults to `VehicleColor`. `BodyMaterialSlotIndex` selects which material slot
receives the dynamic material.

The body material must contain a Vector Parameter with the same name. Connect
that parameter to Base Color, or multiply it with the body texture before Base
Color. Glass, wheels and livery slots remain unchanged.
