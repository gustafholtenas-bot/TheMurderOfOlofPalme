# Memphis södra förbindelser – v83 / forskningsomgång 80

64 nya planerade Delta-rörelser från 31 nya nonstop-scheman. Databasen innehåller nu **5 159 rörelser**: 5 158 enligt tidtabell och en tidigare dokumenterat genomförd rörelse. Alla v82:s 5 095 rörelseposter och 3 186 scheman är oförändrade.

| Förbindelse | Till Memphis | Från Memphis | Nya rörelser totalt |
|---|---:|---:|---:|
| Houston Intercontinental | 8 | 9 | 17 |
| New Orleans | 8 | 9 | 17 |
| Jackson, Mississippi | 6 | 7 | 13 |
| Little Rock | 7 | 5 | 12 |
| Shreveport | 2 | 3 | 5 |
| **Totalt** | **31** | **33** | **64** |

Varje riktning har granskats från sin egen tabell. Returer härleds inte från utgående flyg. Jackson (JAN) och Shreveport (SHV) tillkommer som flygplatser; inga nya operatörer registreras.

## Originalkälla

Delta Air Lines System Timetable, effective February 1, 1986. Delta Flight Museum / Digital Library of Georgia.

- Arkivpost: https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201
- PDF: https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf
- Avgångar till Memphis: tryckta s. 104, 114, 132, 171 och 226.
- Avgångar från Memphis: tryckta s. 146–148.
- Trafikdagar och symboler: tryckt s. 4.
- Flygnummer och linjeföljder: tryckta s. 262–264.

43 rader i tio riktningstabeller har granskats visuellt. 31 nonstop-rader godtas. Elva anslutningsförslag importeras inte som direkta flygsträckor. En nonstop-rad med motsägande tider hålls utanför importen. Originalets a/p-noteringar, flygplatssuffix, trafikdagar, stoppkolumn och importbeslut finns i `delta_memphis_south_batch80.tsv`. Originalskanningen omdistribueras inte.

Houston-tabellens I betyder Intercontinental (IAH), medan H betyder Hobby (HOU). Alla accepterade Houston-rader gäller IAH. HOU förekommer endast i uteslutna anslutningsförslag i detta urval. Tomt frekvensfält betyder dagligen; X6 undantar lördag. Frekvensfält kopieras från den enskilda raden, inte från grannrader. Little Rock–Memphis 1156 är daglig enligt sin egen rad.

## Tidskonflikt som inte importeras

Memphis–Little Rock 597 anges på tryckt s. 147 med **7:51p–6:25p**, X6 och noll stopp. Båda flygplatserna använder UTC−6. Ankomsten ligger därför före avgången om tiderna avser samma dag. Originalskanningen har kontrollerats i förstoring; den visar dessa tider.

Raden finns endast i forskningsunderlaget, med beslutet `held_time_conflict`. Den ingår varken i schemakatalogen eller i spelets rörelser. Ingen gissad tidsrättelse eller ankomst följande dag används. Linjeföljden bekräftar MEM–LIT men löser inte tidskonflikten. Ytterligare samtida underlag behövs innan denna avgång kan importeras. Se `source_conflicts_batch80.json`.

## Tider, trafikdagar och delsträckor

Fönstret är 1986-02-27 22:21:30 UTC till 1986-03-01 22:21:30 UTC. Hela flygintervallet sparas när det överlappar fönstret. Samtliga nya accepterade ändpunkter använder UTC−6 under perioden och alla accepterade ankomster sker samma lokala kalenderdag.

Nya avgångar per lokal kalenderdag: 27 februari 12, 28 februari 31, 1 mars 21. Rörelser som redan pågår vid fönstrets början tas med. Ett dagligt schema kan därför ge tre överlappande rörelser.

Genomgående resor hålls uppdelade vid mellanstopp. Följande anslutningar mellan separat källbelagda ben har kontrollerats för 28 februari:

| Flyg | Separata ben | Markuppehåll |
|---|---|---:|
| 1100 | Houston–Memphis–Detroit | 31 minuter |
| 1124 | Houston–Memphis–Indianapolis | 30 minuter |
| 581 | Indianapolis–Memphis–New Orleans | 31 minuter |
| 334 | Jackson–Memphis–Cincinnati | 39 minuter |
| 547 | Chicago–Memphis–Jackson | 33 minuter |
| 1123 | Indianapolis–Memphis–Jackson | 27 minuter |
| 647 | Detroit–Memphis–New Orleans | 34 minuter |
| 409 | Atlanta–Memphis–Little Rock | 33 minuter |
| 718 | Little Rock–Memphis–Atlanta | 38 minuter |
| 208 | Shreveport–Memphis–Atlanta | 41 minuter |
| 537 | Chicago–Memphis–Shreveport | 35 minuter |
| 692 | New Orleans–Memphis–Indianapolis | 26 minuter |
| 1122 | New Orleans–Memphis–Detroit | 32 minuter |
| 1189 | Chicago–Memphis–Houston | 30 minuter |

Samma flygnummer är inte bevis för ett bestämt individflygplan. Varje accepterat ben har egen källbelagd avgång, ankomst och frekvens.

## Nya flygplatser på kartan

Jackson använder etiketten **Jackson–Allen C. Thompson Field (1986)**. Flygplatsmyndighetens historik beskriver flytten från Hawkins Field till den nya flygplatsen 1963 och namnbytet till Medgar Evers 2005. Ett arkiverat vykort från 1963 benämner platsen Allen C. Thompson Field/Jackson Municipal Airport.

- Historik: https://jmaa.com/our-history/
- Samtida arkivobjekt: https://da.mdah.ms.gov/series/deepsouth/detail/192285
- Ungefärlig position: 32.311199, −90.075897, https://ourairports.com/airports/KJAN/

Shreveport använder **Shreveport Regional**. Flygplatsens historik anger att kommersiell trafik flyttades från Downtown 1952 och att namnet Regional infördes 1971.

- Historik: https://flyshreveport.com/history/
- Ungefärlig position: 32.444747, −93.826741, https://ourairports.com/airports/KSHV/

Koordinaterna är moderna ungefärliga positioner för historiskt kontrollerade flygplatsområden. De är inte exakta rekonstruktioner av 1986 års banor, terminaler eller flygplanspositioner. Se `airport_locations_batch80.json`.

## Validering

- 20 befintliga Python-tester passerar.
- Genererad flyg-JSON samt land- och Europaindex passerar sina aktualitetskontroller.
- Separat transkriberade 24-timmarstider, fasta UTC-offsetar och kalenderdagar ger exakt samma 64 rörelser som kompilatorn med IANA-tidszoner.
- Inga tidsöverlapp mellan berörda ben med samma operatör/flygnummer.
- Fjorton övergångar mellan separat källbelagda ben har kontrollerats för 28 februari.
- Alla tidigare rörelseobjekt och schemarader är oförändrade.
- Houston-suffixen är kontrollerade och den olösta 597-raden är inte importerad.

Full UE 5.8-kompilering, spelkörning och paketerad build har inte utförts här. Detta är ett kumulativt datapaket för befintlig flygmeny med tidigare kompilerad kraschfix f23b73a. C++-filer och byggfiler ingår inte.

## Täckning och fortsatt arbete

244 flygplatser/platser och 1 048 riktade platspar. Delta har 1 177 rörelser. Totalt 95 registrerade operatörer: 49 med rörelser och 46 utan. Inget bolag är verifierat fullständigt; ingen säker global täckningsprocent kan anges.

Nästa urval kan komplettera Memphis–St Louis och Memphis–Louisville i båda riktningarna, separat källbelagda delsträckor via Jackson/Shreveport eller COMAIR Cincinnati–Louisville/Lexington. Europakön behåller Cypern.

Tidtabellen visar planerad trafik. Genomförande, förseningar, passagerare, last, individflygplan och verklig flygbana kräver andra belägg. Utgåvans slutdatum och senare ändringar är inte verifierade.

## Filer

- `delta_memphis_south_batch80.tsv`: samtliga 43 granskade rader och importbeslut.
- `delta_memphis_south_source_evidence_batch80.json`: källor, sidmappning och PDF-checksumma.
- `source_conflicts_batch80.json`: den olösta 597-raden.
- `airport_locations_batch80.json`: plats- och namnunderlag för Jackson och Shreveport.
- `batch_80.json` och `validation_batch80.json`: rörelser, kalender/UTC-kontroll, tidigare data och kontrollerade markuppehåll.
- `WORLDWIDE_COVERAGE_BATCH80.md`: uppdaterad bolagsinventering.
