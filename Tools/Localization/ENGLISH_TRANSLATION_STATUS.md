# Engelsk översättning – status 2026-09-25

Detta är en första översättningsomgång av den exporterade textkatalogen, inte en
färdiggranskad eller speltestad fullständig engelsk utgåva.

Katalogen innehåller 51,201 textposter: 3,093 språkgranskade, 45,757 maskinöversatta utkast och 2,351 poster med upptäckta granskningsproblem. 51,200 poster har en översättning eller ett sparat förslag; 1 OCR-skadad post saknar användbart översättningsförslag. Siffrorna räknar fält och kontexter, inte unika meningar.

`TranslationTemplates/en.catalog.json` är arbetsfilen med svensk originaltext,
engelsk översättning, stabila nycklar, källhash och granskningsstatus.
`Content/Localization/TMOP/en.json` är den kompilerade språkfilen som spelet läser.
Den innehåller bara aktiva översättningar för `reviewed`; övriga poster behåller
svenskan i spelet. Kopiera inte den stora arbetskatalogen direkt till runtime.

| Textområde | Granskade | Utkast | Behöver kontroll |
| --- | ---: | ---: | ---: |
| C++ UI (TMOP) | 544 | 0 | 0 |
| DT_TMOP_AddressRegistry | 2125 | 0 | 0 |
| DT_TMOP_AfterMurderEvents | 143 | 0 | 0 |
| DT_TMOP_Groups | 0 | 233 | 1 |
| DT_TMOP_HistoricalEvents | 95 | 0 | 0 |
| DT_TMOP_HistoricalVehicles | 4 | 259 | 0 |
| DT_TMOP_IntroCards | 12 | 0 | 0 |
| DT_TMOP_ItemMeshes | 35 | 0 | 0 |
| DT_TMOP_MetroArrivals_Northbound | 14 | 0 | 0 |
| DT_TMOP_MurderDayMysteries | 26 | 0 | 0 |
| DT_TMOP_MurderKnowledge | 50 | 0 | 0 |
| DT_TMOP_ObservationLinks | 0 | 23 | 0 |
| DT_TMOP_Observations | 0 | 1811 | 24 |
| DT_TMOP_People | 17 | 4356 | 537 |
| DT_TMOP_RecordedCalls | 28 | 0 | 0 |
| DT_TMOP_Uppslag_REGISTER | 0 | 39075 | 1789 |

## Urval och kvalitet

De senaste MM_DD-exporterna väljs separat för varje tabell. Se `en.coverage.json`
för exakta sökvägar och SHA-256-hashar. Årtal ingår inte i mappnamnen.
People och HistoricalVehicles omfattar endast datamässiga spawnkandidater:
796 personer och 130 fordon. Detta är inte en avläst lista över aktörer i en
viss spelomgång. Övriga valda tabeller omfattas i sin helhet inom fälturvalet.

Alla exporterade TMOP-nycklar från C++ har granskad engelska. Dessutom är intro,
föremålsnamn, historiska händelser, händelser efter mordet, vetskaper, mysterier,
samtalens exporterade textfält, våningsetiketter och tunnelbanans källnotiser
språkgranskade. Tidigare pilotöversättningar har behållits.
Språkgranskning betyder att översättningen jämförts med originalets formulering;
det är ingen ny faktagranskning av historiska uppgifter eller källhänvisningar.
Originalets osäkerhet och uppgiftskedjor har behållits.

De stora textmängderna har maskinöversatts lokalt med Helsinki-NLP opus-mt-sv-en,
i CTranslate2-format från `gaudi/opus-mt-sv-en-ctranslate2`.
Ingen tabelltext har skickats till en extern översättningstjänst.
Modellvikter behöver inte installeras i spelet och ingår inte i paketet.
Utkast kan innehålla felaktig terminologi, tappade betydelsenyanser eller
felöversättningar trots godkända tekniska kontroller. De är inte märkta reviewed.

`en.review_issues.json` listar automatiskt upptäckta problem. I arbetskatalogen
finns förslaget kvar som `machine_candidate`; `translation` är då tom och status
`needs_review`. Flaggorna omfattar tappade skyddade ord/nummer, formatvariabler,
upprepning, längdgräns och misstänkt OCR. OCR-kontrollen är en heuristik som även
kan reagera på korrekta datumrika texter. Den hittar inte all trasig OCR.
Korrigera oläsbar svensk källa innan du godkänner en översättning av den.

## Vad som återstår innan en fullständig utgåva

- Språkgranska utredningsuppslag, grupper och längre person-, fordons- och
  observationstexter; rätta maskinöversättningsfel och lösa markerade problem.
- Kontrollera synliga texter som saknas i exporterade fält, samt delade dialoger
  som ägs av icke-spawnande personer. Fälturvalet räknar inte ut dessa beroenden.
- Kontrollera Blueprint/UMG, namn på faktiskt importerade DataTable-assets och
  text inbakad i tidningsbilder, skyltar, video och ljud. Dessa har inte översatts.
- Bygg och provkör i Unreal. Ingen Unreal-installation finns i denna arbetsmiljö.
  FText byter språk, men paneler som cachar sammansatta FString kan behöva öppnas om.
- Ursprungliga svensktexter, personnamn, ID:n, rutter och spelardata ska bevaras.
  Spelarens egna teorititlar och anteckningar ska inte maskinöversättas.

## Installation och provning

1. Stäng Unreal Editor. Paketet är kumulativt mot Git-commit
   `605bdbf96ebf784765c6c37ceecad5ff8f675dc0`. Om din lokala kod ändrats därefter,
   jämför och slå ihop ändringarna; skriv inte blint över nyare arbete.
2. Installera de medföljande Config-, Plugins-, Content-, Tools- och
   TranslationTemplates-filerna med sina relativa sökvägar. Tillhörande headers
   ingår, inklusive `TMOPVehicleBase.h`. Blanda inte gamla headers med nya cpp-filer.
3. Om du har ett gammalt `Saved/LanguagePacks/en.json`, säkerhetskopiera och flytta
   undan det under provningen. Det har företräde framför den nya Content-filen
   och ett gammalt pilotpaket kan annars återställa nya tabellöversättningar till svenska.
4. Gör en vanlig Development Editor-build; använd inte enbart Live Coding.
5. Starta om processen, välj English under Language och kontrollera startmeny,
   inställningar, intro, anteckningsbok och personinspektör. Byt tillbaka till
   Svenska och öppna panelerna igen. Svenskan är fortfarande standard.
6. I paketerat spel ska Content/Localization/TMOP följa med som UFS enligt den
   medföljande inställningen. Kör även ett paketerat test, inte bara PIE.

## Fortsatt granskning och ändrade svenska tabeller

Ändra `translation` på rätt post i arbetskatalogen. När översättningen jämförts
med aktuell svensk källa, sätt `status` till `reviewed`. Ta bort inaktuella
`review_issues` och `machine_candidate` på den posten. Ändra inte nyckel, ID eller
källhash för att få en gammal översättning att passa en ny svensk text.

```text
python Tools/Localization/tmop_localization.py validate TranslationTemplates/en.catalog.json
python Tools/Localization/tmop_localization.py compile TranslationTemplates/en.catalog.json --output Content/Localization/TMOP/en.json
```

Efter ändringar i de svenska tabellerna:

```text
python Tools/Localization/tmop_localization.py export --table-list Tools/Localization/current_tables.json --latest-dated --spawn-only --merge TranslationTemplates/en.catalog.json --output TranslationTemplates/en.updated.json --audit TranslationTemplates/selection_audit.json
```

Oförändrad källa behåller översättning och status. Ändrad källa blir needs_review
med tidigare källa/översättning sparad. Granska `en.updated.json` innan den ersätter
arbetskatalogen. Starta om spelet efter byte av JSON-fil.

## Genomförda kontroller

23 Python-tester godkända. Hela katalogen och runtime-filen validerade, inklusive
källhashar, unika nycklar och formatvariabler. Den kompilerade filen understiger
runtime-gränsen 32 MiB och släpper inte igenom utkast. Länkar och uppslagsnummer
i de granskade tabellöversättningarna är oförändrade. C++ har inspekterats och
includeordning kontrollerats; Unreal-kompilering och speltest är inte utförda.
