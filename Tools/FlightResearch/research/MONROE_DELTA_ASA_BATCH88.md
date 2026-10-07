# Monroe – Delta och Atlantic Southeast Airlines, omgång 88 / v91

37 nya planerade flygrörelser från 18 nya nonstop-scheman: 25 Delta och 12 Atlantic Southeast Airlines (ASA). Databasen innehåller nu 5 612 rörelser, varav 5 611 tidtabellslagda och en tidigare dokumenterat genomförd. Samtliga 5 575 tidigare rörelseobjekt och 3 439 tidigare scheman från v90 är oförändrade. Monroe Regional (MLU) och ASA tillkommer.

| Förbindelse, båda riktningarna | Scheman | Rörelser | Bolag |
|---|---:|---:|---|
| Monroe–Atlanta | 2 | 4 | Delta |
| Monroe–Dallas/Fort Worth | 2 | 4 | Delta |
| Monroe–Jackson, Mississippi | 3 | 6 | Delta |
| Monroe–Shreveport | 5 | 11 | Delta |
| Monroe–Memphis | 6 | 12 | ASA |
| Totalt | 18 | 37 | |

## Källa och avgränsning

[Delta Air Lines system timetable, gällande från 1 februari 1986](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201), Digital Library of Georgia. [Original-PDF](https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf), 136 sidor, SHA-256 `3aefa36e86dc1970af760d90385d2a887bcf5f0d7e0260e3a2764924db1835e4`. Utgåvans slutdatum är inte fastställt; importen avgränsas till lokala avgångsdatum 27 februari–1 mars 1986. Skanningen ingår inte i datapaketet.

Tio kompletta riktade stadstabeller har lästs visuellt. Totalt 34 rader: 18 nya nonstoprader, nio genomgående rader med mellanstopp och sju anslutningsförslag. Endast de 18 förstnämnda importeras. De övriga sparas i transkriptionen med uteslutningsorsak. Ett anslutningsförslag räknas aldrig som en obruten flygrörelse.

| Underlag | Tryckt sida | PDF-sida, räknat från 1 |
|---|---:|---:|
| Teckenförklaring och operatörsnyckel | 4 | 5 |
| Atlanta–Monroe | 16 | 11 |
| Dallas/Fort Worth–Monroe | 66 | 36 |
| Jackson–Monroe | 114 | 60 |
| Memphis–Monroe | 147 | 76 |
| Monroe–Atlanta/Dallas/Fort Worth/Jackson/Memphis | 159 | 82 |
| Monroe–Shreveport | 160 | 83 |
| Shreveport–Monroe | 226 | 116 |
| Flygplatsförteckning, Monroe Regional | 261 | 133 |
| Deltas linjeföljder | 262–263 | 134 |
| ASA:s linjeföljder | 264 | 135 |

Operatörsnyckeln anger ASA för nummer 1200–1299 och 1400–1524. Alla sex importerade ASA-rader ligger i det senare intervallet. Bolaget registreras därför separat som `atlantic-southeast`; dessa avgångar läggs inte på Delta. Flygnummer 1457 förekommer i båda riktningarna med olika trafikdagar: Memphis–Monroe är X7, Monroe–Memphis dagligen. Även 1456 Monroe–Memphis är X7. X7 betyder alla dagar utom söndag. Övriga importerade rader går dagligen. Inga daterade undantag är angivna för de accepterade raderna.

## Tider, period och delsträckor

Tiderna är lokala. Monroe, Memphis, Jackson, Shreveport och Dallas/Fort Worth använder UTC−6 under perioden; Atlanta UTC−5. Samtliga nya rader anländer samma lokala kalenderdag. En oberoende beräkning med fasta vinteroffset, veckodagar och periodöverlapp ger exakt samma 37 UTC-intervall som generatorn.

Fönstret är 27 februari 22:21:30 UTC till 1 mars 22:21:30 UTC. Hela tidtabellsintervallet behålls när det överlappar fönstret. Delta 691 Monroe–Shreveport, 16:10–16:38 lokal tid, överlappar både den första och sista gränsen och ger därför tre rörelser. De övriga 17 schemana ger två vardera.

Fjorton övergångar mellan separat källbelagda ben med samma flygnummer är kontrollerade. De omfattar Delta 691, 471, 1036, 941, 583, 529, 584 och 662 samt ASA 1457 och 1458. Markuppehållen är 10–25 minuter. Kontrollen visar att tiderna hänger ihop; den fastställer inte individuella flygplan. Genomgående stadstabeller används som kontroll, inte som extra animerade direktsträckor. Fullständiga ID:n och uppehåll finns i `validation_batch88.json`.

## Flygplats och fortsatt arbete

Flygplatsförteckningen på tryckt sida 261 anger Monroe Regional. [Flygplatsens egen historik](https://flymonroe.org/about/) kopplar dagens flygplats till Selman Field och anger överlåtelsen till staden i september 1949. [OurAirports KMLU](https://ourairports.com/airports/KMLU/) ger koordinaterna 32.510899, −92.037697. Markören avser ungefärligt flygplatsområde, inte en rekonstruerad terminal, gate eller bana från 1986.

Registret har nu 96 operatörer: 50 med rörelser, varav 49 civila och US Marine Corps, samt 46 utan. Delta har 1 492 rörelser, ASA 12 och COMAIR fortsatt 381. Inget bolag är verifierat fullständigt. Nästa urval är saknade ben Monroe–Pensacola 584 och Monroe–Mobile 662, därefter Mobile/Birmingham och ASA:s regionala Memphisnät. Europakön behåller Cypern som nästa land.

Rättelserna för COMAIR 1582 CVG–DTW och 1580 ORD–MKE behålls med stabila ID:n. De olösta konflikterna i omgång 80 (Delta 597 MEM–LIT) och 82 (CMH–SDF 1871/1671) är fortsatt undanhållna. Äldre forskning och uppgifter om flygplatser/bolag är bevarade.

## Validering och filer

Alla 20 Python-tester och aktualitetskontroller passerar. Körresultaten redovisas i `automated_checks_batch88.json`. Tidtabellsdata har byggts och jämförts med v90. UE-kompilering, spelkörning och paketerad build är inte utförda här. Paketet förutsätter den redan kompilerade `PaintAirplane`-rättelsen från `f23b73a`.

- `monroe_delta_asa_batch88.tsv`: samtliga 34 granskade rader och beslut.
- `monroe_delta_asa_source_evidence_batch88.json`: källsidor, läsregler och avgränsning.
- `airport_locations_batch88.json` och `operator_review_batch88.json`: nya registerposter.
- `batch_88.json` och `validation_batch88.json`: importresultat och kontroller.
- `WORLDWIDE_COVERAGE_BATCH88.md`: aktuell operatörsöversikt.
