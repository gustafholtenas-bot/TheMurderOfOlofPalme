# ASA kring Memphis – omgång 93 / v96

V96 tillför **79 planerade flygrörelser från 37 nya nonstop-scheman** med Atlantic Southeast Airlines (ASA). Totalt finns **5 914 rörelser**, varav 5 913 tidtabellslagda och en tidigare dokumenterat genomförd. Alla 5 835 rörelser, 3 565 scheman, 259 flygplatser och 96 bolag från v95 är oförändrade.

Fem flygplatser tillkommer. Databasen har nu 264 flygplatser/platser, 1 168 riktade ändpunktspar och 3 581 granskade scheman i den genererade kördatan. ASA har 128 rörelser. Inget bolag är verifierat fullständigt.

## Nya sträckor

| Sträcka | Scheman | Rörelser i fönstret |
|---|---:|---:|
| Memphis → Fort Smith | 3 | 7 |
| Fort Smith → Memphis | 3 | 6 |
| Memphis → Columbus, MS | 4 | 9 |
| Columbus, MS → Memphis | 4 | 8 |
| Memphis → Greenville, MS | 4 | 9 |
| Greenville, MS → Memphis | 4 | 8 |
| Memphis → Meridian | 4 | 9 |
| Meridian → Memphis | 4 | 8 |
| Memphis → Pine Belt | 1 | 3 |
| Pine Belt → Memphis | 1 | 2 |
| Greenville, MS → Pine Belt | 1 | 2 |
| Pine Belt → Greenville, MS | 1 | 2 |
| Meridian → Pine Belt | 1 | 2 |
| Pine Belt → Meridian | 2 | 4 |
| **Summa** | **37** | **79** |

Pine Belt (PIB) är tidtabellens Laurel/Hattiesburg. Columbus avser Golden Triangle (GTR), Mississippi.

## Tidtabell och avgränsning

Primärkälla: [Delta Air Lines, giltig från 1 februari 1986](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201), [original-PDF](https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf).

41 rader i 14 kompletta riktningsrubriker granskades visuellt på tryckta sidor 60, 86, 97, 126, 146, 147 och 149. 37 rader har stoppkolumn 0 och importeras; två anslutningsförslag och två genomgående rader med mellanlandning importeras inte som egna direktflyg. Flygnummer, lokaltider, frekvens och sidnummer finns i `asa_memphis_regional_batch93.tsv`. PDF-hash och sidmappning finns i `asa_memphis_regional_source_evidence_batch93.json`. Originalskanningar ingår inte i paketet.

Triangeln och nummernyckeln på s. 4 identifierar ASA: 1200–1299 respektive 1400–1524. Tomt frekvensfält betyder dagligen, X6 alla dagar utom lördag, X7 alla dagar utom söndag. Inga importerade rader har daterad fotnot. Slutdatum för utgåvan och senare ändringar är inte verifierade.

Varje fysiskt ben behåller sina egna veckodagar. Exempelvis har MEM–GLH 1299 och MEM–MEI 1451 X7, medan deras returer går dagligen. MEM–GLH 1401 går dagligen, men GLH–MEM 1401 har X6.

MEM–PIB 1402 och PIB–MEM 1298 har mellanstopp i Greenville. De representeras av separat tryckta MEM–GLH/GLH–PIB respektive PIB–GLH/GLH–MEM, utan extra direktflyg. MEI–PIB kl. 21:05–21:35 är **1449**, inte 1453; ruttningsnyckeln på s. 264 skiljer ATL–MEI–PIB 1449 från MEM–MEI 1453.

Förstorade originalrader bekräftar MEM–PIB 1424 kl. **15:30–16:45**, MEM–GTR 1247 kl. **16:10–17:00** och MEI–MEM 1447 kl. **17:25–18:28**.

## Historiska flygplatser

| Kod | Markör | Kontroll |
|---|---|---|
| FSM | Fort Smith Municipal | Tidtabell s. 260; [flygplatsens historik](https://flyfsm.com/history/) dokumenterar den äldre terminalen och flygfältet före 1986. Namnet Regional tillkom 1992. |
| GTR | Golden Triangle Regional | Tidtabell s. 260; [flygplatsens historik](https://www.gtra.com/history/) dokumenterar anläggningen från tidigt 1970-tal. |
| GLH | Greenville Municipal / Mid Delta | Tidtabell s. 260; [Federal Register 30 oktober 1969](https://www.govinfo.gov/content/pkg/FR-1969-10-30/pdf/FR-1969-10-30.pdf), tryckt s. 17511, anger Municipal vid 33°29′05″N, 90°59′20″W. Läget överensstämmer med dagens Mid Delta-område. [Staden](https://greenvillems.org/airport/) identifierar flygplatsen som tidigare Greenville Air Force Base. |
| MEI | Key Field | Tidtabell s. 261; [National Park Service](https://www.nps.gov/articles/old-terminal-building-hangar-and-powerhouse-at-key-field.htm) dokumenterar flygplatskomplexet från 1930. |
| PIB | Pine Belt Regional | Tidtabell s. 261; [Hattiesburg Area Historical Society](https://www.hahsmuseum.org/gall_pb01.htm) dokumenterar öppningen 1974 och namnbytet till Hattiesburg-Laurel 1990. |

Moderna OurAirports-koordinater används som ungefärliga flygplatsmarkörer, med separat historisk lägeskontroll. Koordinatkällor och noter finns i `airport_locations_batch93.json`. Gamla Greenville Municipal sydost om staden, Laurel Hesler-Noble, Hattiesburg Municipal och Meridian Naval Air Station används inte. Terminaler, gater och banlägen 1986 rekonstrueras inte.

## Kalender och validering

Alla berörda flygplatser använder UTC−6 under perioden. Samtliga accepterade ankomster sker samma lokala kalenderdag. Separat transkription till 24-timmarstid samt oberoende beräkning med fasta vinteroffsetar matchar alla 79 genererade rörelser.

Fönstret är 27 februari 22:21:30 UTC till 1 mars 22:21:30 UTC 1986. Flygets hela tidsintervall behålls om det överlappar fönstret. Fem scheman överlappar starten och ger tre rörelser vardera: MEM–GTR 1247, MEM–FSM 1282, MEM–GLH 1401, MEM–PIB 1424 och MEM–MEI 1448. Övriga 32 scheman ger två rörelser. Fördelning per lokalt avgångsdatum: torsdag 16, fredag 37, lördag 26.

Tio övergångar med samma operatör och flygnummer har kontrollerade markuppehåll på 10–125 minuter. Det belägger inte individflygplan. Ingen tidsöverlappning upptäcktes för berörda operatör/flygnummer.

Alla 20 Python-tester och aktualitetskontroller passerar. Körresultaten redovisas i `automated_checks_batch93.json`. Detaljkontroller finns i `validation_batch93.json`. Tidigare COMAIR-rättelser och hållna källkonflikter för Delta 597 MEM–LIT respektive 1871/1671 CMH–SDF är bevarade. UE 5.8-kompilering, spelkörning och paketerad build har inte utförts här.

Paketet innehåller hela databasen och forskningshistoriken. Befintlig Flight-meny och kompilerad PaintAirplane-rättelse från `f23b73a` krävs. Inga C++-filer ingår.

Nästa prioritet: ASA:s ben mellan Atlanta och Columbus MS, Meridian och Pine Belt, med särskild kontroll av via Tuscaloosa. Memphis–Fayetteville/Drake Field återstår också som kandidat.
