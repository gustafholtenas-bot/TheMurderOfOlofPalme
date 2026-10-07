# Cleveland, Detroit och Atlanta – v80 / forskningsomgång 77

68 nya planerade flygrörelser: 21 COMAIR och 47 Delta. 40 nya nonstop-scheman från 64 visuellt granskade rader i 14 riktningstabeller. Databasen innehåller totalt 4 951 rörelser: 4 950 enligt tidtabell och en tidigare dokumenterat genomförd rörelse.

| Förbindelse | Nya rörelser |
|---|---:|
| Cleveland–Columbus, båda riktningar | 7 |
| Cleveland–Indianapolis, båda riktningar | 7 |
| Indianapolis–Detroit, båda riktningar | 18 |
| Indianapolis–Fort Wayne, båda riktningar | 4 |
| Columbus → Dayton | 2 |
| Detroit → Milwaukee | 1 |
| Cleveland → Atlanta | 11 |
| Columbus → Atlanta | 9 |
| Indianapolis → Atlanta | 9 |
| **Totalt** | **68** |

Ingen returriktning härleds. Milwaukee–Detroit-tabellen granskades också, men innehåller en genomgående resa med mellanlandning och ett anslutningsförslag; ingen direktsträcka importeras därifrån. Från Atlanta återstår returriktningarna i ett senare urval.

## Källa och avgränsning

Delta Air Lines System Timetable, effective February 1, 1986. Delta Flight Museum / Digital Library of Georgia.

- Arkivpost: https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201
- PDF: https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf
- Nya avgångar: tryckta sidor 52, 53, 61, 62, 74, 89, 110 och 111. Även s. 154 granskas för returriktningen Milwaukee–Detroit.
- Operatörer och trafikdagar: s. 4. Flygnummer 1525–1749 tillhör COMAIR.
- Linjeförteckningen s. 262–265 används för att kontrollera flygnummer mot rätt delsträckor. Den används inte för att härleda avgångstider.
- Rättelse av tidigare post: avgångssida 73 och linjeförteckning s. 263.

40 nonstop-rader importeras (18 COMAIR och 22 Delta). 20 anslutningsförslag och fyra genomgående rader med ett stopp utesluts. Ingen accepterad rad har en daterad fotnot. Förkortningen D i granskningsfilen betyder tomt frekvensfält i originalet, alltså dagligen.

Alla tider lästes i originalbilderna. OCR användes för navigering. Flygnummer med otydlig 6/8 kontrollerades också mot linjeförteckningen. Originalskanningarna omdistribueras inte.

## Tider och trafikdagar

- Exakt fönster: 1986-02-27 22:21:30 UTC till 1986-03-01 22:21:30 UTC.
- Alla nya avgångsflygplatser använder UTC−5 under dessa datum. Milwaukee använder UTC−6. Alla accepterade ankomster sker samma lokala kalenderdag som avgången.
- Detroit–Milwaukee 1683, 06:45–07:00 lokal tid, tar 75 minuter när tidszonsbytet beaktas. Nästa redan inlästa ben med 1683, Milwaukee–Cleveland, har egna tider och trafikdagar.
- Indianapolis–Detroit 1560 är 19:25–20:40, X6. Flygnumret är 1560, inte det liknande 1580 (som enligt linjeförteckningen går Chicago–Milwaukee).
- Indianapolis–Detroit 1574 går X56. Detta ben kompletterar den separat granskade Milwaukee–Indianapolis-raden från v79. Ingen flygning antas fortsätta på fredagen bara för att första benet går den dagen.
- Detroit–Indianapolis 1699 har fredagstiden 20:45–21:59 (5) och en separat X56-variant 20:55–22:10. De importeras med skilda dagmönster.
- Columbus–Atlanta 455 ankommer 07:41 och 249 ankommer 17:44. Siffrorna kontrollerades i förstoring.
- Fort Wayne–Indianapolis 543 och Indianapolis–Atlanta 543 har separata tidtabellsrader. En direktlinje Fort Wayne–Atlanta antas inte.
- Den uteslutna anslutningen Cleveland–Indianapolis 1603/1140 avgår 21:10 och ankommer 01:05 nästa dag. Den skapar ingen nonstopsträcka.

Nya avgångar per lokal kalenderdag: 27 februari 14, 28 februari 37, 1 mars 17. Flyg som överlappar fönsterstarten behåller sina fulla tider. Veckodagar och UTC-tider kontrolleras separat med fasta vinteroffsetar mot byggverktygets historiska IANA-tidszoner.

## Rättelse av ett äldre flygnummer

Den tidigare posten `delta-581-DTW-CVG-0800-b71` hade fel flygnummer. Detroit–Cincinnati 08:00–08:51 är **Delta 561**, vilket också framgår av linjeförteckningen DTW–CVG–MIA på s. 263. Delta 581 går DTW–IND–MEM–MSY.

Rättelsen ändrar flygnummer och källanmärkning på två befintliga rörelser, 28 februari och 1 mars. Tekniska ID:n, tider och sträckor behålls. Det historiska ID:t innehåller därför fortfarande 581, medan visat flygnummer är 561. Äldre granskningsfiler bevaras; `errata_batch77.json` anger vilka uppgifter som ersätts, inklusive en tidigare utesluten anslutningsrad i batch75.

Alla 4 883 tidigare rörelse-ID:n finns kvar. 4 881 rörelseposter är helt oförändrade, två har ovanstående rättelse. Alla övriga tidigare schemarader och samtliga flygplatser, operatörer och länder är oförändrade.

## Täckning och leverans

241 flygplatser/platser, 1 021 riktade platspar. COMAIR har 255 rörelser och Delta 969. Fortfarande 95 registrerade operatörer, 49 med rörelser och 46 utan. Inget bolag är verifierat fullständigt; registret är ingen fullständig inventering av alla världens flygbolag 1986.

V80 är en kumulativ **datauppdatering** för koden i Git-commit `f23b73a6574e466b8e08d410bb92ef86c401a9c2`. Paketet innehåller hela `catalog.json`, hela `flights.json` samt forskningsverktyg och granskningshistorik. Unreal-källkod och byggfiler ingår inte. Den befintliga installationen behöver redan ha den kompilerade rättelsen av `PaintAirplane`.

Tidtabellen belägger planerad trafik. Genomförande, förseningar, passagerare, last, individflygplan och verklig flygbana är inte belagda av detta underlag. Utgåvans slutdatum och senare ändringar är inte verifierade.

Nästa urval: COMAIR Cincinnati–Louisville/Lexington samt Deltas returriktningar från Atlanta till Cleveland, Columbus och Indianapolis. Europakön behåller Cypern.

## Granskningsfiler

- `comair_delta_midwest_batch77.tsv`: varje tidtabellsrad, trafikdagar och beslut.
- `comair_delta_midwest_source_evidence_batch77.json`: källor, PDF-checksumma och sidmappning.
- `batch_77.json` och `validation_batch77.json`: nya ID:n, antal, UTC-kontroll och bevarande av äldre data.
- `errata_batch77.json`: spårbar rättelse av flygnummer 561.
- `WORLDWIDE_COVERAGE_BATCH77.md`: aktuell bolagsinventering.

Full UE 5.8-kompilering och Editor-körning har inte utförts här.

Validerat: 20 Python-tester och tre genererings-/indexkontroller passerar. Separat UTC-/kalenderkontroll, dubblettkontroll och kontroll av överlappande nya flygnummer passerar. Alla tidigare rörelse-ID:n finns kvar; två flygnummeretiketter är rättade enligt erratafilen.
