# TMOP Engine 0.0.63

## Blueprint AnimationState conflict fixed

Versions 0.0.61 and 0.0.62 accidentally added a native
`ATMOPHistoricalAgent.AnimationState` property even though
`BP_TMOPHistoricalAgent` already owns an `AnimationState` component.

That duplicate caused:

`Internal Compiler Error: Tried to create a property AnimationState ... but another object already exists there.`

The duplicate native property and component have been removed. Cinema seats
still find the existing Blueprint `UTMOPAnimationStateComponent` by class and
set its posture to `Sitting`, so the seating fix remains active.

Version 0.0.63 includes the automatic vehicle routing, cinema-seat alignment,
and vehicle anchor placement from versions 0.0.61 and 0.0.62.
