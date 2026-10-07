# Atlanta och sydöstra USA – v106 / batch103

Granskad 1 oktober 2026. Kumulativ datauppdatering från v105.

**171 nya planerade Delta-rörelser från 84 nya nonstop-scheman på 19 riktade sträckor.** Totalt 6 726 rörelser: 6 725 tidtabellslagda och en tidigare dokumenterat genomförd. Alla 6 555 tidigare rörelseobjekt och 3 927 tidigare scheman är oförändrade. Sju flygplatser tillkommer: CHS, CLT, CAE, GSO, DAB, MLB och SAV.

## Sträckor

| Sträcka | Scheman, båda riktningar om inte annat anges | Nya rörelser |
|---|---:|---:|
| Atlanta–Charleston, South Carolina | 13 | 25 |
| Atlanta–Charlotte | 12 | 26 |
| Atlanta–Columbia, South Carolina | 16 | 32 |
| Atlanta–Greensboro/High Point/Winston-Salem | 12 | 22 |
| Atlanta–Daytona Beach | 6 | 13 |
| Atlanta–Melbourne, Florida | 4 | 8 |
| Atlanta–Savannah | 14 | 30 |
| Raleigh/Durham–Dallas/Fort Worth | 4 | 9 |
| Charleston, SC–Savannah | 2 | 4 |
| Columbia, SC → Charleston, SC, endast denna riktning | 1 | 2 |
| **Totalt** | **84** | **171** |

## Källa och avgränsning

Delta Air Lines, systemtidtabell från **1 februari 1986**, [Digital Library of Georgia](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201). Originalets tryckta sidor 14–18, 37, 39, 41, 54–55, 66, 69, 94, 144, 201 och 220 har lästs visuellt för de valda rubrikerna. PDF-sidorna innehåller två tryckta sidor vardera. Teckenförklaringen på tryckt sida 4 och flygplatsförteckningen på sidorna 260–261 används.

Urvalet omfattar **20 kompletta riktningsrubriker med 100 källrader**: 84 explicit nonstop, tio anslutningsförslag och sex genomgående resor med mellanstopp. Augusta–Columbia-rubriken har bara en genomgående rad och tillför ingen direktsträcka. Alla uteslutna rader bevaras i TSV-filen för granskning. Kompletta rubriker innebär inte att hela sidans eller stadens trafik är importerad.

Tidtabellens slutdatum och eventuella senare ändringsblad är inte verifierade. Uppgifterna gäller planerad trafik, inte belagda genomföranden, registreringsnummer eller verkliga flygbanor. Källan återges som transkriberade forskningsdata; original-PDF och sidbilder ingår inte i paketet.

## Tider, fotnoter och delsträckor

- Tomt frekvensfält betyder dagligen och skrivs `D` i råtranskriptionen. `X6` undantar lördag; `X7` undantar söndag. Veckodagarna tillhör respektive fysisk delsträcka.
- Stjärnan är en markering för lågtrafikpris, inte för ankomst nästa dag.
- Columbia–Atlanta **696 08:50–09:40** har fotnot **4**, som enligt förklaringen på tryckt sida 55 gäller från **12 februari 1986**. Den är tillämplig under importdagarna.
- Dallas/Fort Worth använder UTC−6; övriga berörda ändpunkter UTC−5. RDU–DFW 1007 och 1067 tar därför 2 timmar 56 minuter trots en lokal klockskillnad på 1 timme 56 minuter. Returerna tar 2 timmar 20 respektive 21 minuter.
- Charleston–Savannah **512 00:15–00:40** ligger efter föregående lokala dags Atlanta–Charleston **512 23:06–23:55**. Markuppehållet är 20 minuter. Båda schemaraderna anländer samma lokala kalenderdag som sin egen avgång.
- 308 har separat källbelagda ben ATL–CAE–CHS–ATL, med 20 respektive 25 minuters markuppehåll. 228 har ATL–SAV–CHS–ATL, med 25 minuter vid båda stoppen.
- De nya RDU–DFW-benen av 1007/1067 och DFW–RDU-benen av 1068/994 ansluter tidsmässigt till tidigare införda ORF–RDU/RDU–ORF-ben, med 20 minuters mellanrum. Genomgående raders sammanlagda restid används inte för att hitta på mellanliggande flygtider.

Lokala avgångsdatum 27 februari–1 mars prövas mot det exakta fönstret **1986-02-27 22:21:30 UTC till 1986-03-01 22:21:30 UTC**. En rörelse ingår om avgången är före sluttiden och ankomsten efter starttiden. Tiderna klipps inte vid fönstergränserna. Tio scheman ger tre överlappande rörelser, 67 ger två och sju ger en. Samtliga 84 scheman bidrar.

## Flygplatser

Katalogen använder historiska flygplatsnamn där de är kända. CHS avser Charleston i South Carolina, separat från befintliga CRW i West Virginia. CAE avser Columbia i South Carolina; MLB avser Melbourne i Florida. Savannah använder Travis Field, inte Hunter Field. DAB och MLB behåller tidtabellens Regional-beteckningar. Nyare terminaler eller namnbyten har inte flyttats tillbaka till 1986.

Flygplatsmyndigheternas historik och moderna koordinatkällor redovisas i `airport_locations_batch103.json`. Koordinaterna är ungefärliga markörer för respektive flygfält, inte exakta historiska terminal- eller gatepositioner.

## Kontroller och kvarvarande arbete

En separat manuellt inmatad transkription i 24-timmarsformat jämförs med råtidernas tolkning. En separat kalenderberäkning med fasta vintertidsförskjutningar stämmer med samtliga **171 nya UTC-intervall**. De berörda Delta-flygnumren har inga tidsöverlapp mellan nya och redan importerade rörelser. Tolv övergångar mellan separat tryckta delsträckor har kontrollerade markintervall; detta identifierar inte individuella flygplan.

Alla äldre rörelser, scheman, flygplatser, bolag och orelaterade källposter är bevarade. Rättelserna för COMAIR 1582 och 1580 samt den tidigare uteslutna RIC–ATL 1125-radens ankomst 21:49 bevaras. De tre äldre källkonflikterna (Delta 597 MEM–LIT, COMAIR 1871/1671 CMH–SDF och ASA 1412 HSV–ATL) ligger fortsatt utanför animationen.

Automatiska testresultat finns i `automated_checks_batch103.json`; detaljerad datakontroll i `validation_batch103.json`. Unreal-kompilering, Editor och paketerat spel har inte körts här. Paketet förutsätter den redan kompilerade PaintAirplane-rättelsen från `f23b73a`.

Nästa urval kan fortsätta med fler kompletta rubriker för Charleston, Charlotte, Columbia, Greensboro, Savannah och återstående Florida-städer; därefter Norfolk–JFK och nordöstra Delta/Ransome-nätet. Ingen global procentandel eller återstående totalsiffra kan ges: operatörsinventeringen och nätgranskningen är fortfarande ofullständiga.

## Revisionsunderlag

- `atlanta_southeast_batch103.tsv`: samtliga 100 källrader och importbeslut.
- `independent_clock_transcription_batch103.txt`: separat 24-timmarstranskription av de 84 godkända raderna.
- `atlanta_southeast_source_evidence_batch103.json`: sidreferenser, PDF-hash, teckenförklaring och fotnot.
- `airport_locations_batch103.json`: historisk platskontinuitet och koordinatkällor.
- `batch_103.json`: exakta nya ID:n och antal per sträcka.
- `validation_batch103.json`: bevarandekontroll, UTC-intervall och markuppehåll.
- `automated_checks_batch103.json`: resultat från befintliga tester och aktualitetskontroller.
- `worldwide_coverage_batch103.json`: bolagsvis täckningsstatus.
