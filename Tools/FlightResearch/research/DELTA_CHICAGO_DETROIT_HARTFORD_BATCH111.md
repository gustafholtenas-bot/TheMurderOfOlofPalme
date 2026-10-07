# Delta – Chicago, Detroit, Hartford och Florida, v114 / batch111

Granskad 1 oktober 2026. Kumulativ datauppdatering från v113.

156 nya planerade Delta-rörelser från 75 granskade nonstop-scheman på 50 riktade flygplatspar, varav 49 är nya i databasen. Fler linjer kring Chicago O’Hare, Detroit och Hartford, Florida, Boston, Minneapolis, New Orleans och Philadelphia samt Chicago–JFK. 74 scheman ger rörelser i fönstret; en lördagsavgång från Chicago går senare. Totalt **8 071 rörelser**, varav 8 070 tidtabellslagda och en dokumenterat genomförd. Alla 7 915 äldre rörelseobjekt och 4 646 äldre scheman är oförändrade. Se `research/DELTA_CHICAGO_DETROIT_HARTFORD_BATCH111.md`.

## Sträckor

50 riktade ändpunktspar berörs. 49 är nya; **MIA → TPA** fanns redan med Pan Am och får nu Delta-trafik. Varje riktning räknas separat. Inga nya flygplatser, länder eller operatörer tillkommer.

| Riktad sträcka | Nya scheman | Nya rörelser |
|---|---:|---:|
| BDL → FLL | 2 | 4 |
| BDL → MCO | 1 | 2 |
| BDL → PBI | 1 | 3 |
| BDL → TPA | 1 | 2 |
| BOS → ORD | 4 | 9 |
| DTW → FLL | 1 | 2 |
| DTW → MCO | 1 | 2 |
| DTW → MIA | 1 | 1 |
| DTW → ORD | 1 | 2 |
| DTW → PBI | 1 | 2 |
| DTW → TPA | 2 | 5 |
| FLL → BDL | 2 | 4 |
| FLL → DTW | 1 | 2 |
| FLL → MCO | 1 | 2 |
| FLL → ORD | 2 | 5 |
| FLL → TPA | 1 | 2 |
| MCO → BDL | 2 | 4 |
| MCO → DTW | 1 | 3 |
| MCO → FLL | 1 | 2 |
| MCO → MSY | 1 | 2 |
| MCO → ORD | 2 | 4 |
| MCO → PBI | 1 | 2 |
| MIA → DTW | 1 | 1 |
| MIA → ORD | 1 | 2 |
| MIA → TPA | 1 | 2 |
| MSP → ORD | 3 | 7 |
| MSY → MCO | 1 | 2 |
| MSY → ORD | 3 | 6 |
| MSY → TPA | 1 | 3 |
| ORD → BOS | 4 | 6 |
| ORD → DTW | 1 | 2 |
| ORD → FLL | 2 | 5 |
| ORD → JFK | 1 | 2 |
| ORD → MCO | 2 | 4 |
| ORD → MIA | 1 | 2 |
| ORD → MSP | 3 | 6 |
| ORD → MSY | 3 | 7 |
| ORD → PBI | 1 | 2 |
| ORD → TPA | 2 | 4 |
| PBI → BDL | 1 | 2 |
| PBI → DTW | 1 | 2 |
| PBI → ORD | 1 | 3 |
| PHL → TPA | 1 | 2 |
| TPA → BDL | 1 | 2 |
| TPA → DTW | 2 | 4 |
| TPA → MCO | 1 | 2 |
| TPA → MIA | 1 | 2 |
| TPA → MSY | 1 | 2 |
| TPA → ORD | 2 | 4 |
| TPA → PHL | 1 | 2 |
| **Totalt** | **75** | **156** |

## Källa och urval

[Delta Air Lines systemtidtabell från 1 februari 1986, Digital Library of Georgia](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201). Tryckta sidor **31, 46–49, 73–75, 82–83, 100–101, 150–151, 153, 155, 170, 172, 183–185, 193, 230–232 och 248** har granskats visuellt. Tidtabellens operatörsnyckel på s.4 identifierar Delta Connection-intervallen; samtliga nya flyg har Delta-mainline-nummer under 1200.

75 accepterade rader anger **0 stopp**. En ytterligare nonstop-rad börjar först 3 mars och redovisas separat som utesluten. Urvalet gör inte anspråk på ett fullständigt bolagsnät. Genomgående flyg med stopp och anslutningsförslag skapar inga extra nonstop-ben. Miami/Fort Lauderdale-samlingsrubriker kan upprepa samma avgång som en separat flygplatsrubrik; den räknas då en gång.

Hela relevanta raden har granskats, inklusive frekvens- och fotnotskolumner. Skannade uppslag har varierande bredd; sidans faktiska gränser används. Förstorade utsnitt löste bland annat **TPA–ORD 480 11:05–12:31** och **TPA–DTW 206 18:50–21:07** före import. **MCO–ORD 80** har ett stopp och ingår därför inte i den nya nonstop-importen. PDF-hash och sidmappning finns i `delta_chicago_detroit_hartford_source_evidence_batch111.json`.

Utgåvans slutdatum och senare ändringar är inte verifierade. Tidtabellen visar planerad trafik, inte bevisat genomförande, individflygplan eller verkliga flygbanor. Original-PDF och nya sidbilder ingår inte i paketet.

## Daterade fotnoter och trafikdagar

**Fotnot 4, gäller från 12 februari**, finns på tio accepterade rader: **BDL–FLL 777, BDL–PBI 609, FLL–BDL 474, PBI–BDL 608, DTW–MCO 367, MCO–DTW 368, DTW–TPA 607, TPA–DTW 606, DTW–PBI 611 och PBI–DTW 610**. Startdatumen ligger före importperioden.

**ORD–MSP 391 07:00–08:06** finns som två liknande tryckta rader på s.48. Fotnoterna förklaras på s.49:

| Variant | Dagar | Daterad fotnot | Hantering |
|---|---|---|---|
| Daglig avgång | Alla dagar | 14: sista trafikdag 1 mars | Importerad; två rörelser i fönstret |
| Senare variant | Alla dagar utom söndag | 8: från 3 mars | Utesluten; inget nytt schema eller rörelse |

Den senare varianten får inte skapa en dubblett eller ändra den äldre radens trafikdagar. Se `excluded_source_rows_batch111.json`.

**ORD–BOS 598** går 16:00–18:59 utom lördag. **576** går lördag 17:00–19:59. Lördagsavgången den 1 mars är **23:00 UTC**, efter fönstrets slut 22:21:30 UTC. Schemat behålls som granskat underlag utan rörelse. Därför ger 74 av 75 scheman animation i perioden.

**DTW–MIA 501** går helger, medan **MIA–DTW 554** går endast lördag. Varje separat ben behåller sina tryckta dagar; returen antas inte automatiskt ha samma dagar som utresan. D i transkriptionen betyder tomt frekvensfält/dagligen; X undantar dagar, 1=måndag och 7=söndag. Stjärnan betyder lågtrafikpris, inte datumskifte.

Chicago avser **O’Hare/ORD**. **J=JFK**, **M=Miami/MIA** och **F=Fort Lauderdale/FLL** skiljer flygplatserna i samlingsrubrikerna.

## Tider och separat tidsatta delsträckor

**Chicago, Minneapolis och New Orleans använder UTC−6** under importdatumen. Övriga nya ändpunkter använder **UTC−5**. **DTW–ORD 563 08:20–08:18 lokal tid** är således **13:20–14:18 UTC**, en flygtid på 58 minuter samma kalenderdag. Den tidigare lokala ankomstklockan innebär inget datumskifte.

**MCO–FLL 261 00:30–01:15** och **TPA–MIA 141 00:55–01:40** har egna lokala avgångsdatum efter midnatt. De ansluter tidtabellsmässigt till äldre ATL–MCO respektive ATL–TPA-ben som avgår föregående kalenderdag. Kontrollen jämför UTC-tidpunkter och ger 25 respektive 40 minuters markuppehåll.

Fler exempel på separat tidsatta ben:

- FLL–TPA–MSY **91**, med 43 minuter i Tampa.
- BDL–MCO–PBI **483**, med 39 minuter i Orlando.
- TPA–MCO–BDL **496**, med 32 minuter i Orlando.
- FLL–ORD–MSP **458**, med 30 minuter i Chicago.
- PBI–ORD–MSP **204** och MSP–ORD–PBI **203**, med 31 minuter i Chicago.
- MSY–TPA–BDL **826**, med 35 minuter i Tampa.

Genomgående totalrader importeras inte som ytterligare rörelser. Markuppehållen visar tidtabellsmässig kontinuitet och fastställer inte vilket individflygplan som användes.

Fönstret är **1986-02-27 22:21:30 UTC till 1986-03-01 22:21:30 UTC**. Hela intervallet behålls för varje flyg som överlappar fönstret. Rörelser per lokalt avgångsdatum: **27 februari: 27; 28 februari: 72; 1 mars: 57**. Fördelning per nytt schema: ett ger noll, två ger en, 62 ger två och tio ger tre rörelser.

## Validering och bevarande

En separat 24-timmarstranskription av **75 rader** jämförs med tolkningen av originalets klockslag. Oberoende kalenderberäkning med fasta vintertidsförskjutningar stämmer med **alla 156 nya UTC-intervall**. **22 markuppehåll** mellan källbelagda gamla och nya ben har granskats; varje par har två matchande övergångar i det genererade fönstret.

Inga överlapp hittas mellan berörda gamla och nya rörelser med samma operatör och flygnummer. Alla **7 915 äldre rörelseobjekt, 4 646 scheman, 317 flygplatser, 82 länder och 98 operatörer** är oförändrade. Endast Delta-källposten kompletteras med nya granskningsnoteringar. Tidigare rättelser och tre hållna källkonflikter bevaras.

Alla **20 Python-tester** samt kontrollerna av genererad flygdata, landindex och Europaindex passerar. Exakta testutdata finns i `automated_checks_batch111.json`; oberoende UTC-resultat och markintervall finns i `validation_batch111.json`. Verktygskoden är oförändrad. Unreal-kompilering, Editor och paketerat spel har inte körts. Paketet förutsätter redan kompilerad PaintAirplane-rättelse från `f23b73a`.

## Fortsatt arbete

Fler Delta-linjer och separat tidsatta mellanliggande ben återstår. Äldre hållna källkonflikter kräver oberoende underlag. Bolagsnät och världsinventering är fortsatt ofullständiga; ingen säker återstående mängd eller procent färdigt kan anges. Europeiska landkön återupptas vid Cypern.
