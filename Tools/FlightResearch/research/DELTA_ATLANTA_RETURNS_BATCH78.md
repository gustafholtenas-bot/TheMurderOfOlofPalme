# Atlanta-returer och Dayton – v81 / forskningsomgång 78

50 nya planerade Delta-rörelser från 23 nya nonstop-scheman. Databasen innehåller nu 5 001 rörelser: 5 000 enligt tidtabell och en tidigare dokumenterat genomförd rörelse. Detta är ett kumulativt datapaket för den befintliga flygmenyn.

| Riktning | Nya rörelser |
|---|---:|
| Atlanta → Cleveland | 11 |
| Atlanta → Columbus / Port Columbus | 11 |
| Atlanta → Indianapolis | 9 |
| Atlanta → Dayton | 8 |
| Dayton → Atlanta | 11 |
| **Totalt** | **50** |

De tre första riktningarna kompletterar v80:s separat granskade avgångar till Atlanta. Inga returer härleds från utgående flyg.

## Originalkälla

Delta Air Lines System Timetable, effective February 1, 1986. Delta Flight Museum / Digital Library of Georgia.

- Arkivpost: https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201
- PDF: https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf
- Avgångssidor: tryckta s. 14, 15 och 67 (PDF-sidor 10 och 36).
- Trafikdagar och symboler: tryckt s. 4.
- Kontroll av flygnummer och linjer: tryckta s. 262–263 (PDF-sida 134).

Samtliga 31 rader i fem valda riktningstabeller granskades visuellt. 23 nonstop-rader godtas. Fem anslutningsförslag och tre genomgående rader med ett stopp importeras inte som direkta flygsträckor. Accepterade rader har tomt frekvensfält, vilket betyder dagligen, och saknar daterad fotnot.

Originalets 12-timmarsnoteringar med a/p finns uttryckligen kvar i den nya granskningsfilen. Tiderna har även registrerats separat i 24-timmarsformat för kalender- och UTC-kontrollen. Originalskanningen omdistribueras inte.

## Nattflyg och intervall

Fönstret är 1986-02-27 22:21:30 UTC till 1986-03-01 22:21:30 UTC. Alla ändpunkter i detta urval använder UTC−5 under perioden. Flyg tas med när de överlappar fönstret, och deras fullständiga tider bevaras.

- Delta 906 Atlanta–Cleveland: 23:10–00:25 nästa lokala dag.
- Delta 560 Atlanta–Columbus: 23:03–00:15 nästa lokala dag.
- Den genomgående Atlanta–Dayton-raden för 560 har ett stopp. Den används inte som direktsträcka. De separat tryckta benen Atlanta–Columbus och Columbus–Dayton ger 25 minuters markuppehåll.
- Delta 336 Atlanta–Indianapolis: 20:24–21:40. Den separat tryckta Indianapolis–Fort Wayne-raden börjar 22:00, med 20 minuters markuppehåll.
- Delsträckor förblir separata poster. Samma flygnummer är inte bevis för ett bestämt individflygplan.

Nya avgångar per lokal kalenderdag: 27 februari 12, 28 februari 23, 1 mars 15.

## Rättelser av v80

Förstorad läsning av originalraderna visade tre transkriptionsfel i v80:

| Sträcka / flyg | Tid i v80 | Rätt lokal tid | Tryckt sida |
|---|---|---|---:|
| Cleveland–Columbus, COMAIR 1724 | 11:40–12:22 | **11:40–12:20** | 52 |
| Columbus–Dayton, Delta 560 | 10:20–10:58 | **00:40–01:05** | 62 |
| Indianapolis–Fort Wayne, Delta 336 | 10:00–10:28 | **22:00–22:28** | 111 |

Originalet anger 12:40a–1:05a för 560 och 10:00p–10:28p för 336. Dessa korrigeringar bygger på bilderna, inte på antagna flygtider.

Rättelsen av 336 ändrar vilka lokala avgångsdatum som ryms i fönstret: kvällsavgångarna 27 och 28 februari ingår, medan 1 mars 22:00 ligger efter fönstrets slut. Den felaktigt inkluderade 1-mars-posten ersätts därför av 27-februari-posten. Fyra kvarvarande rörelseposter får korrigerade tider/anmärkningar. Nettot av rättelserna är noll rörelser.

4 946 tidigare rörelseposter är helt oförändrade. De enda avvikelserna från v80 är de tre uttryckligen redovisade schemarättelserna och de 50 nya rörelserna. Den tidigare rättelsen av Delta 561 Detroit–Cincinnati från v80 finns kvar.

Tekniska ID:n för scheman behålls för spårbarhet, även när de innehåller en äldre felaktig klockslagssträng. De aktuella `departure`, `arrival`, `departure_local` och `arrival_local` är de gällande tiderna. `errata_batch78.json` innehåller fullständiga före-/efterposter, den borttagna posten och det korrigerade avgångsdatumet. Historiska granskningsfiler bevaras som versionshistorik.

## Täckning och fortsatt arbete

241 flygplatser/platser, 1 026 riktade platspar. Delta har 1 019 rörelser. Fortfarande 95 registrerade operatörer, 49 med rörelser och 46 utan. Inget bolag är verifierat fullständigt; registret ger ingen säker global täckningsprocent.

Nästa urval: COMAIR Cincinnati–Louisville/Lexington eller Delta-linjer kring Memphis och Dallas/Fort Worth. Europakön behåller Cypern.

Tidtabellen visar planerad trafik. Faktiskt genomförande, förseningar, passagerare, last, individflygplan och verklig flygbana kräver andra belägg. Utgåvans slutdatum och senare ändringar är inte verifierade.

## Filer

- `delta_atlanta_returns_batch78.tsv`: 31 originalrader med a/p-notering, 24-timmarstid, trafikdagar och importbeslut.
- `delta_atlanta_returns_source_evidence_batch78.json`: källor, PDF-checksumma och sidmappning.
- `batch_78.json` och `validation_batch78.json`: nya rörelser, kalender/UTC-kontroll och oförändrade äldre poster.
- `errata_batch78.json`: de tre spårbara rättelserna.
- `WORLDWIDE_COVERAGE_BATCH78.md`: aktuell bolagsinventering.

Paketet är en datauppdatering med hela flygdatabasen och forskningshistoriken. C++-filer och byggfiler ingår inte. Full UE 5.8-kompilering och Editor-körning har inte utförts här.

Validerat: 20 Python-tester och tre genererings-/indexkontroller passerar. Kalender/UTC, dubbletter, överlappande flygnummer samt de separat tryckta anslutningarna för flyg 560 och 336 är kontrollerade.
