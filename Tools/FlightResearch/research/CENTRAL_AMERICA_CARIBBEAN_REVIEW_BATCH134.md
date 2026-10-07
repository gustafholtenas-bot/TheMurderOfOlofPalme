# Republic till/från Memphis – v137 / batch134

Granskat 2 oktober 2026. **291 nya planerade flygrörelser**, totalt **10 333**. Tillägget omfattar **40 riktade sträckor, varav 24 är nya i databasen**, och **135 nya granskade scheman**, samtliga med rörelser i projektfönstret. Alla tillägg är inrikes i USA och räknas utanför Centralamerika/Karibien-urvalet.

**Wichita Mid-Continent (ICT)** tillförs som flygplats. Inga nya länder, operatörer eller tidtabellsutgåvor. Republic har nu **768 rörelser från 374 scheman**. Hela nätet är ännu inte granskat.

Alla **10 042 äldre rörelseobjekt och 6 101 äldre scheman är oförändrade**. Alla äldre flygplats-, land- och operatörsposter bevaras. Republic-källpostens granskningsmetadata utökas; äldre forskningsrapporter och rättelser bevaras byte för byte.

## Nya rörelser

| Riktad sträcka | Nya rörelser | Ny sträcka i databasen |
|---|---:|---|
| CVG–MEM | 6 | Nej |
| DCA–MEM | 4 | Nej |
| DEN-STAPLETON–MEM | 4 | Ja |
| DFW–MEM | 8 | Nej |
| IAD–MEM | 8 | Ja |
| ICT–MEM | 6 | Ja |
| IND–MEM | 10 | Nej |
| LAS–MEM | 5 | Ja |
| LAX–MEM | 7 | Ja |
| LGA–MEM | 9 | Nej |
| MCI–MEM | 10 | Ja |
| MEM–CVG | 7 | Nej |
| MEM–DCA | 4 | Nej |
| MEM–DEN-STAPLETON | 5 | Ja |
| MEM–DFW | 9 | Nej |
| MEM–IAD | 9 | Ja |
| MEM–ICT | 7 | Ja |
| MEM–IND | 9 | Nej |
| MEM–LAS | 4 | Ja |
| MEM–LAX | 9 | Ja |
| MEM–LGA | 9 | Nej |
| MEM–MCI | 11 | Ja |
| MEM–MKE | 9 | Ja |
| MEM–OKC | 7 | Ja |
| MEM–ORD | 11 | Nej |
| MEM–PHL | 4 | Ja |
| MEM–PHX | 5 | Ja |
| MEM–SDF | 7 | Nej |
| MEM–SFO | 4 | Ja |
| MEM–STL | 11 | Nej |
| MEM–TUL | 9 | Ja |
| MKE–MEM | 10 | Ja |
| OKC–MEM | 6 | Ja |
| ORD–MEM | 10 | Nej |
| PHL–MEM | 4 | Ja |
| PHX–MEM | 5 | Ja |
| SDF–MEM | 6 | Nej |
| SFO–MEM | 5 | Ja |
| STL–MEM | 10 | Nej |
| TUL–MEM | 8 | Ja |

MEM = Memphis; CVG = Cincinnati/Northern Kentucky; DCA = Washington National; DEN-STAPLETON = Denver Stapleton; DFW = Dallas/Fort Worth; IAD = Washington Dulles; ICT = Wichita Mid-Continent; IND = Indianapolis; LAS = Las Vegas; LAX = Los Angeles; LGA = New York LaGuardia; MCI = Kansas City; MKE = Milwaukee; OKC = Oklahoma City; ORD = Chicago O’Hare; PHL = Philadelphia; PHX = Phoenix; SDF = Louisville; SFO = San Francisco; STL = St. Louis; TUL = Tulsa.

En rörelse är ett daterat fysiskt flygben. Flera avgångar på samma linje räknas var för sig, liksom de två riktningarna.

## Originalkälla och avgränsning

[Republic Airlines Employee System Timetable, 14 februari–1 mars 1986](https://northwestairlineshistory.org/wp-content/uploads/2018/07/REP-employee-schedule-1986-02-14.pdf), bevarad hos Northwest Airlines History Center. Omslagets uttryckliga giltighet täcker hela importperioden. Samma sju sidors original som i tidigare Republic-omgångar används: tabeller på skanning 2–6 och legend på skanning 6. Sidbilder och PDF dokumenteras med URL och SHA-256 i `caribbean_source_evidence_batch134.json`. **Originalskanningarna distribueras inte.**

De 40 utvalda riktningsrubrikerna innehåller **139 rader: 135 separata nonstopben och fyra genomgående resor med ett stopp**. Alla 135 rader med tom ST-kolumn katalogiseras. ST betyder Stops. A/P betyder AM/PM, X betyder undantag och 1–7 betyder måndag–söndag. Tom FRQ tolkas som dagligen. Utrustningskoderna DC9, D9S, D95 och 72S bevaras som tryckta.

Detta är ett avgränsat urval. Resterande nät, ändringsblad och faktisk drift återstår. EXP-rader ingår inte: källans legend skiljer Express Airlines I (1400–1699) från Simmons Airlines (1700–1899). Varje accepterat ben behåller sina egna tider, dagar och flygnummer.

| Genomgående rad som utesluts | Lokala tider | Dagar | ST |
|---|---|---|---:|
| 465 MEM–MKE | 12:30–15:05 | Dagligen | 1 |
| 393 MEM–ORD | 09:05–11:45 | Utom söndag | 1 |
| 387 MEM–ORD | 12:40–15:13 | Dagligen | 1 |
| 54 ORD–MEM | 16:15–18:49 | Dagligen | 1 |

Dessa resor läggs inte ovanpå de fysiska benen som extra direktflyg. **Separat tryckta LAS–SAN/SAN–LAS och SFO–SMF/SMF–SFO ligger kvar som möjliga fortsättningar**; motsvarande MEM–SAN och MEM–SMF-rader med ST=1 är inte nonstopbelägg.

## Flygplatser

**ICT** läggs in som **Wichita Mid-Continent (1986)** i USA, tidszon America/Chicago. [Flygplatsmyndighetens historik](https://www.flywichita.com/history-of-the-airport/) och [beskrivning av terminalprojektet](https://www.flywichita.com/construction-projects/) skiljer flygplatsen som öppnade för denna trafik 1954 från den äldre Municipal-/McConnell-platsen, och skiljer den äldre terminalen från den nya från 2015. Dagens Eisenhower-namn används inte som 1986-namn.

Markören **37.650314, −97.428583** kommer från [OurAirports KICT](https://ourairports.com/airports/KICT/), kontrollerad mot det tidigare hämtade koordinatunderlaget. Den markerar ungefär flygfältet, inte en inmätt terminal, bana eller referenspunkt från 1986. KICT används som nutida uppslagsnyckel, inte som separat bevis för dåtidens kodbruk. Se `airport_location_review_batch134.json`.

Källans **DEN** kopplas till den befintliga historiska posten **DEN-STAPLETON**, inte Denver International som öppnade senare. Övriga befintliga flygplatsmarkörer behålls oförändrade.

## Tider och fönstergränser

Projektfönstret är **27 februari 1986 kl. 22:21:30 UTC till 1 mars kl. 22:21:30 UTC**. Ett flyg tas med när dess intervall överlappar fönstret, även om avgången ligger före startgränsen.

| UTC-offset under de tre datumen | Flygplatser i urvalet |
|---|---|
| −5 timmar | CVG, DCA, IAD, IND, LGA, PHL, SDF |
| −6 timmar | MEM, DFW, ICT, MCI, MKE, OKC, ORD, STL, TUL |
| −7 timmar | DEN-STAPLETON, PHX |
| −8 timmar | LAS, LAX, SFO |

Offseterna är kontrollerade mot varje flygplats tidszon för samtliga tre datum. Lokaltider följer tidtabellskonvention och är förenliga med flygtiderna; ingen separat uttrycklig all-times-local-text har återfunnits i originalet.

**67 rörelser har lokal avgång torsdag, 135 fredag och 89 lördag.** Alla 135 scheman ger utfall; 21 överlappar båda fönstergränserna och ger tre daterade rörelser vardera. De övriga ger två.

- **732 MEM–DFW 22:45–00:13**, **689 MEM–MKE 22:40–00:10** och **481 MEM–ORD 22:35–00:09** ankommer nästa lokala dag. Deras flygtider är 88, 90 respektive 94 minuter. Sex nattben importeras.
- **481:s ankomst är tryckt 1209A**, vilket bekräftades i förstorad originalbild före import. Inga äldre data behövde rättas.
- **519 SDF–MEM 15:05–15:05** tar 60 minuter trots samma lokala klockslag, eftersom flyget går från Eastern till Central Time.
- **569 MEM–LAX 12:25–14:15** ankommer kl. 22:15 UTC, alltså 6 minuter och 30 sekunder före torsdagens startgräns. Det ger endast fredag och lördag.
- Västra tidszoner kan göra att en eftermiddagsavgång ger tre utfall: exempelvis **552 LAS–MEM 13:50–18:50** motsvarar tre timmar i luften och överlappar fönstret torsdag, fredag och lördag.

Samtliga **135 flygtider, 55–250 minuter**, och **291 UTC-intervall** har kontrollerats med separat manuellt granskade varaktigheter och förväntade datum. Beräkningen med fasta UTC-offseter matchar generatorns tidszonsberäkning. Det är samma originalmaterial, inte en andra oberoende källa eller granskare.

## Angränsande ben

**204 daterade följder med samma flygnummer**, fördelade på **104 klock-/sträckmönster**, har kontrollerats. **176** knyter ihop nya ben med äldre importerade ben. Markuppehållen i Memphis är **40–83 minuter**. Klockskillnaderna har granskats manuellt. Detta visar tidsmässig förenlighet; det identifierar inte flygplan eller bekräftar faktisk drift.

| Nummer | Benföljd | Ankomst–nästa avgång i MEM | Markuppehåll, minuter |
|---|---|---|---:|
| 52 | ORD → MEM → BNA | 19:05–19:45 | 40 |
| 57 | BNA → MEM → ORD | 14:54–15:35 | 41 |
| 141 | SRQ → MEM → IND | 11:13–12:25 | 72 |
| 148 | IND → MEM → MOB | 11:27–12:35 | 68 |
| 161 | CVG → MEM → IAH | 08:00–08:50 | 50 |
| 184 | TUL → MEM → BTR | 11:40–12:35 | 55 |
| 185 | IND → MEM → BHM | 21:22–22:35 | 73 |
| 188 | SHV → MEM → IND | 07:54–08:45 | 51 |
| 226 | MCI → MEM → HSV | 21:40–22:35 | 55 |
| 240 | LIT → MEM → DCA | 18:35–19:25 | 50 |
| 258 | DFW → MEM → TYS | 18:53–19:40 | 47 |
| 259 | IND → MEM → VPS | 15:10–16:00 | 50 |
| 261 | CVG → MEM → IAH | 18:45–19:35 | 50 |
| 265 | TPA → MEM → OKC | 11:20–12:05 | 45 |
| 267 | HSV → MEM → MCI | 21:44–22:45 | 61 |
| 288 | DFW → MEM → SDF | 11:53–12:35 | 42 |
| 293 | TYS → MEM → DFW | 07:50–08:45 | 55 |
| 299 | MCI → MEM → MGM | 15:00–16:00 | 60 |
| 382 | MKE → MEM → MCO | 11:32–12:35 | 63 |
| 383 | MCO → MEM → MCI | 14:45–15:55 | 70 |
| 384 | MCI → MEM → MCO | 08:05–09:10 | 65 |
| 385 | MCO → MEM → MKE | 18:45–19:25 | 40 |
| 396 | MGM → MEM → SDF | 14:48–15:55 | 67 |
| 398 | TUL → MEM → BHM | 08:00–09:05 | 65 |
| 401 | ATL → MEM → STL | 07:53–08:45 | 52 |
| 403 | BHM → MEM → STL | 11:17–12:30 | 73 |
| 404 | MCI → MEM → PHL | 11:40–12:45 | 65 |
| 405 | TPA → MEM → STL | 14:50–15:50 | 60 |
| 407 | BHM → MEM → IAD | 18:47–19:30 | 43 |
| 408 | IAD → MEM → BTR | 18:57–19:45 | 48 |
| 409 | LIT → MEM → STL | 21:50–22:45 | 55 |
| 436 | STL → MEM → BTR | 14:58–16:00 | 62 |
| 448 | TPA → MEM → CVG | 07:50–08:45 | 55 |
| 465 | MIA → MEM → ORD | 11:22–12:30 | 68 |
| 470 | ORD → MEM → HOU | 07:53–08:45 | 52 |
| 471 | BTR → MEM → ORD | 07:58–08:50 | 52 |
| 474 | ORD → MEM → HOU | 14:38–15:40 | 62 |
| 476 | STL → MEM → TPA | 07:58–09:00 | 62 |
| 481 | BHM → MEM → ORD | 21:47–22:35 | 48 |
| 484 | SDF → MEM → MGM | 07:47–09:10 | 83 |
| 486 | BTR → MEM → STL | 18:38–19:35 | 57 |
| 519 | SDF → MEM → IAH | 15:05–15:45 | 40 |
| 521 | IND → MEM → SRQ | 18:47–19:55 | 68 |
| 523 | MKE → MEM → MCO | 19:07–19:50 | 43 |
| 534 | MKE → MEM → MSY | 21:47–22:30 | 43 |
| 552 | LAS → MEM → IND | 18:50–19:30 | 40 |
| 553 | LIT → MEM → LAS | 11:25–12:05 | 40 |
| 555 | BNA → MEM → LAX | 07:49–09:00 | 71 |
| 556 | LAX → MEM → HSV | 18:45–19:50 | 65 |
| 557 | HSV → MEM → LAX | 18:39–19:55 | 76 |
| 558 | LAX → MEM → BNA | 21:40–22:35 | 55 |
| 569 | DTW → MEM → LAX | 11:35–12:25 | 50 |
| 630 | LIT → MEM → LGA | 07:40–09:00 | 80 |
| 631 | LGA → MEM → HSV | 11:30–12:40 | 70 |
| 633 | LGA → MEM → LIT | 14:55–16:05 | 70 |
| 634 | TUL → MEM → LGA | 14:55–15:50 | 55 |
| 635 | LGA → MEM → LIT | 18:50–19:50 | 60 |
| 637 | LGA → MEM → HOU | 21:29–22:45 | 76 |
| 641 | BTR → MEM → TUL | 15:03–16:05 | 62 |
| 664 | PHX → MEM → HSV | 15:15–16:05 | 50 |
| 665 | IND → MEM → PHX | 07:52–08:45 | 53 |
| 667 | BHM → MEM → PHX | 14:47–15:40 | 53 |
| 668 | PHX → MEM → LGA | 18:50–19:30 | 40 |
| 670 | ICT → MEM → ATL | 08:10–08:50 | 40 |
| 680 | MKE → MEM → MIA | 07:52–09:00 | 68 |
| 684 | DEN-STAPLETON → MEM → CVG | 14:37–15:50 | 73 |
| 685 | DCA → MEM → DEN-STAPLETON | 08:15–09:10 | 55 |
| 687 | MIA → MEM → MKE | 14:52–15:35 | 43 |
| 688 | MKE → MEM → MIA | 15:12–15:55 | 43 |
| 689 | MIA → MEM → MKE | 21:57–22:40 | 43 |
| 690 | DEN-STAPLETON → MEM → ATL | 21:57–22:45 | 48 |
| 691 | SHV → MEM → DEN-STAPLETON | 14:44–15:40 | 56 |
| 705 | DTW → MEM → SFO | 07:50–08:30 | 40 |
| 708 | LAX → MEM → BNA | 14:50–15:55 | 65 |
| 712 | STL → MEM → MCO | 21:43–22:40 | 57 |
| 725 | STL → MEM → BHM | 18:55–19:50 | 55 |
| 732 | DTW → MEM → DFW | 21:45–22:45 | 60 |
| 801 | GPT → MEM → TUL | 08:00–09:00 | 60 |
| 803 | ATL → MEM → ICT | 15:03–15:50 | 47 |
| 805 | OKC → MEM → CHA | 18:53–19:50 | 57 |
| 806 | VPS → MEM → ICT | 18:50–19:35 | 45 |
| 808 | HSV → MEM → MCI | 11:24–12:15 | 51 |
| 809 | PHL → MEM → TUL | 11:20–12:15 | 55 |
| 810 | IAH → MEM → IAD | 08:15–09:05 | 50 |
| 811 | IAD → MEM → HSV | 08:12–09:05 | 53 |
| 812 | HSV → MEM → IAD | 14:54–16:00 | 66 |
| 814 | VPS → MEM → IND | 15:00–15:50 | 50 |
| 815 | MGM → MEM → MCI | 07:58–08:50 | 52 |
| 819 | IAD → MEM → DFW | 14:52–15:40 | 48 |
| 822 | CVG → MEM → ATL | 11:35–12:20 | 45 |
| 825 | TYS → MEM → OKC | 15:05–15:50 | 45 |
| 826 | TUL → MEM → PHL | 18:45–19:30 | 45 |
| 827 | ATL → MEM → TUL | 18:38–19:40 | 62 |
| 829 | BTR → MEM → ICT | 11:25–12:05 | 40 |
| 831 | PHL → MEM → MCI | 18:55–19:45 | 50 |
| 838 | DFW → MEM → DCA | 08:13–09:15 | 62 |
| 839 | DCA → MEM → DFW | 19:05–20:00 | 55 |
| 840 | ICT → MEM → SHV | 18:55–19:55 | 60 |
| 842 | ICT → MEM → CHA | 15:05–16:05 | 60 |
| 843 | OKC → MEM → SHV | 08:08–09:05 | 57 |
| 844 | IAH → MEM → CVG | 18:55–19:35 | 40 |
| 846 | SHV → MEM → SDF | 18:34–19:40 | 66 |
| 847 | SDF → MEM → MOB | 18:37–19:35 | 58 |
| 848 | OKC → MEM → SHV | 15:03–16:00 | 57 |

## Transkriberade scheman

Skanningar och kolumner räknas från ett, kolumner från vänster. Lokala tider; +1 betyder ankomst nästa lokala dag. D = tom daganmärkning/dagligen; X6 = utom lördag; X7 = utom söndag. Rörelseantal gäller projektfönstret. DEN-STAPLETON återger tryckt DEN med projektets historiska flygplats-ID.

| Nummer | Sträcka | Lokala tider | Dagar | Utrustningskod | Rörelser | Skanning/kolumn |
|---|---|---|---|---|---:|---|
| 161 | CVG → MEM | 07:45–08:00 | X7 | DC9 | 2 | 2/2 |
| 822 | CVG → MEM | 11:20–11:35 | D | DC9 | 2 | 2/2 |
| 261 | CVG → MEM | 18:30–18:45 | D | DC9 | 2 | 2/2 |
| 448 | MEM → CVG | 08:45–10:55 | X7 | DC9 | 2 | 4/1 |
| 684 | MEM → CVG | 15:50–18:00 | D | DC9 | 3 | 4/1 |
| 844 | MEM → CVG | 19:35–21:45 | D | DC9 | 2 | 4/1 |
| 685 | DCA → MEM | 07:00–08:15 | D | DC9 | 2 | 2/2 |
| 839 | DCA → MEM | 17:50–19:05 | D | D9S | 2 | 2/2 |
| 838 | MEM → DCA | 09:15–12:03 | D | D9S | 2 | 4/1 |
| 240 | MEM → DCA | 19:25–21:59 | D | DC9 | 2 | 4/1 |
| 684 | DEN-STAPLETON → MEM | 11:35–14:37 | D | DC9 | 2 | 2/3 |
| 690 | DEN-STAPLETON → MEM | 18:55–21:57 | D | DC9 | 2 | 2/3 |
| 685 | MEM → DEN-STAPLETON | 09:10–10:40 | D | DC9 | 2 | 4/1 |
| 691 | MEM → DEN-STAPLETON | 15:40–17:10 | D | DC9 | 3 | 4/1 |
| 838 | DFW → MEM | 07:00–08:13 | X7 | D9S | 2 | 2/3 |
| 288 | DFW → MEM | 10:40–11:53 | D | DC9 | 2 | 2/3 |
| 258 | DFW → MEM | 17:40–18:53 | D | DC9 | 2 | 2/3 |
| 254 | DFW → MEM | 20:30–21:43 | D | DC9 | 2 | 2/3 |
| 293 | MEM → DFW | 08:45–10:13 | D | DC9 | 2 | 4/1 |
| 819 | MEM → DFW | 15:40–17:08 | D | DC9 | 3 | 4/1 |
| 839 | MEM → DFW | 20:00–21:28 | D | D9S | 2 | 4/1 |
| 732 | MEM → DFW | 22:45–00:13 +1 | X6 | D9S | 2 | 4/1 |
| 811 | IAD → MEM | 07:00–08:12 | X7 | DC9 | 2 | 3/3 |
| 529 | IAD → MEM | 10:15–11:27 | D | D9S | 2 | 3/3 |
| 819 | IAD → MEM | 13:40–14:52 | D | DC9 | 2 | 3/3 |
| 408 | IAD → MEM | 17:45–18:57 | D | D9S | 2 | 3/3 |
| 810 | MEM → IAD | 09:05–11:47 | X7 | DC9 | 2 | 4/2 |
| 244 | MEM → IAD | 12:30–15:10 | D | D9S | 2 | 4/2 |
| 812 | MEM → IAD | 16:00–18:42 | D | D9S | 3 | 4/2 |
| 407 | MEM → IAD | 19:30–22:12 | D | DC9 | 2 | 4/2 |
| 670 | ICT → MEM | 07:00–08:10 | D | DC9 | 2 | 3/3 |
| 842 | ICT → MEM | 13:55–15:05 | D | DC9 | 2 | 3/3 |
| 840 | ICT → MEM | 17:45–18:55 | D | DC9 | 2 | 3/3 |
| 829 | MEM → ICT | 12:05–13:30 | D | DC9 | 2 | 4/2 |
| 803 | MEM → ICT | 15:50–17:15 | D | DC9 | 3 | 4/2 |
| 806 | MEM → ICT | 19:35–21:00 | D | DC9 | 2 | 4/2 |
| 665 | IND → MEM | 07:40–07:52 | D | D9S | 2 | 3/3 |
| 148 | IND → MEM | 11:15–11:27 | D | DC9 | 2 | 3/3 |
| 259 | IND → MEM | 15:00–15:10 | D | DC9 | 2 | 3/3 |
| 521 | IND → MEM | 18:35–18:47 | D | DC9 | 2 | 3/3 |
| 185 | IND → MEM | 21:10–21:22 | D | D9S | 2 | 3/3 |
| 188 | MEM → IND | 08:45–10:55 | D | DC9 | 2 | 4/2 |
| 141 | MEM → IND | 12:25–14:35 | D | DC9 | 2 | 4/2 |
| 814 | MEM → IND | 15:50–18:00 | D | DC9 | 3 | 4/2 |
| 552 | MEM → IND | 19:30–21:40 | D | D9S | 2 | 4/2 |
| 552 | LAS → MEM | 13:50–18:50 | D | D9S | 3 | 3/4 |
| 550 | LAS → MEM | 17:00–22:00 | D | D9S | 2 | 3/4 |
| 551 | MEM → LAS | 08:55–10:30 | D | D9S | 2 | 4/2 |
| 553 | MEM → LAS | 12:05–13:40 | D | D9S | 2 | 4/2 |
| 708 | LAX → MEM | 09:30–14:50 | D | 72S | 2 | 3/4 |
| 556 | LAX → MEM | 13:25–18:45 | D | 72S | 3 | 3/4 |
| 558 | LAX → MEM | 16:20–21:40 | D | 72S | 2 | 3/4 |
| 555 | MEM → LAX | 09:00–10:50 | D | 72S | 2 | 4/2 |
| 569 | MEM → LAX | 12:25–14:15 | D | 72S | 2 | 4/2 |
| 573 | MEM → LAX | 15:40–17:30 | D | 72S | 3 | 4/2 |
| 557 | MEM → LAX | 19:55–21:45 | D | 72S | 2 | 4/2 |
| 631 | LGA → MEM | 09:30–11:30 | D | DC9 | 2 | 3/4 |
| 633 | LGA → MEM | 12:55–14:55 | D | DC9 | 2 | 3/4 |
| 635 | LGA → MEM | 16:50–18:50 | D | D9S | 3 | 3/4 |
| 637 | LGA → MEM | 19:29–21:29 | D | DC9 | 2 | 3/4 |
| 630 | MEM → LGA | 09:00–12:16 | D | DC9 | 2 | 4/2 |
| 632 | MEM → LGA | 12:30–15:40 | D | D95 | 2 | 4/2 |
| 634 | MEM → LGA | 15:50–19:00 | D | DC9 | 3 | 4/2 |
| 668 | MEM → LGA | 19:30–22:29 | D | D9S | 2 | 4/2 |
| 384 | MCI → MEM | 07:00–08:05 | D | D9S | 2 | 3/4 |
| 404 | MCI → MEM | 10:35–11:40 | D | DC9 | 2 | 3/4 |
| 299 | MCI → MEM | 13:55–15:00 | D | DC9 | 2 | 3/4 |
| 186 | MCI → MEM | 17:40–18:45 | D | D9S | 2 | 3/4 |
| 226 | MCI → MEM | 20:35–21:40 | D | D9S | 2 | 3/4 |
| 815 | MEM → MCI | 08:50–10:05 | D | DC9 | 2 | 4/2 |
| 808 | MEM → MCI | 12:15–13:30 | D | DC9 | 2 | 4/2 |
| 383 | MEM → MCI | 15:55–17:10 | D | D9S | 3 | 4/2 |
| 831 | MEM → MCI | 19:45–21:00 | D | DC9 | 2 | 4/2 |
| 267 | MEM → MCI | 22:45–23:59 | D | D9S | 2 | 4/2 |
| 680 | MKE → MEM | 06:15–07:52 | D | D95 | 2 | 5/1 |
| 382 | MKE → MEM | 09:55–11:32 | D | D95 | 2 | 5/1 |
| 688 | MKE → MEM | 13:35–15:12 | D | D95 | 2 | 5/1 |
| 523 | MKE → MEM | 17:30–19:07 | D | D95 | 2 | 5/1 |
| 534 | MKE → MEM | 20:10–21:47 | D | D95 | 2 | 5/1 |
| 256 | MEM → MKE | 08:50–10:20 | D | D95 | 2 | 4/2 |
| 687 | MEM → MKE | 15:35–17:05 | D | D95 | 3 | 4/2 |
| 385 | MEM → MKE | 19:25–20:55 | D | D95 | 2 | 4/2 |
| 689 | MEM → MKE | 22:40–00:10 +1 | D | D9S | 2 | 4/2 |
| 843 | OKC → MEM | 07:00–08:08 | D | DC9 | 2 | 5/4 |
| 848 | OKC → MEM | 13:55–15:03 | D | DC9 | 2 | 5/4 |
| 805 | OKC → MEM | 17:45–18:53 | D | DC9 | 2 | 5/4 |
| 265 | MEM → OKC | 12:05–13:30 | D | DC9 | 2 | 4/2 |
| 825 | MEM → OKC | 15:50–17:15 | D | DC9 | 3 | 4/2 |
| 855 | MEM → OKC | 20:00–21:25 | D | DC9 | 2 | 4/2 |
| 470 | ORD → MEM | 06:25–07:53 | X7 | D9S | 2 | 5/4 |
| 472 | ORD → MEM | 09:55–11:23 | D | D9S | 2 | 5/4 |
| 474 | ORD → MEM | 13:10–14:38 | D | D9S | 2 | 5/4 |
| 52 | ORD → MEM | 17:40–19:05 | D | D95 | 2 | 5/4 |
| 729 | ORD → MEM | 20:05–21:33 | D | DC9 | 2 | 5/4 |
| 471 | MEM → ORD | 08:50–10:24 | D | D9S | 2 | 4/2 |
| 465 | MEM → ORD | 12:30–14:04 | D | D9S | 2 | 4/2 |
| 57 | MEM → ORD | 15:35–17:09 | D | D95 | 3 | 5/1 |
| 479 | MEM → ORD | 19:45–21:14 | D | D95 | 2 | 5/1 |
| 481 | MEM → ORD | 22:35–00:09 +1 | D | DC9 | 2 | 5/1 |
| 809 | PHL → MEM | 09:55–11:20 | D | DC9 | 2 | 6/1 |
| 831 | PHL → MEM | 17:30–18:55 | D | DC9 | 2 | 6/1 |
| 404 | MEM → PHL | 12:45–15:45 | D | DC9 | 2 | 5/1 |
| 826 | MEM → PHL | 19:30–22:30 | D | DC9 | 2 | 5/1 |
| 664 | PHX → MEM | 11:30–15:15 | D | D9S | 2 | 6/1 |
| 668 | PHX → MEM | 15:05–18:50 | D | D9S | 3 | 6/1 |
| 665 | MEM → PHX | 08:45–11:05 | D | D9S | 2 | 5/1 |
| 667 | MEM → PHX | 15:40–18:00 | D | D9S | 3 | 5/1 |
| 484 | SDF → MEM | 07:45–07:47 | D | DC9 | 2 | 6/1 |
| 519 | SDF → MEM | 15:05–15:05 | D | DC9 | 2 | 6/1 |
| 847 | SDF → MEM | 18:35–18:37 | D | DC9 | 2 | 6/1 |
| 288 | MEM → SDF | 12:35–14:40 | D | DC9 | 2 | 5/1 |
| 396 | MEM → SDF | 15:55–18:00 | D | DC9 | 3 | 5/1 |
| 846 | MEM → SDF | 19:40–21:45 | D | DC9 | 2 | 5/1 |
| 660 | SFO → MEM | 09:15–14:45 | D | 72S | 2 | 6/1 |
| 706 | SFO → MEM | 13:25–18:55 | D | 72S | 3 | 6/1 |
| 705 | MEM → SFO | 08:30–10:40 | D | 72S | 2 | 5/1 |
| 663 | MEM → SFO | 19:45–21:55 | D | 72S | 2 | 5/1 |
| 476 | STL → MEM | 07:00–07:58 | D | D9S | 2 | 6/2 |
| 151 | STL → MEM | 10:35–11:33 | D | D95 | 2 | 6/2 |
| 436 | STL → MEM | 14:00–14:58 | D | DC9 | 2 | 6/2 |
| 725 | STL → MEM | 18:00–18:55 | D | D9S | 2 | 6/2 |
| 712 | STL → MEM | 20:45–21:43 | X6 | DC9 | 2 | 6/2 |
| 401 | MEM → STL | 08:45–09:40 | D | D9S | 2 | 5/1 |
| 403 | MEM → STL | 12:30–13:28 | D | DC9 | 2 | 5/1 |
| 405 | MEM → STL | 15:50–16:48 | D | D9S | 3 | 5/1 |
| 486 | MEM → STL | 19:35–20:33 | D | DC9 | 2 | 5/1 |
| 409 | MEM → STL | 22:45–23:43 | D | DC9 | 2 | 5/1 |
| 398 | TUL → MEM | 07:00–08:00 | X7 | DC9 | 2 | 6/2 |
| 184 | TUL → MEM | 10:40–11:40 | X7 | DC9 | 2 | 6/2 |
| 634 | TUL → MEM | 13:55–14:55 | X7 | DC9 | 2 | 6/2 |
| 826 | TUL → MEM | 17:45–18:45 | D | DC9 | 2 | 6/2 |
| 801 | MEM → TUL | 09:00–10:12 | X7 | DC9 | 2 | 5/1 |
| 809 | MEM → TUL | 12:15–13:27 | X7 | DC9 | 2 | 5/1 |
| 641 | MEM → TUL | 16:05–17:17 | D | DC9 | 3 | 5/1 |
| 827 | MEM → TUL | 19:40–20:52 | X6 | DC9 | 2 | 5/1 |

## Regional prioritet och kvarstående arbete

Centralamerika/Karibien, samma **31 forskningsområden**, ligger kvar på **532 rörelser, 158 riktade par, 38 flygplatser och 64 inrikesrörelser**. **St. Thomas 14, Tortola 8 och Nicaragua 0** är oförändrade. Angränsande Mexiko har 20; Mellanösternurvalet har 72. Noll betyder en datalucka, inte frånvaro av historisk trafik.

AirTimes utökade PDF-arkiv undersöktes. [Aero Virgin Islands-indexet](https://airtimes.com/cgat/usa/aerovirginislands.htm) listar 15 december 1985, men ingen PDF-länk till just den utgåvan fanns bland de granskade länkarna. De tillgängliga PDF-utgåvorna från tidigare år och 1987 används inte för att fylla februari 1986. Flamenco-länkarna avsåg 1980. Chalk’s gav inget användbart tidtabellsinnehåll för målperioden.

[Gull Air-spåret](https://www.sunshineskies.com/gull.html) gav en linjekarta från februari 1986 och ett tidtabellsexempel från juli 1984. Varken detta eller de kontrollerade Gull Air-indexen gav en komplett tidtabell med godtagbara nya tider för februari 1986. Inga köp eller kontakter gjordes. Se `regional_source_search_batch134.json`.

Nicaragua och St. Thomas är fortsatt prioriterade. Aero Virgin Islands 15 december 1985 och Gull Air 15 februari 1986 är konkreta inlagor att söka vidare efter, tillsammans med tidigare LACSA-, Caribbean Express-, Eastern-, American-, BWIA-, TACA-, Aviateca-, TAN SAHSA- och sjöflygsspår. Tidigare LIAT-, Challenge-, Arrow-, Airways International-, Air France F27-, PBA789- och specialflygsfrågor bevaras. Inga nya militär-, privat- eller specialflygrörelser importeras i denna omgång.

## Totalt, verifiering och installation

**10 333 rörelser: 10 332 planerade och en tidigare bekräftad. 6 236 katalogscheman, varav 6 215 granskade. 393 flygplatser/platser, 121 länder/territorier och 1 909 riktade par. 111 operatörsposter: 68 med rörelser, 43 utan. 94 källposter och 36 bidragande tidtabellsutgåvor.** Ingen verifierad global bolagsinventering eller färdigprocent finns.

**20 befintliga Python-tester och tre generatorkontroller passerar.** Samtliga 768 Republic-rörelser har kontrollerats för dubbletter och samtidig överlappning av samma flygnummer; inga hittades. Tidigare rörelse- och schemaobjekt bevaras. ZIP-filens CRC och varje fils hash verifieras mot manifestet.

**Unreal Editor och spelet har inte körts här.** Paketet är kumulativt: stäng Unreal och slå samman `Plugins/` och `Tools/` med projektroten bredvid `.uproject`. **PaintAirplane-rättelsen f23b73a måste redan vara kompilerad.** Ingen ny C++-kod ingår.
