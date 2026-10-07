# ASA Fayetteville, Georgia och Florida – omgång 95 / v98

V98 tillför **70 planerade flygrörelser från 34 nya nonstop-scheman** med Atlantic Southeast Airlines (ASA). Totalt finns **6 042 rörelser**, varav 6 041 tidtabellslagda och en tidigare dokumenterat genomförd. Alla 5 972 rörelser, 3 630 scheman, 266 flygplatser och 96 operatörer från v97 är oförändrade.

Fyra flygplatser tillkommer: FYV, ABY, BQK och GNV. Databasen har nu 270 flygplatser/platser, 1 187 riktade ändpunktspar och 3 643 granskade scheman i kördatan. Katalogen innehåller totalt 3 664 scheman. ASA har 256 rörelser. Inget bolag är verifierat fullständigt.

## Nya sträckor

| Sträcka | Scheman | Rörelser i fönstret |
|---|---:|---:|
| Memphis → Fayetteville, Arkansas | 3 | 7 |
| Fayetteville, Arkansas → Memphis | 3 | 6 |
| Atlanta → Albany, Georgia | 6 | 12 |
| Albany, Georgia → Atlanta | 6 | 11 |
| Atlanta → Brunswick, Georgia | 4 | 8 |
| Brunswick, Georgia → Atlanta | 4 | 9 |
| Atlanta → Gainesville, Florida | 4 | 9 |
| Gainesville, Florida → Atlanta | 4 | 8 |
| **Summa** | **34** | **70** |

## Källgranskning

Primärkälla: [Delta Air Lines, giltig från 1 februari 1986](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201), [original-PDF](https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf).

Åtta kompletta riktningsrubriker med 34 rader har granskats visuellt på tryckta sidor 6, 14, 15, 34, 79, 93 och 146. Samtliga rader har stoppkolumn 0 och importeras som individuellt tryckta nonstopben. Inga anslutningsförslag, genomgående stoppresor eller daterade fotnoter finns i dessa åtta rubriker. Flygnummer, originaltider, frekvens och sidnummer finns i `asa_fayetteville_georgia_florida_batch95.tsv`. PDF-hash och sidmappning finns i `asa_fayetteville_georgia_florida_source_evidence_batch95.json`. Originalskanningar ingår inte.

Triangeln och nummernyckeln på s. 4 identifierar ASA: 1200–1299 samt 1400–1524. Tomt frekvensfält betyder dagligen, X6 alla dagar utom lördag och X7 alla dagar utom söndag. Utgåvans slutdatum och senare ändringar är inte verifierade.

Varje ben behåller sin egen frekvens. ATL–ABY 1205 och ATL–GNV 1294 är dagliga, men deras returben har X6. ATL–ABY 1203 och ABY–ATL 1203 har båda X6. Dagliga returben antas inte från utresan.

Förstorade originalrader bekräftar ATL–BQK 1230 kl. **15:16–16:30**, ATL–BQK 1231 kl. **20:30–21:50** och ATL–BQK 1229 kl. **11:47–13:07**. Retur 1229 lämnar Brunswick 13:20. MEM–FYV 1267 går 15:30–16:45, retur FYV–MEM 1267 kl. 17:00–18:15. Ruttningsnyckeln på s. 264 överensstämmer med de separat granskade benen.

## Flygplatser och lägeskontroll

Tidtabellens register på s. 260 skiljer de fyra flygplatserna från andra orter med liknande namn.

| Kod | Periodens flygplats | Underlag för lägesbedömningen |
|---|---|---|
| FYV | Fayetteville, Arkansas – Drake Field | [Stadens historik](https://www.fayetteville-ar.gov/668/About) anger nuvarande flygfältsläge sedan 1936 och tidigare kommersiell trafik. FYV används, inte FAY i North Carolina eller senare XNA. |
| ABY | Albany, Georgia – Dougherty County | Periodregistret anger ABY och Dougherty County. [Stadens aktuella flygplatssida](https://airport.albanyga.gov/Airport-Guide/About-ABY) identifierar KABY som Southwest Georgia Regional vid Newton Road. Kopplingen till dagens fält är en identitetsbedömning utifrån dessa uppgifter; en exakt historisk terminalposition är inte belagd. |
| BQK | Brunswick, Georgia – Glynco Jet Port | [Flygplatskommissionens historik](https://flygcairports.com/glynn-county-airport-commission/history/) bekräftar civilt bruk från 1975 och ASA:s Atlanta-trafik från 1981. Markören gäller fastlandsfältet, inte McKinnon/SSI. Namnet Golden Isles infördes senare. |
| GNV | Gainesville, Florida – Regional | [Flygplatsens historik](https://gra-gnv.com/flygainesville.com/airport-history/) beskriver stadens övertagande 1948 och Regional-namnet från 1977. Florida-fältet används, inte Gainesville i Georgia eller Texas. |

Koordinater från OurAirports: [FYV](https://ourairports.com/airports/KFYV/), [ABY](https://ourairports.com/airports/KABY/), [BQK](https://ourairports.com/airports/KBQK/), [GNV](https://ourairports.com/airports/KGNV/). Dessa är moderna, ungefärliga flygplatsmarkörer. Terminaler, gater och exakta banlägen 1986 rekonstrueras inte. Se `airport_locations_batch95.json`.

## Kalender och validering

Atlanta, Albany, Brunswick och Gainesville använder vintertid UTC−5; Memphis och Fayetteville UTC−6. Alla accepterade ankomster är samma lokala kalenderdag. Separat transkription till 24-timmarstid och oberoende beräkning med fasta vinteroffsetar matchar alla 70 genererade rörelser.

Fönstret är **27 februari 22:21:30 UTC till 1 mars 22:21:30 UTC 1986**. Hela flygets tider behålls om intervallet överlappar fönstret. Fyra dagliga scheman överlappar torsdagsstarten och ger tre rörelser vardera: MEM–FYV 1267, ATL–ABY 1204, BQK–ATL 1230 och ATL–GNV 1294. De två X6-benen med nummer 1203 avslutas före torsdagsstarten och går inte på lördagen; de ger bara fredagens rörelse. Övriga 28 scheman ger två rörelser. Fördelning per lokalt avgångsdatum: torsdag 13, fredag 34, lördag 23.

Tretton övergångar mellan separat källbelagda ben med samma operatör och flygnummer har kontrollerade markuppehåll på 10–130 minuter. MEM–FYV–MEM 1265 har 130 minuter i Fayetteville; övriga ligger på 10–15 minuter. Det belägger inte individflygplan. Inga tidsöverlappningar upptäcktes för berörda operatör/flygnummer.

Alla 20 Python-tester och aktualitetskontroller passerar. Körresultaten redovisas i `automated_checks_batch95.json`. Detaljkontroller finns i `validation_batch95.json`. Tidigare COMAIR-rättelser och hållna källkonflikter för Delta 597 MEM–LIT respektive 1871/1671 CMH–SDF är bevarade. UE 5.8-kompilering, spelkörning och paketerad build har inte utförts här.

Paketet innehåller hela databasen och forskningshistoriken. Befintlig Flight-meny och kompilerad PaintAirplane-rättelse från `f23b73a` krävs. Inga C++-filer ingår.

Nästa prioritet: ASA Atlanta till/från Dothan, Huntsville, Augusta, Columbus GA, Asheville och Macon. Fortsätt med kompletta riktningsrubriker, självständiga frekvenser och kontroll av historiska flygplatser.
