# Dallas/Fort Worth västerut och söderut – v108 / batch105

Granskad 1 oktober 2026. Kumulativ datauppdatering från v107.

185 nya planerade Delta-rörelser på 24 riktade förbindelser till/från Dallas/Fort Worth: Las Vegas, Los Angeles, Ontario, Phoenix, San Diego, San Francisco, Seattle, Kansas City, Lubbock, New Orleans, Tampa och Tulsa. 89 nya granskade nonstop-scheman från 91 valda källrader; två rader börjar först 2 mars och hålls utanför. Totalt **7 130 rörelser**, varav 7 129 tidtabellslagda och en dokumenterat genomförd. Alla 6 945 äldre rörelseobjekt och 4 118 äldre scheman är oförändrade. 22 nya riktade ändpunktspar tillkommer; DFW–SFO i båda riktningarna fanns redan med Northwest. Separat UTC-kontroll stämmer med alla nya rörelser och 20 markuppehåll är kontrollerade. Se `research/DALLAS_WEST_SOUTH_BATCH105.md`.

Inga nya flygplatser eller bolag tillkommer. Samtliga 297 platsmarkörer och 96 operatörer bevaras. Paketet innehåller hela tidigare databasen.

## Sträckor

Samtliga tolv förbindelser är granskade i båda riktningarna.

| Dallas/Fort Worth till/från | Nya scheman | Nya rörelser |
|---|---:|---:|
| Las Vegas–McCarran | 6 | 12 |
| Los Angeles International | 8 | 17 |
| Ontario | 6 | 12 |
| Phoenix | 10 | 21 |
| San Diego | 4 | 9 |
| San Francisco International | 8 | 18 |
| Seattle | 5 | 9 |
| Kansas City International | 8 | 17 |
| Lubbock | 8 | 17 |
| New Orleans | 11 | 22 |
| Tampa | 6 | 13 |
| Tulsa | 9 | 18 |
| **Totalt** | **89** | **185** |

## Källa och avgränsning

Delta Air Lines, systemtidtabell från **1 februari 1986**, [Digital Library of Georgia](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201). Tryckta sidor **65–67, 118, 124, 135, 140, 170, 194, 213, 215, 223, 230 och 238** har granskats visuellt. Teckenförklaring på sida 4 och daterade fotnotsförklaringar på sidorna 65, 135 och 193 används. PDF-hash och sidmappning sparas i källunderlaget.

De 91 transkriberade raderna är samtliga explicit nonstop-rader inom **22 valda stadsriktningsrubriker**. Los Angeles/Ontario-rubrikerna innehåller två flygplatser och ger därför totalt 24 riktade flygplatspar. Anslutningsförslag och genomgående rader med stopp ingår inte i transkriptionsantalet. Exempelvis har DFW–SAN 487, SAN–DFW 423, LBB–DFW 1098 och TUL–DFW 1028 mellanlandningar och importeras inte som nonstop.

Två framtida rader, **DFW–LAX 589 (21:57–22:55)** och **LAX–DFW 500 (01:25–06:03)**, har fotnot 6: trafikstart **2 mars 1986**. De bevaras i råtranskriptionen med uteslutningsorsak men skapar inga aktiva scheman eller rörelser. **DFW–PHX 1019** och **PHX–DFW 1012** har fotnot 4: trafikstart **12 februari**, och ingår därför i importen.

Utgåvans slutdatum och senare ändringsblad är inte verifierade. Källan beskriver planerad trafik, inte belagt genomförande eller en faktisk flygbana. Original-PDF och sidbilder ingår inte i paketet.

## Tider och flygplatsidentitet

- `D` motsvarar tomt frekvensfält, dagligen; `X` undantar angivna dagar, 1=måndag till 7=söndag. Stjärnan är en lågtrafikprismarkering, ingen dygnsregel.
- **Seattle–DFW 832** går alla dagar utom lördag och **874** endast lördag, båda 12:20–17:47 lokal tid. Lördagens 874 avgår 20:20 UTC den 1 mars och anländer 23:47 UTC. Den överlappar fönstrets slut och ingår med hela sitt intervall.
- **DFW–New Orleans 852** avgår 23:21 och anländer 00:30 nästa lokala kalenderdag. Den separat källbelagda SFO–DFW-sträckan med samma nummer lämnar 46 minuters markuppehåll i DFW.
- DFW, Kansas City, Lubbock, New Orleans och Tulsa använder UTC−6; västkustflygplatserna och Las Vegas UTC−8; Phoenix UTC−7 och Tampa UTC−5 under importdatumen.
- Los Angeles-suffixet **L** avser LAX och **O** Ontario/ONT. Dallas avser DFW, inte Love Field. San Francisco avser SFO, Kansas City MCI och Las Vegas den befintliga historiska McCarran-markören.

Flygfönstret är **1986-02-27 22:21:30 UTC till 1986-03-01 22:21:30 UTC**. En rörelse ingår om avgången ligger före fönstrets slut och ankomsten efter dess början; hela tider bevaras. Lokala avgångsdatum 27 februari–1 mars prövas. Åtta scheman ger tre rörelser, 80 ger två och lördagsschemat SEA–DFW 874 ger en. Alla 89 accepterade scheman ger alltså minst en rörelse.

## Kontroller och bevarande

En separat manuellt inmatad transkription i 24-timmarsformat jämförs med råtidernas tolkning. En separat kalender- och UTC-beräkning med fasta vintertidsförskjutningar stämmer med samtliga **185 nya rörelser**. Inga tidsöverlapp hittades mellan nya och äldre rörelser med de berörda Delta-flygnumren.

**20 markuppehåll** mellan separat källbelagda delsträckor är kontrollerade. Exempel: Boston–DFW–San Diego 359 (41 minuter), Boston–DFW–Seattle 831 (49), Orlando–DFW–Los Angeles 57 (57), Tampa–DFW–San Francisco 89 (52), New Orleans–DFW–Las Vegas 793 (53) och Phoenix–DFW–New Orleans 510 (51). Alla kontrollerade ID:n och minuter finns i `validation_batch105.json`. Samma flygnummer och tidsmässig anslutning identifierar inte ett individuellt flygplan.

Alla 6 945 äldre rörelseobjekt, 4 118 äldre scheman, flygplatser, bolag och orelaterade källposter är oförändrade. COMAIR-rättelserna 1582/1580, rättelsen av uteslutna RIC–ATL 1125 och de tre tidigare hållna källkonflikterna bevaras. De befintliga 20 Python-testerna och aktualitetskontrollerna redovisas i `automated_checks_batch105.json`.

Unreal-kompilering, Editor och paketerat spel har inte körts här. Datauppdateringen förutsätter den redan kompilerade PaintAirplane-rättelsen från `f23b73a`.

## Revisionsunderlag och fortsatt arbete

- `dallas_west_south_batch105.tsv`: 91 nonstop-rader med källsida, fotnot och importbeslut.
- `independent_clock_transcription_batch105.txt`: separat klock- och veckodagsinmatning för de 89 accepterade raderna.
- `dallas_west_south_source_evidence_batch105.json`: källa, PDF-hash, sidnummer och tolkningsregler.
- `batch_105.json`: nya schema- och rörelse-ID:n samt antal per sträcka.
- `validation_batch105.json`, `automated_checks_batch105.json` och `package_preservation_batch105.json`: beräkning, tester och bevarande.
- `worldwide_coverage_batch105.json`: operatörsvis täckning.

Fortsätt med återstående DFW-rubriker, särskilt östliga och regionala förbindelser med returer, därefter nordöstra Delta/Ransome och fler separat tidsatta mellanliggande delsträckor. Nätet och världsinventeringen är fortfarande ofullständiga; någon säker global återstående mängd finns inte.
