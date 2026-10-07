# Huntington, Fayetteville och Norfolk – batch102 / v105

Granskat 2026-10-01. Kumulativ datauppdatering från v104.

34 nya nonstop-scheman ger **69 planerade rörelser** i projektets exakta 48-timmarsfönster. Totalt **6 555 rörelser**: 6 554 tidtabellslagda och en tidigare dokumenterat genomförd. Delta får 29, COMAIR 23 och Atlantic Southeast Airlines (ASA) 17 nya rörelser.

| Sträcka, båda riktningarna | Nya scheman | Nya rörelser |
|---|---:|---:|
| Cincinnati–Huntington/Ashland | 9 | 19 |
| Huntington/Ashland–Charleston WV | 3 | 4 |
| Atlanta–Fayetteville/Fort Bragg NC | 8 | 17 |
| Atlanta–Norfolk | 10 | 21 |
| Raleigh/Durham–Norfolk | 4 | 8 |
| Totalt | 34 | 69 |

## Underlag och avgränsning

[Delta Air Lines System Timetable, 1 februari 1986](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201), Delta Flight Museum via Digital Library of Georgia. Tryckta sidor 15, 16, 40, 50, 80, 106, 177, 178 och 202; operatörsnyckel s. 4, flygplatsregister s. 260–261, färdvägar s. 263–265. Original-PDF har SHA256 `3aefa36e86dc1970af760d90385d2a887bcf5f0d7e0260e3a2764924db1835e4`.

Tio kompletta riktningsrubriker omfattar 36 rader: 34 importeras, två bytesförbindelser via RDU utesluts. Inga importerade rader har daterade fotnoter. Varje fysisk delsträcka har en separat tryckt tid. Originalskanningarna följer inte med paketet. Senare tidtabellsändringar, faktiskt genomförande och flygplansidentitet är inte verifierade.

## Trafikdagar och dygn

- 1540 CVG–HTS går alla dagar utom söndag, men HTS–CRW endast lördag. Lördagsuppehållet vid HTS är 15 minuter.
- 1713 CRW–HTS går endast lördag, medan HTS–CVG går alla dagar utom söndag. Lördagsuppehållet är 15 minuter.
- 1572 CRW–HTS–CVG har 10 minuter i HTS. Den befintliga fortsättningen till Dayton har 41 minuter i Cincinnati.
- 1528 CVG–HTS går dagligen även om tidigare ben går måndag–fredag. 1543 HTS–CVG går dagligen medan CVG–DAY undantar lördag.
- ASA 1272 ATL–FAY undantar söndag men returen går dagligen. 1274 ATL–FAY går dagligen men returen undantar lördag. Returerna 1272, 1273 och 1274 har 20, 40 respektive 15 minuters uppehåll.
- Delta 994 RDU–ORF går 23:40–00:15 nästa lokala dag. Två rörelser i fönstret: 04:40–05:15 UTC den 28 februari och 1 mars. Stjärnan anger lågtrafikpris och är ingen dygnssymbol.

Alla nytillkomna ändpunkter använder vintertid UTC−5. Separat avskrift med 24-timmarsklocka, fasta UTC-avvikelser och kalenderdagar matchar samtliga 69 genererade rörelser. Fönstret är 27 februari 22:21:30 till 1 mars 22:21:30 UTC. Tre scheman ger tre överlappande rörelser, 29 ger två och två ger en. Fullständiga tider bevaras även när flyget korsar fönstrets gräns.

## Flygplatser

HTS, FAY och ORF tillkommer. [HTS:s officiella huvudplan, avsnitt 1.2.2](https://www.tristateairport.com/wp-content/uploads/2024/03/HTS-Master-Plan-Study-Final-Report-1.pdf) stöder kontinuiteten från flygplatsens öppnande 1952 och terminalen från 1961. [Fayettevilles egen historia](https://www.fayettevillenc.gov/City-Departments/Airport) skiljer dagens flygplats, öppnad 1949, från den äldre platsen vid Raleigh Road; Concourse B öppnade först 1987. [Norfolks officiella historik](https://www.norfolkairport.com/wp-content/uploads/2021/10/ORF-History-Updated-27Oct21-2.pdf) beskriver nuvarande flygplats från 1938, terminalen från 1974 och namnet International från 1976.

Tidtabellens register identifierar HTS som Tri-State, FAY som Grannis Field och ORF som International. FAY ska inte förväxlas med Fayetteville Arkansas (FYV) eller Pope Field. OurAirports-koordinater används som ungefärliga områdesmarkörer, inte rekonstruktioner av dåtidens gater, terminaler eller banor. Se `airport_locations_batch102.json`.

## Bevarande och validering

Alla 6 486 äldre rörelseobjekt, 3 893 äldre scheman, 287 äldre flygplatser och 96 operatörer är oförändrade. Den tidigare rättelsen av den uteslutna RIC–ATL-raden i batch101 samt äldre flygnummerrättelser och alla tre hållna konflikter bevaras. Ingen ny källkonflikt tillkommer.

Nitton övergångar mellan separat belagda ben med samma operatör och flygnummer har kontrollerade markintervall på 10–65 minuter. Inga berörda flygnummer har överlappande flygtider. Kontrollen bevisar inte att samma individflygplan användes.

Alla 20 Python-tester och aktualitetskontroller passerar. Körresultaten redovisas i `automated_checks_batch102.json`. Oberoende kalender-/UTC-resultat och jämförelser finns i `validation_batch102.json`. Full Unreal-kompilering, spelkörning och paketerad build har inte utförts här.

Totalt finns 3 927 scheman, varav 3 906 granskade, 290 flygplatser/platser och 1 246 riktade platspar. 96 operatörer är registrerade; 50 har rörelser och 46 saknar ännu rörelser. Inget bolag är verifierat fullständigt.

Nästa urval: separat tryckta RDU–DFW/DFW–RDU-ben samt Norfolks övriga direktrutter. Därefter Greensboro/Charlotte eller återstående nordöstra Delta/Ransome-rubriker. Hållna källkonflikter löses först när bättre underlag finns.
