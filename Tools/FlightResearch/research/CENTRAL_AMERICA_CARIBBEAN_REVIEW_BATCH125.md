# Öbolag kring Antigua och Montserrat – v128 / batch125

Granskat 2 oktober 2026. **71 nya planerade flygrörelser**, totalt **8 989 rörelser**. Tidtabellens rubriker tillskriver **30 Four Island Air** och **41 Montserrat Air Services**. **14 nya riktade sträckor och fyra nya flygplatser** tillkommer. Alla **8 918 tidigare rörelseobjekt och 5 266 tidigare scheman är oförändrade**.

69 scheman har granskats: 39 från Four Island Air och 30 från Montserrat Air Services. 44 skapar rörelser i fönstret; 25 skapar inga. 17 riktade flygplatspar har granskats; 14 är aktiva och samtliga är nya i databasen.

## Nya rörelser

| Riktad sträcka | Rörelser |
|---|---:|
| ANU–BBQ | 4 |
| ANU–MNI | 17 |
| ANU–NEV | 4 |
| AXA–SKB | 1 |
| BBQ–ANU | 4 |
| MNI–ANU | 16 |
| MNI–PTP | 2 |
| MNI–SKB | 2 |
| NEV–ANU | 4 |
| NEV–SKB | 6 |
| PTP–MNI | 2 |
| SKB–AXA | 1 |
| SKB–MNI | 2 |
| SKB–NEV | 6 |

Flygrörelser avser daterade fysiska ben. En riktad sträcka kan ha flera avgångar. Antigua–Nevis–St. Kitts är två tidsatta ben och skapar ingen extra direktsträcka över Nevis.

## Källa, datering och operatör

[LIAT W.T.No.40](https://www.timetableimages.com/ttimages/li/li8512/li8512.pdf), med separata rubriker för Four Island Air på **skanning 6** och Montserrat Air Services på **skanning 8**. Sidorna saknar tryckta sidnummer. N och S nedan anger tabellens northbound/southbound-del, följt av kolumn från vänster.

Tryckt giltighetsdatum är **13 december**. Årtalet **1985 är arkivets datering**, även antecknat för hand på skanning 2. [Arkivets utgåveförteckning](https://www.timetableimages.com/ttimages/complete/complete.htm) identifierar de tre öbolagens sidor i samma utgåva. **Inget slutdatum för hela utgåvan är tryckt** och full täckning av ändringsblad är inte fastställd. De särskilda vinterperioderna styr kolumnvalen.

Alla flygnummer på dessa sidor har prefixet **LI**. Importen bevarar detta i källnoterna men använder **sidans bolagsrubrik** för attribution. Två nya poster läggs därför till i bolagsregistret. Det är inte ett påstående om två verifierat självständiga juridiska flygoperatörer den aktuella dagen. Flygningarna får status `scheduled`, inte `confirmed`.

En [sekundär flottförtecknings sökindex](https://www.planespotters.net/airline/Four-Island-Air) anger att Four Island Air upphörde i december 1985. Direktöppningen gav en uppdateringsvägg. Denna kronologi är **inte utredd**; den samtida primärtabellen har samtidigt uttryckliga varianter från **1 februari** under Four Island Air-rubriken. Importen återger därför publicerade planer under källans namn, med juridisk operatör markerad som overifierad. Den bekräftar inte genomförande eller bolagets juridiska existens i februari. Se `operator_attribution_review_batch125.json`.

## Säsongs- och veckodagskontroll

- Four Island Airs lördagskolumner för **LI168/169 gäller till 25 januari** och importeras inte för mars.
- **LI166/164/165** har olika kolumner till 31 januari respektive från 1 februari. De senare används.
- **LI140/141** går tisdag/fredag till 31 januari, därefter tisdag/lördag. Ingen felaktig fredagsrörelse skapas. De korrekta lördagskvällsflygen ligger efter fönstrets slut.
- **LI164/165 på lördag** flyttas i februarivarianten till kvällstid. De äldre eftermiddagskolumnerna får inte skapa falska rörelser i spelfönstret.
- Montserrat Air Services **LI1592/1593** har särskilda fredagsklockslag; kolumnerna för övriga dagar och fredagar används inte samtidigt.
- **LI1594/1595** gäller 15 december–15 januari samt 27 mars–6 april. De utesluts för februari.
- **LI1598/1599** gäller från 15 december och har inget tryckt slutdatum i kolumnen.

## Importerade scheman

Alla klockslag är lokala. D=dagligen, X=utom följande dagar; 1=måndag … 7=söndag. Utrustningskoderna BNI och DHT bevaras som tidtabellsuppgifter, inte som fastställda individflygplan. Källperioder och hänvisningar finns även i `caribbean_source_rows_batch125.json`.

| Sidans bolagsrubrik | Flyg | Fysiskt ben | Lokal tid | Dagar | Skanning/kolumn | Rörelser |
|---|---|---|---|---|---|---:|
| Four Island Air | LI132 | NEV → SKB | 07:45–07:55 | 246 | 6/N1 | 1 |
| Four Island Air | LI152 | ANU → BBQ | 09:00–09:20 | D | 6/N2 | 2 |
| Four Island Air | LI168 | ANU → NEV | 10:00–10:30 | 4 | 6/N3 | 0 |
| Four Island Air | LI162 | ANU → NEV | 10:00–10:30 | D | 6/N5 | 2 |
| Four Island Air | LI162 | NEV → SKB | 10:35–10:45 | D | 6/N5 | 2 |
| Four Island Air | LI142 | SKB → AXA | 10:55–11:30 | 145 | 6/N6 | 1 |
| Four Island Air | LI154 | ANU → BBQ | 16:00–16:20 | D | 6/N7 | 2 |
| Four Island Air | LI166 | ANU → NEV | 16:10–16:40 | 27 | 6/N9 | 0 |
| Four Island Air | LI166 | NEV → SKB | 16:45–16:55 | 27 | 6/N9 | 0 |
| Four Island Air | LI166 | ANU → NEV | 16:55–17:25 | 13456 | 6/N11 | 2 |
| Four Island Air | LI166 | NEV → SKB | 17:30–17:40 | 13456 | 6/N11 | 2 |
| Four Island Air | LI140 | ANU → AXA | 16:55–17:55 | 7 | 6/N12 | 0 |
| Four Island Air | LI144 | SKB → AXA | 17:45–18:20 | 1 | 6/N14 | 0 |
| Four Island Air | LI164 | ANU → NEV | 18:00–18:30 | 27 | 6/N15 | 0 |
| Four Island Air | LI164 | NEV → SKB | 18:35–18:45 | 27 | 6/N15 | 0 |
| Four Island Air | LI138 | NEV → SKB | 18:00–18:10 | 346 | 6/N17 | 1 |
| Four Island Air | LI164 | ANU → NEV | 18:30–19:00 | 6 | 6/N19 | 0 |
| Four Island Air | LI164 | NEV → SKB | 19:05–19:15 | 6 | 6/N19 | 0 |
| Four Island Air | LI140 | ANU → AXA | 18:30–19:30 | 26 | 6/N20 | 0 |
| Four Island Air | LI131 | SKB → NEV | 07:30–07:40 | 246 | 6/S1 | 1 |
| Four Island Air | LI161 | SKB → NEV | 08:00–08:10 | D | 6/S2 | 2 |
| Four Island Air | LI161 | NEV → ANU | 08:15–08:45 | D | 6/S2 | 2 |
| Four Island Air | LI151 | BBQ → ANU | 09:25–09:45 | D | 6/S3 | 2 |
| Four Island Air | LI143 | AXA → SKB | 11:35–12:05 | 145 | 6/S4 | 1 |
| Four Island Air | LI169 | STX → NEV | 12:25–13:40 | 4 | 6/S5 | 0 |
| Four Island Air | LI169 | NEV → ANU | 13:45–14:15 | 4 | 6/S5 | 0 |
| Four Island Air | LI163 | SKB → NEV | 13:15–13:25 | 1 | 6/S7 | 0 |
| Four Island Air | LI163 | NEV → ANU | 13:30–14:00 | 1 | 6/S7 | 0 |
| Four Island Air | LI163 | SKB → NEV | 14:00–14:10 | X1 | 6/S8 | 2 |
| Four Island Air | LI163 | NEV → ANU | 14:15–14:45 | X1 | 6/S8 | 2 |
| Four Island Air | LI153 | BBQ → ANU | 16:25–16:45 | D | 6/S9 | 2 |
| Four Island Air | LI165 | SKB → NEV | 17:05–17:15 | 27 | 6/S11 | 0 |
| Four Island Air | LI165 | NEV → ANU | 17:20–17:50 | 27 | 6/S11 | 0 |
| Four Island Air | LI137 | SKB → NEV | 17:45–17:55 | 346 | 6/S12 | 1 |
| Four Island Air | LI141 | AXA → ANU | 18:05–19:05 | 7 | 6/S14 | 0 |
| Four Island Air | LI145 | AXA → SKB | 18:25–19:00 | 1 | 6/S15 | 0 |
| Four Island Air | LI165 | SKB → NEV | 19:20–19:30 | 6 | 6/S16 | 0 |
| Four Island Air | LI165 | NEV → ANU | 19:35–20:05 | 6 | 6/S16 | 0 |
| Four Island Air | LI141 | AXA → ANU | 19:35–20:35 | 26 | 6/S18 | 0 |
| Montserrat Air Services | LI590 | ANU → MNI | 07:00–07:20 | D | 8/N1 | 2 |
| Montserrat Air Services | LI1590 | ANU → MNI | 07:40–08:00 | D | 8/N2 | 2 |
| Montserrat Air Services | LI584 | MNI → SKB | 08:10–08:40 | 17 | 8/N3 | 0 |
| Montserrat Air Services | LI581 | PTP → MNI | 08:50–09:20 | 25 | 8/N4 | 1 |
| Montserrat Air Services | LI584 | MNI → SKB | 09:30–10:00 | 5 | 8/N5 | 1 |
| Montserrat Air Services | LI592 | ANU → MNI | 10:00–10:20 | D | 8/N6 | 2 |
| Montserrat Air Services | LI1592 | ANU → MNI | 11:00–11:20 | X5 | 8/N7 | 1 |
| Montserrat Air Services | LI1592 | ANU → MNI | 11:20–11:40 | 5 | 8/N8 | 1 |
| Montserrat Air Services | LI594 | ANU → MNI | 13:30–13:50 | D | 8/N9 | 2 |
| Montserrat Air Services | LI588 | MNI → SKB | 15:30–16:00 | 5 | 8/N10 | 1 |
| Montserrat Air Services | LI588 | MNI → SKB | 16:50–17:20 | 1 | 8/N12 | 0 |
| Montserrat Air Services | LI596 | ANU → MNI | 17:00–17:20 | D | 8/N13 | 2 |
| Montserrat Air Services | LI583 | PTP → MNI | 17:40–18:10 | 257 | 8/N14 | 1 |
| Montserrat Air Services | LI598 | ANU → MNI | 18:10–18:30 | D | 8/N15 | 3 |
| Montserrat Air Services | LI1598 | ANU → MNI | 19:45–20:05 | D | 8/N16 | 2 |
| Montserrat Air Services | LI1591 | MNI → ANU | 07:00–07:20 | D | 8/S1 | 2 |
| Montserrat Air Services | LI591 | MNI → ANU | 07:30–07:50 | D | 8/S2 | 2 |
| Montserrat Air Services | LI580 | MNI → PTP | 08:10–08:40 | 25 | 8/S3 | 1 |
| Montserrat Air Services | LI585 | SKB → MNI | 08:50–09:20 | 17 | 8/S4 | 0 |
| Montserrat Air Services | LI1593 | MNI → ANU | 10:10–10:30 | X5 | 8/S5 | 1 |
| Montserrat Air Services | LI585 | SKB → MNI | 10:10–10:40 | 5 | 8/S6 | 1 |
| Montserrat Air Services | LI593 | MNI → ANU | 10:30–10:50 | D | 8/S7 | 2 |
| Montserrat Air Services | LI1593 | MNI → ANU | 10:40–11:00 | 5 | 8/S8 | 1 |
| Montserrat Air Services | LI595 | MNI → ANU | 14:00–14:20 | D | 8/S9 | 2 |
| Montserrat Air Services | LI589 | SKB → MNI | 16:10–16:40 | 5 | 8/S10 | 1 |
| Montserrat Air Services | LI582 | MNI → PTP | 17:00–17:30 | 257 | 8/S12 | 1 |
| Montserrat Air Services | LI597 | MNI → ANU | 17:30–17:50 | D | 8/S13 | 2 |
| Montserrat Air Services | LI589 | SKB → MNI | 17:30–18:00 | 1 | 8/S14 | 0 |
| Montserrat Air Services | LI599 | MNI → ANU | 18:40–19:00 | D | 8/S15 | 2 |
| Montserrat Air Services | LI1599 | MNI → ANU | 20:15–20:35 | D | 8/S16 | 2 |

## Tidsfönster och dubbletter

Fönstret är **27 februari 1986 22:21:30 UTC – 1 mars 22:21:30 UTC**. Alla ändpunkter här använder UTC−4 på måldatumen. Samtliga **71 UTC-intervall** har stämts av mot en separat transkription med varaktigheter och uttryckliga avgångsdatum. Fyra avgångar ligger lokalt den 27 februari, 38 den 28 februari och 29 den 1 mars.

**LI598 ANU–MNI 18:10–18:30** överlappar både torsdagens startgräns och lördagens slutgräns. Hela intervallet behålls. **LI599 MNI–ANU 18:40–19:00**, LI1598 och LI1599 får rörelser på torsdag/fredag men inte lördag, då de startar efter fönstret.

Åtta daterade övergångar i Four Island Airs LI161, LI162, LI163 och LI166 har kontrollerats: samtliga har **fem minuters markuppehåll på Nevis**. Inga tidsöverlapp finns mellan accepterade ben med samma bolag/flygnummer. Samma nummer fastställer inte vilket individflygplan som användes.

En extra kontroll jämför flygnummer, sträcka och UTC-tider över alla accepterade sidor i samma källutgåva. **Inga nya rörelser dubblerar äldre LIAT-rörelser under annat bolagsnamn.**

## Historiska flygplatser

| Kod | 1986-plats | Koordinatpost |
|---|---|---|
| BBQ | Barbuda – Codrington | TAPH, stängd post |
| NEV | Nevis – Newcastle | TKPN |
| AXA | Anguilla – Wallblake | TQPF |
| MNI | Montserrat – Trants, gamla Blackburne/W.H. Bramble | MS-0001, historiskt TRPM |

**BBQ får inte placeras på dagens Burton–Nibbs/TAPB.** [Trinidad och Tobagos luftfartsmyndighets AIP-tillägg 20/24, skanning 12](https://caa.gov.tt/wp-content/uploads/2024/10/VALID-AIP-SUPS-2.pdf#page=12) anger att gamla Codrington stängs 2 oktober 2024 och det nya flygfältet öppnas 3 oktober. Koordinaterna hämtas från [OurAirports gamla TAPH-post](https://ourairports.com/airports/TAPH/), inte posten AG-0001 som nu har BBQ-koden.

**MNI får inte placeras på dagens John A. Osborne/TRPG.** [Montserrat-regeringens kulturhistoriska broschyr, tryckt sida 23](https://www.gov.ms/wp-content/uploads/2020/07/Historic-Cultural-Sights.pdf#page=26) beskriver den gamla flygplatsen vid Trants och ersättaren vid Geralds från 2005. [OurAirports MS-0001](https://ourairports.com/airports/MS-0001/) bevarar den gamla platsen. Den neutrala geografiska benämningen Trants används utan att anta exakt år för namnbytet Blackburne–W.H. Bramble.

Anguillas turistmyndighet daterar [Wallblakes namnbyte till Clayton J. Lloyd](https://www.prnewswire.com/news-releases/wallblake-airport-renamed-clayton-j-lloyd-international-airport-98956504.html) till 4 juli 2010. Namnet Wallblake används därför i 1986-data. [Nevis hamn- och flygplatsmyndighet](https://www.naspakn.com/vancewamoryinternational) bekräftar NEV; det geografiska namnet Newcastle används utan att föra senare anläggningar eller hedersnamn bakåt i tiden.

Alla koordinater är ungefärliga flygfältsmarkörer, inte inmätta terminal- eller banpositioner från 1986. Gamla flygplatsposter är oförändrade. Anguilla och Montserrat tillkommer som territorier, inte som nya självständiga stater.

## Kvarhållna rader och fortsättning

**LI168 NEV–STX importeras inte.** På skanning 6 finns en asterisk vid St. Kitts i den aktuella kolumnen samt vid STX-ankomstraden, utan accepterad förklaring. Ingen direktflygning eller mellanlandning med påhittade klockslag skapas. Det tydliga första benet ANU–NEV är granskat men har inga rörelser i fönstret.

**Inter-Island Air Services på skanning 7 kvarstår.** De tidigare konflikterna kring LI117/126 mellan huvudtabell och IAS-tabell är inte lösta. Allmänna erbjudanden om charter är inte daterade rörelsebelägg.

**Nicaragua saknar ännu verifierade tider. St. Thomas ligger kvar på 14 rörelser** (åtta Pan Am, sex LIAT), Tortola på åtta. Fler operatörer och trafikslag återstår. Tidigare frågor om Cayman Express, Bahamasairs vinterinlaga, Air Jamaicas större nät, Republic EXP, Arrow Air och Air France F27 ligger kvar.

## Regional och global täckning

Forskningskön utökas uttryckligen från **29 till 31 områden** med Anguilla och Montserrat. Jämfört inom samma utökade avgränsning: **390 → 461 rörelser**, **124 → 138 riktade par**, **29 → 33 flygplatser**, **18 → 38 inrikesrörelser** inom samma land/territorium.

Den gamla 29-områdesjämförelsen sparas separat: samma 390 → 461 rörelser berör minst en ändpunkt i det gamla området, men endast 29 → 31 flygplatser ligger där. Montserrat och Anguilla står för de två ytterligare flygplatserna i det utökade urvalet. Ingen ökning av rörelser döljs i en ändrad avgränsning.

| Område med trafik, samt Nicaragua | v127 | v128 |
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
| Antigua och Barbuda | 66 | 115 |
| Saint Kitts och Nevis | 44 | 70 |
| Dominica | 30 | 30 |
| Saint Lucia | 53 | 53 |
| Saint Vincent och Grenadinerna | 32 | 32 |
| Grenada | 27 | 27 |
| Barbados | 53 | 53 |
| Trinidad och Tobago | 27 | 27 |
| Guadeloupe och dåvarande franska karibiska områden | 44 | 48 |
| Martinique | 58 | 58 |
| Nederländska Antillerna (1986) | 55 | 55 |
| Anguilla | 0 | 2 |
| Montserrat | 0 | 41 |

Områdessummor överlappar och får inte adderas. Ändpunkter fastställer inte vilka länder som överflögs. Inga nya militär-, stats-, privat- eller charterflyg har lagts in. Mellanösternurvalet ligger kvar på 72 rörelser.

Totalt **8 989 rörelser**: 8 988 planerade och en tidigare bekräftad. **5 335 katalogscheman / 5 314 granskade**, **378 flygplatser/platser**, **119 länder/territorier**, **1 760 riktade par**. **108 registrerade bolags-/operatörsposter, 65 med rörelser**, 43 utan. Dessa är registerposter, inte en verifierad inventering av självständiga juridiska flygbolag. Oförändrat 90 källposter och 32 bidragande tidtabellsutgåvor. Ingen verifierad global färdigprocent finns.

## Kontroller och installation

**20 befintliga Python-tester och tre generatorkontroller passerar.** Gamla rörelseobjekt, scheman, bolag, länder och flygplatser är oförändrade. En befintlig källpost får utökad titel, granskningssidlista och noter; URL, granskningsdatum och giltighetsuppgifter ändras inte. Före/efter sparas i `source_metadata_update_batch125.json`. Äldre kod, forskningsrapporter, rättelser och hållna konflikter bevaras. Original-PDF och nya källbilder återdistribueras inte.

Stäng Unreal Editor och slå samman ZIP-filens `Plugins/` och `Tools/` med projektroten bredvid `.uproject`. Paketet är kumulativt och innehåller data. **PaintAirplane-rättelsen f23b73a måste redan vara kompilerad.** C++-bygge, Unreal Editor och paketerat spel har inte körts här. Tidtabeller visar planer, inte bekräftad drift.
