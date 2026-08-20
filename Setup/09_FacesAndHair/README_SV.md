# Standardansikten och 1986-hår

Etappen innehåller tre generiska huvudplatser, nio hårvarianter och sju
hårfärgsmaterial. Mesh-källorna fylls i i `SOURCE_MESHES` när kompatibla assets
har importerats.

Standardhuvudet bör stödja morph targets `TMOP_FaceWidth`, `TMOP_JawWidth`,
`TMOP_JawProjection`, `TMOP_CheekboneProminence`, `TMOP_NoseWidth`,
`TMOP_NoseLength`, `TMOP_BrowHeight`, `TMOP_EyeSpacing`, `TMOP_LipThickness`
och `TMOP_Age`.

Koden genererar stabil variation från Appearance Seed och översätter kända
svenska ansiktsbeskrivningar till tydliga morphvärden. Helt okända ansikten
varieras inte utan använder obscured-standardansiktet.

Kör scriptet och kör därefter `Setup/05_DataTableBuilder` igen. Byggaren hittar
den nya CSV-filen automatiskt och bygger totalt 52 systemhanterade rader.
