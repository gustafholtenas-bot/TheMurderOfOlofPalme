# Southwest – Albuquerque och El Paso, v61 / omgång 60

175 nya planerade Southwest-rörelser. Totalt 3 646 rörelser: 3 645 scheduled och en tidigare confirmed. Southwest har nu 1058 rörelser. Alla 3 471 tidigare rörelse-ID finns kvar; 3 469 rörelser är exakt oförändrade och två får rättad ankomsttid enligt nedan. Flygtrafikmenyn och flygplanssilhuetten är bevarade.

## Import och källgranskning

Albuquerque s. 2–4 och El Paso s. 17–20 är visuellt granskade. 170 röda N/S-källrader transkriberas: 94 nya schemarader, 65 redan importerade rader, en rättad tidigare rad och tio Albuquerque–El Paso-rader som återkommer i båda stadsavsnitten. Matchning sker på flygnummer, ändpunkter, tider, dagar och datumförskjutning. Tio nya ABQ–ELP/ELP–ABQ-scheman stämmer mellan s. 2 och 17. source_row, action och schedule i southwest_abq_elp_batch60.tsv gör varje beslut spårbart. Houston–El Paso 736 har ett mellanstopp (via San Antonio) och importeras inte som nonstop. Endast uttryckliga röda N/S-rader används; blå Muse Air-nummer och anslutningar utesluts.

32 nya riktade platspar tillkommer. Tabellen visar fysiska nonstopsträckor, inte genomgående tjänster med okända mellanlandningstider.

| Riktad sträcka | Nya rörelser |
|---|---:|
| ABQ → AMA | 6 |
| ABQ → DEN-STAPLETON | 6 |
| ABQ → ELP | 7 |
| ABQ → LAX | 6 |
| ABQ → LBB | 3 |
| ABQ → MAF | 2 |
| ABQ → MCI | 4 |
| ABQ → SAN | 7 |
| ABQ → SAT | 2 |
| ABQ → SFO | 4 |
| AMA → ABQ | 6 |
| AUS-MUELLER → ELP | 4 |
| DEN-STAPLETON → ABQ | 9 |
| ELP → ABQ | 9 |
| ELP → AUS-MUELLER | 8 |
| ELP → LAX | 6 |
| ELP → LBB | 5 |
| ELP → MAF | 7 |
| ELP → SAN | 2 |
| ELP → SAT | 9 |
| LAX → ABQ | 7 |
| LAX → ELP | 8 |
| LBB → ABQ | 2 |
| LBB → ELP | 8 |
| MAF → ABQ | 4 |
| MAF → ELP | 9 |
| MCI → ABQ | 4 |
| SAN → ABQ | 5 |
| SAN → ELP | 2 |
| SAT → ABQ | 4 |
| SAT → ELP | 6 |
| SFO → ABQ | 4 |

## Rättelse av tidigare import

Southwest 797 El Paso–Houston avgår 21:25 och anländer 00:05 nästa dag. Tryckta s. 18 och 22 överensstämmer. I v59/v60 stod felaktigt 00:25. Samma schema-ID behålls och två tidigare rörelser, med avgång 27 respektive 28 februari, får 20 minuter tidigare ankomst. Övriga tidigare scheman och rörelser är oförändrade. corrections_batch60.json innehåller kompletta före/efter-poster och motiv. Äldre batchrapporter och TSV-filer behålls som historiska granskningsögonblicksbilder; denna rättelse ersätter deras 00:25-värde för den aktiva katalogen.

## Datum, tidszoner och flygplats

Utgåvan dateras 12 januari 1986 av arkivet, med nästa katalogiserade utgåva 18 mars i AirTimes. Tryckt slutdatum och fullständiga ändringsblad är inte verifierade. Importen begränsas till lokala avgångsdatum 27 februari–1 mars. Tiderna tolkas som lokala enligt tidtabellskonvention; en separat all-times-local-legend saknas på de granskade sidorna. Sidfoten definierar 1=måndag–7=söndag, N/S=nonstop och rött Southwest/blått Muse Air.

Albuquerque och El Paso använder America/Denver (UTC−7); Texas/Oklahoma/Kansas City America/Chicago (UTC−6); Kalifornien America/Los_Angeles (UTC−8). Phoenix använder America/Phoenix (UTC−7). Lubbock–El Paso tar 55 minuter trots att ankomstklockan ligger fem minuter före avgångsklockan. Midland/Odessa–El Paso tar 50 minuter trots tio minuter tidigare destinationsklocka. Albuquerque–San Antonio 964 anländer 00:20 nästa kalenderdag. Datumförskjutning är explicit och granskas med fasta vinteroffset och ZoneInfo.

Rörelser per lokalt avgångsdatum: {'1986-02-27': 44, '1986-02-28': 89, '1986-03-01': 42}. 4 nya schemarader skapar inga rörelser i fönstret och listas i batch_60.json. Flyg väljs med intervallöverlapp mot 1986-02-27T22:21:30Z–1986-03-01T22:21:30Z och behåller hela tider över gränserna.

Kansas City International (MCI) tillkommer som flygplatsmarkör. Den togs i kommersiell drift i november 1972 och skiljs från downtown MKC. Koordinaterna avser ungefärligt flygfältsläge, inte 1986 års gatepositioner eller 2023 års nya terminal. Denver avser fortsatt Stapleton, Austin Mueller och Las Vegas McCarran.

## Täckning och kvarstående arbete

Tidtabellen belägger planerad trafik, inte genomförande eller verkliga flygbanor. Individuella flygplan, fullständiga tjänstekedjor och ändringsblad är inte verifierade. Southwest är fortsatt delvis kartlagt. Kvarvarande Kalifornien-, Amarillo-, Lubbock-, Midland/Odessa- och andra avsnitt kan ge fler sträckor efter dubblettkontroll. Muse Air återstår. Deltas kompletta februariutgåva är ett ytterligare stort forskningsspår.

Norden är oförändrat: 161 unika ändpunktsrörelser; Sverige 87, Norge 52, Danmark 85, Finland 4, Island 0. Europakön behåller Cypern. 94 registrerade operatörer, 48 med rörelser och 46 utan. Inget bolag eller land är verifierat fullständigt. Globalt totalantal bolag 1986 och global täckningsprocent är inte fastställda.

## Källor och validering

- https://www.departedflights.com/WN011286intro.html — daterat tidtabellsindex.
- https://www.airtimes.com/cgat/usc/southwest.htm — katalogiserade utgivningsdatum.
- https://www.departedflights.com/WN011286p2.jpg — tryckt s. 2 (en sida).
- https://www.departedflights.com/WN011286p3.jpg — tryckt s. 3–4.
- https://www.departedflights.com/WN011286p17.jpg — tryckt s. 17–18.
- https://www.departedflights.com/WN011286p19.jpg — tryckt s. 19–20.
- https://www.departedflights.com/WN011286p21.jpg — tryckt s. 21–22.
- https://ourairports.com/airports/KMCI/ — flygfältets identitet och läge.
- https://flykc.com/about-us — flygplatsens egen historik.

Originalbilder omdistribueras inte. URL och SHA-256 finns i southwest_source_evidence_batch60.json. validation_batch60.json redovisar kalender/UTC, dubbletter, överlapp för samma flygnummer mot tidigare och nya Southwest-rörelser, avgränsad rättelse, bevarande av äldre filer, 20 Python-tester, tre byggkontroller och börsdatavalidering. Unreal-kompilering och Editor-körning har inte utförts här.
