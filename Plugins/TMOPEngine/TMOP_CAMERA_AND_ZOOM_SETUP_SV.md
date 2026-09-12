# Kameravy och blickzoom

Vybytet fanns redan i `TMOPCameraPerspectiveComponent`: V på tangentbordet och
tryck på höger styrspak (R3). Uppdateringen lägger till håll-in-zoom och kopplar
target/hover och interaktion till den aktiva första- eller tredjepersonskameran.

## Installera

Stäng Unreal. Packa upp ZIP-filens `Plugins` och `Scripts` i projektroten och
bygg **Development Editor**. Öppna projektet igen. Paketet innehåller även
adressinteraktionen och de fristående informationspunkterna från föregående
uppdatering. Inga nivåer eller Input Mapping Context-assets ändras av paketet.

`ATMOPPlayerCharacter` skapar redan komponenten **CameraPerspective**.
Välj den i spelarens Blueprint för att justera inställningarna. Befintliga
Blueprint-värden kan åsidosätta C++-standardvärdena: kontrollera att
**Bind Toggle Keys Automatically**, **Enable Look Zoom** och
**Read Zoom Keys Automatically** är på.

## Kontroller

| Kontroll | Funktion |
| --- | --- |
| V / R3 | Byt mellan första- och tredjeperson. |
| Håll Z | Zooma blicken framåt. Släpp för mjuk återgång. |
| L2 / Gamepad Left Trigger Axis | Steglös zoom efter hur långt avtryckaren trycks in, när inputsystemet skickar denna standardaxel. |
| Två touchpunkter: dra isär fingrarna | Zooma proportionellt mot fingeravståndet. Behåll fingrarna för att behålla zoomen; släpp ett finger för återgång. Kräver att plattformen skickar båda touchpositionerna till Unreal. |

Zoomen fungerar i båda perspektiven och ändrar kamerans synfält, utan att
flytta spelaren eller kamerans position. Ursprungligt synfält sparas och
återställs för respektive kamera. En redan mer inzoomad kamera zoomas inte ut
av zoomknappen. Flera samtidiga zoomsignaler använder den starkaste signalen.

Siktet blir lugnare under zoom genom lägre look-känslighet. Adresser och
informationspunkter använder samma aktiva kamera när de väljs. Zoom förstorar
det du ser men ökar inte avståndet där E-interaktion tillåts.

Menyer, dialog, tidningsläsning, bilåkning, paus och externa kameravyer spärrar
de automatiska kamerakontrollerna och återställer zoomen. En fysisk zoomknapp
som fortfarande hålls inne kan återuppta zoomen när vanlig spelkontroll återgår.
Ett programstyrt zoomvärde nollställs och måste skickas igen.

Spelarens kropp och modulära ansikte, hår och kläder döljs för den egna
kameran i förstaperson och återställs vid vybyte. Separata föremål och
tidningsarmar omfattas inte av denna döljning.

## Inställningar på CameraPerspective

| Fält | Standard / betydelse |
| --- | --- |
| Start In First Person | Av: börja i tredjeperson. |
| Toggle Key / Gamepad Toggle Key | V / R3. |
| Zoom Hold Key | Z. |
| Zoom Hold Gamepad Key | Valfri extra digital knapp, tom som standard. |
| Zoom Analog Key | Gamepad Left Trigger Axis; förväntar 0–1. |
| Zoom Analog Dead Zone | 0,04; tar bort brus nära viloläget. |
| Zoom Field Of View | 40 grader vid full zoom. Lägre värde ger mer förstoring. |
| Zoom Blend Speed | 10; högre värde ger snabbare in- och utzoomning. |
| Scale Look Sensitivity With Zoom | På. Kan stängas av om annan inputkod redan skalar känsligheten. |
| Enable Touch Pinch Zoom | På; gäller två touchpositioner från PlayerController. |
| Pinch Full Zoom Distance Ratio | 2: dubbelt fingeravstånd från gestens början ger full zoom. |
| First Person Camera Offset | (0, 0, 64) relativt spelarens rot/kapselcentrum; justera vid behov efter spelarens höjd. |
| First Person Field Of View | 90 grader före zoom. |

## Touchpad och egen input

Nypzoom förutsätter två touchpositioner. En dators touchpad eller en
DualSense-touchpad som bara rapporterar musrörelse, scroll eller ett knapptryck
ger inte automatiskt dessa positioner. Den befintliga RawInput-konfigurationen
ändras inte här: dess enhetsspecifika axlar behöver först identifieras på den
aktuella datorn. Se [Epics RawInput-dokumentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/rawinput-plugin-in-unreal-engine).

Komponenten har följande Blueprint-anrop för Enhanced Input eller en adapter:

- `BeginLookZoom`: begär full zoom.
- `SetLookZoomAmount(0..1)`: begär en bestämd zoomstyrka från avtryckare eller gest.
- `EndLookZoom`: begär mjuk återgång. Koppla både **Completed** och **Canceled**
  hit; gör samma sak när en touchkontakt släpps.
- `CancelLookZoom`: nollställer begäran och återställer synfältet direkt.
- `GetLookZoomAmount`: aktuell interpolerad zoomstyrka.
- `GetActivePerspectiveCamera`: aktiv kamera för vanlig spelvy.

En adapter som får gestdeltan måste själv hålla ett värde mellan 0 och 1,
skicka detta till `SetLookZoomAmount` och anropa `EndLookZoom` när gesten slutar.
Om en gestkälla saknar slut-/kontaktinformation behövs en separat släppsignal.
Stäng av **Read Zoom Keys Automatically** respektive **Enable Touch Pinch Zoom**
om samma input istället hanteras helt av egna Blueprint-bindningar. Undvik
dubbla V/R3-bindningar genom att stänga av **Bind Toggle Keys Automatically**
om en Input Action redan anropar `TogglePerspective`.

## Verifiering

De fristående C++-testerna för zoommatematiken kontrollerar bland annat neutralt
synfält, full och steglös zoom, dead zone, nypgest och ogiltig enhetsdata.
De verifierar inte Unreal-integrationen eller hårdvarans inputleverans.
Unreal Editor/SDK finns inte i denna arbetsmiljö: bygget och Play-testet återstår.

Efter ombyggnad, kontrollera i Play:

1. Byt V fram och tillbaka och sikta på samma adress/informationspunkt i båda
   vyerna. E ska öppna rätt text när spelaren står inom interaktionsavstånd.
2. Håll/släpp Z i båda vyerna och byt vy medan Z hålls inne. Varje kameras
   vanliga synfält ska återkomma efter släpp.
3. Öppna och stäng paus, läsfönster, tidning, dialog och bilvy under zoom.
   Kontrollera att zoomen återställs och att V inte byter vy i dessa lägen.
4. Testa halv/full L2 med den verkliga kontrollen. Testa nypzoom med en
   enhet som skickar två touchpunkter, inklusive att släppa ett finger.
5. Kontrollera att ansikte/hår inte skymmer förstaperson och att kläderna
   syns igen i tredjeperson, även efter ett klädbyte i förstaperson.
