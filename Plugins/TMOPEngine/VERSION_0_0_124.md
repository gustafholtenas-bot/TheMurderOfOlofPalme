# TMOPEngine 0.0.124

## Palme shot additions

- Optional blood-pool static mesh appears when the synchronized 160-frame shot animation finishes.
- Blood placement can follow Olof or an optional dedicated anchor and is adjustable with a local transform.
- Optional proximity slow-motion experiment evaluates the player four simulation seconds before the first shot.
- The effect activates only within the configured 70 metre radius and ends six simulation seconds after shot one.
- The prior simulation time scale is restored after the sequence and on loop reset or actor shutdown.

The slow-motion mode is disabled by default and can be enabled on `TMOPPalmeShotDirector`.
