# TMOP – kontrollprofiler för 1–4 lokala spelare

Datum: 2026-09-12. Bas: commit `43ce45c8` (`multi`) och den tidigare kumulativa
multiplayerleveransen.

## Vad som nu finns

- Fyra spelarplatser med separat sparad profil för varje tillåten enhet.
- Om-bindning av primär och sekundär tangent/knapp inne i `CONTROLS`.
- Sparad X/Y-känslighet, inverterad Y och zoom-FOV per spelare.
- Separata sammanhang för till-fots-, fordons- och menykontroller. Därför får
  exempelvis hopp och fordonsbroms använda samma knapp.
- Konfliktskydd inom samma aktiva sammanhang.
- Strikt konfliktskydd mellan spelare 1 och 2 när båda använder tangentbordet.
- Dynamiska knapptexter för interaktion, paus, karta, tidning och läsvyer.
- Kvarhängande input nollställs när en meny öppnas/stängs eller en knapp binds om.

Inställningarna sparas separat i `TMOP_ControlSettings_v1`. De följer därför
spelaren mellan nya spel och laddade spel och skrivs inte över av en äldre
spelvärldssparning.

## Standardupplägg

| Spelarläge | Standardstyrning |
| --- | --- |
| 1 spelare, tangentbordsläge | P1 tangentbord och mus |
| 2 spelare, tangentbordsläge | P1 tangentbord/mus och P2 separat tangentbordsuppsättning |
| 2–4 spelare, blandat läge | P1 tangentbord/mus och övriga en handkontroll var |
| Handkontrollsläge | En handkontroll per spelare |

P2 använder som standard piltangenter för rörelse och I/J/K/L för kamera.
Profilerna kan ändras i spelet. Musen tillhör alltid P1; spelet skapar inte två
muspekare. P3 och P4 använder handkontroller.

## Meny

Öppna `INSTÄLLNINGAR` från huvudmenyn eller pausmenyn och välj `CONTROLS`.

1. Välj `SPELARE 1`–`SPELARE 4`.
2. Välj enhet. P1 kan växla tangentbord/mus eller handkontroll. P2 kan växla
   delat tangentbord eller handkontroll. P3–P4 använder handkontroll.
3. Klicka på primär eller sekundär bindning och tryck nästa knapp.
4. Vid konflikt visas vilken spelare och handling som redan använder knappen.
5. Kamerareglagen sparas direkt. Standardknappen återställer bara vald spelare.

Enhetsbyte återanvänder den enhetens sparade profil. Även inaktiva
tangentbordsprofiler reserverar sina tangenter, så att ett senare byte till delat
tangentbord inte skapar en ny krock. Återställning avvisas med ett meddelande om
standardtangenterna redan används av den andra spelaren.

Handkontrollens extra sprint och separata tittzoom är obundna som standard:
tidigare körde de samtidigt med perspektivbyte respektive sekundärhandling.
Använd × 1/× 2 för att frigöra en knapp och bind den önskade funktionen.
Alla funktionerna finns kvar i kontrollmenyn. Vanlig sprint, sekundärhandling
och perspektivbyte behåller sina knappar. Vid migrering tas bara de två kända
äldre dubbelbindningarna bort. Äldre krockar mellan tangentbordsspelare löses
genom att P1:s tangent behålls och motsvarande P2-bindning visas som Ej bunden.

Tidningsbläddring har egna menyhandlingar: Q/E för P1, Y/B för P2 och vänster/höger
trigger på handkontroll. Panorering ligger separat på menyriktningarna.
Fordonets högfartsläge är en egen bindning: vänster Ctrl, N respektive vänster
styrspaksklick. Handbromsen aktiverar inte högfartsläget.

## Viktig integrationsändring

När `bUseControlProfiles` är aktivt på `BP_TMOPPlayerCharacter` läggs den gamla
`DefaultMappingContext` inte till och de gamla direkta C++-fallbackarna körs inte.
Det förhindrar dubbelhändelser. Låt `bUseControlProfiles` vara aktiverat.

Om Blueprinten själv lägger till samma Input Mapping Context eller har helt
separata tangent-events utanför basklassens input behöver dessa Blueprint-noder
stängas av. Native-koden kan inte ta bort okända binära Blueprint-grafer.

## C2248-fixen

Direkt skrivning till den skyddade engine-medlemmen
`APlayerController::bShouldPerformFullTickWhenPaused` är borttagen på båda
ställena. `PrimaryActorTick.bTickEvenWhenPaused` behålls. Pausmenyer och delat
tangentbord hanteras via Slate och den gemensamma input-processorn även medan
världen är pausad.

## Stabilisering och C2259

- Inputprocessorn implementerar nu IInputProcessor::Tick med UE:s signatur.
- Slate-användaren hämtas via en skrivbar `ULocalPlayer`, så UE 5.8 väljer
  `TSharedPtr<FSlateUser>` i stället för den konstanta överlagringen. Det rättar
  C2679 och C2662 i `SetMenuNavigation`.
- Tangenter spärras per spelare och knapp efter meny/ombindning. P2 kan inte
  blockera P1 genom att hålla en knapp. Förlorat applikationsfokus rensar
  tangentbordets tillstånd.
- Kontrollpanelens fokus och Slate-knappnavigering hör till rätt användare.
  Två samtidiga ombindningar tillåts inte. Spelarens tidigare navigeringsregler
  återställs när menyn stängs.
- Musrörelse följer valda kameraaxlar; en gammal fast musväg körs inte samtidigt.
- Kartan släpper analog panorering när den förlorar fokus.
- Loopomstart, laddning och tidsförflyttning stänger menyer och lämnar fordon
  innan historiska aktörer återskapas. Försvunna dialogpersoner stänger sin dialog.
- Fordonssäten städar upp passagerare vid despawn; lokal fordonskamera återställs
  om fordonssessionen upphör. Spelarens borttagning avslutar också dialogfokus.
- Föremål reserverar mängden innan inventariehändelser skickas, vilket förhindrar
  dubbelutdelning från återanrop under samma upptagning.
- Den senast aktiverade/kanaländrade radion äger den gemensamma ljudutgången.
  Övriga radioapparater behåller sina kanaler och visar att en annan radio hörs.
- Gamla debuggenvägar 1–9/B utför inte tidsflyttning/bakning när kontrollprofiler
  används. Tidsflyttning via menyn och direktorns funktioner finns kvar.

## Bygg

1. Stäng Unreal Editor.
2. Kopiera `Plugins` och `Scripts` från ZIP-filen till projektroten.
3. Radera projektets `Binaries` och `Intermediate` om gamla objektfiler ligger kvar.
4. Generera projektfiler och bygg `Development Editor / Win64`.
5. Starta Standalone med editorns Number of Players satt till 1 och välj sedan
   antal spelare i spelets huvudmeny.

## Obligatoriskt Unreal-test

- Bygg med UHT/MSVC.
- Starta 1, 2, 3 och 4 spelare.
- För två spelare: testa P1 WASD/mus och P2 pilar + I/J/K/L samtidigt.
- Öppna varsin paus-/kartvy och kontrollera att P2:s tangenter inte flyttar P1:s fokus.
- Bind om en tangent, håll den nedtryckt när menyn stängs och verifiera att ingen
  rörelse, zoom, gas eller handbroms ligger kvar.
- Testa två bilar samt förare/passagerare i samma bil.
- Kontrollera dynamiska knapptexter efter ombindning och efter omstart av spelet.

Verifierat här: 17 kontroll-/regressionstester och 12 multiplayer-källkontrakt.
Tre fristående C++-testprogram har kompilerats och körts: den produktionsanvända
knappspärren, sessions-/paus-/startpositionspolicyn och loopslutspolicyn.
Detta är inte ett Unreal-bygge eller en speltest. UE 5.8/UHT/MSVC, Blueprint-grafer,
fysiska handkontroller och faktisk split-screen-rendering återstår att verifiera
i projektets Windows-miljö.
