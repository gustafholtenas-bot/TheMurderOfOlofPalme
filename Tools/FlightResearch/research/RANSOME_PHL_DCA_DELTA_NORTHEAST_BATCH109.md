# Nordöstra USA – v112 / batch109

Granskad 1 oktober 2026. Kumulativ datauppdatering från v111.

127 nya planerade rörelser från 80 granskade nonstop-scheman: 61 rörelser med Ransome Airlines och 66 med Delta. Urvalet omfattar 28 riktade flygplatspar, varav 26 är nya i databasen. Fler linjer kring Philadelphia, Washington National och Boston samt två nya flygplatser: Hyannis/HYA och Trenton/TTN. Totalt **7 720 rörelser**, varav 7 719 tidtabellslagda och en dokumenterat genomförd. Alla 7 593 äldre rörelseobjekt och 4 472 äldre scheman är oförändrade. Separat UTC-kontroll stämmer med alla 127 nya rörelser. Se `research/RANSOME_PHL_DCA_DELTA_NORTHEAST_BATCH109.md`.

## Sträckor

47 nya scheman tillhör Ransome och 33 Delta. Varje rad nedan avser en riktning. BOS–PWM och PWM–BOS fanns redan med Ransome; nu tillkommer Delta på samma par. De övriga 26 riktade paren är nya i databasen.

| Riktad sträcka | Nya scheman | Nya rörelser |
|---|---:|---:|
| BDL → BOS | 2 | 4 |
| BOS → BDL | 2 | 4 |
| BOS → DCA | 3 | 6 |
| BOS → HYA | 1 | 1 |
| BOS → LGA | 2 | 4 |
| BOS → PHL | 4 | 8 |
| BOS → PWM | 4 | 8 |
| DCA → BOS | 3 | 6 |
| DCA → PHL | 10 | 12 |
| DCA → TTN | 3 | 4 |
| HYA → LGA | 1 | 1 |
| HYA → PVD | 1 | 1 |
| ISP → PHL | 1 | 1 |
| JFK → BOS | 1 | 2 |
| JFK → DCA | 2 | 4 |
| JFK → PHL | 3 | 5 |
| LGA → BOS | 3 | 6 |
| LGA → HYA | 1 | 1 |
| LGA → PHL | 2 | 3 |
| PHL → BDL | 1 | 2 |
| PHL → BOS | 5 | 10 |
| PHL → DCA | 9 | 10 |
| PHL → ISP | 3 | 3 |
| PHL → JFK | 4 | 7 |
| PHL → LGA | 1 | 1 |
| PWM → BOS | 4 | 8 |
| TTN → DCA | 2 | 2 |
| TTN → PHL | 2 | 3 |
| **Totalt** | **80** | **127** |

## Källor och avgränsning

[Delta Air Lines tidtabell från 1 februari 1986, Digital Library of Georgia](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201). Tryckta sidor **32–34, 100, 110, 134, 173, 174, 176, 191–193, 197, 236, 244, 246 och 247** har granskats visuellt. Operatörsnyckeln på s.4 och flygplatsförteckningen på s.260–261 används. PDF-hash och sidmappning finns i `ransome_phl_dca_delta_source_evidence_batch109.json`.

Alla 80 accepterade rader anger **0 stopp** och saknar daterade radfotnoter. Ransome identifieras genom nummerintervallet **1750–1874** på s.4; de 33 utvalda Delta-numren ligger under Delta Connections intervall. Det generella flygplansregistret används inte för att tilldela varje avgång flygplanstyp eller individ. Urvalet gör inte anspråk på att färdigställa något bolags hela nät. Utgåvans slutdatum och senare ändringar är inte verifierade.

Originaltider, suffix, trafikdagar och sidnummer finns i `ransome_phl_dca_delta_batch109.tsv`. Original-PDF och nya sidbilder ingår inte i paketet.

## Flygplatser, trafikdagar och mellanlandningar

- **New York J=JFK, L=LGA, E=EWR**. **Washington N=National/DCA, I=Baltimore/BWI**. JFK–DCA 1803 avgår **12:45J**, alltså från JFK, och landar **13:59N**. Suffixen bevaras i TSV och kontrolleras mot ändpunkts-ID:n.
- **DCA–PHL 1814** är en genomgående rad med ett stopp. De separat tryckta benen är **DCA–TTN 08:10–09:00** och **TTN–PHL 09:15–09:40**; därefter **PHL–DCA 10:00–10:45**. Den genomgående raden skapar inget extra nonstop-flyg.
- **DCA–PHL–ISP–BOS 1788** kompletterar tidigare ISP–BOS. **DCA–PHL–JFK–PVD–BOS 1804** kompletterar tidigare JFK–PVD–BOS. Varje ben har en egen explicit tidsatt nonstop-rad.
- **PWM–BOS 1787** undantar söndag, medan **BOS–HYA–LGA 1787** undantar både lördag och söndag. **DCA–PHL 1802** undantar lördag/söndag, medan **PHL–JFK 1802** bara undantar söndag. Egna trafikdagar behålls per ben.
- **LGA–HYA–PVD 1793** kompletterar tidigare PVD–BOS. Markuppehållen i Hyannis och Providence är 15 minuter vardera.
- **BOS–JFK–DCA 1839** får det separat tidsatta JFK–DCA-benet **22:15–23:30**. BOS–JFK från v111 importeras inte igen.
- **DCA–PHL 1759, PHL–ISP 1759 och PHL–DCA 1762** går endast söndag och ger inga rörelser i detta torsdag–lördag-fönster. De tre finns kvar som granskade scheman.

Exakta minuter bevaras, bland annat **BOS–PHL 589 17:24–18:34**, **BOS–DCA 323 12:13–13:35**, **JFK–BOS 518 16:10–17:00**, **LGA–BOS 582 22:35–23:23** och **TTN–DCA 1851 07:00–07:59**. Stjärnsymbolen anger lågtrafikpris, inte datumskifte. Ingen ny delsträcka passerar lokal midnatt; **PHL–BOS 256 01:25–02:25** ligger helt efter midnatt och anknyter till det äldre ATL–PHL-benet med 40 minuters uppehåll över dygnsgränsen.

Alla nya ändpunkter använder **UTC−5** under importdatumen. D kodar tomt frekvensfält/dagligen; X undantar angivna dagar, 1=måndag och 7=söndag. Flygfönstret är **1986-02-27 22:21:30 UTC till 1986-03-01 22:21:30 UTC**. Hela intervall behålls vid överlapp. Exempelvis avgår PHL–DCA 1823 den 27 februari kl.17:10 lokal tid och är i luften när fönstret börjar kl.17:21:30.

Tre scheman ger noll rörelser, 27 ger en och 50 ger två. Lokala avgångsdatum ger 25 rörelser den 27 februari, 77 den 28 februari och 25 den 1 mars.

## Två nya historiska flygplatsmarkörer

| Kod | Namn enligt 1986 års tidtabell | Källa | Ungefärlig referenspunkt |
|---|---|---|---|
| HYA | Hyannis – Barnstable County | Tryckt s.260 | 41.6693372, −70.2803592 |
| TTN | Trenton/Princeton – Mercer County | Tryckt s.261 | 40.2766944, −74.8134722 |

Aktuella uppskattade FAA-referenspunkter återgivna av [AirNav HYA](https://www.airnav.com/airport/HYA) och [AirNav TTN](https://www.airnav.com/airport/TTN) används som ungefärliga markörer vid samma namngivna flygfält. FAA-posternas aktiveringsmetadata ligger före 1986 och används inte som exakta historiska invigningsdatum. Namn och koder hämtas från originaltidtabellen. Full metadata finns i `airport_locations_batch109.json`.

Markörerna återskapar inte 1986 års terminaler, uppställningsplatser, bantrösklar eller flygplanspositioner. Underlaget visar planerad trafik, inte bevisat genomförda flyg, identifierade individflygplan eller verkliga flygbanor.

## Validering och bevarande

Separat manuell 24-timmarsinmatning av alla **80 rader** jämförs med originaltidernas tolkning. En separat kalenderberäkning med fast vintertidsförskjutning stämmer med **alla 127 nya UTC-intervall**.

**39 markuppehåll** mellan separat tidsatta ben har granskats, inklusive övergångar mellan äldre och nya scheman. **37** har matchande rörelser i fönstret; två övergångar för söndagsflyg 1759 ligger utanför. Dessa kontroller visar tidtabellsmässig kontinuitet, inte att ett visst individflygplan användes. Inga tidsöverlapp finns i jämförelser av äldre och nya rörelser med samma operatör och berört flygnummer där minst ett ben är nytillagt.

Alla **7 593 äldre rörelseobjekt**, **4 472 äldre scheman**, **313 flygplatser**, **82 länder** och **98 operatörer** är oförändrade. Endast Delta-källposten får kompletterande granskningsnoteringar. Tidigare rättelser och tre hållna källkonflikter bevaras. Verktyg och äldre forskningsfiler är oförändrade utom dokumenterade aktuella index och statusfiler.

Alla **20 Python-tester** samt kontrollerna av genererad flygdata, landindex och Europaindex passerar. Utdata finns i `automated_checks_batch109.json`. Unreal-kompilering, Editor och paketerat spel har inte körts. Paketet förutsätter redan kompilerad PaintAirplane-rättelse från `f23b73a`.

## Revisionsunderlag och fortsatt arbete

- `ransome_phl_dca_delta_batch109.tsv` och `independent_clock_transcription_batch109.txt`: originalrader och separat tidsinmatning.
- `ransome_phl_dca_delta_source_evidence_batch109.json` och `airport_locations_batch109.json`: källor och tolkningsbeslut.
- `batch_109.json`, `validation_batch109.json`, `automated_checks_batch109.json`, `package_preservation_batch109.json`: antal, ID:n, beräkningar, tester och bevarande.
- `worldwide_coverage_batch109.json`: operatörsvis täckning.

Nästa urval kan omfatta fler Delta-linjer i nordöstra USA, Montreal och återstående New York/Washington-förbindelser. Daterade fotnoter och flygplatssuffix behöver granskas för varje ny rad. Bolagsnät och världsinventering är fortsatt ofullständiga. Europeiska landkön återupptas vid Cypern.
