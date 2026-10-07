# Northwest – Mellanvästern, v117 / batch114

Granskat 1 oktober 2026. Kumulativ uppdatering från v116.

114 nya planerade Northwest-rörelser från 58 granskade nonstop-scheman mellan Minneapolis, Chicago O’Hare, Detroit och Milwaukee. 12 riktade par berörs, varav fyra nya i databasen. Totalt **8 303 rörelser**, varav 8 302 tidtabellslagda och en dokumenterat genomförd. Alla 8 189 äldre rörelseobjekt och 4 780 äldre scheman är oförändrade. Se `research/NORTHWEST_MIDWEST_BATCH114.md`.

## Sträckor

Scheman är återkommande källrader; rörelser är daterade flygintervall i spelfönstret. 57 av 58 scheman ger rörelser.

| Riktad sträcka | Nya scheman | Nya rörelser |
|---|---:|---:|
| DTW → MKE | 5 | 11 |
| DTW → MSP | 3 | 7 |
| DTW → ORD | 3 | 7 |
| MKE → DTW | 6 | 10 |
| MKE → MSP | 5 | 8 |
| MKE → ORD | 2 | 4 |
| MSP → DTW | 4 | 8 |
| MSP → MKE | 4 | 8 |
| MSP → ORD | 10 | 20 |
| ORD → DTW | 3 | 6 |
| ORD → MKE | 3 | 5 |
| ORD → MSP | 10 | 20 |
| **Totalt** | **58** | **114** |

## Källgranskning

[Northwest Orient, 18 december 1985, Digital Library of Georgia](https://dlg.usg.edu/record/delta_nwa-tt_nwa-tt-19851218). Valda nonstop-rader har granskats visuellt på tryckta **s.21,22,29,30,74,75,77,79**, motsvarande **PDF-s.12,13,16,17,39,40,41,42**. Färdvägslistan s.142/PDF74 och förklaringar s.150/PDF78 har också granskats. PDF-filens SHA256 och alla råfält finns i `northwest_source_evidence_batch114.json` och `northwest_midwest_batch114.tsv`. Originalskanningen ingår inte i ZIP.

**N/S betyder nonstop.** Daily är dagligen, ExSu utom söndag, ExSa utom lördag, ExSaSu utom lördag/söndag, Sa lördag och Su söndag. Flygnumren i urvalet tillhör Northwest och ligger under Airlink-serierna. O markerar O’Hare; **Midway-opererade MDW-rader importeras inte som Northwest**. Befintliga DTW–MSP7 09:40–10:18 behålls utan kopia.

**ORD–MKE707 och MKE–ORD706** förekommer i tre daterade varianter. Endast raden med daglig trafik **från 12 februari** används. Varianterna till 7 januari och 8 januari–11 februari gäller inte spelfönstret.

**MKE–DTW38** har en separat lördagsrad på s.74; **DTW–MKE49** har en daglig rad på s.30. Ingen av dessa inrikesrader har datumfotnot. De separata inrikesbenen och frekvenserna bekräftas på s.142 med Boeing 727. Atlanttabellens uppehåll gäller de tidigare importerade internationella raderna och utvidgas inte till dessa inrikesben. Äldre internationella scheman och deras undantag förblir oförändrade.

MKE–MSP375 och MSP–MKE224 har mellanlandning i Madison och importeras inte som nonstop. MSP–DTW458 har ett stopp i Chicago; nu importeras dess **separat tidsatta MSP–ORD och ORD–DTW**, med 37 minuter på marken. Anslutningsförslag med snedstreck skapar inte egna flyg.

[Arkivindexet](https://northwestairlineshistory.org/timetables-northwest/) anger 2 mars 1986 som nästa listade utgåva. Mellanliggande ändringar är inte verifierade. Importen begränsas till 27 februari–1 mars. Tidtabellen visar planerad trafik, inte bevisat genomförande, individflygplan eller verkliga flygbanor. Northwest-nätet är fortfarande partiellt.

## Kalender och UTC

Fönstret är exakt **1986-02-27 22:21:30 UTC till 1986-03-01 22:21:30 UTC**. Hela intervallet behålls när det överlappar fönstret. Minneapolis, O’Hare och Milwaukee har UTC−6, Detroit UTC−5 i denna period. Samtliga nya rader anländer samma lokala kalenderdag. Tidigare lokalt ankomstklockslag västerut från Detroit betyder alltså inte övernattning. `12:00n` betyder middag.

Separat handskriven 24-timmarstranskription och fasta UTC-förskjutningar verifierar **samtliga 114 nya intervall**, oberoende av kompilatorns ZoneInfo. Avgångsdatum: 27 februari 22, 28 februari 55, 1 mars 37. 47 scheman ger två rörelser, fem ger tre, fem ger en och **MKE–MSP711 söndag ger noll**. Den sistnämnda raden finns i katalogen men skapar ingen trafik torsdag–lördag.

Sex sammanhängande benpar har kontrollerade markuppehåll: 271 i MKE 30 minuter, 278 i MKE 28, 352 i MKE 33, 370 i MKE 29, 458 i ORD 37 och 707 i MKE 32. De första fem ger vardera två övergångar; 707 ger en, den 28 februari. Den 27 februari slutar det inkommande benet 22:18 UTC före startgränsen; den 1 mars börjar det utgående 22:50 UTC efter slutgränsen. Varje flygintervall prövas självständigt. Detta identifierar inga individflygplan.

## Verifiering och leverans

Alla **20 Python-tester och tre aktualitetskontroller passerar**. Inga tidsöverlapp finns mellan berörda ben med samma operatör och flygnummer. Alla tidigare rörelseobjekt, scheman, flygplatser, länder och operatörer är oförändrade. Endast Northwest-källposten kompletteras. Äldre forskningsfiler, rättelser och hållna källkonflikter bevaras, inklusive v116:s nio Delta-rättelser och den öppna granskningen av ATL–NAS125.

Unreal-kompilering, Editor och paketerat spel har inte körts. Detta är ett datapaket för befintlig Flight-meny med redan kompilerad **PaintAirplane-rättelse `f23b73a`**. ZIP-filen installerar inte C++-rättelsen.
