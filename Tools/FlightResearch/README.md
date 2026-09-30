# Flygtrafik kring mordet – första forskningsurvalet

Egen kategori **FLYGTRAFIK KRING MORDET** i pausmenyn. Rutter ritas på den befintliga globen och flygplanssilhuetter rör sig mellan avgångs- och ankomsttider och vrider sig längs rutten. Spela/pausa, tidsreglage, tre hastigheter, land/bolag/sökning, lista över flyg i luften och klickbara källor ingår. Gränssnitt och anmärkningar finns på svenska och engelska.

**Detta är inte en fullständig rekonstruktion av världens faktiska flygtrafik.** Det är ett första källbelagt urval och ett arbetsflöde för att fylla på. Tidtabeller visar planerad trafik; genomförande, passagerare, last, förseningar och inställda flyg kräver andra handlingar. En rutt innebär ingen belagd anknytning till mordet.

## Installation

ZIP-paketets mappar `Plugins/` och `Tools/` hör direkt till projektroten, bredvid `.uproject`. Paketet innehåller också föregående börsmeny, eftersom gemensamma meny- och byggfiler bygger vidare på den versionen. Det är en ändringsleverans, inte ett helt Unreal-projekt.

1. Stäng Unreal Editor och lägg filerna i projektet med bibehållen mappstruktur. Om samma kodfiler har ändrats i din nyare version, slå ihop ändringarna i Git.
2. Bygg projektets **Development Editor** för UE 5.8. Live Coding räcker inte för att verifiera nya filer och ändrad paketering.
3. Öppna spelets **FLYGTRAFIK KRING MORDET**. Inga nya Blueprint-variabler eller modeller behöver kopplas in.
4. Prova paus, tidsreglage, land/bolagsfilter, val av listpost/flygsymbol, återgång till andra världsflikar och byte svenska/engelska. Kontrollera även en paketerad build: `flights.json` deklareras som UFS i `TMOPEngine.Build.cs`.

UE-kompilering, spelkörning, layout i olika upplösningar och paketering har **inte** kunnat utföras i denna arbetsmiljö. De tjugo Python-testerna och kontrollen av genererad JSON har körts. Tre Unreal-automationstester under `TMOP.Flights` är tillagda men behöver köras i Editor. Befintlig börsdatavalidering ska också köras vid ändringar av det kumulativa paketet.

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

Senaste paketet **v79**: 64 nya planerade flygrörelser: 52 COMAIR och 12 Delta. 43 nya nonstop-scheman från 62 granskade rader. Totalt 4 883 rörelser. Indianapolis och Port Columbus tillkommer som flygplatser. Alla 4 819 tidigare rörelseposter och kodrättelserna från v77/v78 bevaras. Se `research/COMAIR_INDIANAPOLIS_COLUMBUS_BATCH76.md`. V77 och v78 var kodrättelser; nästa forskningsomgång är därför 76.

Föregående forskningspaket **v76**: 33 nya planerade COMAIR-rörelser: Dayton–Detroit, Cleveland–Milwaukee och Cleveland–Fort Wayne med retur, samt Detroit–Cleveland. 26 nya scheman från 60 granskade rader. Totalt 4 819 rörelser. Alla 4 786 tidigare rörelseposter, scheman och flygplatser är oförändrade. Se `research/COMAIR_OHIO_LINKS_BATCH75.md`.

Föregående paket **v75**: 73 nya planerade flygrörelser mellan Cincinnati, Cleveland och Dayton i båda riktningarna: 60 COMAIR och 13 Delta. 47 nya scheman från 58 granskade rader. Totalt 4 786 rörelser. Alla 4 713 tidigare rörelseposter och scheman är oförändrade. Cleveland Hopkins (CLE) och Dayton International (DAY) tillkommer; tidigare flygplatser bevaras. Se `research/OHIO_TRIANGLE_BATCH74.md`.

Föregående paket **v74**: 33 nya planerade COMAIR-rörelser: Cincinnati–Toronto, Cincinnati–St. Louis och Chicago O’Hare–Milwaukee med retur, samt Milwaukee–Fort Wayne och Fort Wayne–Detroit. 22 nya scheman från 23 granskade rader. Totalt 4 713 rörelser. Alla 4 680 tidigare rörelseposter och scheman är oförändrade. Toronto Pearson (YYZ) tillkommer; övriga flygplatser bevaras. Se `research/COMAIR_TORONTO_STLOUIS_CHICAGO_BATCH73.md`.

Föregående paket **v73**: 56 nya planerade COMAIR-rörelser mellan Cincinnati och Detroit, Milwaukee, Fort Wayne samt Toledo, i båda riktningarna. 35 nya scheman från 53 granskade rader. Totalt 4 680 rörelser. Alla 4 624 tidigare rörelseposter, scheman och flygplatser är oförändrade. Se `research/COMAIR_CINCINNATI_BATCH72.md`.

Föregående paket **v72**: 54 nya planerade Delta-returrörelser till Cincinnati på tolv riktade sträckor. 24 nya scheman från 87 granskade rader; anslutningar, rader med mellanstopp och COMAIR-rader hålls utanför denna import. BOS849 och SFO850 börjar 1 mars och ger en rörelse vardera. Totalt 4 624 rörelser. Alla 4 570 tidigare rörelseposter, scheman och flygplatser är oförändrade. Delta har nu 897 rörelser. Se `research/DELTA_CINCINNATI_RETURNS_BATCH71.md`.

Föregående paket **v71**: 45 nya planerade Delta-rörelser från Cincinnati på tolv riktade sträckor. 23 nya scheman från 83 granskade rader; anslutningar, rader med mellanstopp, COMAIR-rader och DL850 utanför fönstret hålls utanför denna import. DL849 till San Francisco börjar 1 mars och ger en rörelse. Totalt 4 570 rörelser. Alla 4 525 tidigare rörelseposter, scheman och flygplatser är oförändrade. Delta har nu 843 rörelser. Se `research/DELTA_CINCINNATI_OUTBOUND_BATCH70.md`.

Föregående paket **v70**: 46 nya planerade Delta-rörelser: returer till Atlanta från Austin Mueller, Fort Wayne, Honolulu, Little Rock, San Antonio och Toledo samt Atlanta–Cincinnati i båda riktningarna. 22 nya scheman från 52 granskade källrader; 21 anslutningar och nio rader med mellanstopp hålls utanför. Totalt 4 525 rörelser. Alla 4 479 tidigare rörelseposter och scheman är oförändrade. CVG tillkommer; tidigare flygplatsposter bevaras. Delta har nu 798 rörelser. Se `research/DELTA_ATLANTA_CINCINNATI_BATCH69.md`.

Föregående paket **v69**: 47 nya planerade Delta-rörelser till Atlanta från Kansas City, Minneapolis/St Paul, New Orleans, Oklahoma City, St Louis och Tulsa. 23 nya scheman från 35 granskade källrader; 12 anslutningar hålls utanför. Totalt 4 479 rörelser. Alla 4 432 tidigare rörelseposter, scheman och flygplatser är oförändrade. Delta har nu 752 rörelser. Se `research/DELTA_CENTRAL_RETURNS_BATCH68.md`.

Föregående paket **v68** tillför **57 planerade Delta-rörelser** till Atlanta från västra USA och Houston. Totalt **4 432 rörelser**, varav 4 431 planerade och en tidigare bekräftad. Alla 4 375 tidigare rörelseposter, scheman och flygplatser är oförändrade. 26 nya scheman från 42 granskade källrader; anslutningar, genomgående rader och framtida DL68 hålls utanför. Delta har nu 705 rörelser. Se `research/DELTA_WESTERN_RETURNS_BATCH67.md`, `research/delta_western_returns_batch67.tsv` och `research/validation_batch67.json`.

Föregående paket **v67** tillför **123 planerade Delta-rörelser** till Atlanta från Boston, Chicago O’Hare, Dallas/Fort Worth, Detroit, Miami, Philadelphia, Tampa och Washington National. Totalt **4 375 rörelser**, varav 4 374 planerade och en tidigare bekräftad. Alla 4 252 tidigare rörelseposter, scheman och flygplatser är oförändrade. 59 nya scheman från 80 granskade källrader; tolv redan importerade rader matchas utan dubbelimport. Delta har nu 648 rörelser. Se `research/DELTA_ATLANTA_RETURNS_BATCH66.md`, `research/delta_atlanta_returns_batch66.tsv` och `research/validation_batch66.json`.

Föregående paket **v66** tillför **123 planerade Delta-rörelser** mellan Atlanta och Baltimore, Bradley/Hartford, LaGuardia, Newark och Pittsburgh, med separat granskning av båda riktningarna. Totalt **4 252 rörelser**, varav 4 251 planerade och en tidigare bekräftad. Alla 4 129 tidigare rörelseposter och scheman är oförändrade. 55 nya scheman från 56 källrader; en anslutning via LaGuardia hålls utanför som obruten sträcka. BWI, BDL, LGA, EWR och PIT tillkommer. Delta har nu 525 rörelser. Se `research/DELTA_NORTHEAST_BATCH65.md`, `research/delta_northeast_batch65.tsv` och `research/validation_batch65.json`.

Föregående paket **v65** tillför **137 planerade Delta-rörelser** mellan Atlanta och fem Florida-flygplatser, med separat granskning av båda riktningarna. Totalt **4 129 rörelser**, varav 4 128 planerade och en tidigare bekräftad. Alla 3 992 tidigare rörelseposter och scheman är oförändrade. 67 nya scheman från 72 källrader; fem rader med mellanstopp eller trafik utanför perioden lämnas utanför. FLL, RSW, JAX, MCO och PBI tillkommer. Delta har nu 402 rörelser. Se `research/DELTA_FLORIDA_BATCH64.md`, `research/delta_florida_batch64.tsv` och `research/validation_batch64.json`.

Föregående paket **v64** tillför **185 planerade Delta-rörelser** från Atlanta på 23 nya riktade platspar. Totalt **3 992 rörelser**, varav 3 991 planerade och en tidigare bekräftad. Alla 3 807 tidigare rörelseposter och scheman är oförändrade. 87 nya scheman från 123 källrader; två otydliga tider och en framtida start lämnas utanför. Delta har nu 265 rörelser. Se `research/DELTA_ATLANTA_BATCH63.md`, `research/delta_atlanta_batch63.tsv` och `research/validation_batch63.json`.

Föregående paket **v63** tillför **32 planerade Southwest-rörelser** på åtta nya riktade platspar i Texas. En felaktig äldre lördagsrörelse WN736 tas bort efter trafikdagskontroll. Netto +31 och totalt **3 807 rörelser**, varav 3 806 planerade och en tidigare bekräftad. 3 775 tidigare rörelse-ID och tider bevaras; tre anteckningar uppdateras. 16 nya scheman från 358 källrader; inga nya flygplatser. Southwest har nu 1 219 rörelser. Se `research/SOUTHWEST_REMAINING_CITIES_BATCH62.md`, `research/corrections_batch62.json` och `research/validation_batch62.json`.

Föregående paket **v62** tillför **130 planerade Southwest-rörelser** på 21 nya riktade platspar i Kalifornien och kring Midway, Kansas City, St. Louis, Little Rock och Tulsa. Totalt **3 776 rörelser**, varav 3 775 planerade och en tidigare bekräftad. Alla 3 646 tidigare rörelse-ID och tider är bevarade. Två trafikdagsrättelser ändrar endast tre rörelsers anteckningar. 66 nya scheman från 244 källrader; Chicago Midway (MDW) tillkommer. Southwest har nu 1188 rörelser. Se `research/SOUTHWEST_CALIFORNIA_MIDWEST_BATCH61.md`, `research/corrections_batch61.json` och `research/validation_batch61.json`.

Föregående paket **v61** tillför **175 planerade Southwest-rörelser** kring Albuquerque och El Paso på 32 nya riktade platspar. Totalt **3 646 rörelser**, varav 3 645 planerade och en tidigare bekräftad. Alla 3 471 tidigare rörelse-ID finns kvar; två WN797-rörelser får rättad ankomst 00:05 nästa dag (tidigare 00:25). 94 nya scheman från 170 källrader. Kansas City International (MCI) tillkommer. Southwest har nu 1058 rörelser. Se `research/SOUTHWEST_ABQ_ELP_BATCH60.md`, `research/corrections_batch60.json` och `research/validation_batch60.json`.

Föregående paket **v60** tillför **248 planerade Southwest-rörelser** kring Phoenix och Las Vegas på 29 nya riktade platspar. Totalt **3 471 rörelser**, varav 3 470 planerade och en tidigare bekräftad. Alla 3 223 tidigare rörelser är bevarade. 132 nya schemarader efter dubblettkontroll av 149 källrader. Tre flygplatser tillkommer: historiska Denver–Stapleton, Las Vegas–McCarran och Ontario. Tulsa–Phoenix får endast belagd direkttrafik mot Phoenix. Southwest har nu 883 rörelser. Se `research/SOUTHWEST_WEST_BATCH59.md` och `research/validation_batch59.json`.

Föregående paket **v59** tillför **220 planerade Southwest-rörelser** på 12 förbindelser till/från Houston Hobby, alltså 24 nya riktade platspar. Totalt **3 223 rörelser**, varav 3 222 planerade och en tidigare bekräftad. Alla 3 003 tidigare rörelser är bevarade. 116 granskade rader och fyra flygplatser tillkommer; Dallas-flygen dubbleras inte. Southwest har nu 635 rörelser. Nästa större urval Phoenix/Las Vegas; Europakön behåller Cypern. Se `research/SOUTHWEST_HOUSTON_BATCH58.md` och `research/validation_batch58.json`.

Föregående paket **v58** tillför **415 planerade Southwest-rörelser** på 13 förbindelser till/från Dallas–Love Field, alltså 26 riktade platspar. Totalt **3 003 rörelser**, varav 3 002 planerade och en tidigare bekräftad. Alla 2 588 tidigare rörelser är bevarade. 235 granskade tidtabellsrader, ett nytt bolag och 14 historiskt placerade flygplatser tillkommer. Austin avser Mueller. Efter användarens nya prioritering granskas källor med många nya rutter worldwide; nästa större urval föreslås kring Houston Hobby/Phoenix/Las Vegas. Europakön behåller Cypern. Se `research/SOUTHWEST_DALLAS_BATCH57.md` och `research/validation_batch57.json`.

Föregående paket **v57** tillför **två planerade Pan Am-rörelser** Aten–Frankfurt och retur. Totalt **2 588 rörelser**, varav 2 587 planerade och en tidigare bekräftad. Alla 2 586 tidigare rörelser är bevarade. Grekland har 17 ändpunktsrörelser; Albanien har fortsatt källuckor. Nästa land **Cypern**, därefter Turkiet. Se `research/GREECE_ALBANIA_FOLLOWUP_BATCH56.md` och `research/validation_batch56.json`.

Föregående paket **v56** tillför **två planerade Pan Am-rörelser** mellan Zagreb och München-Riem: PA77 den 28 februari och PA76 den 1 mars 1986. Totalt **2 586 rörelser**, varav 2 585 planerade och en tidigare bekräftad helikoptertransport. Alla 2 584 tidigare rörelseposter och äldre schemarader är oförändrade. Två nya riktade flygplatspar tillkommer. Rumänien har tre ändpunktsrörelser, Bulgarien fem och Jugoslavien tolv. Alla tre länder är återgranskade med kvarstående luckor. Nästa land är **Albanien**, därefter Grekland. Se `research/BALKANS_FOLLOWUP_BATCH55.md`, `research/panam_zagreb_batch55.tsv` och `research/validation_batch55.json`.

Föregående paket **v55** tillför **tre planerade Pan Am-rörelser**: Warszawa–Frankfurt samt Budapest–Dubrovnik i båda riktningarna. Totalt **2 584 rörelser**, varav 2 583 planerade och en tidigare bekräftad helikoptertransport. Alla 2 581 tidigare rörelseposter och äldre schemarader är oförändrade. Dubrovnik–Čilipi tillkommer som markör i Jugoslavien och tre nya riktade flygplatspar tillkommer. Polen har sex ändpunktsrörelser, Ungern 22 och Jugoslavien tio. San Marino, Vatikanstaten, Malta och Tjeckoslovakien återgranskade med kvarstående luckor. Nästa land är **Rumänien**, därefter Bulgarien och Jugoslavien. Se `research/CENTRAL_EUROPE_FOLLOWUP_BATCH54.md`, `research/panam_central_europe_batch54.tsv` och `research/validation_batch54.json`.

Föregående paket **v54** tillför **två planerade Dan-Air-rörelser**: Gatwick–Innsbruck och tillbaka den 1 mars 1986. Totalt **2 581 rörelser**, varav 2 580 planerade och en tidigare bekräftad helikoptertransport. Alla 2 579 tidigare rörelseposter och äldre schemarader är oförändrade. Innsbruck–Kranebitten tillkommer som markör och två nya riktade flygplatspar tillkommer. Liechtenstein och Italien är återgranskade utan nya importklara tider. Österrike har tio ändpunktsrörelser, Italien 100 och Norden fortsatt 161. Nästa land är **San Marino**, därefter Vatikanstaten och Malta. Se `research/AUSTRIA_FOLLOWUP_BATCH53.md`, `research/austria_batch53.tsv` och `research/validation_batch53.json`.

Föregående paket **v53** tillför **13 planerade Dan-Air-rörelser** från elva granskade rader: Bern–Gatwick, Zürich–Gatwick och Zürich–Manchester, i båda riktningarna. Totalt **2 579 rörelser**, varav 2 578 planerade och en tidigare bekräftad helikoptertransport. Alla 2 566 tidigare rörelseposter och schemarader är oförändrade. Bern–Belp tillkommer som markör och sex nya riktade flygplatspar tillkommer. Östtyskland och Västberlin är återgranskade utan nya importklara tider. Schweiz har 165 ändpunktsrörelser och Norden fortsatt 161. Nästa land är **Liechtenstein**, därefter Österrike. Se `research/SWITZERLAND_FOLLOWUP_BATCH52.md`, `research/switzerland_batch52.tsv` och `research/validation_batch52.json`.

Föregående paket **v52** tillför **10 planerade Dan-Air-rörelser** från åtta granskade rader: Saarbrücken–Tegel och Gatwick–München-Riem, i båda riktningarna. Totalt **2 566 rörelser**, varav 2 565 planerade och en tidigare bekräftad helikoptertransport. Alla 2 556 tidigare rörelseposter och schemarader är oförändrade. Saarbrücken–Ensheim tillkommer som markör och fyra nya riktade flygplatspar tillkommer. Luxemburg är återgranskat utan nya importklara tider. Västtyskland har 327 ändpunktsrörelser och Norden fortsatt 161. Nästa land är **Östtyskland**. Se `research/WEST_GERMANY_FOLLOWUP_BATCH51.md`, `research/west_germany_batch51.tsv` och `research/validation_batch51.json`.

Föregående paket **v51** tillför **15 planerade rörelser**: tio Dan-Air-avgångar Amsterdam–Bristol/Tegel och fem tidsatta ben ur Sabenas januariutgåva, inklusive SN403 via Barcelona. Totalt **2 556 rörelser**, varav 2 555 planerade och en tidigare bekräftad helikoptertransport. Alla 2 541 tidigare rörelseposter och schemarader är oförändrade. Åtta nya riktade flygplatspar och en källpost tillkommer, utan nya flygplatser. Monaco, Andorra, Gibraltar och Portugal är återgranskade med luckor; Spanien får två nya delsträckor trots det saknade AF-uppslaget. Nederländerna har 117 och Spanien 61 ändpunktsrörelser; Norden fortsatt 161. Nästa land är **Luxemburg**. Se `research/EUROPE_FOLLOWUP_BATCH50.md`, `research/BELGIUM_FOLLOWUP_BATCH50.md` och `research/validation_batch50.json`.

Föregående paket **v50** tillför **74 planerade Air Inter-rörelser i Frankrike** från 69 granskade schemarader: regionala flyg mellan Bordeaux, Lyon, Marseille, Nice, Toulouse och Strasbourg samt Paris–Strasbourg. Totalt **2 541 rörelser**, varav 2 540 planerade och en tidigare bekräftad helikoptertransport. Alla 2 467 tidigare rörelseposter och schemarader är oförändrade. 26 nya riktade flygplatspar tillkommer; befintliga flygplatsmarkörer används. Dagfärger och flygnummer är granskade; äldre tidskonflikter kvarstår. Norden har fortsatt 161 unika ändpunktsrörelser. Nästa land är **Monaco**. Se `research/FRANCE_FOLLOWUP_BATCH49.md`, `research/france_batch49.tsv` och `research/validation_batch49.json`.

Föregående paket **v49** tillför **37 planerade rörelser med irländsk ändpunkt** från 28 granskade rader. Totalt **2 467 rörelser**, varav 2 466 planerade och en tidigare bekräftad helikoptertransport. Alla 2 430 tidigare rörelseposter och schemarader är oförändrade. Fyra flygplatser tillkommer: Cork–Ballygarvan, Shannon–Rineanna, Birmingham–Elmdon och East Midlands–Castle Donington. Aer Lingus egen vintertabell ger nya Dublin-, Cork- och Shannonflyg; Dan-Air tillför fyra avgångar. EI624 Dublin–Köpenhamn höjer Norden till 161 unika ändpunktsrörelser. Isle of Man är återgranskat utan importklara tider. Täckningen är ofullständig; nästa land är **Frankrike**. Se `research/IRELAND_FOLLOWUP_BATCH48.md`, `research/ireland_batch48.tsv` och `research/validation_batch48.json`.

Föregående paket **v48** tillför **54 planerade Dan-Air-rörelser** från 47 granskade nonstoprader: brittiska inrikesflyg och Kanalöarna. Totalt **2 430 rörelser**, varav 2 429 planerade och en tidigare bekräftad helikoptertransport. Alla 2 376 tidigare rörelser och schemarader är oförändrade. Fyra flygplatser tillkommer: Bristol–Lulsgate, Cardiff–Rhoose, Bournemouth–Hurn och Inverness–Dalcross. Tidsatta mellanlandningar bevaras; tre ofullständiga/motstridiga rader hålls utanför. Danmark, Finland och Island är återgranskade utan nya avgångar; Norden har fortsatt 160 unika ändpunktsrörelser. Storbritannien, Jersey och Guernsey har fördjupats med luckor kvar. Nästa land är **Isle of Man**. Se `research/EUROPE_FOLLOWUP_BATCH47.md`, `research/british_danair_batch47.tsv` och `research/validation_batch47.json`.

Föregående paket **v47** tillför **sju planerade flygrörelser** från Dan-Airs vintertabell: sex med DA-kod och en med SK-kod. Fem har norsk ändpunkt; två är brittiska ben på samma tjänster. Totalt **2 376 rörelser**, varav 2 375 planerade och en tidigare bekräftad helikoptertransport. Alla 2 369 tidigare rörelser och schemarader är oförändrade. Newcastle–Stavanger och Newcastle–Fornebu får båda riktningarna; Bergen–Gatwick får SK517. DA102/107 har 40 respektive 95 minuters markuppehåll i Newcastle. Norden har **160** unika ändpunktsrörelser, Norge **52**, Sverige oförändrat **87**. Tre nya källposter, inga nya flygplatser eller katalogkandidater. Sju anslutningsrader och två tidsjämförelser sparas som forskning utan extra animation. Nästa land är **Danmark**. Se `research/NORWAY_FOLLOWUP_BATCH46.md`, `research/SWEDEN_FOLLOWUP_BATCH46.md` och `research/nordic_danair_batch46.tsv`.

Föregående paket **v46** tillför **tolv planerade flygrörelser** med sovjetiska ändpunkter: Paris–Moskva, Moskva–Narita, Paris–Borispol och Paris–Pulkovo. Totalt **2 369 rörelser**, varav 2 368 planerade och en tidigare bekräftad helikoptertransport. Alla 2 357 äldre rörelser och schemarader är oförändrade. AF270 har två delsträckor med 75 minuters markuppehåll i Moskva. Borispol tillkommer som fysisk flygplatsmarkör; AFSU700 räknas en gång under AF. En ny källpost utan scheman och två dokumenterade datum-/veckodagsuteslutningar, inga nya katalogkandidater. Europaköns **40 poster har nu ett första granskat urval med luckor**; nästa fördjupning är **Sverige och Norden**. Se `research/SOVIET_UNION_COUNTRY_REVIEW_BATCH45.md`, `research/soviet_union_batch45.tsv` och `research/EUROPE_COUNTRY_INDEX.md`.

Föregående paket **v45** tillför **åtta planerade flygrörelser** mellan Istanbul–Atatürk och Paris: fyra AF till/från CDG, fyra TK till/från Orly. Sex nonstoprader i Air France nr 25 ger **totalt 2 357 rörelser**, varav 2 356 planerade och en tidigare bekräftad helikoptertransport. Alla 2 349 äldre rörelser och schemarader är oförändrade. Turkish Airlines registreras och får sina första animerade avgångar. TK928:s ankomst efter lokal midnatt hanteras med nästa-dagsflagga. Två Ankara-anslutningsrader sparas enbart som forskning. En ny källpost utan egna scheman, inga nya katalogkandidater eller flygplatsmarkörer. Nästa fördjupning är **Sovjetunionen**. Se `research/TURKEY_COUNTRY_REVIEW_BATCH44.md`, `research/turkey_batch44.tsv` och `research/EUROPE_COUNTRY_INDEX.md`.

Föregående paket **v44** tillför **elva planerade flygrörelser**: tio Aten–Frankrike och en Paris CDG–Larnaca. Elva nonstoprader i Air France nr 25 ger sju AF-grupperade och fyra OA-poster. **Totalt 2 349 rörelser**, varav 2 348 planerade och en tidigare bekräftad helikoptertransport. Alla 2 338 äldre rörelser och schemarader är oförändrade. Aten använder historiska Ellinikon. Cypern och Larnaca tillkommer; AF164 landar nästa dag och överlappar fönstrets slut. Cyprus Airways registreras utan animerade avgångar: sex läsbara förhandsrader sparas enbart som forskning. Tre nya källposter utan egna scheman; inga nya katalogkandidater. Nästa fördjupning är **Turkiet**. Se `research/GREECE_COUNTRY_REVIEW_BATCH43.md`, `research/CYPRUS_COUNTRY_REVIEW_BATCH43.md`, `research/greece_cyprus_batch43.tsv` och `research/EUROPE_COUNTRY_INDEX.md`.

Föregående paket **v43** tillför **åtta planerade flygrörelser**: fyra Belgrad–Orly och fyra Zagreb–Orly. Åtta nonstoprader i Air France nr 25 ger fyra AF- och fyra JU-poster. **Totalt 2 338 rörelser**, varav 2 337 planerade och en tidigare bekräftad helikoptertransport. Alla 2 330 äldre rörelser och schemarader är oförändrade. Belgrad–Surčin och Zagreb–Pleso tillkommer under det historiska landet Jugoslavien. JAT får sina första animerade avgångar; Inex-Adria köförs utan avgångar. Albanien registreras och granskas utan nya rörelser eftersom kompletta vintertider saknas. Fyra nya källposter utan egna scheman; inga nya kandidater. Nästa fördjupning är **Grekland**. Se `research/YUGOSLAVIA_COUNTRY_REVIEW_BATCH42.md`, `research/ALBANIA_COUNTRY_REVIEW_BATCH42.md`, `research/yugoslavia_batch42.tsv` och `research/EUROPE_COUNTRY_INDEX.md`.

Föregående paket **v42** tillför **åtta planerade flygrörelser**: tre Bukarest–Otopeni/Orly och fem Sofia/Orly, varav två fraktflyg. Åtta granskade nonstoprader ur Air France nr 25 ger två poster med AF-kod, två med RO-kod och fyra med första kod LZ. **Totalt 2 330 rörelser**, varav 2 329 planerade och en tidigare bekräftad helikoptertransport. Alla 2 322 äldre rörelser och schemarader är oförändrade. Otopeni och Sofia–Vrazhdebna tillkommer som markörer. TAROM och Balkan får sina första animerade avgångar; Bulgarien och Balkan är nya registerposter. Två nya forskningskällor utan egna scheman; inga nya kandidater. Två genomgående itinerarier hålls utanför animation eftersom delsträckstider saknas. Nästa fördjupning är **Jugoslavien**. Se `research/ROMANIA_COUNTRY_REVIEW_BATCH41.md`, `research/BULGARIA_COUNTRY_REVIEW_BATCH41.md`, `research/romania_bulgaria_batch41.tsv` och `research/EUROPE_COUNTRY_INDEX.md`.

Föregående paket **v41** tillför **sex planerade flygrörelser**: två Prag–Orly och fyra Budapest–Orly. Sex granskade nonstoprader i Air France nr 25 ger tre poster med första kod AF, två med MA och en med OK. Samarbetsrader räknas en gång; faktisk operatör är inte fastställd. **Totalt 2 322 rörelser**, varav 2 321 planerade och en tidigare bekräftad helikoptertransport. Alla 2 316 äldre rörelser och schemarader är oförändrade. Prag–Ruzyně tillkommer som markör; Tjeckoslovakien får två ändpunktsrörelser och Ungern har nu 20. En ny omslagskälla utan scheman; inga nya kandidater eller operatörer. Nästa fördjupning är **Rumänien**. Se `research/CZECHOSLOVAKIA_COUNTRY_REVIEW_BATCH40.md`, `research/HUNGARY_COUNTRY_REVIEW_BATCH40.md`, `research/czechoslovakia_hungary_batch40.tsv` och `research/EUROPE_COUNTRY_INDEX.md`.

Föregående paket **v40** tillför **två planerade Polen–Frankrike-rörelser** från två granskade rader i Air France nr 25: AFLO280 Orly–Warszawa och LOAF273 Warszawa–Orly, fredag 28 februari. Samarbetsrader räknas en gång under första koden; faktisk operatör är inte fastställd. **Totalt 2 316 rörelser**, varav 2 315 planerade och en tidigare bekräftad helikoptertransport. Alla 2 314 äldre rörelser och schemarader är oförändrade. Polen har fem ändpunktsrörelser. San Marino, Vatikanstaten och Malta registreras med dokumenterade källuckor; Air Malta registreras utan avgångar. Tre nya källposter, inga nya flygplatser eller kandidater. Nästa fördjupning är **Tjeckoslovakien (1986)**. Se `research/POLAND_COUNTRY_REVIEW_BATCH39.md`, de övriga landrapporterna för omgång 39, `research/poland_batch39.tsv` och `research/EUROPE_COUNTRY_INDEX.md`.

Föregående paket **v39** tillför **96 planerade Italien-rörelser** från 65 granskade rader ur Air France nr 25: 48 med AF-kod och 48 med AZ-kod. **Totalt 2 314 rörelser**, varav 2 313 planerade och en tidigare bekräftad helikoptertransport. Alla 2 218 äldre rörelseposter och schemarader är oförändrade. Sju italienska flygplatsmarkörer tillkommer; Alitalia får sina första animerade avgångar. Italien har 100 ändpunktsrörelser, varav två inrikes Genua–Neapel. Två nya källposter utan egna scheman; inga nya kandidater. Nästa fördjupning är **San Marino**, därefter Vatikanstaten och Malta. Se `research/ITALY_COUNTRY_REVIEW_BATCH38.md`, `research/italy_batch38.tsv` och `research/EUROPE_COUNTRY_INDEX.md`.

Föregående paket **v38** tillför **åtta planerade Wien–Paris-rörelser** från fyra granskade rader ur Air France nr 25: AF till/från CDG och OS till/från Orly. **Totalt 2 218 rörelser**, varav 2 217 planerade och en tidigare bekräftad helikoptertransport. Alla 2 210 äldre rörelseposter är oförändrade. Wien–Schwechat tillkommer; Austrian Airlines får sina första animerade avgångar. Liechtenstein, Rhein-Helicopter och Tyrolean registreras för fortsatt forskning utan nya rörelser från dessa operatörer. Tre nya källposter; inga nya katalogkandidater. Nästa fördjupning är **Italien**. Se `research/LIECHTENSTEIN_COUNTRY_REVIEW_BATCH37.md`, `research/AUSTRIA_COUNTRY_REVIEW_BATCH37.md`, `research/austria_batch37.tsv` och `research/EUROPE_COUNTRY_INDEX.md`.

Föregående paket **v37** tillför **80 planerade Schweiz–Frankrike-rörelser**, från 48 granskade nonstoprader i Air France nr 25. **Totalt 2 210 rörelser**, varav 2 209 planerade och en tidigare bekräftad helikoptertransport. Alla 2 130 äldre rörelseposter är oförändrade. Genève/Zürich får förbindelser med Paris CDG, Nice, Lyon, Bordeaux, Marseille och Toulouse. Schweiz har nu 152 ändpunktsrörelser, med luckor kvar. Ett nytt Swissair-vinteromslag registreras utan avgångar; inga nya flygplatsmarkörer eller kandidater. Nästa fördjupning är **Liechtenstein**, därefter Österrike. Se `research/SWITZERLAND_COUNTRY_REVIEW_BATCH36.md`, `research/switzerland_batch36.tsv` och `research/EUROPE_COUNTRY_INDEX.md`.

Föregående paket **v36** tillför **nio planerade Air France-rörelser mellan Tegel och Düsseldorf**, från sex granskade nonstoprader. **Totalt 2 130 rörelser**, varav 2 129 planerade och en tidigare bekräftad helikoptertransport. Alla 2 121 äldre rörelseposter är oförändrade. Västberlin har nu 138 ändpunktsrörelser. Östtysklands 19 rörelser är oförändrade; 22 tryckta Interflug-rader sparas enbart som forskning tills teckenförklaring och stoppmönster verifierats. Två nya Interflug-källor; inga nya flygplatsmarkörer eller katalogkandidater. Nästa fördjupning är **Schweiz**. Se `research/EAST_GERMANY_COUNTRY_REVIEW_BATCH35.md`, `research/WEST_BERLIN_COUNTRY_REVIEW_BATCH35.md`, `research/west_berlin_batch35.tsv`, `research/interflug_research_rows_batch35.tsv` och `research/EUROPE_COUNTRY_INDEX.md`.

Föregående paket **v35** tillför **89 planerade rörelser** med västtysk ändpunkt, från 56 granskade AF-/LH-rader i Air France nr 25. **Totalt 2 121 rörelser**, varav 2 120 planerade och en tidigare bekräftad helikoptertransport. Alla 2 032 äldre rörelseposter är oförändrade. Köln/Bonn–Wahn tillkommer som markör; München använder fortsatt Riem. Luxemburg, Luxair och Cargolux registreras utan avgångar eftersom rätt tidtabellsinlagor ännu saknas. Fyra nya källposter. Västtyskland har nu 307 ändpunktsrörelser, med stora luckor kvar. Nästa fördjupning är **Östtyskland**. Se landrapporterna i `research/LUXEMBOURG_COUNTRY_REVIEW_BATCH34.md`, `research/WEST_GERMANY_COUNTRY_REVIEW_BATCH34.md`, transkriptionen `research/west_germany_batch34.tsv` och `research/EUROPE_COUNTRY_INDEX.md`.

Föregående paket **v34** tillför **31 planerade rörelser** mellan Bryssel och Paris CDG, Lyon, Nice samt Strasbourg. 24 granskade nonstoprader ur Air France nr 25 ger **totalt 2 032 rörelser**, varav 2 031 planerade och en tidigare bekräftad helikoptertransport. Alla 2 001 äldre rörelseposter är oförändrade. Strasbourg–Entzheim tillkommer som markör och Sabenas vinteromslag som källspår. Belgien har nu 62 ändpunktsrörelser, med stora luckor kvar. Nästa fördjupning är **Luxemburg**. Se `research/BELGIUM_COUNTRY_REVIEW_BATCH33.md`, `research/belgium_batch33.tsv` och `research/EUROPE_COUNTRY_INDEX.md`.

Föregående paket **v33** tillför **35 planerade rörelser** med nederländsk ändpunkt: Amsterdam–Paris CDG/Lyon/Nice samt Rotterdam–Paris CDG. 22 granskade nonstoprader ur Air France nr 25 ger **totalt 2 001 rörelser**, varav 2 000 planerade och en tidigare bekräftad helikoptertransport. Alla 1 966 äldre rörelseposter är oförändrade. Rotterdam tillkommer som markör och KLM:s vinteromslag som källspår. Nederländerna har nu 105 ändpunktsrörelser, med stora luckor kvar. Nästa fördjupning är **Belgien**. Se `research/NETHERLANDS_COUNTRY_REVIEW_BATCH32.md`, `research/netherlands_batch32.tsv` och `research/EUROPE_COUNTRY_INDEX.md`.

Föregående paket **v32** tillför **15 planerade rörelser** med portugisisk ändpunkt: Lissabon–Orly, Lissabon–Lyon och Porto–Orly, från 15 granskade rader ur Air France nr 25. **Totalt 1 966 rörelser**, varav 1 965 planerade och en tidigare bekräftad helikoptertransport. Alla 1 951 äldre rörelseposter är oförändrade. Lissabon och Porto tillkommer som markörer, TAP får sina första animerade poster. TP412* sparas som kandidat utan animation på grund av olöst symbol. TAP:s vinteromslag är en ny källpost. Gibraltar har återgranskats utan nya avgångar. Nästa fördjupning är **Nederländerna**. Se `research/PORTUGAL_COUNTRY_REVIEW_BATCH31.md`, `research/GIBRALTAR_COUNTRY_REVIEW_BATCH31.md`, `research/portugal_batch31.tsv` och `research/EUROPE_COUNTRY_INDEX.md`.

Föregående paket **v31** tillför **50 planerade rörelser** med spansk ändpunkt, från 42 granskade nonstoprader ur Air France nr 25. **Totalt 1 951 rörelser**, varav 1 950 planerade och en tidigare bekräftad helikoptertransport. Alla 1 901 tidigare rörelseposter är oförändrade. Bilbao, Palma och Valencia tillkommer som markörer. Monaco och Heli Air Monaco tillkommer i källregistret utan animerade avgångar. Två nya omslagskällor är registrerade. Monaco, Andorra och Spanien har granskats i riktade urval med luckor kvar. Nästa fördjupning är **Gibraltar**. Se `research/SPAIN_COUNTRY_REVIEW_BATCH30.md`, övriga landrapporter för omgång 30, `research/spain_batch30.tsv` och `research/EUROPE_COUNTRY_INDEX.md`.

Föregående paket **v30** tillför **164 planerade Air Inter-rörelser** mellan Paris och Bordeaux, Lyon, Marseille, Toulouse, Nice samt Basel/Mulhouse, i båda riktningarna. **Totalt 1 901 rörelser**, varav 1 900 planerade och en tidigare bekräftad helikoptertransport. Alla 1 737 tidigare rörelseposter är oförändrade. 150 nya schemarader är godkända; tio ytterligare rader har tidskonflikter och animeras inte. Tre flygplatser och Air Inter tillkommer. Den egna flygtrafikmenyn och flygplanssilhuetten är oförändrade. Frankrike har granskats i ett riktat första urval med luckor kvar. Nästa fördjupning är **Monaco**. Se `research/FRANCE_COUNTRY_REVIEW_BATCH29.md`, `research/france_batch29.tsv` och `research/EUROPE_COUNTRY_INDEX.md`.

Föregående paket **v29** tillför **sex tidtabellsrörelser Dublin–Paris Charles de Gaulle** från fyra granskade EI-/AF-rader. **Totalt 1 737 rörelser**, varav 1 736 planerade och en tidigare bekräftad helikoptertransport. Alla 1 731 tidigare rörelseposter är oförändrade. Jersey, Guernsey och Isle of Man har genomgått riktade källurval utan nya importklara avgångar; samtliga har kvarvarande luckor. Nästa fördjupning är **Frankrike**. Se `research/IRELAND_COUNTRY_REVIEW_BATCH28.md`, övriga landrapporter för omgång 28 och `research/EUROPE_COUNTRY_INDEX.md`.

Föregående paket **v28** tillför **82 tidtabellsrörelser** med brittisk ändpunkt: Heathrow/Manchester mot Paris, Lyon och Nice, samt en Air UK-avgång Amsterdam–Southampton. **Totalt 1 731 rörelser**, varav 1 730 planerade och en tidigare bekräftad helikopterrörelse. 51 nya schemarader har granskats, inklusive decemberändringar och AFBA919:s start 1 mars. De 1 649 tidigare rörelseposterna är oförändrade. Danmark, Finland och Island har genomgått riktade källurval med kvarvarande luckor; två danska forskningskällor har tillkommit utan nya rörelser. Nästa fördjupning är **Jersey**, följt av Guernsey, Isle of Man och Irland. Se `research/GREAT_BRITAIN_COUNTRY_REVIEW_BATCH27.md`, övriga landrapporter för omgång 27 och `research/EUROPE_COUNTRY_INDEX.md`.

Föregående paket **v27** dokumenterar Norges landgenomgång. **47 Norge-rörelser och samtliga 1 649 rörelser totalt är oförändrade**; inga ytterligare avgångar hade tillräckligt underlag för import. Sju Braathens-anslutningsrader sparas för kontroll av stoppmönster, utan animation. **Danmark är nästa land.** Se `research/NORWAY_COUNTRY_REVIEW_BATCH26.md`, `research/norway_country_review_batch26.json` och `research/EUROPE_COUNTRY_INDEX.md`. Flygdata, egen meny och flygplanssymbol är oförändrade från v26.

Paket **v26** inför en systematisk Europakö land för land. Sverige har granskats som första land, med **87 befintliga ändpunktsrörelser och inga nya importklara avgångar** i de kompletterande källorna. Totalt är **1 649 rörelser oförändrade**. **Norge är nästa fördjupning**, därefter Danmark, Finland och Island. Se `research/EUROPE_COUNTRY_INDEX.md`, `research/SWEDEN_COUNTRY_REVIEW_BATCH25.md` och `research/europe_country_queue.json`. Landstalen räknar start/destination oavsett bolagets hemland; inget land har fullständig täckning.

Föregående paket **v25** tillför **90 planerade rörelser** från 62 granskade nonstoprader: sex Köpenhamn–Nice/Lyon-rörelser från AF/SK-samarbeten och 84 Air UK-rörelser kring Stansted, Southampton och Kanalöarna. Totalt finns 1 649 rörelser, varav 155 med nordisk ändpunkt. Alla 1 559 tidigare rörelser är oförändrade. Flygplansikonen och den egna flygtrafikmenyn följer med. Se `research/NORDIC_AIRUK_BATCH24.md`, `research/nordic_airuk_batch24.tsv` och `research/validation_batch24.json`. Samarbetsflygen räknas en gång, grupperade efter första tryckta flygkod; faktisk operatör är inte fastställd. De tidigare 33 JAL-rörelserna från förhandsutgåvan behåller sin märkning; senare slutlig utgåva är inte verifierad.

| Bolag | Transkriberade delar | Avgångar som överlappar perioden |
|---|---|---:|
| Golden Air | Direktsträckor Karlskoga–Bromma och gamla Karlstad–Fornebu | 12 |
| Pan Am | Tidigare nät samt Aten–Frankfurt i båda riktningarna | 328 |
| COMAIR | Tidigare nät samt Cincinnati–Columbus/Indianapolis, Indianapolis–Milwaukee/Columbus med retur; partiellt | 234 |
| Delta Air Lines | Tidigare nät samt Cincinnati–Columbus och Cincinnati–Indianapolis med retur; partiellt | 922 |
| Norcanair | Urval ur PDF-sidor 3–4 | 17 |
| Líneas Aéreas Paraguayas | Europeiska förbindelser via Recife, fysiska delsträckor | 10 |
| Ladeco | Urval av norra Chile | 18 |
| Air Nauru | Två tryckta direktsträckor; odaterade handändringar uteslutna | 2 |
| Air Wisconsin | Utökat urval av nonstoprader på s. 2–13 | 442 |
| Air Zimbabwe | Regionalt/inrikes urval fredag/lördag samt nattflyg till/från Gatwick | 43 |
| South African Airways | SA-kodade förbindelser i Air Zimbabwes tidtabell | 8 |
| British Airways | Tidigare Europa/Afrika samt nya Dublin–Birmingham och Heathrow–Cork/Shannon | 116 |
| Air Botswana | BP256/BP267 via Francistown i Air Zimbabwes tidtabell | 4 |
| US Marine Corps | En tidsatt helikoptertransport i officiell dagbok | 1 |
| Northwest | Vinterflyg via Gatwick, Arlanda, Gardermoen och Prestwick samt USA–Japan/Sydkorea och tidsatta Asiensegment | 116 |
| GB Airways | Gatwick–Gibraltar samt Gibraltar–Tanger/retur | 12 |
| Braathens SAFE | Norska nonstoprader och tidsatta delsträckor via Stavanger/Haugesund, s. 10, 11 och 20 | 28 |
| British Midland | Teesside–Heathrow/retur | 16 |
| Casair | Fem delsträckor; tre ytterligare rader undantagna efter tidskonflikt | 5 |
| Jersey European Airways | Teesside–Blackpool/retur | 2 |
| Dan-Air | Tidigare Europa/Norden samt Innsbruck–Gatwick | 103 |
| Air UK | Nordiska och brittiska delsträckor, Amsterdam, Bryssel, Paris CDG och Kanalöarna; kompletterad lördagsavgång till Southampton | 177 |
| Swissair | Tidigare Europa/Västafrika/Mellanöstern-urval samt Genève/Zürich–Frankrike | 68 |
| Aer Lingus | Tidigare rutter samt nya Dublin-, Cork- och Shannonförbindelser | 54 |
| Lufthansa | Düsseldorf/Frankfurt–Schweiz, Frankfurt–Moskva samt nya västtyska Frankrikeförbindelser | 60 |
| KLM | Genève–Amsterdam, Amsterdam–Newcastle samt nya Paris-/Nice-/Rotterdamförbindelser | 27 |
| Crossair | Genève–Basel/Mulhouse | 3 |
| Iberia | Genève–Barcelona, Malaga–Dublin och utvalda Spanien–Frankrikeförbindelser | 26 |
| Olympic Airways | Genève–Aten samt Aten–Orly/Marseille i båda riktningar | 6 |
| Air Algérie | Genève–Alger samt Bryssel–Alger i båda riktningar | 4 |
| Japan Air Lines | 33 rörelser ur förhandsutgåvan samt Frankfurt–Moskva ur Aeroflots vintertabell | 34 |
| Sabena | Tidigare Brysselförbindelser samt Bryssel–Barcelona–Alger och Alger–Bryssel | 19 |
| Aeroflot | Utvalda europeiska nonstoprader, inklusive Moskva–Köpenhamn, Budapest–Alger, Moskva–CDG och CDG–Leningrad | 29 |
| Interflug | Moskva–Schönefeld och Dresden–Moskva | 12 |
| Malév | Budapest–Moskva/retur, Budapest–Warszawa och Budapest–Orly | 7 |
| SAS | Aberdeen–Stavanger/retur, Arlanda–Kastrup/retur, Paris, SKAF-rader till Nice/Lyon samt Bergen–Gatwick | 78 |
| Air Business A/S | Esbjerg–Århus fredag, 8A163 | 1 |
| Air France | Paris–Norden/Dublin, samarbetsrader, brittiska förbindelser, övriga Europarader samt Italien–Frankrike och Orly–Warszawa/Prag/Budapest/Otopeni/Sofia/Belgrad/Zagreb samt Aten–Frankrike, CDG–Larnaca/Istanbul/Moskva/Borispol och Moskva–Narita | 307 |
| THY Turkish Airlines | Istanbul–Atatürk/Orly i båda riktningar | 4 |
| JAT Yugoslav Airlines | Belgrad/Zagreb–Orly, båda riktningarna | 4 |
| Tarom | Otopeni–Orly, båda riktningarna | 2 |
| Balkan Bulgarian Airlines | Sofia–Orly samt frakt i båda riktningarna, första kod LZ | 4 |
| CSA Czechoslovak Airlines | Prag–Orly, första kod OK i OKAF766 | 1 |
| LOT Polish Airlines | Warszawa–Orly, första kod LO i LOAF273 | 1 |
| Alitalia | Italien–Frankrike och Genua–Neapel | 48 |
| Austrian Airlines | Wien–Paris Orly, båda riktningarna | 4 |
| Air Inter | Tidigare Parislinjer samt nya regionala direktflyg och Paris–Strasbourg | 238 |
| TAP Air Portugal | Lissabon–Orly/retur, Lissabon–Lyon/retur samt Orly–Porto | 7 |
| Southwest Airlines | Röda N/S-rader: stadsavsnitt genomgångna; fortsatt partiell bolagstäckning | 1219 |
| **Totalt** | **3 057 godkända tidtabellsrader samt en separat rörelsehandling** | **4 883** |

4 882 har status `scheduled`; en har status `confirmed`. Underlaget omfattar 1008 olika riktade platspar och 241 registrerade flygplatser/platser. Helikopterresans två ändpunkter är ungefärliga platsmarkörer. Tjugosju tidtabellsutgåvor bidrar med godkända schemarader. Därutöver finns bland annat 22 forskningsrader från Interflugs förhandsutgåva och sex från Cyprus Airways förhandsutgåva utan animation. Air Zimbabwes utgåva bidrar även med SA-, BA- och BP-kodade förbindelser. Bolagstillhörigheten följer källans flygkod och belägger inte faktisk operatör vid eventuell inhyrning av flygplan. AFBA919 räknas en gång under första koden AF; faktisk operatör är inte fastställd.

Omgång 2 tillförde **174 avgångar och 53 riktade flygplatspar**. Air Wisconsin 2740 Muskegon–Battle Creek är sparat som `candidate`: s. 2 anger ankomst 16.06 och s. 10 anger 16.05. Posten animeras inte. Anslutningar och motstridiga rader räknas inte som ytterligare direktflyg. Detaljer finns i `research/batch_02.json`.

Källregistret har **84 poster**, inklusive en separat sommarutgåva endast för teckenförklaring. `research/COUNTRY_INDEX.md` är en genererad landvis innehållsförteckning över de 93 registrerade operatörerna: inlästa rader och återstående arbete. Den visas också i flygmenyn. Inget land är färdiginventerat; bolag utanför registret kan saknas. Forskningskön har **92 flyg-/helikopterbolag och en militär operatör, 81 länder/territorier**, däribland separata dåtida Västtyskland, Östtyskland och Sovjetunionen. Västberlin redovisas som eget territorium. SAS hör till tre hemländer. Kön är inte en inventering av alla bolag som fanns 1986; `operator_census_complete` och `operator_census_verified` är tills vidare `false`.

Internet Archive-loggen omfattar nu **70 separata bolagssökningar**; den nordiska prioriteringsomgången kompletterade med Norving, Maersk Air/Danair och Cimber Air/Danair utan nya träffar. Loggen innehåller fortfarande **191 unika råträffar**, till stor del irrelevanta rapporter/tidningar eller bara omslag. Ett separat tidtabellsindex gav **147 unika kandidat-URL:er** från avsnitten 1985–1986. Detta är **inte** 147 verifierade, tillämpliga tidtabeller; många gäller senare perioder. Ingen råträff förs automatiskt över till kartan.

**Aktuell prioritet är användarens worldwide-uppdrag: källor med många saknade flygrörelser.** Europakön är bevarad med Cypern som nästa land. Följ `research/europe_country_queue.json`; återbesök tidigare länder när nya källor hittas. Överflygningar över Norden kvarstår som en separat evidensuppgift. Kön i `research/nordic_priority.json` omfattar nordiska bolag och utländska bolag att undersöka. Se `research/NORDIC_OVERFLIGHT_RESEARCH.md` för luckor, källspår och kraven på färdvägsbelägg. Delta prioriteras nu enligt användarens senaste worldwide-inriktning; nordiska ändpunkter och överflygningar följs fortsatt separat. SAA-indexets aprilutgåva 1986 får inte användas som bevis för februari. De nya SA-kodade sträckorna kommer i stället från Air Zimbabwes novemberutgåva 1985.

Omgång v13 tillför 22 Braathens-flyg och sju norska flygplatser. Fyra sidor ur SAS/Linjeflygs Malmöutgåva har också granskats men saknar tillräckliga delsträcksuppgifter för import. Se `research/NORDIC_TIMETABLE_BATCH12.md`. I v13 berörde 37 unika rörelser nordiska ändpunkter; inga faktiska överflygningar har ännu verifierats.

Omgång v14 tillför 29 planerade delsträckor kring Teesside, sammanlagt 975 rörelser. De nordiska anslutningsförslagen är ännu inte tillräckligt uppdelade för animation. En samtida notis om SE-FNZ:s avgång Teesside–Halmstad den 28 februari sparas i `research/nordic_movement_notes.json`, utan animation eftersom ankomsttid och tidszon saknas. LOT:s vintersidor och Finnairs blandade sommar-/vinterbilder är granskade och köförda. Se `research/NORDIC_TIMETABLE_BATCH13.md`.

Omgång v15 tillför sex norska delsträckor och UK620 Humberside–Esbjerg. Tre tidigare Casair-poster har motstridiga vintertider och är nu kandidater utan animation. Totalt finns **979 rörelser, varav 44 med nordisk ändpunkt**. Se `research/NORDIC_TIMETABLE_BATCH14.md` och `research/schedule_conflicts_batch14.json`. Inga faktiska överflygningar tillagda.

Omgång **v16** tillför **101 planerade rörelser** från Swissairs Beneluxutgåva, Dublins vinterguide och British Airways. Totalt **1 080 rörelser**; de 979 tidigare är oförändrade. Två nya Aer Lingus-rader hålls som kandidater med motstridiga tider. Norden har fortsatt 44 berörda rörelser och inga belagda faktiska överflygningar. Se `research/EUROPE_TIMETABLE_BATCH15.md`, `research/europe_batch15.tsv` och `research/batch_15.json`.

Omgång **v17** tillför **55 planerade rörelser** från Northwests Stillahavstabell. Totalt **1 135 rörelser**. 1 079 tidigare rörelser är exakt oförändrade; NW48 Gatwick–Glasgow har rättad destination till Prestwick (PIK) med oförändrade tider. Två SAS-arkivhänvisningar är sparade för fortsatt nordisk research; Norden har fortsatt 44 rörelser. Se `research/NORTHWEST_PACIFIC_BATCH16.md`, `research/northwest_batch16.tsv`, `research/batch_16.json` och `research/validation_batch16.json`.

Omgång **v18**, tillför **47 planerade rörelser** på **25 nya riktade platspar** i Asien och över Stilla havet. Totalt **1 182 rörelser**; alla 1 135 tidigare är oförändrade. Åtta historiskt kontrollerade flygplatser tillkommer. Pilmarkören är ersatt med en fylld flygplanssilhuett med bibehållna statusfärger. **C++-ändringen kräver ombyggnad av Development Editor.** Se `research/NORTHWEST_ASIA_BATCH17.md`, `research/northwest_batch17.tsv`, `research/batch_17.json` och `research/flight_icon_preview_v18.png`.

## Data och kod

| Fil/plats | Funktion |
|---|---|
| `Plugins/TMOPEngine/Content/WorldAtlas/Flights/catalog.json` | Redigerbart register: länder, bolag, historiska flygplatser/platser, källor, tidtabellsrader, observationer, enstaka rörelser och forskningsstatus |
| `Plugins/TMOPEngine/Content/WorldAtlas/Flights/flights.json` | Genererat, kontrollerat spelunderlag med individuella avgångar i UTC; redigera normalt katalogen och bygg om |
| `Tools/FlightResearch/build_flights.py` | Omvandlar lokala tidtabeller till det exakta 48-timmarsfönstret |
| `Tools/FlightResearch/discover_sources.py` | Återupptagbara sökningar på Internet Archive, land för land/bolag för bolag |
| `Tools/FlightResearch/research/archive_airline_searches.json` | Sökfrågor, sökdatum, resultat, fel och granskningsstatus |
| `Tools/FlightResearch/research/timetable_candidate_urls.json` | Kandidater från indexet över fullständigt skannade tidtabeller |
| `Tools/FlightResearch/build_country_index.py` och `research/COUNTRY_INDEX.md` | Genererar en landvis innehållsförteckning från aktuell katalog och avgångar |
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

Inget land markeras automatiskt klart. Privata, militära, frakt- och charterflyg kräver ofta andra arkiv än reguljära tidtabeller. Fristående, oschemalagda rörelser importeras genom `movements` enligt evidenskraven nedan.

## Kommandon från projektroten

Python 3.9 eller senare med IANA-tidszonsdatabas behövs för forskningsverktygen. På Windows kan tidszonsdatabasen installeras med `python -m pip install tzdata`. Spelet behöver varken Python eller tzdata: omräknade UTC-tider ligger redan i `flights.json`.

```sh
python Tools/FlightResearch/discover_sources.py --priority nordic --list
python Tools/FlightResearch/discover_sources.py --priority nordic
python Tools/FlightResearch/discover_sources.py --country se
python Tools/FlightResearch/discover_sources.py
python Tools/FlightResearch/build_flights.py
python Tools/FlightResearch/build_flights.py --check
python Tools/FlightResearch/build_country_index.py --check
python Tools/FlightResearch/build_europe_index.py --check
python -m unittest discover -s Tools/FlightResearch -v
```

`--priority nordic` följer den uttryckliga operatörskön och inkluderar utländska bolag som ska undersökas. Det bekräftar inte nordisk trafik eller överflygningar. `--list` visar urval/cache utan nätverk eller filändringar. `--country` tillsammans med prioritet begränsar fortfarande efter bolagets hemland, inte flygväg.

Sökverktyget hoppar över lyckade tidigare bolagssökningar. `--refresh` söker om urvalet. Misslyckade anrop loggas och kan köras igen; inga inloggningsuppgifter behövs. Verktyget arbetar med högst tre samtidiga sökningar, sparar efter varje bolag och anger om en sökning hade fler än 200 resultat. Det importerar inga flygrutter.

Tidszonerna är historiska IANA-regler, inte dagens UTC-offset. Recife och Santiago hade exempelvis sommartid i denna period, och Air Naurus Pago Pago-flyg passerar datumgränsen. Tvetydiga klockslag kräver explicit `departure_fold`/`arrival_fold`; obefintliga lokala tider stoppas. Publicerad runtime-JSON bör checkas in så att ändringar i en framtida tzdata-version blir synliga i Git.

## Språk och uppdateringar

Landnamn, metodtext, källanmärkningar och flyganmärkningar har `sv`, `en`, `en_source`. Ändras svenskan måste engelskan granskas och `en_source` uppdateras till den nya svenska texten. Annars faller spelet tillbaka på svenska. Externa språkpaket kan använda tabellen `TMOP_Flights`, stabilt land-/käll-/tidtabells-ID och fältet `name` eller `notes`; metodtexten använder rad `method`, fält `text`. Menytexterna använder det befintliga lokaliseringssystemet och 30 nya nycklar i `TMOPMenuTranslations.inl`.

## Verifiering och återstående arbete

Körda Python-tester täcker exakta periodgränser, Golden Airs vardagar, Pan Ams dagkoder, överlappande flyg, gamla flygplatser, datumgränsen, giltiga källor, dubbletter, datumundantag, evidenskrav för inställda/genomförda flyg, sommartidsgap/fold, halvtimmeszoner, LAP:s mellanlandningar och inaktuella översättningar. Nya tester täcker dessutom månad utan dag, Air Wisconsins källkonflikt och tidszonsskifte samt Air Zimbabwe/BA:s nattflyg. De 30 menyöversättningarnas svenska källtexter och formatplatshållare är kontrollerade.

Unreal-testerna täcker luftburen/landad-gränsen, hastighet, spelarisolerad klocka, sluttid, dataimport, land/bolagsfilter och språkfallback. De har inte körts här. Grafiska tester, klickytor, split-screen, prestanda och paketerad build återstår i UE. Listan är virtualiserad och likadana rutter ritas en gång, men prestanda med världsomfattande tiotusentals rörelser är ännu inte verifierad.

Transkriptionerna är källbaserade, inte dubbelt oberoende granskade. Nästa datasteg är att läsa klart de elva påbörjade utgåvorna, granska de funna vinterutgåvorna och fortsätta komplettera bolagsinventeringen land för land. Inga fullständiga skanningar återdistribueras i kodpaketet; källorna länkas.

## Omgång 3 – korrigering i omgång 4

Ny kontroll av kolumnrubrikerna på PDF-sida 2 visar att RH166 Johannesburg–Harare 18.30 och SA026 Johannesburg–Harare 12.50 tillhör **söndag**, inte lördag. Dessa två avgångar från v04 är borttagna. RH161 Harare–Johannesburg går fredag/söndag; den felaktiga lördagsavgången från den tidigare importen är också borttagen. RH844-rättelsen från omgång 3 kvarstår.

## Omgång 4 / paket v05

- 223 nya tidtabellsrader från Air Wisconsin ger 323 ytterligare fysiska flygsträckor i fönstret.
- Air Botswana BP256 fredag och BP267 lördag ger fyra nya delsträckor Harare–Francistown–Gaborone och tillbaka.
- Tre tidigare avgångar har korrigerats bort på grund av veckodagskolumnerna ovan. Netto +324, totalt 574.
- 17 nya flygplatser, ett nytt registrerat land (Botswana) och ett nytt bolag.
- Läs `research/batch_04.json` för ID:n, rättelser och kontrollsummor till källbilderna. `research/air_wisconsin_batch04.tsv` innehåller transkriptionen med tryckta veckodagskoder.
- Inga originalskanningar redistribueras i paketet. Koordinater är schematiska flygplatslägen, inte specifika 1986-terminaler eller uppställningsplatser.

## Omgång 5 / paket v06 – fler reguljära och militära flyg

- 50 nya Pan Am-rader ger 90 ytterligare tidtabellslagda flygsträckor. Transkription finns i `research/panam_batch05.tsv` och granskning i `research/batch_05.json`.
- En dokumenterad US Marine Corps-transport läggs till med egen rörelsehandling. Välj **US Marine Corps** i operatörsfiltret och tryck **Visa vid avgång**.
- Nio nya platser, däribland Tegel, gamla München-Riem och två ungefärliga helikopterändpunkter. Flyglinjerna visar inte verkliga korridorer genom dåtidens luftrum.
- `research/military_research.json` och `research/MILITARY_RESEARCH.md` innehåller övriga militära transportuppgifter och kvarstående belägg. Dessa forskningsposter är inte egna kort i spelmenyn och animeras inte.

### Enstaka dokumenterade flygningar

`catalog.json` har den valfria listan `movements`. Varje importerad post måste ha ett stabilt ID, registrerad operatör i `airline`, två platser, granskad källa av typen `movement_record`, sidangivelse, dokumenterade start-/sluttider med UTC-offset, `status: confirmed`, `nonstop: true`, `review_status: reviewed`, `time_precision: minute` eller `second`, `movement_type` och språktexter. `flight` kan vara en uttryckligt förklarad etikett när källan saknar flygnummer. `military`, `charter`, `private`, `state` och `commercial` är tillåtna typer.

Datumuppgifter, flerdagarsoperationer och uppskattade flygtider hör till forskningskön. Skriv inte in påhittade klockslag för att få med dem i animationen. Militär personal eller militär last räcker inte för klassificeringen `military`.

Gamla `observations` används fortsatt för att bekräfta eller ställa in en redan registrerad tidtabellsavgång. `movements` används när ingen sådan tidtabell finns; registrera inte samma flyg i båda. Den genererade runtime-postens äldre fältnamn `schedule` är en lokaliseringsnyckel (`movement:<id>`), inte en påstådd tidtabell. Befintlig Unreal-läsare kan därför läsa de nya rörelserna utan schemaändring.

20 Python-tester går igenom, inklusive krav på rörelsehandling, datumprecision, dubbletter, gränsöverlapp, EST/UTC och Pan Ams nattflyg från Berlin. Unreal-kompilering och visuell kontroll återstår i Editor.

## Omgång 6 / paket v07 – Norden och England

- 26 nya tidtabellsrader från Northwest och GB Airways/BA ger 28 nya planerade flygsträckor. Totalt 693 rörelser: 692 tidtabellslagda och en tidigare dokumenterad militär transport.
- Sverige får Gatwick–Arlanda fredag och Arlanda–Gardermoen lördag. Norge får också Gardermoen–JFK lördag. Gardermoen behåller sin historiska kod GEN och blandas inte ihop med Fornebu.
- England får fler Gatwick-flyg. Den kompletta GB/BA-tabellen tillför även Gibraltar–Tanger och retur. Alla nya käll- och flyganmärkningar har svenska och engelska språkfält.
- Northwests Köpenhamnstrafik har granskats men ligger utanför fönstret. Danmark har fortfarande inga importerade avgångar. Maersk Air och Cimber Air är tillagda i forskningskön.
- Northwests vinterundantag och genomgående nummer 840–845 är hanterade. Samma fysiska Europasträcka räknas en gång; NW49 visas inte på fredagen.
- `research/COUNTRY_INDEX.md` visar nu både operatörernas hemländer och trafiken till/från varje land inklusive utländska operatörer. Noll importerade rörelser betyder inte att trafik saknades.
- `research/NORDIC_UK_RESEARCH.md`, `research/nordic_uk_batch06.tsv` och `research/batch_06.json` redovisar fynd, transkriptioner, undantag, källor och fortsatt kö.
- Anchor Express i Norge har lagts till som militärt forskningsunderlag. Inga nya militära avgångar med exakta tider har fastställts. Forskningsposter animeras inte.

Datakontrollerna kan köras utan Unreal. Kompilering och visuell kontroll av paketet måste fortfarande göras i projektets Unreal Editor.

## Omgång 7 / paket v08 – Israel och Sydafrika

- Tre nya Pan Am-rader ger sex ytterligare planerade flygsträckor; totalt 699 rörelser. PA114 går JFK–Paris–Tel Aviv och PA115 tillbaka. Varje fysisk sträcka har egna tider; genomgående nummer innebär inte samma flygplan.
- SAA:s och El Als rätta vinteromslag finns nu som källposter. Avgångssidorna återstår och omslagen används inte som belägg för flygtider.
- Johannesburg-raderna från Air Zimbabwe är återkontrollerade; inga nya sydafrikanska avgångar har fastställts i denna omgång.
- Irantransporten den 27 februari har fått en kompletterande, tydligt märkt sekundärkälla med ortparet Tel Aviv–Teheran. Den ligger kvar som forskningsunderlag utan animation.
- Läs `research/ISRAEL_SOUTH_AFRICA_RESEARCH.md` och `research/batch_07.json` för källor, exakta import-ID:n och kvarstående arbete. Nya språkfält finns på svenska och engelska.

## Omgång 8 / paket v09 – Atlanten och Afrika

- 37 nya granskade Pan Am-rader ger 60 ytterligare planerade flygsträckor i 48-timmarsfönstret. Totalt 759 rörelser: 758 tidtabellslagda och en tidigare dokumenterad militär transport.
- Fler Europaflyg från Heathrow, fler Atlantflyg och USA-inrikesflyg. PA188 New York–Dakar–Monrovia–Lagos–Nairobi är uppdelad i fyra fysiska sträckor med dokumenterade stopptider. Av PA189:s retur ligger två sträckor inom fönstret.
- Månadsskiftets trafikändringar är hanterade: PA121 London–Los Angeles får en lördagsavgång från 1 mars. PA120 i motsatt riktning får också en lördagsavgång, men den avgår efter animationens slut.
- Sju nya flygplatser, 28 nya riktade platspar och tre registrerade länder: Senegal, Liberia och Nigeria. Dakar använder Yoff och Monrovia Roberts. Landtabellen visar importerad trafik; de nya ländernas inhemska operatörer återstår att inventera.
- Fem tidigare tidtabellsrader kopplas samman med nytillagda delar av genomgående tjänster. Flygtider och antal tidigare rörelser ändras inte. Samma nummer belägger inte samma flygplan.
- Läs `research/ATLANTIC_AFRICA_RESEARCH.md`, `research/panam_batch08.tsv` och `research/batch_08.json` för sidkällor, tidsgränser och fortsatt kö.

## Omgång 9 / paket v10 – Europa och USA:s västkust

- 43 nya godkända Pan Am-rader ger 73 ytterligare planerade flygrörelser. Totalt 832 rörelser, varav 831 tidtabellslagda och en tidigare dokumenterad militär transport.
- 42 nya ankomster till Berlin Tegel från Hamburg, München Riem, Nürnberg, Stuttgart och Zürich. Därutöver tillkommer bland annat Rom, Nice, Genève och USA:s västkust.
- Fredagens London–Seattle–San Francisco och fler genomgående tjänster har kompletterats med dokumenterade mellanlandningstider. Nummerbytena PA264/84 och PA265/85 den 1 mars skapar inte dubbla avgångar.
- Fyra flygplatser är registrerade; tre får nya animerade rörelser. Två Istanbul-rader finns som kandidater och hålls utanför animationen eftersom tidtabellens GMT+3 strider mot IANA:s historiska UTC+2. Turkiet tillkommer i landregistret, med fortsatt ofullständig operatörsinventering.
- De 759 tidigare rörelsernas tider och status bevaras. Nio äldre rader får utökad gruppering.
- Läs `research/EUROPE_WEST_COAST_RESEARCH.md`, `research/panam_batch09.tsv` och `research/batch_09.json` för underlag, undantag och kvarstående kontroll.

## Omgång 10 / paket v11 – Delta och Europa

- 39 nya Delta-rader ger 80 planerade rörelser. Delta tillkommer i animationen med Atlanta–Frankfurt/Gatwick/Orly i båda riktningarna samt utvalda inrikesavgångar från Atlanta.
- Sex Pan Am-rader ger tolv ytterligare rörelser via Bryssel, Amsterdam och Hamburg. Elva befintliga rader binds ihop till längre resor utan ändrade tider eller ID:n.
- Totalt 924 rörelser, 622 godkända tidtabellsrader och 15 operatörer med importerad trafik. De nya platsmarkörerna är Atlanta/Hartsfield och Paris/Orly.
- Datumkoderna, DL455:s helgtrafik och DL181:s start först 2 mars är kontrollerade. De tidigare Istanbul-kandidaterna animeras fortfarande inte.
- Se `research/DELTA_EUROPE_RESEARCH.md`, `research/batch_10.json` och de två transkriptionsfilerna för källor, urval och fortsatt kö. Alla 832 tidigare rörelser behåller sina ID:n, flygplatser, flygnummer, tider, status och anmärkningar.

## Global inventering i v57

Se `research/WORLDWIDE_COVERAGE_BATCH56.md` för de 93 registrerade operatörerna, de 46 utan rörelser och förklaringen till varför ingen global täckningsprocent ännu kan anges.
