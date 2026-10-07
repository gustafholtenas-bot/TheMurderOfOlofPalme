# LIAT, St. Thomas och Tortola – v126 / batch123

Granskat 1 oktober 2026. **75 nya planerade flygrörelser med LIAT**, totalt **8 772 rörelser**. St. Thomas ökar från **åtta till 14** rörelser. Tortola/Beef Island tillkommer med **åtta**. Alla **8 697 tidigare rörelseobjekt och 5 111 tidigare scheman är oförändrade**.

51 nya scheman är granskade; 37 skapar trafik i spelfönstret. 21 riktade flygplatspar är granskade, 18 har nya rörelser och 16 av dessa är nya i databasen. LIAT registreras som nytt bolag, EIS som ny flygplats och Brittiska Jungfruöarna som nytt forskningsområde.

## Nya rörelser

| Sträcka | Antal |
|---|---:|
| ANU–EIS | 2 |
| ANU–SKB | 10 |
| ANU–SXM | 6 |
| EIS–SJU | 2 |
| EIS–SKB | 2 |
| SJU–EIS | 2 |
| SJU–SXM | 2 |
| SKB–ANU | 12 |
| SKB–STX | 2 |
| SKB–SXM | 6 |
| STT–SXM | 2 |
| STX–SKB | 2 |
| STX–SXM | 4 |
| SXM–ANU | 6 |
| SXM–SJU | 2 |
| SXM–SKB | 7 |
| SXM–STT | 4 |
| SXM–STX | 2 |

St. Thomas har fyra nya ankomster från St. Maarten och två nya avgångar dit. Dessa sex LIAT-rörelser kompletterar de åtta tidigare Pan Am-rörelserna. Tortola har två rörelser vardera Antigua–Tortola, Tortola–San Juan, San Juan–Tortola och Tortola–St. Kitts.

## Källa och dateringsbegränsning

[LIAT Agency–Interliners–Employees Working Timetable, W.T.No.40](https://www.timetableimages.com/ttimages/li/li8512/li8512.pdf), tillgänglig via [utgåvesidan](https://www.timetableimages.com/ttimages/li8512.htm). Originalet har åtta skanningar och saknar tryckta sidnummer. Hänvisningarna nedan är skanning och flygkolumn från vänster.

Omslaget anger **13 december**, W.T.No.40 och att W.T.39 ersätts. **Årtalet 1985 är arkivets datering**, också antecknat för hand på skanning 2; det är inte ett fullständigt tryckt datum. [Airline Timetable Images förteckning](https://www.timetableimages.com/ttimages/li.htm) daterar utgåvan till december 1985. Tabellens särskilda perioder 13 januari–26 mars och från 1 februari används för att välja rätt vinterkolumner.

**Hela utgåvan har inget tryckt slutdatum.** Något senare ändringsblad för målperioden har inte fastställts i denna genomgång. Det är en begränsning i ändringstäckningen, inte bevis för att ändringar saknades. AirTimes förteckning hoppar över denna utgåva och kan inte användas för att fastställa dess slutdatum.

Originalbilderna på skanningarna 2–5 har granskats visuellt. Tider anges uttryckligen som lokala. Endast LIAT:s ben med båda ändpunkterna bland **ANU, SKB, SXM, EIS, STX, STT och SJU** ingår här. Det är ett avgränsat nordligt urval, inte hela bolagets nät.

## Läsregler och operatörer

Kolumnerna läses nedåt. Varje importerat ben går mellan **två på varandra följande tidsatta flygplatsanrop**. Ett tidigare startställe och en slutdestination får inte slås ihop över tidsatta mellanlandningar. Tomma rutor och vertikala genomgångsstreck skapar inte extra landningar.

Källans utrustningsnyckel anger BNI=Britten-Norman Islander (9 platser), DHT=Twin Otter (19) och HS7=BAe Super 748 (44). Koderna bevaras som tidtabellsuppgifter; individflygplan eller registrering är inte fastställda.

Sidorna 6–8 har separata rubriker för **Four Island Air, Inter-Island Air Services och Montserrat Air Services**. De har inte importerats under LIAT trots att deras flygnummer också använder LI. Dessa sidor är nästa forskningsspår och kräver egen operatörs- och ruttgranskning.

## Importerade scheman

Alla tider är lokala. D=dagligen, X=utom följande dagar; 1=måndag … 7=söndag. Sifferkoderna normaliserar källans engelska veckodagar. Källperiod per kolumn finns i JSON-filen.

| Flyg | Fysiskt ben | Lokal tid | Dagar | Skanning/kolumn | Rörelser |
|---|---|---|---|---|---:|
| LI506 | ANU → SKB | 05:35–06:05 | D | 2/1 | 2 |
| LI504 | ANU → SXM | 07:00–07:55 | X34 | 2/8 | 2 |
| LI550 | ANU → SKB | 08:15–08:45 | D | 2/10 | 2 |
| LI550 | SKB → SXM | 08:55–09:25 | D | 2/10 | 2 |
| LI550 | SXM → STT | 09:35–10:20 | D | 2/10 | 2 |
| LI510 | ANU → EIS | 08:30–09:45 | D | 2/13 | 2 |
| LI510 | EIS → SJU | 09:55–10:30 | D | 2/13 | 2 |
| LI540 | ANU → SXM | 09:30–10:25 | X14 | 2/15 | 2 |
| LI540 | SXM → STX | 10:35–11:25 | X14 | 2/15 | 2 |
| LI552 | ANU → SKB | 10:35–11:05 | X34 | 2/17 | 2 |
| LI552 | SKB → STX | 11:15–12:05 | X34 | 2/17 | 2 |
| LI548 | ANU → SXM | 11:15–12:10 | 14 | 2/22 | 0 |
| LI548 | SXM → EIS | 12:20–13:10 | 14 | 2/22 | 0 |
| LI542 | ANU → SXM | 11:15–12:10 | X14 | 3/1 | 2 |
| LI542 | SXM → STT | 12:20–13:15 | X14 | 3/1 | 2 |
| LI512 | ANU → SKB | 13:40–14:10 | D | 3/6 | 2 |
| LI512 | SKB → SXM | 14:20–14:50 | D | 3/6 | 2 |
| LI512 | SXM → SJU | 15:00–15:55 | D | 3/6 | 2 |
| LI544 | ANU → SKB | 16:35–17:05 | D | 3/14 | 2 |
| LI544 | SKB → SXM | 17:15–17:45 | D | 3/14 | 2 |
| LI560 | ANU → SKB | 16:45–17:15 | 7 | 3/15 | 0 |
| LI560 | SKB → EIS | 17:25–18:15 | 7 | 3/15 | 0 |
| LI560 | ANU → SKB | 18:40–19:10 | 26 | 3/19 | 0 |
| LI560 | SKB → EIS | 19:20–20:10 | 26 | 3/19 | 0 |
| LI558 | ANU → SKB | 19:30–20:00 | 7 | 3/23 | 0 |
| LI507 | SKB → ANU | 06:10–06:40 | D | 4/5 | 2 |
| LI503 | SXM → SKB | 08:05–08:35 | X34 | 4/9 | 2 |
| LI503 | SKB → ANU | 08:45–09:15 | X34 | 4/9 | 2 |
| LI555 | STX → SXM | 10:50–11:35 | D | 4/13 | 2 |
| LI555 | SXM → SKB | 11:45–12:15 | D | 4/13 | 2 |
| LI555 | SKB → ANU | 12:25–12:55 | D | 4/13 | 2 |
| LI541 | STX → SXM | 11:50–12:40 | X14 | 4/17 | 2 |
| LI541 | SXM → ANU | 12:50–13:45 | X14 | 4/17 | 2 |
| LI553 | STX → SKB | 13:00–13:50 | X4 | 4/20 | 2 |
| LI553 | SKB → ANU | 14:00–14:30 | X4 | 4/20 | 2 |
| LI511 | SJU → EIS | 13:15–13:50 | D | 4/21 | 2 |
| LI511 | EIS → SKB | 14:00–14:50 | D | 4/21 | 2 |
| LI511 | SKB → ANU | 15:00–15:30 | D | 4/21 | 2 |
| LI543 | STT → SXM | 13:40–14:35 | X14 | 5/1 | 2 |
| LI543 | SXM → ANU | 14:45–15:40 | X14 | 5/1 | 2 |
| LI549 | EIS → SXM | 13:45–14:35 | 14 | 5/2 | 0 |
| LI549 | SXM → ANU | 14:45–15:40 | 14 | 5/2 | 0 |
| LI545 | SXM → SKB | 18:00–18:30 | D | 5/15 | 3 |
| LI545 | SKB → ANU | 18:40–19:10 | D | 5/15 | 2 |
| LI513 | SJU → SXM | 18:30–19:25 | D | 5/16 | 2 |
| LI513 | SXM → ANU | 19:35–20:20 | D | 5/16 | 2 |
| LI561 | EIS → SKB | 18:35–19:30 | 7 | 5/17 | 0 |
| LI561 | SKB → ANU | 19:40–20:10 | 7 | 5/17 | 0 |
| LI559 | SKB → ANU | 20:10–20:40 | 7 | 5/18 | 0 |
| LI561 | EIS → SKB | 20:30–21:25 | 26 | 5/20 | 0 |
| LI561 | SKB → ANU | 21:35–22:05 | 26 | 5/20 | 0 |

Se `caribbean_source_rows_batch123.json` för fullständiga hänvisningar, utrustning och datumvillkor. Fjorton granskade scheman har inga rörelser i fönstret; de skapar ingen påhittad trafik.

## Säsongsvarianter och tidsgränser

- **LI540/541:** varianterna från 13 januari används. Jul- och tidiga januaridagar ger inte andra veckodagar i februari.
- **LI552/553:** kolumnerna 13 januari–26 mars används. Julhelgsvarianter och 27 mars–6 april hålls utanför importen.
- **LI560/561:** kolumnerna från 1 februari går tisdag/lördag. De tidigare tisdag/fredag-kolumnerna gäller till 31 januari och får inte skapa flygningar fredag 28 februari. Lördagskvällens korrekta variant börjar efter spelfönstret.
- **LI548/549:** måndag/torsdag; torsdagsflygen avslutas före fönstrets början. Söndagsvarianterna för LI558/559/560/561 ligger också utanför.

Spelfönstret är **27 februari 1986 22:21:30 UTC – 1 mars 22:21:30 UTC**. Alla sju ändpunkter använder UTC−4 under perioden. Samtliga **75 UTC-intervall** stämmer med separat transkriberade varaktigheter och uttryckliga avgångsdatum, jämförda med generatorns historiska tidszoner.

LI545 St. Maarten–St. Kitts går 22:00–22:30 UTC och överlappar både vänstergränsen på torsdag och högergränsen på lördag. Hela intervallet behålls. Lördagens fortsättning St. Kitts–Antigua börjar 22:40 UTC och ligger utanför. LI513 San Juan–St. Maarten börjar på lördag 22:30 UTC, 8 minuter 30 sekunder efter slutgränsen, och tas därför inte med den dagen.

**38 daterade övergångar inom 19 benkedjor** har kontrollerats mot originalet: samtliga markuppehåll är tio minuter. Inga tidsöverlapp finns mellan accepterade ben med samma LIAT-flygnummer. Samma nummer bevisar inte samma individflygplan.

## Tortola och geografisk avgränsning

**EIS = Tortola – Beef Island (1986)**. IATA-koden EIS är uttryckligen angiven bredvid Tortola i den samtida tabellen. Flygplatsen ligger på Beef Island och får denna äldre geografiska benämning; det senare hedersnamnet Terrance B. Lettsome förs inte bakåt till 1986. [BVI Airports Authority](https://www.bviaacloud.com/corporate/about-us/airport-authority) identifierar Beef Island som plats för huvudflygplatsen.

America/Tortola används som tidszon. OurAirports-koordinater är ungefärliga flygfältsmarkörer, inte inmätta terminal- eller banpositioner från 1986. Senare utbyggnader rekonstrueras inte. Befintliga ANU, SKB, SXM, STX, STT och SJU är oförändrade. Brittiska Jungfruöarna är ett territorium, inte en ny självständig stat. Se `airport_location_review_batch123.json`.

Den tidigare avgränsningen med **29 forskningsområden** behålls: **169 → 244 unika regionala rörelser**, **76 → 92 riktade par**, **23 → 24 regionala flygplatser**.

| Område med trafik, samt Nicaragua | v125 | v126 |
|---|---:|---:|
| Nicaragua | 0 | 0 |
| Guatemala | 4 | 4 |
| Amerikanska Jungfruöarna / St. Thomas, St. Croix | 15 | 31 |
| Brittiska Jungfruöarna | 0 | 8 |
| Puerto Rico | 18 | 26 |
| Jamaica | 4 | 4 |
| Haiti | 6 | 6 |
| Dominikanska republiken | 4 | 4 |
| Bahamas | 27 | 27 |
| Caymanöarna | 38 | 38 |
| Turks- och Caicosöarna | 4 | 4 |
| Antigua och Barbuda | 1 | 37 |
| Saint Kitts och Nevis | 3 | 44 |
| Saint Lucia | 2 | 2 |
| Barbados | 14 | 14 |
| Trinidad och Tobago | 9 | 9 |
| Guadeloupe och dåvarande franska karibiska områden | 17 | 17 |
| Martinique | 19 | 19 |
| Nederländska Antillerna (1986) | 14 | 55 |

Områdesrader och flygplatssummor överlappar och får inte adderas. Ändpunkterna visar inte vilka länder som faktiskt överflögs.

## Luckor och fortsättning

**Nicaragua saknar fortfarande verifierade klockrader för målperioden.** Simulatorplaner märkta Aeronica 1986 men baserade på sommaren 1987 och en nyhet från juni 1986 har inte accepterats som februariunderlag. Noll betyder datalucka, inte frånvaro av historisk trafik.

Nästa läsbara underlag är LIAT:s sydliga och centrala ökedjor på samma skanningar samt de tre separat rubricerade operatörerna på sidorna 6–8. Dominicas två flygplatser DOM och DCF måste hållas isär. Fler bolag och trafikslag kring St. Thomas återstår. Cayman Express-frågorna, Bahamasairs vinterinlaga, Air Jamaicas större nät, Republic EXP, Arrow Air-dateringen och Air France F27-frågan är fortsatt öppna.

Inga militär-, stats- eller privatflyg har tillkommit. Mellanösternurvalet är oförändrat med 72 rörelser. Inget verifierat globalt slutantal eller procent färdig finns. Se `central_america_caribbean_queue_batch123.json` och `caribbean_withheld_batch123.json`.

## Kontroller och installation

Totalt **8 772 rörelser**: 8 771 planerade och en tidigare bekräftad. **5 162 katalogscheman / 5 141 granskade**, **369 flygplatser/platser**, **114 länder/territorier**, **1 714 riktade par**. **106 registrerade operatörer, 63 med rörelser** (62 civila och US Marine Corps), 43 utan. 90 källposter och 32 bidragande tidtabellsutgåvor. Täckningen är partiell.

**20 befintliga Python-tester och tre generatorkontroller passerar.** Äldre rörelser, scheman, kod, källposter, rättelser och hållna konflikter är bevarade. PDF och bilder återdistribueras inte; adresser och kontrollsummor finns i källbevisen.

Stäng Unreal Editor och slå samman ZIP-filens `Plugins/` och `Tools/` med projektroten bredvid `.uproject`. Detta är ett kumulativt datapaket; `PaintAirplane`-rättelsen **f23b73a måste redan vara kompilerad**. C++-bygge, Unreal Editor och paketerat spel har inte körts här. Tidtabeller visar planerad trafik, inte bekräftat genomförande.
