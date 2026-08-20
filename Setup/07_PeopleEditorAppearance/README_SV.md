# Appearance-stöd i TMOP People Editor

People Editor har nu två nya knappar i verktygsraden.

## Generate Appearance

Läser den valda personens aktuella editorfält och visar det deterministiskt
lösta resultatet i statusraden:

- grundkropp;
- ansikte;
- ytterplagg;
- överdel;
- byxor;
- skor;
- antal delar som är okända i källmaterialet.

Om ett känt signalement saknar en matchande katalogasset visas en orange
varning. Okända fält som avsiktligt använder obscured-fallback räknas och visas,
men behandlas inte som fel.

## Validate All Appearances

Löser samtliga personer i `DT_TMOP_People` och visar:

- antal kontrollerade personer;
- totalt antal varningar;
- hur många personer som påverkas;
- totalt antal källmässigt okända delar.

Varje detaljerad varning skrivs till Unreal Output Log tillsammans med
personens Row Name. Det gör att vi kan hitta signalement som saknar motsvarande
rock, mössa, färg eller annan katalogasset innan en packaged build skapas.

Knapparna laddar om Appearance DataTable automatiskt om tabellen skapades efter
att People Editor öppnades.
