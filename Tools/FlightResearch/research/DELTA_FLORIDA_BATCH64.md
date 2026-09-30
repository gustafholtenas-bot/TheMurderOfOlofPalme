# Delta – Atlanta och fem Florida-flygplatser, v65 / omgång 64

137 nya planerade rörelser från 67 scheman på tio nya riktade platspar. Totalt 4 129 rörelser: 4 128 scheduled och en tidigare confirmed. Delta har nu 402 rörelser. Alla 3 992 tidigare rörelseposter och alla tidigare scheman är oförändrade, inklusive tidigare Southwest-rättelser. Flygtrafikmenyn, flygplanssilhuetten och spelkoden är bevarade.

## Granskat urval

Deltas tidtabell gäller från 1 februari 1986. Atlanta–Fort Lauderdale, Fort Myers, Jacksonville, Orlando och West Palm Beach har lästs i båda riktningarna. Varje riktning har egna tidtabellsbelägg. 72 källrader finns i delta_florida_batch64.tsv: 67 nya scheman, tre genomgående rader med mellanstopp, en söndagsvariant och en framtida start. 137 nya rörelser fördelas så här:

| Från | Till | Nya rörelser |
|---|---|---:|
| ATL | FLL | 15 |
| ATL | JAX | 17 |
| ATL | MCO | 17 |
| ATL | PBI | 10 |
| ATL | RSW | 10 |
| FLL | ATL | 14 |
| JAX | ATL | 15 |
| MCO | ATL | 17 |
| PBI | ATL | 12 |
| RSW | ATL | 10 |

## Trafikdagar, stopp och datum

- Stoppkolumn 0 krävs för import av en nonstopsträcka. ATL–FLL DL261, ATL–PBI DL807 och FLL–ATL DL596 har ett mellanstopp och importeras inte som obrutna flygsträckor. Deras enskilda delsträckor måste beläggas separat. ATL–MCO DL261, ATL–JAX DL807 och MCO–ATL DL596 finns bland de accepterade delsträckorna.
- DL434 ATL–JAX 16:49–17:45 har X6, alltså alla dagar utom lördag. DL435 har samma tider men trafikdag 6, endast lördag. Varianterna skapar ingen dubbel rörelse samma dag.
- DL763 ATL–JAX och DL764 JAX–ATL 14:45–15:45 har X7, ingen söndag. DL764:s söndagsvariant 14:55–15:55 sparas som forskningsrad utan katalogschema eller rörelse eftersom söndag ligger utanför de lokala avgångsdatumen.
- DL158 MCO–ATL 14:30–15:49 har fotnot 6, start 2 mars 1986, och utesluts. DL94 14:30–15:50 har fotnot 14 på s. 183: sista trafikdag 1 mars. DL94 importeras därför för aktuell period; DL158 ersätter inte DL94 i februari.
- DL261 ATL–MCO 23:01–00:05 anländer nästa lokala kalenderdag. Alla övriga nya scheman anländer samma lokala dag.

Teckenförklaringen s. 4 anger daglig trafik om frekvensfältet är tomt, 1=måndag–7=söndag och X=utom. Stjärnor avser lågtrafikpriser. Trianglar och pendelbolagens nummerområden hålls isär från Delta; alla accepterade nummer här är under 1200.

Samtliga 67 accepterade scheman ger rörelser. Lokala avgångsdatum: 27 februari 26, 28 februari 66 och 1 mars 45. Båda ändpunkterna använder Eastern Standard Time UTC−5. Intervallöverlapp mot 1986-02-27T22:21:30Z–1986-03-01T22:21:30Z avgör urvalet. Hela flygtider bevaras vid fönstergränserna. De nya schemalagda tiderna är 51–100 minuter. Tidtabellens tidszonskarta och lokaltidskonvention ligger till grund; separat all-times-local-text har inte återfunnits.

## Flygplatsidentitet

Fem flygplatser tillkommer. Koder och namn är kontrollerade mot tidtabellens s. 260–261 och flygplatsernas historik. Koordinater från OurAirports anger ungefärliga flygfältslägen, inte 1986 års gater, bantrösklar eller individpositioner.

| Kod 1986 | Flygplats | Avgränsning |
|---|---|---|
| FLL | Fort Lauderdale–Hollywood International | Skild från Miami/MIA. |
| RSW | Southwest Florida Regional, Fort Myers | RSW öppnade 1983. Page Field/FMY används inte. Historiskt Regional-namn bevaras. |
| JAX | Jacksonville International | Den flygplats som öppnade 1968, inte äldre Imeson. |
| MCO | Orlando International | MCO, skild från Orlando Executive/ORL. |
| PBI | Palm Beach International, West Palm Beach | Kod och namn från 1986 bevaras även om dagens koordinatkälla visar senare namn/kod. |

Historikkällor och exakta koordinatkällor finns per flygplats i catalog.json samt delta_source_evidence_batch64.json. Det sammanlagda registret innehåller nu 230 flygplatser/platser och 909 riktade platspar.

## Begränsningar och fortsättning

Tidtabellen belägger planerad trafik, inte genomförande, förseningar, individflygplan eller verkliga flygbanor. Den övergripande utgåvans slutdatum och senare ändringsblad är inte fastställda. Likadana flygnummer kan visa en fortsatt tidtabellsresa men bevisar inte samma individflygplan; servicegrupper mellan importerade omgångar har inte slagits samman.

Detta urval täcker de lästa nonstopraderna för fem Florida-flygplatser i båda riktningarna till/från Atlanta under vår period. Det är inte en fullständig Florida- eller Delta-kartläggning. Fler inrikesorter från Atlanta och returtrafik återstår, liksom Dallas/Fort Worth, Cincinnati och pendeloperatörer. DL475 och DL705 med otydliga minuter från v64 ligger fortsatt utanför katalog och animation.

Norden är oförändrat: 161 unika ändpunktsrörelser; Sverige 87, Norge 52, Danmark 85, Finland 4, Island 0. Europakön behåller Cypern. 94 registrerade operatörer, 48 med rörelser och 46 utan. Inget bolag eller land är verifierat fullständigt. Registret är inte en global bolagsinventering för 1986; global täckningsprocent saknar fastställd nämnare.

## Källor och validering

- https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201 — Digital Library of Georgia, original från Delta Flight Museum.
- https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf — 136 PDF-sidor. Tidtabellsrader på tryckta s. 15, 17, 18, 81, 84, 115, 182 och 247; fotnoter s. 183; flygplatskoder s. 260–261; teckenförklaring s. 4 och vinterkarta PDF-sida 2.

Original-PDF och skannade bilder omdistribueras inte. URL, SHA-256, sidmappning och flygplatsunderlag finns i delta_source_evidence_batch64.json. validation_batch64.json redovisar oberoende kalender/UTC-kontroll, dagkoder, specialdatum, dubbletter, gamla datas bevarande, 20 Python-tester, tre byggkontroller och börsdatavalidering. Unreal-kompilering och Editor-körning har inte utförts här.
