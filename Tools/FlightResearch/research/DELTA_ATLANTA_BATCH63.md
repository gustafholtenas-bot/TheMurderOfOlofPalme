# Delta – ytterligare avgångar från Atlanta, v64 / omgång 63

185 nya planerade rörelser från 87 scheman på 23 nya riktade platspar. Totalt 3 992 rörelser: 3 991 scheduled och en tidigare confirmed. Delta har nu 265 rörelser. Alla 3 807 tidigare rörelseposter och alla tidigare scheman är oförändrade, inklusive Southwest-rättelserna från v63. Flygtrafikmenyn, flygplanssilhuetten och all spelkod är bevarade.

## Källurval och beslut

Deltas tidtabell gäller från 1 februari 1986. Tryckta s. 14–18 har lästs för ett urval av Atlantas inrikes nonstopavgångar. 123 källrader: 87 nya scheman, 33 redan importerade, två rader med otydliga minutangivelser och en framtida start. Raderna och varje importbeslut finns i delta_atlanta_batch63.tsv. De två otydliga raderna är forskningsunderlag utan animerad rörelse eller aktiv katalogkandidat.

Sidans stoppkolumn måste ange 0. Rader med anslutningsnummer eller mellanstopp tas inte in som nonstop. Teckenförklaringen på s. 4 anger daglig trafik om annat inte står, 1=måndag–7=söndag och X=utom. Stjärnan avser särskilda biljettpriser, inte genomförandestatus. Pendelbolagens triangelsymbol och nummerområden skiljs från Delta. Detta urval innehåller endast Delta-huvudlinjer, nummer under 1200.

| Sträcka från Atlanta | Nya rörelser |
|---|---:|
| ATL → AUS-MUELLER | 2 |
| ATL → DCA | 19 |
| ATL → DEN-STAPLETON | 9 |
| ATL → DFW | 20 |
| ATL → DTW | 11 |
| ATL → FWA | 2 |
| ATL → HNL | 2 |
| ATL → HOU | 6 |
| ATL → IAH | 13 |
| ATL → LAS | 2 |
| ATL → LIT | 5 |
| ATL → MCI | 9 |
| ATL → MSY | 13 |
| ATL → OKC | 6 |
| ATL → ONT | 2 |
| ATL → PHL | 13 |
| ATL → PHX | 4 |
| ATL → SAN | 2 |
| ATL → SAT | 6 |
| ATL → STL | 9 |
| ATL → TOL | 4 |
| ATL → TPA | 19 |
| ATL → TUL | 7 |

## Särskilda kontroller

- DL137 Atlanta–Dallas/Fort Worth 12:49–13:55 har trafikdag 5 och fotnot 3: 28 februari–7 mars 1986. Endast fredag 28 februari ger en rörelse i vår period.
- DL181 Atlanta–Los Angeles omfattas av fotnot 6, start 2 mars 1986, och importeras inte.
- DL25 Atlanta–Honolulu 12:02–16:50 har X15, alltså ingen måndag eller fredag. Torsdag och lördag ger två rörelser, båda delvis över fönstrets gränser. Honolulu använder UTC−10.
- Flygnumren DL790 (13:42–15:20) och DL760 (23:12–00:45 nästa dag) till Detroit har kontrollerats mot den separata linjeförteckningen på s. 263. OCR misstolkade siffror; inga sådana OCR-varianter importerades.
- Houston-suffixen I och H är Intercontinental/IAH respektive Hobby/HOU. DL627 går till IAH; DL297 och DL282 går till HOU. DCA är Washington National enligt sidans suffix N, skild från Baltimore och Dulles.
- DL475 Atlanta–Austin har otydliga avgångsminuter (18:46/18:48); DL705 Atlanta–Little Rock har otydliga avgångsminuter (11:46/11:48). De hålls utanför tills bättre läsning eller ytterligare tidtabellsbelägg finns. Alternativen i TSV-filen är läsningsförslag, inte fastställda tider.

Alla 87 godkända nya scheman producerar rörelser. Lokala avgångsdatum: 27 februari 41, 28 februari 86, 1 mars 58. Intervallöverlapp mot 1986-02-27T22:21:30Z–1986-03-01T22:21:30Z avgör urvalet. Hela flygtider bevaras vid fönstrets gränser. DL865 till DFW, DL760 till DTW, DL256 till PHL och DL141 till TPA anländer nästa lokala kalenderdag; övriga nya scheman anländer samma lokala kalenderdag.

Atlanta använder Eastern Standard Time UTC−5. Destinationerna använder sina historiska vinterzoner: Eastern UTC−5, Central UTC−6, Mountain/Arizona UTC−7, Pacific UTC−8 eller Hawaii UTC−10. Fast vinteroffset och ZoneInfo används i oberoende kalenderkontroll. Källans tidszonskarta visar vinterzoner; tider tolkas som lokala enligt tidtabellskonvention. Separat all-times-local-text har inte återfunnits. Austin använder Mueller och Denver Stapleton. Inga flygplatser eller koordinater tillkommer.

## Täckning och fortsättning

Detta är ett riktat urval av avgångar från Atlanta, inte en komplett kartläggning av Delta eller Atlanta. Returflyg antas inte och behöver granskas separat. Fler inrikesorter, Dallas/Fort Worth, Cincinnati och pendelbolagens egna operatörskoder är fortsatta spår. Den övergripande utgåvans slutdatum och ändringsblad är inte fastställda. Tidtabellen belägger planerad trafik, inte faktiskt genomförande, individflygplan eller verkliga flygbanor. Linjeförteckningen används som kontroll av flygnummer och sträcka; den är inte ett bevis på samma individflygplan.

Norden oförändrat: 161 unika ändpunktsrörelser, Sverige 87, Norge 52, Danmark 85, Finland 4 och Island 0. Europakön behåller Cypern. 94 registrerade operatörer, 48 med rörelser och 46 utan. Inget bolag eller land är verifierat fullständigt. Registret är inte en global bolagsinventering för 1986 och någon täckningsprocent kan inte beräknas.

## Källor och validering

- https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201 — Digital Library of Georgia, original från Delta Flight Museum.
- https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf — 136 PDF-sidor, ofta två tryckta sidor per PDF-sida. Tryckta s. 14–18 är PDF-sidor 10–12; teckenförklaring s. 4 är PDF-sida 5; linjeförteckning s. 262–264 finns i PDF-sidor 134–135.

Original-PDF och skannade bilder omdistribueras inte. URL och SHA-256 finns i delta_source_evidence_batch63.json. validation_batch63.json redovisar oberoende kalender/UTC-kontroll, dagkoder, specialdatum, dubbletter, tidigare datas bevarande, 20 Python-tester, tre byggkontroller och börsdatavalidering. Unreal-kompilering och Editor-körning har inte utförts här.
