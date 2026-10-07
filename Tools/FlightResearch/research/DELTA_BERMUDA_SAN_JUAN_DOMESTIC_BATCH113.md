# Delta – Bermuda, San Juan och fler flygben, v116 / batch113

Granskat 1 oktober 2026. Kumulativ uppdatering från v115.

47 nya planerade Delta-rörelser från 25 granskade nonstop-scheman på 19 riktade flygplatspar, varav 14 nya i databasen. Bermuda till/från Atlanta och Boston, San Juan till/från Atlanta, daterade Dallas–Frankfurt-avgångar samt 13 inrikesben i USA. BDA och SJU samt territorierna Bermuda och Puerto Rico tillkommer. Nio äldre scheman rättas med bibehållna ID. Totalt **8 189 rörelser**, varav 8 188 tidtabellslagda och en dokumenterat genomförd. Se `research/DELTA_BERMUDA_SAN_JUAN_DOMESTIC_BATCH113.md`.

## Sträckor

Varje riktning räknas separat. Scheman är återkommande tidtabellsrader; rörelser är daterade flygintervall i spelfönstret. Av de 25 nya schemana ger 24 rörelser i fönstret.

| Riktad sträcka | Nya scheman | Nya rörelser |
|---|---:|---:|
| AMA → LBB | 2 | 4 |
| ATL → AUS-MUELLER | 1 | 2 |
| ATL → BDA | 1 | 2 |
| ATL → LIT | 1 | 2 |
| ATL → SJU | 3 | 5 |
| BDA → ATL | 1 | 2 |
| BDA → BOS | 1 | 3 |
| BDL → PHL | 1 | 2 |
| BOS → BDA | 1 | 2 |
| DFW → FRA | 1 | 1 |
| DFW → LIT | 1 | 2 |
| DFW → MCO | 1 | 2 |
| FRA → DFW | 1 | 1 |
| FWA → TOL | 1 | 2 |
| JAX → PBI | 1 | 2 |
| LBB → AMA | 2 | 4 |
| SJU → ATL | 3 | 5 |
| TOL → FWA | 1 | 2 |
| TUL → OKC | 1 | 2 |
| **Totalt** | **25** | **47** |

## Källa och urval

[Delta Air Lines systemtidtabell från 1 februari 1986, Digital Library of Georgia](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201). Nya rader har granskats visuellt på tryckta sidor **10, 14, 16, 17, 27, 31, 65, 66, 90, 101, 117, 140, 216, 234 och 239**. Alla anger 0 stopp. Tomt frekvensfält transkriberas D för dagligen; 5 betyder fredag och 6 lördag. Flygnumren ligger under Delta Connection-serierna. Stjärna betyder lågtrafikpris, inte datumskifte. Källans slutdatum och senare ändringar är inte verifierade.

Daterade Frankfurt-rader är **DFW–FRA 22, fredag 17:05–09:25 nästa lokala dygn**, fotnot 3 med trafik 28 februari–7 mars, och **FRA–DFW 23, lördag 10:10–13:35**, fotnot 5 med trafik 1–8 mars. Fotnotsdefinitionerna har granskats på s.65 och s.91. I detta paket begränsas importgiltigheten till respektive relevanta dagar 28 februari och 1 mars. Restiden är 9 timmar 20 minuter respektive 10 timmar 25 minuter.

OCR användes endast för att hitta kandidater. Originalens destinationer, tider, stopp, veckodagar och fotnoter granskades i bilder. Några OCR-träffar var anslutningsförslag i andra kolumner: DFW–MIA ”270”, DTW–DFW ”481” och MCO–DFW ”158” importeras inte som nonstop. DFW–FRA 14 har ett stopp, medan det importerade flyg 22 har noll.

Planerad trafik är inte belagt genomförande. Individflygplan, verklig flygbana och operatörsnätets fullständighet fastställs inte. PDF-hash, sidlista, råtranskription och granskningsnoteringar medföljer; PDF och nya sidbilder ingår inte.

## Flygplatser och territorier

| Kod | Namn i 1986-paketet | Tidszon i perioden | Ungefärlig kartmarkör |
|---|---|---|---|
| BDA | Bermuda – Kindley Field | UTC−4 | 32.363802, −64.678240 |
| SJU | San Juan – Luis Muñoz Marín International | UTC−4 | 18.439400, −66.001801 |

Den samtida tidtabellens flygplatskodlista anger **BDA – Kindley Field på s.260** och **SJU – Luis Munoz Marin Int’l på s.261**. Dessa är underlaget för namn och historisk flygplatsidentitet. Dagens BDA-namn L.F. Wade används inte bakåt i tiden; SJU är skilt från Isla Grande. Nuvarande koordinater och identitetsmetadata kommer från [OurAirports BDA](https://ourairports.com/airports/TXKF/) och [OurAirports SJU](https://ourairports.com/airports/TJSJ/).

Koordinaterna är ungefärliga markörer för respektive flygfält. De rekonstruerar inte 1986 års referenspunkter, terminaler, gater eller bantrösklar. `airport_location_review_batch113.json` redovisar denna precision och underlaget. **Bermuda (`bm`) och Puerto Rico (`pr`)** läggs till som territorieposter med ofullständig operatörsinventering. Inga nya operatörer tillkommer. Austin-benet använder det befintliga historiska **AUS-MUELLER**, inte dagens Austin-Bergstrom.

## Rättelser av äldre avskrifter

Nio äldre scheman ändras enligt nya visuella avläsningar. Deras **schema-ID och servicegrupp-ID behålls som stabila proveniensnycklar**, även när strängarna innehåller ett gammalt flygnummer eller klockslag. De faktiska fälten `flight`, `departure` och `arrival` styr den korrigerade trafiken.

| Sträcka | Rätt flygnummer | Ändrade fält | Tryckt sida |
|---|---|---|---:|
| ATL → MCI | 620 | departure: 08:33 → 08:35 | 15 |
| ATL → CLE | 906 | departure: 23:10 → 23:01 | 14 |
| ATL → MEM | 764 | flight: 704 → 764 | 16 |
| ATL → SDF | 352 | departure: 22:52 → 22:58 | 16 |
| ATL → DAB | 1041 | departure: 16:50 → 16:47 | 15 |
| SEA → DFW | 832 | departure: 12:20 → 13:20, arrival: 17:47 → 18:47 | 223 |
| SEA → DFW | 874 | departure: 12:20 → 13:20, arrival: 17:47 → 18:47 | 223 |
| DFW → LIT | 1085 | flight: 1065 → 1085, departure: 13:15 → 15:15, arrival: 14:10 → 16:10 | 65 |
| DFW → LIT | 388 | departure: 19:50 → 19:55 | 65 |

Fullständiga före-/eftervärden och 19 berörda rörelse-ID finns i `errata_batch113.json`. Dessa rättelser har företräde framför berörda avskrifter i tidigare forskningsfiler; gamla rapporter behålls som historiska versionsunderlag. Övriga **8 123 äldre rörelseobjekt och 4 746 äldre scheman är oförändrade**. Samtliga 8 142 tidigare rörelse-ID bevaras.

JAX–ATL 764 har separata rader för **14:45–15:45 alla dagar utom söndag** och **14:55–15:55 söndag**. Den befintliga X7-raden var korrekt och har inte ändrats. Även ATL–CHS 936 och CVG–PHL 288 behålls efter bildgranskning.

En kvarstående granskningsfråga gäller avgångsminuten för äldre **ATL–NAS 125** på s.16. Bildförstoring och OCR gav olika läsningar; befintlig 11:55 har inte ersatts med en gissning. Tydligare, oberoende underlag behövs. Det är ingen ny importerad rad. Tidigare hållna konflikter för Delta 597 MEM–LIT, CMH–SDF 1871/1671 och ASA 1412 HSV–ATL bevaras.

## Fönster, dygnsskiften och markuppehåll

Spelfönstret är exakt **1986-02-27 22:21:30 UTC till 1986-03-01 22:21:30 UTC**. Hela flygintervallet behålls när någon del överlappar detta fönster.

**BDA–BOS 62 och SJU–ATL 96** ger tre rörelser vardera eftersom även flygningarna som pågår vid första gränsen inkluderas. 19 scheman ger två rörelser vardera. Frankfurt 22/23 och ATL–SJU 771 ger en vardera. **SJU–ATL 770** är lördagstrafik med avgång 23:15 UTC den 1 mars, efter fönstrets slut, och ger därför noll rörelser. Den källgranskade tidtabellsraden finns ändå i katalogen. Avgångsdatumen för de 47 nya rörelserna fördelas på 27 februari: 7, 28 februari: 22, 1 mars: 18.

30 par av anslutande, separat tidsatta ben med samma flygnummer har granskats. 28 har två matchande övergångar. **IAH–DFW–LIT 1156** och **BDL–PHL–TPA 287** har varsin övergång den 28 februari: första dagens inkommande ben slutar före startgränsen, sista dagens utgående ben börjar efter slutgränsen. Varje ben prövas självständigt.

**ATL–JAX–PBI 807** fortsätter efter midnatt med 36 minuters markuppehåll. Rättad **JAX–ATL–MEM 764** får 61 minuter i Atlanta; rättad **AUS–DFW–LIT 1085** får 49 minuter i Dallas. Samma flygnummer och rimlig marktid bevisar inte att samma individflygplan användes.

## Validering

Separat 24-timmarstranskription och kalenderberäkning med fasta vinterförskjutningar verifierar alla **47 nya och 19 korrigerade UTC-intervall**, oberoende av byggverktygets ZoneInfo-beräkning. Alla 34 nya eller korrigerade schemarader omfattas. Inga tidsöverlapp hittas där minst ett berört ben jämförs med äldre eller nya ben med samma operatör och flygnummer.

Alla **20 Python-tester och tre aktualitetskontroller** passerar. De tidigare 319 flygplatserna, 82 land-/territorieposterna och 98 operatörerna är oförändrade. Endast Delta-källposten har kompletterats. Äldre källposter, kod och forskningsfiler bevaras; rättelserna dokumenteras i nya filer.

Unreal-kompilering, Editor och paketerat spel har inte körts. Paketet innehåller data och förutsätter redan kompilerad **PaintAirplane-rättelse `f23b73a`**. ZIP-filen installerar inte själva C++-rättelsen.
