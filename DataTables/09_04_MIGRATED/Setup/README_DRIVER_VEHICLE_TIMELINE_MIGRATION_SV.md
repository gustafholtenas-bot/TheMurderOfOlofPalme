# Migrering från förarens timeline till bilens timeline

Detta paket flyttar äldre `BeginDriving`-rader från `DT_TMOP_People` till
respektive bil i `DT_TMOP_HistoricalVehicles`. Migreringen är additiv och
skriver aldrig över en befintlig bilkörning.

## Resultat för tabellerna i 09_04

- 39 saknade eller bristfälliga bilkörningar kompletterades från personerna.
- 25 kompletta körningar som redan fanns i biltabellen fick bilen som ägare;
  deras befintliga rutter och tider ändrades inte.
- Totalt togs 64 lyckat konverterade `BeginDriving`-rader bort från personerna.
- 2 körningar tillhörde nyligen redigerade, skyddade bilar och lämnades kvar.
- 1 äldre körning saknar både lanes och destination och kunde inte migreras
  säkert. Den ligger kvar hos föraren och finns i rapporten.
- 0 rader skrevs över.

Följande nio bilar skyddades eftersom deras route-/timelinedata skiljer sig
från basexporten 09_01:

- `AMBULANCE_912`
- `AMBULANCE_A951`
- `VEHICLE_AKE_LARSSONS_BIL_FORD_ESCORT_GRAMETALLIC`
- `VEHICLE_ANDERS_DELBOMS_TAXI_BIL_JARFALLA_TAXI_MITSUBISHI_GALANT`
- `VEHICLE_ANNETTE_KOHUTS_BIL`
- `VEHICLE_BENGT_PALM_MERCEDES_GUL`
- `VEHICLE_EAE501_00_WOMENS_MASKED_CAR`
- `VEHICLE_EAE9773_05_WHITE_MERCEDES_190`
- `VEHICLE_INTRO_JAN_NILSSON_LIMO`

## Installera

1. Stäng Unreal Editor.
2. Kopiera paketets `Plugins` och `Setup` till projektet.
3. Bygg `TheMurderOfOlofPalmeEditor` innan tabellerna importeras. De migrerade
   raderna använder nya structfält i C++.
4. Säkerhetskopiera de två befintliga Data Table-assetsen.
5. Importera
   `DT_TMOP_HistoricalVehicles_MIGRATED.json` till den befintliga
   `DT_TMOP_HistoricalVehicles`.
6. Packa upp `DT_TMOP_People_MIGRATED.zip` och importera JSON-filen till den
   befintliga `DT_TMOP_People`.
7. Importera alltid båda tabellerna från samma paket. Då tas endast de 64
   lyckat kopierade `BeginDriving`-raderna bort från personerna.
8. Kör Data Table Validation och provspela minst en migrerad bil.

## Runtimebeteende

`Auto Start From Vehicle Timeline` är bara aktiverat på de nykonverterade
raderna. Gamla och manuellt redigerade bilrader fortsätter därför fungera som
tidigare. Vid rätt avgångstid väntar bilen tills den angivna föraren sitter i
förarsätet, och startar sedan exakt den aktuella bilraden. Den väntar inte på
alla personer i bilens generella `AssociatedPersonEntityIds`, eftersom det
inte var ett krav i det äldre förarstyrda systemet.

## Ej automatiskt lösbar rad

`UNKNOWN_DRIVER_GLANTZ_TAXI_E9979_BEGIN_DRIVING` för
`VEHICLE_GLANTZ_BASEN_TAXI_E9979` saknar både `OrderedLaneIds` och
`DrivingDestinationAnchorId`. Ange en korrekt lane-rutt eller destination i
editorn innan den flyttas till bilens timeline.

Detaljer för varje migrerad, hoppad och olöst rad finns i
`TMOP_VehicleTimelineMigration_Report.json`.

## Köra verktyget igen

```powershell
python Setup/tmop_migrate_driver_vehicle_timelines.py `
  --people DataTables/09_04/DT_TMOP_People.zip `
  --vehicles DataTables/09_04/DT_TMOP_HistoricalVehicles.json `
  --baseline-vehicles DataTables/09_01/DT_TMOP_HistoricalVehicles.json `
  --output-dir VehicleTimelineMigration
```

Lägg till `--protect VEHICLE_ID` för varje ytterligare bil som aldrig får
ändras.
