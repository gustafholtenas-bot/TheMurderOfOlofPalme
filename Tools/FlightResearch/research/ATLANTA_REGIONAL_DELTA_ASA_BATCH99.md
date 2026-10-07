# Atlanta regionalt: fem destinationer – omgång 99 / v102

V102 tillför **82 planerade rörelser från 42 nya nonstop-scheman**: 15 Delta och 67 Atlantic Southeast Airlines (ASA). Totalt **6 374 rörelser**, varav 6 373 tidtabellslagda och en tidigare dokumenterat genomförd. Alla 6 292 rörelser, 3 796 scheman, 278 flygplatser och 96 operatörer från v101 är oförändrade.

Fem flygplatser tillkommer: Anniston–Calhoun County (ANB), Gadsden Municipal (GAD), Roanoke Regional–Woodrum Field (ROA), Tri-City Regional (TRI) och Tallahassee Municipal (TLH). Totalt 283 flygplatser/platser och 1 218 riktade ändpunktspar. Katalogen har 3 838 scheman, varav 3 817 granskade. Delta har 1 752 rörelser och ASA 514. Inget bolag är verifierat fullständigt.

## Nya sträckor

| Sträcka | Nya nonstop-scheman | Rörelser |
|---|---:|---:|
| Atlanta → Anniston | 2 | 4 |
| Anniston → Atlanta | 3 | 6 |
| Atlanta → Gadsden | 2 | 4 |
| Gadsden → Atlanta | 2 | 4 |
| Gadsden → Anniston | 1 | 2 |
| Atlanta → Roanoke | 4 | 8 |
| Roanoke → Atlanta | 4 | 9 |
| Atlanta → Tri-City | 4 | 7 |
| Tri-City → Atlanta | 4 | 7 |
| Atlanta → Tallahassee | 8 | 16 |
| Tallahassee → Atlanta | 8 | 15 |
| **Summa** | **42** | **82** |

## Källgranskning

Primärkälla: [Delta Air Lines, giltig från 1 februari 1986](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201), [original-PDF](https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf).

Elva kompletta riktningsrubriker med 47 rader har granskats visuellt på tryckta sidor 11, 14, 15, 17, 91, 204, 227 och 236. 42 individuellt tryckta nonstopben accepteras, 34 ASA och åtta Delta. Två genomgående rader med stopp och tre anslutningsförslag importeras inte som egna nonstopflyg. Originaltider, veckodagar och beslut finns i `atlanta_regional_delta_asa_batch99.tsv`; PDF-hash och sidmappning i `atlanta_regional_delta_asa_source_evidence_batch99.json`.

Triangeln och nummernyckeln på s. 4 identifierar ASA:s flyg. Tomt frekvensfält betyder dagligen; X6 utesluter lördag, X7 söndag och X67 båda helgdagarna. Ingen accepterad rad har daterad fotnot. Utgåvans slutdatum, senare ändringar och faktiskt genomförande är inte verifierade. Originalskanningar distribueras inte.

## Delsträckor, tidszoner och midnatt

ASA 1210 följer enligt s. 264 **ATL–GAD–ANB–ATL**. De individuellt tryckta benen är Atlanta–Gadsden **09:00–08:45 Central, X7**, Gadsden–Anniston **08:50–09:10, X7** och Anniston–Atlanta **09:25–11:10 Eastern, dagligen**. Markuppehållen är fem och femton minuter. Varje ben behåller sin egen frekvens; genomgående ATL–ANB och GAD–ATL skapar inga extra flyg.

Fyra Alabama-ben anländer med ett tidigare lokalt klockslag än avgången, men samma kalenderdag: ATL–ANB 1211 **14:25–14:10**, 1212 **21:25–21:10**, ATL–GAD 1210 **09:00–08:45** och 1472 **20:50–20:35**. Tidszonsskillnaden ger 45 minuters flygtid. Inget falskt dygnsskifte läggs till.

Två riktiga nattankomster finns: ATL–ROA 1512 **22:55–00:25 nästa dag** och ATL–TRI 1499 **23:00–00:20 nästa dag**, båda X6. Ingen otryckt retur härleds ur dem.

Förstoringen bekräftar ATL–ROA 1509 **10:30–12:01**, följt av retur 12:30: 29 minuters markuppehåll. ATL–TRI 1497 **11:47–13:07** följs av retur 13:20: tretton minuter. Övriga kontrollerade vändningar har 15–25 minuter.

Atlanta, Roanoke, Tri-City och Tallahassee använder vintertid UTC−5. Anniston och Gadsden använder UTC−6. Tallahassee ligger i Eastern Time. Tryckt `1200n` på ATL–TLH 311 betyder 12:00.

## Exakt tidsfönster

Lokala avgångsdatum 27 februari–1 mars 1986 prövas mot **27 februari 22:21:30 UTC till 1 mars 22:21:30 UTC**. Hela flygets tider behålls vid överlapp. Alla 42 scheman ger rörelser: två ger tre vardera, 36 ger två och fyra ger en. Fördelningen är torsdag 15, fredag 42, lördag 25.

ROA–ATL 1510 och ATL–TLH 904 överlappar båda fönstergränserna och ger tre rörelser. De två 1496-benen mellan Atlanta och Tri-City, ATL–TLH 1103 och TLH–ATL 616 ger endast en fredagsrörelse vardera. Benens veckodagar är olika och bevaras: ATL–TRI 1496 X67 mot TRI–ATL 1496 X6, samt ATL–ROA 1509 X7 mot ROA–ATL 1509 dagligen.

## Historiska flygplatser

| Kod | Underlag för placering och periodnamn |
|---|---|
| ANB | Deltas register s. 260: Anniston Calhoun County. [FAA-uppgifter via AirNav](https://www.airnav.com/airport/KANB) anger aktivering mars 1941 för det civila fältet sydväst om staden. [University of Alabama](https://alabamamaps.ua.edu/aerials/Counties/Calhoun/Calhoun.html) förtecknar äldre flygbilder av Anniston Metropolitan Airport. ANB skiljs från ASN/Talladega och Fort McClellan. |
| GAD | Register s. 260: Gadsden Municipal. [FAA-uppgifter via AirNav](https://www.airnav.com/airport/KGAD) anger aktivering januari 1944 och fältet sydväst om Gadsden. Markören avser dagens Northeast Alabama Regional, inte det äldre Gulf/Republic Steel-fältet. |
| ROA | Register s. 261 använder Municipal. [Flygplatsens historik](https://flyroa.com/airport-history) anger namnbytet till Regional/Woodrum 1983 och kontinuitet på samma plats. Den nuvarande terminalen stod klar först 1989. |
| TRI | Register s. 261: Tri-City Regional. [Kingsports stadsarkiv](https://kingsportarchives.wordpress.com/2009/08/18/national-aviation-week/) dokumenterar regionalfältet McKellar från 1937. TRI vid Blountville i Tennessee skiljs från PSC i Washington. |
| TLH | Register s. 261: Tallahassee Municipal. [Kommunens rapport 1620, tryckt s. 10](https://www.talgov.com/Uploads/Public/documents/transparency/ig/fy16/1620.pdf) beskriver flytten från Dale Mabry Field till det nya fältet 1961 samt en ny terminal 1989. Markören avser flygplatsområdet från 1961. |

Koordinater från OurAirports används som ungefärliga flygplatsmarkörer. Terminaler, gater och banlägen 1986 rekonstrueras inte. Fullständiga koordinater och länkar finns i `airport_locations_batch99.json`.

## Validering och fortsättning

En separat 24-timmarstranskription med uttryckliga dygnsregler och oberoende beräkning av vinteroffsetar, veckodagar och överlapp matchar alla 82 nya rörelser. Tretton övergångar med samma operatör och flygnummer har kontrollerade markuppehåll på 5–29 minuter. Det belägger inte individflygplan. Inga tidsöverlappningar upptäcktes för berörda operatör/flygnummer.

Alla 20 Python-tester och aktualitetskontroller passerar. Körresultaten redovisas i `automated_checks_batch99.json`; detaljer i `validation_batch99.json`. Alla tidigare dataobjekt och rättelser bevaras. De tre hållna konflikterna, Delta 597 MEM–LIT, 1871/1671 CMH–SDF och ASA 1412 HSV–ATL, förblir olösta.

UE 5.8-kompilering, spelkörning och paketerad build har inte utförts här. Paketet innehåller hela databasen och forskningshistoriken. Befintlig Flight-meny och kompilerad PaintAirplane-rättelse från `f23b73a` krävs; inga C++-filer ingår.

Nästa prioritet: Muscle Shoals med individuella ben till/från Atlanta och Gadsden, därefter COMAIR mellan Roanoke, Cincinnati och Richmond.
