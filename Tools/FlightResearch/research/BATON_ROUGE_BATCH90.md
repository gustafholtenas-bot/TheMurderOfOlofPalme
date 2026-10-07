# Baton Rouge – omgång 90 / v93

37 nya planerade Delta-rörelser från 19 nya nonstop-scheman. Databasen innehåller nu 5 707 rörelser, varav 5 706 tidtabellslagda och en tidigare dokumenterat genomförd. Samtliga 5 670 tidigare rörelseobjekt och 3 485 tidigare scheman från v92 är oförändrade. Baton Rouge Metropolitan (BTR) tillkommer som flygplatsmarkör.

| Förbindelse | Scheman | Rörelser |
|---|---:|---:|
| Baton Rouge ↔ Atlanta | 6 | 11 |
| Baton Rouge ↔ Dallas/Fort Worth | 9 | 18 |
| Baton Rouge → Mobile | 2 | 4 |
| Pensacola → Baton Rouge | 2 | 4 |
| Totalt | 19 | 37 |

## Källa och urval

[Delta Air Lines system timetable, gällande från 1 februari 1986](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201), Digital Library of Georgia. [Original-PDF](https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf), 136 sidor, SHA-256 `3aefa36e86dc1970af760d90385d2a887bcf5f0d7e0260e3a2764924db1835e4`. Utgåvans slutdatum är inte fastställt. Importen avser lokala avgångsdatum 27 februari–1 mars 1986. Skanningen ingår inte i datapaketet.

Åtta kompletta riktade stadstabeller har lästs visuellt, totalt 38 rader: 19 accepterade nonstoprader, sju anslutningsförslag och tolv genomgående rader med mellanstopp. De senare 19 raderna sparas med uteslutningsorsak och skapar inga extra direktsträckor. De två Pensacola–Dallas-tabellerna används som kontroll av delsträckorna. Inga omvända direktflyg Baton Rouge–Pensacola eller Mobile–Baton Rouge har lagts till utan egen tidtabellsrad.

| Underlag | Tryckt sida | PDF-sida, räknat från 1 |
|---|---:|---:|
| Teckenförklaring | 4 | 5 |
| Atlanta–Baton Rouge | 14 | 10 |
| Baton Rouge–Atlanta/Dallas-Fort Worth | 25 | 15 |
| Baton Rouge–Mobile | 26 | 16 |
| Dallas/Fort Worth–Baton Rouge | 64 | 35 |
| Dallas/Fort Worth–Pensacola | 66 | 36 |
| Pensacola–Baton Rouge/Dallas-Fort Worth | 189 | 97 |
| Flygplatsförteckning | 260 | 133 |
| Linjeföljder | 262–264 | 134–135 |

## Trafikdagar och period

Alla accepterade rader gäller Delta. Två har X6, alltså alla dagar utom lördag: 1192 Baton Rouge–Atlanta och 929 Atlanta–Baton Rouge. Övriga accepterade rader går dagligen. Inga daterade fotnoter är angivna för dem. Stjärnan betyder lågtrafikpris enligt teckenförklaringen, inte ankomst nästa dag.

Alla nya direktsträckor anländer samma lokala kalenderdag. Baton Rouge, Dallas/Fort Worth, Mobile och Pensacola använder UTC−6; Atlanta UTC−5. De otydliga minutsiffrorna har kontrollerats i förstorad bild: flyg 804 anländer Atlanta 15:09, medan 923 och 211 anländer Dallas/Fort Worth 11:09 respektive 21:09.

Det exakta fönstret är 27 februari 22:21:30 UTC till 1 mars 22:21:30 UTC. Hela tidtabellsintervallet behålls när det överlappar fönstret. Atlanta–Baton Rouge 929 går 15:25 Eastern–15:50 Central, alltså 20:25–21:50 UTC. Torsdagens flyg ligger före fönstret och X6 utesluter lördagen; endast fredagen ger en rörelse. Övriga 18 scheman ger två vardera. Lokal avgångsdag: sex rörelser torsdag, 19 fredag och tolv lördag.

## Delsträckor och tidskontroll

Elva övergångar mellan separat källbelagda ben med samma flygnummer har kontrollerats:

| Flyg | Följd som kontrollerats | Markuppehåll |
|---|---|---|
| 636 | Dallas–Baton Rouge–Mobile–Atlanta | 35, 32 minuter |
| 980 | Dallas–Baton Rouge–Mobile–Atlanta–Minneapolis | 20, 22, 63 minuter |
| 923 | Atlanta–Pensacola–Baton Rouge–Dallas | 20, 22 minuter |
| 991 | Atlanta–Pensacola–Baton Rouge–Dallas | 20, 22 minuter |
| 804 | Baton Rouge–Atlanta–LaGuardia | 37 minuter |
| 595 | Baltimore–Atlanta–Baton Rouge | 69 minuter |

Kontrollen visar att tiderna hänger ihop, inte vilka individuella flygplan som användes. Alla tidigare ben behåller sina egna källbelagda tider och veckodagar. Inga tidsöverlapp med samma bolag/flygnummer förekommer för de nya rörelserna.

Genomgående Pensacola–Dallas 923/991 har stopp i Baton Rouge, och Atlanta–Baton Rouge 923/991 har stopp i Pensacola. Baton Rouge–Atlanta 636/980 har stopp i Mobile. Dessa genomgående rader dubblerar inte de fysiska benen. Dallas–Pensacola 584, 928 och 387 har två, ett respektive ett stopp och importeras heller inte som obrutna direktflyg.

## Flygplats och kvarvarande arbete

Tidtabellens sida 260 anger BTR som Metropolitan. [Flygplatsens egen historik](https://www.flybtr.com/about-btr/our-history) kopplar den nuvarande platsen till tidigare Harding Field, öppnat för civilt bruk 1948. Markören använder [OurAirports KBTR](https://ourairports.com/airports/KBTR/), 30.533199, −91.149597. Koordinaterna avser ungefärligt flygplatsområde, inte en rekonstruerad terminal eller bana från 1986.

Registret har fortsatt 96 operatörer: 50 med rörelser, varav 49 civila och US Marine Corps, samt 46 utan. Delta har 1 587 rörelser, ASA 12 och COMAIR 381. Inget bolag är verifierat fullständigt. Baton Rouge–Birmingham 976 och Birmingham–Baton Rouge 317 återstår, tillsammans med Birmingham/Montgomerys övriga nät. Därefter prioriteras ASA:s Memphisnät. Europakön behåller Cypern som nästa land.

Tidigare COMAIR-rättelser från v87 och v89 är bevarade. Konflikterna i omgång 80, Delta 597 MEM–LIT, och 82, CMH–SDF 1871/1671, är fortsatt undanhållna. Äldre forskning och tidigare bolags-/flygplatsposter behålls oförändrade.

## Validering och filer

Alla 20 Python-tester och aktualitetskontroller passerar. Körresultaten redovisas i `automated_checks_batch90.json`. Separat beräkning av UTC, trafikdagar och periodöverlapp stämmer med alla 37 nya rörelser. UE-kompilering, spelkörning och paketerad build är inte utförda här. Paketet förutsätter den redan kompilerade `PaintAirplane`-rättelsen från `f23b73a`.

- `baton_rouge_batch90.tsv`: samtliga 38 källrader och importbeslut.
- `baton_rouge_source_evidence_batch90.json`: källsidor, regler och avgränsning.
- `airport_locations_batch90.json`: ny flygplatsmarkör och historiskt belägg.
- `batch_90.json` och `validation_batch90.json`: importresultat, UTC-intervall och uppehåll.
- `WORLDWIDE_COVERAGE_BATCH90.md`: aktuell operatörsöversikt.
