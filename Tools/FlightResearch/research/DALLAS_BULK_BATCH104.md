# Dallas/Fort Worth – v107 / batch104

Granskad 1 oktober 2026. Kumulativ datauppdatering från v106.

**219 nya planerade Delta-rörelser från 107 granskade nonstop-scheman på 24 riktade sträckor.** Totalt 6 945 rörelser: 6 944 tidtabellslagda och en tidigare dokumenterat genomförd. Alla 6 726 tidigare rörelseobjekt och 4 011 tidigare scheman är oförändrade. Inga nya flygplatser eller bolag tillkommer.

Tillsammans tillför v106 och v107 **390 rörelser och 191 scheman sedan v105**, på 43 nya riktade sträckor. v107 innehåller hela föregående databasen och båda omgångarnas forskningsunderlag.

## Sträckor

Alla nedanstående förbindelser är granskade i båda riktningarna.

| Dallas/Fort Worth till/från | Nya scheman | Nya rörelser |
|---|---:|---:|
| Austin Mueller | 14 | 28 |
| Chicago O’Hare | 10 | 22 |
| Denver Stapleton | 12 | 21 |
| El Paso | 8 | 17 |
| Boston | 6 | 14 |
| Fort Lauderdale | 4 | 9 |
| Jacksonville | 2 | 5 |
| Orlando | 5 | 11 |
| Houston Hobby | 7 | 14 |
| Houston Intercontinental | 14 | 28 |
| San Antonio | 14 | 28 |
| Oklahoma City | 11 | 22 |
| **Totalt** | **107** | **219** |

## Källa och urval

Delta Air Lines, systemtidtabell från **1 februari 1986**, [Digital Library of Georgia](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201). Tryckta sidor **21, 32, 47, 64–66, 71, 78, 82, 103, 116, 179, 183 och 211** har granskats visuellt. Teckenförklaringen på tryckt sida 4 används. Ingen av de valda nonstop-raderna har en daterad fotnot.

Denna större omgång transkriberar **samtliga explicit nonstop-rader inom 22 valda stadsriktningsrubriker**. Houston-rubrikerna innehåller två flygplatser, vilket ger 24 riktade flygplatspar. Anslutningsförslag och genomgående rader med stopp har lästs för att skilja dem från nonstop, men transkriberas och räknas inte i denna omgång. Därför betyder 107 antalet valda nonstop-rader, inte samtliga tryckta resalternativ under rubrikerna. Tidigare omgångars kompletta råtranskriptioner bevaras.

Tidtabellens slutdatum och senare ändringsblad är inte verifierade. Importen använder lokala avgångsdatum 27 februari–1 mars och beskriver planerad trafik, inte belagt genomförande eller faktiska flygbanor. Original-PDF och sidbilder ingår inte i paketet; länkar och PDF-hash finns i källunderlaget.

## Tid, frekvens och flygplatsidentitet

- `D` i TSV motsvarar tidtabellens tomma frekvensfält, dagligen. `X` undantar angivna dagar; 1=måndag till 7=söndag. `X3` på DFW–BOS 358 undantar onsdag, medan `X36` på BOS–DFW 487 undantar både onsdag och lördag.
- Denver–DFW **230 och 461 går endast lördag**, medan 812 och 695 vid samma klockslag går övriga dagar. De två lördagsschemana sparas som granskade, men deras avgångar ligger efter fönstrets slut och skapar inga rörelser.
- DFW–Oklahoma City **929 avgår 23:19 och anländer 00:05 nästa lokala kalenderdag**. Stjärnan är en lågtrafikprismarkering och används inte som dygnsregel.
- Dallas/Fort Worth, Austin, Chicago, Houston, San Antonio och Oklahoma City använder UTC−6; Denver och El Paso UTC−7; Boston, Fort Lauderdale, Jacksonville och Orlando UTC−5 under importdagarna. Boston–DFW 831/359/487 tar därför 4 timmar 1 minut, trots 3 timmar 1 minuts lokal klockskillnad.
- DFW-rubriken avser **Dallas/Fort Worth International**, inte Dallas Love Field. Chicago avser O’Hare. Austin använder **Mueller**, Denver **Stapleton**. Befintliga historiska flygplatsmarkörer bevaras.
- Houstons ändpunktssuffix **I** betyder Intercontinental/IAH och **H** Hobby/HOU. De har separata rörelser och platsmarkörer. Ingen sammanblandning av stadens flygplatser görs.

Flygfönstret är **1986-02-27 22:21:30 UTC till 1986-03-01 22:21:30 UTC**. En flygning ingår om dess avgång ligger före slutet och ankomst efter början. Hela tider bevaras, även för flyg som redan är i luften vid startgränsen. Nio scheman ger tre överlappande rörelser, 96 ger två och två ger noll.

## Kontroller

En separat manuellt inmatad transkription i 24-timmarsformat jämförs mot råtidernas tolkning. Kalenderberäkning med fasta vintertidsförskjutningar stämmer med samtliga **219 nya UTC-intervall**. Inga tidsöverlapp finns mellan nya och äldre rörelser för de berörda Delta-flygnumren.

Åtta markuppehåll mellan separat källbelagda delsträckor är kontrollerade, bland annat Denver–DFW–Orlando 344 (31 minuter), San Antonio–DFW–Denver 377 (38), Orlando–DFW–Denver 233 (47), Denver–DFW–Boston 870 (40), El Paso–DFW–Oklahoma City 1066 (51), Oklahoma City–DFW–Austin 1081 (50), San Antonio–DFW–Oklahoma City 1092 (36) och Raleigh/Durham–DFW–Austin 1007 (43). Flygnumret och tidssambandet identifierar inte ett individuellt flygplan.

Alla äldre scheman, rörelser, flygplatser, bolag och orelaterade källposter är oförändrade. Tidigare rättelser för COMAIR 1582/1580 och RIC–ATL 1125 samt de tre tidigare hållna källkonflikterna bevaras. Automatiska testresultat redovisas i `automated_checks_batch104.json`; bevarandekontroll och exakta UTC-intervall i `validation_batch104.json`.

Unreal-kompilering, Editor och paketerat spel har inte körts här. Datauppdateringen förutsätter den redan kompilerade PaintAirplane-rättelsen från `f23b73a`.

## Revisionsunderlag och nästa steg

- `dallas_bulk_batch104.tsv`: de 107 valda nonstop-raderna och deras källsidor.
- `independent_clock_transcription_batch104.txt`: separat transkription med 24-timmarsklocka och dygnsregel.
- `dallas_bulk_source_evidence_batch104.json`: PDF-hash, sidor, frekvensregler och avgränsning.
- `batch_104.json`: exakta nya schema- och rörelse-ID:n samt antal per sträcka.
- `validation_batch104.json` och `automated_checks_batch104.json`: kontrollresultat.
- `worldwide_coverage_batch104.json`: operatörsvis täckning.

Återstående Dallas/Fort Worth-rubriker, bland annat västra USA och fler östkustförbindelser, är lämpliga för nästa större urval. Därefter kan nordöstra Delta/Ransome-nätet och fler separat tidsatta regionala delsträckor granskas. Varken Deltas nät eller världsinventeringen är verifierat fullständig.
