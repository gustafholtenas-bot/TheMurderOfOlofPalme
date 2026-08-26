# TMOPEngine 0.0.111

## Forced floating newspaper layout

- Added `bForceFloatingWaistLayout`, enabled by default.
- Stale Blueprint values can no longer attach the newspaper to `hand_rSocket`
  or show the optional reading-arms mesh while the forced layout is active.
- The newspaper is attached to the dedicated reading camera and positioned by
  `NewspaperRelativeTransform` in front of the player's waist.
- Added bounds-based mesh centring so folded/open meshes with an edge pivot are
  visually centred instead of appearing to the right.
- Bounds centring is retained while changing page, panning and zooming.
