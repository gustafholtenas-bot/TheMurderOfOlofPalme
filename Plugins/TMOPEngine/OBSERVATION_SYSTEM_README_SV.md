# Observationssystem 0.0.96

## DataTables

Skapa två DataTables:

1. `DT_TMOP_Observations`
   - Row Structure: `TMOP Observation Definition`
   - Importera `Data/DT_TMOP_Observations_TEST_IMPORT.json`
2. `DT_TMOP_ObservationLinks`
   - Row Structure: `TMOP Observation Link Definition`
   - Importera `Data/DT_TMOP_ObservationLinks_TEST_IMPORT.json`
`TMOP Observation Link Definition` innehåller både kopplingen, huvudrutten och
eventuella alternativa rutter. Det finns ingen separat inferred-track-tabell.

## Director

1. Placera `TMOPObservationDirector` i nivån.
2. Tilldela de två tabellerna i motsvarande fält.
3. Se till att `TMOPHistoricalEventDirector` har registrerat
   `Palme_shot_1`.
4. Starta simuleringen före testobservationens tidsfönster.

Directorn beräknar den kanoniska tiden från det redan existerande shared
eventet. Den flyttar, väntar in eller teleporterar aldrig observatören eller den
observerade aktören.

## Observerade okända personer

Testobservationerna refererar till vanliga `HistoricalAgent`-rader i
`DT_TMOP_People`. Kör:

```text
python tmop_mark_observed_unknown_096.py <senaste People JSON> <ny People JSON>
```

och importera den nya People-filen. Skriptet ändrar bara de sex angivna
testpersonernas `CategoryId` till `OBSERVED_UNKNOWN`. Dessa agenter behåller sina
egna signalement och timelines men visar ingen overhead-namntext i vanlig
simulering.

`THOMAS_PILTZ_MED_WALKIE_TALKIE` behandlas därmed som en anonym
observationsfigur. Personradens Notes dokumenterar att Mauno Luukas senare
pekade ut mannen som ”polisman D”, vilken i Granskningskommissionens redovisning
anges vara Thomas Piltz. Utpekandet sparas som källuppgift och inte som
fastställd identitet för observationen.

En färdig version baserad på den senaste People-exporten ligger i:

`Data/DT_TMOP_People_OBSERVED_UNKNOWN_096_IMPORT.json`

Passaten är på samma sätt en vanlig rad i `DT_TMOP_HistoricalVehicles`, med en
egen timeline. En färdig fordonsfil finns i:

`Data/DT_TMOP_HistoricalVehicles_OBSERVED_UNKNOWN_096_IMPORT.json`

Den ändrar endast Passatens kategori till `OBSERVED_UNKNOWN`. Observerade okända
fordon visar inte heller någon overhead-namntext.

## Runtime-resultat

`Get All Observation Runtime` visar:

- `Pending`: väntar på kanonisk tid eller observationsfönster
- `Observed`: minst en observatör, observerad aktör, avstånd och eventuell
  fri sikt stämde
- `Missed`: observationsfönstret stängdes utan giltig geometri
- `Invalid`: observationen är avstängd

Blueprint-eventet `On Observation Evaluated` skickas när observationen blir
`Observed` eller `Missed`.

## Mauno Luukas och Kicki J

Importera även:

`Data/DT_TMOP_Groups_MAUNO_KICKI_096_IMPORT.json`

People- och observationsfilerna innehåller därefter:

- `GROUP_MAUNO_LUUKAS_KICKI_J`, med Mauno som gruppledare och Kicki som följare.
- WT-mannen vid Adolf Fredriks kyrkogata 13 som `OBSERVED_UNKNOWN`.
- En separat okänd springande man i porten 5-7 som `OBSERVED_UNKNOWN`.
- Två oberoende observationsrader med Mauno och Kicki som observatörer.

Anchorn `Adolffredrikskyrkog_13` måste finnas i nivån med exakt detta ID.
Portobservationen använder den befintliga `Adolffredrikskyrkog_5`.
