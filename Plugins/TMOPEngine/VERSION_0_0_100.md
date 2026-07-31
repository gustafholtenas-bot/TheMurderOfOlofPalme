# TMOPEngine 0.0.100 – Vehicle wheel crash guard

This cumulative release prevents editor construction/reinstancing from
crashing in `ATMOPConfiguredVehicle::ApplyWheel`.

- Validates the configured model and required vehicle components.
- Validates every wheel component before applying its mesh or transform.
- Keeps a stable local model pointer during configuration and animation.
- Skips invalid wheels during animation.
- Logs a warning instead of dereferencing an invalid component.

The fix was prompted by a crash while loading a Historical Vehicles DataTable
containing a newly spawn-enabled configured vehicle.
