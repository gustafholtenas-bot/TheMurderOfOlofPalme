# Polisernas och polisbilarnas startpunkter – kontrollunderlag

Detta är kontrollunderlaget för spelintervallet 23:00–23:45 den
28 februari 1986. En startpunkt är inte automatiskt historiskt belagd:

- **Belagd** = plats och ungefärlig tid stöds av angiven källa.
- **Rekonstruerad** = praktisk infart eller simuleringstid som behöver godkännas.
- **Okänd** = ska inte aktiveras i Unreal förrän uppgiften kompletterats.

Alla personer i samma bil är tänkta att börja i bilen om inget annat anges.
`bSpawnInSimulation` är fortfarande `false` för grundrader utan färdig
Blueprint, sätesplacering och verifierad tidslinje.

## Fordon och gemensam startpunkt

| Fordon | Personer | Startpunkt/tid som nu används eller föreslås | Status |
|---|---|---|---|
| Pikét 3230 | Kjell Östling, Leif Svensson, Claes Djurfeldt, Peter Wikström, Jan Hermansson, Klas Gedda | 23:00 vid VD1/Södermalm, utanför spelområdet. Kör mot city cirka 23:08. Första tydliga citypunkter: Regeringsgatan cirka 23:15 och området David Bagares gata/Malmskillnadsgatan cirka 23:20–23:23. | Delvis belagd. Exakt transform 23:00 måste kontrolleras. |
| Pikét 1230 | Christer Persson, Kent Bäcklund, Ulf Hellman, Lena Löhr, Per Borg | Exakt plats 23:00 okänd. Belagd kontrollpunkt: Norra Bantorget cirka 23:21; därefter mot mordplatsen. | Start 23:00 okänd; Norra Bantorget belagd. |
| RB 2520 | Gösta Söderström, Ingvar Widén | Första belagda spelpunkt cirka 23:28, öster om Sveavägen på Kungsgatan, körande västerut. | Belagd från cirka 23:28; tidigare patrulläge okänt. |
| RB 1210 | Lars Christiansson, Hans-Erik Rehnstam | USA:s ambassad cirka 23:20–23:22. Därefter via Norrmalmstorg mot Malmskillnadsgatan/Brunnsgatan och sökområdet. | Belagd. |
| RB 1170 | Anders Pettersson, Mats Eriksson | Grand Hôtel cirka 23:24; därefter Kungsträdgårdsgatan–Hamngatan och vidare mot Snickarbacken. | Belagd från cirka 23:24; plats 23:00 okänd. |
| Civil 2733 | Pontus Wikner, Erik Sundin | Stureplan när larmet tas emot; därefter Kungsgatan västerut och förbi mordplatsen. Normaliserad KMZ innehåller en tidsförskjutning: objektets klockslag 23:28:45 men etiketten anger cirka 23:23. | Plats belagd, exakt tid måste rättas/kontrolleras. |
| RB 1520 | Christian Dalsgaard; Thomas Ekesäter ska fortfarande läggas till | Kungsträdgårdsgatan/Hamngatan cirka 23:21 när larmet går. | Belagd för Dalsgaard. |
| RB 1227 / civil Volvo eller Saab | Per Hall, Göran Stigson | Norrmalms polisstation cirka 23:25. Därefter Mäster Samuelsgatan och kontroll vid Regeringsgatan/Malmskillnadsgatan, sedan Kareliaområdet. | Belagd rutt; förare och exakt bilnummer/modell okända. |
| Holmér/Dahlgrens civila tjänstebil | Rolf Dahlgren (förare), Hans Holmér | Rekonstruerad spawn 23:28 vid `EnterSveavagenS_Car`; norrut på Sveavägen, långsam passage utan stopp, despawn cirka 23:30 vid `ExitSveavagenN_Car`. | Omstridd händelse och rekonstruerad tid/rutt. |
| Livvaktsbil Sund/Andersson | Göran Sund, Dan Andersson | Historisk utgångspunkt: USA:s ambassad. Föreslagen spelinfart cirka 23:40 från söder på Birger Jarlsgatan vid `KungsXBirgerJarl_SE`; därefter Kungsgatan västerut och Sveavägen norrut. Ankomst mordplatsen 23:40–23:45. | Ambassad och ankomstintervall belagda i D21659-14; infart, exakt sekund, bilmodell och förare rekonstruerade/okända. |

## Individuell kontrollista

| Polis | Bil/grupp | Startpunkt som ska kontrolleras |
|---|---|---|
| Gösta Söderström | RB 2520 | I RB 2520 på Kungsgatan öster om Sveavägen cirka 23:28. |
| Ingvar Widén | RB 2520 | I RB 2520 på Kungsgatan öster om Sveavägen cirka 23:28. |
| Lars Christiansson | RB 1210 | I RB 1210 vid USA:s ambassad cirka 23:20–23:22. |
| Hans-Erik Rehnstam | RB 1210 | I RB 1210 vid USA:s ambassad cirka 23:20–23:22. |
| Anders Pettersson | RB 1170 | I RB 1170 vid Grand Hôtel cirka 23:24. |
| Mats Eriksson | RB 1170 | I RB 1170 vid Grand Hôtel cirka 23:24. |
| Pontus Wikner | Civil 2733 | I civil 2733 vid Stureplan; exakt tid behöver korrigeras. |
| Erik Sundin | Civil 2733 | I civil 2733 vid Stureplan; exakt tid behöver korrigeras. |
| Kjell Östling | Pikét 3230 | I 3230 vid VD1/Södermalm 23:00. |
| Leif Svensson | Pikét 3230 | Förare i 3230 vid VD1/Södermalm 23:00. |
| Claes Djurfeldt | Pikét 3230 | I 3230 vid VD1/Södermalm 23:00; lämnar tillfälligt bussen för att flytta sin privata VW cirka 23:15–23:18. |
| Peter Wikström | Pikét 3230 | I 3230 vid VD1/Södermalm 23:00. |
| Jan Hermansson | Pikét 3230 | I 3230 vid VD1/Södermalm 23:00. |
| Klas Gedda | Pikét 3230 | I 3230 vid VD1/Södermalm 23:00; denna punkt finns uttryckligen i KMZ-materialet. |
| Christer Persson | Pikét 1230 | Exakt 23:00-punkt okänd; i/vid 1230 på Norra Bantorget cirka 23:21. |
| Kent Bäcklund | Pikét 1230 | Exakt 23:00-punkt okänd; följer 1230 till Norra Bantorget. |
| Ulf Hellman | Pikét 1230 | Exakt 23:00-punkt okänd; följer 1230 till Norra Bantorget. |
| Lena Löhr | Pikét 1230 | Exakt 23:00-punkt okänd; följer 1230 till Norra Bantorget. |
| Per Borg | Pikét 1230 | Kopplad till 1230 enligt tjänstgöringslistan; separat KMZ-spår börjar på väg mot mordplatsen cirka 23:25:40 och måste samordnas med 1230-spåret. |
| Christian Dalsgaard | RB 1520 | I RB 1520 vid Kungsträdgårdsgatan/Hamngatan cirka 23:21. |
| Per Hall | RB 1227 | Vid Norrmalms polisstation cirka 23:25, därefter i civil Volvo eller Saab. |
| Göran Stigson | RB 1227 | Vid Norrmalms polisstation cirka 23:25, därefter i civil Volvo eller Saab. |
| Hans Holmér | Holmér/Dahlgren | Passagerare i rekonstruerad bilspawn på södra Sveavägen 23:28. Omstritt. |
| Rolf Dahlgren | Holmér/Dahlgren | Förare vid rekonstruerad bilspawn på södra Sveavägen 23:28. Omstritt. |
| Göran Sund | Livvaktsbil Sund/Andersson | I gemensam bil från USA:s ambassad; föreslagen spelinfart södra Birger Jarlsgatan cirka 23:40. |
| Dan Andersson | Livvaktsbil Sund/Andersson | I gemensam bil från USA:s ambassad; föreslagen spelinfart södra Birger Jarlsgatan cirka 23:40. |

## Punkter som behöver ditt besked

1. Ska Sund/Anderssons bil spawnas exakt **23:40:00** vid
   `KungsXBirgerJarl_SE`, eller ska 23:40 vara deras ankomst till mordplatsen?
2. Vem kör Sund/Anderssons bil? D21659-14 anger inte detta.
3. Var befinner sig 1230 exakt klockan 23:00?
4. Ska 3230 verkligen börja vid VD1/Södermalm 23:00, eller ska den först
   spawnas när den kommer in i den spelbara kartan?
5. Civil 2733 har motstridiga tider mellan KMZ-klockslag och etikett. Vilken
   tidslinje ska styra?
6. Ska Per Borg följa hela 1230-tidslinjen eller sitt separata KMZ-spår?

