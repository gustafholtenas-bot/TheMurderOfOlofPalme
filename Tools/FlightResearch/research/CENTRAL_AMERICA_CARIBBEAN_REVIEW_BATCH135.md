# Republic kring Detroit och Las Vegas–San Diego – v138 / batch135

Granskat 2 oktober 2026. **299 nya planerade flygrörelser**, totalt **10 632**. Tillägget omfattar **36 riktade sträckor, varav 27 är nya i databasen**, och **146 nya granskade scheman**. 145 ger rörelser i projektfönstret; en lördagsavgång ligger efter slutgränsen.

Alla tillägg är inrikes i USA och räknas utanför Centralamerika/Karibien-urvalet. **Inga nya flygplatser, länder, operatörer eller tidtabellsutgåvor**. Republic har nu **1 067 rörelser från 520 scheman**. Hela nätet är ännu inte granskat.

Alla **10 333 äldre rörelseobjekt och 6 236 äldre scheman är oförändrade**. Äldre flygplats-, land- och operatörsposter bevaras. Republic-källpostens granskningsmetadata utökas; tidigare forskningsrapporter och rättelser bevaras byte för byte.

## Nya rörelser

| Riktad sträcka | Nya rörelser | Ny sträcka i databasen |
|---|---:|---|
| BOS–DTW | 15 | Ja |
| DCA–DTW | 6 | Ja |
| DFW–DTW | 5 | Nej |
| DTW–BOS | 16 | Ja |
| DTW–DCA | 6 | Ja |
| DTW–DFW | 5 | Nej |
| DTW–IAD | 6 | Ja |
| DTW–JFK | 4 | Ja |
| DTW–LAS | 2 | Ja |
| DTW–LAX | 10 | Ja |
| DTW–LGA | 16 | Ja |
| DTW–MCI | 7 | Ja |
| DTW–MKE | 15 | Nej |
| DTW–ORD | 23 | Nej |
| DTW–PHL | 13 | Ja |
| DTW–PHX | 6 | Ja |
| DTW–SAN | 2 | Ja |
| DTW–SEA | 2 | Ja |
| DTW–SFO | 7 | Ja |
| DTW–STL | 7 | Ja |
| IAD–DTW | 6 | Ja |
| JFK–DTW | 4 | Ja |
| LAS–DTW | 2 | Ja |
| LAS–SAN | 5 | Nej |
| LAX–DTW | 9 | Ja |
| LGA–DTW | 16 | Ja |
| MCI–DTW | 6 | Ja |
| MKE–DTW | 14 | Nej |
| ORD–DTW | 23 | Nej |
| PHL–DTW | 13 | Nej |
| PHX–DTW | 7 | Ja |
| SAN–DTW | 2 | Ja |
| SAN–LAS | 4 | Nej |
| SEA–DTW | 2 | Ja |
| SFO–DTW | 7 | Ja |
| STL–DTW | 6 | Ja |

BOS = Boston; DTW = Detroit; DCA = Washington National; DFW = Dallas/Fort Worth; IAD = Washington Dulles; JFK = New York Kennedy; LGA = New York LaGuardia; LAS = Las Vegas; LAX = Los Angeles; MCI = Kansas City; MKE = Milwaukee; ORD = Chicago O’Hare; PHL = Philadelphia; PHX = Phoenix; SAN = San Diego; SEA = Seattle; SFO = San Francisco; STL = St. Louis. Befintliga historiskt granskade flygplatsmarkörer används oförändrade.

En rörelse är ett daterat fysiskt flygben. Flera avgångar på samma linje räknas var för sig, liksom de två riktningarna.

## Källa och avgränsning

[Republic Airlines Employee System Timetable, 14 februari–1 mars 1986](https://northwestairlineshistory.org/wp-content/uploads/2018/07/REP-employee-schedule-1986-02-14.pdf), bevarad hos Northwest Airlines History Center. Omslagets uttryckliga giltighet täcker importperioden. Den tidigare hämtade kompletta originaltidtabellen återanvänds: tabeller på skanning **2, 3, 5 och 6**, legend på skanning 6. PDF och sidbilder dokumenteras med URL och SHA-256 i `caribbean_source_evidence_batch135.json`. **Originalskanningarna distribueras inte.**

Alla **146 rader i de 36 valda riktningsrubrikerna** har tom ST-kolumn och katalogiseras. Det finns inga EXP-rader eller genomgående ST=1-rader inom just dessa rubriker. ST betyder Stops. A/P betyder AM/PM, X betyder undantag och 1–7 betyder måndag–söndag. Tom FRQ tolkas som dagligen. Utrustningskoderna **DC9, D9S, D95, 72S, 757 och M80** bevaras som tryckta.

Detta är ett avgränsat urval. Övriga Detroit-linjer, Minneapolis-nätet, Express-trafiken, ändringsblad och faktisk drift återstår. Källans legend skiljer Express Airlines I (1400–1699) från Simmons Airlines (1700–1899); inga sådana rader importeras här.

## Las Vegas–San Diego kompletterar fysiska ben

Fyra separata scheman tillförs mellan LAS och SAN, med sammanlagt **nio nya rörelser**. De kan kopplas tidsmässigt till de tidigare Memphis-benen med samma nummer. De genomgående rubrikerna nedan finns utanför de 36 valda riktningsrubrikerna och räknas inte som nya nonstopflyg:

| Genomgående resa | Lokala tider | Tryckt ST | Fysiska ben |
|---|---|---:|---|
| 551 MEM–SAN | 08:55–11:55 | 1 | MEM–LAS + LAS–SAN |
| 553 MEM–SAN | 12:05–15:05 | 1 | MEM–LAS + LAS–SAN |
| 552 SAN–MEM | 12:25–18:50 | 1 | SAN–LAS + LAS–MEM |
| 550 SAN–MEM | 15:35–22:00 | 1 | SAN–LAS + LAS–MEM |

Varje delben behåller sina egna tider och datum. LAS–SAN553 kan exempelvis överlappa fönstret en torsdag trots att dagens MEM–LAS553 har landat före fönstrets start. Det äldre benet läggs då inte till bara för att komplettera en resa.

## Tider och fönstergränser

Projektfönstret är **27 februari 1986 kl. 22:21:30 UTC till 1 mars kl. 22:21:30 UTC**. Ett flyg tas med när dess intervall överlappar fönstret. Avgång före startgränsen kan alltså ge en rörelse om flyget fortfarande är i luften.

| UTC-offset under de tre datumen | Flygplatser i urvalet |
|---|---|
| −5 timmar | BOS, DTW, DCA, IAD, JFK, LGA, PHL |
| −6 timmar | DFW, MCI, MKE, ORD, STL |
| −7 timmar | PHX |
| −8 timmar | LAS, LAX, SAN, SEA, SFO |

Offseterna är kontrollerade mot varje flygplats tidszon för samtliga tre datum. Lokaltider följer tidtabellskonvention och är förenliga med flygtiderna; ingen separat uttrycklig all-times-local-text har återfunnits i originalet.

**63 rörelser har lokal avgång torsdag, 145 fredag och 91 lördag.** Fördelningen per schema är **22 med tre utfall, 110 med två, 13 med ett och ett med noll**. Veckodagsundantag och exakt intervallöverlapp styr resultatet.

- **531 DTW–LAS 19:30–20:35** är endast lördag. Avgången ligger efter lördagens slutgräns kl. 17:21:30 Eastern. Schemat katalogiseras men ger **noll** rörelser. **533 DTW–LAS 21:55–23:00, X6**, ger torsdag och fredag.
- **584 PHX–DTW 18:35–00:10** ankommer nästa lokala kalenderdag och tar 215 minuter. Två daterade nattben importeras.
- **338 LAX–DTW 00:25–07:30** avgår efter midnatt och ankommer samma lokala dag. Avgången kopplas inte felaktigt till föregående kalenderdag.
- **326 DTW–DCA 16:00–17:19** landar 2 minuter och 30 sekunder före torsdagens startgräns. **773 DTW–LGA 15:55–17:25** landar 3 minuter och 30 sekunder efter gränsen. Den första ger två rörelser, den andra tre.
- **720 DFW–DTW 16:20–19:45** avgår kl. 22:20 UTC, 1 minut och 30 sekunder före lördagens slutgräns. Den ger torsdag, fredag och lördag.
- Samma eller tidigare lokalt ankomstklockslag på DTW–ORD/MKE följer skillnaden mellan Eastern och Central Time. Exempelvis **201 DTW–MKE 09:25–09:23** tar 58 minuter.

**JFK–DTW652 är X6 och DC9**, vilket kontrollerades i förstorad originalbild före import. Även **713 IAD–DTW** och **174 STL–DTW** har tryckt utrustningskod DC9. Dessa kontroller gällde nya transkriptioner; inga äldre poster ändrades.

**MKE–DTW212 och 202** har samma tider 07:00–08:53 men skilda tryckta nummer och utrustningskoder. **DTW–MKE759 och 699** har samma tider 17:10–17:08 men skilda nummer/koder. Alla fyra rader bevaras med sina respektive anmärkningar. Samma klockslag i sig är inte tillräckligt för att slå ihop posterna; inget individflygplan identifieras.

Samtliga **146 flygtider, 50–297 minuter**, även nollutfallet, och **299 UTC-intervall** har kontrollerats mot separat manuellt granskade varaktigheter och förväntade datum. Fasta UTC-offseter matchar generatorns tidszonsberäkning. Det är samma originalmaterial, inte en andra oberoende källa eller granskare.

## Angränsande ben

**56 daterade följder med samma flygnummer**, fördelade på **30 klock-/sträckmönster**, har kontrollerats. **29** knyter ihop nya ben med äldre importerade ben. Uppehållen är **40–67 minuter i Detroit**, **30–32 i Las Vegas**, **24/29/39 i St. Louis** och **30 i Chicago**.

Klockskillnaderna har granskats manuellt. Kontrollen visar tidsmässig förenlighet; den bekräftar inte faktisk drift, samma flygplansindivid eller en bokningsbar anslutning. Källans angivna 30-minuters minimum gäller dess tre namngivna nav Memphis, Detroit och Minneapolis och används inte som ett generellt krav på uppehåll i St. Louis.

| Nummer | Benföljd | Ankomst–nästa avgång vid mellanpunkten | Markuppehåll, minuter |
|---|---|---|---:|
| 17 | LGA → DTW → ORD | 09:35–10:30 | 55 |
| 23 | FLL → DTW → ORD | 12:30–13:30 | 60 |
| 26 | ORD → DTW → PHL | 16:58–17:40 | 42 |
| 72 | MEM → DTW → BOS | 11:29–12:10 | 41 |
| 100 | PHX → DTW → BOS | 15:10–16:15 | 65 |
| 151 | DTW → STL → MEM | 10:11–10:35 | 24 |
| 201 | PHL → DTW → MKE | 08:45–09:25 | 40 |
| 202 | MKE → DTW → PHL | 08:53–09:40 | 47 |
| 206 | MKE → DTW → PHL | 15:18–16:15 | 57 |
| 209 | PHL → DTW → MKE | 18:40–19:20 | 40 |
| 212 | MKE → DTW → FLL | 08:53–09:50 | 57 |
| 335 | BOS → DTW → LAX | 12:30–13:15 | 45 |
| 338 | LAX → DTW → LGA | 07:30–08:30 | 60 |
| 343 | LGA → DTW → SFO | 12:30–13:15 | 45 |
| 417 | MIA → DTW → MKE | 12:15–13:00 | 45 |
| 420 | ORD → DTW → MCO | 11:58–13:05 | 67 |
| 543 | LGA → DTW → MKE | 13:19–14:10 | 51 |
| 546 | MCO → DTW → LGA | 12:30–13:15 | 45 |
| 550 | SAN → LAS → MEM | 16:30–17:00 | 30 |
| 551 | MEM → LAS → SAN | 10:30–11:00 | 30 |
| 552 | SAN → LAS → MEM | 13:18–13:50 | 32 |
| 553 | MEM → LAS → SAN | 13:40–14:10 | 30 |
| 569 | BOS → DTW → MEM | 09:55–10:40 | 45 |
| 701 | BOS → DTW → SFO | 08:55–09:45 | 50 |
| 712 | DTW → STL → MEM | 20:06–20:45 | 39 |
| 725 | DTW → STL → MEM | 17:31–18:00 | 29 |
| 729 | DTW → ORD → MEM | 19:35–20:05 | 30 |
| 743 | JFK → DTW → ORD | 20:27–21:30 | 63 |
| 748 | MCI → DTW → IAD | 19:35–20:25 | 50 |
| 770 | MKE → DTW → PHL | 19:57–20:45 | 48 |

## Transkriberade scheman

Skanningar och kolumner räknas från ett, kolumner från vänster. Lokala tider; +1 betyder ankomst nästa lokala dag. D = tom daganmärkning/dagligen; 6 = lördag; X6 = utom lördag; X7 = utom söndag; X67 = måndag–fredag. Rörelseantal gäller projektfönstret.

| Nummer | Sträcka | Lokala tider | Dagar | Utrustningskod | Rörelser | Skanning/kolumn |
|---|---|---|---|---|---:|---|
| 701 | BOS → DTW | 07:00–08:55 | D | 757 | 2 | 2/1 |
| 569 | BOS → DTW | 07:45–09:55 | D | 72S | 2 | 2/1 |
| 335 | BOS → DTW | 10:30–12:30 | D | 72S | 2 | 2/1 |
| 723 | BOS → DTW | 14:30–16:30 | D | D95 | 2 | 2/1 |
| 541 | BOS → DTW | 16:30–18:40 | D | D95 | 3 | 2/1 |
| 595 | BOS → DTW | 18:35–20:45 | X6 | M80 | 2 | 2/1 |
| 583 | BOS → DTW | 19:20–21:25 | X6 | D95 | 2 | 2/1 |
| 580 | DTW → BOS | 07:00–08:40 | X7 | D9S | 2 | 2/3 |
| 348 | DTW → BOS | 08:20–09:50 | X7 | 72S | 2 | 2/3 |
| 72 | DTW → BOS | 12:10–13:50 | D | D95 | 2 | 2/3 |
| 588 | DTW → BOS | 13:10–14:50 | D | D95 | 2 | 2/3 |
| 100 | DTW → BOS | 16:15–17:55 | D | M80 | 3 | 2/3 |
| 586 | DTW → BOS | 17:00–18:40 | D | D95 | 3 | 2/3 |
| 334 | DTW → BOS | 20:40–22:20 | D | 757 | 2 | 2/3 |
| 563 | DCA → DTW | 07:25–08:45 | D | D95 | 2 | 2/2 |
| 193 | DCA → DTW | 11:05–12:30 | D | D9S | 2 | 2/2 |
| 753 | DCA → DTW | 15:00–16:25 | D | D9S | 2 | 2/2 |
| 234 | DTW → DCA | 09:25–10:40 | D | D9S | 2 | 2/4 |
| 326 | DTW → DCA | 16:00–17:19 | D | D9S | 2 | 2/4 |
| 710 | DTW → DCA | 20:40–21:59 | D | D95 | 2 | 2/4 |
| 592 | DFW → DTW | 11:50–15:15 | D | D9S | 2 | 2/3 |
| 720 | DFW → DTW | 16:20–19:45 | D | D9S | 3 | 2/3 |
| 179 | DTW → DFW | 13:15–14:55 | D | D9S | 2 | 2/4 |
| 719 | DTW → DFW | 17:05–18:45 | D | D9S | 3 | 2/4 |
| 131 | IAD → DTW | 07:35–08:55 | D | D9S | 2 | 3/3 |
| 713 | IAD → DTW | 15:00–16:22 | D | DC9 | 2 | 3/3 |
| 137 | IAD → DTW | 19:30–20:52 | X6 | D9S | 2 | 3/3 |
| 134 | DTW → IAD | 12:20–13:34 | D | DC9 | 2 | 2/4 |
| 524 | DTW → IAD | 16:00–17:14 | D | D9S | 2 | 2/4 |
| 748 | DTW → IAD | 20:25–21:35 | D | D9S | 2 | 2/4 |
| 652 | JFK → DTW | 16:25–18:27 | X6 | DC9 | 2 | 3/3 |
| 743 | JFK → DTW | 18:25–20:27 | D | DC9 | 2 | 3/4 |
| 275 | DTW → JFK | 13:35–15:25 | X6 | DC9 | 1 | 2/4 |
| 775 | DTW → JFK | 15:50–17:40 | D | DC9 | 3 | 2/4 |
| 750 | LAS → DTW | 09:00–15:20 | D | M80 | 2 | 3/4 |
| 531 | DTW → LAS | 19:30–20:35 | 6 | M80 | 0 | 2/4 |
| 533 | DTW → LAS | 21:55–23:00 | X6 | M80 | 2 | 2/4 |
| 338 | LAX → DTW | 00:25–07:30 | D | 72S | 2 | 3/4 |
| 330 | LAX → DTW | 08:05–15:10 | D | 72S | 2 | 3/4 |
| 347 | LAX → DTW | 12:45–19:45 | D | 757 | 3 | 3/4 |
| 336 | LAX → DTW | 15:25–22:30 | D | 72S | 2 | 3/4 |
| 333 | DTW → LAX | 09:35–11:15 | D | 757 | 2 | 2/4 |
| 335 | DTW → LAX | 13:15–14:45 | D | 72S | 3 | 2/4 |
| 337 | DTW → LAX | 17:15–18:45 | D | 72S | 3 | 2/4 |
| 339 | DTW → LAX | 19:20–20:58 | D | 72S | 2 | 2/4 |
| 81 | LGA → DTW | 07:05–08:55 | D | D9S | 2 | 3/4 |
| 17 | LGA → DTW | 07:45–09:35 | X67 | D9S | 1 | 3/4 |
| 343 | LGA → DTW | 10:40–12:30 | D | 72S | 2 | 3/4 |
| 543 | LGA → DTW | 11:29–13:19 | D | D9S | 2 | 3/4 |
| 739 | LGA → DTW | 14:30–16:20 | D | D9S | 2 | 3/4 |
| 545 | LGA → DTW | 16:29–18:19 | D | D9S | 3 | 3/4 |
| 547 | LGA → DTW | 18:05–19:55 | X6 | D9S | 2 | 3/4 |
| 549 | LGA → DTW | 18:59–20:49 | D | D9S | 2 | 3/4 |
| 540 | DTW → LGA | 07:00–08:29 | X67 | D9S | 1 | 2/4 |
| 338 | DTW → LGA | 08:30–10:00 | X7 | 72S | 2 | 2/4 |
| 358 | DTW → LGA | 09:30–11:00 | D | D9S | 2 | 2/4 |
| 544 | DTW → LGA | 12:20–13:50 | D | D9S | 2 | 2/4 |
| 546 | DTW → LGA | 13:15–14:45 | D | D9S | 2 | 2/4 |
| 773 | DTW → LGA | 15:55–17:25 | D | D9S | 3 | 2/4 |
| 737 | DTW → LGA | 19:30–21:00 | X6 | D9S | 2 | 2/4 |
| 752 | DTW → LGA | 20:45–22:15 | D | D9S | 2 | 2/4 |
| 608 | MCI → DTW | 08:55–11:30 | D | D9S | 2 | 3/4 |
| 604 | MCI → DTW | 12:40–15:15 | D | D9S | 2 | 3/4 |
| 748 | MCI → DTW | 17:00–19:35 | D | D9S | 2 | 3/4 |
| 323 | DTW → MCI | 10:35–11:32 | D | D9S | 2 | 3/1 |
| 747 | DTW → MCI | 17:05–18:02 | D | D9S | 3 | 3/1 |
| 143 | DTW → MCI | 21:55–22:52 | X6 | D9S | 2 | 3/1 |
| 212 | MKE → DTW | 07:00–08:53 | X7 | D95 | 2 | 5/1 |
| 202 | MKE → DTW | 07:00–08:53 | X7 | D9S | 2 | 5/1 |
| 614 | MKE → DTW | 09:45–11:38 | D | D95 | 2 | 5/1 |
| 206 | MKE → DTW | 13:25–15:18 | D | D95 | 2 | 5/1 |
| 530 | MKE → DTW | 14:40–16:33 | X6 | D9S | 1 | 5/1 |
| 760 | MKE → DTW | 15:40–17:33 | D | D95 | 3 | 5/1 |
| 770 | MKE → DTW | 18:00–19:57 | D | D95 | 2 | 5/1 |
| 201 | DTW → MKE | 09:25–09:23 | D | D95 | 2 | 3/1 |
| 417 | DTW → MKE | 13:00–12:58 | D | D95 | 2 | 3/1 |
| 543 | DTW → MKE | 14:10–14:08 | X67 | D9S | 1 | 3/1 |
| 759 | DTW → MKE | 17:10–17:08 | D | D95 | 3 | 3/1 |
| 699 | DTW → MKE | 17:10–17:08 | D | D9S | 3 | 3/1 |
| 209 | DTW → MKE | 19:20–19:18 | D | D9S | 2 | 3/1 |
| 215 | DTW → MKE | 22:00–21:58 | X6 | D95 | 2 | 3/1 |
| 10 | ORD → DTW | 07:00–08:58 | X7 | DC9 | 2 | 5/4 |
| 12 | ORD → DTW | 08:00–09:58 | X6 | DC9 | 1 | 5/4 |
| 14 | ORD → DTW | 09:00–10:58 | X7 | DC9 | 2 | 5/4 |
| 420 | ORD → DTW | 10:00–11:58 | X67 | D95 | 1 | 5/4 |
| 18 | ORD → DTW | 11:00–12:58 | D | DC9 | 2 | 5/4 |
| 20 | ORD → DTW | 12:00–13:50 | X67 | DC9 | 1 | 5/4 |
| 22 | ORD → DTW | 13:00–14:58 | D | D95 | 2 | 5/4 |
| 24 | ORD → DTW | 14:00–15:58 | D | D9S | 2 | 5/4 |
| 26 | ORD → DTW | 15:00–16:58 | X7 | D95 | 2 | 5/4 |
| 28 | ORD → DTW | 16:00–17:58 | X6 | D9S | 2 | 5/4 |
| 728 | ORD → DTW | 17:00–18:58 | D | D95 | 2 | 5/4 |
| 32 | ORD → DTW | 18:00–19:58 | D | D9S | 2 | 5/4 |
| 36 | ORD → DTW | 19:00–20:58 | X6 | D9S | 2 | 5/4 |
| 9 | DTW → ORD | 07:35–07:35 | X67 | D95 | 1 | 3/1 |
| 11 | DTW → ORD | 08:25–08:30 | X7 | DC9 | 2 | 3/1 |
| 15 | DTW → ORD | 09:30–09:35 | D | D95 | 2 | 3/1 |
| 17 | DTW → ORD | 10:30–10:35 | X67 | D9S | 1 | 3/1 |
| 19 | DTW → ORD | 11:30–11:35 | X67 | DC9 | 1 | 3/1 |
| 21 | DTW → ORD | 12:30–12:35 | X67 | D9S | 1 | 3/1 |
| 23 | DTW → ORD | 13:30–13:35 | D | D95 | 2 | 3/1 |
| 727 | DTW → ORD | 14:30–14:35 | X67 | DC9 | 1 | 3/1 |
| 27 | DTW → ORD | 15:40–15:44 | D | D95 | 2 | 3/1 |
| 29 | DTW → ORD | 16:30–16:35 | X6 | D9S | 2 | 3/1 |
| 31 | DTW → ORD | 17:30–17:35 | D | D9S | 2 | 3/1 |
| 33 | DTW → ORD | 18:35–18:35 | X6 | D9S | 2 | 3/1 |
| 729 | DTW → ORD | 19:35–19:35 | D | DC9 | 2 | 3/1 |
| 743 | DTW → ORD | 21:30–21:35 | X6 | DC9 | 2 | 3/1 |
| 201 | PHL → DTW | 07:10–08:45 | D | D95 | 2 | 6/1 |
| 205 | PHL → DTW | 10:45–12:20 | D | D9S | 2 | 6/1 |
| 39 | PHL → DTW | 13:15–14:50 | D | D9S | 2 | 6/1 |
| 715 | PHL → DTW | 14:55–16:30 | D | D9S | 2 | 6/1 |
| 209 | PHL → DTW | 17:05–18:40 | D | D9S | 3 | 6/1 |
| 211 | PHL → DTW | 19:40–21:15 | X6 | D95 | 2 | 6/1 |
| 200 | DTW → PHL | 08:40–10:01 | X7 | D9S | 2 | 3/1 |
| 202 | DTW → PHL | 09:40–11:01 | D | D9S | 2 | 3/1 |
| 204 | DTW → PHL | 12:15–13:36 | D | D9S | 2 | 3/1 |
| 206 | DTW → PHL | 16:15–17:36 | D | D95 | 3 | 3/1 |
| 26 | DTW → PHL | 17:40–19:01 | D | D95 | 2 | 3/1 |
| 770 | DTW → PHL | 20:45–22:06 | D | D95 | 2 | 3/1 |
| 100 | PHX → DTW | 09:35–15:10 | D | M80 | 2 | 6/1 |
| 734 | PHX → DTW | 14:10–19:35 | D | 757 | 3 | 6/1 |
| 584 | PHX → DTW | 18:35–00:10 +1 | D | D9S | 2 | 6/1 |
| 585 | DTW → PHX | 09:45–11:50 | D | 757 | 2 | 3/1 |
| 587 | DTW → PHX | 13:05–15:15 | D | 72S | 2 | 3/1 |
| 733 | DTW → PHX | 19:25–21:35 | D | M80 | 2 | 3/1 |
| 772 | SAN → DTW | 08:05–15:10 | D | 72S | 2 | 6/1 |
| 741 | DTW → SAN | 19:35–21:05 | D | 72S | 2 | 3/1 |
| 742 | SEA → DTW | 08:15–15:05 | D | 72S | 2 | 6/1 |
| 769 | DTW → SEA | 19:35–21:20 | D | 72S | 2 | 3/1 |
| 344 | SFO → DTW | 08:05–15:10 | D | 72S | 2 | 6/1 |
| 702 | SFO → DTW | 12:35–19:43 | D | 757 | 3 | 6/1 |
| 346 | SFO → DTW | 16:00–23:05 | D | 72S | 2 | 6/1 |
| 701 | DTW → SFO | 09:45–11:42 | D | 757 | 2 | 3/1 |
| 343 | DTW → SFO | 13:15–15:05 | D | 72S | 3 | 3/1 |
| 751 | DTW → SFO | 19:35–21:25 | D | 72S | 2 | 3/1 |
| 174 | STL → DTW | 09:00–11:20 | D | DC9 | 2 | 6/2 |
| 126 | STL → DTW | 12:55–15:15 | D | D9S | 2 | 6/2 |
| 726 | STL → DTW | 17:20–19:40 | D | D9S | 2 | 6/2 |
| 151 | DTW → STL | 09:40–10:11 | D | D95 | 2 | 3/1 |
| 725 | DTW → STL | 17:00–17:31 | D | DC9 | 3 | 3/1 |
| 712 | DTW → STL | 19:35–20:06 | D | DC9 | 2 | 3/1 |
| 551 | LAS → SAN | 11:00–11:55 | D | D9S | 2 | 3/4 |
| 553 | LAS → SAN | 14:10–15:05 | D | D9S | 3 | 3/4 |
| 552 | SAN → LAS | 12:25–13:18 | D | D9S | 2 | 6/1 |
| 550 | SAN → LAS | 15:35–16:30 | D | D9S | 2 | 6/1 |

## Regional prioritet och kvarstående arbete

Centralamerika/Karibien, samma **31 forskningsområden**, ligger kvar på **532 rörelser, 158 riktade par, 38 flygplatser och 64 inrikesrörelser**. **St. Thomas 14, Tortola 8 och Nicaragua 0** är oförändrade. Angränsande Mexiko har 20; Mellanösternurvalet har 72. Noll betyder en datalucka, inte frånvaro av historisk trafik.

**Ingen ny regional arkivsökning gjordes denna omgång.** Arbetet fokuserade på återstående rader i den redan tillgängliga Republic-originaltidtabellen. `regional_source_search_batch135.json` redovisar detta uttryckligen och hänvisar till den föregående sökomgången.

Nicaragua och St. Thomas står kvar som prioritet. De tidigare spåren efter Aero Virgin Islands 15 december 1985, Gull Air 15 februari 1986, LACSA, Caribbean Express, Eastern, American, BWIA, TACA, Aviateca, TAN SAHSA och sjöflygets tider/hamnar bevaras. Tidigare LIAT-, Challenge-, Arrow-, Airways International-, Air France F27-, PBA789- och specialflygsfrågor är oförändrade. Inga nya militär-, privat- eller specialflygrörelser importeras.

För Republic återstår andra Detroit-linjer, Minneapolis-nätet, separata SFO–SMF/SMF–SFO-ben, Express-operatörerna, ändringsblad och belägg för faktiskt genomförande. Inga extra genomgående MEM–SAN- eller MEM–SMF-rörelser skapas.

## Totalt, verifiering och installation

**10 632 rörelser: 10 631 planerade och en tidigare bekräftad. 6 382 katalogscheman, varav 6 361 granskade. 393 flygplatser/platser, 121 länder/territorier och 1 936 riktade par. 111 operatörsposter: 68 med rörelser, 43 utan. 94 källposter och 36 bidragande tidtabellsutgåvor.** Ingen verifierad global bolagsinventering eller färdigprocent finns.

**20 befintliga Python-tester och tre generatorkontroller passerar.** Samtliga 1 067 Republic-rörelser har kontrollerats för dubbletter och samtidig överlappning av samma flygnummer; inga hittades. Alla äldre rörelse- och schemaobjekt är oförändrade. ZIP-filens CRC och varje fils hash verifieras mot manifestet.

**Unreal Editor och spelet har inte körts här.** Paketet är kumulativt: stäng Unreal och slå samman `Plugins/` och `Tools/` med projektroten bredvid `.uproject`. **PaintAirplane-rättelsen f23b73a måste redan vara kompilerad.** Ingen ny C++-kod ingår.
