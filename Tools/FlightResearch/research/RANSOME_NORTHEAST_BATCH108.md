# Ransome Airlines – v111 / batch108

Granskad 1 oktober 2026. Kumulativ datauppdatering från v110.

158 nya planerade rörelser med nytillagda Ransome Airlines, från 108 granskade nonstop-scheman på 20 nya riktade sträckor. 102 scheman ger rörelser i fönstret. Regionala linjer kring Boston, New York och Providence samt fem nya flygplatser: ALB, BTV, ISP, PWM och PVD. Totalt **7 593 rörelser**, varav 7 592 tidtabellslagda och en dokumenterat genomförd. Alla 7 435 äldre rörelseobjekt och 4 364 äldre scheman är oförändrade. Separat UTC-kontroll stämmer med alla nya rörelser och 17 markuppehåll är kontrollerade. V110 och v111 har tillsammans lagt till 328 rörelser, 200 scheman, 14 flygplatser och två operatörer sedan v109. Se `research/RANSOME_NORTHEAST_BATCH108.md`.

## Sträckor

De första nio förbindelserna har båda riktningarna. De två sista raderna gäller endast den angivna riktningen. Trafikdagar och antal avgångar skiljer mellan riktningarna.

| Förbindelse | Nya scheman | Nya rörelser |
|---|---:|---:|
| Boston–Burlington VT | 14 | 23 |
| Boston–Long Island MacArthur | 15 | 20 |
| Boston–Portland ME | 8 | 13 |
| Boston–Providence | 14 | 17 |
| LaGuardia–Albany NY | 11 | 15 |
| LaGuardia–Hartford/Bradley | 10 | 15 |
| Providence–LaGuardia | 16 | 25 |
| Providence–Newark | 9 | 12 |
| Providence–JFK | 9 | 14 |
| Hartford/Bradley → Providence | 1 | 2 |
| Boston → JFK | 1 | 2 |
| **Totalt** | **108** | **158** |

## Källa, operatör och avgränsning

[Delta Air Lines systemtidtabell från 1 februari 1986, Digital Library of Georgia](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201). Tryckta sidor **8, 31–33, 37, 101, 134, 173, 174, 176, 197, 199 och 200** granskades visuellt. Teckenförklaringen på s.4 och flygplatsförteckningen på s.260–261 används. PDF-hash, sidmappning och tolkningsbeslut finns i `ransome_northeast_source_evidence_batch108.json`.

Sidan 4 anger **Ransome Airlines för flygnummer 1750–1874**. Även **1874 Boston–Portland** hör alltså till Ransome. Rios angränsande intervall börjar först vid 1875. Ransome registreras som ett eget bolag med källan Delta och alias Delta Connection. Den generella flygplansförklaringen används inte för att tillskriva varje avgång en viss typ eller flygplansindivid.

Urvalet omfattar samtliga explicit nonstop-rader för Ransome i **16 valda stadsriktningsrubriker**, motsvarande 20 fysiska riktade flygplatspar. De två Providence/New York-rubrikerna innehåller tre separata New York-flygplatser. Delta-mainline, anslutningsförslag och genomgående rader med stopp ingår inte i detta operatörsurval. Ingen accepterad rad har en daterad fotnot. Utgåvans slutdatum och senare ändringsblad är inte verifierade. Original-PDF och sidbilder ingår inte i paketet.

## Flygplatskoder, mellanlandningar och veckodagar

- **L=LaGuardia/LGA, J=Kennedy/JFK och E=Newark/EWR**. Suffixen finns kvar i TSV-filens originaltider och valideras mot varje avkodat ändpunkts-ID. **Long Island** i dessa rubriker är MacArthur/ISP. Albany avser **New York/ALB**, inte Georgia/ABY. Portland avser **Maine/PWM**, inte Oregon/PDX.
- **BOS–New York 1757/1815/1827/1849** har ett stopp vid Providence. Endast separat tryckta BOS–PVD och PVD–JFK-ben importeras. Genomgående tider skapar inga extra nonstop-flyg. BOS–JFK 1839 är däremot explicit nonstop, **20:45–21:55**.
- **LGA–PVD 1784** har ett stopp vid Hartford. Det finns som **LGA–BDL 18:59–19:50** och **BDL–PVD 20:10–20:45**. Även här behålls egna ben, med 20 minuter på marken.
- **BTV–BOS 1831** undantar lördag, medan **BOS–ISP 1831** går dagligen. **BTV–BOS 1801** undantar söndag, men **BOS–PVD 1801** undantar både lördag och söndag. **BOS–PVD 1849** går dagligen, medan fortsättningen **PVD–JFK 1849** undantar söndag. Egna veckodagar bevaras för varje ben.
- Originalets exakta minuter bevaras, exempelvis **BOS–BTV 1777 12:13**, **LGA–ALB 1865 12:29**, **LGA–ALB 1798 14:57** och **BOS–PVD 1827 15:51**.
- **1754 BTV–BOS, 1760 BOS–ISP, 1759 ISP–BOS, 1753 PWM–BOS och 1777 PVD–BOS** går bara söndag och saknar rörelser i detta torsdag–lördag-fönster. **1752 JFK–PVD** går lördag/söndag men lördagsavgången 17:59 ligger efter fönstrets slut 17:21:30 lokal tid. Alla sex behålls som granskade scheman med noll rörelser.

Alla ändpunkter använder **UTC−5** under importdatumen. Inget nytt ben passerar lokal midnatt. Tom frekvens, kodad D, betyder dagligen; X undantar angivna dagar, 1=måndag och 7=söndag.

Flygfönstret är **1986-02-27 22:21:30 UTC till 1986-03-01 22:21:30 UTC**. Hela intervall bevaras vid överlapp. BOS–BTV 1820 den 27 februari avgår 17:00 och landar 18:10 lokal tid: den ingår eftersom den fortfarande är i luften vid fönstrets start. Den 1 mars ingår samma flyg även med landning efter fönstrets slut. Det schemat ger tre rörelser. Fördelningen totalt är sex scheman med noll, 51 med en, 46 med två och fem med tre rörelser. Lokala avgångsdatum ger 31 rörelser den 27 februari, 93 den 28 februari och 34 den 1 mars.

## Fem nya historiska flygplatsmarkörer

| Kod | Namn för 1986 | Primär identifikation |
|---|---|---|
| ALB | Albany County | Tidtabell s.260 |
| BTV | Burlington International | Tidtabell s.260 |
| ISP | Long Island–MacArthur | Tidtabell s.260 och Long Island-rubriker |
| PWM | Portland International Jetport | Tidtabell s.261 |
| PVD | Providence–Theodore Francis Green | Tidtabell s.261 |

Aktuella uppskattade FAA-referenspunkter, återgivna av AirNav, används som ungefärliga markörer vid samma namngivna flygfält. FAA-posternas aktiveringsmetadata ligger före 1986 men används inte som exakta historiska invigningsdatum. [Burlingtons officiella historik](https://btv.aero/about/) beskriver flygfältet från 1920 och utveckling före 1986; det senare Leahy-namnet förs inte tillbaka till 1986. Historiska namn hämtas från originaltidtabellen. Individuella koordinatkällor och avgränsningar finns i `airport_locations_batch108.json`.

Markörerna återskapar inte 1986 års terminaler, uppställningsplatser, bantrösklar eller flygplanspositioner. Underlaget visar planerad trafik, inte bevisat genomförda flyg, identifierade individflygplan eller verkliga flygbanor.

## Validering och bevarande

Separat manuell 24-timmarsinmatning av alla 108 rader jämförs med tolkningen av originaltiderna. En separat kalenderberäkning med fast vintertidsförskjutning stämmer med **alla 158 nya UTC-intervall**. **17 markuppehåll** mellan separat tidsatta ben är kontrollerade; 16 har matchande rörelser i fönstret. PVD–BOS–BTV 1777 kan bara bilda den kedjan söndagar och har därför inga matchande kedjor i detta fönster. Inga tidsöverlapp förekommer mellan nya ben med samma Ransome-flygnummer.

Alla **7 435 äldre rörelseobjekt**, **4 364 scheman**, **308 flygplatser**, **82 länder** och **97 operatörer** är oförändrade, inklusive hela Rio-omgången i v110. Endast Delta-källposten får kompletterande granskningsnoteringar. Tidigare COMAIR-rättelser, RIC–ATL 1125-rättelsen och alla tre hållna källkonflikter bevaras. Äldre forskningsfiler och verktyg är oförändrade utom dokumenterade aktuella index och statusfiler.

Alla 20 Python-tester samt kontrollerna av genererad flygdata, landindex och Europaindex passerar. Utdata finns i `automated_checks_batch108.json`. Unreal-kompilering, Editor och paketerat spel har inte körts. Paketet förutsätter redan kompilerad PaintAirplane-rättelse från `f23b73a`.

## Revisionsunderlag och fortsatt arbete

- `ransome_northeast_batch108.tsv` och `independent_clock_transcription_batch108.txt`: originaltider, terminalkoder och separat tidsinmatning.
- `ransome_northeast_source_evidence_batch108.json` och `airport_locations_batch108.json`: källor och tolkningsbeslut.
- `batch_108.json`, `validation_batch108.json`, `automated_checks_batch108.json`, `package_preservation_batch108.json`: antal, ID:n, beräkningar, tester och bevarande.
- `worldwide_coverage_batch108.json`: operatörsvis täckning.

Nästa urval kan omfatta Ransomes återstående separat tidsatta ben kring Philadelphia, Washington, Trenton och Hyannis samt fler Delta-mainline-linjer i nordöstra USA. Världsinventeringen och bolagsnäten är ofullständiga. Europeiska landkön återupptas fortsatt vid Cypern.
