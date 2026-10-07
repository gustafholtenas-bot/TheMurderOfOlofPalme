# PBA kring Miami, västra Florida och New Orleans – v134 / batch131

Granskat 2 oktober 2026. **83 nya planerade flygrörelser på 15 nya riktade sträckor**, totalt **9 571 rörelser**. Alla nya flyg är **inrikes i USA**. **76 nya granskade utgåvespecifika scheman**, varav 66 ger rörelser och tio har noll utfall i projektfönstret.

Alla **9 488 äldre rörelseobjekt och 5 771 äldre scheman är oförändrade**. Inga flygplats-, land-, operatörs- eller källposter tillkommer eller ändras. Äldre forskningsrapporter och rättelser bevaras byte för byte. PBA har nu **385 rörelser från 399 scheman** i v132–v134 tillsammans.

## Sträckor med nya rörelser

Alla nedanstående riktade par är nya i databasens rörelser.

| Riktad sträcka | Nya rörelser |
|---|---:|
| DAB–MIA | 5 |
| MIA–DAB | 6 |
| MIA–MLB | 7 |
| MIA–PBI | 4 |
| MIA–SRQ | 10 |
| MLB–MIA | 6 |
| MLB–PBI | 1 |
| MSY–PNS | 4 |
| PBI–MIA | 5 |
| PFN–TPA | 5 |
| PNS–MSY | 4 |
| PNS–TLH | 5 |
| SRQ–MIA | 10 |
| TLH–PNS | 5 |
| TPA–PFN | 6 |

DAB = Daytona Beach, MIA = Miami, MLB = Melbourne i Florida, PBI = Palm Beach/West Palm Beach, SRQ = Sarasota–Bradenton, MSY = New Orleans, PNS = Pensacola, PFN = historiska Panama City-flygplatsen, TLH = Tallahassee, TPA = Tampa. **PFN ersätts inte med senare ECP**, och MLB är inte Melbourne i Australien. En rörelse är ett daterat fysiskt flygben.

## Original och avgränsning

Samma två original som i v132/v133:

- [PBA Southern System, giltig från 15 januari 1986](https://flypba.com/timetable/s-1986-01-15/): **53 nya scheman och 70 nya rörelser**, inlagor 1–4. Filnamnet `01-01` ändrar inte omslagets uttryckliga datum 15 januari. Sista tillämpningsdag 28 februari är härledd ur nästa utgåvas start, inte tryckt som slutdatum.
- [PBA Southern System, giltig från 1 mars 1986](https://flypba.com/timetable/s-1986-03-01/): **23 nya scheman och 13 nya rörelser**, inlagor 2–3. Används endast på lördagen i projektfönstret.

Original från **Gordon K. Werners samling, [flyPBA.com](https://flypba.com/timetables/)**. Skanningarna distribueras inte. Alla 13 originalbildfiler har verifierats mot samma SHA-256 som tidigare; URL och hash finns i `caribbean_source_evidence_batch131.json`. Ingen ny källutgåva tillkommer. Ändringsblad och faktisk drift är inte fullständigt verifierade.

Ett tryckt flygnummer utan stopp-/anslutningsanmärkning tolkas som nonstop enligt tabellstrukturen. D betyder tom daganmärkning/daglig trafik; X6 = utom lördag, X7 = utom söndag, X67 = utom lördag och söndag. Uttryckliga stopp, flera nummer och anslutningsflygplats ger inga extra direktflyg. Separat uttrycklig nonstop-/lokaltidslegend har inte återfunnits. Lokala tider följer tidtabellskonvention.

Flygnummer återges numeriskt som tryckta. PBA-attributionen följer publikationen. Easterns bonusreklam gör inte detta till Eastern-flyg. Flygplanstyp, individflygplan, faktisk operatör vid eventuell inhyrning och juridisk drifthistorik är inte fastställda. Tidtabellen visar publicerade planer, inte bekräftade genomföranden.

## Tidszoner och datumgränser

Fönstret är **27 februari 1986 kl. 22:21:30 UTC till 1 mars kl. 22:21:30 UTC**. Ett flyg tas med om dess intervall överlappar fönstret. **MSY, PNS och PFN har UTC−6**; övriga sju flygplatser i tillägget har **UTC−5**, inklusive Tallahassee. Gränserna motsvarar 16:21:30 i Central Time och 17:21:30 i Eastern Time. Offseterna har kontrollerats mot befintliga flygplatsposters tidszoner för samtliga importdatum.

Nya rörelser efter lokal avgångsdag: **torsdag 17, fredag 53, lördag 13**. Januariutgåvan används endast torsdag/fredag. Januariutgåvans Miami–Daytona Beach- och MSY/PNS/PFN-rader förlängs inte till mars. Detta är ingen slutsats om att all faktisk trafik upphörde.

- **Tallahassee–Pensacola** visar samma lokala klockslag vid start och landning, men flygtiden är **60 minuter** i UTC. Motsatt riktning visar två klocktimmar men tar också en faktisk timme.
- **Pensacola–New Orleans 1358**, 15:55–16:55 CST på torsdagen, överlappar startgränsen och ingår. Föregående **TLH–PNS 1358**, som landar 15:45 CST, ligger helt före fönstret den dagen.
- **Tampa–Panama City 1090**, 16:25 EST–16:55 CST på torsdagen, tar **90 minuter** och överlappar startgränsen. Tiderna får inte subtraheras utan tidszonsjustering.
- **Miami–Melbourne 1161**, 16:20–17:20 EST, slutar före torsdagens 17:21:30-gräns och räknas endast fredag/lördag.
- **Miami–Melbourne 1165** har tom daganmärkning/daglig trafik. X6 från anslutande APF–MIA 1165 kopieras inte till detta ben. Marsraden ger ändå noll utfall eftersom avgången 20:15 ligger efter fönstret.

Tio marsrader ger inga rörelser på grund av lördagsundantag och/eller sena avgångar. De behålls som granskade scheman. Orsak för varje rad finns i `caribbean_withheld_batch131.json`. Alla 76 flygtider har kontrollerats, även de tio nollutfallen. Flygtiderna i detta tillägg är 35–90 minuter.

## Anslutningar med äldre ben

**51 daterade följder av angränsande ben med samma nummer** har kontrollerats. **43** ansluter till bevarade v132/v133-ben. Tabellen visar de **35 sträckmönstren** och deras markuppehåll. Denna tidskontroll identifierar inte ett enskilt flygplan.

| Nummer | Benföljd | Markuppehåll, minuter |
|---|---|---:|
| 808 | PBI → TPA → PFN | 10 |
| 1088 | SRQ → TPA → PFN | 10 |
| 1091 | PFN → TPA → SRQ | 10 |
| 1111 | RSW → MIA → PBI | 21 |
| 1112 | PBI → MIA → APF | 20 |
| 1120 | MTH → MIA → DAB | 45 |
| 1121 | DAB → MIA → MTH | 20 |
| 1127 | MLB → MIA → MTH | 15 |
| 1130 | MTH → MIA → DAB | 20 |
| 1131 | DAB → MIA → MTH | 10 |
| 1155 | APF → MIA → MLB | 40 |
| 1156 | MLB → PBI → MIA | 10 |
| 1156 | PBI → MIA → APF | 25 |
| 1161 | APF → MIA → MLB | 35 |
| 1165 | APF → MIA → MLB | 30 |
| 1174 | EYW → MIA → PBI | 10 |
| 1177 | SRQ → MIA → EYW | 60 |
| 1188 | EYW → MIA → DAB | 10 |
| 1189 | PBI → MIA → EYW | 35 |
| 1191 | DAB → MIA → EYW | 10 |
| 1193 | MLB → MIA → EYW | 15 |
| 1301 | PNS → TLH → JAX | 10 |
| 1302 | JAX → TLH → PNS | 15 |
| 1302 | TLH → PNS → MSY | 10 |
| 1303 | MSY → PNS → TLH | 10 |
| 1303 | PNS → TLH → JAX | 10 |
| 1304 | JAX → TLH → PNS | 15 |
| 1356 | JAX → TLH → PNS | 15 |
| 1356 | TLH → PNS → MSY | 10 |
| 1357 | MSY → PNS → TLH | 10 |
| 1357 | PNS → TLH → JAX | 15 |
| 1358 | JAX → TLH → PNS | 15 |
| 1358 | TLH → PNS → MSY | 10 |
| 1359 | MSY → PNS → TLH | 10 |
| 1359 | PNS → TLH → JAX | 10 |

Inga genomgående sammanfattningsrader har lagts ovanpå de fysiska benen. Saknade mellanlandningsklockslag räknas inte fram ur en genomgående resa.

## Transkriberade klockslag

Inlaga = bildfilens `inside-N`; kolumn räknas från vänster. Januariinlagorna har fyra kolumner; mars inlaga 2 har sex. Tiderna är lokala för respektive flygplats. Rörelseantal gäller endast projektfönstret.

### Januariutgåvan – tillämpad 27–28 februari

| Flygnummer | Sträcka | Lokala tider | Dagar | Rörelser | Inlaga/kolumn |
|---|---|---|---|---:|---|
| 1121 | DAB → MIA | 06:30–07:55 | X67 | 1 | 1/1 |
| 1113 | DAB → MIA | 10:15–11:45 | D | 1 | 1/1 |
| 1191 | DAB → MIA | 14:20–15:50 | D | 1 | 1/1 |
| 1131 | DAB → MIA | 18:15–19:45 | D | 2 | 1/1 |
| 1120 | MIA → DAB | 08:25–09:55 | X67 | 1 | 2/3 |
| 1188 | MIA → DAB | 12:35–14:05 | D | 1 | 2/3 |
| 1106 | MIA → DAB | 16:25–17:55 | D | 2 | 2/3 |
| 1130 | MIA → DAB | 19:15–20:45 | D | 2 | 2/3 |
| 1109 | MLB → MIA | 10:00–11:00 | D | 1 | 2/3 |
| 1127 | MLB → MIA | 14:00–15:00 | D | 1 | 2/3 |
| 1193 | MLB → MIA | 17:45–18:45 | D | 2 | 2/3 |
| 1155 | MIA → MLB | 08:20–09:20 | X67 | 1 | 2/4 |
| 1110 | MIA → MLB | 12:30–13:30 | D | 1 | 2/4 |
| 1161 | MIA → MLB | 16:20–17:20 | D | 1 | 2/4 |
| 1165 | MIA → MLB | 20:15–21:15 | D | 2 | 2/4 |
| 1224 | MIA → SRQ | 08:10–09:10 | X7 | 1 | 2/4 |
| 1228 | MIA → SRQ | 11:15–12:15 | D | 1 | 2/4 |
| 1232 | MIA → SRQ | 14:10–15:10 | D | 1 | 2/4 |
| 1236 | MIA → SRQ | 17:35–18:35 | D | 2 | 2/4 |
| 1240 | MIA → SRQ | 20:20–21:20 | X6 | 2 | 2/4 |
| 1223 | SRQ → MIA | 06:45–07:44 | X7 | 1 | 3/4 |
| 1227 | SRQ → MIA | 09:50–10:49 | D | 1 | 3/4 |
| 1177 | SRQ → MIA | 12:30–13:30 | D | 1 | 3/4 |
| 1235 | SRQ → MIA | 15:25–16:25 | D | 1 | 3/4 |
| 1239 | SRQ → MIA | 18:55–19:55 | X6 | 2 | 3/4 |
| 1174 | MIA → PBI | 11:05–11:40 | D | 1 | 2/4 |
| 1111 | MIA → PBI | 19:20–19:55 | X6 | 2 | 2/4 |
| 1156 | PBI → MIA | 07:35–08:10 | X67 | 1 | 4/3 |
| 1189 | PBI → MIA | 11:50–12:25 | D | 1 | 4/3 |
| 1112 | PBI → MIA | 20:10–20:45 | X6 | 2 | 4/3 |
| 1156 | MLB → PBI | 06:45–07:25 | X67 | 1 | 2/3 |
| 1357 | MSY → PNS | 09:30–10:30 | D | 1 | 3/2 |
| 1303 | MSY → PNS | 12:55–13:55 | D | 1 | 3/2 |
| 1359 | MSY → PNS | 17:15–18:15 | D | 2 | 3/2 |
| 1356 | PNS → MSY | 08:00–08:59 | D | 1 | 3/3 |
| 1302 | PNS → MSY | 11:35–12:35 | D | 1 | 3/3 |
| 1358 | PNS → MSY | 15:55–16:55 | D | 2 | 3/3 |
| 1301 | PNS → TLH | 06:30–08:30 | X67 | 1 | 3/3 |
| 1357 | PNS → TLH | 10:40–12:40 | D | 1 | 3/3 |
| 1303 | PNS → TLH | 14:05–16:05 | D | 1 | 3/3 |
| 1359 | PNS → TLH | 18:25–20:25 | D | 2 | 3/3 |
| 1356 | TLH → PNS | 07:50–07:50 | X67 | 1 | 4/1 |
| 1302 | TLH → PNS | 11:25–11:25 | D | 1 | 4/1 |
| 1358 | TLH → PNS | 15:45–15:45 | D | 1 | 4/1 |
| 1304 | TLH → PNS | 19:25–19:25 | D | 2 | 4/1 |
| 1085 | PFN → TPA | 06:00–08:30 | X67 | 1 | 3/3 |
| 1087 | PFN → TPA | 09:45–12:15 | X7 | 1 | 3/3 |
| 1089 | PFN → TPA | 13:20–15:50 | D | 1 | 3/3 |
| 1091 | PFN → TPA | 17:25–19:55 | X6 | 2 | 3/3 |
| 1086 | TPA → PFN | 08:55–09:25 | X67 | 1 | 4/2 |
| 1088 | TPA → PFN | 12:30–12:59 | D | 1 | 4/2 |
| 1090 | TPA → PFN | 16:25–16:55 | D | 2 | 4/2 |
| 808 | TPA → PFN | 20:30–20:59 | X6 | 2 | 4/2 |
### Marsutgåvan – tillämpad endast 1 mars

| Flygnummer | Sträcka | Lokala tider | Dagar | Rörelser | Inlaga/kolumn |
|---|---|---|---|---:|---|
| 1109 | MLB → MIA | 10:00–11:00 | D | 1 | 2/2 |
| 1127 | MLB → MIA | 14:00–15:00 | D | 1 | 2/2 |
| 1193 | MLB → MIA | 17:45–18:45 | D | 0 | 2/2 |
| 1155 | MIA → MLB | 08:20–09:20 | X67 | 0 | 2/2 |
| 1110 | MIA → MLB | 12:30–13:30 | D | 1 | 2/2 |
| 1161 | MIA → MLB | 16:20–17:20 | D | 1 | 2/2 |
| 1165 | MIA → MLB | 20:15–21:15 | D | 0 | 2/2 |
| 1224 | MIA → SRQ | 08:10–09:10 | X7 | 1 | 2/3 |
| 1228 | MIA → SRQ | 11:15–12:15 | D | 1 | 2/3 |
| 1232 | MIA → SRQ | 14:10–15:10 | D | 1 | 2/3 |
| 1236 | MIA → SRQ | 17:35–18:35 | D | 0 | 2/3 |
| 1240 | MIA → SRQ | 20:20–21:20 | X6 | 0 | 2/3 |
| 1223 | SRQ → MIA | 06:45–07:44 | X7 | 1 | 2/5 |
| 1227 | SRQ → MIA | 09:50–10:49 | D | 1 | 2/5 |
| 1177 | SRQ → MIA | 12:30–13:30 | D | 1 | 2/5 |
| 1235 | SRQ → MIA | 15:25–16:25 | D | 1 | 2/5 |
| 1239 | SRQ → MIA | 18:55–19:55 | X6 | 0 | 2/5 |
| 1174 | MIA → PBI | 11:05–11:40 | D | 1 | 2/3 |
| 1111 | MIA → PBI | 19:20–19:55 | X6 | 0 | 2/3 |
| 1156 | PBI → MIA | 07:35–08:10 | X67 | 0 | 3/2 |
| 1189 | PBI → MIA | 11:50–12:25 | D | 1 | 3/2 |
| 1112 | PBI → MIA | 20:10–20:45 | X6 | 0 | 3/2 |
| 1156 | MLB → PBI | 06:45–07:25 | X67 | 0 | 2/2 |

## Avslutad radräkning i dessa två sydsystemutgåvor

En genomgång av alla sju inlagor ger **201 synliga enkelnummer-rader utan stopp-/anslutningsanmärkning i januariutgåvan och 198 i marsutgåvan**. Dessa 399 rader är nu katalogiserade över v132–v134. Antalet per tryckt avgångsort stämmer med katalogen:

| Avgångsort | Januari | Mars |
|---|---:|---:|
| APF | 15 | 17 |
| DAB | 4 | 5 |
| EYW | 17 | 19 |
| GNV | 3 | 5 |
| JAX | 13 | 24 |
| MIA | 42 | 39 |
| MLB | 4 | 5 |
| MSY | 3 | 0 |
| MTH | 6 | 6 |
| PBI | 11 | 11 |
| PFN | 4 | 0 |
| PNS | 7 | 0 |
| RSW | 10 | 16 |
| SRQ | 11 | 11 |
| TLH | 12 | 8 |
| TPA | 39 | 32 |
| **Totalt** | **201** | **198** |

Detta avslutar **den avgränsade radräkningen**, inte hela PBA:s historiska nät eller den faktiska trafiken. Northern System, ändringsblad, inställda flyg, inhyrda flygplan och eventuella saknade källor återstår. En nolla i marskolumnen är en källavgränsning.

**RSW–SRQ 1208** saknar självständig benrad i den granskade Fort Myers-sektionen; RSW–TPA med ett stopp får inte användas för att hitta på Sarasota-tider. Januariutgåvans **APF–RSW** får inte heller härledda mellanlandningstider ur genomgående resor. Dessa luckor finns kvar även när antalet synliga nonstop-rader stämmer. Se `pba_source_sweep_batch131.json`.

## Regional prioritet

Centralamerika/Karibien, samma **31 forskningsområden**, ligger kvar på **532 rörelser, 158 riktade par, 38 flygplatser och 64 inrikesrörelser**. **St. Thomas 14, Tortola 8 och Nicaragua 0** är oförändrade. Mexiko, separat angränsande område, har 20; Mellanösternurvalet har 72.

**Ingen ny regional källsökning gjordes i denna omgång**; arbetet slutförde återstående PBA-rader i redan hämtade original. Nicaragua och St. Thomas är fortsatt prioriterade, och noll är en datalucka, inte historisk trafikfrånvaro. Föregående omgångs TACA/Aviateca-index och TAN SAHSA-arkivspår kvarstår som underlagsluckor/spår. LACSA:s januari-/marsinlagor, Caribbean Express februariutgåva, sjöflygets ankomsttider och hamnidentiteter samt American/Eastern/BWIA behöver fortsatt undersökas. Äldre LIAT-, Challenge-, Arrow- och Airways International-frågor är oförändrade. Inga militär- eller specialrörelser tillkommer.

Fortsatt kö: `central_america_caribbean_queue_batch131.json`.

## Totalt, verifiering och installation

**9 571 rörelser: 9 570 planerade och en tidigare bekräftad. 5 847 katalogscheman, varav 5 826 granskade. 388 flygplatser/platser, 121 länder/territorier, 1 849 riktade par. 111 operatörsposter: 68 med rörelser, 43 utan. 93 källposter och 35 bidragande tidtabellsutgåvor.** Ingen verifierad global bolagsinventering eller färdigprocent finns.

**20 befintliga Python-tester och tre generatorkontroller passerar.** Alla 83 nya UTC-intervall matchar separat beräkning med fasta UTC-offseter, manuellt granskade flygtider och datum. Samtliga 385 PBA-rörelser har kontrollerats för dubbletter och överlappning av samma flygnummer; inga hittades. Detta är separata beräkningar mot samma originalmaterial, inte en andra oberoende källa eller granskare.

Alla äldre objekt och forskningsrapporter/rättelser bevaras. Aktuell README, data och genererade index uppdateras. **Unreal Editor och spelet har inte körts.**

Paketet är kumulativt: stäng Unreal och slå samman `Plugins/` och `Tools/` med projektroten bredvid `.uproject`. **PaintAirplane-rättelsen f23b73a måste redan vara kompilerad.** Ingen ny C++-kod ingår.
