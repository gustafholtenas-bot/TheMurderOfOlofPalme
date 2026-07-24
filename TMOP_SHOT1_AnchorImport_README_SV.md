# TMOP – import av vittnespositioner vid första skottet

## Installation

1. Stäng Unreal Editor och Visual Studio.
2. Packa upp zip-filen i projektroten, bredvid
   `TheMurderOfOlofPalme.uproject`.
3. Radera projektets `Binaries` och `Intermediate`.
4. Bygg `TheMurderOfOlofPalmeEditor`, `Development Editor`, `Win64`.

JSON-filen installeras som:

`Content/TMOP/Data/TMOP_SHOT1_WITNESS_ANCHORS.json`

## Skapa anchors i leveln

1. Öppna `Lvl_ThirdPerson`.
2. Placera en `TMOPShot1AnchorImporter` i leveln.
3. Kontrollera att `Json File Path` är:
   `TMOP/Data/TMOP_SHOT1_WITNESS_ANCHORS.json`
4. Kontrollera att `Anchor Class` är `TMOPHistoricalAnchor` eller din
   Blueprint-subklass av den.
5. Tryck `Import Or Update Shot1 Anchors` i Details-panelen.
6. Spara leveln.

Importen ska rapportera:

`36 created, 0 updated, 0 errors`

Alla skapade aktörer läggs i World Outliner-mappen:

`TMOP/Anchors/Shot1Witnesses`

Varje aktör får sitt exakta `ANCHOR_SHOT1_...`-ID, kategorin
`WitnessPosition`, källtid `1986-02-28 23:21:30` och position konverterad från
Blender till Unreal-centimeter.

## Omimport

Kör samma knapp igen efter en ny JSON-export. Importern hittar befintliga
aktörer genom deras Anchor ID och uppdaterar dem. Den skapar därför inte
dubletter. Ett normalt andra körresultat är:

`0 created, 36 updated, 0 errors`

`Use Exact Positions` bör vara aktiverad. Då flyttas inte vittnespositionerna
till NavMesh och ingen placeringsradie läggs till.
