TMOP – SAMMANSLAGET KUMULATIVT PAKET L835–L841 + SENASTE BILAR

BAS
- Uppslags- och observationstabeller: kumulativt till och med L841.
- Fordonstabell: GitHub commit 3a0f602, "latest datatables of cars".
- GitHubs fem motsvarande JSON-tabeller kontrollerades och var byte-för-byte
  identiska med L841-paketet.

VIKTIGT OM FORDONSTABELLEN
Importera INTE en äldre DT_TMOP_HistoricalVehicles.json. Den skulle skriva
över de bilmodeller och fordonsinställningar som du lagt till i Unreal.
Detta paket använder i stället den senaste Unreal-filen:

COPY_TO_PROJECT/Content/TMOP/Vehicles/DT_TMOP_HistoricalVehicles.uasset

INSTALLATION
1. Stäng Unreal Editor.
2. Säkerhetskopiera projektets nuvarande
   Content/TMOP/Vehicles/DT_TMOP_HistoricalVehicles.uasset.
3. Kopiera innehållet i COPY_TO_PROJECT till projektroten. Mappstrukturen är
   redan korrekt. Om projektet redan är uppdaterat till commit 3a0f602 är
   fordonsfilen samma version och behöver inte kopieras igen.
4. Starta Unreal Editor.
5. Importera JSON-tabellerna från IMPORT_JSON i följande ordning:
   a. DT_TMOP_HistoricalEvents.json
   b. DT_TMOP_Groups.json
   c. DT_TMOP_People.json
   d. DT_TMOP_Observations.json
   e. DT_TMOP_Uppslag_REGISTER.json
6. Importera inte någon HistoricalVehicles-JSON efter detta.
7. Kör Reload Observation Data och Validate Observation Data.
8. Kör Validate på person-, grupp- och fordonsdirektorerna.
9. Kör om scenario-bake.

ANCHORS
Se ANCHORS_L835_L841_SV.txt och ANCHORS_L835_L841.json.

INNEHÅLL FRÅN L835–L841
- L835: arkiverad observerad man; inga nya anchors.
- L837: Per och hans bil på Tegnérgatan.
- L838: tre vittnen och en observerad man i salong 1.
- L839: två befintliga vittnen i salong 2, fullständiga tidslinjer.
- L841: tre vittnen, en observerad mörkklädd man och två observationer.
