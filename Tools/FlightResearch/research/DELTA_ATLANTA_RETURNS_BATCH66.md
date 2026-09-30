# Delta – åtta retursträckor till Atlanta, v67 / omgång 66

123 nya planerade rörelser från 59 scheman på åtta nya riktade platspar. Totalt 4 375 rörelser: 4 374 scheduled och en tidigare confirmed. Delta har nu 648 rörelser. Alla 4 252 tidigare rörelseposter, scheman och flygplatser är oförändrade. Flygtrafikmenyn, flygplanssilhuetten, spelkoden och tidigare Southwest-rättelser bevaras.

## Granskat urval

Deltas tidtabell gäller från 1 februari 1986. Returtrafiken till Atlanta från Boston, Chicago O’Hare, Dallas/Fort Worth, Detroit, Miami, Philadelphia, Tampa och Washington National har lästs separat. De motsatta riktningarna finns från tidigare omgångar; inga returtider har härletts från dem.

| Från | Till | Nya rörelser |
|---|---|---:|
| BOS | ATL | 14 |
| DCA | ATL | 17 |
| DFW | ATL | 20 |
| DTW | ATL | 10 |
| MIA | ATL | 13 |
| ORD | ATL | 17 |
| PHL | ATL | 13 |
| TPA | ATL | 19 |

80 källrader finns i delta_atlanta_returns_batch66.tsv: 59 nya scheman, tolv exakta matchningar till redan importerade FLL/BWI-scheman, fem genomgående rader med ett stopp, två anslutningar och två lördagsavgångar efter tidsfönstret.

## Dagkoder, datum och tid

- DL138 DFW–ATL 15:50–18:43 har trafikdag 6 och fotnot 5 på s. 65: trafik 1–8 mars 1986. Importen gäller därför endast lördagen 1 mars. Ankomsten ligger efter fönstrets slut men avgången före; hela rörelsen bevaras.
- DL864 DFW–ATL 19:25–22:09 har X6 och ger torsdag/fredag. Lördagsraden DL874 19:28–22:12 avgår 2 mars 01:28 UTC, efter fönstrets slut, och sparas enbart som forskningsrad.
- DL481 DTW–ATL 17:50–19:30 är lördagsavgång efter fönstrets slut, 1 mars 22:50 UTC. Inget schema eller rörelse importeras för den.
- DL295 DTW–ATL 06:00–07:37 har X67 och ger bara fredag inom fönstret. DL476 MIA–ATL 11:00–12:43 har 67, lördag/söndag, och ger bara lördag. DL1162 ORD–ATL har X7, DL719 DCA–ATL X6; övriga nya scheman har daglig trafik.
- DL386 DFW–ATL 21:58–00:37 anländer nästa lokala kalenderdag. Övriga nya scheman anländer samma lokala kalenderdag.

Chicago och Dallas använder Central Standard Time UTC−6; Atlanta och övriga berörda flygplatser Eastern Standard Time UTC−5. Fönstret är exakt 1986-02-27T22:21:30Z–1986-03-01T22:21:30Z. Urval sker med intervallöverlapp och flygtider klipps inte vid gränserna. 22 nya rörelser avgår lokalt 27 februari, 57 den 28 februari och 44 den 1 mars. Flygtiderna är 72–156 minuter. Samtliga 59 importerade scheman ger rörelser.

Teckenförklaringen s. 4 anger tomt frekvensfält=dagligen, 1=måndag–7=söndag och X=utom. Stjärnor markerar lågtrafikpriser. Pendelbolagens nummerområden och trianglar innebär inte automatiskt Delta som faktisk operatör; anslutningar via CVG/DAY importeras inte här.

## Flygplatser, stopp och återanvända rader

Chicago-rubriken anger O’Hare/ORD, Dallas/Fort Worth är DFW. Miami-tabellens M/F betyder MIA/FLL. Washington/Baltimore-tabellens N/I betyder DCA/BWI; I betyder här Baltimore, inte Dulles. Alla flygplatsposter är tidigare granskade och bevaras oförändrade. Sju FLL-rader och fem BWI-rader stämmer med tidigare importerade tider och trafikdagar och skapar inga nya poster.

BOS–ATL DL429 och ORD–ATL DL425 har ett stopp och importeras inte som obrutna sträckor. DL429:s PHL–ATL-del är separat belagd. MIA–ATL DL516, FLL–ATL DL596 och MIA–ATL DL764 har också ett stopp; DL764 är dessutom söndagsbunden. DL516:s TPA–ATL-del är separat belagd. DTW–ATL 1650/805 via CVG och 1721/441 via DAY är anslutningsrader. Mellanliggande sträckor eller faktisk pendeloperatör härleds inte från dessa rader.

## Begränsningar och fortsättning

Detta är planerad trafik, inte belagda genomförda flygningar, förseningar, individflygplan eller verkliga flygbanor. Utgåvans övergripande slutdatum och senare ändringsblad är inte fastställda. Tidszonskartan och lokaltidskonvention används; separat all-times-local-text har inte återfunnits. Samma flygnummer på flera sträckor bevisar inte samma individflygplan; servicegrupper mellan omgångar har inte slagits ihop.

Registret har nu 235 flygplatser och 927 riktade platspar. Norden är oförändrat med 161 unika ändpunktsrörelser (Sverige 87, Norge 52, Danmark 85, Finland 4, Island 0). Europakön behåller Cypern. 94 registrerade operatörer, 48 med rörelser och 46 utan; ingen verifierat komplett global inventering eller täckningsprocent. Fler Atlanta-sträckor och returflyg återstår, liksom Dallas/Fort Worth, Cincinnati och pendeltrafik. Otydliga DL475/705-minuter från v64 ligger fortsatt utanför katalogen.

## Källor och validering

- https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201 — Digital Library of Georgia, original från Delta Flight Museum.
- https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf — 136 PDF-sidor. Rader på tryckta s. 31, 46, 64, 73, 150, 191, 229 och 244; DL138-fotnot s. 65; teckenförklaring s. 4; vinterkarta PDF-sida 2.

Originalskanningar omdistribueras inte. delta_source_evidence_batch66.json anger källadresser, SHA-256 och sidmappning. validation_batch66.json redovisar oberoende kalender/UTC-granskning, dubblettkontroll, bevarade äldre data, 20 Python-tester, tre byggkontroller och börsdatavalidering. Unreal-kompilering och Editor-körning har inte utförts här.
