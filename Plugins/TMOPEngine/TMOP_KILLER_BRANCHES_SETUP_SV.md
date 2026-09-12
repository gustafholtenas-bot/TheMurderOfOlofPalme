# Gärningsmannens alternativa flyktvägar

Originalet `THE_KILLER` fortsätter längs sin befintliga tidslinje. Vid ett
markerat hörn uppstår genomskinliga, blå versioner som springer längs de
alternativa vägar du kopplat in. När en spökversion når nästa markerade hörn
delar den sig igen om flera fortsättningar finns.

Det här är ett visuellt lager för möjliga flyktvägar. Spökena är egna visuella
actors, med kopior av gärningsmannens kropp/kläder och egen löpanimation.
De saknar person-id, AI, kollision, dialog, skott, automatisk speech och
registrering som vittnen eller observationer. Originalets personrad, rörelse,
utseende och händelser ändras inte. Andra misstänkta används aldrig som källa:
koden kräver exakt `THE_KILLER`.

## Installera och förbered banan

1. Stäng Unreal, packa upp `Plugins` och `Scripts` i projektroten och bygg
   **Development Editor**. Starta om Unreal.
2. Öppna banan med de befintliga flyktankarna och laddad navmesh.
3. Kör `Scripts/tmop_setup_killer_branches.py` via **Tools → Execute Python Script**.
   Python Editor Script Plugin behöver vara aktiverad.
4. Scriptet skapar `M_TMOP_KillerGhost`, lägger ut en **TMOP Killer Branch
   Director** och startpunkter där följande ankare finns: vändningen mot
   Tunnelgatan, trappans nederdel, trappans överdel, mötet med Yvonne och
   mötet med Anki. Saknade eller dubbla ankare rapporteras i Output Log.
5. Kontrollera att hörnpunkterna ligger på marken. Kör **Save All**.

Scriptets startnät visar originalets riktning. **Lägg till de alternativa
vägarna enligt nedan för att få spöken.** Nivåfilen och möjliga gatukopplingar
har inte kunnat granskas eller redigeras i denna arbetsmiljö. Inga alternativa
gator eller historiska observationer har därför hittats på.

Scriptet kan köras igen: befintliga punkter flyttas inte och deras inställningar
skrivs inte över. Material och löpanimation kopplas till director-actor som
assetreferenser, så att de kan följa med när den sparade banan paketeras.
Paketet är kumulativt och innehåller även kamera/zoom, adresser och informationspunkter.

## Koppla ett hörn

Lägg en **TMOP Killer Branch Point** vid varje korsning eller gatuhörn där
du vill ha ett vägval. Använd samma **Network Id**, normalt `KillerEscape`.

| Inställning | Användning |
| --- | --- |
| Next Corners | Närliggande hörn som går att springa till härifrån. Lägg in en referens per riktning med pipetten. |
| Branch When Original Passes | På för hörn längs originalets flyktväg. Av för hörn som bara spökversionerna besöker. |
| Original Next Corner | Nästa hörn på originalets ordinarie väg. Den riktningen får inget extra spöke när originalet passerar. |
| Original Previous Corner | Föregående hörn på originalets väg. Hindrar en omedelbar förgrening tillbaka dit. |
| Arrival Radius Cm | Hur nära originalet behöver komma, normalt 90 cm. |

Exempel: vid hörn A går originalets väg till B och en alternativ väg går till C.
Sätt **Next Corners = B, C** och **Original Next Corner = B**. När originalet
når A springer ett spöke mot C. Om C sedan har **Next Corners = D, E** delar
spöket där upp sig mot D och E. Lägg nya fortsättningar vid D och E för fler led.

Pilar visas i editorn: vit pil för den angivna originalriktningen, cyan för
övriga kopplingar. Pilarna visar kopplingen, inte den exakta navigeringsvägen.
Länkar är riktade; lägg bara till en bakåtlänk där den faktiskt ska tillåtas.
Varje enskild spökväg får inte återbesöka ett hörn den redan passerat. Två
olika spökvägar kan däremot nå samma plats som separata möjligheter.

Systemet känner igen **utplacerade förgreningspunkter**, inte alla svängar i
navmesh automatiskt. Navmesh används för vägen mellan punkterna. Koppla
grannhörn och sätt ut mellanpunkter om en väg annars passerar en korsning där
du vill kunna grena av. Ofullständiga eller saknade navigeringsvägar hoppas
över. Nätet är avsett för gång/löpning på mark och trappor; särskilda hopp,
hissar och interaktiva dörrpassager behöver en egen lösning.

## Tid och loopar

Förgrening är tillåten från **Escape Event Id + Escape Delay Seconds** på
directorn, normalt `Palme_shot_1 + 4 sekunder`. Vanligtvis registreras tiden
när originalet når hörnet. Därefter beräknas varje spökvägs ankomsttider från
sträckan längs navmesh och **Ghost Run Speed**, normalt 450 cm/s.

Rörelse och animationsfas följer spelets exakta klocka. Att pausa klockan
stoppar dem. Bakåtspolning och framåtspolning visar rätt läge för de
hörnpassager som redan registrerats under samma loop. En ny loop rensar
spöken och registrerade passager. Teleportering vid ett tidshopp räknas inte
som en passage genom alla mellanliggande hörn.

För **direkta tidshopp förbi hörn som ännu inte observerats**: aktivera
**Use Shared Event Arrival** på det aktuella originalhörnet och ange
**Arrival Event Id / Arrival Event Offset Seconds**. Tiden ska motsvara
originalets ankomst till hörnet. Scriptet fyller i förslag från den granskade
tidslinjen för trappan och mötesankarna, men lämnar funktionen avstängd.
Kontrollera förslagen mot din aktuella tidslinje innan du aktiverar dem.
Det krävs att originalet finns tillgängligt för att kopiera utseendet, eller
att ett utseende redan har fångats under loopen.

## Utseende och gränser

| Director-inställning | Standard |
| --- | --- |
| Ghost Opacity | 0,22. Lägre är mer genomskinligt. |
| Ghost Color | Ljusblå. |
| Fade Seconds | 0,4 sekunder vid första uppkomsten och sista vägsträckans slut. |
| Run Animation | Scriptet väljer `A_TMOP_RunFast`. Välj en löpcykel som passar gärningsmannens skelett. |
| Animation Play Rate | 1. Justera tillsammans med löphastigheten om fötterna glider. |
| Max Concurrent Ghosts | 24 synliga spöken. |
| Max Branch Depth | 8 vägsträckor från varje originalhörn. |
| Max Scheduled Legs | 512 vägsträckor totalt. |

Det finns även en fast gräns på 128 hörnpunkter per nät. När en gräns nås
begränsas ytterligare alternativ i stabil ordning och en summering skrivs i
Output Log. En återvändsgränd, djupgräns eller kapad fortsättning avslutar
spökvägen med uttoning. Originalet fortsätter sin egen rutt.

Välj **Validate Network** på directorn för att jämföra antal författade länkar
med hittade kompletta navmesh-vägar. Korrigera saknade länkar och kontrollera
material/animation. Håll hela förgreningsnätet laddat under spelrundan.
Efter ändrad navmesh, punkter, länkar, hastighet eller gränser i Play: anropa
**Rebuild Network** och spela om flykten. Detta rensar registrerade passager.

## Verifiering

21 fristående C++-kontroller godkända: förgrening i flera led, undantag för
originalriktningen, återvändning/cykler, återförenade vägar, tider, antals- och
djupgränser samt upprepad sampling framåt/bakåt. Python-scriptets syntax är
kontrollerad. **Unreal-bygge, rendering, navmesh och scriptkörning i editorn
är inte testade här eftersom Unreal Editor/SDK saknas.**

Prova först A→B/C och C→D/E i en liten del av banan. Kontrollera att bara
originalet påverkar de historiska händelserna, att spökenas kläder och
löpanimation ser rätt ut, att trappornas markhöjd fungerar, samt att paus,
tidshopp och ny loop ger rätt antal spöken. Kontrollera också paketbygget
efter att material, animation och nivå har sparats.

API-underlag: [Unreal NavigationSystem](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/NavigationSystem/UNavigationSystemV1),
[Single Node Animation](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/UAnimSingleNodeInstance)
och [MaterialEditingLibrary](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/MaterialEditingLibrary?application_version=5.6).
