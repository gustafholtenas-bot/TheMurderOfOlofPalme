# Installation – världsoperationer 2026-09-26

Detta är en kumulativ uppdatering av tidigare Bofors- och CIA-paket. Senaste world.json innehåller 224 poster, inklusive 22 nya från operationsgenomgången. Äldre rapporters totalsiffror beskriver deras tidigare versioner.

1. Säkerhetskopiera projektets befintliga world.json om du gjort egna ändringar.
2. Stäng Unreal Editor. Packa upp ZIP-filen i projektroten så att Plugins och Tools hamnar bredvid motsvarande befintliga mappar. Ersätt de medföljande filerna.
3. Paketet innehåller också den tidigare C2660-korrigerade STMOPWorldAtlas.cpp. Bygg om projektets Editor-target efter att C++-filen ersatts.
4. Öppna projektet och Grupperingar i världen. Atlasen läser world.json; ingen DataTable-import behövs. De nya händelseposterna nås i Grupper och aktörer och via relaterade poster för länder och konflikter.
5. Kontrollera svenska/engelska, relaterade länkar och poster med senare datum. Vid användning av datumfilter ska senare uppgifter inte behandlas som kända på morddagen.

Detta är ett uppdateringspaket för det befintliga projektet, inte ett fristående Unreal-projekt. Egna ändringar i world.json måste sammanfogas före ersättning.

Källor och granskningsnoteringar finns i SOURCES/OPERATIONS_REVIEW_2026-09-26.md och .json. 51 automatiska tester och hierarkivalideringen går igenom. Ingen provkörning eller kompilering i Unreal har utförts här.
