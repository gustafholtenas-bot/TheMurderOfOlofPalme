# Delta – västra USA och Houston till Atlanta, v68 / omgång 67

57 nya planerade rörelser från 26 scheman på tio nya riktade platspar. Totalt 4 432 rörelser: 4 431 scheduled och en tidigare confirmed. Delta har nu 705 rörelser. Alla 4 375 tidigare rörelseposter, scheman och flygplatser är oförändrade. Flygtrafikmenyn, flygplanssilhuetten, spelkoden och tidigare Southwest-rättelser bevaras.

## Granskat urval

Returtrafik till Atlanta från Denver Stapleton, Houston Intercontinental och Hobby, Las Vegas, Los Angeles, Ontario, Phoenix, San Diego, San Francisco och Seattle. Tiderna har lästs separat i Deltas tidtabell från 1 februari 1986. Motsatta riktningar finns i tidigare omgångar och används inte för att härleda returer.

42 källrader finns i delta_western_returns_batch67.tsv: 26 nya nonstop-scheman, tio anslutningar, fem genomgående rader med mellanstopp och ett framtida nonstopflyg.

| Från | Till | Nya rörelser |
|---|---|---:|
| DEN-STAPLETON | ATL | 9 |
| HOU | ATL | 7 |
| IAH | ATL | 10 |
| LAS | ATL | 2 |
| LAX | ATL | 9 |
| ONT | ATL | 2 |
| PHX | ATL | 4 |
| SAN | ATL | 3 |
| SEA | ATL | 4 |
| SFO | ATL | 7 |

## Datum, tidszoner och avgränsningar

Alla accepterade scheman har tomt frekvensfält, alltså daglig trafik enligt s. 4. DL68 LAX–ATL 15:00–21:47 har fotnot 6 på s. 135 och börjar först 2 mars. Även anslutningen 500/132 från LAX har denna fotnot. Ingen av dem importeras. PHX 1012/386 har fotnot 4 på s. 193, start 12 februari, men utesluts ändå som anslutning.

DL40 LAX–ATL 23:20–06:02, DL52 SFO–ATL 23:00–06:03 och DL866 SEA–ATL 22:50–05:55 anländer nästa lokala kalenderdag. DL504 PHX–ATL 00:55–05:52 anländer samma dag. Övriga accepterade scheman anländer samma dag.

Denver och Phoenix använder UTC−7, Houston UTC−6 och övriga västliga avgångsflygplatser UTC−8. Atlanta använder UTC−5. Den exakta perioden är 1986-02-27T22:21:30Z–1986-03-01T22:21:30Z. Intervallöverlapp avgör urvalet; hela tider bevaras vid gränserna. Lokala avgångar: 27 februari 11, 28 februari 26, 1 mars 20. Varje accepterat schema ger rörelser; flygtider 99–257 minuter.

Houston-tabellens I/H avser Intercontinental/IAH respektive Hobby/HOU. Los Angeles-tabellens L/O avser LAX respektive Ontario/ONT. SFO-rubriken anger San Francisco International, inte Oakland eller San Jose. Denvers historiska Stapleton-post används. Alla flygplatsposter bevaras oförändrade.

IAH–ATL DL100, LAS–ATL DL748, SAN–ATL DL860 och DL504 samt SEA–ATL DL874 har ett mellanstopp och importeras inte som obrutna sträckor. HOU–ATL DL100, ONT–ATL DL860 och PHX–ATL DL504 är separat belagda nonstopdelar. Detta bevisar inte samma individflygplan. Tio anslutningsrader via DFW/SAN sparas enbart som forskningsunderlag; mellanliggande sträckor härleds inte.

## Begränsningar och fortsättning

Tidtabellen belägger planerad trafik, inte genomförande, förseningar, individflygplan eller verklig flygbana. Utgåvans övergripande slutdatum och senare ändringar är inte fastställda. Tidszonskartan och lokaltidskonvention används; separat all-times-local-text har inte återfunnits. Servicegrupper mellan omgångar har inte slagits samman.

Registret har 235 flygplatser och 937 riktade platspar. Norden är oförändrat med 161 unika ändpunktsrörelser (Sverige 87, Norge 52, Danmark 85, Finland 4, Island 0). Europakön behåller Cypern. 94 registrerade operatörer, 48 med rörelser och 46 utan; detta är inte en fullständig global inventering. Fler Atlanta-returer och nya orter återstår, liksom övriga Delta-knutpunkter och pendeltrafik. Otydliga DL475/705 från v64 hålls fortsatt utanför.

## Källor och validering

- https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201 — Digital Library of Georgia, original från Delta Flight Museum.
- https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf — 136 PDF-sidor. Rader på tryckta s. 71, 103, 124, 134, 193, 213, 214 och 222; fotnoter s. 135 och 193; legend s. 4; vinterkarta PDF-sida 2.

Originalskanningar omdistribueras inte. delta_source_evidence_batch67.json innehåller källadresser, SHA-256 och sidmappning. validation_batch67.json redovisar oberoende kalender/UTC-kontroll, dubbletter och gamla datas bevarande, 20 Python-tester, tre byggkontroller och börsdatavalidering. Unreal-kompilering och Editor-körning har inte utförts här.
