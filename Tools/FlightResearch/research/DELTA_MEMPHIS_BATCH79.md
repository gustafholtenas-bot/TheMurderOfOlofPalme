# Memphis – v82 / forskningsomgång 79

94 nya planerade Delta-rörelser från 45 nya nonstop-scheman. Databasen innehåller nu **5 095 rörelser**: 5 094 enligt tidtabell och en tidigare dokumenterat genomförd rörelse. Hela v81:s 5 001 rörelseposter och 3 141 scheman är oförändrade, inklusive tidigare rättelser.

| Förbindelse | Till Memphis | Från Memphis | Nya rörelser totalt |
|---|---:|---:|---:|
| Atlanta | 14 | 17 | 31 |
| Dallas/Fort Worth | 8 | 9 | 17 |
| Chicago O’Hare | 8 | 9 | 17 |
| Cincinnati | 4 | 4 | 8 |
| Detroit | 4 | 4 | 8 |
| Indianapolis | 6 | 7 | 13 |
| **Totalt** | **44** | **50** | **94** |

Varje riktning har granskats från sin egen tryckta tabell. Returer härleds inte från utgående flyg. Memphis International (MEM) tillkommer som flygplats; inga nya operatörer registreras.

## Originalkälla

Delta Air Lines System Timetable, effective February 1, 1986. Delta Flight Museum / Digital Library of Georgia.

- Arkivpost: https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201
- PDF: https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf
- Avgångar till Memphis: tryckta s. 16, 48, 51, 65, 74 och 111.
- Avgångar från Memphis: tryckta s. 145–147.
- Trafikdagar och symboler: tryckt s. 4.
- Flygnummer och linjeföljder: tryckta s. 262–264.

82 rader i tolv valda riktningstabeller har granskats visuellt. 45 nonstop-rader godtas. 29 anslutningsförslag och åtta genomgående rader med ett stopp importeras inte som direkta flygsträckor. Originalets a/p/n-noteringar, trafikdagar, stoppkolumn och importbeslut finns i `delta_memphis_batch79.tsv`. Originalskanningen omdistribueras inte.

Accepterade rader saknar daterade fotnoter. Tomt frekvensfält betyder dagligen, X6 betyder alla dagar utom lördag och X7 alla dagar utom söndag. En daterad fotnot på det uteslutna anslutningsförslaget Detroit–Memphis 619/479 är dokumenterad separat och skapar ingen direktsträcka.

## Tider, trafikdagar och delsträckor

Fönstret är 1986-02-27 22:21:30 UTC till 1986-03-01 22:21:30 UTC. Flygets hela tid sparas när dess intervall överlappar fönstret.

Memphis, Dallas/Fort Worth och Chicago använder UTC−6. Atlanta, Cincinnati, Detroit och Indianapolis använder UTC−5 under importperioden. Alla accepterade ankomster är samma lokala kalenderdag. Lokal klockskillnad är därför inte alltid samma sak som flygtid: Atlanta–Memphis 409 anges 08:48–08:55, vilket motsvarar 67 minuter.

Nya avgångar per lokal kalenderdag: 27 februari 20, 28 februari 45, 1 mars 29. Lördag undantas uttryckligen för X6-rader. Rörelser som redan pågår vid fönstrets början tas med, vilket gör att ett dagligt schema ibland ger tre överlappande rörelser.

Genomgående resor hålls uppdelade vid mellanstopp. Exempel, kontrollerade för 28 februari:

| Flyg | Separat källbelagda ben | Markuppehåll |
|---|---|---:|
| 273 | Atlanta–Memphis–Chicago | 26 minuter |
| 305 | Atlanta–Memphis–Atlanta | 33 minuter |
| 334 | Memphis–Cincinnati–Cleveland | 48 minuter |
| 427 | Atlanta–Memphis–Indianapolis–Detroit | 26 respektive 25 minuter |
| 581 | Detroit–Indianapolis–Memphis | 29 minuter |
| 658 | Dallas/Fort Worth–Memphis–Chicago | 38 minuter |
| 661 | Detroit–Memphis–Dallas/Fort Worth | 28 minuter |
| 692 | Memphis–Indianapolis–Detroit | 25 minuter |
| 1123 | Detroit–Indianapolis–Memphis | 25 minuter |
| 1124 | Memphis–Indianapolis–Detroit | 20 minuter |

Flyg 305:s tryckta linjeföljd är uttryckligen Atlanta–Memphis–Atlanta. Första benet har X6 medan returen anges dagligen; frekvenserna kopieras från respektive rad. Samma flygnummer är inte bevis för ett bestämt individflygplan.

Memphis–Detroit 692, 1124 och 427 har mellanlandning i Indianapolis. Cincinnati–Memphis 1101 går via Louisville. Detroit–Memphis 581 och 1123 går via Indianapolis. Dessa genomgående rader används inte som extra direktsträckor.

## Memphis på kartan

1986 års namn **Memphis International** används. Flygplatsens egen historik anger namnbytet till International 1969 och kontinuitet från terminalen som öppnades 1963. FAA:s moderna flygplatsreferenspunkt används som en ungefärlig position för samma flygplats, inte som en exakt rekonstruerad bana, terminal eller flygplansposition från 1986.

- Flygplatshistorik: https://flymemphis.com/airport-history/
- FAA, AD 2.2.1: https://www.faa.gov/air_traffic/publications/atpubs/aip_html/part3_ad_2.0_tennessee.html
- Källkoordinater: 35-02-32.681N / 89-58-36.045W.
- Geografisk redovisning: `airport_locations_batch79.json`.

## Validering

- 20 befintliga Python-tester passerar.
- Genererad flyg-JSON och både land- och Europaindex passerar sina aktualitetskontroller.
- Separat transkriberade 24-timmarstider, fasta UTC-offsetar och kalenderdagar ger exakt samma 94 rörelser som kompilatorn med IANA-tidszoner.
- Inga nya dubbletter eller tidsöverlapp mellan berörda ben med samma operatör/flygnummer.
- Elva övergångar mellan separat källbelagda ben kontrolleras för 28 februari.
- Alla tidigare rörelseobjekt och schemarader är oförändrade.

Full UE 5.8-kompilering, spelkörning och paketerad build har inte utförts här. Detta är ett kumulativt datapaket för befintlig flygmeny med tidigare kompilerad kraschfix. C++-filer och byggfiler ingår inte.

## Täckning och fortsatt arbete

242 flygplatser/platser och 1 038 riktade platspar. Delta har 1 113 rörelser. Totalt 95 registrerade operatörer: 49 med rörelser och 46 utan. Inget bolag är verifierat fullständigt; ingen säker global täckningsprocent kan anges.

Nästa urval kan komplettera Memphis–Houston/Jackson/New Orleans/Little Rock/Shreveport eller COMAIR Cincinnati–Louisville/Lexington. Europakön behåller Cypern.

Tidtabellen visar planerad trafik. Genomförande, förseningar, passagerare, last, individflygplan och verklig flygbana kräver andra belägg. Utgåvans slutdatum och senare ändringar är inte verifierade.

## Filer

- `delta_memphis_batch79.tsv`: samtliga 82 granskade rader och importbeslut.
- `delta_memphis_source_evidence_batch79.json`: källor, sidmappning och PDF-checksumma.
- `airport_locations_batch79.json`: Memphis plats- och namnunderlag.
- `batch_79.json` och `validation_batch79.json`: rörelser, kalender/UTC-kontroll, tidigare data och kontrollerade markuppehåll.
- `WORLDWIDE_COVERAGE_BATCH79.md`: uppdaterad bolagsinventering.
