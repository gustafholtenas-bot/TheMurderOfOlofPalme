# Världsgrupperingar: interaktiv jordglob

Implementerad i TMOPEngine för den befintliga C++-pausmenyn. Grunddatum är **28 februari 1986**.

## Installera och öppna

1. Stäng Unreal Editor. Kopiera paketets `Plugins` och `Tools` till projektroten och slå ihop mapparna. Paketet innehåller pluginens kompletta källkod från denna arbetsgren, inklusive det tidigare lokaliseringsramverket (`cbdf13d`) och den nya atlasen. Övriga spelassets och tabellöversättningar ingår inte. Jämför och slå ihop eventuella egna nyare ändringar innan du ersätter filer.
2. Generera projektfiler vid behov och bygg projektets **Development Editor**-mål. Gör en full ombyggnad, inte enbart Live Coding, eftersom widgetens UPROPERTY-fält och konstruktor har ändrats.
3. Öppna spelet, pausa och välj **Grupperingar i världen**. Ingen ny Blueprint-widget, aktör i spelkartan eller editorimport av tabeller krävs.
4. Standardgloben använder Unreals sfär med kustlinjer. I pauswidgetens Blueprint, under **Class Defaults → TMOP → UI → Pause → World Atlas**, kan du tilldela **World Globe Mesh**, **World Globe Material**, **World Globe Alignment** och **World Globe Coastlines**. Använder projektet C++-klassen direkt fungerar standardgloben utan detta steg. Skapa vid behov en Blueprint-underklass och använd den där projektet väljer pauswidgetklass.
5. Den fotograferade modellen har inte funnits som mesh-/materialfil i underlaget. Importera den separat. Modellen ska vara en sfär; materialet ska vara ett vanligt Surface-material, helst Unlit för jämn kartläsbarhet. Materialoverride påverkar materialplats 0. Meshens egna övriga material behålls.

**Kompilering och körning i Unreal är inte verifierade i leveransmiljön.** Tester som kan köras utan Unreal samt lämpliga UE-testfall finns nedan. Ingen GitHub-push har gjorts.

## Hur det fungerar

- `FTMOPGlobeScene` skapar en separat `FPreviewScene` från runtime-modulen Engine, en StaticMesh-komponent, en ortografisk SceneCapture2D och en render target på 1024×1024. Inga editor-only-moduler behövs.
- Sfären skalas till radie 100 och meshens bounds-centrum flyttas till origo. Sfären behöver ha samma radie längs alla axlar. Kamera: `(400,0,0)`, rotation `(0,180,0)`, ortografisk bredd 240.
- Geografisk punkt: `x=cos(lat)*cos(lon)`, `y=-cos(lat)*sin(lon)`, `z=sin(lat)`. Minustecknet anpassar öst till Unreals kamerakoordinater. Latitud/longitud anges i grader, nord/öst positiva.
- Samma quaternion roterar sfären och markörernas 3D-positioner. Projektionen är `screenX=centerX-y*radius`, `screenY=centerY-z*radius`. Bara punkter med positivt kameravänt x visas eller kan väljas.
- Mesh Alignment korrigerar modellens poler/nollmeridian; markördata ändras inte. Kontrollera Greenwich, ekvatorn, Sverige och USA. Om texturen är spegelvänd måste UV/material korrigeras; en rotation kan inte korrigera en spegling.
- Vapen- och pengalinjer följer samplade storcirklar. Baksidan klipps bort, riktningspilar visar flödet. Pengar är streckade. Linjer är samband mellan representativa punkter, **inte dokumenterade transportvägar**.
- Musdrag roterar; mushjul zoomar. Klick väljer markör eller linje. När globen har fokus fungerar piltangenter/D-pad och plus/minus eller axelknappar för zoom. Vanliga knappar och listor fungerar med Slates fokusnavigering. Land och konflikt kan dela position; den översta markören väljs, båda nås alltid i listorna.
- Renderingen uppdateras vid rotation, inte via game-world tick. Den fungerar därför även under paus. Zoom ändrar bildens storlek. Varje öppen lokal spelares meny har egen vy; språkvalet följer spelets gemensamma språkval.
- När menyn stängs med `SetMenuVisible(false)`, byter sida eller släpper Slate-resurser förstörs preview-scenen. Inga Root-objekt eller permanenta spelvärldsaktörer skapas.

## Data och översättningar

`Plugins/TMOPEngine/Content/WorldAtlas/world.json` är innehållsfilen. JSON-filerna följer med paketerade byggen genom Build.cs/UFS. Starta om menyn efter filändringar i editorkörning; efter paketering krävs ett nytt paket.

99 poster: 33 länder, 2 konfliktaktörer i Nicaragua, 3 nätverk, 53 konflikt-/krisposter, 2 möten och 6 vapen-/finansieringsposter. Se `CONFLICT_COVERAGE.md` för urvalet, tidsgränser och överlappande delstrider. Alla länder har nu regering/styrande parti, ett urval centrala ministrar och avsnitt för underrättelseverksamhet. 27 landsposter har namngivna tjänstechefer eller politiskt ansvariga säkerhetsministrar; sex har uttryckligen markerade luckor. Detta är urval, inte kompletta ministerier eller personalregister. Se `COUNTRY_COVERAGE.md`. Ofärdiga allians-/relationsfält är fortfarande märkta som forskningsposter. Detta är inte en fullständig geopolitisk databas eller en förteckning över misstänkta. Poster kan redigeras utan C++-ändring.

En post har:

| Fält | Betydelse |
|---|---|
| `id` | Stabilt unikt ID. Byt inte ID när texten ändras. |
| `kind` | `country`, `actor`, `group`, `conflict`, `arms`, `funds` eller `event`. |
| `lat`, `lon` | Representativ punkt. Inte en gräns eller ett territorium. |
| `country` | För `actor`: befintligt land-ID, exempelvis `ni`. |
| `marker_offset` | Valfri `[x,y]` i Slate-enheter, högst ±80. Förskjutning med ledarlinje, inte ändrad geografi. Samma förskjutning används för ritning och träfftest. |
| `profile_as_of` | Redaktionellt datum för profilens fakta: 1986-02-28. Uppdateras inte när senare historik aktiveras. |
| `intelligence_coverage` | `named_selection` eller `date_verification_pending`; redaktionell täckningsstatus, inte garanti om fullständighet. |
| `marker` | Om posten har en geografisk markör. Nätverk har ingen påhittad huvudort. |
| `from`, `to` | ISO-datum för filtrering. Krävs för konflikter; kan vara tomma för andra posttyper. |
| `date_precision` | För konflikter: `day`, `month` eller `year`; periodtexten anger faktisk precision. |
| `participants` | Konfliktens lokala aktörer: unikt `id`, textfältets `label`, `lat/lon`, samt `offset: [x,y]`, högst ±80. Ingen ny landprofil krävs. |
| `links` | Konfliktens relationer: `from`, `to` refererar till lokala aktörs-ID:n; `kind` är `opposition`, `support` eller `violence`. |
| `parent_conflict` | Redaktionellt ID för överordnad konflikt. Relaterade länkar ger navigation i båda riktningar. |
| `later_only` | Kräver ”Visa även senare 1986”, även när källan endast anger helåret. |
| `text` | Namngivna flerspråkiga textfält, se exempel. |
| `related` | Relaterade post-ID:n, visas som knappar; betyder inte automatiskt allierad/medlem. |
| `route` | Ordnade geografiska post-ID:n för flödets riktning. Endast vapen/pengar. |
| `sources` | Titel, HTTPS-URL och publiceringsuppgift. Valfria `fields` anger redaktionellt vilka avsnitt källan avser; `checked` är granskningsdatum och ska inte förväxlas med `published`. Tom publiceringsuppgift betyder inte att källan publicerades 1986. |

Textfält: `title`, `period`, `leaders`, `government`, `ministers`, `intelligence`, `personnel`, `research`, `map_note`, `body`, `allies`, `opponents`, `parties`, `legal`, `evidence`, `investigation`. Tomma/utelämnade fält visas inte. För country ska ofärdiga fält uttryckligen markeras, inte fyllas med antaganden.

```json
"body": {
  "sv": "Svensk originaltext.",
  "en": "English translation.",
  "en_source": "Svensk originaltext."
}
```

`en_source` är den svenska text som översättningen granskades mot. Om `sv` ändras blir den gamla engelskan automatiskt inaktiv och svensk text visas tills `en` och `en_source` uppdaterats tillsammans. **Kopiera inte bara ny svensk text till en_source utan att granska översättningen.** Andra språk kan lagras på samma sätt (`de`, `de_source`), men för att välja ett nytt språk måste spelets centrala språklista också utökas.

Atlasen använder befintligt språkval och stöder även externa stabila översättningsnycklar via `FTMOPLocalization::TableText("TMOP_WorldAtlas", id, field, Swedish)`. De medföljande atlasöversättningarna ligger i `world.json`, inte i det äldre `en.catalog.json`. Menytexterna finns i spelets befintliga `TMOPMenuTranslations.inl`. Befintliga tabeller och deras kataloger förändras inte av denna leverans.

## Nicaraguas två aktörer

Landsposten `ni` behålls. `ni-fsln` och `ni-contras` visas i **Grupper och aktörer** och på ett eget lager **Aktörspunkter**. Deras punkter förskjuts 34 enheter åt olika håll och 30 upp/ned från samma geografiska punkt. Ledarlinjer visar ankaret. Därmed överlappar inte de två aktörernas träffytor med varandra eller den centrala landspunkten när den är centrerad. Det är inte två nya länder eller påstådda territorier. När ankaret är på globens baksida döljs även den förskjutna punkten.

Contras är ett samlingsnamn; den första uppdelningen förenklar flera fraktioner. UNO avser organisationen från 1985. Finansieringens mottagare är `ni-contras`. Portugalleveransens dokumenterade rutt slutar fortsatt i Honduras, med Contras som relaterad mottagare. Ingen ny transportsträcka har hittats på.

Nya aktörer kan läggas i JSON med unikt `id`, `kind: actor`, `country`, koordinater, texter, källor och ömsesidiga `related`-länkar. Lägg till `map_note` när punkten är schematisk. Datumfälten fungerar som för länder. En aktör kräver ett giltigt land-ID vid inläsningen.

## Datum och källkritik i visningen

- Utgångsläget visar historik till 1986-02-28. Det betyder inte att en tidigare vapenleverans inträffade på morddagen.
- Senarevalet utökar historikens slutdatum till 1986-12-31. Det är ett tillägg till grundläget, inte en komplett tidslinje med dag-för-dag-regimer.
- Konfliktlagret visar som standard **45 pågående konfliktperioder på morddatumet**. Närtidsvalet lägger till sex nyligen avslutade och två kommande händelser inom ±90 dagar. Det vanliga senare-1986-valet påverkar inte konfliktlagret. Alla konflikter har daterade filtergränser och angiven precision. Se `CONFLICT_COVERAGE.md`.
- För ett datum känt endast på månads-/årsnivå används periodens gränser i `from/to`, medan `period` uttrycker faktisk precision. Filtergränserna skrivs inte ut som påstådda exakta händelsedatum.
- Helårsöversikten över Iran–Contra-finansiering är `later_only`. Den kan inte framstå som en dokumenterad överföring den 28 februari.
- Källor publicerade 1987/1993 får belägga tidigare händelser. Gränssnittet är en retrospektiv faktavy, inte en simulering av vad en aktör redan visste den 28 februari 1986.
- Allians, samarbete, konflikt, hemlig leverans, olaglig export och smuggling ska inte likställas. Kategorin vapenhandel betyder inte att varje affär är rättsligt klassificerad som smuggling.
- The Enterprise är ett operativt nätverk, inte hela CIA. Bilderbergs deltagare är inte permanenta medlemmar. NATO:s 16 medlemsländer gäller 1986.

Historiska källänkar finns per post. Geografiska kustlinjer: Natural Earth 1:110m (public domain), distribuerat via `world-atlas@2.0.2/land-110m.json`. Inga nutida statsgränser används för 1986. Kustlinjer är förenklade och ska inte användas för exakta gränsfrågor.

Tekniska API-referenser: Epic Games, [FPreviewScene](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/FPreviewScene), [ConstructionValues](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/FPreviewScene/ConstructionValues).

## Verifiering

Utan Unreal:

```sh
python -m unittest discover -s Tools/WorldAtlas -p 'test_*.py'
python -m unittest discover -s Tools/Localization -p 'test_*.py'
```

I Unreal: öppna Session Frontend → Automation och kör **TMOP.WorldAtlas**. Tester omfattar centrering, öst/nord-riktning, bortklippt baksida, datumlinjen, antipoder, datainläsning och historikfilter.

Manuell acceptans i Development Editor och sedan paketerad Development-build:

1. Öppna World Groups under paus: sfär, kustlinjer och Israel visas; inga saknade JSON-filer.
2. Rotera till USA och tillbaka. Punkterna följer samma plats på globen. Klicka aldrig igenom till baksidan. Dra över en punkt utan att oavsiktligt välja den.
3. Välj NATO, Bilderberg, Enterprise och respektive konflikt/flöde i listan. Växla lager. Välj både överlappande land och konflikt via listan.
4. Kontrollera att Gleneagles och helårsfinansiering saknas i grundläget men kommer fram med senarevalet. Stäng av senarevalet efter att ha valt en sådan post.
5. Växla svenska/engelska. Öppna på nytt och kontrollera rubriker, listor och detaljer. Prova ändrad svensk text med gammal en_source: svensk fallback ska visas.
6. Öppna/stäng 20 gånger, byt sida, tvinga garbage collection; inga kvarvarande preview-worlds. Testa två lokala spelare med oberoende rotation samt tangentbord/handkontroll.
7. Kontrollera egen mesh/material med nollmeridian och poler. Testa paketerad build på målplattformens renderingsbackend. Renderingen är inte verifierad på mobil eller konsol.

Ingen Unreal-binär, shaderkompilering eller spelkörning har funnits tillgänglig i arbetsmiljön. Automationstesterna för Unreal är därför medföljande testkod, inte rapporterade som godkända.

För denna utökning: prova båda Nicaraguapunkterna vid minsta/största zoom, lager av/på, mus/tangentbord, bakre halvklotet och svenska/engelska. Välj varje aktör via landets relaterade knappar. Kontrollera att landets ministerprofil fortfarande avser februari när senare historik är aktiverad.

## Kontrollera det utökade konfliktlagret

1. Välj Konflikter. Kontrollera egen kryssad romb, numrerade aktörer och detaljpanelens motsvarande nummer.
2. Välj Nicaragua, Palestina/Israel och vart och ett av de kurdiska konfliktområdena. Kontrollera separata motparter, pilar och läsbara detaljer på svenska/engelska.
3. Välj Afghanistan eller Tchad: dubbelpilar och blå stödpilar ska skilja sig. Gukurahundi ska ha enkelriktad våldspil till civila.
4. Aktivera Alla konfliktpilar och rotera. Bakre markörer/pilar får inte synas eller fångas av klick. Slå av Konflikter: både symboler och konfliktpilar ska försvinna.
5. I grundläget finns Egyptens myteri men inte Sydjemens januarikonflikt eller mars/aprilstriderna vid Libyen. Senare-1986-valet får inte ändra det.
6. Aktivera ±90 dagar: sex avslutade och två kommande poster tillkommer. Välj en av dem och stäng filtret: detaljpanelen ska rensas och posten försvinna.
7. Testa minsta/största zoom, språkbyte, överlappande markörer via listan och paketerad körning. Numren ersätter långa etiketter på globen; namnen finns i detaljpanelen.

JSON-valideringen avvisar okända relationstyper, saknade ändpunkter, självlänkar, dubletter, ogiltiga koordinater/offsets och ofullständiga daterade konflikter. Utbyggnaden är bakåtkompatibel för övriga posttyper; äldre konfliktposter måste få datum, aktörer och relationer.
