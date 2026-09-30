# Southwest – Phoenix och Las Vegas, v60 / omgång 59

248 nya planerade Southwest-rörelser. Totalt 3 471 rörelser: 3 470 scheduled och en tidigare confirmed. Southwest har nu 883 rörelser. Alla 3 223 tidigare rörelser, äldre schemarader och 21 kandidater är oförändrade. Flygtrafikmenyn och flygplanssilhuetten är bevarade.

## Import och dubbelkontroll

Phoenix s. 44–46 och Las Vegas s. 27–29 är visuellt granskade. 149 röda N/S-källrader transkriberas: 132 nya schemarader, fyra Houston-rader som redan finns i v59 samt 13 Phoenix–Las Vegas-rader som återkommer i båda stadsavsnitten. Matchningen kontrollerar flygnummer, ändpunkter, tider, veckodagar och datumförskjutning. Återkommande rader ger aldrig extra flygrörelser. Den separata kolumnen action och schedule i southwest_west_batch59.tsv gör varje beslut spårbart. Alla 13 upprepade rader stämmer mellan s. 28 och 45.

29 nya riktade platspar tillkommer. Fjorton flygplatsförbindelser har nya direktflyg i båda riktningarna; Tulsa–Phoenix tillkommer endast i riktningen mot Phoenix. Resorna från Phoenix till Tulsa anges med mellanlandning och får ingen antagen direktretur. Även San Francisco, Amarillo och andra redovisade anslutningar/mellanstopp hålls utanför nonstopimporten. Endast redovisade fysiska N/S-sträckor räknas.

| Riktad sträcka | Nya rörelser |
|---|---:|
| ABQ → LAS | 4 |
| ABQ → PHX | 21 |
| AUS-MUELLER → PHX | 5 |
| DEN-STAPLETON → PHX | 7 |
| ELP → LAS | 4 |
| ELP → PHX | 12 |
| LAS → ABQ | 6 |
| LAS → ELP | 4 |
| LAS → PHX | 13 |
| LAS → SAN | 4 |
| LAS → SAT | 3 |
| LAX → PHX | 14 |
| OKC → PHX | 2 |
| ONT → PHX | 10 |
| PHX → ABQ | 18 |
| PHX → AUS-MUELLER | 5 |
| PHX → DEN-STAPLETON | 8 |
| PHX → ELP | 14 |
| PHX → LAS | 13 |
| PHX → LAX | 16 |
| PHX → OKC | 6 |
| PHX → ONT | 11 |
| PHX → SAN | 14 |
| PHX → SAT | 2 |
| SAN → LAS | 7 |
| SAN → PHX | 14 |
| SAT → LAS | 2 |
| SAT → PHX | 5 |
| TUL → PHX | 4 |

## Datum och tidszoner

Utgåvan dateras 12 januari 1986 av arkivet, med nästa katalogiserade utgåva 18 mars i AirTimes. Ett fullständigt ändringsbladsbestånd och tryckt slutdatum är inte verifierade. Importen begränsas till lokala avgångsdatum 27 februari–1 mars. Tiderna tolkas som lokala enligt tidtabellskonvention; separat all-times-local-legend saknas i de granskade sidorna. Sidfoten definierar 1=måndag till 7=söndag och N/S=nonstop samt rött Southwest/blått Muse Air.

Phoenix använder America/Phoenix, Albuquerque/El Paso/Stapleton America/Denver (alla UTC−7 under importen). Kalifornien och Las Vegas har Pacific Time UTC−8, Texas/Oklahoma Central Time UTC−6. Phoenix–Las Vegas anländer fem minuter tidigare på destinationsklockan samma dag: verklig tidtabellslängd 55 minuter. Phoenix–Ontario/San Diego har samma lokala avgångs- och ankomstklockslag men tar 60 minuter. Datumförskjutning anges explicit och härleds inte bara från klockslagens ordning. Alla konverteringar granskas oberoende med fasta vinteroffset och ZoneInfo.

Rörelser per lokalt avgångsdatum: {'1986-02-27': 61, '1986-02-28': 123, '1986-03-01': 64}. 7 nya schemarader skapar ingen rörelse inom fönstret och listas i batch_59.json. Söndagsvarianter kan därför vara granskade utan att animeras. Flyg väljs med intervallöverlapp mot 1986-02-27T22:21:30Z–1986-03-01T22:21:30Z och behåller hela tider även över fönstrets gränser.

## Historiska flygplatser och kvarstående arbete

Tre flygplatsmarkörer tillkommer: DEN-STAPLETON, LAS och ONT. Denver avser nedlagda Stapleton, inte Denver International som öppnade 28 februari 1995. Stapleton har egen post och historiska koordinater. Las Vegas visas som McCarran; dagens namn Harry Reid i koordinatregistret backdateras inte. Ontario ligger i Kalifornien. Austin använder fortsatt Mueller. Koordinaterna är ungefärliga flygfältspositioner, inte rekonstruerade gater eller faktiska flygplanspositioner.

Tidtabellen belägger planerad trafik, inte genomförande eller verkliga flygbanor. Flygplanens registreringar och fullständiga tjänstekedjor är inte verifierade. Inga faktiska överflygningar tillkommer. Southwest är fortsatt delvis kartlagt; kvarvarande Albuquerque-, El Paso- och Kalifornien-avsnitt är lämpliga nästa steg, med dubblettkontroll mot Dallas, Houston, Phoenix och Las Vegas. Muse Air och ändringsblad återstår. Deltas kompletta februariutgåva finns som ytterligare stort forskningsspår.

Norden är oförändrat: 161 unika ändpunktsrörelser; Sverige 87, Norge 52, Danmark 85, Finland 4, Island 0. Europakön behåller Cypern. 94 registrerade operatörer, 48 med rörelser och 46 utan. Inget bolag eller land är verifierat fullständigt. Globalt totalantal bolag 1986 och global täckningsprocent är inte fastställda.

## Källor och validering

- https://www.departedflights.com/WN011286intro.html — daterat tidtabellsindex.
- https://www.airtimes.com/cgat/usc/southwest.htm — katalogiserade utgivningsdatum.
- https://www.departedflights.com/WN011286p27.jpg — tryckta s. 27–28.
- https://www.departedflights.com/WN011286p29.jpg — tryckta s. 29–30.
- https://www.departedflights.com/WN011286p44.jpg — tryckta s. 44–45.
- https://www.departedflights.com/WN011286p46.jpg — tryckta s. 46–47.
- https://raw.githubusercontent.com/datasets/airport-codes/main/data/airport-codes.csv — koordinatregister.
- https://ourairports.com/airports/US-12484/ — Denver–Stapleton International (1986), flygfältets läge.
- https://www.flydenver.com/press-release/denver-international-airport-celebrates-25-years-of-success-and-growth/ — historiskt plats-/namnstöd.
- https://ourairports.com/airports/KLAS/ — Las Vegas–McCarran International (1986), flygfältets läge.
- https://www.clarkcountynv.gov/adobe/assets/urn%3Aaaid%3Aaem%3A1885026b-ec70-4a66-b176-cac868ab2cc7/original/as/history-timeline.pdf — historiskt plats-/namnstöd.
- https://ourairports.com/airports/KONT/ — Ontario International, California, flygfältets läge.
- https://www.flyontario.com/history — historiskt plats-/namnstöd.

Originalbilder omdistribueras inte. URL, SHA-256 och granskade sidor finns i southwest_source_evidence_batch59.json. southwest_west_batch59.tsv innehåller 149 källrader och deras importbeslut. validation_batch59.json redovisar kalender/UTC, dubbletter, överlapp för samma flygnummer mot tidigare och nya Southwest-rörelser, bevarande av äldre filer, 20 Python-tester, tre byggkontroller och börsdatavalidering. Unreal-kompilering och Editor-körning har inte utförts här.
