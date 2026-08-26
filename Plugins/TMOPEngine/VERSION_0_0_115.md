# TMOP Engine 0.0.115

## World-positioned target HUD

- Projects the selected target's world-space midpoint into the viewport.
- The reticle, name, type, distance and interaction prompt now follow the
  focused person, vehicle or item instead of remaining at screen centre.
- Person markers use an adjustable body-height fraction suitable for seated
  occupants inside vehicles.
- Screen-edge clamping keeps target information readable near the viewport edge.
- Falls back safely to the screen centre if projection is temporarily unavailable.
