# ASA kring Atlanta – omgång 94 / v97

V97 tillför **58 planerade flygrörelser från 28 nya nonstop-scheman** med Atlantic Southeast Airlines (ASA). Totalt finns **5 972 rörelser**, varav 5 971 tidtabellslagda och en tidigare dokumenterat genomförd. Alla 5 914 rörelser, 3 602 scheman, 264 flygplatser och 96 bolag från v96 är oförändrade.

Tuscaloosa (TCL) och Valdosta (VLD) tillkommer. Databasen har nu 266 flygplatser/platser, 1 179 riktade ändpunktspar och 3 609 granskade scheman i kördatan. ASA har 186 rörelser. Inget bolag är verifierat fullständigt.

## Nya sträckor

| Sträcka | Scheman | Rörelser i fönstret |
|---|---:|---:|
| Atlanta → Columbus, MS | 3 | 7 |
| Columbus, MS → Atlanta | 3 | 6 |
| Atlanta → Meridian | 2 | 5 |
| Meridian → Atlanta | 3 | 6 |
| Atlanta → Pine Belt | 1 | 2 |
| Atlanta → Tuscaloosa | 3 | 6 |
| Tuscaloosa → Atlanta | 3 | 6 |
| Tuscaloosa → Columbus, MS | 1 | 2 |
| Columbus, MS → Tuscaloosa | 1 | 2 |
| Atlanta → Valdosta | 4 | 8 |
| Valdosta → Atlanta | 4 | 8 |
| **Summa** | **28** | **58** |

Pine Belt (PIB) är tidtabellens Laurel/Hattiesburg. Columbus avser Golden Triangle (GTR), Mississippi. Även Pine Belt → Atlanta granskades; samtliga tre rader har mellanlandning eller flygbyte och ger ingen ny direktsträcka.

## Källgranskning

Primärkälla: [Delta Air Lines, giltig från 1 februari 1986](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201), [original-PDF](https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf).

48 rader i tolv kompletta riktningsrubriker granskades visuellt på tryckta sidor 15, 16, 17, 59, 61, 125, 148, 240 och 241. 28 rader med stoppkolumn 0 importeras; 14 anslutningsförslag och sex genomgående rader med mellanlandning importeras inte som egna direktflyg. Flygnummer, lokaltider, frekvens och sidnummer finns i `asa_atlanta_regional_batch94.tsv`. PDF-hash och sidmappning finns i `asa_atlanta_regional_source_evidence_batch94.json`. Originalskanningar ingår inte.

Triangeln och nummernyckeln på s. 4 identifierar ASA: 1200–1299 samt 1400–1524. Tomt frekvensfält betyder dagligen, X6 alla dagar utom lördag, X7 alla dagar utom söndag. Inga importerade rader har daterad fotnot. Utgåvans slutdatum och senare ändringar är inte verifierade.

Ruttningsnyckeln på s. 264 kontrollerades. ATL–GTR 1248 går via Tuscaloosa; motsatt genomgående 1240 går GTR–TCL–ATL. De separat tryckta benen har tio minuters markuppehåll. ATL–MEI 1446 går via Pine Belt, medan ATL–PIB 1449 går via Meridian. De nya benen ansluter till tidigare importerade PIB–MEI 1445/1446 och MEI–PIB 1449, utan extra direktflyg.

Varje ben behåller sina egna veckodagar. ATL–GTR 1241, ATL–VLD 1521 och ATL–PIB 1446 har X7, medan respektive vidare/returben är dagligt. Ingen frekvens kopieras från ett annat ben.

Förstorade originalrader bekräftar ATL–GTR 1241 kl. **10:25–10:45**, ATL–MEI 1447 kl. **16:46–17:15** och ATL–PIB 1446 kl. **10:17–11:12**. Atlanta använder UTC−5 och dessa destinationer UTC−6, vilket förklarar de korta skillnaderna mellan tryckta klockslag.

## Historiska flygplatser

| Kod | Markör | Kontroll |
|---|---|---|
| TCL | Tuscaloosa Municipal Airport (1986) | Tidtabellens register s. 261 anger Municipal. [Stadens flygplatshistorik](https://airport.tuscaloosa.com/) beskriver övertagandet efter andra världskriget och fortsatt användning av flygfältet. |
| VLD | Valdosta Regional Airport | Tidtabellens register s. 261 anger Valdosta. [Flygplatsens historik](https://flyvaldosta.com/about-us/) beskriver det civila fältet söder om staden, öppnat efter militär användning, med Atlanta-pendeltrafik från 1980. |

Moderna koordinater från [OurAirports TCL](https://ourairports.com/airports/KTCL/) och [OurAirports VLD](https://ourairports.com/airports/KVLD/) används som ungefärliga flygplatsmarkörer. Historiska lägen är separat kontrollerade. Valdosta-markören avser det civila fältet, inte Moody Air Force Base. Terminaler, gater och exakta banlägen 1986 rekonstrueras inte. Se `airport_locations_batch94.json`.

## Kalender och validering

Atlanta och Valdosta använder UTC−5; Columbus MS, Meridian, Pine Belt, Tuscaloosa och Memphis UTC−6. Alla accepterade ankomster är samma lokala kalenderdag. Separat transkription till 24-timmarstid och oberoende beräkning med fasta vinteroffsetar matchar alla 58 genererade rörelser.

Fönstret är 27 februari 22:21:30 UTC till 1 mars 22:21:30 UTC 1986. Flygets hela tidsintervall behålls om det överlappar fönstret. ATL–GTR 1246 och ATL–MEI 1447 överlappar starten och ger tre rörelser vardera. Övriga 26 scheman ger två. Fördelning per lokalt avgångsdatum: torsdag 12, fredag 28, lördag 18.

Nitton övergångar med samma operatör och flygnummer har kontrollerade markuppehåll på 10–15 minuter. Det belägger inte individflygplan. Inga tidsöverlappningar upptäcktes för berörda operatör/flygnummer.

Alla 20 Python-tester och aktualitetskontroller passerar. Körresultaten redovisas i `automated_checks_batch94.json`. Detaljkontroller finns i `validation_batch94.json`. Tidigare COMAIR-rättelser och hållna källkonflikter för Delta 597 MEM–LIT respektive 1871/1671 CMH–SDF är bevarade. UE 5.8-kompilering, spelkörning och paketerad build har inte utförts här.

Paketet innehåller hela databasen och forskningshistoriken. Befintlig Flight-meny och kompilerad PaintAirplane-rättelse från `f23b73a` krävs. Inga C++-filer ingår.

Nästa prioritet: ASA Memphis–Fayetteville/Drake Field och fler regionala Atlanta-linjer, exempelvis Albany, Brunswick, Dothan, Huntsville och Gainesville. Fortsätt med kompletta riktningsrubriker och kontroll av historiska flygplatser.
