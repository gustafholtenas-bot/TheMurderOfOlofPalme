# Dallas/Fort Worth, Nassau och Salt Lake City – v109 / batch106

Granskad 1 oktober 2026. Kumulativ datauppdatering från v108.

135 nya planerade Delta-rörelser från 65 nya nonstop-scheman på 31 nya riktade sträckor. Fler Dallas/Fort Worth-linjer, Nassau till/från Dallas, Atlanta, Fort Lauderdale och LaGuardia, Salt Lake City till/från Dallas och Atlanta samt West Palm Beach–Fort Lauderdale. Totalt **7 265 rörelser**, varav 7 264 tidtabellslagda och en dokumenterat genomförd. Alla 7 130 äldre rörelseobjekt och 4 207 äldre scheman är oförändrade. Nassau och Salt Lake City tillkommer som flygplatser, Bahamas som land. Separat UTC-kontroll stämmer med alla nya rörelser; 19 markuppehåll är kontrollerade. Se `research/DALLAS_NASSAU_SLC_BATCH106.md`.

## Sträckor

Alla förbindelser nedan har båda riktningarna, utom sista raden som enbart avser PBI–FLL. Antalet dagliga avgångar kan skilja mellan riktningarna.

| Förbindelse | Nya scheman | Nya rörelser |
|---|---:|---:|
| Dallas/Fort Worth–Albuquerque | 8 | 17 |
| Dallas/Fort Worth–Amarillo | 6 | 12 |
| Dallas/Fort Worth–Baltimore | 4 | 9 |
| Dallas/Fort Worth–Detroit | 4 | 9 |
| Dallas/Fort Worth–Little Rock | 7 | 14 |
| Dallas/Fort Worth–Newark | 4 | 9 |
| Dallas/Fort Worth–LaGuardia | 8 | 17 |
| Dallas/Fort Worth–Philadelphia | 5 | 11 |
| Dallas/Fort Worth–West Palm Beach | 2 | 4 |
| Dallas/Fort Worth–Salt Lake City | 3 | 6 |
| Dallas/Fort Worth–Nassau | 2 | 3 |
| Atlanta–Salt Lake City | 3 | 6 |
| Atlanta–Nassau | 4 | 8 |
| Fort Lauderdale–Nassau | 2 | 4 |
| LaGuardia–Nassau | 2 | 4 |
| West Palm Beach → Fort Lauderdale | 1 | 2 |
| **Totalt** | **65** | **135** |

## Tidtabellsunderlag

Delta Air Lines, systemtidtabell från **1 februari 1986**, [Digital Library of Georgia](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201). Tryckta sidor **9, 10, 16, 17, 23, 63–67, 73, 83, 131, 168, 169, 173–175, 191, 208 och 248** granskades visuellt. Teckenförklaringen på sida 4 och flygplatsförteckningen på sidorna 260–261 används. Ingen av de 65 importerade raderna har en daterad fotnot. Källa, PDF-hash och sidmappning sparas i `dallas_nassau_slc_source_evidence_batch106.json`.

Urvalet innehåller alla explicit nonstop-rader i **29 valda stadsriktningsrubriker**, motsvarande 31 fysiska riktade flygplatspar. Newark och LaGuardia ligger under gemensamma New York-rubriker. Ytterligare en genomgående rad granskas och sparas som utesluten; övriga anslutningar och genomgående resalternativ ingår inte i transkriptionsantalet. Det är därför 66 rader i råunderlaget och 65 accepterade scheman.

**DFW–Columbus 210 (20:10–23:53) har ett stopp.** Förstorad originalrad bekräftar stoppkolumn 1. Rutten finns redan som DFW–Cincinnati 20:10–23:02 och Cincinnati–Columbus 23:22–23:53. Den genomgående raden skapar därför inga nya scheman eller rörelser. Båda tidigare delsträckorna är oförändrade. Även den granskade returtabellen CMH–DFW saknar nonstop-rader.

Utgåvans slutdatum och senare ändringsblad är inte verifierade. Importen beskriver planerad trafik, inte belagt genomförande, ett identifierat individflygplan eller verkliga flygbanor. Original-PDF och sidbilder ingår inte i leveransen.

## Klockslag, frekvens och mellanlandningar

- New York-suffixen **L=LaGuardia, E=Newark och J=Kennedy** hålls isär. Flyg 92/93 mellan Nassau och New York använder **LaGuardia**. JFK får inga nya rörelser i denna omgång.
- **DFW–Nassau 522 och Nassau–DFW 521 har X36**, alltså ingen trafik onsdag eller lördag. **Philadelphia–DFW 589 har X6**, ingen lördagstrafik. Tomt frekvensfält, kodat `D`, betyder dagligen. Varje fysisk delsträcka behåller sina egna trafikdagar.
- **DFW–West Palm Beach 538 går 13:21–16:40**. Det fortsätter som en separat källbelagd sträcka **PBI–Fort Lauderdale 17:25–17:45**, med 45 minuter på marken. En genomgående DFW–FLL-linje med stopp skapas inte som en extra nonstop-rörelse.
- **LaGuardia–DFW 467** fortsätter efter 49 minuter på marken till San Diego. **Newark–DFW 281** har 51 minuter till DFW–Salt Lake City. **Nassau–FLL 775** har 40 minuter till den befintliga FLL–DFW-sträckan. Samma flygnummer och sammanhängande tider bevisar inte individflygplanets identitet.
- Dallas, Amarillo och Little Rock använder UTC−6; Albuquerque och Salt Lake City UTC−7; övriga tillagda ändpunkter, inklusive Nassau, UTC−5 under importdatumen. Stjärnan i tidtabellen är en prismarkering och inte en dygnsregel.

Flygfönstret är **1986-02-27 22:21:30 UTC till 1986-03-01 22:21:30 UTC**. En avgång ingår om den börjar före fönstrets slut och landar efter dess början; hela tider behålls. DFW–Nassau 522 den 27 februari landar 22:15 UTC, före fönstrets start, och ingår inte. Den 28 februari ger en rörelse och den 1 mars är lördag utan trafik. Sex scheman ger tre rörelser, 58 ger två och ett ger en.

## Nya historiska flygplatsmarkörer

**Salt Lake City International (SLC)** tillkommer. [Flygplatsmyndighetens historik](https://slcairport.com/about-the-airport/airport-overview/airport-history/) beskriver namnet från 1968 och anläggningsarbeten före 1986. Koordinaterna bygger på [FAA:s referenspunkt](https://www.faa.gov/air_traffic/publications/atpubs/aip_html/part3_ad_2.0_utah.html).

**Nassau International–Windsor Field (NAS)** tillkommer under det nya landet Bahamas. [Bahamas Department of Aviation](https://www.doabahamas.com/about) anger att Windsor Field blev Nassau International 1957 och att namnet Lynden Pindling infördes 2006. Det senare namnet används därför inte för 1986. Landet var självständigt sedan 1973. [Flygplatsoperatören](https://nassaulpia.com/news/nad-begins-demolition-of-old-domestic-intl-terminal-at-lpia/) beskriver den äldre terminalens öppnande 1957. Koordinaterna är [AirNavs uppskattade FAA-position för MYNN](https://airnav.com/airport/MYNN).

Moderna referenskoordinater används enbart som ungefärliga markörer vid samma flygfält. De återskapar inte 1986 års terminal, uppställningsplats, bana eller flygplansposition. Källor och avgränsning finns även i `airport_locations_batch106.json`.

## Kontroller och bevarande

Separat manuell 24-timmarsinmatning av de 65 accepterade raderna jämförs med råtidernas tolkning. En separat kalenderberäkning med fasta vintertidsförskjutningar överensstämmer med samtliga **135 nya UTC-intervall**. Dubblett- och tidsöverlappskontrollen upptäckte den genomgående Columbus-raden; efter uteslutningen finns inga tidsöverlapp för berörda Delta-flygnummer. **19 markuppehåll** mellan separat källbelagda delsträckor är kontrollerade.

Alla 7 130 äldre rörelseobjekt, 4 207 äldre scheman, 297 äldre flygplatser, 81 äldre länder och 96 operatörer är oförändrade. Endast Delta-källposten får kompletterande granskningsnoteringar. Tidigare COMAIR-rättelser 1582/1580, rättelsen av uteslutna RIC–ATL 1125 samt alla tre hållna källkonflikter bevaras. De 20 befintliga Python-testerna och aktualitetskontrollerna redovisas i `automated_checks_batch106.json`.

Unreal-kompilering, Editor och paketerat spel har inte körts här. Paketet förutsätter redan kompilerad PaintAirplane-rättelse från `f23b73a`.

## Revisionsunderlag och fortsatt arbete

- `dallas_nassau_slc_batch106.tsv`: 65 accepterade nonstop-rader samt en utesluten genomgående rad.
- `independent_clock_transcription_batch106.txt`: separat inmatade klockslag, dygn och veckodagar.
- `dallas_nassau_slc_source_evidence_batch106.json` och `airport_locations_batch106.json`: källor och tolkningsbeslut.
- `batch_106.json`: nya schema- och rörelse-ID:n samt antal per riktning.
- `validation_batch106.json`, `automated_checks_batch106.json` och `package_preservation_batch106.json`: beräkningar, tester och bevarande.
- `worldwide_coverage_batch106.json`: operatörsvis täckning.

Nästa större urval kan omfatta Delta Connections regionala linjer i Texas/Oklahoma kring DFW, fler separat tidsatta mellanliggande ben samt Delta/Ransome i nordöstra USA. Världsinventeringen och representerade bolags nät är fortfarande ofullständiga. Europeiska landkön återupptas fortsatt vid Cypern.
