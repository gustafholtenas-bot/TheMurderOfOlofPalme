# Europa och USA:s västkust – omgång 9 / paket v10

Granskat 26 september 2026. **43 godkända nya tidtabellsrader ger 73 planerade flygrörelser**, fördelade på 22 nya riktade platspar. Två ytterligare transkriberade Istanbul-rader har en olöst tidszonskonflikt och animeras inte. Totalt finns 832 rörelser: 831 tidtabellslagda och en tidigare dokumenterad militär transport.

Primärkälla är [Pan Ams tidtabell 11 februari–26 april 1986, University of Miami](https://digitalcollections.library.miami.edu/digital/collection/asm0341/id/37370/). Skanningarna har granskats visuellt, med OCR som sökhjälp. `panam_batch09.tsv` innehåller de 45 transkriberade raderna. `batch_09.json` anger import-ID:n, vilka tidigare grupperingar som utökats och källbildernas URL:er och SHA-256. Sidnumren nedan är tryckta sidnummer.

## Nya rörelser i animationen

| Sträckor | Nya rörelser | Tryckta sidor |
|---|---:|---|
| Hamburg–Berlin Tegel | 15 | 45 |
| München Riem–Berlin Tegel | 11 | 67 |
| Nürnberg–Berlin Tegel | 6 | 74 |
| Stuttgart–Berlin Tegel | 8 | 97 |
| Zürich–Berlin Tegel | 2 | 109 |
| Nürnberg–Bryssel | 2 | 74 |
| Stuttgart–Zürich och retur | 4 | 98, 110 |
| Genève–Zürich och retur | 4 | 43, 109 |
| Zürich–JFK | 2 | 110 |
| Rom Fiumicino–JFK och retur | 4 | 72, 87 |
| Nice–JFK och retur | 4 | 72–73 |
| San Francisco–JFK och retur | 5 | 72, 94 |
| San Francisco–Seattle och retur | 2 | 94, 97 |
| Seattle–Heathrow | 2 | 97 |
| San Francisco–Heathrow | 1 | 94 |
| San Francisco–Los Angeles | 1 | 94 |
| **Summa** | **73** | |

Berlin avser Tegel i Västberlin och München gamla Riem. Globlinjerna visar schematiska sträckor mellan flygplatserna, inte de verkliga flygkorridorerna genom dåtidens luftrum.

## Genomgående tjänster och datum

**PA125 London–Seattle–San Francisco:** fredagen 28 februari avgår den redan importerade London-sträckan 10.25 GMT och ankommer Seattle 12.00 PST. Den nya sträckan avgår Seattle 13.40 PST och ankommer San Francisco 15.35 PST. Markuppehållet blir 100 minuter. Lördagens redan importerade London–San Francisco är en annan, nonstop sträcka; den dupliceras inte.

**PA122 San Francisco–Seattle–London–Frankfurt:** torsdagens avgång från San Francisco 15.45 PST fortsätter från Seattle 18.45 PST, ankommer London fredag 11.45 GMT och fortsätter därifrån 13.35 GMT till Frankfurt 16.10 CET. Fredagens Seattle–London finns också. Ingen San Francisco–Seattle-avgång uppfinns för fredagen.

**PA124 San Francisco–London–Frankfurt:** fredagens 17.45 PST följs av lördagens Heathrow-avgång 13.35 GMT. Lördagens avgång från San Francisco ligger efter 48-timmarsfönstrets slut i UTC.

**PA150 San Francisco–Los Angeles–Frankfurt:** fredagens nytillagda 12.45–14.04 PST ansluter till den tidigare importerade Los Angeles-avgången 15.20 PST.

**PA90/91:** de granskade Genève–Zürich-delarna kopplas till befintliga Atlant- och USA-sträckor. Genève–Zürich och Zürich–Genève har anmärkningen *No Local Traffic*; restriktionen gäller lokal passagerartrafik. Istanbul-delarna ingår inte i de godkända grupperna.

**Nummerbyten den 1 mars:** SFO–JFK 08.30–16.40 byter PA264 till PA84; JFK–SFO 16.15–19.20 byter PA265 till PA85. Giltighetsintervallen skiljs vid datumgränsen. Samma fysiska avgång räknas bara en gång, även när tidtabellen använder andra genomgående nummer i destinationslistorna.

Nya granskade rader har avgränsats till lokala avgångsdatum 26 februari–1 mars. Senare ändringar i mars/april används inte. Flyg räknas med om de är i luften någon gång mellan 27 februari 22.21.30 och 1 mars 22.21.30 UTC. Därför ingår även PA265 från JFK den 27 februari, trots att avgången sker före periodens start.

PA692 München–Berlin har granskats i förstoring: den tryckta ankomsten är **22.30 CET**, inte 23.30. Den 27 februari har flyget därmed redan ankommit när animationen börjar och ska inte ingå.

## Istanbul: läsbara tider men olöst tidszonskonflikt

| Rad | Tryckta lokala tider | Sida | Status |
|---|---|---|---|
| PA90 Genève–Istanbul | 12.40–16.35, dagligen | 42 | Kandidat, ej animerad |
| PA91 Istanbul–Genève | 07.00–09.05, dagligen | 52 | Kandidat, ej animerad |

Rubriken på s. 51 anger Istanbul **GMT+3**. [IANA:s Europe/Istanbul-regler](https://data.iana.org/time-zones/tzdb/europe) och arbetsmiljöns `zoneinfo` ger **UTC+2** den 28 februari 1986. IANA-reglerna anger övergång till sommartid sista söndagen i mars. Detta är en konflikt mellan tidtabellens uppgift och databasens historiska tolkning; ingen av dem har tyst skrivits över för att producera en animation.

Radernas lokala tider är sparade med `review_status: candidate` och `review_blocker: timezone_conflict`. Kontroll mot exempelvis en samtida OAG-, THY- eller flygplatsutgåva återstår. Källbild och IANA-underlag har separata kontrollsummor i omgångens metadata.

## Flygplatser och fortsatt arbete

Fyra flygplatser är registrerade: Nice, Rom Fiumicino, Genève och dåvarande Istanbul Atatürk. De första tre får nya animerade rörelser. Atatürk finns som plats för kandidatraderna. Turkiet tillkommer i landregistret; ingen inhemsk operatörsinventering är färdig och ingen Istanbul-avgång ingår i animationen.

`airport_locations_batch09.json` har koordinatkällor och kompletterande historik från flygplats-/myndighetssidor. Atatürk använder **LTBA**, inte det nya Istanbulflygfältet LTFM. Dagens IATA-kod ISL i koordinatunderlaget har därför inte förväxlats med flygplatsens historiska IST-kod. Nice-terminal 2 invigdes först 1987; markören avser flygfältsområdet och inte en terminal eller uppställningsplats.

Nästa arbetssteg: utred Istanbul-konflikten, komplettera ännu saknade delsträckor mellan Bryssel och London och läs fler rader ur de påbörjade utgåvorna. SAA, El Al, SAS och andra bolag i den tidigare kön kvarstår. Omgången tillför inga nya dokumenterade militära rörelser.

Alla 73 nya rörelser har status `scheduled`. Tidtabellerna belägger inte faktiskt genomförande, passagerare, last eller någon anknytning till mordet. Inga originalskanningar återdistribueras.
