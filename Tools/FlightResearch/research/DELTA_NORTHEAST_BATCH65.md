# Delta – Atlanta och nordöstra USA, v66 / omgång 65

123 nya planerade rörelser från 55 scheman på tio nya riktade platspar. Totalt 4 252 rörelser: 4 251 scheduled och en tidigare confirmed. Delta har nu 525 rörelser. Alla 4 129 tidigare rörelseposter och alla tidigare scheman är oförändrade, inklusive tidigare Southwest-rättelser. Flygtrafikmenyn, flygplanssilhuetten och spelkoden är bevarade.

## Granskat urval

Deltas tidtabell gäller från 1 februari 1986. Atlanta–Baltimore, Bradley/Hartford, LaGuardia, Newark och Pittsburgh har lästs i båda riktningarna. Varje riktning har egna tidtabellsbelägg. 56 källrader finns i delta_northeast_batch65.tsv: 55 nya nonstop-scheman och en utesluten anslutning via LaGuardia.

| Från | Till | Nya rörelser |
|---|---|---:|
| ATL | BDL | 9 |
| ATL | BWI | 11 |
| ATL | EWR | 11 |
| ATL | LGA | 20 |
| ATL | PIT | 11 |
| BDL | ATL | 9 |
| BWI | ATL | 11 |
| EWR | ATL | 12 |
| LGA | ATL | 18 |
| PIT | ATL | 11 |

## Trafikdagar, stopp och datum

- Stoppkolumn 0 krävs för import av en nonstopsträcka. ATL–BDL-raden 106/1658 kl. 07:04–10:55 har anslutning via LGA och importeras inte som obruten sträcka. DL106 ATL–LGA kl. 07:04–08:59 finns bland de separat belagda nonstopraderna. LGA–BDL eller dess faktiska pendeloperatör härleds inte från anslutningsraden.
- New York-tabellens suffix L betyder LaGuardia och E Newark. De läses på varje rad även i returriktningen; inget JFK-flyg läggs till genom antagande.
- DL1014 ATL–BWI och DL710 ATL–LGA har X6, alltså alla dagar utom lördag. DL399 BWI–ATL och DL371 LGA–ATL har X7, ingen söndag. Övriga accepterade rader har tomt frekvensfält och daglig trafik.
- DL1014 ATL–BWI 22:58–00:20 och DL660 ATL–PIT 23:08–00:30 anländer nästa lokala kalenderdag. Alla andra nya scheman anländer samma lokala dag.
- DL107 LGA–ATL har ankomst 12:24 efter avgång 10:10; minutsiffrorna är visuellt kontrollerade.

Teckenförklaringen s. 4 anger daglig trafik om frekvensfältet är tomt, 1=måndag–7=söndag och X=utom. Stjärnor avser lågtrafikpriser. Trianglar och pendelbolagens nummerområden hålls isär från Delta; alla accepterade nummer här är under 1200.

Samtliga 55 accepterade scheman ger rörelser. Lokala avgångsdatum: 27 februari 27, 28 februari 55 och 1 mars 41. Båda ändpunkterna använder Eastern Standard Time UTC−5. Intervallöverlapp mot 1986-02-27T22:21:30Z–1986-03-01T22:21:30Z avgör urvalet. Hela flygtider bevaras vid fönstergränserna. De nya schemalagda tiderna är 82–142 minuter. Tidtabellens tidszonskarta och lokaltidskonvention ligger till grund; separat all-times-local-text har inte återfunnits.

## Flygplatsidentitet

Fem flygplatser tillkommer. Koder och namn är kontrollerade mot tidtabellens s. 260–261 och historikkällor. Koordinater från OurAirports anger ungefärliga flygfältslägen, inte 1986 års gater, bantrösklar eller individpositioner.

| Kod 1986 | Flygplats | Avgränsning |
|---|---|---|
| BWI | Baltimore–Washington International | Baltimore-rubriken avser BWI, skild från Washington National/DCA och Dulles/IAD. Senare Thurgood Marshall-namn används inte. |
| BDL | Hartford/Springfield–Bradley International | Windsor Locks-flygplatsen, inte Hartford-Brainard/HFD. |
| LGA | New York–LaGuardia | Suffix L, skild från JFK och Newark. |
| EWR | Newark International | Suffix E. Historiskt namn från 1986 utan senare Liberty-tillägg. |
| PIT | Greater Pittsburgh International | Skild från äldre Allegheny County Airport/AGC. |

Historikkällor och exakta koordinatkällor finns per flygplats i catalog.json samt delta_source_evidence_batch65.json. Newark- och Pittsburgh-historiken bygger på sökindexerade textavsnitt från respektive flygplatsmyndighets PDF och den samtida Delta-kodtabellen; fullständiga historik-PDF:er kunde inte hämtas. Det sammanlagda registret innehåller nu 235 flygplatser/platser och 919 riktade platspar.

## Begränsningar och fortsättning

Tidtabellen belägger planerad trafik, inte genomförande, förseningar, individflygplan eller verkliga flygbanor. Den övergripande utgåvans slutdatum och senare ändringsblad är inte fastställda. Likadana flygnummer kan visa en fortsatt tidtabellsresa men bevisar inte samma individflygplan; servicegrupper mellan importerade omgångar har inte slagits samman.

Detta urval täcker de lästa nonstopraderna för fem flygplatser i båda riktningarna till/från Atlanta under vår period. Det är inte en fullständig regional eller Delta-kartläggning. Fler inrikesorter från Atlanta och returtrafik återstår, liksom Dallas/Fort Worth, Cincinnati och pendeloperatörer. DL475 och DL705 med otydliga minuter från v64 ligger fortsatt utanför katalog och animation.

Norden är oförändrat: 161 unika ändpunktsrörelser; Sverige 87, Norge 52, Danmark 85, Finland 4, Island 0. Europakön behåller Cypern. 94 registrerade operatörer, 48 med rörelser och 46 utan. Inget bolag eller land är verifierat fullständigt. Registret är inte en global bolagsinventering för 1986; global täckningsprocent saknar fastställd nämnare.

## Källor och validering

- https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201 — Digital Library of Georgia, original från Delta Flight Museum.
- https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf — 136 PDF-sidor. Tidtabellsrader på tryckta s. 14, 15, 16, 17, 22, 100, 173 och 195; flygplatskoder s. 260–261; teckenförklaring s. 4 och vinterkarta PDF-sida 2.

Original-PDF och skannade bilder omdistribueras inte. URL, SHA-256, sidmappning och flygplatsunderlag finns i delta_source_evidence_batch65.json. validation_batch65.json redovisar oberoende kalender/UTC-kontroll, dagkoder, dubbletter, gamla datas bevarande, 20 Python-tester, tre byggkontroller och börsdatavalidering. Unreal-kompilering och Editor-körning har inte utförts här.
