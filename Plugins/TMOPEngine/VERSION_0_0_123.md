# TMOPEngine 0.0.123 – Palme shot sequence

Place one `TMOPPalmeShotDirector` actor in the persistent level and assign:

- `Olof Shot Animation`: the imported 0–160 frame, 30 fps Olof sequence.
- `Killer Shot Animation`: the matching killer sequence.
- `Bullet Light Trail Effect`: Niagara beam with world-vector parameters
  `User.Start` and `User.End`.
- `Muzzle Smoke Effect`, `Wall Impact Effect`, and `Snow Impact Effect`.
- Optional separate first/second shot sounds.

Required anchors:

- `Mordplatsen` – Olof's animation start.
- `Dekorimaingang` – killer spawn and animation start.
- `PALME_SHOT_WALL_IMPACT` – wall strike.
- `PALME_SHOT_RICOCHET_END` – visible continuation after the bounce.
- `PALME_SHOT_SNOW_IMPACT` – snow-bank strike.

Timing defaults are frame 51 (1.7 s), frame 138 (4.6 s), and frame 160
(5.333 s). `First Shot Uses Wall Route` swaps the two ballistic routes without
changing code.
