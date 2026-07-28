# TMOP Police Foundation

Importfilerna bygger vidare på de fullständiga DataTables som användaren
exporterade från Unreal. De innehåller:

- 26 nya polisrader i `DT_TMOP_People`.
- 10 nya polisfordonsrader i `DT_TMOP_HistoricalVehicles`.
- 11 nya patrull-/sökgrupper i `DT_TMOP_Groups`.

## Avsiktligt ofyllda fält

- `KnownDriverEntityId` är `None` tills föraren är källbelagd. Undantag:
  Leif Svensson är chaufför i 3230 och Rolf Dahlgren kör Holmérs bil.
- `VehicleClass` och `ModelData` är `None` tills rätt Blueprint/modell anges.
- `bSpawnInSimulation` är `false`.
- Timeline-arrayerna är tomma tills Blender-exporten
  `TMOP_Police_For_Unreal.json` har konverterats.
- Grupperna har `bCreateAtScenarioStart=false`; de ska skapas/upplösas vid
  rätt timeline-händelse så att gruppföljning inte konkurrerar med bilresan.

## 3230

Hela pikétstyrkan finns som `GROUP_POLICE_PIKET3230`. Två separata sökpar
finns också för Djurfeldt/Hermansson och Wikström/Gedda. Själva delningen
läggs senare in som timeline-actions när tidpunkten fastställts.

Tjänstgöringslistan kopplar Christer Persson, Kent Bäcklund, Ulf Hellman,
Lena Löhr och Per Borg till anrop `1230`. Detta styrker grupptillhörigheten,
men bevisar inte på egen hand vem som körde eller exakt vilka som satt i bilen
vid varje minut. `KnownDriverEntityId` och säten lämnas därför tomma.

Leif Svensson är kopplad som chaufför i 3230.

## RB 1227

Per Hall och Göran Stigson är kopplade till RB 1227. Stigsons förhör
bekräftar att de åkte tillsammans i en civil Volvo eller Saab, men anger
varken vem som körde eller bilens exakta nummer.

## Hans Holmér och Rolf Dahlgren

En preliminär civil tjänstebil har lagts in med Rolf Dahlgren som chaufför
och Hans Holmér som passagerare. Den rekonstruerade rutten startar cirka
23:28:00 vid `EnterSveavagenS_Car`, följer nordgående R1-körfält från
`SVEAVAGENN_001_R1` till `SVEAVAGENN_007_R1` och passerar mordplatsen
ungefär 23:28:30 utan stopp. Körhastigheten bör sättas till cirka 20–25 km/h.
Vid cirka 23:30 når bilen `ExitSveavagenN_Car` och despawnas utanför
spelplanen. Bilmodell och Blueprint måste fortfarande anges innan bilen kan
spawnas.

## Göran Sund och Dan Andersson

Göran Sund och Dan Andersson har lagts in som en gemensam livvaktsgrupp och
kopplats till `VEHICLE_POLICE_LIVVAKTER_SUND_ANDERSSON`. D21659-14 placerar
dem på skyddstjänst vid USA:s ambassad och anger att de kom tillsammans i bil
till mordplatsen cirka 23:40–23:45. Föreslagen spelinfart är Birger
Jarlsgatan från söder, därefter Kungsgatan västerut och Sveavägen norrut.
Exakt bilmodell, förare, säten och startsekund är inte belagda. Fordons- och
persontidslinjerna lämnas därför inaktiva tills kontrollpunkterna i
`TMOP_POLIS_STARTPUNKTER_KONTROLL.md` är besvarade.

## Källkomplettering för 3230

Claes Djurfeldts uppgifter A14205-02-A anger att 3230 var en ny Ford
Econoline. Kjell Östling var befäl och Leif Svensson förare. Övriga i bussen
var Claes Djurfeldt, Peter Wikström, Jan Hermansson och Klas Gedda.

Importera inte dessa filer innan de kompletterande fordonsuppgifterna har
lagts in om du vill göra allt i en enda Unreal-import. De är annars säkra
grundversioner: befintliga rader är bevarade och de nya raderna spawna inte.
