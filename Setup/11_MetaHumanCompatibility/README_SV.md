# MetaHuman-kompatibilitet

TMOP:s MetaHuman-läge kräver inte att `TMOPEngine` länkar mot
MetaHuman-pluginen. Kopplingen görs i en Blueprint per specialbyggd person.

## Skapa en MetaHuman-person

1. Skapa en Blueprint som ärver från `ATMOPHistoricalAgent`.
2. Placera personens MetaHuman-kropp på den ärvda `BodyMesh`-komponenten.
3. Placera MetaHuman-huvudet på den ärvda `FaceMesh`-komponenten. Ytterligare
   skeletal mesh-komponenter får finnas kvar i Blueprinten.
4. Behåll MetaHuman-grooms och `LODSync` i Blueprinten. TMOP ändrar inte groom-
   assets och har därför inget direkt pluginberoende.
5. Sätt `AppearanceProfile.GenerationMode` till `MetaHuman` på personraden.
6. Sätt radens `AgentClass` till personens nya Blueprint.
7. Lägg källstyrda jackor, byxor, skor, huvudbonader, glasögon och halsdukar i
   signalementsfälten som vanligt.

## Vad runtime-systemet gör

- Bevarar befintlig kropp, ansikte, hår och ansiktsbehåring.
- Hoppar över TMOP:s automatiska ansikts- och kroppsmorphs.
- Bevarar kroppens Blueprint-placering som standard.
- Monterar fortfarande modulära 1986-kläder och accessoarer.
- Beräknar kapsel, ögonhöjd, namnskylt, pratbubbla och rörelsehastighet.
- Tillämpar cull distance och animation update-rate på alla skeletal meshes,
  även extra MetaHuman-komponenter.

## Viktigt

MetaHumanens kropp och modulkläderna måste använda kompatibla skeletons. Om en
klädmesh har ett annat skeleton döljs den och en diagnostikrad skapas. Groom-
LOD och MetaHumans `LODSync` konfigureras i personens Blueprint, eftersom dessa
delar ägs av MetaHuman-systemet och inte av TMOP:s plugin-oberoende kod.

Knappen `Validate All Appearances` varnar om en person står i MetaHuman-läge
utan en tilldelad `AgentClass`.
