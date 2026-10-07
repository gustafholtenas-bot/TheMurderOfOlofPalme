# Republic kring Memphis – v136 / batch133

Granskat 2 oktober 2026. **219 nya planerade flygrörelser**, totalt **10 042**. Tillägget ger rörelser på **22 riktade sträckor, varav 12 är nya i databasen**. Samtliga är inrikes i USA och räknas utanför Centralamerika/Karibien-urvalet.

**110 nya granskade scheman på 23 riktade schemapar**: 108 ger rörelser och två sena helgscheman ger noll utfall. **Inga nya flygplatser, länder, operatörer eller källutgåvor**. Republic har sammanlagt **477 rörelser från 239 scheman** efter v124, v131 och v136. Nätgranskningen är fortsatt partiell.

Alla **9 823 äldre rörelseobjekt och 5 991 äldre scheman är oförändrade**. Flygplatser, länder och operatörer bevaras. Endast Republic-källpostens granskningsmetadata utökas; äldre forskningsrapporter och rättelser bevaras byte för byte.

## Nya rörelser

| Riktad sträcka | Nya rörelser | Ny sträcka i databasen |
|---|---:|---|
| ATL–MEM | 8 | Nej |
| BHM–MEM | 10 | Nej |
| BNA–MEM | 10 | Ja |
| BTR–MEM | 8 | Ja |
| CHA–MEM | 6 | Ja |
| DTW–MEM | 15 | Nej |
| HSV–MEM | 9 | Ja |
| LIT–MEM | 10 | Nej |
| MEM–ATL | 11 | Nej |
| MEM–BHM | 11 | Nej |
| MEM–BNA | 11 | Ja |
| MEM–BTR | 9 | Ja |
| MEM–CHA | 6 | Ja |
| MEM–DTW | 15 | Nej |
| MEM–HSV | 10 | Ja |
| MEM–LIT | 11 | Nej |
| MEM–MSP | 13 | Ja |
| MEM–SHV | 8 | Nej |
| MEM–TYS | 9 | Ja |
| MSP–MEM | 14 | Ja |
| SHV–MEM | 7 | Nej |
| TYS–MEM | 8 | Ja |

MEM = Memphis; ATL = Atlanta; BHM = Birmingham; BNA = Nashville; BTR = Baton Rouge; CHA = Chattanooga; DTW = Detroit; HSV = Huntsville; LIT = Little Rock; MSP = Minneapolis/St. Paul; SHV = Shreveport; TYS = Knoxville/McGhee Tyson. Befintliga historiskt granskade flygplatsmarkörer används oförändrade.

En rörelse är ett daterat fysiskt flygben. Samma linje kan ha flera avgångar och varje riktning räknas separat. **Knoxville–Chattanooga finns som granskat schema men ger ingen rörelse i fönstret**, vilket förklarar skillnaden mellan 23 schemapar och 22 par med nya rörelser.

## Originalkälla och urval

[Republic Airlines Employee System Timetable, 14 februari–1 mars 1986](https://northwestairlineshistory.org/wp-content/uploads/2018/07/REP-employee-schedule-1986-02-14.pdf), bevarad hos [Northwest Airlines History Center](https://northwestairlineshistory.org/timetables-republic/). Omslagets uttryckliga giltighet täcker hela importperioden. Samma sju sidors original som tidigare: denna omgång använder tabeller på skanning 2–6, legend på skanning 6 och befintliga flygplatsidentiteter. Originalets PDF och sidbilder dokumenteras med URL och SHA-256 i `caribbean_source_evidence_batch133.json`. Skanningarna distribueras inte.

**Tom ST-kolumn** betyder inga mellanstopp enligt legendens ST=Stops. Vi importerar endast sådana rader med Republics jetkoder. **EXP** betecknar Republic Express och legenden anger Express Airlines I för 1400–1699 samt Simmons Airlines för 1700–1899. Inga EXP-rader ingår i detta mainline-urval.

A/P betyder AM/PM, X betyder undantag och 1–7 betyder måndag–söndag. Tom FRQ tolkas som dagligen. Lokaltider följer tidtabellskonvention och kontrolleras mot tidszonerna; ingen separat uttrycklig all-times-local-text har återfunnits. Utrustningskoderna **DC9, D9S, D95 och 72S** bevaras exakt som tryckta. De identifierar inte ett visst individflygplan och utgör ingen bekräftelse på att avgången genomfördes.

De 23 utvalda riktningsrubrikerna innehåller **112 rader: 110 accepterade delben och två genomgående rader med ett stopp som utesluts**. Detta är ett avgränsat urval, inte en genomgång av hela bolagets trafik. Ändringsblad och faktisk drift är fortfarande ofullständigt undersökta.

## Genomgående resor och parallella nummer

| Resa som inte importeras som direktflyg | Tider | Dagar | Skäl |
|---|---|---|---|
| MEM–CHA 804 | 15:40–18:35 | 67 | Tryckt ST=1 |
| TYS–MEM 287 | 18:05–18:55 | 67 | Tryckt ST=1 |

För TYS–MEM 287 finns separata TYS–CHA och CHA–MEM-rader. Dessa två scheman katalogiseras med sina egna tider och dagar men ligger båda efter lördagens slutgräns. Ingen extra genomgående rörelse läggs ovanpå benen. MEM–CHA 804 används inte för att härleda ett obelagt delben med samma nummer.

**MEM–MSP 669** har tom ST och X6, liksom den separata dagliga raden **457** med samma tider 19:40–21:35. Båda har olika tryckta nummer och bevaras. Detsamma gäller övriga parallella avgångstider med olika nummer; samma klockslag i sig bevisar inte en dubblett. Vi gör inget påstående om vilka faktiska flygplan som användes.

Förstorade original kontrollerades särskilt för **72 MEM–DTW** (72 följt av D95, inte flyg 725), **450 MSP–MEM 06:00**, **234 MEM–TYS 12:30** och **835 TYS–MEM 18:30–18:40**. Dessa kontroller gjordes innan importen; inga äldre poster behövde rättas.

## Tidszoner och gränser

Projektfönstret är **27 februari 1986 kl. 22:21:30 UTC till 1 mars kl. 22:21:30 UTC**. Ett flyg ingår när dess intervall överlappar fönstret. **ATL, CHA, TYS och DTW har UTC−5**; de övriga åtta flygplatserna har **UTC−6**, verifierat för alla tre importdatum. Gränserna är därför 17:21:30 Eastern respektive 16:21:30 Central.

Nya rörelser efter lokal avgångsdag: **torsdag 49, fredag 106, lördag 64**.

- **Chattanooga–Memphis** kan ha ankomstklockslag före avgångsklockslaget eftersom flyget går från Eastern till Central Time. Exempel: 655 avgår 07:40 och anländer 07:38, men flygtiden är **58 minuter**. Det är inte en negativ flygtid eller ett dygnsskifte.
- **690 MEM–ATL**, 22:45–00:50, tar 65 minuter. **298 MEM–DTW**, 22:40–01:14, tar 94 minuter. **459 MEM–MSP**, 22:40–00:35, tar 115 minuter. Alla tre ankommer nästa lokala kalenderdag; sex daterade nattben importeras.
- Nio scheman överlappar båda fönstergränserna och ger **tre rörelser vardera**. Avgångsfiltrering utan hänsyn till ankomst skulle tappa torsdagsflyg som redan är i luften.
- **804 MEM–TYS** har två separata rader: 15:40–17:38 på helgen och 16:00–17:58 på vardagar. Helgraden ger lördagen; vardagsraden torsdag/fredag.
- **287 TYS–CHA 18:05–18:35 och CHA–MEM 19:00–18:55** går endast lördag/söndag och ger noll utfall, eftersom lördagens avgångar ligger efter fönstret. De behålls som granskade scheman.

Samtliga **110 flygtider**, även nollutfallen, har kontrollerats. De är **30–115 minuter**. Alla **219 UTC-intervall** matchar separat beräkning med fasta offseter, manuellt granskade flygtider och förväntade datum. Det är samma originalmaterial, inte en andra oberoende källa eller granskare.

## Följder av angränsande ben

**69 daterade följder med samma flygnummer** har kontrollerats. **58** knyter ihop nya poster med äldre importerade ben. De omfattar **36 klockmönster och 35 sträckmönster**, med **40–82 minuters markuppehåll i Memphis**. Två mönster för 804 beror på vardags-/helgtider. Kontroll av benföljd belägger inte individflygplan eller faktisk drift.

| Nummer | Benföljd | Nästa avgång från MEM | Markuppehåll, minuter |
|---|---|---|---:|
| 54 | BNA → MEM → MSY | 19:35 | 46 |
| 136 | MSP → MEM → BHM | 12:40 | 77 |
| 247 | TYS → MEM → VPS | 12:10 | 40 |
| 281 | DTW → MEM → SHV | 12:20 | 45 |
| 294 | BHM → MEM → DTW | 08:55 | 68 |
| 296 | MSY → MEM → DTW | 20:00 | 68 |
| 297 | DTW → MEM → MSY | 15:55 | 65 |
| 298 | HOU → MEM → DTW | 22:40 | 60 |
| 387 | HOU → MEM → BNA | 12:40 | 40 |
| 393 | MOB → MEM → BNA | 09:05 | 67 |
| 414 | MOB → MEM → ATL | 15:50 | 52 |
| 450 | MSP → MEM → MSY | 08:55 | 62 |
| 451 | MSY → MEM → MSP | 08:50 | 50 |
| 452 | MSP → MEM → MIA | 12:45 | 82 |
| 453 | MCO → MEM → MSP | 12:30 | 70 |
| 454 | MSP → MEM → MCO | 15:45 | 42 |
| 455 | FLL → MEM → MSP | 15:55 | 70 |
| 456 | MSP → MEM → MIA | 19:55 | 52 |
| 457 | MIA → MEM → MSP | 19:40 | 48 |
| 458 | MSP → MEM → MOB | 22:40 | 47 |
| 459 | MCO → MEM → MSP | 22:40 | 45 |
| 462 | MSP → MEM → TPA | 19:55 | 52 |
| 520 | IAH → MEM → CHA | 12:45 | 45 |
| 581 | BNA → MEM → MSY | 12:25 | 50 |
| 655 | CHA → MEM → MOB | 08:55 | 77 |
| 714 | MSY → MEM → DTW | 15:45 | 43 |
| 757 | DTW → MEM → HOU | 19:45 | 40 |
| 774 | MSY → MEM → DTW | 12:40 | 68 |
| 804 | LIT → MEM → TYS | 15:40 | 50 |
| 804 | LIT → MEM → TYS | 16:00 | 70 |
| 818 | GPT → MEM → ATL | 20:00 | 65 |
| 823 | ATL → MEM → LIT | 12:40 | 67 |
| 830 | HOU → MEM → TYS | 08:55 | 40 |
| 835 | TYS → MEM → VPS | 19:50 | 70 |
| 837 | HSV → MEM → LIT | 09:05 | 81 |
| 841 | CHA → MEM → MOB | 15:45 | 47 |

## Transkriberade scheman

Skanningar och kolumner räknas från ett, kolumner från vänster. Alla tider är lokala; +1 betyder ankomst nästa lokala dag. D = tom daganmärkning/dagligen; 6 = lördag; 67 = lördag/söndag; X6 = utom lördag; X7 = utom söndag; X67 = måndag–fredag. Rörelseantal gäller endast projektfönstret.

| Nummer | Sträcka | Lokala tider | Dagar | Utrustningskod | Rörelser | Skanning/kolumn |
|---|---|---|---|---|---:|---|
| 401 | ATL → MEM | 07:40–07:53 | D | D9S | 2 | 2/1 |
| 823 | ATL → MEM | 11:20–11:33 | D | DC9 | 2 | 2/1 |
| 803 | ATL → MEM | 14:50–15:03 | D | DC9 | 2 | 2/1 |
| 827 | ATL → MEM | 18:25–18:38 | D | DC9 | 2 | 2/1 |
| 670 | MEM → ATL | 08:50–10:55 | D | DC9 | 2 | 4/1 |
| 822 | MEM → ATL | 12:20–14:25 | D | DC9 | 2 | 4/1 |
| 414 | MEM → ATL | 15:50–17:55 | D | DC9 | 3 | 4/1 |
| 818 | MEM → ATL | 20:00–22:05 | D | DC9 | 2 | 4/1 |
| 690 | MEM → ATL | 22:45–00:50 +1 | X6 | DC9 | 2 | 4/1 |
| 294 | BHM → MEM | 07:00–07:47 | D | D9S | 2 | 2/1 |
| 403 | BHM → MEM | 10:30–11:17 | D | DC9 | 2 | 2/1 |
| 667 | BHM → MEM | 14:00–14:47 | D | D9S | 2 | 2/1 |
| 407 | BHM → MEM | 18:00–18:47 | D | DC9 | 2 | 2/1 |
| 481 | BHM → MEM | 21:00–21:47 | X6 | DC9 | 2 | 2/1 |
| 398 | MEM → BHM | 09:05–09:50 | X7 | DC9 | 2 | 4/1 |
| 136 | MEM → BHM | 12:40–13:25 | D | D9S | 2 | 4/1 |
| 277 | MEM → BHM | 16:05–16:50 | D | DC9 | 3 | 4/1 |
| 725 | MEM → BHM | 19:50–20:35 | D | DC9 | 2 | 4/1 |
| 185 | MEM → BHM | 22:35–23:20 | D | D9S | 2 | 4/1 |
| 555 | BNA → MEM | 07:00–07:49 | D | 72S | 2 | 2/1 |
| 581 | BNA → MEM | 10:45–11:35 | D | D95 | 2 | 2/1 |
| 57 | BNA → MEM | 14:05–14:54 | D | D95 | 2 | 2/1 |
| 54 | BNA → MEM | 18:00–18:49 | D | D95 | 2 | 2/1 |
| 441 | BNA → MEM | 20:55–21:44 | X6 | D95 | 2 | 2/1 |
| 393 | MEM → BNA | 09:05–09:50 | X7 | D9S | 2 | 4/1 |
| 387 | MEM → BNA | 12:40–13:25 | D | D9S | 2 | 4/1 |
| 708 | MEM → BNA | 15:55–16:40 | D | 72S | 3 | 4/1 |
| 52 | MEM → BNA | 19:45–20:30 | D | D95 | 2 | 4/1 |
| 558 | MEM → BNA | 22:35–23:20 | D | 72S | 2 | 4/1 |
| 471 | BTR → MEM | 07:00–07:58 | D | D9S | 2 | 2/2 |
| 829 | BTR → MEM | 10:30–11:25 | X7 | DC9 | 2 | 2/2 |
| 641 | BTR → MEM | 14:05–15:03 | D | DC9 | 2 | 2/2 |
| 486 | BTR → MEM | 17:40–18:38 | D | DC9 | 2 | 2/2 |
| 177 | MEM → BTR | 09:00–10:02 | X7 | DC9 | 2 | 4/1 |
| 184 | MEM → BTR | 12:35–13:37 | D | DC9 | 2 | 4/1 |
| 436 | MEM → BTR | 16:00–17:02 | D | DC9 | 3 | 4/1 |
| 408 | MEM → BTR | 19:45–20:47 | D | D9S | 2 | 4/1 |
| 655 | CHA → MEM | 07:40–07:38 | D | DC9 | 2 | 2/2 |
| 841 | CHA → MEM | 15:00–14:58 | X7 | DC9 | 2 | 2/2 |
| 287 | CHA → MEM | 18:20–18:18 | X67 | DC9 | 2 | 2/2 |
| 287 | CHA → MEM | 19:00–18:55 | 67 | DC9 | 0 | 2/2 |
| 520 | MEM → CHA | 12:45–14:35 | X7 | DC9 | 2 | 4/1 |
| 842 | MEM → CHA | 16:05–17:55 | X67 | DC9 | 2 | 4/1 |
| 805 | MEM → CHA | 19:50–21:40 | D | DC9 | 2 | 4/1 |
| 837 | HSV → MEM | 07:00–07:44 | D | D9S | 2 | 3/3 |
| 808 | HSV → MEM | 10:40–11:24 | X6 | DC9 | 1 | 3/3 |
| 812 | HSV → MEM | 14:10–14:54 | D | D9S | 2 | 3/3 |
| 557 | HSV → MEM | 17:55–18:39 | D | D9S | 2 | 3/3 |
| 267 | HSV → MEM | 21:00–21:44 | X6 | D9S | 2 | 3/3 |
| 811 | MEM → HSV | 09:05–09:48 | X67 | DC9 | 1 | 4/1 |
| 631 | MEM → HSV | 12:40–13:23 | D | DC9 | 2 | 4/1 |
| 664 | MEM → HSV | 16:05–16:48 | D | D9S | 3 | 4/1 |
| 556 | MEM → HSV | 19:50–20:30 | D | D9S | 2 | 4/2 |
| 226 | MEM → HSV | 22:35–23:18 | D | D9S | 2 | 4/2 |
| 630 | LIT → MEM | 07:00–07:40 | D | DC9 | 2 | 3/4 |
| 553 | LIT → MEM | 10:45–11:25 | D | D9S | 2 | 3/4 |
| 804 | LIT → MEM | 14:10–14:50 | D | DC9 | 2 | 3/4 |
| 240 | LIT → MEM | 17:55–18:35 | X7 | DC9 | 2 | 3/4 |
| 409 | LIT → MEM | 21:10–21:50 | X6 | D9S | 2 | 3/4 |
| 837 | MEM → LIT | 09:05–09:45 | X7 | D9S | 2 | 4/2 |
| 823 | MEM → LIT | 12:40–13:20 | D | DC9 | 2 | 4/2 |
| 633 | MEM → LIT | 16:05–16:45 | X7 | DC9 | 3 | 4/2 |
| 635 | MEM → LIT | 19:50–20:25 | D | D9S | 2 | 4/2 |
| 649 | MEM → LIT | 22:40–23:20 | D | DC9 | 2 | 4/2 |
| 188 | SHV → MEM | 07:00–07:54 | X7 | DC9 | 2 | 6/2 |
| 324 | SHV → MEM | 10:35–11:29 | D | DC9 | 2 | 6/2 |
| 691 | SHV → MEM | 13:50–14:44 | X6 | DC9 | 1 | 6/2 |
| 846 | SHV → MEM | 17:40–18:34 | D | DC9 | 2 | 6/2 |
| 843 | MEM → SHV | 09:05–10:07 | X7 | DC9 | 2 | 5/1 |
| 281 | MEM → SHV | 12:20–13:22 | X6 | DC9 | 1 | 5/1 |
| 848 | MEM → SHV | 16:00–17:02 | D | DC9 | 3 | 5/1 |
| 840 | MEM → SHV | 19:55–20:57 | D | DC9 | 2 | 5/1 |
| 293 | TYS → MEM | 07:40–07:50 | X7 | DC9 | 2 | 6/2 |
| 247 | TYS → MEM | 11:20–11:30 | D | DC9 | 2 | 6/2 |
| 825 | TYS → MEM | 14:55–15:05 | D | DC9 | 2 | 6/2 |
| 835 | TYS → MEM | 18:30–18:40 | X67 | DC9 | 2 | 6/2 |
| 830 | MEM → TYS | 08:55–10:53 | X7 | DC9 | 2 | 5/1 |
| 234 | MEM → TYS | 12:30–14:28 | D | DC9 | 2 | 5/1 |
| 804 | MEM → TYS | 15:40–17:38 | 67 | DC9 | 1 | 5/1 |
| 804 | MEM → TYS | 16:00–17:58 | X67 | DC9 | 2 | 5/1 |
| 258 | MEM → TYS | 19:40–21:38 | D | DC9 | 2 | 5/1 |
| 287 | TYS → CHA | 18:05–18:35 | 67 | DC9 | 0 | 6/2 |
| 697 | DTW → MEM | 07:00–07:50 | 6 | D9S | 1 | 3/1 |
| 705 | DTW → MEM | 07:00–07:50 | D | 72S | 2 | 3/1 |
| 281 | DTW → MEM | 10:40–11:35 | D | DC9 | 2 | 3/1 |
| 569 | DTW → MEM | 10:40–11:35 | D | 72S | 2 | 3/1 |
| 297 | DTW → MEM | 13:55–14:50 | D | D95 | 2 | 3/1 |
| 589 | DTW → MEM | 18:10–19:05 | D | D95 | 2 | 3/1 |
| 757 | DTW → MEM | 18:10–19:05 | X6 | D9S | 2 | 3/1 |
| 732 | DTW → MEM | 20:50–21:45 | D | D9S | 2 | 3/1 |
| 294 | MEM → DTW | 08:55–11:29 | D | D9S | 2 | 4/1 |
| 72 | MEM → DTW | 08:55–11:29 | X7 | D95 | 2 | 4/1 |
| 774 | MEM → DTW | 12:40–15:14 | D | D95 | 2 | 4/1 |
| 744 | MEM → DTW | 12:40–15:14 | D | D9S | 2 | 4/1 |
| 714 | MEM → DTW | 15:45–18:19 | D | D95 | 3 | 4/1 |
| 296 | MEM → DTW | 20:00–22:34 | D | D95 | 2 | 4/1 |
| 298 | MEM → DTW | 22:40–01:14 +1 | D | D9S | 2 | 4/1 |
| 450 | MSP → MEM | 06:00–07:53 | X7 | D95 | 2 | 5/3 |
| 136 | MSP → MEM | 09:30–11:23 | D | D9S | 2 | 5/3 |
| 452 | MSP → MEM | 09:30–11:23 | D | D95 | 2 | 5/3 |
| 454 | MSP → MEM | 13:10–15:03 | D | D95 | 2 | 5/3 |
| 462 | MSP → MEM | 17:10–19:03 | X6 | D9S | 2 | 5/3 |
| 456 | MSP → MEM | 17:10–19:03 | D | D95 | 2 | 5/3 |
| 458 | MSP → MEM | 20:00–21:53 | D | D9S | 2 | 5/3 |
| 451 | MEM → MSP | 08:50–10:45 | D | D95 | 2 | 4/2 |
| 453 | MEM → MSP | 12:30–14:25 | D | D95 | 2 | 4/2 |
| 455 | MEM → MSP | 15:55–17:50 | D | D95 | 3 | 4/2 |
| 669 | MEM → MSP | 19:40–21:35 | X6 | D9S | 2 | 4/2 |
| 457 | MEM → MSP | 19:40–21:35 | D | D95 | 2 | 4/2 |
| 459 | MEM → MSP | 22:40–00:35 +1 | X6 | D95 | 2 | 4/2 |

## Radräkning i valt urval

| Riktning | Accepterade scheman |
|---|---:|
| ATL–MEM | 4 |
| BHM–MEM | 5 |
| BNA–MEM | 5 |
| BTR–MEM | 4 |
| CHA–MEM | 4 |
| DTW–MEM | 8 |
| HSV–MEM | 5 |
| LIT–MEM | 5 |
| MEM–ATL | 5 |
| MEM–BHM | 5 |
| MEM–BNA | 5 |
| MEM–BTR | 4 |
| MEM–CHA | 3 |
| MEM–DTW | 7 |
| MEM–HSV | 5 |
| MEM–LIT | 5 |
| MEM–MSP | 6 |
| MEM–SHV | 4 |
| MEM–TYS | 5 |
| MSP–MEM | 7 |
| SHV–MEM | 4 |
| TYS–CHA | 1 |
| TYS–MEM | 4 |
| **Totalt** | **110** |

Därtill kommer de två ovan beskrivna ST=1-raderna: 112 granskade rader. Inga EXP-rader fanns bland de 23 utvalda riktningsrubrikerna. Resterande mainline-nät, separata Express-operatörer, ändringsblad och faktiskt genomförande återstår.

## Regional prioritet

Centralamerika/Karibien, samma **31 forskningsområden**, ligger kvar på **532 rörelser, 158 riktade par, 38 flygplatser och 64 inrikesrörelser**. **St. Thomas 14, Tortola 8 och Nicaragua 0** är oförändrade. Angränsande Mexiko har 20; Mellanösternurvalet har 72. Noll betyder datalucka, inte frånvaro av historisk trafik.

LACSA:s januari-/marsutgåvor, Eastern januari1986, Caribbean Express och BWIA söktes vidare. Träffarna gav index, omslag eller senare utgåvor, men inga nya godtagbara klockrader för målregionen och datumen. Den granskade Eastern-listningen visade endast en bild; inget köp eller kontakt med säljare gjordes. Ett TimetableWorld-spår gav ingen identifierbar datumgiltig1986-klocktabell. Granskad lokal Pan Am-OCR gav i detta fall kontorslistor och inga accepterade Nicaragua-/Costa Rica-/Panama-tider.

Se `regional_source_search_batch133.json` och `central_america_caribbean_queue_batch133.json`. Nicaragua och St. Thomas är fortsatt prioriterade, tillsammans med LACSA/Caribbean Express-inlagor, Eastern/American/BWIA, sjöflygets tider och hamnidentiteter samt TACA/Aviateca/TAN SAHSA. Alla tidigare LIAT-, Challenge-, Arrow-, Airways International-, Air France F27-, PBA789- och specialflygsfrågor bevaras. Inga nya militär-, privat- eller specialflygrörelser har importerats.

## Totalt, verifiering och installation

**10 042 rörelser: 10 041 planerade och en tidigare bekräftad. 6 101 katalogscheman, varav 6 080 granskade. 392 flygplatser/platser, 121 länder/territorier och 1 885 riktade par. 111 operatörsposter: 68 med rörelser, 43 utan. 94 källposter och 36 bidragande tidtabellsutgåvor.** Ingen verifierad global bolagsinventering eller färdigprocent finns.

**20 befintliga Python-tester och tre generatorkontroller passerar.** Samtliga 477 Republic-rörelser har kontrollerats för dubbletter och samtidig överlappning av samma flygnummer; inga hittades. Alla äldre rörelse- och schemaobjekt, flygplats-, land- och operatörsposter är oförändrade. Republic-källans granskningsnot och sidlista utökas; övriga källposter och all äldre forskning bevaras. ZIP-filens CRC och varje fils hash verifieras mot manifestet.

**Unreal Editor och spelet har inte körts här.** Paketet är kumulativt: stäng Unreal och slå samman `Plugins/` och `Tools/` med projektroten bredvid `.uproject`. **PaintAirplane-rättelsen f23b73a måste redan vara kompilerad.** Ingen ny C++-kod ingår.
