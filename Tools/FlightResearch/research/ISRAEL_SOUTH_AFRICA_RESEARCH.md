# Israel och Sydafrika – omgång 7 / paket v08

Kontrollerat 26 september 2026. Period: 27 februari 1986 22.21.30–1 mars 22.21.30 UTC. Sex nya planerade fysiska flygsträckor ger totalt **699 rörelser**, varav 698 tidtabellslagda och en tidigare dokumenterad militär transport. Inga nya faktiskt genomförda flygningar har fastställts i denna omgång.

## Importerade tidtabellsrader

Alla klockslag i tabellen är lokala. Varje rad ger två avgångar som överlappar perioden.

| Bolag/flyg | Sträcka | Avgång–ankomst | Tryckt trafik | Sida |
|---|---|---|---|---|
| Pan Am 114 | New York JFK–Paris CDG | 18.45–07.40 nästa dag | Dagligen | 72 |
| Pan Am 114 | Paris CDG–Tel Aviv | 10.35–16.00 | Alla dagar utom tisdag/onsdag | 79 |
| Pan Am 115 | Paris CDG–New York JFK | 12.00–14.05 | Dagligen | 78 |

Primärkälla: [Pan Am, 11 februari–26 april 1986, University of Miami](https://digitalcollections.library.miami.edu/digital/collection/asm0341/id/37370/). Bild-ID:n: 37307, 37314 och 37313. Visuellt kontrollerade tabeller, inklusive undantag och nästa-dag-markering. Transkription finns i `israel_south_africa_batch07.tsv`; filhashar och rad-ID:n finns i `batch_07.json`.

Den redan importerade PA115 Tel Aviv–Paris, 05.45–09.35, kopplas till fortsättningen mot JFK. Genomgående PA114/115 har byte av flygplan i Paris enligt tabellens asterisk. Därför registreras två fysiska sträckor, aldrig ett påhittat nonstopflyg New York–Tel Aviv. Datakontrollen verifierar nattflygets datum och markuppehållen i Paris.

## Landvis fortsatt arbete

| Land/operatör | Kontrollerat | Kvarstår |
|---|---|---|
| Israel / El Al | [Omslag, vinter 1985/6, issue one](https://www.timetableimages.com/i-kl/ly8510a.jpg): 27 oktober 1985–29 mars 1986 | Avgångssidor, teckenförklaring och eventuella senare utgåvor/rättelser. Omslaget ger inga flygtider. |
| Israel / utländska bolag | Pan Am: Tel Aviv–Paris och retur; USA-delsträckorna ovan | Fler bolags rätta vinterutgåvor; hela trafiken till Israel är inte kartlagd. |
| Sydafrika / SAA | [Vinteromslag](https://www.timetableimages.com/i-s/sa851027.jpg), daterat 27 oktober 1985 i [arkivindexet](https://www.timetableimages.com/ttimages/sa.htm); omslaget anger 27 oktober–29 mars | Läsbara avgångssidor för internationell och inrikes trafik. Året kommer från indexet, inte från ett synligt årtal på omslaget. |
| Sydafrika / SAA och Air Zimbabwe | [Air Zimbabwe, november 1985](https://www.timetableimages.com/ttimages/um/um8511/um8511.pdf), PDF-sida 2: Johannesburg-raderna återkontrollerade | Inga nya rader från denna sida. Söndagsflygen hör fortsatt inte till fönstret. |
| Sydafrika / SAA, USA-utgåva | [Omslag, giltigt från 1 oktober 1985](https://www.airtimes.com/cgat/za/saa/2a/us/sa851001.jpg) | Avgångssidor och slutdatum saknas; ännu ingen import. |
| Israel / militär- och chartertrafik | Iranleveransen nedan, nu med kompletterande källhänvisning | Rörelsejournaler, exakta tider, operatör och flygplatser. |
| Sydafrika / SAAF | Sökning efter daterade flyguppgifter genomförd | Inga nya tidsatta flyg belagda; flygförbandsjournaler och transportloggar behövs. |

SAA har kvar åtta importerade SA-kodade avgångar. Noll nya fynd betyder inte att ingen ytterligare trafik förekom. El Als omslag ger status `catalog_found`, inte `scan_found` eller färdiginläst. Båda vinteromslagen finns som separata källposter i spelets underlag med `access: cover` och `window_reviewed: false`.

Andra kontrollerade spår: El Als fullständiga utgåva från mars 1985 är fel säsong; SAA:s aprilutgåva 1986 är för sen. [Brasiliens Guia Aeronáutico 1986](https://airline-memorabilia.blogspot.com/2012/07/brasil-guia-aeronautico-1986.html) har också kontrollerats: omslaget anger JULHO/86, nr 475 (juli 1986), alltså för sent för detta fönster. Ingen rad därifrån har importerats. Tidtabellsarkiven har luckor, så detta är inte en komplett inventering av 1986 års bolag.

## Irantransporten 27 februari – forskningsuppgift

Walshrapportens uppgift om 500 TOW till Iran kompletteras med [IPIS/TransArms, The Arms Flyers](https://ipisresearch.be/wp-content/uploads/2022/12/201102_The-Armsflyers.pdf), s. 24–25. Rapporten anger charterflyg Tel Aviv–Teheran den 27 februari. Fotnot 102 hänvisar samlat till kongressvittnesmål och National Security Archives kronologi; originalbilagan till just denna rad återstår att identifiera.

Uppgiften är märkt som en senare sammanställning i `military_research.json`. Exakta tider, flygplatser och operatör är inte fastställda. Den animeras inte och visas ännu inte som ett eget forskningskort i spelmenyn. Särskilt viktigt: datumbeteckningen den 27 februari belägger inte överlapp med det exakta 48-timmarsfönstret. Den tidigare februarileveransens flygplan, registreringar och färdväg förs inte över till den 27:e.

Originalskanningar återdistribueras inte. Källorna belägger ingen anknytning mellan dessa flygningar och Palmemordet.
