# Teoribygge – installation och användning

Detta paket innehåller uppdaterade källfiler till den befintliga TMOPEngine-pluginen,
inklusive de tidigare levererade meny-, anteckningsboks- och ankomständringarna.
Det är inte en komplett plugin eller en färdigkompilerad DLL.

## Installera

1. Stäng Unreal Editor och kopiera paketets `Plugins` till projektets rot. Ersätt
   motsvarande filer och behåll resten av pluginen.
2. Generera om Visual Studio-projektfiler vid behov.
3. Bygg projektets Editor-target i Development Editor / Win64. Nya USTRUCT- och
   UCLASS-typer kräver en vanlig byggning med Unreal Header Tool, inte enbart Live Coding.
4. Öppna spelets pausmeny och välj **TEORIBYGGE**, direkt under **MINA TEORIER**.

## Mallarna

Mallarnas roller, gruppering och relativa placering följer de fyra inskickade bilderna.
Personrutor är stående och fordonsrutor liggande på en vit arbetsyta.

| Mall | Personrutor | Fordonsrutor | Innehåll |
| --- | ---: | ---: | --- |
| Ensam gärningsman | 1 | 0 | Gärningsmannen |
| Liten konspiration | 5 | 2 | Ledare, tre kumpaner och gärningsmannen |
| Stor konspiration | 14 | 5 | Dessutom uppdragsgivare, mellanhand, fyra spanare i hjälpgruppen och tre personer utanför gruppen |
| Stor konspiration + övervakning | 19 | 7 | Dessutom SÄPO-gruppens ledare, fyra spanare och två fordon |

Grupprubrikerna är också flyttbara och redigerbara. De är mallens rollbeteckningar,
inte slutsatser om historiska personers inblandning. Rutorna börjar utan linjer,
precis som i skisserna. Spelaren väljer själv sambanden.

Varje klick på en mall skapar ett nytt träd. Befintliga träd väljs i rullistan och
skrivs inte över när en ny mall väljs. Träd kan namnges, dupliceras och raderas.
Radering av hela träd kräver ett andra klick och kan ångras under samma besök.

## Redigera

- Klicka på en ruta och välj **Välj från Mina observationer** i panelen nedanför
  arbetsytan. Personrutor visar personer och fordonsrutor visar fordon. En observation
  kan användas en gång per träd och återanvändas i andra träd.
- **+ Person**, **+ Fordon**, **+ Anteckning** lägger till rutor. Rubriker/roller och
  anteckningar redigeras i panelen. Insamlade personers och bilars namn kommer från
  observationen och ändras inte när rollen i teorin ändras.
- Vänster musknapp flyttar rutor. Höger- eller mittknappen panorerar. Mushjulet zoomar.
  **Visa hela trädet** passar in arbetsytan i rutan.
- **Dra linjer**: klicka på två rutor. Klicka på en linje för att namnge sambandet.
- **Ta bort markerad** eller Delete tar bort en ruta/linje. När en ruta tas bort
  försvinner dess anslutna linjer också.
- **Ångra/Gör om** har upp till 50 steg under det aktuella besöket i sidan.
  Ctrl+Z, Ctrl+Y och Ctrl+Shift+Z fungerar när arbetsytan har tangentbordsfokus.
- **Töm rutan** tar bort en tilldelad observation men behåller rutan och dess roll.

Skyttplatsen kan flyttas och antecknas men inte raderas eller tilldelas en annan
person. Den fylls när en insamlad person tillhör kategorin **Skytten**. Inspektion
av `THE_KILLER` ger automatiskt den kategorin. Även redan skapade träd uppdateras
när personen samlas in. Före upptäckten visar platsen att skytten inte är hittad.

## Observationer av fordon

E på ett grönt observerat fordon öppnar en fordonsakt. När akten stängs samlas
fordonet in och en notis visas. Samma fordon läggs inte in flera gånger.
**Mina observationer** behåller de fem personkategorierna och har dessutom **FORDON**.

Fordon kvalificerar när deras kategori eller ID börjar med `OBSERVED_`, samma regel
som deras gröna namnskylt. Fordonet behöver ett stabilt, icke-tomt `VehicleId`.
Vanliga vittnen och vanliga bilar samlas inte in. E på vanliga bilar behåller det
befintliga instigningsbeteendet; på gröna observerade bilar prioriteras fordonsakten.

## Spara och ladda

Alla träd, valda observationer, roller, anteckningar, linjer, positioner, panorering,
zoom och valt träd ingår i spelets ordinarie sparning, separat för varje lokal spelare.
**Spara träd och spel** skapar en ny manuell sparning. Om sparplatserna är fulla,
använd **SPARA/LADDA** för att skriva över eller radera en befintlig sparning.

Ändringar ligger kvar när man byter menysida, men måste sparas innan spelet avslutas.
Ångrahistoriken sparas inte. Äldre sparfiler får en tom trädsamling och deras befintliga
observationer tolkas som personer, vilket bevarar tidigare beteende.

## Informationsrubriker och brödtexter

Under trädet finns två separata listor: **ENSAM GÄRNINGSMAN** och **KONSPIRATION**.
Endast rubriken visas tills den fälls ut. Då visas brödtext och eventuell källhänvisning.
Startposternas rubriker kommer från skisserna. Ingen källtext har hittats på: tomma
brödtexter visas som **Texten är inte inlagd ännu**.

Använd befintlig Blueprint-underklass av `TMOPPauseMenuWidget`, eller skapa en sådan
och välj den som **Pause Menu Widget Class** på spelarens Blueprint. I widgetens
Class Defaults finns **TMOP → UI → Pause → Theories → Theory Information Entries**.
Varje post har:

- `Track`: LoneGunman eller Conspiracy.
- `SortOrder`: ordning i listan.
- `Title`: klickbar rubrik.
- `Body`: brödtext med stöd för radbrytningar.
- `Source`: källhänvisning.

Alternativt skapa en Data Table med radtypen **TMOPTheoryInformationRow** och tilldela
**Theory Information Table** i samma widget. En giltig tilldelad tabell ersätter
widgetens egen lista. Inget historiskt sakpåstående om exempelvis antal gärningsmän
eller bilar har lagts till utan underlag.

## Verifiering

Körda här: tre godkända kontroller – de två befintliga menytesterna och ett portabelt
C++-test som kompilerar och kör den faktiska trädmodellen mot en begränsad typadapter.
Det kontrollerar alla fyra mallar, upptäckt, typbegränsning, dubbletter, linjer,
borttagning och återställning från ångra-kopior. Adaptern ersätter inte Unreal.

Tillagda Unreal Automation-tester finns under **TMOP.TheoryBuilder** och täcker
också sparning/laddning med noter, roller, linjer, fordonsidentitet och isolering
mellan två lokala spelare. De har inte körts här. Ingen Unreal-kompilering, UHT-
kontroll eller visuell PIE-testning har kunnat göras i denna miljö.

Testa efter byggning: samla en person och ett grönt fordon, placera dem i ett träd,
skriv anteckningar, dra linjer, spara/ladda och kontrollera återställningen. Skapa
också ett träd före upptäckten av skytten och kontrollera att platsen fylls efteråt.
