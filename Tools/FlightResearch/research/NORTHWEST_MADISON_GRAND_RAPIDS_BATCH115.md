# Northwest – Madison och Grand Rapids, v118 / batch115

Granskat 1 oktober 2026. Kumulativ uppdatering från v117.

40 nya planerade Northwest-rörelser från 23 granskade nonstop-scheman. Madison till/från O’Hare, Milwaukee och Minneapolis samt Grand Rapids i Michigan till/från Detroit och Minneapolis. 10 riktade par berörs, varav åtta nya i databasen; GRR tillkommer som flygplats. Totalt **8 343 rörelser**, varav 8 342 tidtabellslagda och en dokumenterat genomförd. Alla 8 303 äldre rörelseobjekt och 4 838 äldre scheman är oförändrade. V117 och v118 ger tillsammans 154 nya rörelser. Se `research/NORTHWEST_MADISON_GRAND_RAPIDS_BATCH115.md`.

## Sträckor

Alla 23 nya scheman ger rörelser i spelfönstret. Scheman är återkommande källrader; rörelser är daterade flygintervall. Båda riktningarna räknas separat.

| Riktad sträcka | Nya scheman | Nya rörelser |
|---|---:|---:|
| DTW → GRR | 2 | 4 |
| GRR → DTW | 2 | 4 |
| GRR → MSP | 2 | 5 |
| MKE → MSN | 3 | 5 |
| MSN → MKE | 4 | 4 |
| MSN → MSP | 2 | 5 |
| MSN → ORD | 2 | 3 |
| MSP → GRR | 2 | 4 |
| MSP → MSN | 2 | 3 |
| ORD → MSN | 2 | 3 |
| **Totalt** | **23** | **40** |

## Källa och avgränsning

[Northwest Orient, 18 december 1985, Digital Library of Georgia](https://dlg.usg.edu/record/delta_nwa-tt_nwa-tt-19851218). Nya rader har granskats visuellt på tryckta **s.22,30,47,66,67,75,78,79**, motsvarande **PDF-s.13,17,25,35,39,41**. Legenden s.150/PDF78 granskades i batch114. PDF-hash, sida och råfält för varje rad medföljer i `northwest_source_evidence_batch115.json` och `northwest_madison_grand_rapids_batch115.tsv`; originalskanningen ingår inte i ZIP.

Alla accepterade rader anger **N/S**, nonstop. Daily betyder dagligen, ExSa utom lördag, ExSaSu utom lördag/söndag och Sa lördag. Inga valda rader har datumfotnot. O-suffixet anger Chicago O’Hare. Flygnumren ligger under Airlink-serierna och tillhör Northwest mainline. OCR användes bara för att hitta kandidater; tider, flygnummer, stopp och veckodagar lästes i originalbilderna.

**Grand Rapids i Michigan** hålls skilt från **Grand Rapids i Minnesota**. Det senare står i nästa avsnitt på s.48–49 och har bland annat Airlink1052/1056; dessa hör inte till GRR och importeras inte här.

**MSP–MKE224** och **DTW–MSP375** är genomgående resor med mellanlandningar. Endast de separat tidsatta fysiska delsträckorna läggs in. Flyg375 blir därmed **DTW–MKE–MSN–MSP**, och flyg224 **MSP–MSN–MKE**. Anslutningar såsom MSP–GRR524/763 via Detroit och MSN–DTW202/370 via Milwaukee importeras inte som nonstop.

[Arkivindexet](https://northwestairlineshistory.org/timetables-northwest/) anger 2 mars 1986 som nästa listade utgåva. Mellanliggande ändringar är inte verifierade. Importen begränsas till 27 februari–1 mars. Tidtabellen belägger planerad trafik, inte genomförande, individflygplan eller faktisk flygbana. Nätet är fortfarande partiellt.

## Ny flygplats

**GRR – Grand Rapids / Kent County International (1986)** läggs till med tidszonen America/Detroit. [Flygplatsmyndighetens historik](https://www.grr.org/history) anger flytten till nuvarande plats i Cascade Township den 23 november 1963, namnet Kent County International från 27 januari 1977 och namnbytet till Gerald R. Ford först 1999. Därför används 1986 års namn i katalogen.

[OurAirports KGRR/GRR](https://ourairports.com/airports/KGRR/) ger kartmarkören **42.880798, −85.522797**. Det är en ungefärlig markör för samma flygfält, inte en rekonstruktion av 1986 års referenspunkt, terminal, gate eller bantröskel. `airport_location_review_batch115.json` redovisar underlaget och precisionen. De tidigare 321 flygplats-/platsposterna är oförändrade. Inga nya länder eller operatörer tillkommer.

## Tider och kontroll

Det exakta fönstret är **1986-02-27 22:21:30 UTC till 1986-03-01 22:21:30 UTC**. Hela flygintervall behålls när de överlappar fönstret. Samtliga nya rader anländer samma lokala kalenderdygn. Madison, Milwaukee, Minneapolis och O’Hare har UTC−6, Grand Rapids och Detroit UTC−5. GRR–MSP tar 77 minuter trots 17 minuters skillnad mellan lokala klockslag.

Separat handskriven 24-timmarstranskription och fast-offsetberäkning verifierar **alla 40 nya UTC-intervall**, oberoende av ZoneInfo i byggverktyget. 13 scheman ger två rörelser, åtta ger en och två ger tre. De två med tre är **MSN–MSP375** och **GRR–MSP167**, vilka också pågår vid fönstrets början. Avgångsdatumen fördelas på 27 februari:7, 28 februari:22, 1 mars:11.

Fyra sammanhängande benpar har granskats: flyg224 i Madison **24 minuter**, flyg247 i O’Hare **33**, flyg375 i Milwaukee **25** och i Madison **22**. Flyg224/247 har varsin övergång den 28 februari; 27 februari slutar deras ben före startgränsen och lördagstrafik är utesluten. Flyg375 har båda övergångarna den 28 februari och 1 mars. Den 27 februari är bara sista benet MSN–MSP kvar i luften vid startgränsen. Markuppehållen bevisar inte individflygplan.

**20 Python-tester och tre aktualitetskontroller passerar.** Inga tidsöverlapp finns där ett nytt ben jämförs med andra ben med samma operatör och flygnummer. Alla äldre rörelseobjekt, scheman, länder och operatörer är oförändrade. Endast Northwest-källposten kompletteras. Äldre forskningsfiler, rättelser och hållna källkonflikter följer med, inklusive v116:s nio Delta-rättelser och öppna avgångsminuten för ATL–NAS125.

Unreal-kompilering, Editor och paketerat spel har inte körts. Paketet innehåller data för befintlig Flight-meny och kräver redan kompilerad **PaintAirplane-rättelse `f23b73a`**. ZIP-filen installerar inte C++-rättelsen.
