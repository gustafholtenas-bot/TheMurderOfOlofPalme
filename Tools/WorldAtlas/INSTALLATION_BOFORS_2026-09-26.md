# Bofors och dokument från morddagen – innehållsuppdatering

Förutsätter att den tidigare världsatlasen finns i projektet. Paketet innehåller inte hela spelet eller atlasens grundinstallation.

1. Stäng spelet och Unreal Editor.
2. Packa upp ZIP-filen i projektroten, där projektets `.uproject` ligger. Slå ihop mapparna `Plugins/` och `Tools/`.
3. Ersätt `Plugins/TMOPEngine/Content/WorldAtlas/world.json` och `Plugins/TMOPEngine/Source/TMOPEngine/Private/WorldAtlas/STMOPWorldAtlas.cpp` med filerna i paketet. C++-filen innehåller rättningen av kompileringsfelet C2660, där Slate tidigare anropades med sex obligatoriska argument i stället för fem.
4. Starta igen och öppna **Grupperingar i världen → Vapen och finansiering → Bofors och Nobelkrut · exportfall**.
5. De nya dokumenten nås även från USA/Sydafrika och under **Grupper och aktörer**. Sök på **28 februari**.
6. För Indienavtalet och den senare FH 77-granskningen: slå på **Visa även senare 1986 (efter morddatumet)**.

Ingen DataTable-import eller ändring av `en.catalog.json` behövs. Bygg om projektet i Unreal efter att ha bytt C++-filen. Atlasens svenska och engelska text ligger direkt i `world.json` och följer språkvalet. Ett redan paketerat spel behöver paketeras om för att få med innehållet och kodfixen.

## Innehåll

- Hela aktuella `world.json`: 193 poster, inklusive allt tidigare atlasinnehåll.
- 34 exportfall/samlingsposter för Bofors och Nobelkrut samt två Robot 70-ruttgrenar.
- Tre lästa samtida amerikanska originalhandlingar från 28 februari 1986, ett sydafrikanskt dokument från samma datum som återges i en senare inlaga, samt en separat CIA-analys från 1988.
- Källförteckning, avgränsningar och ändringsindex under `Tools/WorldAtlas/SOURCES/BOFORS_RESEARCH_2026-09-26.*`.
- Uppdaterad datavalidering och test för leveransstatus, osäkra slutled, mängder och dokumentdatum.
- Den tidigare rättningen för Slate C2660: `DrawCoastlines` skickas som namngivet Slate-argument, medan konstruktorn har fem obligatoriska argument.

Detta är samtliga identifierade fall i det granskade underlaget, inte en komplett journal över världens alla Boforsleveranser. Ingen ny Boforsleverans exakt den 28 februari är fastställd här.

## Kontroll

28 atlasprov och 23 lokaliseringsprov godkända (51 totalt). Alla 33 hierarkiträd och flaggreferenser validerade. Alla 138 tidigare post-ID:n bevarade. Kodraden har kontrollerats mot Slate-anropets antal argument, men Unreal-kompilering, editorstart och visuell spelkontroll har inte kunnat utföras i arbetsmiljön.
