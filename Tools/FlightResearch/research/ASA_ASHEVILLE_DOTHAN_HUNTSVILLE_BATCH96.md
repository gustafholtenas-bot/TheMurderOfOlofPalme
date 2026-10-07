# ASA Asheville, Dothan och Huntsville – omgång 96 / v99

V99 tillför **78 planerade flygrörelser från 38 nya nonstop-scheman** med Atlantic Southeast Airlines (ASA). Totalt finns **6 120 rörelser**, varav 6 119 tidtabellslagda och en tidigare dokumenterat genomförd. Alla 6 042 rörelser, 3 664 scheman, 270 flygplatser och 96 operatörer från v98 är oförändrade.

Tre flygplatser tillkommer: AVL, DHN och HSV. Databasen har nu 273 flygplatser/platser, 1 193 riktade ändpunktspar och 3 681 granskade scheman i kördatan. Katalogen innehåller totalt 3 702 scheman. ASA har 334 rörelser. Inget bolag är verifierat fullständigt.

## Nya sträckor

| Sträcka | Scheman | Rörelser i fönstret |
|---|---:|---:|
| Atlanta → Asheville | 4 | 8 |
| Asheville → Atlanta | 4 | 9 |
| Atlanta → Dothan | 7 | 13 |
| Dothan → Atlanta | 6 | 12 |
| Atlanta → Huntsville | 9 | 19 |
| Huntsville → Atlanta | 8 | 17 |
| **Summa** | **38** | **78** |

## Källgranskning och undantag

Primärkälla: [Delta Air Lines, giltig från 1 februari 1986](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201), [original-PDF](https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf).

Sex kompletta riktningsrubriker med 40 rader har granskats visuellt på tryckta sidor 12, 14, 15, 75 och 107. 38 individuellt tryckta nonstopben importeras. En genomgående rad med mellanlandning och en rad med motstridiga tider importeras inte. Originaltider, frekvens, sidnummer och beslut finns i `asa_asheville_dothan_huntsville_batch96.tsv`. PDF-hash och sidmappning finns i `asa_asheville_dothan_huntsville_source_evidence_batch96.json`. Originalskanningar ingår inte.

Triangeln och nummernyckeln på s. 4 identifierar ASA: 1200–1299 samt 1400–1524. Tomt frekvensfält betyder dagligen, X6 alla dagar utom lördag, X7 alla dagar utom söndag och X67 måndag till fredag. Alla accepterade rader saknar daterad fotnot. Utgåvans slutdatum och senare ändringar är inte verifierade.

**HSV–ATL 1412 hålls utanför.** Den förstorade direktflygsraden på s. 107 läses 08:00–08:00, X7. Huntsville är UTC−6 och Atlanta UTC−5, vilket ger negativ flygtid. På samma sida anges däremot avgång 06:00 i anslutningsraderna 1412/1201 till Albany och 1412/428 till Augusta, båda via Atlanta. 06:00 är en rimlig avsedd avgång, men den införs inte genom antagande. De två anslutningsraderna används endast som stöd för konfliktgranskningen. Se `source_conflicts_batch96.json`; ett ändringsblad eller annat tillförlitligt flygspecifikt underlag behövs för att lösa konflikten.

**DHN–ATL 1481 kl. 19:05–22:10 har ett stopp.** Ruttningsnyckeln på s. 264 anger ATL–DHN–PFN–ATL. Endast det separat tryckta nonstopbenet ATL–DHN kl. 18:45–18:55, X6, importeras nu. Mellanbenen via Panama City behöver egen tidsgranskning; inget direktflyg DHN–ATL 1481 skapas.

ATL–AVL 1219 har avgång **20:25**, inte 20:20. Flyg 1256 har **X67 i båda riktningarna**: ATL–DHN 12:05–12:10 och DHN–ATL 12:25–14:30. ATL–HSV 1413 och 1414 har X7, medan deras returben är dagliga. Frekvenser hämtas från varje bens egen rad.

## Historiska flygplatser

Tidtabellens register på s. 260 anger följande fält:

| Kod | Periodens namn | Historisk lägeskontroll |
|---|---|---|
| AVL | Asheville Regional | [Flygplatsens historik](https://flyavl.com/airport-authority/history) beskriver det nya regionalfältet från 1961, som ersatte ett tidigare fält. |
| DHN | Dothan–Houston County | [Flygplatsens historik](https://flydothan.com/history/) bekräftar att den civila trafiken flyttade till tidigare Napier Army Airfield 1965. Det äldre stadsfältet vid nuvarande Westgate Park används inte. |
| HSV | Huntsville–Madison County | [Port of Huntsvilles historik](https://www.flyhuntsville.com/about/history/) anger det nuvarande flygplatsområdets öppnande 1967. |

Moderna koordinater från [OurAirports AVL](https://ourairports.com/airports/KAVL/), [DHN](https://ourairports.com/airports/KDHN/) och [HSV](https://ourairports.com/airports/KHSV/) används som ungefärliga markörer för flygplatsområdena. Historiska terminaler, gater och exakta banlägen rekonstrueras inte. Se `airport_locations_batch96.json`.

## Kalender och validering

Atlanta och Asheville använder vintertid UTC−5; Dothan och Huntsville UTC−6. Alla accepterade ankomster är samma lokala kalenderdag. De identiska västgående klockslagen ATL–HSV innebär en timmes flygtid efter tidszonsomräkning. Flera ATL–DHN-rader har fem minuters skillnad mellan tryckta lokaltider men motsvarar 65 minuters flygtid.

Separat transkription till 24-timmarstid och oberoende beräkning med fasta vinteroffsetar matchar alla 78 genererade rörelser. Fönstret är **27 februari 22:21:30 UTC till 1 mars 22:21:30 UTC 1986**; fullständiga flygtider behålls vid överlapp.

Fyra scheman överlappar torsdagsstarten och ger tre rörelser vardera: AVL–ATL 1217, DHN–ATL 1258, HSV–ATL 1418 och ATL–HSV 1419. De båda X67-benen med nummer 1256 ger bara fredagens rörelse. Övriga 32 scheman ger två. Fördelning per lokalt avgångsdatum: torsdag 13, fredag 38, lördag 27.

Sexton övergångar mellan separat källbelagda ben med samma operatör och flygnummer har kontrollerade markuppehåll på 15 eller 20 minuter. Detta belägger inte individflygplan. Inga tidsöverlappningar upptäcktes för berörda operatör/flygnummer.

Alla 20 Python-tester och aktualitetskontroller passerar. Körresultaten redovisas i `automated_checks_batch96.json`. Detaljkontroller finns i `validation_batch96.json`. Tidigare COMAIR-rättelser och hållna källkonflikter för Delta 597 MEM–LIT respektive 1871/1671 CMH–SDF är bevarade. UE 5.8-kompilering, spelkörning och paketerad build har inte utförts här.

Paketet innehåller hela databasen och forskningshistoriken. Befintlig Flight-meny och kompilerad PaintAirplane-rättelse från `f23b73a` krävs. Inga C++-filer ingår.

Nästa prioritet: Atlanta till/från Augusta, Columbus GA och Macon; ASA Panama City inklusive de separata DHN–PFN–ATL-benen med nummer 1481. HSV–ATL 1412 kräver separat konfliktlösning.
