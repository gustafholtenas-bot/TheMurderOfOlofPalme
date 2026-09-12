# TMOP – lokalt 1–4-spelarläge

Datum: 2026-09-12. Bas: projektets commit `e2035906` med tidigare kumulativa kodändringar.

## Status och omfattning

Detta är en C++-implementation för lokalt spel med delad skärm, klar för integrationstest.
Den är **inte byggd med Unreal Header Tool/MSVC eller speltestad i Unreal 5.8 här**.
Källkontroller och portabla C++-tester passerar, men det bevisar inte att samtliga
Blueprints, handkontroller eller kameravyer fungerar i den färdiga nivån.

En dator, en värld och en gemensam klocka används. Huvudmenyn har val för 1, 2, 3
eller 4 spelare. En spelare ger helskärm; flera ger Unreals splitscreen enligt
projektets layoutinställningar. Ingen nätverkssession, lobby eller replikering skapas.

## Installera

1. Säkerhetskopiera projektet och stäng Unreal Editor.
2. Kopiera paketets `Plugins` och `Scripts` till projektroten. Behåll undermapparna.
   Paketet är en kumulativ kodpatch, inte ett komplett projekt. Tidigare kodfixar
   för C4150, adressläsning, informationspunkter, kamerazoom, namninitialer,
   gärningsmannens spökgrenar och slutmenyn följer med. Inga DataTables eller
   binära Blueprint-/nivåfiler ändras av detta paket.
3. Generera projektfiler igen och bygg **Development Editor / Win64** med editorn
   stängd. Använd en vanlig ombyggnad, inte enbart Live Coding efter dessa UCLASS-
   och USTRUCT-ändringar. `EngineSettings` har lagts till i modulberoendena.
4. Kontrollera att huvudnivån använder projektets TMOP-spelare/GameMode,
   `TMOPMainMenuIntroDirector` och `TMOPSimulationDebugDirector` (den senare behövs
   för laddning/tidshopp). Extra spelare skapas från samma Pawn-klass som spelare 1.
   Spelarpawns som skapas av GameMode ska ha **Auto Possess Player = Disabled**
   så att de inte försöker ta över spelare 1 när de skapas.
5. Starta **Standalone Game**, med editorns *Number of Players = 1*. Välj sedan
   1–4 i spelets huvudmeny. Editorns flerspelarinställning avser separata nätverks-
   PIE-instanser och är inte detta lokala läge.

## Kontroller och startplats

Huvudmenyn väljer också mellan tangentbordsläge och handkontroller för alla.
Med exakt två spelare ger tangentbordsläget P1 tangentbord/mus och P2 en separat
tangentbordsuppsättning. Med tre eller fyra spelare använder P1 tangentbord/mus
och övriga handkontroller. Det finns inte flera oberoende muspekare.

Gamepad-offset och splitscreen ändras bara för sessionen och skrivs inte över i
projektets ini. Spelarnas egna kontrollprofiler sparas däremot avsiktligt i
`TMOP_ControlSettings_v1`. Meny-/introfasen har bara en lokal spelare.

På `TMOPMainMenuIntroDirector`:

- `LocalPlayerCount`: startvärde för menyn, 1–4.
- `bKeyboardForPlayerOne`: startvärde för kontrollfördelningen.
- `PlayerStartAnchorId`: gemensamt startankare. Om tomt används introts slutankare.
- Om inget ankare alls är angivet används spelarens befintliga starttransform.

Mark och fri kapselvolym kontrolleras för samtliga startplatser innan gruppen
flyttas. Platser söks inom sex meter från ankaret, med mellanrum mellan spelarna.
Golvet behöver blockera Visibility-spårningen och spelarnas kollisionsprofil.
Saknat ankare eller otillräckligt med fritt golv ger fel i huvudmenyn; extra
spelare tas då bort. Ett misslyckat återstartsförsök håller världen pausad och
loggar felet i Output Log.

## De fyra utseendena

Befintliga `TMOPPlayerAppearanceDirector` med `PlayerIndex` **0, 1, 2, 3** används
för spelare **1, 2, 3, 4**. Separata utplacerade directors per spelarplats har företräde.

Alternativt kan en director med `PlayerIndex = 0` ha fyra poster i
`LocalPlayerAppearances`. Varje post anger personprofil eller inline-utseende och
kön, med samma befintliga assetkatalog. Runtime-directors skapas för platser som
saknar egen director. Om en platsspecifik profil saknas ärvs grundprofilen;
systemet hittar inte på fyra nya utseenden. Kontrollera därför de fyra profilerna
i Details. Inga material eller karaktärsassets har skapats/ändrats i denna patch.

## Vad som är gemensamt respektive individuellt

| System | Beteende i implementationen |
| --- | --- |
| Klocka, historiska händelser, NPC:er, timed dialogue, trafik och killer-grenar | En gemensam värld och simulering, inte en kopia per spelare. |
| Paus | En spelares pausmeny stoppar världen och klockan för alla. Varje spelare äger sitt pauskrav; alla aktiva krav måste släppas innan tiden fortsätter. |
| Karta/tidning | Kartan pausar hela världen. Tidningen gör det om dess befintliga pausinställning är aktiv. Andra spelare kan öppna/stänga egna pausvyer under pausen. |
| Slut 23:45 | Alla får egen slutmeny. Ett val gäller gruppen. Omstart flyttar alla tillbaka till startområdet; huvudmeny tar bort extra spelare och laddar om nivån. Avsluta stänger programmet. |
| Kamera | Egen vy per spelare; befintlig första-/tredjeperson och blickzoom. Separata fordonskameror även när två sitter i samma bil. |
| Personkort, adresser och informationspunkter | Öppnas på den interagerande spelarens skärmdel och blockerar den spelarens rörelse. Läspanelerna pausar inte automatiskt hela världen. |
| Fokuserad NPC-dialog | En aktiv fokuserad dialog per NPC. En andra spelare kan inte ta över pågående fokus; vanliga personkort är separata. |
| Inventarier och upptäckta bevis-ID:n | Eget tillstånd per spelare. Ett fysiskt världsobjekt delas och kan inte plockas upp två gånger. Upptäckta bevis kopieras inte automatiskt till andra spelare. |
| Fordon | Säten delas. En annan spelares förarsäte kan inte tas över; ledigt passagerarsäte används när möjligt. |
| Spelgräns | Kontrollerar alla spelare och berörda fordon, inte bara spelare 1. |
| Namnetiketter | Egna projicerade etiketter per vy ersätter globala kameravända etiketter i multiplayer; befintlig censurering används. Närmaste 24 inom 20 meter visas per vy. |
| Ljud | Avståndsberoende aktivering kontrollerar närmaste lokala kamera. Ljuden hörs via samma datorutgång. Ingen separat hörlursmix per spelare. |
| Radio och Grandfilm | Gemensam scenariotid. Radioapparater kan ha olika kanaler men blandas i samma ljudutgång. Grandfilmen synkroniseras också under världspaus. |
| Inställningar | Grafik och datorns ljudinställningar är gemensamma, inte per spelare. |

Omstart använder befintliga historiska loop-återställningar. Den är inte en ny,
fullständig snapshot-återställning av alla användarändrade världsobjekt;
inventarier/upptäckter behålls enligt den befintliga loopmodellen.

## Handkontroller i läsvyer

De beständiga kontrollprofilerna används för spelandet när `bUseControlProfiles`
är aktivt. Den äldre Enhanced Input-contexten och de direkta native-fallbackarna
kopplas då bort för att undvika dubbelinput. Se `TMOP_CONTROL_PROFILES_SETUP_SV.md`.
Följande direkta UI-kontroller har kompletterats:

| Vy | Standardknappar (Xbox-benämningar) |
| --- | --- |
| Paus | Start/Menu. Navigera med vanlig Slate-knappnavigering och bekräfta med A. |
| Karta | Back/View öppnar; vänsterspak eller styrkors panorerar; LB/RB zoomar; X återställer; B stänger. |
| Adress/informationspunkt/personkort | Styrkors upp/ned rullar; LB/RB rullar längre; B stänger. |
| Tidning | Styrkors flyttar läsläget; LB/RB bläddrar; LT/RT zoomar; B stänger. |

`WorldMapGamepadFallbackKey` kan ändras om Back/View redan används till annat i
din Blueprint. Befintliga mappningar kan också skilja sig från knapptexterna.
Kartans filterväxlar behåller sin musinteraktion; separat gamepad-filterväljare
är inte implementerad. Kontrollera särskilt detta om kartfilter behövs för P2–P4.

## Spara och ladda

Spelare 1 sparar/laddar hela sessionen. Format 3 lagrar spelarantal, kontrollfördelning,
gemensam tid samt varje spelares transform, blickrotation, inventarie, utrustat föremål,
upptäckta bevis och radiokanal. Äldre format läses som enspelarsessioner.
Ogiltig tid, NaN-position eller fel nivå avvisas före gruppändring.

**Alla måste stiga ur fordon innan sparning.** Fordonsbesittning och godtyckliga
ändringar av fordonsvärlden är inte en komplett sparad snapshot. Laddning använder
projektets historiska rekonstruktion vid sparad tid. Förbrukade/flyttade
världsobjekt återställs inte som en exakt snapshot heller; detta följer den
befintliga sparmodellens begränsning. Spara/ladda är därför inte verifierat som
förlustfri återställning av varje möjlig interaktion.

## Obligatoriskt test i Unreal

Kör varje relevant rad med 1, 2, 3 och 4 spelare. Kör både tangentbord + pads och
bara pads. Markera som godkänt först efter faktiskt test, inte utifrån källkontrollerna.

| Test | Godkänt när |
| --- | --- |
| Fullt bygge | UHT och Development Editor bygger utan fel; C4150 återkommer inte. |
| Start med/utan intro | Rätt antal vyer, rätt Pawn-klass och rätt fyra utseenden. Alla står på golvet nära samma ankare och kan röra sig direkt. |
| Inputisolation | En handkontroll påverkar bara sin spelare. P1:s mus påverkar inte P2:s kamera. Ingen knapp dubbeltogglar paus/karta. |
| Överlappande pauser | P2 öppnar karta, P1 pausar. NPC, trafik, klocka och film står still. Stäng bara en meny: världen står fortfarande still. Stäng sista: allt fortsätter. |
| UI-fokus | P1 läser och P2 öppnar/stänger egen vy. P1 behåller fokus. Testa särskilt tangentbord + gamepad-offset och navigering efter flera öppna/stäng-cykler. |
| Läsning | Långa adresser/personkort går att rulla; kartan och tidningen går att använda på P2–P4. Ingen panel ritas över en annan spelares område. |
| Kamera/namn | Första/tredjeperson, zoom och censurerade namn fungerar från alla vyer. Övriga ser inte en spelares first-person-armar eller lästidning som felaktiga helskärmsobjekt. |
| Föremål/dialog | Två försöker ta samma objekt: bara en lyckas. Två läser samma personkort utan att stänga varandras UI. |
| Fordon | Två spelare kör olika bilar, därefter förare + passagerare i samma bil. Egen kamera, ingen stulen spelarplats; urstigning och paus/fortsätt utan kvarhängande gas. |
| Tidsbundna händelser | Alla ser/hör samma skott, tal, trafik och spökgrenar vid samma scenariotid, utan dubbla världsspawns. |
| Ljud/avstånd | P2 går långt från P1: lokal trafik och personer fortsätter fungera kring P2. Bedöm den gemensamma radiomixen. |
| Sparning | P2:s sparförsök avvisas. P1 sparar med alla ur bil; laddning återställer antal, innehav, radio och positioner. Testa även äldre enspelarsparning. |
| Slut 23:45 | Tiden stannar exakt. En spelares omstartsval återställer alla. Huvudmeny ger en vy. Starta sedan med annat antal och upprepa flera gånger. |
| Ladda vid sluttid | Slutmenyer syns, har fungerande fokus och kan inte kringgås av StartClock. |
| Upplösning/prestanda | Läsbarhet i fyra vyer på målskärmen och acceptabel GPU-belastning; ingen vy försvinner efter upplösningsbyte. |
| Bortkopplad handkontroll | Testa återanslutning och verifiera spelarordning. Automatisk disconnect-meny/paus är inte implementerad. |

Blueprint-widgets kan fortfarande innehålla egna `GetPlayerController(0)` eller
`AddToViewport`-anrop. Sådana måste granskas i editorn och använda owning player
och `AddToPlayerScreen` för individuella HUD-vyer. Huvudmenyn ska däremot vara
gemensam/fullskärm. Detta paket ändrar native TMOP-vägarna, inte ogenomgångna
binära Blueprint-grafer eller Epic-exempellägen som SideScroller.

## Tester som kan köras utan Unreal

`python Scripts/tests/test_local_multiplayer_contracts.py`: 12 källkontrakt.
`test_local_session_policy.cpp`: spelargränser, 16 klockkombinationer, 65
startkandidater och plats för fyra kapslar. Portabla regressionstester för
slutmeny, blickzoom (25), killer-grenar (21), namn (25) och informationstider (19)
är också körda med g++17, `-Wall -Wextra -Werror`. `git diff --check` passerar.

Unreal-automationstestet `TMOP.LocalMultiplayer.PauseOwnership` följer med men
är **inte kört här**. Kör det i Session Frontend → Automation. Det testar
pausägarskap i subsystemet, inte faktisk bild-/ljudfrysning eller fysisk input.

## Fortsatt nätverksspel

Nätverk är en separat etapp: en auktoritativ server för värld/klocka, replikering
av spelare/NPC:er/fordon och relevanta tillstånd, serverkontrollerade interaktioner,
anslutning/avbrott, gemensam pauspolicy och nätverkstester. Att bara slå på
replikering på spelaren gör inte alla dessa system nätverksklara. Den lokala
implementationen avvisar nätverksläge i sessionsstarten.

Teknisk referens: Unreals inställningar för splitscreen och gamepad-offset finns i
[UGameMapsSettings](https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/EngineSettings/UGameMapsSettings);
layouten hanteras av
[UGameViewportClient](https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/Engine/UGameViewportClient).
