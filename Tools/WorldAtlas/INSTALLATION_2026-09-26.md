# Installera forskningsuppdateringen

Paketet har projektets mappstruktur direkt i ZIP-roten: `Plugins/` och `Tools/`.

## Om världskartan redan fungerar

1. Stäng spelet/Unreal Editor.
2. Ersätt `Plugins/TMOPEngine/Content/WorldAtlas/world.json` med filen i ZIP-paketet.
3. Kopiera gärna `Tools/WorldAtlas/` för källor och dokumentation.
4. Starta igen och öppna Grupperingar i världen. Ingen DataTable-import behövs. Atlasens texter följer språkvalet svenska/engelska.

De 22 nya posterna nås genom relaterade länder, konflikter och The Enterprise. Händelser finns även i grupperingslistan; vapen och betalningar under flöden. Provhanteringen den 1 mars och Defex-översikten kräver ”Visa även senare 1986”.

Enbart JSON-uppdateringen kräver ingen C++-kompilering. Ett redan paketerat spel måste paketeras om för att få med den nya innehållsfilen.

## Om tidigare atlasfixar saknas

Paketet innehåller även atlasens aktuella kod och integrationsfiler från arbetsgrenen. Det är ett tillägg till ett befintligt TMOP-projekt med lokaliseringsramverket, inte ett komplett spelprojekt. Säkerhetskopiera egna nyare kodändringar och jämför innan du ersätter kodfiler. Kopiera `Plugins/` till projektroten, slå ihop mapparna och bygg projektets Development Editor-mål med Unreal stängt. Se README.md för grundinstallation.

## Innehåll och kontroll

- Hela `world.json`: 134 poster, inklusive tidigare tidnings-, Kurdistan-, North- och Sundqvistposter.
- 22 nya daterade poster från de senaste forskningsomgångarna, samtliga svenska/engelska.
- Atlasens kod, kustlinjer, källregister, läsloggar och befintliga valideringstester.
- Nya uppgifter skiljer originalhandlingar, senare utredningar, pressuppgifter och obekräftade tolkningar åt.
- Data- och översättningskontroller körda. Unreal-kompilering och visuell speltest har inte kunnat köras här.

Se SOURCES/MURDER_DAY_RESEARCH_2026-09-26.md för de nya posterna.
