# Republic kring Minneapolis och Portland–Seattle – v139 / batch136

Granskat 2 oktober 2026. **312 nya planerade flygrörelser**, totalt **10 944**. Tillägget omfattar **43 riktade sträckor, varav 31 är nya i databasen**, och **152 nya granskade scheman**. 151 ger rörelser i projektfönstret; ett söndagsschema ger noll.

Alla tillägg är inrikes i USA och räknas utanför Centralamerika/Karibien-urvalet. **Inga nya flygplatser, länder, operatörer eller tidtabellsutgåvor**. Republic har nu **1 379 rörelser från 672 scheman**. Hela nätet är ännu inte granskat.

## Rättelse av äldre flygnummer från v138

Originalets **LAX–DTW 12:45–19:45, dagligen, 757**, på skanning 3 kolumn 4, har flygnummer **334**. Det fellästes som 347 i v138. Felet upptäcktes när tidskontrollen jämförde med den korrekt tryckta nya raden **347 MSP–SFO 17:25–19:10**. Förstorad originalbild visar 334 för Los Angeles–Detroit.

**Ett äldre schema och dess tre daterade rörelser rättas. Endast fälten `flight` och `notes` ändras.** Samtliga äldre ID, datum, tider, sträckor och övriga fält bevaras. Det stabila schema-ID:t `republic-airlines-347-LAX-DTW-1245-b135` behålls; texten 347 i ID:t är alltså historisk och anger inte längre aktuellt flygnummer. Före/efter-värden, källhänvisning och de tre oförändrade UTC-intervallen finns i `errata_batch136.json`.

Alla **10 632 äldre rörelser** och **6 382 äldre scheman** behålls. **10 629 rörelseobjekt och 6 381 schemaobjekt är helt oförändrade**. Äldre flygplats-, land- och operatörsposter bevaras. Republic-källpostens granskningsmetadata utökas. Äldre forskningsrapporter och rättelser bevaras byte för byte som historiska ögonblicksbilder; denna rättelse ersätter deras felläsning av just LAX–DTW-numret.

## Nya rörelser

| Riktad sträcka | Nya rörelser | Ny sträcka i databasen |
|---|---:|---|
| ATL–MSP | 7 | Nej |
| BOS–MSP | 4 | Ja |
| DEN-STAPLETON–MSP | 7 | Ja |
| DFW–MSP | 4 | Ja |
| DTW–MSP | 22 | Nej |
| IAD–MSP | 5 | Ja |
| JFK–MSP | 2 | Ja |
| LAS–MSP | 5 | Ja |
| LAX–MSP | 9 | Ja |
| LGA–MSP | 9 | Ja |
| MCI–MSP | 6 | Ja |
| MKE–MSP | 11 | Nej |
| MSP–ATL | 5 | Nej |
| MSP–BOS | 4 | Ja |
| MSP–DEN-STAPLETON | 6 | Ja |
| MSP–DFW | 4 | Ja |
| MSP–DTW | 23 | Nej |
| MSP–IAD | 4 | Ja |
| MSP–JFK | 2 | Ja |
| MSP–LAS | 4 | Ja |
| MSP–LAX | 6 | Ja |
| MSP–LGA | 8 | Ja |
| MSP–MCI | 6 | Ja |
| MSP–MKE | 10 | Nej |
| MSP–ORD | 24 | Nej |
| MSP–PDX | 4 | Ja |
| MSP–PHL | 4 | Nej |
| MSP–PHX | 10 | Ja |
| MSP–SAN | 2 | Ja |
| MSP–SEA | 4 | Nej |
| MSP–SFO | 6 | Ja |
| MSP–SLC | 4 | Ja |
| MSP–STL | 6 | Ja |
| ORD–MSP | 27 | Nej |
| PDX–MSP | 2 | Ja |
| PDX–SEA | 2 | Nej |
| PHL–MSP | 4 | Ja |
| PHX–MSP | 12 | Ja |
| SAN–MSP | 2 | Ja |
| SEA–MSP | 7 | Nej |
| SFO–MSP | 7 | Ja |
| SLC–MSP | 5 | Ja |
| STL–MSP | 7 | Ja |

MSP = Minneapolis/St. Paul; ATL = Atlanta; BOS = Boston; DEN-STAPLETON = historiska Denver Stapleton; DFW = Dallas/Fort Worth; DTW = Detroit; IAD = Washington Dulles; JFK = New York Kennedy; LGA = New York LaGuardia; LAS = Las Vegas; LAX = Los Angeles; MCI = Kansas City; MKE = Milwaukee; ORD = Chicago O’Hare; PDX = Portland; PHL = Philadelphia; PHX = Phoenix; SAN = San Diego; SEA = Seattle; SFO = San Francisco; SLC = Salt Lake City; STL = St. Louis. Befintliga historiskt granskade flygplatsmarkörer används oförändrade.

En rörelse är ett daterat fysiskt flygben. Flera avgångar på samma linje räknas var för sig, liksom de två riktningarna.

## Källa och avgränsning

[Republic Airlines Employee System Timetable, 14 februari–1 mars 1986](https://northwestairlineshistory.org/wp-content/uploads/2018/07/REP-employee-schedule-1986-02-14.pdf), bevarad hos Northwest Airlines History Center. Omslagets uttryckliga giltighet täcker importperioden. Den tidigare hämtade kompletta originaltidtabellen återanvänds: tabeller på skanning **2, 3, 5 och 6**, legend på skanning 6. PDF och sidbilder dokumenteras med URL och SHA-256 i `caribbean_source_evidence_batch136.json`. **Originalskanningarna distribueras inte.**

De **43 valda riktningsrubrikerna innehåller 154 rader**: **152 med tom ST-kolumn** katalogiseras som separata nonstopben; **två ST=1-rader** är genomgående resor och importeras inte som extra nonstopflyg. Inga EXP-rader ingår i urvalet. ST betyder Stops. A/P betyder AM/PM, X betyder undantag och 1–7 betyder måndag–söndag. Tom FRQ tolkas som dagligen. Tryckta utrustningskoder bevaras som de står.

Detta är ett avgränsat urval av framför allt större Minneapolis-linjer. Övriga Detroit-/Minneapolis-linjer, andra huvudlinjer, Express-trafiken, ändringsblad och faktisk drift återstår. Källans legend skiljer Express Airlines I (1400–1699) från Simmons Airlines (1700–1899); inga sådana rader importeras här.

## Portland–Seattle och genomgående resor

| Genomgående rad, inte extra nonstop | Lokala tider | ST | Bedömning |
|---|---|---:|---|
| 75 MSP–SEA | 09:30–12:10 | 1 | 75 MSP–PDX 09:30–11:05 finns separat. PDX–SEA trycks som 76, inte 75. Inget 75-ben eller byte av flygnummer/flygplansindivid gissas. |
| 76 PDX–MSP | 11:35–17:45 | 1 | Separata 76 PDX–SEA 11:35–12:10 och SEA–MSP 12:40–17:45 importeras. |

Nummer 76 fortsätter tidsmässigt MSP–LGA 18:30–21:52. De separata benen behåller egna tider och datum. Tidsmässig förenlighet belägger inte samma flygplansindivid eller faktiskt genomförande.

## Tider och fönstergränser

Projektfönstret är **27 februari 1986 kl. 22:21:30 UTC till 1 mars kl. 22:21:30 UTC**. Ett flyg tas med när dess intervall överlappar fönstret. Avgång före startgränsen kan alltså ge en rörelse om flyget fortfarande är i luften.

| UTC-offset under de tre datumen | Flygplatser i urvalet |
|---|---|
| −5 timmar | ATL, BOS, DTW, IAD, JFK, LGA, PHL |
| −6 timmar | MSP, DFW, MCI, MKE, ORD, STL |
| −7 timmar | DEN-STAPLETON, PHX, SLC |
| −8 timmar | LAS, LAX, PDX, SAN, SEA, SFO |

Offseterna är kontrollerade mot varje flygplats tidszon för samtliga tre datum. Lokaltider följer tidtabellskonvention och är förenliga med flygtiderna; ingen separat uttrycklig all-times-local-text har återfunnits i originalet.

**69 rörelser har lokal avgång torsdag, 151 fredag och 92 lördag.** Fördelningen per schema är **19 med tre utfall, 123 med två, nio med ett och ett med noll**. Veckodagsundantag och exakt intervallöverlapp styr resultatet.

- **124 LAS–MSP 11:45–16:45** är endast söndag. Projektfönstret omfattar torsdag–lördag, så schemat katalogiseras men ger **noll** rörelser.
- **728 MSP–ORD 15:15–16:17** landar kl. 22:17 UTC, 4 minuter och 30 sekunder före torsdagens startgräns. Det ger fredag och lördag, två rörelser.
- **55 ORD–MSP 16:14–17:30** avgår kl. 22:14 UTC, 7 minuter och 30 sekunder före lördagens slutgräns. Det ger tre rörelser.
- Inget nytt schema har ankomst nästa lokala kalenderdag. UTC-datumskiften är ändå normaliserade per daterad rörelse.
- **MKE–MSP 10:45–11:45 är nummer 256**, inte 265. Detta kontrollerades i förstorad originalbild före import; ingen äldre post berörs av den transkriptionskontrollen.

Tryckta rader med parallella nummer och samma klockslag behålls med respektive dag- och utrustningskod: DTW–MSP 771/423; MSP–DTW 58/204, 740/788 och 718/710; PHX–MSP 120/102 och 132/104; MSP–PHX 101/103 och 107/505. Identiska klockslag räcker inte för att fastställa samma fysiska flygplansindivid eller slå samman de olika publicerade raderna.

Samtliga **152 flygtider, 35–236 minuter**, även nollutfallet, och **312 nya UTC-intervall** har kontrollerats mot separat manuellt granskade varaktigheter och förväntade datum. Även de tre äldre UTC-intervallen för rättad 334 är oförändrade och separat kontrollerade. Fasta UTC-offseter matchar generatorns tidszonsberäkning. Detta är samma originalmaterial, inte en andra oberoende källa eller granskare.

## Angränsande ben

**91 daterade följder med samma flygnummer**, fördelade på **49 klock-/sträckmönster**, har kontrollerats. **43** knyter ihop nya och äldre ben; **två** är äldre LAX–DTW–BOS-följder som blir förenliga med rättat nummer 334; **46** kopplar nya ben till nya ben. Uppehållen är **25–70 minuter**.

Klockskillnaderna har granskats manuellt. Kontrollen visar tidsmässig förenlighet; den bekräftar inte faktisk drift, samma flygplansindivid eller en bokningsbar anslutning. Källans angivna 30-minuters minimum gäller dess tre namngivna nav Memphis, Detroit och Minneapolis och används inte som ett generellt krav i Milwaukee. Exempelvis har 256 MEM–MKE–MSP 25 minuter där, och 760 MSP–MKE–DTW 27 minuter.

| Nummer | Benföljd | Ankomst–nästa avgång vid mellanpunkten | Markuppehåll, minuter |
|---|---|---|---:|
| 9 | DTW → ORD → MSP | 07:35–08:15 | 40 |
| 17 | DTW → ORD → MSP | 10:35–11:15 | 40 |
| 18 | MSP → ORD → DTW | 10:14–11:00 | 46 |
| 21 | DTW → ORD → MSP | 12:35–13:15 | 40 |
| 22 | MSP → ORD → DTW | 12:17–13:00 | 43 |
| 23 | DTW → ORD → MSP | 13:35–14:14 | 39 |
| 24 | MSP → ORD → DTW | 13:17–14:00 | 43 |
| 29 | DTW → ORD → MSP | 16:35–17:14 | 39 |
| 31 | DTW → ORD → MSP | 17:35–18:15 | 40 |
| 32 | MSP → ORD → DTW | 17:17–18:00 | 43 |
| 33 | DTW → ORD → MSP | 18:35–19:15 | 40 |
| 36 | MSP → ORD → DTW | 18:17–19:00 | 43 |
| 64 | SEA → MSP → MCO | 12:25–13:10 | 45 |
| 75 | STL → MSP → PDX | 08:40–09:30 | 50 |
| 76 | PDX → SEA → MSP | 12:10–12:40 | 30 |
| 76 | SEA → MSP → LGA | 17:45–18:30 | 45 |
| 85 | LGA → MSP → LAS | 16:30–17:40 | 70 |
| 101 | PHL → MSP → PHX | 08:50–09:30 | 40 |
| 104 | PHX → MSP → MKE | 17:32–18:25 | 53 |
| 105 | BOS → MSP → PHX | 11:35–12:20 | 45 |
| 120 | PHX → MSP → LGA | 12:27–13:30 | 63 |
| 204 | MSP → DTW → PHL | 11:15–12:15 | 60 |
| 222 | IAD → MSP → MCI | 08:50–09:35 | 45 |
| 226 | MSP → MCI → MEM | 19:54–20:35 | 41 |
| 256 | MEM → MKE → MSP | 10:20–10:45 | 25 |
| 300 | SFO → MSP → MKE | 12:40–13:25 | 45 |
| 301 | MKE → MSP → LAX | 08:50–09:35 | 45 |
| 303 | ORD → MSP → LAX | 11:30–12:25 | 55 |
| 304 | LAX → MSP → MKE | 20:15–20:55 | 40 |
| 334 | LAX → DTW → BOS | 19:45–20:40 | 55 |
| 345 | DTW → MSP → SFO | 11:27–12:15 | 48 |
| 401 | MEM → STL → MSP | 09:40–10:10 | 30 |
| 423 | MCO → DTW → MSP | 18:30–19:25 | 55 |
| 478 | LAS → MSP → PHL | 17:40–18:30 | 50 |
| 505 | MKE → MSP → PHX | 16:40–17:35 | 55 |
| 560 | DEN-STAPLETON → MSP → DTW | 20:20–21:00 | 40 |
| 562 | DEN-STAPLETON → MSP → PHL | 12:30–13:20 | 50 |
| 563 | DCA → DTW → MSP | 08:45–09:25 | 40 |
| 566 | DEN-STAPLETON → MSP → IAD | 17:35–18:15 | 40 |
| 567 | STL → MSP → DEN-STAPLETON | 16:35–17:35 | 60 |
| 614 | MSP → MKE → DTW | 09:13–09:45 | 32 |
| 675 | ATL → MSP → SLC | 08:40–09:20 | 40 |
| 704 | SFO → MSP → BOS | 17:40–18:20 | 40 |
| 710 | MSP → DTW → DCA | 19:50–20:40 | 50 |
| 727 | DTW → ORD → MSP | 14:35–15:15 | 40 |
| 728 | MSP → ORD → DTW | 16:17–17:00 | 43 |
| 740 | LAX → MSP → DTW | 12:20–13:00 | 40 |
| 759 | DTW → MKE → MSP | 17:08–17:40 | 32 |
| 760 | MSP → MKE → DTW | 15:13–15:40 | 27 |

## Transkriberade scheman

Skanningar och kolumner räknas från ett, kolumner från vänster. Alla tider är lokala. D = tom daganmärkning/dagligen; 6 = lördag; 7 = söndag; X6 = utom lördag; X7 = utom söndag; X67 = måndag–fredag. Rörelseantal gäller projektfönstret.

| Nummer | Sträcka | Lokala tider | Dagar | Utrustningskod | Rörelser | Skanning/kolumn |
|---|---|---|---|---|---:|---|
| 675 | ATL → MSP | 07:10–08:40 | X7 | DC9 | 2 | 2/1 |
| 463 | ATL → MSP | 15:15–16:45 | D | D9S | 3 | 2/1 |
| 467 | ATL → MSP | 18:40–20:08 | D | DC9 | 2 | 2/1 |
| 460 | MSP → ATL | 08:15–11:25 | D | DC9 | 2 | 5/2 |
| 468 | MSP → ATL | 14:50–18:09 | D | DC9 | 3 | 5/2 |
| 105 | BOS → MSP | 09:30–11:35 | D | D9S | 2 | 2/1 |
| 69 | BOS → MSP | 17:45–19:42 | D | D9S | 2 | 2/1 |
| 620 | MSP → BOS | 13:20–16:50 | D | D9S | 2 | 5/2 |
| 704 | MSP → BOS | 18:20–21:44 | D | 72S | 2 | 5/2 |
| 562 | DEN-STAPLETON → MSP | 09:50–12:30 | D | D9S | 2 | 2/3 |
| 566 | DEN-STAPLETON → MSP | 14:55–17:35 | D | DC9 | 3 | 2/3 |
| 560 | DEN-STAPLETON → MSP | 17:40–20:20 | D | DC9 | 2 | 2/3 |
| 561 | MSP → DEN-STAPLETON | 08:20–09:15 | D | D9S | 2 | 5/2 |
| 565 | MSP → DEN-STAPLETON | 12:20–13:15 | D | DC9 | 2 | 5/2 |
| 567 | MSP → DEN-STAPLETON | 17:35–18:30 | D | DC9 | 2 | 5/2 |
| 217 | DFW → MSP | 07:00–09:08 | D | D9S | 2 | 2/3 |
| 219 | DFW → MSP | 19:20–21:28 | D | D9S | 2 | 2/3 |
| 216 | MSP → DFW | 08:20–10:30 | D | D9S | 2 | 5/2 |
| 218 | MSP → DFW | 17:30–19:44 | D | DC9 | 2 | 5/2 |
| 119 | DTW → MSP | 08:00–08:42 | D | D9S | 2 | 3/1 |
| 563 | DTW → MSP | 09:25–10:07 | D | D95 | 2 | 3/1 |
| 345 | DTW → MSP | 10:45–11:27 | D | M80 | 2 | 3/1 |
| 253 | DTW → MSP | 13:00–13:40 | D | D95 | 2 | 3/1 |
| 717 | DTW → MSP | 15:30–16:12 | D | D95 | 2 | 3/1 |
| 117 | DTW → MSP | 16:05–16:47 | D | M80 | 3 | 3/1 |
| 67 | DTW → MSP | 17:00–17:38 | D | D95 | 3 | 3/1 |
| 771 | DTW → MSP | 19:25–20:07 | X6 | DC9 | 2 | 3/1 |
| 423 | DTW → MSP | 19:25–20:07 | D | D95 | 2 | 3/1 |
| 325 | DTW → MSP | 21:55–22:35 | X6 | 757 | 2 | 3/1 |
| 575 | MSP → DTW | 06:15–08:43 | X7 | 757 | 2 | 5/3 |
| 58 | MSP → DTW | 08:45–11:15 | D | 72S | 2 | 5/3 |
| 204 | MSP → DTW | 08:45–11:15 | X7 | D9S | 2 | 5/3 |
| 528 | MSP → DTW | 10:50–13:20 | D | D95 | 2 | 5/3 |
| 740 | MSP → DTW | 13:00–15:30 | D | M80 | 2 | 5/3 |
| 788 | MSP → DTW | 13:00–15:30 | D | D9S | 2 | 5/3 |
| 707 | MSP → DTW | 16:00–18:30 | D | D9S | 3 | 5/3 |
| 718 | MSP → DTW | 17:20–19:50 | X6 | D95 | 2 | 5/3 |
| 710 | MSP → DTW | 17:20–19:50 | D | D95 | 2 | 5/3 |
| 112 | MSP → DTW | 18:35–21:05 | D | M80 | 2 | 5/3 |
| 560 | MSP → DTW | 21:00–23:30 | D | DC9 | 2 | 5/3 |
| 222 | IAD → MSP | 07:20–08:50 | X7 | DC9 | 2 | 3/3 |
| 129 | IAD → MSP | 15:35–17:05 | D | D9S | 3 | 3/3 |
| 66 | MSP → IAD | 06:40–09:45 | X7 | D9S | 2 | 5/3 |
| 566 | MSP → IAD | 18:15–21:20 | X6 | DC9 | 2 | 5/3 |
| 745 | JFK → MSP | 17:30–19:30 | D | D9S | 2 | 3/4 |
| 746 | MSP → JFK | 13:15–16:45 | D | D9S | 2 | 5/3 |
| 82 | LAS → MSP | 07:35–12:25 | D | D9S | 2 | 3/4 |
| 124 | LAS → MSP | 11:45–16:35 | 7 | M80 | 0 | 3/4 |
| 478 | LAS → MSP | 12:50–17:40 | D | D9S | 3 | 3/4 |
| 83 | MSP → LAS | 09:35–10:45 | D | D9S | 2 | 5/3 |
| 85 | MSP → LAS | 17:40–18:50 | D | D9S | 2 | 5/3 |
| 740 | LAX → MSP | 07:00–12:20 | D | M80 | 2 | 3/4 |
| 302 | LAX → MSP | 12:30–17:45 | D | 72S | 3 | 3/4 |
| 304 | LAX → MSP | 15:00–20:15 | D | 72S | 2 | 3/4 |
| 308 | LAX → MSP | 18:15–23:30 | D | 72S | 2 | 3/4 |
| 301 | MSP → LAX | 09:35–11:10 | D | 72S | 2 | 5/3 |
| 303 | MSP → LAX | 12:25–14:00 | D | 72S | 2 | 5/3 |
| 305 | MSP → LAX | 17:30–19:10 | D | M80 | 2 | 5/3 |
| 73 | LGA → MSP | 07:00–08:50 | D | D9S | 2 | 3/4 |
| 571 | LGA → MSP | 10:00–12:00 | D | D9S | 2 | 3/4 |
| 85 | LGA → MSP | 14:30–16:30 | D | D9S | 3 | 3/4 |
| 507 | LGA → MSP | 17:40–19:44 | D | D9S | 2 | 3/4 |
| 500 | MSP → LGA | 08:15–11:29 | D | D9S | 2 | 5/3 |
| 502 | MSP → LGA | 12:10–15:29 | D | D9S | 2 | 5/3 |
| 120 | MSP → LGA | 13:30–16:52 | D | D9S | 2 | 5/3 |
| 76 | MSP → LGA | 18:30–21:52 | D | D9S | 2 | 5/3 |
| 221 | MCI → MSP | 07:30–08:35 | D | DC9 | 2 | 3/4 |
| 223 | MCI → MSP | 11:20–12:25 | D | DC9 | 2 | 3/4 |
| 225 | MCI → MSP | 18:30–19:35 | D | D9S | 2 | 3/4 |
| 222 | MSP → MCI | 09:35–10:49 | D | DC9 | 2 | 5/3 |
| 224 | MSP → MCI | 13:20–14:34 | D | D9S | 2 | 5/3 |
| 226 | MSP → MCI | 18:40–19:54 | D | D9S | 2 | 5/3 |
| 301 | MKE → MSP | 07:50–08:50 | D | 72S | 2 | 5/1 |
| 256 | MKE → MSP | 10:45–11:45 | D | D95 | 2 | 5/1 |
| 505 | MKE → MSP | 15:30–16:40 | D | M80 | 3 | 5/1 |
| 759 | MKE → MSP | 17:40–18:40 | D | D95 | 2 | 5/1 |
| 260 | MKE → MSP | 18:35–19:35 | D | D9S | 2 | 5/1 |
| 614 | MSP → MKE | 08:15–09:13 | D | D95 | 2 | 5/3 |
| 300 | MSP → MKE | 13:25–14:23 | D | M80 | 2 | 5/3 |
| 760 | MSP → MKE | 14:15–15:13 | D | D95 | 2 | 5/3 |
| 104 | MSP → MKE | 18:25–19:23 | D | D95 | 2 | 5/3 |
| 304 | MSP → MKE | 20:55–21:50 | D | 72S | 2 | 5/3 |
| 37 | ORD → MSP | 07:15–08:30 | X7 | D95 | 2 | 5/4 |
| 9 | ORD → MSP | 08:15–09:30 | X67 | D95 | 1 | 5/4 |
| 41 | ORD → MSP | 09:15–10:30 | X67 | D9S | 1 | 5/4 |
| 303 | ORD → MSP | 10:15–11:30 | D | 72S | 2 | 5/4 |
| 17 | ORD → MSP | 11:15–12:30 | X67 | D9S | 1 | 5/4 |
| 393 | ORD → MSP | 12:15–13:30 | X67 | D9S | 1 | 5/4 |
| 21 | ORD → MSP | 13:15–14:30 | D | D9S | 2 | 5/4 |
| 23 | ORD → MSP | 14:14–15:30 | X6 | D9S | 1 | 5/4 |
| 727 | ORD → MSP | 15:15–16:30 | X7 | DC9 | 3 | 5/4 |
| 55 | ORD → MSP | 16:14–17:30 | D | D9S | 3 | 5/4 |
| 29 | ORD → MSP | 17:14–18:30 | X6 | D9S | 2 | 5/4 |
| 31 | ORD → MSP | 18:15–19:30 | D | D9S | 2 | 5/4 |
| 33 | ORD → MSP | 19:15–20:30 | X6 | D9S | 2 | 5/4 |
| 35 | ORD → MSP | 20:15–21:30 | X6 | 72S | 2 | 5/4 |
| 545 | ORD → MSP | 20:55–22:10 | X6 | D9S | 2 | 5/4 |
| 40 | MSP → ORD | 07:15–08:17 | X67 | D95 | 1 | 5/3 |
| 38 | MSP → ORD | 08:15–09:17 | D | 72S | 2 | 5/3 |
| 18 | MSP → ORD | 09:15–10:14 | X7 | DC9 | 2 | 5/3 |
| 16 | MSP → ORD | 10:15–11:17 | X67 | D9S | 1 | 5/3 |
| 22 | MSP → ORD | 11:15–12:17 | X6 | D95 | 1 | 5/3 |
| 24 | MSP → ORD | 12:15–13:17 | D | D9S | 2 | 5/4 |
| 53 | MSP → ORD | 13:15–14:17 | X7 | D95 | 2 | 5/4 |
| 30 | MSP → ORD | 14:15–15:17 | X6 | D9S | 1 | 5/4 |
| 728 | MSP → ORD | 15:15–16:17 | D | D9S | 2 | 5/4 |
| 32 | MSP → ORD | 16:15–17:17 | X6 | D9S | 2 | 5/4 |
| 36 | MSP → ORD | 17:15–18:17 | X6 | D95 | 2 | 5/4 |
| 34 | MSP → ORD | 18:15–19:17 | D | 72S | 2 | 5/4 |
| 60 | MSP → ORD | 19:15–20:17 | X6 | D9S | 2 | 5/4 |
| 42 | MSP → ORD | 20:15–21:14 | X6 | D9S | 2 | 5/4 |
| 78 | PDX → MSP | 07:20–12:30 | D | D9S | 2 | 6/1 |
| 75 | MSP → PDX | 09:30–11:05 | D | D9S | 2 | 5/4 |
| 71 | MSP → PDX | 17:40–19:15 | D | D9S | 2 | 5/4 |
| 76 | PDX → SEA | 11:35–12:10 | D | D9S | 2 | 6/1 |
| 101 | PHL → MSP | 07:00–08:50 | D | D9S | 2 | 6/1 |
| 263 | PHL → MSP | 18:20–20:10 | D | D9S | 2 | 6/1 |
| 562 | MSP → PHL | 13:20–16:35 | D | D9S | 2 | 5/4 |
| 478 | MSP → PHL | 18:30–21:45 | D | D9S | 2 | 5/4 |
| 120 | PHX → MSP | 08:35–12:27 | D | D9S | 2 | 6/1 |
| 102 | PHX → MSP | 08:35–12:27 | D | M80 | 2 | 6/1 |
| 132 | PHX → MSP | 13:40–17:32 | D | D9S | 3 | 6/1 |
| 104 | PHX → MSP | 13:40–17:32 | D | 72S | 3 | 6/1 |
| 106 | PHX → MSP | 16:10–20:02 | D | 72S | 2 | 6/1 |
| 101 | MSP → PHX | 09:30–11:42 | D | D9S | 2 | 5/4 |
| 103 | MSP → PHX | 09:30–11:42 | D | 72S | 2 | 5/4 |
| 105 | MSP → PHX | 12:20–14:32 | D | D9S | 2 | 5/4 |
| 107 | MSP → PHX | 17:35–19:47 | D | D9S | 2 | 5/4 |
| 505 | MSP → PHX | 17:35–19:47 | D | M80 | 2 | 5/4 |
| 116 | SAN → MSP | 07:15–12:30 | D | D9S | 2 | 6/1 |
| 69 | MSP → SAN | 08:55–10:45 | D | D9S | 2 | 5/4 |
| 64 | SEA → MSP | 07:20–12:25 | D | D9S | 2 | 6/1 |
| 76 | SEA → MSP | 12:40–17:45 | D | D9S | 3 | 6/1 |
| 80 | SEA → MSP | 14:55–20:00 | D | D9S | 2 | 6/1 |
| 81 | MSP → SEA | 12:20–13:50 | D | D9S | 2 | 5/4 |
| 79 | MSP → SEA | 17:35–19:05 | D | D9S | 2 | 5/4 |
| 300 | SFO → MSP | 07:10–12:40 | D | M80 | 2 | 6/1 |
| 704 | SFO → MSP | 12:20–17:40 | D | 72S | 3 | 6/1 |
| 342 | SFO → MSP | 14:50–20:20 | D | M80 | 2 | 6/1 |
| 341 | MSP → SFO | 09:35–11:20 | D | 72S | 2 | 5/4 |
| 345 | MSP → SFO | 12:15–14:11 | D | M80 | 2 | 5/4 |
| 347 | MSP → SFO | 17:25–19:10 | D | M80 | 2 | 5/4 |
| 376 | SLC → MSP | 09:15–12:25 | D | D9S | 2 | 6/2 |
| 678 | SLC → MSP | 14:35–17:45 | D | DC9 | 3 | 6/2 |
| 675 | MSP → SLC | 09:20–11:00 | D | DC9 | 2 | 5/4 |
| 355 | MSP → SLC | 17:40–19:20 | D | D9S | 2 | 5/4 |
| 75 | STL → MSP | 07:20–08:40 | D | D9S | 2 | 6/2 |
| 401 | STL → MSP | 10:10–11:30 | D | D9S | 2 | 6/2 |
| 567 | STL → MSP | 15:15–16:35 | D | DC9 | 3 | 6/2 |
| 400 | MSP → STL | 09:25–10:46 | D | D9S | 2 | 5/4 |
| 410 | MSP → STL | 13:20–14:41 | D | DC9 | 2 | 5/4 |
| 88 | MSP → STL | 20:50–22:11 | X6 | D9S | 2 | 5/4 |

## Regional prioritet och kvarstående arbete

Centralamerika/Karibien, samma **31 forskningsområden**, ligger kvar på **532 rörelser, 158 riktade par, 38 flygplatser och 64 inrikesrörelser**. **St. Thomas 14, Tortola 8 och Nicaragua 0** är oförändrade. Angränsande Mexiko har 20; Mellanösternurvalet har 72. Noll betyder en datalucka, inte frånvaro av historisk trafik.

**Ingen ny regional arkivsökning gjordes denna omgång.** Arbetet fokuserade på återstående rader i den redan tillgängliga Republic-originaltidtabellen. `regional_source_search_batch136.json` redovisar detta och hänvisar till föregående sökomgång.

Nicaragua och St. Thomas står kvar som prioritet. Tidigare spår efter Aero Virgin Islands 15 december 1985, Gull Air 15 februari 1986, LACSA, Caribbean Express, Eastern, American, BWIA, TACA, Aviateca, TAN SAHSA och sjöflygets tider/hamnar bevaras. Tidigare LIAT-, Challenge-, Arrow-, Airways International-, Air France F27-, PBA789- och specialflygsfrågor är oförändrade. Inga nya militär-, privat- eller specialflygrörelser importeras.

För Republic återstår bland annat mindre Detroit-/Minneapolis-linjer, separata SFO–SMF/SMF–SFO-ben, Express-operatörerna, ändringsblad och belägg för faktiskt genomförande. Inga extra genomgående MSP–SEA, PDX–MSP eller MEM–SMF-rörelser skapas.

## Totalt, verifiering och installation

**10 944 rörelser: 10 943 planerade och en tidigare bekräftad. 6 534 katalogscheman, varav 6 513 granskade. 393 flygplatser/platser, 121 länder/territorier och 1 967 riktade par. 111 operatörsposter: 68 med rörelser, 43 utan. 94 källposter och 36 bidragande tidtabellsutgåvor.** Ingen verifierad global bolagsinventering eller färdigprocent finns.

**20 befintliga Python-tester och tre generatorkontroller passerar.** Samtliga 1 379 Republic-rörelser har kontrollerats för dubbletter och samtidig överlappning av samma flygnummer; inga kvarstår efter den dokumenterade rättelsen. Alla äldre ID och tider bevaras. Jämförelsen mot v138 tillåter exakt ett äldre schema och tre rörelser med enbart rättat nummer och anmärkning. ZIP-filens CRC och varje fils hash kontrolleras mot manifestet vid paketeringen.

**Unreal Editor och spelet har inte körts här.** Paketet är kumulativt: stäng Unreal och slå samman `Plugins/` och `Tools/` med projektroten bredvid `.uproject`. **PaintAirplane-rättelsen f23b73a måste redan vara kompilerad.** Ingen ny C++-kod ingår.
