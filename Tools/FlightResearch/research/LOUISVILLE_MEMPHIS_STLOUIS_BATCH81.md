# Louisville, Memphis och St Louis – v84 / forskningsomgång 81

64 nya planerade flygrörelser: 52 Delta och 12 COMAIR. Databasen innehåller nu **5 223 rörelser**, varav 5 222 enligt tidtabell och en tidigare dokumenterat genomförd. Alla 5 159 rörelseposter och 3 217 scheman från v83 är oförändrade.

| Förbindelse | Första riktningen | Returriktningen | Nya rörelser totalt |
|---|---:|---:|---:|
| Memphis–St Louis | 6 | 6 | 12 |
| Memphis–Louisville | 2 | 2 | 4 |
| Louisville–Cincinnati | 15 | 9 | 24 |
| Louisville–Atlanta | 11 | 13 | 24 |
| **Totalt** | **34** | **30** | **64** |

Louisville–Standiford Field (SDF) tillkommer som flygplats. Inga nya operatörer registreras.

## Källgranskning

Delta Air Lines System Timetable, effective February 1, 1986. Delta Flight Museum / Digital Library of Georgia.

- Arkivpost: https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201
- Original-PDF: https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf
- Nya scheman: tryckta s. 16, 51, 137–139, 147, 148 och 207.
- Omkontroll av befintliga St Louis–Cincinnati-scheman: tryckta s. 51 och 206.
- Trafikdagar och operatörsnyckel: s. 4. Flygnummer och linjeföljder: s. 262–265.

58 rader i tio fullständiga riktningstabeller granskades visuellt. 37 nya nonstop-scheman godtas. Sex tidigare COMAIR-scheman mellan Cincinnati och St Louis stämmer i tid och trafikdagar med originalet och importeras inte igen. Femton anslutningsförslag utesluts som direkta fysiska flygsträckor. Returer läses ur sina egna tabeller. Originalskanningen omdistribueras inte.

COMAIR identifieras genom originalets operatörsnyckel för flygnummer 1525–1749. Deltas huvudlinjer och COMAIR hålls isär. Alla accepterade rader har noll stopp och saknar daterade fotnoter.

## Trafikdagar och tider

Tomt frekvensfält betyder dagligen. X6 undantar lördag; X67 betyder måndag–fredag. Siffrorna 5, 6 och 7 betyder enbart fredag, lördag respektive söndag.

COMAIR 1608 och 1693 från Cincinnati till Louisville går enbart söndagar. De sparas som granskade scheman men ger inga rörelser under torsdag–lördag i spelfönstret. Delta 639 St Louis–Memphis går enbart lördag och ger en rörelse. COMAIR 1717 Louisville–Cincinnati går enbart fredag och ger en rörelse.

Memphis och St Louis använder UTC−6; Louisville, Cincinnati och Atlanta använder UTC−5. Alla accepterade ankomster är samma lokala kalenderdag. Louisville–Memphis 1101 anges 09:00–09:03 lokalt men tar 63 minuter på grund av tidszonsbytet.

Det exakta fönstret är 1986-02-27 22:21:30 UTC till 1986-03-01 22:21:30 UTC. Hela flygintervallet sparas när det överlappar fönstret. Nya lokala avgångar: 27 februari 14, 28 februari 34, 1 mars 16. Atlanta–Louisville 254 och Louisville–Atlanta 493 överlappar fönstergränserna och ger tre rörelser vardera.

Memphis–St Louis har kontrollerats i förstoring: 670 är 12:25–13:20, 1166 är 15:20–16:15 och 638 är 18:55–19:50. Dessa tider stämmer även med de separat källbelagda inkommande benen via Memphis.

## Kontrollerade delsträckor

Följande övergångar mellan separat källbelagda ben med samma flygnummer kontrollerades för 28 februari:

| Operatör/flyg | Separata ben | Markuppehåll |
|---|---|---:|
| Delta 670 | Houston–Memphis–St Louis | 33 minuter |
| Delta 1166 | Little Rock–Memphis–St Louis | 27 minuter |
| Delta 638 | New Orleans–Memphis–St Louis | 27 minuter |
| Delta 1109 | St Louis–Memphis–Little Rock | 31 minuter |
| Delta 663 | St Louis–Memphis–Houston | 34 minuter |
| Delta 1155 | St Louis–Memphis–Jackson | 37 minuter |
| Delta 1170 | Jackson–Memphis–Louisville–Cincinnati | 37 och 20 minuter |
| Delta 1101 | Cincinnati–Louisville–Memphis–Houston | 21 och 27 minuter |
| Delta 318 | Atlanta–Louisville–Cincinnati | 20 minuter |
| Delta 392 | Atlanta–Louisville–Cincinnati | 25 minuter |
| Delta 254 | Atlanta–Louisville–Cincinnati | 35 minuter |
| COMAIR 1733 | Louisville–Cincinnati–Louisville | 15 minuter |

Sammanlagt fjorton övergångar. COMAIR 1733:s utgående ben har X67, medan returen har X6; respektive rads frekvens bevaras. Originalets linjeföljd anger uttryckligen SDF–CVG–SDF. Samma flygnummer belägger inte ett bestämt individflygplan.

## Louisville på kartan

1986 års namn **Louisville–Standiford Field** används. Flygplatsens egen historik anger att den kommersiella trafiken flyttade från Bowman Field 1947, att en ny landsideterminal öppnade 1985 och att namnet Louisville International infördes 1995.

- Historik: https://www.flylouisville.com/corporate/sdf-history/
- Ungefärlig position: 38.170600, −85.735076, https://ourairports.com/airports/KSDF/

Den moderna koordinaten används endast som kartmarkör för samma flygplatsområde. Senare banombyggnader innebär att den inte representerar exakt bana, terminal eller flygplansposition från 1986. Se `airport_locations_batch81.json`.

## Validering och begränsningar

20 befintliga Python-tester passerar. Genererad flyg-JSON, landindex och Europaindex är kontrollerade. Separat transkriberade 24-timmarstider, fasta UTC-offsetar och explicita veckodagar ger exakt samma 64 nya rörelser som kompilatorn med IANA-tidszoner. Alla tidigare rörelseobjekt och schemarader är oförändrade. Inga nya dubbletter eller tidsöverlapp mellan berörda ben med samma operatör/flygnummer.

Full UE 5.8-kompilering, spelkörning och paketerad build har inte utförts här. Detta är ett kumulativt datapaket för befintlig flygmeny med tidigare kompilerad kraschfix f23b73a; C++-filer och byggfiler ingår inte.

Tidtabellen visar planerad trafik. Genomförande, förseningar, passagerare, last, individflygplan och faktisk flygbana kräver annat underlag. Senare tidtabellsändringar och utgåvans slutdatum är inte verifierade. Den tidigare motsägande raden Memphis–Little Rock 597 förblir utanför importen.

## Täckning och nästa urval

245 flygplatser/platser och 1 056 riktade platspar. Delta har 1 229 rörelser, COMAIR 267. Totalt 95 registrerade operatörer: 49 med rörelser och 46 utan. Inget bolag är verifierat fullständigt; ingen global täckningsprocent kan anges.

Nästa urval kan omfatta Louisville–Chicago/Detroit/Columbus/Lexington, saknade delsträckor via Jackson och Shreveport samt Memphis–New York LaGuardia. Europakön behåller Cypern.

Underlag: `louisville_memphis_stl_batch81.tsv`, `louisville_memphis_stl_source_evidence_batch81.json`, `batch_81.json`, `validation_batch81.json` och `WORLDWIDE_COVERAGE_BATCH81.md`.
