# Panama City och Greenville–Spartanburg – omgång 98 / v101

V101 tillför **55 planerade flygrörelser från 28 nya nonstop-scheman**: 15 rörelser med Delta Air Lines och 40 med Atlantic Southeast Airlines (ASA). Totalt finns **6 292 rörelser**, varav 6 291 tidtabellslagda och en tidigare dokumenterat genomförd. Alla 6 237 rörelser, 3 768 scheman, 276 flygplatser och 96 operatörer från v100 är oförändrade.

Panama City–Bay County (PFN) och Greenville–Spartanburg (GSP) tillkommer. Databasen har nu 278 flygplatser/platser och 1 207 riktade ändpunktspar. Katalogen innehåller 3 796 scheman, varav 3 775 granskade. Delta har 1 737 rörelser och ASA 447. Inget bolag är verifierat fullständigt.

## Nya sträckor

| Sträcka | Nya nonstop-scheman | Rörelser i fönstret |
|---|---:|---:|
| Atlanta → Panama City | 2 | 5 |
| Panama City → Atlanta | 3 | 6 |
| Dothan → Panama City | 2 | 4 |
| Panama City → Dothan | 1 | 2 |
| Atlanta → Greenville–Spartanburg | 10 | 19 |
| Greenville–Spartanburg → Atlanta | 10 | 19 |
| **Summa** | **28** | **55** |

Alla 28 scheman ger minst en rörelse under perioden.

## Källgranskning

Primärkälla: [Delta Air Lines, giltig från 1 februari 1986](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201), [original-PDF](https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf).

Sex kompletta riktningsrubriker med 31 rader har granskats visuellt på tryckta sidor 15, 17, 77, 97, 185 och 186. 28 individuellt tryckta nonstopben importeras: åtta Delta-scheman och 20 ASA-scheman. Tre genomgående resor med mellanlandning importeras inte som egna nonstopflyg. Inga anslutningsrader förekommer inom dessa sex rubriker. Originaltider, frekvens, sidnummer och beslut finns i `panama_greenville_delta_asa_batch98.tsv`. PDF-hash och sidmappning finns i `panama_greenville_delta_asa_source_evidence_batch98.json`. Originalskanningar ingår inte.

Triangeln och operatörsnyckeln på s. 4 identifierar ASA:s nummer 1200–1299 samt 1400–1524. Övriga accepterade nummer tillhör Deltas huvudlinjer. Tomt frekvensfält betyder dagligen; X6 utesluter lördag och X7 söndag. En ensam 6 betyder endast lördag. Varje ben behåller sina egna trafikdagar. Ingen accepterad rad har daterad fotnot. Utgåvans slutdatum och senare ändringar är inte verifierade.

## Delsträckor och markuppehåll

ASA 1481 är **ATL–DHN–PFN–ATL**, enligt ruttningsnyckeln på s. 264. Det tidigare benet Atlanta–Dothan 18:45–18:55 Central behålls. De nya, separat tryckta benen är Dothan–Panama City **19:05–19:35** och Panama City–Atlanta **19:50–22:10 Eastern**. Markuppehållen är tio respektive femton minuter. Alla tre ben är X6. Genomgående ATL–PFN och DHN–ATL skapar inga extra nonstopflyg.

ASA 1253 går Panama City–Dothan **06:45–07:15** och fortsätter med det befintliga Dothan–Atlanta-benet 07:25. ASA 1260 anländer Dothan 20:35 Central med det befintliga benet från Atlanta; det nya Dothan–Panama City-benet går **20:45–21:15**. Båda markuppehållen är tio minuter. PFN–ATL 1253 är en genomgående rad och importeras inte som nonstop.

ATL–PFN 1478 går **11:47–12:07 Central**, med returavgång 12:20: tretton minuters markuppehåll. ATL–PFN 1480 går **16:50–17:05 Central**, med returavgång 17:20. Förstoringen bekräftar ankomst 17:05.

ATL–GSP 1410 är en lördagsrad **11:47–12:52**; returbenet är också endast lördag, **13:05–14:10**, med tretton minuters markuppehåll. Fyra andra ASA-vändningar vid GSP har femton minuter mellan benen. Detta belägger inte individflygplan.

## Kalender och tidsfönster

Atlanta och Greenville–Spartanburg använder vintertid UTC−5; Dothan och Panama City UTC−6. Alla nya accepterade scheman anländer samma lokala kalenderdag. ATL–GSP 1409 kl. **22:55–23:55** får båda UTC-tiderna nästa kalenderdag: 03:55–04:55.

Lokala avgångsdatum 27 februari–1 mars 1986 prövas mot **27 februari 22:21:30 UTC till 1 mars 22:21:30 UTC**. Hela flygets tider behålls vid överlapp.

Tre scheman ger tre rörelser genom överlapp vid båda fönstergränserna: ATL–PFN 1480, ATL–GSP 1407 och GSP–ATL 833. 21 scheman ger två rörelser och fyra ger en. Fördelningen per lokalt avgångsdatum är torsdag 12, fredag 26, lördag 17.

De två 1410-benen ger varsin lördagsrörelse. Delta 1150 ATL–GSP och 1161 GSP–ATL är X6 och ligger före torsdagens startgräns, så de ger en fredagsrörelse vardera. Skillnader mellan ut- och returfrekvenser bevaras: ATL–GSP 1405 är X7 men GSP–ATL 1405 daglig; ATL–GSP 1408 är daglig men GSP–ATL 1408 X6.

## Flygplatser

| Kod | Historiskt fält | Lägesunderlag |
|---|---|---|
| PFN | Panama City–Bay County | Registret s. 261 anger Bay County. [Flygplatsmyndigheten](https://www.iflybeaches.com/documents/a-decade-of-growth-for-ecp-op-ed-written-by-del-lee-chairman-of-the-board) bekräftar att ECP ersatte PFN på en ny plats 23 maj 2010. [OurAirports arkiverade fältpost](https://ourairports.com/airports/US-9348/) identifierar gamla PFN/KPFN och ger koordinater 30.212099, −85.682800. |
| GSP | Greenville–Spartanburg | Registret s. 260 anger Greenville/Spartanburg. [Flygplatsens historik](https://gspairport.com/our-history/) beskriver det gemensamma fältets öppnande vid Flatwood 1962. [OurAirports](https://ourairports.com/airports/KGSP/) ger koordinater 34.895699, −82.218903. |

PFN-markören ligger vid det gamla fältet, inte moderna ECP. GSP avser det gemensamma fältet nära Greer, inte Greenville Downtown, Spartanburg Downtown eller Greenville i Mississippi. Koordinaterna är ungefärliga markörer för flygplatsområdena; terminaler, gater och exakta banlägen 1986 rekonstrueras inte. Se `airport_locations_batch98.json`.

## Validering och nästa steg

Separat transkription till 24-timmarstid och oberoende beräkning med fasta vinteroffsetar, trafikdagar och dygnsregler matchar alla 55 nya rörelser. Elva övergångar mellan separat källbelagda ben med samma operatör och flygnummer har kontrollerade markuppehåll på 10–15 minuter, tio på fredagen och 1410 på lördagen. Inga tidsöverlappningar upptäcktes för berörda operatör/flygnummer.

Alla 20 Python-tester och aktualitetskontroller passerar. Körresultaten redovisas i `automated_checks_batch98.json`. Detaljer finns i `validation_batch98.json`. Alla äldre dataobjekt och rättelser bevaras. De hållna konflikterna Delta 597 MEM–LIT, 1871/1671 CMH–SDF och ASA 1412 HSV–ATL är fortsatt olösta.

UE 5.8-kompilering, spelkörning och paketerad build har inte utförts här. Paketet innehåller hela databasen och forskningshistoriken. Befintlig Flight-meny och kompilerad PaintAirplane-rättelse från `f23b73a` krävs; inga C++-filer ingår.

Nästa prioritet: Atlanta-linjer till/från Anniston, Gadsden, Tri-City, Roanoke och Tallahassee, med kontroll av individuella ben och historiska flygplatser.
