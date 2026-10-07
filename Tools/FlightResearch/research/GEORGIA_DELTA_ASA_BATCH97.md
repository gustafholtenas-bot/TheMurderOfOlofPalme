# Georgia: Delta och ASA – omgång 97 / v100

V100 tillför **117 planerade flygrörelser från 66 nya nonstop-scheman**: 44 rörelser med Delta Air Lines och 73 med Atlantic Southeast Airlines (ASA). Totalt finns **6 237 rörelser**, varav 6 236 tidtabellslagda och en tidigare dokumenterat genomförd. Alla 6 120 rörelser, 3 702 scheman, 273 flygplatser och 96 operatörer från v99 är oförändrade.

Augusta–Bush Field (AGS), Columbus Metropolitan i Georgia (CSG) och Macon Municipal (MCN) tillkommer. Databasen har nu 276 flygplatser/platser och 1 201 riktade ändpunktspar. Katalogen innehåller 3 768 scheman, varav 3 747 granskade. Delta har 1 722 rörelser och ASA 407. Inget bolag är verifierat fullständigt.

## Nya sträckor

| Sträcka | Granskade nonstop-scheman | Rörelser i fönstret |
|---|---:|---:|
| Atlanta → Augusta | 11 | 20 |
| Augusta → Atlanta | 10 | 18 |
| Atlanta → Columbus, Georgia | 10 | 19 |
| Columbus, Georgia → Atlanta | 10 | 19 |
| Atlanta → Macon | 11 | 20 |
| Macon → Atlanta | 12 | 17 |
| Columbus, Georgia → Macon | 1 | 2 |
| Macon → Columbus, Georgia | 1 | 2 |
| **Summa** | **66** | **117** |

64 av de 66 nya schemana ger minst en rörelse i det aktuella tidsfönstret. Två granskade scheman finns kvar med sina källbelagda veckodagar men utan animation under perioden.

## Källgranskning

Primärkälla: [Delta Air Lines, giltig från 1 februari 1986](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201), [original-PDF](https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf).

Åtta kompletta riktningsrubriker med 85 rader har granskats visuellt på tryckta sidor 14, 16, 18, 57, 58, 141 och 142. 66 individuellt tryckta nonstopben importeras: 25 Delta-scheman och 41 ASA-scheman. 17 anslutningsförslag och två genomgående resor med mellanlandning importeras inte som egna direktflyg. Originaltider, frekvens, sidnummer och beslut finns i `georgia_delta_asa_batch97.tsv`. PDF-hash och sidmappning finns i `georgia_delta_asa_source_evidence_batch97.json`. Originalskanningar ingår inte.

Triangeln och operatörsnyckeln på s. 4 identifierar ASA:s nummer 1200–1299 samt 1400–1524. Övriga accepterade nummer tillhör Deltas huvudlinjer. Tomt frekvensfält betyder dagligen; X6 utesluter lördag, X7 söndag och X67 båda helgdagarna. En ensam 6 betyder endast lördag och en ensam 7 endast söndag. Varje ben behåller sina egna trafikdagar. Ingen accepterad rad har daterad fotnot. Tidtabellsutgåvans slutdatum och senare ändringar är inte verifierade.

## Mellanlandning och midnatt

Genomgående **ATL–MCN 1238** går via Columbus. De separat tryckta benen är ATL–CSG **22:56–23:36** och CSG–MCN **23:45–00:15 nästa dag**, båda X6. Markuppehållet är nio minuter. Inget extra ATL–MCN-direktflyg med nummer 1238 skapas.

Morgonresan **MCN–ATL 1233** går också via Columbus. De egna nonstopraderna är MCN–CSG **04:45–05:15** och CSG–ATL **05:30–06:09**, båda X7. Markuppehållet är 15 minuter. Ruttningsnyckeln på s. 264 bekräftar båda kedjorna.

ATL–MCN 1436 går **15:16–15:56**, och dess retur lämnar Macon 16:10: 14 minuters markuppehåll. Förstorade originalrader bekräftar minuterna. Tryckt `1200n` betyder 12:00 mitt på dagen. Stjärnan på ATL–AGS 312 anger lågtrafikpris enligt s. 4, inte ankomst nästa dag.

## Kalender och tidsfönster

Alla fyra flygplatser använder vintertid UTC−5. Endast CSG–MCN 1238 anländer nästa lokala kalenderdag bland de nya accepterade benen. Lokala avgångsdatum 27 februari–1 mars 1986 prövas mot **27 februari 22:21:30 UTC till 1 mars 22:21:30 UTC**. Hela flygets tider behålls vid överlapp.

Två nya scheman ger ingen rörelse:

- MCN–ATL 1434 kl. 09:00–09:35 går endast söndagar; ingen söndag ingår.
- AGS–ATL 1225 kl. 18:05–19:05 går endast lördagar men börjar efter fönstrets slut. Utresan ATL–AGS 1225 kl. 16:50–17:50 samma lördag överlappar slutgränsen och finns med.

Fyra scheman ger tre rörelser vardera genom att även överlappa torsdagsstarten: ATL–CSG 1236, ATL–MCN 1437, AGS–ATL 1105 och CSG–ATL 634. 45 scheman ger två rörelser, 15 ger en och två ger ingen. Fördelning per lokalt avgångsdatum: torsdag 21, fredag 61, lördag 35.

ATL–AGS/AGS–ATL 1225 och ATL–CSG/CSG–ATL 1232 är lördagsrader. Skillnader mellan ut- och returdagar behålls, exempelvis ATL–MCN 1433 X67 mot MCN–ATL 1433 X6, samt ATL–MCN 1436 X7 mot MCN–ATL 1436 X6.

## Flygplatser

| Kod | Namn i perioden | Lägesunderlag |
|---|---|---|
| AGS | Augusta–Bush Field | Registret s. 260 anger Bush Field. [Flygplatsens historik](https://flyags.com/business/about/history/) kopplar fältet till 1941 och daterar Regional-namnet till 2000. Markören gäller inte Daniel Field. |
| CSG | Columbus Metropolitan, Georgia | Registret s. 260 anger Columbus Metro. [Flygplatskommissionen](https://www.flycolumbusga.com/about-csg/our-history-columbus-ga-airport/) beskriver Delta-trafik från 1968 och en senare terminal från 1991. Columbus i Ohio och Mississippi är andra flygplatser. |
| MCN | Macon Municipal | Registret s. 261 anger Municipal. [Flygplatsens historik](https://iflymacon.com/about-us/) kopplar dagens Middle Georgia Regional till Cochran Field och efterkrigstidens kommunala trafikflygplats. Det separata downtownfältet används inte. |

Moderna koordinater från [OurAirports AGS](https://ourairports.com/airports/KAGS/), [CSG](https://ourairports.com/airports/KCSG/) och [MCN](https://ourairports.com/airports/KMCN/) används som ungefärliga markörer för flygplatsområdena. Terminaler, gater och exakta banlägen 1986 rekonstrueras inte. Se `airport_locations_batch97.json`.

## Validering och nästa steg

Separat transkription till 24-timmarstid och oberoende beräkning med fasta vinteroffsetar, trafikdagar och uttryckligt dygnsskifte matchar alla 117 genererade rörelser. Femton övergångar med samma operatör och flygnummer har kontrollerade markuppehåll på 9–60 minuter: fjorton på fredagen och 1232-kedjan på lördagen. Det belägger inte individflygplan. Inga tidsöverlappningar upptäcktes för berörda operatör/flygnummer.

Alla 20 Python-tester och aktualitetskontroller passerar. Körresultaten redovisas i `automated_checks_batch97.json`. Detaljer finns i `validation_batch97.json`. Alla äldre dataobjekt och rättelser bevaras. De hållna konflikterna Delta 597 MEM–LIT, 1871/1671 CMH–SDF och ASA 1412 HSV–ATL är fortsatt olösta. Att flyg 428 ATL–AGS nu finns med löser inte den motstridiga avgången för det separata benet HSV–ATL 1412.

UE 5.8-kompilering, spelkörning och paketerad build har inte utförts här. Paketet innehåller hela databasen och forskningshistoriken. Befintlig Flight-meny och kompilerad PaintAirplane-rättelse från `f23b73a` krävs; inga C++-filer ingår.

Nästa prioritet: ASA Panama City och separata DHN–PFN–ATL-ben med nummer 1481, därefter fler Atlanta-linjer såsom Greenville–Spartanburg, Anniston och Gadsden.
