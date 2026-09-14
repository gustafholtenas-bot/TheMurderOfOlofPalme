# Adressinteraktion – 2026-09-12

Den här uppdateringen kopplar det befintliga adressregistret till spelaren.
Sikta på en kopplad adress vid dörrskyltshöjd: target-markeringen visar adressen.
Inom interaktionsavståndet visas **[E] Läs boendeförteckning**. E öppnar en
scrollbar lista. E, Esc eller **Stäng** stänger den. Muspekaren visas medan
listan är öppen och spelarens rörelse och föremålsanvändning spärras.
Simuleringens klocka fortsätter gå.

## Installera

1. Stäng Unreal Editor. Packa upp ZIP-filens `Plugins` och `Scripts` i
   projektroten, bredvid `TheMurderOfOlofPalme.uproject`. Slå ihop mapparna.
2. Bygg om projektets **Development Editor** och öppna editorn igen.
   Uppdateringen innehåller nya reflekterade C++-klasser; använd en vanlig
   editorbuild och omstart. Ingen Widget Blueprint behöver skapas eller tilldelas.
3. Öppna spelbanan med de befintliga adressankarna. Stoppa Play och ladda de
   sublevels/World Partition-regioner som ska kopplas. Spelaren ska vara
   `BP_TMOPPlayerCharacter`, med `TMOPPlayerCharacter` som C++-förälder.
4. Kontrollera att den fullständiga filen finns i
   `DataTables/09_13/DT_TMOP_AddressRegistry.json` och kör sedan
   **Tools → Execute Python Script → `Scripts/tmop_install_address_components.py`**.
   Aktivera **Python Editor Script Plugin** om Python-menyn saknas.
5. Scriptet uppdaterar den befintliga DataTable-asseten, kopplar alla säkra
   träffar och sparar tabellen och den aktuella banan. Läs sammanfattningen i
   Output Log och rapporten i `Saved/TMOP/Reports/`. Om automatisk sparning
   rapporteras som ofullständig, välj **Save All**.

Scriptet använder `/Game/TMOP/Data/DT_TMOP_AddressRegistry` och den medföljande
JSON-filen med 334 adressrader. Tabellen måste ha radtypen
`TMOPAddressRegistryRow`. Asseten ersätts inte: raderna uppdateras i samma
DataTable så att befintliga komponentreferenser fortsätter fungera. Manuellt
skapade `EntranceAnchorId`, `BuildingAnchorId` och `DoorbellActorTag` bevaras
när motsvarande värde är tomt i JSON-filen. Rader som bara finns i DataTable
bevaras också. En JSON-backup av tabellen skrivs före varje körning.

## Vad scriptet kopplar

Scriptet söker bland redan utplacerade `TMOPHistoricalAnchor` i den laddade
banan. Det skapar eller flyttar inga ankare. Matchningsordningen är:

1. Explicit `EntranceAnchorId`, annars `BuildingAnchorId`.
2. En redan kopplad adresskomponent för samma tabellrad.
3. Adressradens `DoorbellActorTag` på ankaret.
4. Exakt adress, `RegistrySearchText`, AddressId eller radnamn mot ankarets ID,
   label, display name eller tags, normaliserat för mellanslag, understreck och
   svenska bokstäver. Tekniska prefix som `ADDRESS_`, `DOORBELL_` och `ANCHOR_`
   får tas bort, men resten måste fortfarande vara en exakt adress.

`5–7` hålls skilt från `57`, och `12A` hålls skilt från `12` och `12B`.
Scriptet gissar inte utifrån närmaste position eller delar av ett namn.
En saknad explicit ankarlänk ersätts inte med en annan automatisk träff.

En entydig träff får en sparbar `TMOPAddressComponent` med rätt `Registry`
och `RowName`. Adressradens `EntranceAnchorId` fylls i och ankarets
`Actor Enable Collision` aktiveras så att träffytan kan hittas. Befintliga andra
kopplingar, dubbletter eller konflikter lämnas för manuell rättning.
Scriptet kan köras igen utan att skapa extra komponenter; handjusterad
`InteractionOffset` och övriga komponentinställningar bevaras.

Rapporten sparas i `Saved/TMOP/Reports/address_components_<tid>.json` och
redovisar varje adress: kopplad, redan kopplad, saknad, tvetydig eller konflikt.
Den visar också registermergen och om allt sparades. Backupen heter
`address_registry_before_<tid>.json`. Scriptet räknar bara laddade ankare.
Ladda fler delar av banan och kör igen vid behov. För en ren förhandskontroll,
sätt `DRY_RUN = True`; sätt `AUTO_SAVE = False` om du vill granska innan du sparar.

## Höjd, placering och namn

Adresskomponenten skapar vid Play en osynlig träffyta, normalt **140 cm ovanför
ankaret**, med radie **45 cm**. Den stoppar inte spelare eller NPC:er och
påverkar inte navmesh. Target-systemet använder samma punkt för markeringen
och sikttestet. Interaktionsavståndet mäts från spelaren, även med tredjepersonskamera.
Standardvärdet är spelarens `InteractionDistance`, normalt 300 cm.

Om ankaret redan ligger vid ringklockan, sätt komponentens `InteractionOffset`
till `(0, 0, 0)`. Om det ligger vid sidan av dörren, justera komponentens
`InteractionOffset` i Details före Play; flytta inte ett ankare som används av
NPC-rutter. Scriptet slår på ankarets **Actor Enable Collision** för träffytans
sökning. Komponentens **Interaction Enabled** kan stänga av just adressläsningen.

Listan visar våning och lägenhet där sådana uppgifter finns, annars exempelvis
”Våning okänd”. För- och efternamn visas som initialer som standard, till
exempel ”J. A.”. Ett uttryckligen valt visningsnamn kan visa förnamnet fullt,
men efternamnet förkortas även där. ”Familjen A.” används endast för bekräftad
familj. Födelsedata visas inte.

## Kontroller

Lokalt godkända tester täcker adressmatchning, registermerge, Python-syntax och
diffkontroll.
Testerna täcker bland annat portbokstäver, nummerintervall, tvetydiga ankare,
prioritet för explicita länkar, bevarade befintliga kopplingar och att flera
adresser inte tilldelas samma ankare.

Unreal Editor/UE 5.8 finns inte i byggmiljön här. C++-koden är därför inte
kompilerad eller speltestad här, och scriptet har inte körts mot din öppna bana.
Automationstesterna `TMOP.Address.Display` och `TMOP.Address.Component` finns
för körning i Unreal efter kompilering.

Efter körningen ska summan av `connected` och `already_connected` motsvara alla
adressrader vars ankare är laddade. `missing` betyder att något säkert matchande
ankare inte fanns i de laddade nivådelarna; scriptet skapar ingen position genom
gissning. Rätta ankarnamnet eller koppla raden manuellt i TMOP Address Editor.

Prova i Play: sikta på en adress, gå inom tre meter, öppna och scrolla listan,
stäng med E/Esc/knappen, öppna en annan adress, och kontrollera att personer och
fordon fortfarande går att välja. Prova även en skymd adress bakom en annan
byggnad samt en adress utom räckhåll. Kör scriptet en andra gång och kontrollera
att komponentantalet är oförändrat. Spara och öppna banan igen för att kontrollera
att kopplingarna följer med.

Python-testkommando utanför Unreal:

```text
python -m unittest discover -s Scripts/tests -p test_address_component_matching.py -v
```

API-referens för scriptets tabelläsning:
[Epic: DataTableFunctionLibrary](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/DataTableFunctionLibrary?application_version=5.6).
