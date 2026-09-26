# Verifieringsprotokoll — 25 september 2026

- 9 Python-tester för atlasdata, referenser, koordinatintervall, historiska urval, landprofiler, separata aktörspunkter/mottagare, datumkänsliga befattningar, aktuella svensk/engelsk-texter, källor och UFS-paketeringsregler: godkända.
- 23 befintliga Python-tester för lokaliseringsramverket: godkända.
- Faktiska `TMOPGlobeMath.h` kompilerad med g++ C++17 mot en tillfällig minimal matematikadapter: 30 vycentreringar, bortklippt baksida, öst/nord-riktning, datumlinje och antipodinterpolation godkända. Detta kontrollerar matematiken, inte Unreals renderingsimplementation.
- `git diff --check`: godkänd.
- UE 5.8 API-signaturer för FPreviewScene, ConstructionValues och Slate-linjer kontrollerade mot Epic Games dokumentation; MakeLines använder FVector2f.
- Två Unreal Automation-testfall finns under `TMOP.WorldAtlas`, men har **inte körts** eftersom Unreal saknas i arbetsmiljön.
- UHT, Unreal Build Tool, shaderkompilering, visuell speltest, handkontroll, flera lokala spelare och paketerad körning: **inte verifierade**. Följ acceptansstegen i README.
- Utökad data: 54 poster, 33 länder och 2 Nicaraguaaktörer. Regering och ministerurval finns för alla länder; sex landsposter saknar säkert daterade tjänstechefer och är markerade. Se COUNTRY_COVERAGE.md.
- De tidigare matematik-/API-kontrollerna ovan gäller grundatlasen. Denna utökning har inte kompilerats i Unreal; aktörernas ritning/träfftest har granskats i koden men inte visuellt provkörts.

## Konfliktutökningen

- 12 atlasdatatester och 23 lokaliseringstester: **godkända** (35 totalt).
- Den faktiska `FTMOPAtlasEntry::Visible`-funktionen extraherades och kompilerades med g++ C++17 mot en minimal FString-adapter: **15 assertions godkända**, inklusive ±90/±91 dagar, inkluderande slutdatum, separat senarefilter och oförändrad flödeshistorik. Det är inte en Unreal-kompilering.
- UE Automation-testet DatesAndData har utökats med samma datumgränser och verkliga närtidsposter. **Ej kört i Unreal.**
- Data: 99 poster totalt, varav 53 konflikt-/krisposter: 45 pågående perioder, sex avslutade och två kommande inom närtidsfönstret. Delstrider får inte räknas som separata krig.
- Alla nya textfält har svenska, engelska och aktuell `en_source`. Aktörspunkter och relationer valideras; civila i Gukurahundi har inte dubbelriktad stridspil.
- Ritning och träfftest delar `LinkPoint`, samma sfärprojektion, skärmförskjutning och bortklippning av baksidan. Kodgranskad; **inte visuellt verifierad i Unreal**.
- Källor/avgränsning och kategoriinventering: `CONFLICT_COVERAGE.md`. Detta är ett redaktionellt urval, inte verifierad fullständighet för alla globala våldshändelser.

## Forskningsleverans 26 september 2026

- Totalt 134 poster; 22 nytillagda. Alla textfält har svenska, engelska och aktuell en_source.
- 12 atlasdatatester och 23 lokaliseringstester godkända (35 totalt).
- Nya referenser, datum, källfält och tvåvägslänkar validerade; senare-1986-posterna omfattar Defex-periodöversikten och provhanteringen den 1 mars.
- Ingen atlas-C++ har ändrats i just denna forskningsuppdatering. Paketet inkluderar tidigare kodfixar.
- git diff --check: godkänd. Unreal-kompilering, visuell körning och paketering har inte utförts här.
