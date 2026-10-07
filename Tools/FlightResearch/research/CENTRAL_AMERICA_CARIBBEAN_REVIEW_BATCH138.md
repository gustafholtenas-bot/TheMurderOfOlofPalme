# Fler Republic-linjer kring Detroit och kompletterande mellanben – v141 / batch138

Granskat 2 oktober 2026. **192 nya planerade flygrörelser**, totalt **11 347**. Tillägget omfattar **27 riktade sträckor, varav 19 är nya i databasen**, och **96 nya granskade scheman**. 95 ger rörelser i projektfönstret; ett lördagsschema ger noll.

**Alla 11 155 äldre rörelseobjekt och 6 637 äldre scheman är oförändrade.** Även v139-rättelsen av LAX–DTW 12:45 till flygnummer **334**, med dess stabila äldre ID, är bevarad. V141 ändrar inga äldre rörelser. Flygplats-, land- och operatörsposter bevaras; Republic-källpostens granskningsmetadata utökas. Äldre forskningsrapporter och rättelser bevaras byte för byte.

Alla tillägg är inrikes i USA, utanför Centralamerika/Karibien-urvalet. **Inga nya flygplatser, länder, operatörer eller tidtabellsutgåvor**. Republic har nu **1 782 rörelser från 871 scheman**; nätet är fortfarande partiellt.

## Nya rörelser

| Riktad sträcka | Nya rörelser | Ny sträcka i databasen |
|---|---:|---|
| ALB–DTW | 6 | Ja |
| ATL–DTW | 8 | Nej |
| BDL–DTW | 8 | Ja |
| BNA–DTW | 6 | Ja |
| BNA–ORD | 6 | Ja |
| BUF–DTW | 8 | Ja |
| BWI–DTW | 8 | Ja |
| CLE–DTW | 10 | Ja |
| CMH–CVG | 2 | Nej |
| CMH–DTW | 10 | Ja |
| CVG–CMH | 2 | Nej |
| CVG–DTW | 8 | Nej |
| DTW–ALB | 7 | Ja |
| DTW–ATL | 9 | Nej |
| DTW–BDL | 9 | Ja |
| DTW–BNA | 6 | Ja |
| DTW–BUF | 8 | Ja |
| DTW–BWI | 9 | Ja |
| DTW–CLE | 10 | Nej |
| DTW–CMH | 10 | Ja |
| DTW–CVG | 9 | Nej |
| DTW–PIT | 6 | Ja |
| LGA–MKE | 6 | Ja |
| MKE–LGA | 6 | Ja |
| MSN–ORD | 2 | Nej |
| ORD–BNA | 7 | Ja |
| PIT–DTW | 6 | Ja |

ALB = Albany County; ATL = Atlanta/Hartsfield; BDL = Hartford/Springfield/Bradley; BNA = Nashville/Berry Field; BUF = Buffalo; BWI = Baltimore/Washington; CLE = Cleveland/Hopkins; CMH = Columbus/Port Columbus; CVG = Greater Cincinnati; DTW = Detroit; LGA = New York/LaGuardia; MKE = Milwaukee/General Mitchell; MSN = Madison/Dane County; ORD = Chicago/O’Hare; PIT = Greater Pittsburgh. Befintliga historiskt granskade flygplatsmarkörer används oförändrade.

En rörelse är ett daterat fysiskt flygben. Flera avgångar på samma linje och de två riktningarna räknas var för sig.

## Källa och omfattning

[Republic Airlines Employee System Timetable, 14 februari–1 mars 1986](https://northwestairlineshistory.org/wp-content/uploads/2018/07/REP-employee-schedule-1986-02-14.pdf), bevarad hos Northwest Airlines History Center. Omslagets uttryckliga giltighet täcker importperioden. Den tidigare hämtade kompletta originaltidtabellen återanvänds: tabeller på skanning **2, 3, 5 och 6**, teckenförklaring på skanning 6. URL och SHA-256 finns i `caribbean_source_evidence_batch138.json`. **Originalskanningarna distribueras inte.**

De **27 valda riktningsrubrikerna innehåller 133 tryckta rader**: **96 med tom ST och utan EXP** katalogiseras; **37 EXP-rader** lämnas till en separat operatörsattribuerad import. Inga ST=1-rader finns inom just dessa rubriker. Fyra relevanta genomgående resor utanför rubrikurvalet dokumenteras nedan; de räknas inte som extra nonstopflyg.

ST betyder Stops. A/P betyder AM/PM, X betyder undantag och 1–7 betyder måndag–söndag. Tom FRQ tolkas som dagligen. Tryckta utrustningskoder **CVR, D9S, D95, DC9, M80 och 72S** bevaras. Inga accepterade nya rader har ny-service-asterisk. All trafik är tidtabellsplaner; faktiskt genomförande och flygplansindivid är inte belagda.

## Tider och veckodagsvarianter

Projektfönstret är **27 februari 1986 kl. 22:21:30 UTC till 1 mars kl. 22:21:30 UTC**. Flygets tidsintervall ska överlappa fönstret; avgång före startgränsen kan ge en rörelse om flyget ännu är i luften.

| UTC-offset under samtliga tre datum | Flygplatser |
|---|---|
| −5 timmar | ALB, ATL, BDL, BUF, BWI, CLE, CMH, CVG, DTW, LGA, PIT |
| −6 timmar | BNA, MKE, MSN, ORD |

Offseterna har kontrollerats mot varje flygplats tidszon under alla tre datum. Lokaltider följer tidtabellskonvention och är förenliga med flygtiderna; ingen separat uttrycklig all-times-local-text har återfunnits i originalet.

**35 rörelser avgår lokalt torsdag, 93 fredag och 64 lördag.** Fördelningen per schema är **åtta med tre utfall, 81 med två, sex med ett och ett med noll**. Alla **96 flygtider, 30–135 minuter**, även nollutfallet, och **192 UTC-intervall** har kontrollerats mot separat manuellt granskade varaktigheter och datum. Samma original används; detta är inte en andra oberoende källa eller granskare.

| Rad | Variant | Lokala tider | Kod | Utfall |
|---|---|---|---|---:|
| 995 CLE–DTW | Utom lördag | 14:05–14:50 | CVR | 1 |
| 995 CLE–DTW | Lördag | 14:05–14:47 | DC9 | 1 |
| 998 DTW–CLE | Utom lördag | 12:25–13:08 | CVR | 1 |
| 998 DTW–CLE | Lördag | 12:25–13:03 | DC9 | 1 |
| 692 DTW–BWI | Lördag | 20:50–22:07 | D95 | 0 |
| 718 DTW–BWI | Utom lördag | 20:50–22:07 | D95 | 2 |

Varianterna behåller egna tider, dagar och utrustningskoder. Nya schema-ID:n inkluderar avgångs- och ankomstklockslag så att varianter med samma avgångstid förblir separata. Äldre ID ändras inte.

- **312 DTW–ALB** och **194 DTW–BWI** ankommer kl. **17:27 Eastern**, 5 minuter och 30 sekunder efter torsdagens startgräns: tre rörelser vardera.
- **54 ORD–BNA 16:15–17:30 Central** avgår 6 minuter och 30 sekunder före lördagens slutgräns och ger tre rörelser.
- **692 DTW–BWI 20:50** går bara lördag och ligger efter fönstrets slut. Raden sparas som granskat schema utan rörelser.
- **351 LGA–MKE**, X67, och **350 MKE–LGA**, X6, ger en rörelse vardera, fredag, när deras dagar och fönsteröverlapp kombineras.
- Inga nya scheman ankommer nästa lokala kalenderdag. UTC-datumskiften hanteras ändå per daterat ben.

## Kompletterande fysiska ben

Följande genomgående rader ligger utanför de 27 valda rubrikerna. De importeras inte som extra nonstopflyg:

| Genomgående rad | Lokala tider/dagar | Tryckt ST | Separata ben |
|---|---|---:|---|
| 545 MSN–MSP | 19:50–22:10, X6 | 1 | Nytt MSN–ORD 19:50–20:27 + befintligt ORD–MSP 20:55–22:10 |
| 617 CVG–MSP | 07:05–08:43, dagligen | 1 | Nytt CVG–CMH 07:05–07:35 + befintligt CMH–MSP 08:00–08:43 |
| 656 MSP–CVG | 18:35–22:05, dagligen | 1 | Befintligt MSP–CMH 18:35–21:10 + nytt CMH–CVG 21:35–22:05 |
| 393 BNA–MSP | 10:25–13:30, X67 | 1 | Nytt BNA–ORD 10:25–11:45, dagligen + befintligt ORD–MSP 12:15–13:30, X67 |

**393 BNA–ORD har tom FRQ och är dagligt enligt den separata benraden.** Det äldre ORD–MSP-benet behåller X67. Därför finns BNA–ORD även lördag utan att ett obelagt lördagsben till Minneapolis skapas. Genomgående resans veckodagar kopieras inte över till det separat tryckta benet.

De nya fysiska benen är 30 minuter för CVG–CMH/CMH–CVG och 37 minuter för MSN–ORD. Uppehållen är 25 minuter i Columbus och 28 minuter i Chicago. De följer tryckta benklockor; inga saknade mellantider gissas.

## Express-rader för senare granskning

Originalets legend anger **1400–1699 som Express Airlines I** och **1700–1899 som Simmons Airlines**. Alla **37 EXP-rader** inom det här urvalet ligger i Simmons-serien. De importeras inte som Republics egna nya rörelser i v141. Samtliga har tom ST.

| Nummer | Sträcka | Lokala tider | Dagar | Skanning/kolumn |
|---|---|---|---|---|
| 1826 | CLE → DTW | 09:00–09:40 | X7 | 2/2 |
| 1741 | CLE → DTW | 10:00–10:40 | X7 | 2/2 |
| 1717 | CLE → DTW | 11:00–11:40 | X7 | 2/2 |
| 1786 | CLE → DTW | 12:00–12:40 | X6 | 2/2 |
| 1727 | CLE → DTW | 13:00–13:40 | D | 2/2 |
| 1789 | CLE → DTW | 14:00–14:40 | X6 | 2/2 |
| 1704 | CLE → DTW | 15:00–15:40 | X6 | 2/2 |
| 1791 | CLE → DTW | 16:00–16:40 | D | 2/2 |
| 1707 | CLE → DTW | 17:00–17:40 | X6 | 2/2 |
| 1793 | CLE → DTW | 18:00–18:40 | X6 | 2/2 |
| 1784 | CLE → DTW | 19:00–19:40 | X6 | 2/2 |
| 1796 | CLE → DTW | 20:00–20:40 | X6 | 2/2 |
| 1759 | CMH → DTW | 08:20–09:15 | X67 | 2/2 |
| 1781 | CMH → DTW | 09:10–10:10 | X67 | 2/2 |
| 1798 | CMH → DTW | 12:15–13:10 | X6 | 2/2 |
| 1798 | CMH → DTW | 12:15–13:15 | 6 | 2/2 |
| 1734 | CMH → DTW | 16:30–17:30 | X6 | 2/2 |
| 1736 | CMH → DTW | 18:50–19:50 | X6 | 2/2 |
| 1747 | CMH → DTW | 19:25–20:20 | X6 | 2/2 |
| 1821 | DTW → CLE | 08:00–08:40 | X7 | 2/3 |
| 1724 | DTW → CLE | 09:00–09:40 | X7 | 2/3 |
| 1716 | DTW → CLE | 10:00–10:40 | X7 | 2/3 |
| 1785 | DTW → CLE | 11:00–11:40 | X6 | 2/3 |
| 1726 | DTW → CLE | 12:00–12:40 | D | 2/3 |
| 1788 | DTW → CLE | 13:00–13:40 | X6 | 2/3 |
| 1728 | DTW → CLE | 14:00–14:40 | X6 | 2/3 |
| 1790 | DTW → CLE | 15:00–15:40 | D | 2/3 |
| 1705 | DTW → CLE | 16:00–16:40 | X6 | 2/3 |
| 1792 | DTW → CLE | 17:00–17:40 | X6 | 2/3 |
| 1794 | DTW → CLE | 18:00–18:40 | X6 | 2/3 |
| 1795 | DTW → CLE | 19:00–19:40 | X6 | 2/3 |
| 1758 | DTW → CMH | 07:10–08:05 | X67 | 2/3 |
| 1780 | DTW → CMH | 07:55–08:55 | X67 | 2/3 |
| 1797 | DTW → CMH | 11:00–12:00 | D | 2/3 |
| 1733 | DTW → CMH | 15:15–16:15 | X6 | 2/3 |
| 1735 | DTW → CMH | 17:35–18:35 | X6 | 2/3 |
| 1744 | DTW → CMH | 18:15–19:10 | X6 | 2/3 |

## Angränsande ben

**105 daterade följder med samma flygnummer**, fördelade på **53 klock-/sträckmönster**, har granskats. **87** kopplar nya och äldre ben; **18** kopplar nya ben till nya. Markuppehållen är **25–67 minuter**. Kontrollen visar tidsmässig förenlighet och belägger inte faktisk drift, samma flygplansindivid eller en bokningsbar anslutning. Originalets minsta anslutningstider vid Memphis, Detroit och Minneapolis används inte som generellt krav vid andra flygplatser.

| Nummer | Benföljd | Ankomst–nästa avgång vid mellanpunkten | Markuppehåll, minuter |
|---|---|---|---:|
| 15 | CLE → DTW → ORD | 08:47–09:30 | 43 |
| 32 | ORD → DTW → PIT | 19:58–20:45 | 47 |
| 54 | ORD → BNA → MEM | 17:30–18:00 | 30 |
| 57 | BDL → DTW → BNA | 12:23–13:10 | 47 |
| 57 | DTW → BNA → MEM | 13:33–14:05 | 32 |
| 67 | BDL → DTW → MSP | 16:18–17:00 | 42 |
| 139 | BDL → DTW → TPA | 08:48–09:35 | 47 |
| 194 | GRB → DTW → BWI | 15:20–16:10 | 50 |
| 215 | BDL → DTW → MKE | 21:03–22:00 | 57 |
| 238 | CVG → DTW → BUF | 11:40–12:25 | 45 |
| 253 | BWI → DTW → MSP | 12:17–13:00 | 43 |
| 266 | CVG → DTW → BDL | 07:50–08:30 | 40 |
| 268 | HOU → DTW → BWI | 11:25–12:15 | 50 |
| 270 | IND → DTW → BDL | 11:38–12:20 | 42 |
| 289 | BUF → DTW → HOU | 12:20–13:10 | 50 |
| 312 | CVG → DTW → ALB | 15:20–16:05 | 45 |
| 316 | CMH → DTW → ALB | 11:33–12:15 | 42 |
| 320 | IND → DTW → BWI | 07:48–08:30 | 42 |
| 329 | BWI → DTW → CMH | 20:57–21:55 | 58 |
| 330 | LAX → DTW → BUF | 15:10–16:05 | 55 |
| 339 | BUF → DTW → LAX | 18:22–19:20 | 58 |
| 373 | ATL → DTW → GRB | 12:19–13:00 | 41 |
| 379 | MIA → DTW → BUF | 19:50–20:40 | 50 |
| 387 | MEM → BNA → ORD | 13:25–13:55 | 30 |
| 393 | BNA → ORD → MSP | 11:45–12:15 | 30 |
| 393 | MEM → BNA → ORD | 09:50–10:25 | 35 |
| 541 | BOS → DTW → CVG | 18:40–19:25 | 45 |
| 545 | DTW → MSN → ORD | 19:24–19:50 | 26 |
| 545 | MSN → ORD → MSP | 20:27–20:55 | 28 |
| 565 | ATL → DTW → GRB | 08:44–09:45 | 61 |
| 581 | BWI → DTW → BNA | 08:47–09:50 | 63 |
| 581 | DTW → BNA → MEM | 10:13–10:45 | 32 |
| 605 | ALB → DTW → CMH | 08:47–09:30 | 43 |
| 617 | CVG → CMH → MSP | 07:35–08:00 | 25 |
| 656 | MSP → CMH → CVG | 21:10–21:35 | 25 |
| 683 | BUF → DTW → IND | 08:30–09:25 | 55 |
| 707 | MSP → DTW → BNA | 18:30–19:25 | 55 |
| 708 | MEM → BNA → DTW | 16:40–17:15 | 35 |
| 716 | GRR → DTW → ATL | 19:44–20:40 | 56 |
| 718 | MSP → DTW → BWI | 19:50–20:50 | 60 |
| 719 | ALB → DTW → DFW | 16:22–17:05 | 43 |
| 721 | BUF → DTW → CVG | 16:20–17:00 | 40 |
| 724 | IND → DTW → BDL | 19:50–20:50 | 60 |
| 726 | STL → DTW → CLE | 19:40–20:30 | 50 |
| 729 | CMH → DTW → ORD | 18:28–19:35 | 67 |
| 733 | CLE → DTW → PHX | 18:27–19:25 | 58 |
| 735 | BWI → DTW → HOU | 16:27–17:15 | 48 |
| 740 | MSP → DTW → CLE | 15:30–16:20 | 50 |
| 751 | PIT → DTW → SFO | 18:30–19:35 | 65 |
| 772 | SAN → DTW → PIT | 15:10–16:00 | 50 |
| 773 | BNA → DTW → LGA | 15:05–15:55 | 50 |
| 774 | MEM → DTW → BDL | 15:14–16:10 | 56 |
| 998 | CWA → DTW → CLE | 11:40–12:25 | 45 |

## Transkriberade scheman

Skanningar och kolumner räknas från ett, kolumner från vänster. Alla tider är lokala. D = tom daganmärkning/dagligen; 6 = lördag; X6 = utom lördag; X7 = utom söndag; X67 = måndag–fredag. Rörelseantal gäller projektfönstret.

| Nummer | Sträcka | Lokala tider | Dagar | Kod | Rörelser | Skanning/kolumn |
|---|---|---|---|---|---:|---|
| 605 | ALB → DTW | 07:15–08:47 | D | D9S | 2 | 2/1 |
| 719 | ALB → DTW | 14:50–16:22 | D | D9S | 2 | 2/1 |
| 159 | ALB → DTW | 18:10–19:42 | D | DC9 | 2 | 2/1 |
| 565 | ATL → DTW | 07:00–08:44 | X7 | DC9 | 2 | 2/1 |
| 373 | ATL → DTW | 10:35–12:19 | D | D9S | 2 | 2/1 |
| 779 | ATL → DTW | 13:20–15:04 | D | DC9 | 2 | 2/1 |
| 711 | ATL → DTW | 18:10–19:50 | D | D9S | 2 | 2/1 |
| 139 | BDL → DTW | 07:00–08:48 | D | D95 | 2 | 2/1 |
| 57 | BDL → DTW | 10:35–12:23 | D | D95 | 2 | 2/1 |
| 67 | BDL → DTW | 14:30–16:18 | D | D95 | 2 | 2/1 |
| 215 | BDL → DTW | 19:15–21:03 | X6 | D95 | 2 | 2/1 |
| 272 | BNA → DTW | 09:05–11:25 | D | D9S | 2 | 2/1 |
| 773 | BNA → DTW | 12:45–15:05 | D | D9S | 2 | 2/1 |
| 708 | BNA → DTW | 17:15–19:35 | D | 72S | 2 | 2/1 |
| 43 | BNA → ORD | 08:05–09:23 | X7 | D9S | 2 | 2/1 |
| 393 | BNA → ORD | 10:25–11:45 | D | D9S | 2 | 2/1 |
| 387 | BNA → ORD | 13:55–15:13 | D | D9S | 2 | 2/1 |
| 683 | BUF → DTW | 07:35–08:30 | D | D95 | 2 | 2/2 |
| 289 | BUF → DTW | 11:20–12:20 | D | D9S | 2 | 2/2 |
| 721 | BUF → DTW | 15:20–16:20 | D | D9S | 2 | 2/2 |
| 339 | BUF → DTW | 17:30–18:22 | D | 72S | 2 | 2/2 |
| 581 | BWI → DTW | 07:20–08:47 | D | D95 | 2 | 2/2 |
| 253 | BWI → DTW | 10:50–12:17 | D | D95 | 2 | 2/2 |
| 735 | BWI → DTW | 15:00–16:27 | D | D9S | 2 | 2/2 |
| 329 | BWI → DTW | 19:30–20:57 | X6 | D9S | 2 | 2/2 |
| 991 | CLE → DTW | 07:05–07:45 | X7 | D9S | 2 | 2/2 |
| 15 | CLE → DTW | 08:05–08:47 | D | D95 | 2 | 2/2 |
| 995 | CLE → DTW | 14:05–14:50 | X6 | CVR | 1 | 2/2 |
| 995 | CLE → DTW | 14:05–14:47 | 6 | DC9 | 1 | 2/2 |
| 733 | CLE → DTW | 17:45–18:27 | D | M80 | 2 | 2/2 |
| 989 | CLE → DTW | 20:40–21:25 | X6 | CVR | 2 | 2/2 |
| 656 | CMH → CVG | 21:35–22:05 | D | D95 | 2 | 2/2 |
| 230 | CMH → DTW | 07:30–08:18 | X7 | D9S | 2 | 2/2 |
| 316 | CMH → DTW | 10:45–11:33 | D | D9S | 2 | 2/2 |
| 993 | CMH → DTW | 14:15–15:07 | D | CVR | 2 | 2/2 |
| 729 | CMH → DTW | 17:40–18:28 | D | DC9 | 2 | 2/2 |
| 955 | CMH → DTW | 20:35–21:25 | X6 | CVR | 2 | 2/2 |
| 617 | CVG → CMH | 07:05–07:35 | D | D95 | 2 | 2/2 |
| 266 | CVG → DTW | 06:55–07:50 | X7 | D95 | 2 | 2/2 |
| 238 | CVG → DTW | 10:45–11:40 | D | D9S | 2 | 2/2 |
| 312 | CVG → DTW | 14:25–15:20 | D | DC9 | 2 | 2/2 |
| 722 | CVG → DTW | 18:55–19:50 | D | D9S | 2 | 2/2 |
| 316 | DTW → ALB | 12:15–13:37 | D | D9S | 2 | 2/3 |
| 312 | DTW → ALB | 16:05–17:27 | D | DC9 | 3 | 2/3 |
| 654 | DTW → ALB | 20:50–22:12 | D | D9S | 2 | 2/3 |
| 370 | DTW → ATL | 08:20–10:05 | X7 | D9S | 2 | 2/3 |
| 372 | DTW → ATL | 13:00–14:45 | D | D9S | 2 | 2/3 |
| 768 | DTW → ATL | 16:00–17:45 | D | D9S | 3 | 2/3 |
| 716 | DTW → ATL | 20:40–22:25 | D | D9S | 2 | 2/3 |
| 266 | DTW → BDL | 08:30–09:55 | X7 | D95 | 2 | 2/3 |
| 270 | DTW → BDL | 12:20–13:45 | D | D95 | 2 | 2/3 |
| 774 | DTW → BDL | 16:10–17:35 | D | D95 | 3 | 2/3 |
| 724 | DTW → BDL | 20:50–22:15 | D | D95 | 2 | 2/3 |
| 581 | DTW → BNA | 09:50–10:13 | D | D95 | 2 | 2/3 |
| 57 | DTW → BNA | 13:10–13:33 | D | D95 | 2 | 2/3 |
| 707 | DTW → BNA | 19:25–19:48 | D | D9S | 2 | 2/3 |
| 170 | DTW → BUF | 09:35–10:30 | D | D9S | 2 | 2/3 |
| 238 | DTW → BUF | 12:25–13:20 | D | D9S | 2 | 2/3 |
| 330 | DTW → BUF | 16:05–17:00 | D | 72S | 2 | 2/3 |
| 379 | DTW → BUF | 20:40–21:35 | D | D95 | 2 | 2/3 |
| 320 | DTW → BWI | 08:30–09:47 | X7 | D95 | 2 | 2/3 |
| 268 | DTW → BWI | 12:15–13:32 | D | D9S | 2 | 2/3 |
| 194 | DTW → BWI | 16:10–17:27 | D | D9S | 3 | 2/3 |
| 692 | DTW → BWI | 20:50–22:07 | 6 | D95 | 0 | 2/3 |
| 718 | DTW → BWI | 20:50–22:07 | X6 | D95 | 2 | 2/3 |
| 990 | DTW → CLE | 07:00–07:38 | X7 | D95 | 2 | 2/3 |
| 998 | DTW → CLE | 12:25–13:08 | X6 | CVR | 1 | 2/3 |
| 998 | DTW → CLE | 12:25–13:03 | 6 | DC9 | 1 | 2/3 |
| 740 | DTW → CLE | 16:20–16:58 | D | M80 | 2 | 2/3 |
| 756 | DTW → CLE | 19:30–20:13 | X6 | CVR | 2 | 2/3 |
| 726 | DTW → CLE | 20:30–21:05 | D | D9S | 2 | 2/3 |
| 605 | DTW → CMH | 09:30–10:18 | X7 | D9S | 2 | 2/3 |
| 992 | DTW → CMH | 12:25–13:18 | D | CVR | 2 | 2/3 |
| 776 | DTW → CMH | 16:15–17:03 | D | DC9 | 2 | 2/3 |
| 958 | DTW → CMH | 19:20–20:13 | D | CVR | 2 | 2/3 |
| 329 | DTW → CMH | 21:55–22:43 | X6 | D9S | 2 | 2/3 |
| 135 | DTW → CVG | 09:25–10:20 | D | D9S | 2 | 2/4 |
| 175 | DTW → CVG | 13:00–13:58 | D | DC9 | 2 | 2/4 |
| 721 | DTW → CVG | 17:00–17:51 | D | D9S | 3 | 2/4 |
| 541 | DTW → CVG | 19:25–20:23 | X6 | D95 | 2 | 2/4 |
| 694 | DTW → PIT | 09:50–10:48 | X7 | DC9 | 2 | 3/1 |
| 772 | DTW → PIT | 16:00–16:58 | D | 72S | 2 | 3/1 |
| 32 | DTW → PIT | 20:45–21:43 | D | D9S | 2 | 3/1 |
| 351 | LGA → MKE | 07:30–08:45 | X67 | D9S | 1 | 3/4 |
| 353 | LGA → MKE | 11:59–13:10 | D | D9S | 2 | 3/4 |
| 357 | LGA → MKE | 16:20–17:35 | D | D95 | 3 | 3/4 |
| 350 | MKE → LGA | 09:10–11:55 | X6 | D9S | 1 | 5/1 |
| 352 | MKE → LGA | 15:35–18:20 | D | D9S | 3 | 5/1 |
| 354 | MKE → LGA | 20:00–22:45 | X6 | D9S | 2 | 5/1 |
| 545 | MSN → ORD | 19:50–20:27 | X6 | D9S | 2 | 5/2 |
| 390 | ORD → BNA | 07:20–08:35 | X7 | D9S | 2 | 5/4 |
| 278 | ORD → BNA | 11:00–12:15 | D | D9S | 2 | 5/4 |
| 54 | ORD → BNA | 16:15–17:30 | D | D95 | 3 | 5/4 |
| 657 | PIT → DTW | 07:45–08:40 | X7 | DC9 | 2 | 6/1 |
| 651 | PIT → DTW | 11:25–12:20 | D | DC9 | 2 | 6/1 |
| 751 | PIT → DTW | 17:35–18:30 | D | 72S | 2 | 6/1 |

## Regional prioritet och kvarstående arbete

Centralamerika/Karibien, samma **31 forskningsområden**, ligger kvar på **532 rörelser, 158 riktade par, 38 flygplatser och 64 inrikesrörelser**. **St. Thomas 14, Tortola 8 och Nicaragua 0** är oförändrade. Mexiko har 20; Mellanösternurvalet 72. Noll betyder en datalucka, inte frånvaro av historisk trafik.

**Ingen ny regional arkivsökning gjordes denna omgång.** `regional_source_search_batch138.json` dokumenterar återanvändningen av Republic-originalet. Prioriteten Nicaragua/St. Thomas och tidigare spår efter Aero Virgin Islands 15 december 1985, Gull Air 15 februari 1986, LACSA, Caribbean Express, Eastern, American, BWIA, TACA, Aviateca, TAN SAHSA och sjöflygets tider/hamnar kvarstår. Tidigare LIAT-, Challenge-, Arrow-, Airways International-, Air France F27-, PBA789- och specialflygsfrågor bevaras. Inga nya militär-, privat- eller specialflyg importeras.

För Republic återstår andra huvudlinjer, separat operatörsattribuerad Express-trafik, bland annat SFO–SMF/SMF–SFO som separata fysiska ben, ändringsblad och belägg för faktisk drift. Det tidigare öppna MSN–ORD545-benet är nu infört.

## Totalt, verifiering och installation

**11 347 rörelser: 11 346 planerade och en tidigare bekräftad. 6 733 katalogscheman, varav 6 712 granskade. 393 flygplatser/platser, 121 länder/territorier och 2 008 riktade par. 111 operatörsposter: 68 med rörelser, 43 utan. 94 källposter och 36 bidragande tidtabellsutgåvor.** Ingen verifierad global bolagsinventering eller färdigprocent finns.

**20 befintliga Python-tester och tre generatorkontroller passerar.** Samtliga **1 782 Republic-rörelser** har kontrollerats för dubbletter och överlappning av samma flygnummer; inga hittades. Äldre schema- och rörelseobjekt är oförändrade. ZIP-filens CRC och varje fils hash kontrolleras mot manifestet vid paketering.

**Unreal Editor och spelet har inte körts här.** Paketet är kumulativt: stäng Unreal och slå samman `Plugins/` och `Tools/` med projektroten bredvid `.uproject`. **PaintAirplane-rättelsen f23b73a måste redan vara kompilerad.** Ingen ny C++-kod ingår.
