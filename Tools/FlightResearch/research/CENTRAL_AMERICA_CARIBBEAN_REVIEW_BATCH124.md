# LIAT i södra och centrala Karibien – v127 / batch124

Granskat 2 oktober 2026. **146 nya planerade flygrörelser med LIAT**, totalt **8 918 rörelser**. **32 nya riktade sträckor** och **fem nya flygplatser**. Alla **8 772 tidigare rörelseobjekt och 5 162 tidigare scheman är oförändrade**.

104 nya scheman har granskats; 82 skapar rörelser i spelfönstret och 22 saknar rörelser där. 37 riktade flygplatspar är granskade, 32 är aktiva och samtliga 32 är nya i databasen. LIAT har nu sammanlagt **155 scheman och 221 rörelser**. Inget nytt bolag har registrerats i denna omgång.

## Nya rörelser

| Riktad sträcka | Rörelser |
|---|---:|
| ANU–DCF | 4 |
| ANU–FDF | 2 |
| ANU–PTP | 6 |
| ANU–SLU | 2 |
| BGI–FDF | 2 |
| BGI–GND | 4 |
| BGI–SLU | 8 |
| BGI–SVD | 6 |
| DCF–FDF | 6 |
| DCF–PTP | 6 |
| DOM–ANU | 2 |
| FDF–ANU | 2 |
| FDF–DCF | 4 |
| FDF–DOM | 2 |
| FDF–SLU | 12 |
| GND–BGI | 3 |
| GND–POS | 5 |
| GND–SVD | 6 |
| POS–GND | 5 |
| POS–SLU | 1 |
| POS–SVD | 2 |
| PTP–ANU | 9 |
| PTP–DCF | 6 |
| SLU–ANU | 2 |
| SLU–BGI | 11 |
| SLU–FDF | 9 |
| SLU–POS | 1 |
| SLU–SVD | 2 |
| SVD–BGI | 5 |
| SVD–GND | 4 |
| SVD–POS | 4 |
| SVD–SLU | 3 |

Flygrörelser räknas per daterat fysiskt ben. En riktad sträcka kan ha flera flygrörelser. Genomgående resor över tidsatta mellanlandningar räknas inte som extra direktflyg.

## Källa och dateringsbegränsning

[LIAT Agency–Interliners–Employees Working Timetable, W.T.No.40](https://www.timetableimages.com/ttimages/li/li8512/li8512.pdf), via [utgåvesidan](https://www.timetableimages.com/ttimages/li8512.htm). Åtta skanningar utan tryckta sidnummer; hänvisningarna nedan anger skanning och flygkolumn från vänster.

Omslaget anger **13 december**, W.T.No.40 och att W.T.39 ersätts. **1985 är arkivets datering**, också antecknat för hand på skanning 2. [Utgåveförteckningen](https://www.timetableimages.com/ttimages/li.htm) daterar tidtabellen till december 1985. De tryckta vinterperioderna används för att välja rätt kolumner för februari 1986.

**Inget slutdatum för hela utgåvan är tryckt.** Ändringsblad eller en ersättande utgåva för målperioden har inte fastställts. Det begränsar ändringstäckningen. Importen ska inte tolkas som en fullständig inventering av faktiskt genomförd trafik.

Originalbilderna på skanningarna 2–5 har granskats visuellt. Denna omgång gäller återstående utvalda **LIAT-flyg med 300-nummer** i södra och centrala nätet. LI117/126 och separat rubricerade öbolag på skanningarna 6–8 är inte importerade.

## Läsregler och datumval

Tiderna anges uttryckligen som lokala. Kolumnerna läses nedåt; endast två på varandra följande tidsatta flygplatsanrop bildar ett importerat ben. Tomma rutor och genomgångsstreck skapar inte landningar.

- **LI350:** kolumnerna från 13 januari används. Onsdag och övriga dagar behålls som separata, ömsesidigt uteslutande scheman även när första benets klockslag är lika.
- **LI398, LI320/354/352/397/321/351/353:** tryckta perioder omfattar inte måldagarna. Dessa varianter importeras inte.
- **LI334/316/335/317/337:** perioden 17 februari–26 mars används. Andra jul-/nyårs- och 6–16 februari-kolumner hålls utanför.
- **LI336:** kolumn 21 gäller 13 januari–26 mars, tisdag/fredag/lördag. Kolumn 22 gäller annan period. Söndagsvarianten i kolumn 25 granskas utan att skapa rörelser i fönstret.

Källans utrustningsnyckel anger BNI=Britten-Norman Islander, DHT=Twin Otter och HS7=BAe Super 748. Uppgifterna sparas som tidtabellskoder; de fastställer inte ett individflygplan eller faktiskt använd utrustning.

## Importerade scheman

D=dagligen, X=utom angivna dagar; 1=måndag … 7=söndag. Alla klockslag är lokala. Giltighetsperiod och utrustning per kolumn finns i `caribbean_source_rows_batch124.json`.

| Flyg | Fysiskt ben | Lokal tid | Dagar | Skanning/kolumn | Rörelser |
|---|---|---|---|---|---:|
| LI350 | GND → SVD | 06:05–06:35 | X3 | 2/2 | 2 |
| LI350 | SVD → BGI | 06:45–07:30 | X3 | 2/2 | 2 |
| LI350 | BGI → SLU | 08:00–08:45 | X3 | 2/2 | 2 |
| LI350 | SLU → ANU | 08:55–10:05 | X3 | 2/2 | 2 |
| LI350 | GND → SVD | 06:05–06:35 | 3 | 2/3 | 0 |
| LI350 | SVD → BGI | 06:45–07:30 | 3 | 2/3 | 0 |
| LI350 | BGI → SLU | 08:00–08:45 | 3 | 2/3 | 0 |
| LI350 | SLU → DOM | 08:55–09:40 | 3 | 2/3 | 0 |
| LI350 | DOM → ANU | 09:50–10:35 | 3 | 2/3 | 0 |
| LI342 | DCF → PTP | 06:45–07:15 | D | 2/6 | 2 |
| LI342 | PTP → ANU | 07:25–07:55 | D | 2/6 | 2 |
| LI314 | BGI → FDF | 07:30–08:40 | D | 2/9 | 2 |
| LI304 | POS → SLU | 08:15–09:30 | 15 | 2/11 | 1 |
| LI304 | SLU → FDF | 09:40–10:05 | 15 | 2/11 | 1 |
| LI302 | POS → GND | 08:15–08:55 | X15 | 2/12 | 1 |
| LI302 | GND → BGI | 09:05–10:00 | X15 | 2/12 | 1 |
| LI340 | SLU → FDF | 08:30–08:55 | D | 2/14 | 2 |
| LI340 | FDF → DCF | 09:05–09:35 | D | 2/14 | 2 |
| LI340 | DCF → PTP | 09:45–10:15 | D | 2/14 | 2 |
| LI340 | PTP → ANU | 10:25–10:55 | D | 2/14 | 2 |
| LI344 | SLU → FDF | 11:15–11:40 | X34 | 2/21 | 2 |
| LI344 | FDF → DOM | 11:50–12:20 | X34 | 2/21 | 2 |
| LI344 | DOM → ANU | 12:30–13:25 | X34 | 2/21 | 2 |
| LI364 | GND → SVD | 11:30–12:05 | D | 3/2 | 2 |
| LI364 | SVD → SLU | 12:15–12:45 | D | 3/2 | 2 |
| LI322 | POS → SVD | 12:45–13:45 | 15 | 3/3 | 1 |
| LI322 | SVD → SLU | 13:55–14:40 | 15 | 3/3 | 1 |
| LI330 | POS → GND | 13:00–13:40 | 15 | 3/4 | 1 |
| LI330 | GND → BGI | 14:00–14:55 | 15 | 3/4 | 1 |
| LI330 | POS → GND | 13:00–13:40 | X15 | 3/5 | 1 |
| LI330 | GND → SVD | 14:00–14:30 | X15 | 3/5 | 1 |
| LI330 | SVD → BGI | 14:40–15:25 | X15 | 3/5 | 1 |
| LI324 | POS → SVD | 13:55–14:55 | 2 | 3/7 | 0 |
| LI324 | SVD → BGI | 15:05–15:50 | 2 | 3/7 | 0 |
| LI380 | CCS → BGI | 14:30–17:00 | 47 | 3/10 | 0 |
| LI348 | BGI → SLU | 14:55–15:50 | D | 3/11 | 2 |
| LI348 | SLU → FDF | 16:00–16:25 | D | 3/11 | 2 |
| LI348 | FDF → DCF | 16:35–17:05 | D | 3/11 | 2 |
| LI348 | DCF → PTP | 17:20–17:50 | D | 3/11 | 2 |
| LI348 | PTP → ANU | 18:00–18:30 | D | 3/11 | 3 |
| LI346 | PTP → ANU | 15:20–16:15 | X34 | 3/12 | 2 |
| LI334 | POS → GND | 18:35–19:15 | 2567 | 3/16 | 1 |
| LI334 | GND → BGI | 19:25–20:20 | 2567 | 3/16 | 1 |
| LI356 | BGI → SLU | 19:00–19:45 | D | 3/20 | 2 |
| LI356 | SLU → FDF | 19:55–20:20 | D | 3/20 | 2 |
| LI356 | FDF → ANU | 20:30–21:30 | D | 3/20 | 2 |
| LI336 | POS → SVD | 19:15–20:15 | 256 | 3/21 | 1 |
| LI336 | SVD → BGI | 20:25–21:10 | 256 | 3/21 | 1 |
| LI316 | POS → GND | 19:35–20:15 | 134 | 3/24 | 1 |
| LI316 | GND → SVD | 20:25–20:55 | 134 | 3/24 | 1 |
| LI316 | SVD → BGI | 21:05–21:50 | 134 | 3/24 | 1 |
| LI336 | POS → SVD | 19:45–20:45 | 7 | 3/25 | 0 |
| LI336 | SVD → BGI | 20:55–21:40 | 7 | 3/25 | 0 |
| LI318 | BGI → SLU | 20:00–20:55 | D | 3/26 | 2 |
| LI341 | ANU → DCF | 05:40–06:35 | D | 4/1 | 2 |
| LI343 | ANU → PTP | 05:50–06:20 | D | 4/2 | 2 |
| LI343 | PTP → DCF | 06:30–07:00 | D | 4/2 | 2 |
| LI343 | DCF → FDF | 07:10–07:40 | D | 4/2 | 2 |
| LI343 | FDF → SLU | 07:50–08:15 | D | 4/2 | 2 |
| LI301 | BGI → GND | 06:00–06:55 | D | 4/3 | 2 |
| LI301 | GND → POS | 07:05–07:45 | D | 4/3 | 2 |
| LI313 | SLU → BGI | 06:15–07:10 | D | 4/6 | 2 |
| LI333 | ANU → DOM | 07:30–08:15 | 3 | 4/7 | 0 |
| LI333 | DOM → SLU | 08:25–09:10 | 3 | 4/7 | 0 |
| LI333 | SLU → BGI | 09:20–10:05 | 3 | 4/7 | 0 |
| LI333 | BGI → SVD | 10:35–11:20 | 3 | 4/7 | 0 |
| LI333 | SVD → POS | 11:30–12:30 | 3 | 4/7 | 0 |
| LI333 | ANU → SLU | 08:00–09:10 | X3 | 4/8 | 2 |
| LI333 | SLU → BGI | 09:20–10:05 | X3 | 4/8 | 2 |
| LI333 | BGI → SVD | 10:35–11:20 | X3 | 4/8 | 2 |
| LI333 | SVD → POS | 11:30–12:30 | X3 | 4/8 | 2 |
| LI347 | ANU → PTP | 08:20–08:50 | X34 | 4/10 | 2 |
| LI347 | PTP → DCF | 09:00–09:30 | X34 | 4/10 | 2 |
| LI347 | DCF → FDF | 09:40–10:10 | X34 | 4/10 | 2 |
| LI347 | FDF → SLU | 10:20–10:45 | X34 | 4/10 | 2 |
| LI363 | FDF → SLU | 09:10–09:35 | D | 4/12 | 2 |
| LI363 | SLU → SVD | 09:45–10:15 | D | 4/12 | 2 |
| LI363 | SVD → GND | 10:25–11:00 | D | 4/12 | 2 |
| LI303 | FDF → SLU | 10:25–10:50 | 15 | 4/14 | 1 |
| LI303 | SLU → POS | 11:00–12:15 | 15 | 4/14 | 1 |
| LI381 | BGI → CCS | 11:15–13:45 | 47 | 4/15 | 0 |
| LI323 | BGI → SVD | 11:30–12:15 | 2 | 4/16 | 0 |
| LI323 | SVD → POS | 12:25–13:25 | 2 | 4/16 | 0 |
| LI367 | SLU → BGI | 13:20–14:15 | D | 4/22 | 2 |
| LI349 | ANU → DCF | 14:15–15:10 | X34 | 5/3 | 2 |
| LI355 | ANU → FDF | 16:00–17:00 | D | 5/7 | 2 |
| LI355 | FDF → SLU | 17:10–17:35 | D | 5/7 | 2 |
| LI355 | SLU → BGI | 17:45–18:30 | D | 5/7 | 3 |
| LI355 | BGI → SVD | 20:50–21:35 | D | 5/7 | 2 |
| LI355 | SVD → GND | 21:45–22:15 | D | 5/7 | 2 |
| LI345 | ANU → PTP | 16:00–16:30 | D | 5/8 | 2 |
| LI345 | PTP → DCF | 16:40–17:10 | D | 5/8 | 2 |
| LI345 | DCF → FDF | 17:25–17:55 | D | 5/8 | 2 |
| LI345 | FDF → SLU | 18:05–18:30 | D | 5/8 | 3 |
| LI345 | SLU → BGI | 18:40–19:35 | D | 5/8 | 2 |
| LI335 | BGI → SVD | 16:10–16:55 | 2567 | 5/9 | 2 |
| LI335 | SVD → POS | 17:05–18:05 | 2567 | 5/9 | 2 |
| LI317 | BGI → SVD | 16:50–17:35 | 134 | 5/11 | 0 |
| LI317 | SVD → GND | 17:45–18:15 | 134 | 5/11 | 0 |
| LI317 | GND → POS | 18:25–19:05 | 134 | 5/11 | 1 |
| LI337 | BGI → GND | 17:00–17:55 | 256 | 5/12 | 2 |
| LI337 | GND → POS | 18:05–18:45 | 256 | 5/12 | 2 |
| LI337 | BGI → GND | 17:30–18:25 | 7 | 5/14 | 0 |
| LI337 | GND → POS | 18:35–19:15 | 7 | 5/14 | 0 |

## Exakt tidsfönster och markuppehåll

Spelfönstret är **27 februari 1986 22:21:30 UTC – 1 mars 22:21:30 UTC**. Alla ändpunkter i denna omgång använder UTC−4 på måldatumen. Samtliga **146 UTC-intervall** har kontrollerats mot separat transkriberade varaktigheter och uttryckliga avgångsdatum. Lokala avgångsdatum: 14 rörelser den 27 februari, 73 den 28 februari och 59 den 1 mars.

LI348 PTP–ANU, LI355 SLU–BGI och LI345 FDF–SLU överlappar både torsdagens startgräns och lördagens slutgräns. Hela flygintervallet behålls. LI317 SVD–GND på torsdag slutar 22:15 UTC, före startgränsen; fortsättningen GND–POS börjar 22:25 UTC och tas med. LI380/381 till/från Caracas skapar inga rörelser: torsdagsflygen slutar före fönstret och söndagen ligger utanför.

**87 nya daterade övergångar inom 49 benkedjemönster** har kontrollerats mot originalet. 75 har tio minuters markuppehåll, fyra har 15 minuter, två har 20 minuter, fyra har 30 minuter och två har 140 minuter. De 38 tidigare övergångarna är oförändrade, totalt 125.

**LI355 landar BGI 18:30 och avgår därifrån 20:50 enligt källan.** Dessa 140 minuter behålls; inget extra flyg har hittats på mellan tiderna. Inga accepterade LIAT-ben med samma flygnummer överlappar i tid. Samma flygnummer bevisar inte samma individflygplan.

## Historiska flygplatser

| Kod | Plats som används i 1986-data | Avgränsning |
|---|---|---|
| GND | Point Salines, Grenada | Senare namn Maurice Bishop används inte bakåt i tiden. |
| SVD | Arnos Vale, Saint Vincent | Gamla flygfältet; inte dagens Argyle. |
| SLU | Vigie, Saint Lucia | Skiljs från Hewanorra/UVF. |
| DOM | Melville Hall, Dominica | Skiljs från Canefield. |
| DCF | Canefield, Dominica | Egen plats nära Roseau. |

SVD använder den stängda OurAirports-posten **VC-0001**, historiskt TVSV. Dagens TVSA/SVD avser Argyle och ger fel position för 1986. [Premiärministerns anförande 2005](https://pmoffice.gov.vc/pmoffice/images/stories/Speeches/the%20international%20airport%20project%20at%20argyle.pdf) skiljer gamla flygplatsen i Arnos Vale från det planerade Argyle. [Flygplatsens egen tidslinje](https://www.svg-airport.com/timeline/operations-begin-at-the-aia/) daterar Argyles trafikstart till 14 februari 2017. Den neutrala benämningen Arnos Vale används utan antagande om när hedersnamnet E. T. Joshua infördes.

[Vigies namnbyte anges av Saint Lucias regering](https://archive.stlucia.gov.lc/pr1997/vigie_airport_renamed_george_f_l__charles.htm) till 4 augusti 1997. Det äldre namnet i originaltabellen behålls. DASPA skiljer [Melville Hall/Douglas–Charles](https://www.domports.daspa.dm/index.php/airports/douglas-charles-airport/) från [Canefield](https://www.domports.daspa.dm/index.php/airports/canefield-airport/). En [återgivning av Grenadas premiärministers anförande vid namnbytet 2009](https://www.grenadianconnection.com/Grenada/ViewNews.asp?CID=15008&Hl=Prime+Minister+speech+at+airport+renaming+ceremony+Grenada&NID=6835&Sch=&cat=0000&yr=2009) beskriver Point Salines som invigd 1984.

**Originalets kodmarginal på skanning 4 upprepar DOM för Canefield.** Det fullständiga flygplatsnamnet och övriga skanningars kodnycklar styr normaliseringen till DCF. Melville Hall behåller DOM. Inga flygningar flyttas mellan de två flygplatserna enbart utifrån den felaktiga marginalkoden.

Koordinaterna är ungefärliga flygfältsmarkörer från OurAirports, inte inmätta terminal- eller banpositioner från 1986. Senare utbyggnader rekonstrueras inte. Alla 369 äldre flygplatsposter är oförändrade. Grenada, Saint Vincent och Grenadinerna samt Dominica tillkommer som forskningsområden; Saint Lucia fanns redan. Se `airport_location_review_batch124.json`.

## Operatörskonflikter och nästa steg

**LI117 och LI126 hålls utanför importen.** LI126 på huvudtabellens skanning 3 förefaller ange MQS–FDF med BNI, medan Inter-Island Air Services på skanning 7 har en annan kedja till BGI med DHT. LI117 på huvudtabellens skanning 5 förefaller ange SVD–GND medan IAS-tabellen börjar BGI–MQS. Rutt, utrustning och operatör måste redas ut innan någon variant accepteras.

Skanningarna 6–8 har separata rubriker för **Four Island Air, Inter-Island Air Services och Montserrat Air Services**. LI-prefixet är inte tillräckligt för att tillskriva deras trafik LIAT. Four Island Air och Montserrat Air Services är nästa läsbara spår; Montserrats historiska flygplats måste kontrolleras innan koordinater läggs in.

**Nicaragua saknar fortfarande verifierade klockrader för målperioden.** Noll betyder datalucka, inte frånvaro av historisk trafik. St. Thomas ligger kvar på **14 rörelser** (åtta Pan Am, sex LIAT) och Tortola på **åtta**. Fler operatörer och trafikslag återstår. Tidigare frågor om Cayman Express, Bahamasairs vinterinlaga, Air Jamaicas större nät, Republic EXP, Arrow Air och Air France F27 kvarstår.

## Regional täckning

Samma 29 forskningsområden som tidigare: **244 → 390 unika regionala rörelser**, **92 → 124 riktade par**, **24 → 29 regionala flygplatser**. Inrikesrörelser inom samma land/territorium ligger kvar på 18.

| Område med trafik, samt Nicaragua | v126 | v127 |
|---|---:|---:|
| Nicaragua | 0 | 0 |
| Guatemala | 4 | 4 |
| Amerikanska Jungfruöarna / St. Thomas, St. Croix | 31 | 31 |
| Brittiska Jungfruöarna | 8 | 8 |
| Puerto Rico | 26 | 26 |
| Jamaica | 4 | 4 |
| Haiti | 6 | 6 |
| Dominikanska republiken | 4 | 4 |
| Bahamas | 27 | 27 |
| Caymanöarna | 38 | 38 |
| Turks- och Caicosöarna | 4 | 4 |
| Antigua och Barbuda | 37 | 66 |
| Saint Kitts och Nevis | 44 | 44 |
| Dominica | 0 | 30 |
| Saint Lucia | 2 | 53 |
| Saint Vincent och Grenadinerna | 0 | 32 |
| Grenada | 0 | 27 |
| Barbados | 14 | 53 |
| Trinidad och Tobago | 9 | 27 |
| Guadeloupe och dåvarande franska karibiska områden | 17 | 44 |
| Martinique | 19 | 58 |
| Nederländska Antillerna (1986) | 55 | 55 |

Områdesrader och flygplatssummor överlappar och får inte adderas. Ändpunkter visar inte vilka länder som faktiskt överflögs. Inga militär-, stats- eller privatflyg har tillkommit. Mellanösternurvalet är oförändrat med 72 rörelser. Inget verifierat globalt slutantal eller färdigprocent finns.

## Kontroller och installation

Totalt **8 918 rörelser**: 8 917 planerade och en tidigare bekräftad. **5 266 katalogscheman / 5 245 granskade**, **374 flygplatser/platser**, **117 länder/territorier**, **1 746 riktade par**. **106 registrerade operatörer, 63 med rörelser** (62 civila och US Marine Corps), 43 utan. Oförändrat 90 källposter och 32 bidragande tidtabellsutgåvor.

**20 befintliga Python-tester och tre generatorkontroller passerar.** Alla äldre rörelseobjekt, scheman, flygplatser, länder och bolag bevaras. En befintlig LIAT-källposts titel, granskningssidlista, granskningsdatum och noter utökas för denna genomgång; URL och giltighetsuppgifter ändras inte. Före/efter finns i `source_metadata_update_batch124.json`. Äldre kod, forskningsrapporter, rättelser och hållna konflikter är oförändrade. Original-PDF och nya skannade bilder återdistribueras inte.

Stäng Unreal Editor och slå samman ZIP-filens `Plugins/` och `Tools/` med projektroten bredvid `.uproject`. Detta är ett kumulativt datapaket; **PaintAirplane-rättelsen f23b73a måste redan vara kompilerad**. C++-bygge, Unreal Editor och paketerat spel har inte körts här. Tidtabeller visar planerad trafik, inte bekräftat genomförande.
