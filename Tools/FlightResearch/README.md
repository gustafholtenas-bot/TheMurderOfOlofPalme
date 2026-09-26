# Flygtrafik kring mordet – första forskningsurvalet

Ny flik **Flygtrafik ±24 h** i **Grupperingar i världen**. Rutter ritas på den befintliga globen och flygsymboler rör sig mellan avgångs- och ankomsttider. Spela/pausa, tidsreglage, tre hastigheter, land/bolag/sökning, lista över flyg i luften och klickbara källor ingår. Gränssnitt och anmärkningar finns på svenska och engelska.

**Detta är inte en fullständig rekonstruktion av världens faktiska flygtrafik.** Det är ett första källbelagt urval och ett arbetsflöde för att fylla på. Tidtabeller visar planerad trafik; genomförande, passagerare, last, förseningar och inställda flyg kräver andra handlingar. En rutt innebär ingen belagd anknytning till mordet.

## Installation

ZIP-paketets mappar `Plugins/` och `Tools/` hör direkt till projektroten, bredvid `.uproject`. Paketet innehåller också föregående börsmeny, eftersom gemensamma meny- och byggfiler bygger vidare på den versionen. Det är en ändringsleverans, inte ett helt Unreal-projekt.

1. Stäng Unreal Editor och lägg filerna i projektet med bibehållen mappstruktur. Om samma kodfiler har ändrats i din nyare version, slå ihop ändringarna i Git.
2. Bygg projektets **Development Editor** för UE 5.8. Live Coding räcker inte för att verifiera nya filer och ändrad paketering.
3. Öppna spelets **Grupperingar i världen → Flygtrafik ±24 h**. Inga nya Blueprint-variabler eller modeller behöver kopplas in.
4. Prova paus, tidsreglage, land/bolagsfilter, val av listpost/flygsymbol, återgång till andra världsflikar och byte svenska/engelska. Kontrollera även en paketerad build: `flights.json` deklareras som UFS i `TMOPEngine.Build.cs`.

UE-kompilering, spelkörning, layout i olika upplösningar och paketering har **inte** kunnat utföras i denna arbetsmiljö. De femton Python-testerna och kontrollen av genererad JSON har körts. Tre Unreal-automationstester under `TMOP.Flights` är tillagda men behöver köras i Editor. Befintlig börsdatavalidering ska också köras vid ändringar av det kumulativa paketet.

## Period och animation

Projektets ankare är **28 februari 1986 23:21:30 CET**, alltså 22:21:30 UTC. Perioden är därför:

| Gräns | Svensk tid, CET | UTC |
|---|---|---|
| −24 h | 27 feb 23:21:30 | 27 feb 22:21:30 |
| Mordtiden | 28 feb 23:21:30 | 28 feb 22:21:30 |
| +24 h | 1 mars 23:21:30 | 1 mars 22:21:30 |

Trafik räknas med när flygets tidsintervall överlappar perioden, även när avgången sker tidigare eller ankomsten senare. Tidtabellerna har minutprecision; ankarets sekunder gör inte flygtiderna mer exakta. Avgång ingår i intervallet, ankomst gör det inte.

Varje fysisk delsträcka har egna tider. Stopp innebär att flygsymbolen försvinner under markuppehållet. Anslutningsförslag och bil-/bussanslutningar skapar inga flygsträckor. En konstant hastighet längs en storcirkel illustrerar tidtabellen; modellen rekonstruerar inte verklig flygbana, flygledning, luftrumsrestriktioner, vind eller flyghöjd.

Flygklockan är separat per öppnad lokal spelares världsmeny. Den ändrar inte speltid, NPC:er, fordon eller sparfiler. Uppspelning pausas när en annan världsflik väljs och stannar vid +24 h. Tidsläge och filter lever så länge menyinstansen finns. Ingen nätverksanslutning krävs för animationen. Källknappar öppnar webbläsaren.

## Vad som ingår nu

| Bolag | Transkriberade delar | Avgångar som överlappar perioden |
|---|---|---:|
| Golden Air | Direktsträckor Karlskoga–Bromma och gamla Karlstad–Fornebu | 12 |
| Pan Am | Urval från Frankfurt samt Tel Aviv–Paris; tryckta sidor 41 och 102 | 15 |
| Norcanair | Urval ur PDF-sidor 3–4 | 17 |
| Líneas Aéreas Paraguayas | Europeiska förbindelser via Recife, fysiska delsträckor | 10 |
| Ladeco | Urval av norra Chile | 18 |
| Air Nauru | Två tryckta direktsträckor; odaterade handändringar uteslutna | 2 |
| Air Wisconsin | S. 2–3 samt Muskegon och New Haven på s. 10–11 | 119 |
| Air Zimbabwe | Regionalt/inrikes urval fredag/lördag samt nattflyg till/från Gatwick | 44 |
| South African Airways | SA-kodade förbindelser i Air Zimbabwes tidtabell | 8 |
| British Airways | BA-kodad förbindelse Harare–Heathrow i Air Zimbabwes tidtabell | 3 |
| **Totalt** | **191 granskade återkommande tidtabellsrader** | **248** |

Alla 248 har status `scheduled`. **Noll avgångar har separat dokumentation om genomförande.** Underlaget omfattar 91 olika riktade flygplatspar och 52 flygplatser. Åtta tidtabellsutgåvor är delvis transkriberade. Air Zimbabwes utgåva bidrar även med SA- och BA-kodade förbindelser. Bolagstillhörigheten följer källans flygkod och belägger inte faktisk operatör vid eventuell inhyrning av flygplan.

Omgång 2 tillförde **174 avgångar och 53 riktade flygplatspar**. Air Wisconsin 2740 Muskegon–Battle Creek är sparat som `candidate`: s. 2 anger ankomst 16.06 och s. 10 anger 16.05. Posten animeras inte. Anslutningar och motstridiga rader räknas inte som ytterligare direktflyg. Detaljer finns i `research/batch_02.json`.

Källregistret har **18 poster**. Forskningskön har **66 bolag, 52 länder/territorier**, däribland separata dåtida Västtyskland, Östtyskland och Sovjetunionen. SAS hör till tre hemländer. Kön är inte en inventering av alla bolag som fanns 1986; `operator_census_complete` och `operator_census_verified` är tills vidare `false`.

Internet Archive har sökts separat för samtliga 66 registrerade bolag. Loggen innehåller **191 unika råträffar**, till stor del irrelevanta rapporter/tidningar eller bara omslag. Ett separat tidtabellsindex gav **147 unika kandidat-URL:er** från avsnitten 1985–1986. Detta är **inte** 147 verifierade, tillämpliga tidtabeller; många gäller senare perioder. Ingen råträff förs automatiskt över till kartan.

Bra fortsättningspunkter är den stora Pan Am-utgåvan och Delta 1 februari 1986, därefter SAS och Finnairs vinterutgåvor. SAA-indexets aprilutgåva 1986 får inte användas som bevis för februari. De nya SA-kodade sträckorna kommer i stället från Air Zimbabwes novemberutgåva 1985.

## Data och kod

| Fil/plats | Funktion |
|---|---|
| `Plugins/TMOPEngine/Content/WorldAtlas/Flights/catalog.json` | Redigerbart register: länder, bolag, historiska flygplatser, källor, tidtabellsrader, observationer och forskningsstatus |
| `Plugins/TMOPEngine/Content/WorldAtlas/Flights/flights.json` | Genererat, kontrollerat spelunderlag med individuella avgångar i UTC; redigera normalt katalogen och bygg om |
| `Tools/FlightResearch/build_flights.py` | Omvandlar lokala tidtabeller till det exakta 48-timmarsfönstret |
| `Tools/FlightResearch/discover_sources.py` | Återupptagbara sökningar på Internet Archive, land för land/bolag för bolag |
| `Tools/FlightResearch/research/archive_airline_searches.json` | Sökfrågor, sökdatum, resultat, fel och granskningsstatus |
| `Tools/FlightResearch/research/timetable_candidate_urls.json` | Kandidater från indexet över fullständigt skannade tidtabeller |
| `Tools/FlightResearch/research/batch_02.json` | Spårbar lista över tillagda rader och den undanhållna källkonflikten i omgång 2 |
| `Tools/FlightResearch/research/SOURCES.md` | Källöversikt med tillämpnings- och granskningsanmärkningar |
| `Private/WorldAtlas/Flights/` i TMOPEngine | C++-datamodell, klocka, filter, Slate-paneler, globritning och Unreal-tester |

`world.json` behöver inte ändras för nya flygbolag/rutter. Samma historiska lands-ID används när det är tillämpligt, men flygkatalogen kan även innehålla länder som inte har någon informationspunkt i `world.json`. Landfiltret matchar bolagets hemland **eller** avgångs-/ankomstlandet. Listan i källpanelen grupperar bolagen efter hemland.

## Fortsätt metodiskt

1. **Inventera bolagen i ett land år 1986.** Använd samtida bolagsregister och notera namnbyten, dotterbolag, charter och frakt. Fyll på kön innan landet markeras färdiginventerat. Markera aldrig landet färdigt enbart för att en sökning är tom.
2. **Sök vintertidtabell med rätt giltighet.** Spara URL, titel, datumintervall och åtkomsttyp. Ett omslag/katalogkort ger status `catalog_found`; en läsbar tabell ger `scan_found`. Kontrollera tryckta datum, fotnoter och ersättningsutgåvor. Okänd start/slut ska vara `null`, aldrig ett påhittat datum. En månad på omslaget sparas som `effective_from_month` (`YYYY-MM`); importeraren godtar då bara avgångsdatum efter den månadens slut, tills en exakt startdag har belagts.
3. **Läs teckenförklaringen och granska varje rad mot bilden.** OCR är ett sökhjälpmedel. Kontrollera vardagar, undantag, lokaltid, nästa dag, flygnummer, faktisk operatör, stopp, sommartid och anslutningar. Spara exakt sida/bild-ID. Håll osäkra rader som `candidate` eller `rejected`.
4. **Registrera historiska flygplatser.** Använd koordinater för den plats som användes 1986. Exempel: `KSD-OLD` är Karlstad–Jakobsberg, `FBU` är Fornebu och `YXD` är Edmonton Municipal, inte deras senare ersättare. Ange koordinatkälla och IANA-tidszon.
5. **Dela upp resor i fysiska delsträckor.** Ange `service_group`, `leg_index` från 1 och `service_day_offset` relativt resans första lokala avgångsdatum. `days` gäller just delsträckans lokala avgångsdag, inte första benets. LAP fredag Frankfurt–Bryssel–Recife fortsätter exempelvis lördag Recife–Asunción.
6. **Bygg och validera.** Godkänd källa, lokala tider, giltighet, veckodagar, undantag, datumgräns, koppling mellan delsträckor och dubbletter kontrolleras före spelimport. Konflikter mellan utgåvor/operatörer behöver avgöras manuellt. Räknaren avser fysiska delsträckor, inte unika flygplan eller hela resor.
7. **Komplettera med faktiska rörelsedata när de finns.** Flygplatsjournal, operatörslogg eller annan specificerad handling får källtypen `movement_record`. Först då kan en granskad observation ändra status till `confirmed` eller `cancelled`. Inställda avgångar sparas i `excluded` och animeras inte. Passagerare/last följer inte av tidtabellen.
8. **Redovisa luckor.** `partial` betyder delvis inläst, inte full täckning. `not_searched` avser kvarstående manuell källgranskning; automatiska sökningar redovisas separat i loggen. Ge varje nytt bolag permanenta ID:n och behåll källhänvisningarna vid rättningar.

Inget land markeras automatiskt klart. Privata, militära, frakt- och charterflyg kräver ofta andra arkiv än reguljära tidtabeller. Denna version saknar import för fristående, oschemalagda rörelseloggar; en sådan logg bör få ett separat flöde innan dessa flyg förs in, inte maskeras som en tidtabell.

## Kommandon från projektroten

Python 3.9 eller senare med IANA-tidszonsdatabas behövs för forskningsverktygen. På Windows kan tidszonsdatabasen installeras med `python -m pip install tzdata`. Spelet behöver varken Python eller tzdata: omräknade UTC-tider ligger redan i `flights.json`.

```sh
python Tools/FlightResearch/discover_sources.py --country se
python Tools/FlightResearch/discover_sources.py
python Tools/FlightResearch/build_flights.py
python Tools/FlightResearch/build_flights.py --check
python -m unittest discover -s Tools/FlightResearch -v
```

Sökverktyget hoppar över lyckade tidigare bolagssökningar. `--refresh` söker om urvalet. Misslyckade anrop loggas och kan köras igen; inga inloggningsuppgifter behövs. Verktyget arbetar med högst tre samtidiga sökningar, sparar efter varje bolag och anger om en sökning hade fler än 200 resultat. Det importerar inga flygrutter.

Tidszonerna är historiska IANA-regler, inte dagens UTC-offset. Recife och Santiago hade exempelvis sommartid i denna period, och Air Naurus Pago Pago-flyg passerar datumgränsen. Tvetydiga klockslag kräver explicit `departure_fold`/`arrival_fold`; obefintliga lokala tider stoppas. Publicerad runtime-JSON bör checkas in så att ändringar i en framtida tzdata-version blir synliga i Git.

## Språk och uppdateringar

Landnamn, metodtext, källanmärkningar och flyganmärkningar har `sv`, `en`, `en_source`. Ändras svenskan måste engelskan granskas och `en_source` uppdateras till den nya svenska texten. Annars faller spelet tillbaka på svenska. Externa språkpaket kan använda tabellen `TMOP_Flights`, stabilt land-/käll-/tidtabells-ID och fältet `name` eller `notes`; metodtexten använder rad `method`, fält `text`. Menytexterna använder det befintliga lokaliseringssystemet och 30 nya nycklar i `TMOPMenuTranslations.inl`.

## Verifiering och återstående arbete

Körda Python-tester täcker exakta periodgränser, Golden Airs vardagar, Pan Ams dagkoder, överlappande flyg, gamla flygplatser, datumgränsen, giltiga källor, dubbletter, datumundantag, evidenskrav för inställda/genomförda flyg, sommartidsgap/fold, halvtimmeszoner, LAP:s mellanlandningar och inaktuella översättningar. Nya tester täcker dessutom månad utan dag, Air Wisconsins källkonflikt och tidszonsskifte samt Air Zimbabwe/BA:s nattflyg. De 30 menyöversättningarnas svenska källtexter och formatplatshållare är kontrollerade.

Unreal-testerna täcker luftburen/landad-gränsen, hastighet, spelarisolerad klocka, sluttid, dataimport, land/bolagsfilter och språkfallback. De har inte körts här. Grafiska tester, klickytor, split-screen, prestanda och paketerad build återstår i UE. Listan är virtualiserad och likadana rutter ritas en gång, men prestanda med världsomfattande tiotusentals rörelser är ännu inte verifierad.

Transkriptionerna är källbaserade, inte dubbelt oberoende granskade. Nästa datasteg är att läsa klart de åtta påbörjade utgåvorna, granska de funna vinterutgåvorna och fortsätta komplettera bolagsinventeringen land för land. Inga fullständiga skanningar återdistribueras i kodpaketet; källorna länkas.
