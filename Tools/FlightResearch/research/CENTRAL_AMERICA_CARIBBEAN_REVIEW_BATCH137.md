# Republics mindre Minneapolis-/Detroit-linjer – v140 / batch137

Granskat 2 oktober 2026. **211 nya planerade flygrörelser**, totalt **11 155**. Tillägget omfattar **32 riktade sträckor, varav 22 är nya i databasen**, och **103 nya granskade scheman**. 102 ger rörelser i projektfönstret; ett lördagsschema ger noll.

**Alla 10 944 äldre rörelseobjekt och 6 534 äldre scheman är oförändrade.** Rättelsen av äldre LAX–DTW 12:45 till flygnummer **334** från v139 är bevarad, inklusive stabila äldre ID och dokumentation i `errata_batch136.json`. V140 tillför inga ytterligare ändringar av äldre rörelser. Äldre flygplats-, land- och operatörsposter bevaras; Republic-källpostens granskningsmetadata utökas. Tidigare forskningsrapporter och rättelser bevaras byte för byte.

Alla tillägg är inrikes i USA, utanför Centralamerika/Karibien-urvalet. **Inga nya flygplatser, länder, operatörer eller tidtabellsutgåvor**. Republic har nu **1 590 rörelser från 775 scheman**. Nätet är fortfarande partiellt.

## Nya rörelser

| Riktad sträcka | Nya rörelser | Ny sträcka i databasen |
|---|---:|---|
| ATW–DTW | 4 | Ja |
| ATW–MSP | 4 | Ja |
| CID–MSP | 4 | Nej |
| CMH–MSP | 5 | Ja |
| CVG–MSP | 3 | Ja |
| CWA–DTW | 4 | Ja |
| CWA–MSP | 9 | Ja |
| DTW–ATW | 5 | Ja |
| DTW–CWA | 5 | Ja |
| DTW–GRB | 9 | Ja |
| DTW–GRR | 13 | Nej |
| DTW–IND | 11 | Nej |
| DTW–MSN | 9 | Ja |
| GRB–DTW | 8 | Ja |
| GRB–MSP | 9 | Ja |
| GRR–DTW | 12 | Nej |
| GRR–MSP | 7 | Nej |
| IND–DTW | 8 | Nej |
| IND–MSP | 7 | Ja |
| LSE–MSP | 5 | Ja |
| MSN–DTW | 8 | Ja |
| MSN–MSP | 7 | Nej |
| MSP–ATW | 2 | Ja |
| MSP–CID | 4 | Nej |
| MSP–CMH | 4 | Ja |
| MSP–CVG | 2 | Ja |
| MSP–CWA | 8 | Ja |
| MSP–GRB | 8 | Ja |
| MSP–GRR | 6 | Nej |
| MSP–IND | 6 | Ja |
| MSP–LSE | 7 | Ja |
| MSP–MSN | 8 | Nej |

ATW = Appleton/Outagamie County; CID = Cedar Rapids; CMH = Columbus/Port Columbus; CVG = Greater Cincinnati; CWA = Central Wisconsin; DTW = Detroit; GRB = Green Bay/Austin Straubel; GRR = Grand Rapids/Kent County; IND = Indianapolis; LSE = La Crosse; MSN = Madison/Dane County; MSP = Minneapolis/St. Paul. Befintliga historiskt granskade flygplatsmarkörer används oförändrade.

En rörelse är ett daterat fysiskt flygben. Flera avgångar på samma linje och de två riktningarna räknas var för sig.

## Källa och avgränsning

[Republic Airlines Employee System Timetable, 14 februari–1 mars 1986](https://northwestairlineshistory.org/wp-content/uploads/2018/07/REP-employee-schedule-1986-02-14.pdf), bevarad hos Northwest Airlines History Center. Omslagets uttryckliga giltighet täcker importperioden. Den tidigare hämtade kompletta originaltidtabellen återanvänds: tabeller på skanning **2, 3 och 5**, teckenförklaring på skanning 6. URL och SHA-256 finns i `caribbean_source_evidence_batch137.json`. **Originalskanningarna distribueras inte.**

De **32 valda riktningsrubrikerna innehåller 124 tryckta rader**: **103 nonstoprader utan EXP** katalogiseras; **elva ST=1-rader** är genomgående resor; **tio EXP-rader** lämnas till en senare import med uttrycklig operatörstillhörighet. ST betyder Stops. A/P betyder AM/PM, X betyder undantag och 1–7 betyder måndag–söndag. Tom FRQ tolkas som dagligen. Tryckta utrustningskoder **CVR, D9S, D95, DC9, M80 och 72S** bevaras; urvalet består alltså inte bara av jetkoder.

**Asterisk betyder ny service enligt originalets teckenförklaring.** Åtta accepterade rader har asterisk: CID–MSP, MSP–CID, CWA–DTW och DTW–CWA, två i varje riktning. Ingen separat senare startdag trycks för dessa rader; de följer utgåvans giltighet. Symbolen bevaras i källrader och anmärkningar.

## Veckodagsvarianter och tidsgränser

Projektfönstret är **27 februari 1986 kl. 22:21:30 UTC till 1 mars kl. 22:21:30 UTC**. Ett flyg tas med när dess intervall överlappar fönstret; avgång före startgränsen kan alltså ge en rörelse.

| UTC-offset under samtliga tre datum | Flygplatser |
|---|---|
| −5 timmar | CMH, CVG, DTW, GRR, IND |
| −6 timmar | ATW, CID, CWA, GRB, LSE, MSN, MSP |

Offseterna, även historisk tid för Indianapolis, kontrolleras mot varje flygplats tidszon. Lokaltider följer tidtabellskonvention och är förenliga med flygtiderna; ingen separat uttrycklig all-times-local-text har återfunnits i originalet.

**41 rörelser avgår lokalt torsdag, 100 fredag och 70 lördag.** Fördelningen per schema är **tolv med tre utfall, 85 med två, fem med ett och ett med noll**. Alla **103 flygtider, 35–105 minuter**, och **211 UTC-intervall** har kontrollerats mot separat manuellt granskade varaktigheter och datum. Samma original används, inte en andra oberoende källa eller granskare.

| Rad | Variant | Lokala tider | Kod | Utfall |
|---|---|---|---|---:|
| 314 MSP–CWA | Utom lördag | 13:25–14:08 | D9S | 1 |
| 314 MSP–CWA | Lördag | 13:25–14:15 | CVR | 1 |
| 462 CWA–MSP | Utom lördag | 15:45–16:28 | D9S | 2 |
| 462 CWA–MSP | Lördag | 15:40–16:30 | CVR | 1 |
| 545 MSN–MSP | Lördag, nonstop | 19:50–20:42 | D95 | 0 |

Varianterna slås inte samman. De nya schema-ID:na inkluderar både avgångs- och ankomstklockslag för att skilja 314-raderna med samma avgångstid. Det ändrar inga äldre ID.

- **885 CID–MSP 15:20–16:20** landar 1 minut och 30 sekunder före torsdagens startgräns och ger fredag/lördag, två rörelser.
- **896 LSE–MSP 15:35–16:15, X6**, landar före torsdagens startgräns och går inte lördag: endast fredag. Även **532 MSP–LSE 13:40–14:15, X6**, ger endast fredag.
- Lördagens **545 MSN–MSP 19:50** avgår efter kl. 16:21:30 Central, alltså efter fönstrets slut. Schemat sparas utan rörelser.
- **373 DTW–GRB 13:00–13:00** tar 60 minuter; identiska lokala klockslag följer tidsskillnaden Eastern/Central.
- Förstorad originalgranskning visar **78 MSP–GRB 13:25–14:17** och **82 MSP–GRR 13:25–15:35**. Tidiga, ännu ej levererade transkriptioner lästes om och rättades före leveransen. De ansluter tidsmässigt till tidigare 78 PDX–MSP respektive 82 LAS–MSP med 55 respektive 60 minuter på marken.
- **576 MSP–MSN**, **576 MSN–DTW** och lördagens **545 MSN–MSP** har tryckt kod **D95**, kontrollerad i förstoring.
- Inga nya scheman ankommer nästa lokala kalenderdag. UTC-datum hanteras ändå separat för varje daterat ben.

## Genomgående rader som inte blir extra nonstopflyg

Samtliga följande har ST=1. Separata fysiska ben får egna tider; saknade mellantider gissas inte.

| Nummer | Genomgående resa | Lokala tider | Dagar | Kod | Skanning/kolumn |
|---|---|---|---|---|---|
| 915 | ATW → MSP | 15:00–16:36 | X6 | CVR | 2/1 |
| 910 | MSP → ATW | 13:10–14:40 | X6 | CVR | 5/2 |
| 936 | MSP → ATW | 18:40–20:15 | D | CVR | 5/2 |
| 617 | CVG → MSP | 07:05–08:43 | D | D95 | 2/2 |
| 656 | MSP → CVG | 18:35–22:05 | D | D95 | 5/2 |
| 544 | LSE → MSP | 06:30–07:39 | D | D9S | 3/4 |
| 933 | LSE → MSP | 07:25–08:36 | X7 | CVR | 3/4 |
| 915 | LSE → MSP | 15:20–16:36 | 6 | CVR | 3/4 |
| 910 | MSP → LSE | 13:10–14:20 | 6 | CVR | 5/3 |
| 225 | MSP → LSE | 20:50–22:00 | D | D9S | 5/3 |
| 545 | MSN → MSP | 19:50–22:10 | X6 | D9S | 5/2 |

Exempelvis finns separat CMH–MSP617 och MSP–CMH656 i det nya urvalet. De genomgående CVG–MSP617/MSP–CVG656 räknas inte som ytterligare direktflyg. För MSN–MSP545 utom lördag återstår det separat tryckta MSN–ORD-benet till en senare omgång; det redan importerade ORD–MSP-benet gör inte den genomgående raden till ett nonstopflyg.

## Express-rader för senare operatörsgranskning

Originalets legend anger **1400–1699 som Express Airlines I** och **1700–1899 som Simmons Airlines**. Alla tio EXP-rader inom det här urvalet ligger i Simmons-serien. De importeras inte som Republics egna nya flygrörelser i v140.

| Nummer | Sträcka | Lokala tider | Dagar |
|---|---|---|---|
| 1739 | GRR → DTW | 09:10–10:00 | X67 |
| 1842 | GRR → DTW | 11:40–12:30 | X67 |
| 1743 | GRR → DTW | 13:45–14:35 | X6 |
| 1745 | GRR → DTW | 17:30–18:20 | X6 |
| 1840 | DTW → GRR | 08:05–08:55 | X67 |
| 1841 | DTW → GRR | 10:20–11:10 | X67 |
| 1742 | DTW → GRR | 12:40–13:30 | X6 |
| 1843 | DTW → GRR | 14:20–15:10 | X6 |
| 1844 | DTW → GRR | 16:10–17:00 | X6 |
| 1818 | DTW → GRR | 16:15–17:05 | 6 |

## Angränsande ben

**104 daterade följder med samma flygnummer**, fördelade på **52 klock-/sträckmönster**, har granskats. **82** kopplar nya och äldre ben; **22** kopplar nya ben till nya. Markuppehållen är **15–70 minuter**. Kontrollen visar tidsmässig förenlighet och belägger inte faktisk drift, samma flygplansindivid eller en bokningsbar anslutning.

De kortaste är exempelvis 998 MSP–CWA–DTW med **15 minuter** i Central Wisconsin och 880 MSP–ATW–DTW med **18 minuter** i Appleton. Originalets angivna minsta anslutningstid gäller Memphis, Detroit och Minneapolis; det är inte ett generellt krav på alla dessa genomgående bens markuppehåll.

| Nummer | Benföljd | Ankomst–nästa avgång vid mellanpunkten | Markuppehåll, minuter |
|---|---|---|---:|
| 71 | GRR → MSP → PDX | 16:40–17:40 | 60 |
| 78 | PDX → MSP → GRB | 12:30–13:25 | 55 |
| 79 | GRB → MSP → SEA | 16:35–17:35 | 60 |
| 80 | SEA → MSP → GRB | 20:00–20:50 | 50 |
| 81 | DTW → MSN → MSP | 09:54–10:25 | 31 |
| 81 | LGA → DTW → MSN | 08:55–09:50 | 55 |
| 81 | MSN → MSP → SEA | 11:17–12:20 | 63 |
| 82 | LAS → MSP → GRR | 12:25–13:25 | 60 |
| 83 | GRB → MSP → LAS | 08:35–09:35 | 60 |
| 102 | PHX → MSP → MSN | 12:27–13:30 | 63 |
| 107 | CMH → MSP → PHX | 16:53–17:35 | 42 |
| 116 | SAN → MSP → CVG | 12:30–13:10 | 40 |
| 134 | GRB → DTW → IAD | 11:37–12:20 | 43 |
| 134 | MSP → GRB → DTW | 09:12–09:40 | 28 |
| 142 | GRR → DTW → FLL | 15:14–16:10 | 56 |
| 151 | GRB → DTW → STL | 08:52–09:40 | 48 |
| 185 | DTW → IND → MEM | 20:37–21:10 | 33 |
| 188 | MEM → IND → MSP | 10:55–11:20 | 25 |
| 193 | DCA → DTW → GRR | 12:30–13:10 | 40 |
| 235 | DTW → CWA → MSP | 10:15–10:40 | 25 |
| 302 | LAX → MSP → MSN | 17:45–18:25 | 40 |
| 305 | MSN → MSP → LAX | 16:32–17:30 | 58 |
| 341 | MSN → MSP → SFO | 08:42–09:35 | 53 |
| 347 | IND → MSP → SFO | 16:45–17:25 | 40 |
| 355 | CVG → MSP → SLC | 16:55–17:40 | 45 |
| 358 | MSN → DTW → LGA | 08:50–09:30 | 40 |
| 375 | FLL → DTW → GRR | 19:40–20:30 | 50 |
| 376 | SLC → MSP → CMH | 12:25–13:10 | 45 |
| 400 | GRR → MSP → STL | 08:40–09:25 | 45 |
| 416 | GRR → DTW → MIA | 07:49–08:30 | 41 |
| 418 | MSN → DTW → MCO | 15:20–16:00 | 40 |
| 418 | MSP → MSN → DTW | 12:56–13:20 | 24 |
| 419 | DTW → GRR → MSP | 10:22–10:50 | 28 |
| 421 | MCO → DTW → GRR | 21:25–22:05 | 40 |
| 462 | CWA → MSP → MEM | 16:28–17:10 | 42 |
| 544 | GRR → DTW → LGA | 11:39–12:20 | 41 |
| 544 | MSP → GRR → DTW | 10:30–11:00 | 30 |
| 545 | LGA → DTW → MSN | 18:19–19:20 | 61 |
| 549 | LGA → DTW → MSN | 20:49–21:55 | 66 |
| 565 | DTW → GRB → MSP | 09:46–10:15 | 29 |
| 565 | GRB → MSP → DEN-STAPLETON | 11:10–12:20 | 70 |
| 576 | MSP → MSN → DTW | 09:06–09:35 | 29 |
| 583 | BOS → DTW → GRB | 21:25–22:05 | 40 |
| 715 | PHL → DTW → GRR | 16:30–17:15 | 45 |
| 723 | BOS → DTW → IND | 16:30–17:10 | 40 |
| 732 | GRB → DTW → MEM | 19:42–20:50 | 68 |
| 739 | LGA → DTW → GRB | 16:20–17:05 | 45 |
| 752 | MSN → DTW → LGA | 19:50–20:45 | 55 |
| 753 | DCA → DTW → MSN | 16:25–17:10 | 45 |
| 880 | MSP → ATW → DTW | 09:12–09:30 | 18 |
| 913 | DTW → ATW → MSP | 10:07–10:30 | 23 |
| 998 | MSP → CWA → DTW | 09:00–09:15 | 15 |

## Transkriberade scheman

Skanningar och kolumner räknas från ett, kolumner från vänster. Lokala tider. D = tom daganmärkning/dagligen; 6 = lördag; X6 = utom lördag; X7 = utom söndag. Asterisk i sträckkolumnen markerar originalets ny-service-symbol. Rörelseantal gäller projektfönstret.

| Nummer | Sträcka | Lokala tider | Dagar | Kod | Rörelser | Skanning/kolumn |
|---|---|---|---|---|---:|---|
| 880 | ATW → DTW | 09:30–11:40 | D | CVR | 2 | 2/1 |
| 754 | ATW → DTW | 17:50–19:50 | D | CVR | 2 | 2/1 |
| 891 | ATW → MSP | 07:20–08:30 | D | CVR | 2 | 2/1 |
| 913 | ATW → MSP | 10:30–11:40 | D | CVR | 2 | 2/1 |
| 883 | CID → MSP * | 07:50–08:50 | D | CVR | 2 | 2/2 |
| 885 | CID → MSP * | 15:20–16:20 | D | CVR | 2 | 2/2 |
| 617 | CMH → MSP | 08:00–08:43 | D | D95 | 2 | 2/2 |
| 107 | CMH → MSP | 16:10–16:53 | D | D9S | 3 | 2/2 |
| 355 | CVG → MSP | 16:10–16:55 | D | D9S | 3 | 2/2 |
| 998 | CWA → DTW * | 09:15–11:40 | D | CVR | 2 | 2/2 |
| 997 | CWA → DTW * | 17:50–20:10 | D | CVR | 2 | 2/2 |
| 537 | CWA → MSP | 07:50–08:33 | D | CVR | 2 | 2/2 |
| 235 | CWA → MSP | 10:40–11:30 | D | CVR | 2 | 2/2 |
| 462 | CWA → MSP | 15:40–16:30 | 6 | CVR | 1 | 2/2 |
| 462 | CWA → MSP | 15:45–16:28 | X6 | D9S | 2 | 2/2 |
| 879 | CWA → MSP | 19:30–20:20 | X6 | CVR | 2 | 2/2 |
| 913 | DTW → ATW | 09:40–10:07 | D | CVR | 2 | 2/3 |
| 709 | DTW → ATW | 17:00–17:27 | D | CVR | 3 | 2/3 |
| 235 | DTW → CWA * | 09:45–10:15 | D | CVR | 2 | 2/4 |
| 755 | DTW → CWA * | 17:00–17:30 | D | CVR | 3 | 2/4 |
| 565 | DTW → GRB | 09:45–09:46 | D | DC9 | 2 | 2/4 |
| 373 | DTW → GRB | 13:00–13:00 | D | D9S | 2 | 2/4 |
| 739 | DTW → GRB | 17:05–17:06 | D | D9S | 3 | 2/4 |
| 583 | DTW → GRB | 22:05–22:06 | X6 | D9S | 2 | 2/4 |
| 419 | DTW → GRR | 09:40–10:22 | D | D9S | 2 | 2/4 |
| 193 | DTW → GRR | 13:10–13:52 | D | D9S | 2 | 2/4 |
| 715 | DTW → GRR | 17:15–17:57 | D | D9S | 3 | 2/4 |
| 763 | DTW → GRR | 19:20–20:10 | D | CVR | 2 | 2/4 |
| 375 | DTW → GRR | 20:30–21:10 | X6 | D9S | 2 | 2/4 |
| 421 | DTW → GRR | 22:05–22:47 | X6 | D9S | 2 | 2/4 |
| 683 | DTW → IND | 09:25–10:20 | D | D95 | 2 | 2/4 |
| 695 | DTW → IND | 13:00–13:57 | D | D9S | 2 | 2/4 |
| 723 | DTW → IND | 17:10–18:07 | D | D9S | 3 | 2/4 |
| 185 | DTW → IND | 19:40–20:37 | D | D9S | 2 | 2/4 |
| 539 | DTW → IND | 22:05–23:02 | X6 | D9S | 2 | 2/4 |
| 81 | DTW → MSN | 09:50–09:54 | D | D9S | 2 | 3/1 |
| 753 | DTW → MSN | 17:10–17:14 | D | D9S | 3 | 3/1 |
| 545 | DTW → MSN | 19:20–19:24 | D | D9S | 2 | 3/1 |
| 549 | DTW → MSN | 21:55–21:59 | X6 | D9S | 2 | 3/1 |
| 151 | GRB → DTW | 06:55–08:52 | X7 | D95 | 2 | 3/3 |
| 134 | GRB → DTW | 09:40–11:37 | D | DC9 | 2 | 3/3 |
| 194 | GRB → DTW | 13:25–15:20 | D | D9S | 2 | 3/3 |
| 732 | GRB → DTW | 17:45–19:42 | D | D9S | 2 | 3/3 |
| 83 | GRB → MSP | 07:40–08:35 | D | D9S | 2 | 3/3 |
| 565 | GRB → MSP | 10:15–11:10 | D | DC9 | 2 | 3/3 |
| 79 | GRB → MSP | 15:40–16:35 | D | D9S | 3 | 3/3 |
| 867 | GRB → MSP | 19:10–20:15 | D | CVR | 2 | 3/3 |
| 416 | GRR → DTW | 07:10–07:49 | X7 | D95 | 2 | 3/3 |
| 172 | GRR → DTW | 08:05–08:44 | D | D9S | 2 | 3/3 |
| 544 | GRR → DTW | 11:00–11:39 | D | D9S | 2 | 3/3 |
| 142 | GRR → DTW | 14:35–15:14 | D | D9S | 2 | 3/3 |
| 716 | GRR → DTW | 19:05–19:44 | D | D9S | 2 | 3/3 |
| 606 | GRR → DTW | 20:30–21:12 | X6 | CVR | 2 | 3/3 |
| 400 | GRR → MSP | 08:20–08:40 | D | D9S | 2 | 3/3 |
| 419 | GRR → MSP | 10:50–11:10 | D | D9S | 2 | 3/3 |
| 71 | GRR → MSP | 16:20–16:40 | D | D9S | 3 | 3/3 |
| 320 | IND → DTW | 06:55–07:48 | X7 | D95 | 2 | 3/3 |
| 270 | IND → DTW | 10:45–11:38 | D | D9S | 2 | 3/3 |
| 154 | IND → DTW | 14:25–15:18 | D | D9S | 2 | 3/3 |
| 724 | IND → DTW | 19:00–19:50 | D | D95 | 2 | 3/3 |
| 461 | IND → MSP | 08:05–08:35 | D | D9S | 2 | 3/3 |
| 188 | IND → MSP | 11:20–11:50 | D | DC9 | 2 | 3/3 |
| 347 | IND → MSP | 16:15–16:45 | D | M80 | 3 | 3/3 |
| 935 | LSE → MSP | 10:45–11:25 | D | CVR | 2 | 3/4 |
| 896 | LSE → MSP | 15:35–16:15 | X6 | CVR | 1 | 3/4 |
| 929 | LSE → MSP | 19:40–20:20 | D | CVR | 2 | 3/4 |
| 358 | MSN → DTW | 06:55–08:50 | X7 | D9S | 2 | 5/2 |
| 576 | MSN → DTW | 09:35–11:35 | D | D95 | 2 | 5/2 |
| 418 | MSN → DTW | 13:20–15:20 | D | D9S | 2 | 5/2 |
| 752 | MSN → DTW | 17:50–19:50 | D | D9S | 2 | 5/2 |
| 341 | MSN → MSP | 07:50–08:42 | D | 72S | 2 | 5/2 |
| 81 | MSN → MSP | 10:25–11:17 | D | D9S | 2 | 5/2 |
| 305 | MSN → MSP | 15:40–16:32 | D | M80 | 3 | 5/2 |
| 545 | MSN → MSP | 19:50–20:42 | 6 | D95 | 0 | 5/2 |
| 880 | MSP → ATW | 08:10–09:12 | D | CVR | 2 | 5/2 |
| 882 | MSP → CID * | 13:20–14:20 | D | CVR | 2 | 5/2 |
| 884 | MSP → CID * | 20:50–21:50 | D | CVR | 2 | 5/2 |
| 376 | MSP → CMH | 13:10–15:45 | D | D9S | 2 | 5/2 |
| 656 | MSP → CMH | 18:35–21:10 | D | D95 | 2 | 5/2 |
| 116 | MSP → CVG | 13:10–15:44 | D | D9S | 2 | 5/2 |
| 998 | MSP → CWA | 08:10–09:00 | X7 | CVR | 2 | 5/2 |
| 314 | MSP → CWA | 13:25–14:08 | X6 | D9S | 1 | 5/2 |
| 314 | MSP → CWA | 13:25–14:15 | 6 | CVR | 1 | 5/2 |
| 876 | MSP → CWA | 18:20–19:10 | D | CVR | 2 | 5/2 |
| 68 | MSP → CWA | 20:50–21:40 | D | CVR | 2 | 5/2 |
| 134 | MSP → GRB | 08:20–09:12 | D | DC9 | 2 | 5/3 |
| 78 | MSP → GRB | 13:25–14:17 | D | D9S | 2 | 5/3 |
| 924 | MSP → GRB | 17:30–18:37 | D | CVR | 2 | 5/3 |
| 80 | MSP → GRB | 20:50–21:40 | D | D9S | 2 | 5/3 |
| 544 | MSP → GRR | 08:20–10:30 | D | D9S | 2 | 5/3 |
| 82 | MSP → GRR | 13:25–15:35 | D | D9S | 2 | 5/3 |
| 110 | MSP → GRR | 19:20–21:30 | D | D95 | 2 | 5/3 |
| 674 | MSP → IND | 08:25–10:47 | D | DC9 | 2 | 5/3 |
| 114 | MSP → IND | 12:20–14:42 | D | M80 | 2 | 5/3 |
| 658 | MSP → IND | 18:25–20:47 | D | D9S | 2 | 5/3 |
| 931 | MSP → LSE | 06:30–07:05 | X7 | CVR | 2 | 5/3 |
| 868 | MSP → LSE | 09:40–10:15 | D | CVR | 2 | 5/3 |
| 532 | MSP → LSE | 13:40–14:15 | X6 | CVR | 1 | 5/3 |
| 928 | MSP → LSE | 18:40–19:15 | D | CVR | 2 | 5/3 |
| 576 | MSP → MSN | 08:20–09:06 | D | D95 | 2 | 5/3 |
| 418 | MSP → MSN | 12:10–12:56 | D | D9S | 2 | 5/3 |
| 102 | MSP → MSN | 13:30–14:16 | D | M80 | 2 | 5/3 |
| 302 | MSP → MSN | 18:25–19:11 | D | 72S | 2 | 5/3 |

## Regional prioritet och kvarstående arbete

Centralamerika/Karibien, samma **31 forskningsområden**, ligger kvar på **532 rörelser, 158 riktade par, 38 flygplatser och 64 inrikesrörelser**. **St. Thomas 14, Tortola 8 och Nicaragua 0** är oförändrade. Mexiko har 20; Mellanösternurvalet 72. Noll betyder en datalucka, inte frånvaro av historisk trafik.

**Ingen ny regional arkivsökning gjordes denna omgång.** `regional_source_search_batch137.json` dokumenterar återanvändningen av den befintliga Republic-originaltidtabellen. Prioriteten Nicaragua/St. Thomas och tidigare spår efter Aero Virgin Islands 15 december 1985, Gull Air 15 februari 1986, LACSA, Caribbean Express, Eastern, American, BWIA, TACA, Aviateca, TAN SAHSA och sjöflygets tider/hamnar kvarstår. Tidigare LIAT-, Challenge-, Arrow-, Airways International-, Air France F27-, PBA789- och specialflygsfrågor är bevarade. Inga nya militär-, privat- eller specialflyg importeras.

Republics övriga huvudlinjer, separat operatörsattribuerad Express-trafik, fysiska SFO–SMF/SMF–SFO- och MSN–ORD-ben, ändringsblad och faktisk drift återstår. Inga saknade mellantider fylls i genom antaganden.

## Totalt, verifiering och installation

**11 155 rörelser: 11 154 planerade och en tidigare bekräftad. 6 637 katalogscheman, varav 6 616 granskade. 393 flygplatser/platser, 121 länder/territorier och 1 989 riktade par. 111 operatörsposter: 68 med rörelser, 43 utan. 94 källposter och 36 bidragande tidtabellsutgåvor.** Ingen verifierad global bolagsinventering eller färdigprocent finns.

**20 befintliga Python-tester och tre generatorkontroller passerar.** Samtliga **1 590 Republic-rörelser** har kontrollerats för dubbletter och överlappning av samma flygnummer; inga kvarstår. Samtliga äldre schema- och rörelseobjekt är oförändrade. ZIP-filens CRC och varje fils hash kontrolleras mot manifestet vid paketering.

**Unreal Editor och spelet har inte körts här.** Paketet är kumulativt: stäng Unreal och slå samman `Plugins/` och `Tools/` med projektroten bredvid `.uproject`. **PaintAirplane-rättelsen f23b73a måste redan vara kompilerad.** Ingen ny C++-kod ingår.
