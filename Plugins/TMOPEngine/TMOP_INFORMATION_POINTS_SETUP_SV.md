# Fristående informationspunkter

Informationspunkter är texter som du själv lägger ut på spelplanen. De använder
samma target-markering, E-interaktion och läsfönster som adressregistret.
Du behöver ingen adressrad, inget ankare och ingen datatabell för en sådan punkt.

## Installera uppdateringen

ZIP-filen innehåller både den tidigare adressuppdateringen och informationspunkterna.
Stäng editorn, packa upp `Plugins` och `Scripts` i projektroten och bygg
**Development Editor**. Öppna sedan Unreal igen. Nya reflekterade klasser och
en ny gemensam basklass kräver ombyggnad och omstart.

Tidigare adresskomponenter, register och adresskopplingsscript finns kvar.
Adresskomponenternas inställningar heter fortfarande `Registry`, `RowName`,
`InteractionOffset`, `InteractionRadiusCm` och `InteractionEnabled`.
Boendenas efternamn visas nu som initialer enligt den senare namnuppdateringen;
våningsreglerna är desamma.

## Lägg ut en punkt

1. Sök efter **TMOP Information Point** i **Place Actors → All Classes** och
   dra ut en i banan. Klassen heter `TMOPInformationPoint`; den finns också
   under pluginens C++ Classes när C++-klasser/plugininnehåll visas.
2. Markera punkten och välj komponenten **Information** i komponentlistan
   överst i Details.
3. Skriv **Title** och **Description**. Brödtexten har stöd för flera stycken.
4. Fyll vid behov i **Category Label** och **Source Reference**.
5. Spara banan. I Play: sikta på punkten, gå nära och tryck **E**.

Punkten visar rubriken vid hover och **[E] Läs information** inom spelarens
interaktionsavstånd, normalt tre meter. Texten och den valfria källan går att
scrolla. **E**, **Esc** eller **Stäng** stänger fönstret. Spelklockan fortsätter
gå medan spelaren läser. Rörelse och föremålsanvändning spärras under läsningen.

Punkter med tom rubrik eller tom brödtext blir inte läsbara i spelet.
Editorbeskrivningen är dold under Play; i spelet används den befintliga
target-markeringen när du siktar på punkten.

## Fälten

| Fält | Användning |
| --- | --- |
| Title | Rubriken i target-markeringen och läsfönstret. |
| Description | Din förklarande text, med flera stycken om du vill. |
| Category Label | Valfri text, till exempel Företag, Händelse, Plats eller Pågår nu. Standard är Information. |
| Source Reference | Valfri källa, till exempel ett uppslagsnummer, dokument/sida eller en URL som vanlig text. |
| Interaction Enabled | Slår på eller av läsningen av just punkten. |
| Interaction Offset | Flyttar läspunkten relativt actorn, utan att flytta actorn. |
| Interaction Radius Cm | Storleken på den osynliga träffytan. Standard 45 cm. |
| Use Time Window | Begränsar när punkten kan läsas. Avstängd som standard. |
| Visible From / Visible Until | Tid enligt spelets klocka när tidsfönstret är aktiverat. |

En fristående actor får läspunkten **140 cm ovanför sin position**: du kan
alltså dra ut den på marken. Placera den vid den fasad, dörr eller sak texten
handlar om. Om du placerar actorn direkt i läshöjd sätter du Interaction Offset
till `(0, 0, 0)`. Träffytan stoppar ingen rörelse och påverkar inte navmesh.

Du kan även välja ett befintligt föremål eller en dörr och lägga till
**TMOP Information** via **Add Component**. Komponenten på ett sådant objekt
har som standard offset `(0, 0, 0)`; flytta läspunkten med Interaction Offset
vid behov. Actorns **Actor Enable Collision** måste vara på för att den
osynliga träffytan ska kunna hittas. Använd en läskomponent per actor; placera
flera fristående informationspunkter om du vill ha separata texter.

## Historisk bakgrund eller något som pågår

Låt **Use Time Window** vara avstängd för bakgrundsinformation som ska kunna
läsas under hela spelrundan. Till exempel en beskrivning av ett företags kontor
eller av en tidigare händelse på platsen.

Slå på tidsfönstret om texten endast är giltig under en bestämd period.
Exempel för att visa hur fälten används: en aktivitet som i din rekonstruktion
pågår 23:10–23:30 får Visible From `23:10:00` och Visible Until `23:30:00`.
Starten ingår, slutet ingår inte. Klockan 23:30 försvinner möjligheten att läsa
punkten och ett öppet fönster för den stängs. Exemplens tider är inte källuppgifter.

Tidsfönstret följer spelklockan även vid bakåtspolning och nya loopar.
Fönster över midnatt fungerar också. Lika start- och sluttid ger ett tomt
tidsfönster; använd avstängd tidsstyrning för en permanent punkt. Om ingen
spelklocka finns visas inte tidsbegränsade punkter. Den här versionen använder
absoluta klockslag för informationspunkterna.

## Exempel på innehåll att fylla i

| Typ av punkt | Förslag till upplägg |
| --- | --- |
| Ett vapenföretag på adressen | Företagsnamn som rubrik, uppgifter om verksamheten och adressen i brödtexten, dokumenthänvisning som källa. |
| En förstörd gren | Rubrik om grenen, beskrivning av vad källan uppger och när, samt källa. |
| Brandförsvarets radiotest | Rubrik om testet, beskrivning av plats och tid enligt underlaget. |
| Firmafest just nu | Kategori Pågår nu och text om festen. Använd tidsfönster om den bara ska gälla en del av spelrundan. |

Exemplen är mallar. Uppdateringen placerar inte ut historiska påståenden,
företagsnamn eller tider i banan automatiskt. Du kan duplicera en ifylld punkt
och ändra plats och text för att snabbt lägga till fler.

## Kontroller och återstående provkörning

Lokalt verifierat: 19 fall för den C++-funktion som avgör tidsfönstret,
11 tester för det befintliga adresskopplingsscriptet, Python-syntax och
diffkontroll. Tidsfallen omfattar gränssekunder, bakåtspolning, omstart,
midnatt och ogiltiga intervall.

Unreal Editor/UE 5.8 finns inte här. Hela pluginen, Blueprint-laddningen och
interaktionen har därför inte kompilerats eller provkörts i Unreal i den här
miljön. Automationstesterna `TMOP.Information.Content`, `TMOP.Address.Display`
och `TMOP.Address.Component` medföljer för körning där.

Kontrollera efter bygget: öppna en informationspunkt med lång text och källa,
stäng den, öppna en adress och kontrollera boendelistan. Kontrollera sedan
att en tidsbegränsad punkt aktiveras/stängs vid rätt tid och återkommer när
tiden spolas tillbaka. Spara och öppna banan igen för att kontrollera texterna.
