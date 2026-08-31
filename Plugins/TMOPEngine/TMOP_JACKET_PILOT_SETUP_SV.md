# TMOP jackpilot

Målet med pilotläget är att verifiera en enda modular jacka på både vanliga
historiska NPC:er och `OBSERVED_UNKNOWN` innan hela Appearance-systemet slås på.

## Asset som saknas

`DT_TMOP_AppearanceAssets` refererar redan till:

`/Game/TMOP/Characters/Appearance/Clothing/1986/Meshes/SK_TMOP_Jacket_Standard`

Själva Skeletal Mesh-asseten finns däremot inte i den pushade versionen. En
Static Mesh från Fab kan inte användas direkt eftersom ärmar och tyg då inte
följer animationerna.

## Första fungerande test

1. Rigga en jacka till samma skeleton och bind pose som Manny/Quinn.
2. Importera den som Skeletal Mesh med namnet `SK_TMOP_Jacket_Standard` i
   `/Game/TMOP/Characters/Appearance/Clothing/1986/Meshes/`.
3. Öppna `DT_TMOP_AppearanceAssets` och kontrollera att raden
   `UNKNOWN_OUTERWEAR_OBSCURED` använder denna mesh.
4. Kontrollera att levelns `TMOPPersonRegistryDirector` har
   `DT_TMOP_AppearanceAssets` i fältet `Appearance Asset Table`.
5. Kör `Validate Appearance Asset Table` på directorn.
6. Starta simuleringen. `CharacterAppearance` har
   `Outerwear Only Pilot Mode` aktiverat som standard och applicerar därför
   jackan samt den tekniska könsbaserade kroppen. Kvinnliga profiler använder
   Quinn, manliga profiler använder Manny och okänt kön behåller befintlig
   kropp. Ansikte, hår och övriga kläddelar lämnas orörda.
   `Force Outerwear On Everyone In Pilot Mode` är också aktivt, så testjackan
   visas även på profiler med `None` eller `Hidden`. Detta ändrar inte
   `DT_TMOP_People` och kan stängas av efter det tekniska testet.

När jackan sitter och animeras korrekt kan `Outerwear Only Pilot Mode` stängas
av för att aktivera hela modular-utseendet.

## Manny/Quinn

`CharacterAppearance` har `Automatically Select Manny Or Quinn By Gender`
aktiverat som standard. Standardvägarna är:

- `/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple`
- `/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple`

Om projektets assets ligger på andra sökvägar väljs de en gång i komponentens
fält `Male Base Body Mesh` och `Female Base Body Mesh`. MetaHuman-profiler och
profiler med okänt/ej angivet kön skrivs inte över.
