# Southwest – Kalifornien och Midwest, v62 / omgång 61

130 nya planerade Southwest-rörelser. Totalt 3 776 rörelser: 3 775 scheduled och en tidigare confirmed. Southwest har nu 1188 rörelser. Alla 3 646 tidigare rörelse-ID och tider är bevarade. Tre befintliga rörelser får enbart rättade anteckningar efter korrigering av två schemans trafikdagar. Flygtrafikmenyn, flygplanssilhuetten och all spelkod är bevarade.

## Import och dubbelkontroll

Los Angeles s. 30–32, Ontario s. 43–44, San Diego s. 51–53, San Francisco s. 53–54, Chicago Midway s. 10–11, Kansas City s. 25–27, St. Louis s. 55–56, Little Rock s. 29–30 och Tulsa s. 56–58 har granskats visuellt. 244 röda N/S-källrader: 66 nya scheman, 129 tidigare importerade, två korrigerade tidigare och 47 återkommande rader. Matchningen kontrollerar flygnummer, ändpunkter, tider, veckodagar och datumförskjutning. Alla importbeslut redovisas i southwest_california_midwest_batch61.tsv.

De 47 upprepningarna gäller SAN–SFO (12), MDW–MCI (5), MDW–STL (18), MCI–TUL (6) och STL–LIT (6), i båda riktningarna. De ger inte extra scheman eller rörelser. Nästan alla övriga Kalifornien-rader fanns redan i tidigare stadsimporter. Endast uttryckliga röda N/S-rader importeras; blå Muse Air-nummer, anslutningar och rader med mellanstopp utesluts. IAH–Tulsa flight 20 är nonstop i denna riktning; returresorna i Tulsa-avsnittet har mellanstopp eller anslutning och får ingen antagen nonstopretur.

21 nya riktade platspar tillkommer:

| Riktad sträcka | Nya rörelser |
|---|---:|
| IAH → TUL | 1 |
| LIT → MSY | 4 |
| LIT → STL | 6 |
| MAF → TUL | 2 |
| MCI → MDW | 4 |
| MCI → OKC | 6 |
| MCI → TUL | 6 |
| MDW → MCI | 4 |
| MDW → STL | 17 |
| MSY → LIT | 4 |
| MSY → STL | 2 |
| OKC → MCI | 6 |
| OKC → TUL | 7 |
| SAN → SFO | 12 |
| SFO → SAN | 13 |
| STL → LIT | 6 |
| STL → MDW | 19 |
| STL → MSY | 2 |
| TUL → MAF | 1 |
| TUL → MCI | 6 |
| TUL → OKC | 2 |

## Korrigering av trafikdagar

WN703 Los Angeles–Phoenix 07:30–09:40 anges måndag och fredag (15) på både s. 32 och 45; tidigare import hade bara fredag (5). WN717 San Diego–Phoenix 19:25–21:25 anges måndag–fredag (12345) på både s. 46 och 52; tidigare import hade även söndag (123457). De avvikande måndags-/söndagsdagarna ligger utanför de använda lokala datumen 27 februari–1 mars. Därför ändras inga tidigare avgångar eller ankomsttider och inga rörelser tillkommer eller försvinner till följd av rättelserna. Tre rörelsers anteckningar uppdateras. Fullständiga före/efter-poster finns i corrections_batch61.json. Äldre granskningsrapporter och TSV-filer behålls som historiska ögonblicksbilder. Rättelsen av WN797 från v61 är bevarad.

## Datum, tider och historisk flygplats

Utgåvan dateras 12 januari 1986 av arkivet; nästa katalogiserade utgåva är 18 mars enligt AirTimes. Tryckt slutdatum och fullständiga ändringsblad är inte verifierade. Importen begränsas till lokala avgångsdatum 27 februari–1 mars. Tiderna tolkas som lokala enligt tidtabellskonvention; separat all-times-local-legend har inte återfunnits. Sidfoten definierar 1=måndag–7=söndag, N/S=nonstop och rött Southwest/blått Muse Air.

Nya Kalifornien-flyg använder Pacific Standard Time (UTC−8), alla övriga nytillkomna sträckor Central Standard Time (UTC−6). Samtliga nya scheman har ankomst samma lokala kalenderdag. Datum och tider prövas oberoende med fasta vinteroffset och ZoneInfo. Rörelser per lokalt avgångsdatum: {'1986-02-27': 29, '1986-02-28': 65, '1986-03-01': 36}. 1 nya schemarader skapar ingen rörelse inom fönstret och listas i batch_61.json. Intervallöverlapp mot 1986-02-27T22:21:30Z–1986-03-01T22:21:30Z avgör urvalet, och hela flygtider bevaras vid fönstrets gränser.

Chicago Midway (MDW) tillkommer med separat flygplatsmarkör, skild från O’Hare (ORD). Stadens flygplatshistorik och Southwests egen jubileumsartikel styrker flygfältets identitet och Southwest-trafik där sedan mars 1985. Koordinaterna avser ungefärligt flygfältsläge, inte rekonstruerade gater eller flygplanspositioner 1986. Historiska Denver–Stapleton, Austin–Mueller och namnet Las Vegas–McCarran bevaras.

## Täckning och fortsättning

Planerad trafik är inte belagt genomförande. Flygbanorna är schematiska; individflygplan och fullständiga tjänstekedjor är inte verifierade. Southwest är fortsatt delvis kartlagt trots att fler stadsavsnitt nu är genomgångna för N/S-rader. Återstående luckor kan finnas i Amarillo, Austin, Corpus Christi, Midland/Odessa, Lubbock, Harlingen, San Antonio och New Orleans. Muse Air och ändringsblad återstår. Deltas februariutgåva är ytterligare ett stort forskningsspår.

Norden oförändrat: 161 unika ändpunktsrörelser; Sverige 87, Norge 52, Danmark 85, Finland 4, Island 0. Europakön behåller Cypern. 94 registrerade operatörer, 48 med rörelser och 46 utan. Inget bolag eller land är verifierat fullständigt. Globalt antal bolag 1986 och global täckningsprocent är inte fastställda.

## Källor och validering

- https://www.departedflights.com/WN011286intro.html — daterat tidtabellsindex.
- https://www.airtimes.com/cgat/usc/southwest.htm — katalogiserade utgivningsdatum.
- https://www.departedflights.com/WN011286p9.jpg — tryckt s. 9–10.
- https://www.departedflights.com/WN011286p11.jpg — tryckt s. 11–12.
- https://www.departedflights.com/WN011286p25.jpg — tryckt s. 25–26.
- https://www.departedflights.com/WN011286p27.jpg — tryckt s. 27–28.
- https://www.departedflights.com/WN011286p29.jpg — tryckt s. 29–30.
- https://www.departedflights.com/WN011286p31.jpg — tryckt s. 31 (en sida).
- https://www.departedflights.com/WN011286p32.jpg — tryckt s. 32–33.
- https://www.departedflights.com/WN011286p42.jpg — tryckt s. 42–43.
- https://www.departedflights.com/WN011286p44.jpg — tryckt s. 44–45.
- https://www.departedflights.com/WN011286p46.jpg — tryckt s. 46–47.
- https://www.departedflights.com/WN011286p50.jpg — tryckt s. 50–51.
- https://www.departedflights.com/WN011286p52.jpg — tryckt s. 52–53.
- https://www.departedflights.com/WN011286p54.jpg — tryckt s. 54–55.
- https://www.departedflights.com/WN011286p56.jpg — tryckt s. 56–57.
- https://www.departedflights.com/WN011286p58.jpg — tryckt s. 58 (en sida).
- https://ourairports.com/airports/KMDW/ — flygfältets identitet och läge.
- https://www.flychicago.com/business/cda/pages/midway.aspx — Chicagos flygplatshistorik.
- https://investors.southwest.com/news-events/press-releases/detail/1488/southwest-airlines-celebrates-25-years-of-luv-in-chicago — Southwest om trafikstart vid Midway.

Originalbilder omdistribueras inte. URL och SHA-256 finns i southwest_source_evidence_batch61.json. validation_batch61.json redovisar kalender/UTC, dubbletter, överlapp för samma flygnummer, bevarande av tidigare rörelsetider, 20 Python-tester, tre byggkontroller och börsdatavalidering. Unreal-kompilering och Editor-körning har inte utförts här.
