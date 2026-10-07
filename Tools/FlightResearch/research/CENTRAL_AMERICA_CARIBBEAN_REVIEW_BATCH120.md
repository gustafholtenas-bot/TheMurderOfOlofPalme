# Centralamerika och Karibien – v123 / batch120

Granskat 1 oktober 2026. **47 nya planerade flygrörelser**: 34 Air France och 13 Pan Am. Totalt **8 535 rörelser**. 43 nya scheman på **29 nya riktade flygplatspar**. Alla **8 488 äldre rörelseobjekt och 4 970 äldre scheman är oförändrade**. Kumulativ datauppdatering från v122.

## Tillägg och källor

Bahamas, Turks- och Caicosöarna, Antigua, Martinique, Guadeloupe och Haiti får fler eller sina första registrerade förbindelser. Cayenne tillkommer som anslutande sydamerikansk destination. Passagerarflyg kompletteras med **nio tidtabellslagda fraktrörelser**. Bolagsregistreringen är oförändrad: inget nytt bolag räknas som färdigkartlagt.

- [Pan Am, 11 februari–26 april 1986, University of Miami](https://digitalcollections.library.miami.edu/digital/collection/asm0341/id/37370/): nio Nonstop-rader, tryckta s.42–43, 63, 70–72 och 83.
- [Air France utgåva 25, 27 oktober 1985–29 mars 1986](https://www.airtimes.com/cgat/fr/airfrance/pdf/1a/af851027-1a.pdf): 34 separat tidsatta nonstop-rader, tryckta s.24, 33–34, 48, 51, 66, 74, 79–80 och 85. Pil i VIA-kolumnen betyder nonstop enligt s.4; dagar, lokala tider och nästa-dagssymbol enligt s.2–4.

Originalrader har granskats visuellt, med förstoring av små datum- och flygplatskolumner. Bild-/PDF-adresser och kontrollsummor finns i `caribbean_source_evidence_batch120.json`. Originalskanningarna återdistribueras inte. Urvalet är partiellt.

## Importerade scheman

Tiderna är lokala; +1 betyder ankomst nästa lokala dag. Dagar: 1=måndag … 7=söndag. Giltigheten begränsas till relevanta datum 27 februari–2 mars och filtreras sedan mot det exakta spelfönstret. Fullständiga källvillkor och importgränser finns i `caribbean_source_rows_batch120.json`.

| Flyg | Fysiskt ben | Lokal tid | Dagar | Tryckt sida | Typ |
|---|---|---|---|---:|---|
| PA206 | NAS → JFK | 13:05–15:45 | 1234567 | 70 | Passagerare |
| PA205 | JFK → NAS | 09:20–12:05 | 1234567 | 72 | Passagerare |
| PA418 | PLS → MIA | 15:00–16:35 | 36 | 83 | Passagerare |
| PA417 | MIA → PLS | 12:45–14:25 | 36 | 63 | Passagerare |
| PA418 | GDT → MIA | 14:55–16:35 | 257 | 43 | Passagerare |
| PA417 | MIA → GDT | 12:45–14:30 | 257 | 63 | Passagerare |
| PA408 | FPO → MIA | 13:50–14:24 | 1234567 | 42 | Passagerare |
| PA407 | MIA → FPO | 12:50–13:20 | 1234567 | 63 | Passagerare |
| PA227 | JFK → ANU | 17:10–22:05 | 6 | 71 | Passagerare |
| AF364 | FDF → PTP | 08:50–09:20 | 5 | 33 | Passagerare |
| AF366 | FDF → PTP | 08:50–09:20 | 6 | 33 | Passagerare |
| AF235 | FDF → PTP | 17:55–18:35 | 5 | 34 | Passagerare |
| AF358 | FDF → PTP | 22:10–22:40 | 4 | 34 | Passagerare |
| AF1347 | FDF → PTP | 15:00–15:40 | 6 | 34 | Frakt |
| AF363 | FDF → CAY | 19:20–22:25 | 25 | 33 | Passagerare |
| AF206 | FDF → CDG | 16:35–05:35 +1 | 4 | 33 | Passagerare |
| AF242 | FDF → ORY | 22:35–11:40 +1 | 4 | 33 | Passagerare |
| AF227 | FDF → MRS | 19:45–08:55 +1 | 5 | 33 | Passagerare |
| AF351 | PTP → FDF | 07:30–08:00 | 145 | 79 | Passagerare |
| AF227 | PTP → FDF | 17:45–18:25 | 5 | 79 | Passagerare |
| AF363 | PTP → FDF | 18:00–18:30 | 25 | 79 | Passagerare |
| AF366 | PTP → MIA | 10:10–12:25 | 6 | 79 | Passagerare |
| AF364 | PTP → SJU | 10:10–11:10 | 5 | 80 | Passagerare |
| AF1106 | PTP → SJU | 16:00–17:00 | 5 | 80 | Frakt |
| AF235 | PTP → CDG | 19:55–08:50 +1 | 5 | 80 | Passagerare |
| AF1347 | PTP → CDG | 17:45–06:40 +1 | 6 | 80 | Frakt |
| AF363 | PAP → SJU | 13:10–15:20 | 25 | 80 | Passagerare |
| AF1105 | PAP → SJU | 09:30–11:40 | 5 | 80 | Frakt |
| AF1106 | PAP → MIA | 19:40–21:35 | 5 | 80 | Frakt |
| AF363 | SJU → PTP | 16:05–17:05 | 25 | 85 | Passagerare |
| AF1105 | SJU → PTP | 12:40–13:40 | 5 | 85 | Frakt |
| AF364 | SJU → PAP | 12:00–12:10 | 5 | 85 | Passagerare |
| AF1106 | SJU → PAP | 18:00–18:10 | 5 | 85 | Frakt |
| AF365 | MIA → PTP | 14:00–18:00 | 6 | 51 | Passagerare |
| AF1105 | MIA → PAP | 06:45–08:30 | 5 | 51 | Frakt |
| AF366 | CAY → FDF | 07:00–08:00 | 6 | 24 | Passagerare |
| AF242 | CAY → FDF | 20:00–21:10 | 4 | 24 | Passagerare |
| AF255 | FDF → BOD | 18:05–06:50 +1 | 4 | 33 | Passagerare |
| AF235 | ORY → FDF | 12:45–16:30 | 5 | 66 | Passagerare |
| AF257 | ORY → FDF | 13:40–17:25 | 6 | 66 | Passagerare |
| AF1347 | ORY → FDF | 09:15–13:00 | 6 | 66 | Frakt |
| AF225 | CDG → PTP | 16:40–20:15 | 6 | 74 | Passagerare |
| AF251 | MRS → FDF | 16:20–20:20 | 6 | 48 | Passagerare |

Ett schema kan ge flera avgångar under fönstret, och flera scheman kan följa samma sträcka. Därför skiljer sig antalet scheman, rörelser och riktade flygplatspar. Flygnummer och utrustningskod identifierar inte ett individflygplan och styrker inte faktiskt genomförande.

## Datum, Parisflygplatser och mellanlandningar

- **Inkommande långdistansben**: Orly–Fort-de-France AF235 fredag och AF257 lördag, fraktflyget AF1347 lördag, CDG–Pointe-à-Pitre AF225 lördag samt Marseille–Fort-de-France AF251 lördag tillkommer. Den senare fortsättningen FDF–PTP med AF251 ligger efter fönstret.
- **AF235 fredag**: giltig variant från 17 januari. FDF–PTP 17:55–18:35 följs av PTP–**CDG** 19:55–08:50 nästa dag. Ankomstsuffix **A** anger Charles de Gaulle; detta är inte Orly.
- **AF1347 lördag**: giltig från 8 februari. FDF–PTP 15:00–15:40 och PTP–**CDG** 17:45–06:40 nästa dag. Fraktmarkeringen och terminalsuffixet är kontrollerade.
- **AF242 torsdag**: CAY–FDF 20:00–21:10 och FDF–**ORY** 22:35–11:40 nästa dag; ankomstsuffix **S** anger Orly Sud.
- **AF1349** importeras inte: PTP–FDF gäller till 31 januari och FDF–CDG endast 3–31 januari. **AF1114 PTP–JFK** har slutdatum 28 december. **AF224 PTP–ORY torsdag** gäller bara 2–9 januari. Datumkolumnerna FROM och TO har skilts åt.
- **PA227 JFK–ANU lördag** gäller till och med 1 mars och överlappar fönstrets sista 11 minuter och 30 sekunder.
- **AF365 MIA–PTP lördag** ingår. Fortsättningen PTP–FDF börjar 22:50 UTC, efter fönstrets slut, och skapar ingen extra rörelse här.

Genomgående resor, exempelvis PAP–CAY med AF363, FDF–PAP med AF364 och CAY–MIA med AF366, delas upp i de separat tidsatta fysiska benen. De läggs inte också in som obrutna direktflyg. Detsamma gäller fraktkedjorna MIA–PAP–SJU–PTP och PTP–SJU–PAP–MIA.

**17 intilliggande benpar** med samma flygnummer har kontrollerade markuppehåll på 45–125 minuter. Inga tidsöverlapp mellan berörda flygnummer finns i de gamla och nya rörelserna. Kontrollen fastställer inte vilket enskilt flygplan som utförde benen.

## Flygplatser

| Kod | Namn i uppdateringen | Forskningsområde |
|---|---|---|
| FPO | Freeport – Grand Bahama | bs |
| PLS | Providenciales (1986) | tc |
| GDT | Grand Turk | tc |
| ANU | Antigua – V. C. Bird | ag |
| FDF | Fort-de-France – Le Lamentin | mq |
| PTP | Pointe-à-Pitre – Le Raizet | gp |
| PAP | Port-au-Prince (tidtabell: François Duvalier) | ht |
| CAY | Cayenne – Rochambeau (1986) | gf |

Flygplatsidentiteter är kontrollerade mot de samtida tabellerna. [Antiguas flygplats beskriver namnbytet till V. C. Bird år 1985](https://vcbia.com/vcbia-history-info/). Le Lamentin, Le Raizet och Rochambeau följer källornas namn. Port-au-Princes François Duvalier anges uttryckligen som **tidtabellens namn**, inte som ett påstående om det officiella namnet efter den politiska förändringen i februari 1986.

Koordinaterna är ungefärliga markörer för flygfälten från OurAirports, inte inmätta 1986-positioner för terminaler eller bantrösklar. Källor och begränsningar finns i `airport_location_review_batch120.json`. Sex nya länder/forskningsområden registreras; de franska utomeuropeiska områdena räknas inte som självständiga stater.

## Kvarstående luckor och nästa steg

**Nicaragua saknar fortfarande godkända tidtabellsklockslag och har noll importerade rörelser.** Det är en datalucka. **St. Thomas har fortsatt åtta rörelser**, enbart det tidigare Pan Am-urvalet; fler operatörer återstår.

Denna omgång granskade också följande spår, dokumenterade i `caribbean_withheld_batch120.json`:

- American Airlines 31 januari 1986: sex tillgängliga annonsbilder visar omslag/baksida, inga inre klockrader. Majstarter på baksidan används inte för februari.
- Eastern: ett omslag för 1 januari 1986 är en fortsatt ledtråd; utgåvan 2 mars börjar efter fönstret. Ingen tillämplig tidtabellssida godkändes.
- LACSA: maj 1986 är fel period; äldre PDF-utgåvor styrker inte februari 1986.
- LIAT: den tillgängliga arbetstidtabellen från 26 oktober 1986 gäller en senare säsong. Juli 1985-indexet ger ännu inga verifierade vinterklockslag.
- Air Frances F27-rader mellan öarna hålls för ytterligare kontroll av operatörsangivelsen. Inga flyg skapas genom antagna bolag eller frekvenser.

Fortsätt med Nicaragua, fler operatörer kring St. Thomas och kvarvarande regionaltrafik. De fem tillagda inkommande långdistansbenen ger fortfarande inte ett komplett nät. Militär-, stats-, privat-, charter- och specialtrafik kräver daterade rörelsehandlingar. Inga nya sådana rörelser tillförs här. Sökningen är inte uttömmande och inget säkert slutantal återstående flyg finns. Se `central_america_caribbean_queue_batch120.json`.

## Regional täckning

Unika rörelser med minst en ändpunkt i de ursprungliga 29 forskningsområdena ökar **84 → 131**, riktade flygplatspar **36 → 65**, regionala flygplatser **11 → 18**. Cayenne ligger utanför dessa 29 områden och redovisas separat: tre rörelser, två riktade par, en flygplats. Dessa tre rörelser har också en regional ändpunkt i Martinique och ingår redan i 131; de läggs inte till en gång till.

| Område med trafik, samt Nicaragua | v122 | v123 |
|---|---:|---:|
| Nicaragua | 0 | 0 |
| Guatemala | 4 | 4 |
| Amerikanska Jungfruöarna / St. Thomas, St. Croix | 15 | 15 |
| Puerto Rico | 10 | 18 |
| Haiti | 0 | 6 |
| Dominikanska republiken | 4 | 4 |
| Bahamas | 19 | 27 |
| Turks- och Caicosöarna | 0 | 4 |
| Antigua och Barbuda | 0 | 1 |
| Saint Kitts och Nevis | 3 | 3 |
| Saint Lucia | 2 | 2 |
| Barbados | 14 | 14 |
| Trinidad och Tobago | 9 | 9 |
| Guadeloupe och dåvarande franska karibiska områden | 0 | 17 |
| Martinique | 0 | 19 |
| Nederländska Antillerna (1986) | 14 | 14 |

Områdesraderna överlappar och får inte summeras. Ändpunkter fastställer inte vilka länder ett flyg faktiskt överflög. All täckning är fortsatt partiell.

## Kontroll och installation

Fönstret är **27 februari 1986 22:21:30 UTC – 1 mars 22:21:30 UTC**. Alla 47 nya UTC-intervall har jämförts med en separat klock-/varaktighetstranskription och fasta historiska UTC-förskjutningar. AF206 FDF–CDG och AF255 FDF–BOD börjar före fönstret men landar inom det; dessa fullständiga intervall behålls. Lördagens AF1347 PTP–CDG och PA227 JFK–ANU landar efter slutet men överlappar fönstret.

**20 befintliga Python-tester och tre generatorkontroller passerar.** Totalt: **8 535 rörelser** (8 534 planerade och en tidigare bekräftad), **5 013 katalogscheman / 4 992 granskade**, **362 flygplatser/platser**, **111 länder/territorier**, **1 670 riktade par**. 102 registrerade operatörer, 59 med rörelser och 43 utan; 87 källposter och 29 bidragande tidtabellsutgåvor. Pan Am har 392 rörelser och Air France 372.

Stäng Unreal Editor och slå samman ZIP-filens `Plugins/` och `Tools/` med projektroten bredvid `.uproject`. Behåll mappstrukturen. Detta är **endast data**; `PaintAirplane`-rättelsen **f23b73a** måste redan vara kompilerad. C++-bygge, Unreal Editor och paketerat spel har inte körts här. Äldre rörelser, scheman, flygplatser, bolag, kod, rättelser och hållna konflikter är bevarade.
