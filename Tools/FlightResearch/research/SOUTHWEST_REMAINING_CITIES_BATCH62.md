# Southwest – återstående stadsavsnitt, v63 / omgång 62

32 nya planerade Southwest-rörelser på åtta riktade platspar. En felaktig äldre lördagsrörelse tas bort. Netto +31, totalt 3 807 rörelser: 3 806 scheduled och en tidigare confirmed. Southwest har nu 1 219 rörelser. 3 775 tidigare rörelse-ID och deras tider är bevarade. Tre kvarvarande rörelser får endast uppdaterade anteckningar efter två korrigerade schemans trafikdagar. Flygtrafikmenyn, flygplanssilhuetten och all spelkod är bevarade.

## Källgenomgång

Amarillo s. 4–6, Austin 7–9, Corpus Christi 11–13, Denver 16–17, Houston Intercontinental 24–25, Lubbock 33–35, Midland/Odessa 35–38, New Orleans 38–40, Oklahoma City 40–42, Harlingen 46–48 och San Antonio 48–51 har granskats visuellt. 358 röda N/S-rader: 16 nya scheman, 324 tidigare importerade, två rättade äldre och 16 upprepningar av nya scheman i den motsatta stadens avsnitt. Alla beslut finns i southwest_remaining_cities_batch62.tsv. De 16 nya schemana stämmer mellan båda ändpunkternas sidor; 16 upprepade källrader skapar inga extra rörelser.

Endast uttryckliga röda N/S-rader importeras. Sidfotens N/S betyder nonstop; blå flygnummer tillhör Muse Air. Numeriska stoppantal och flygnummer med snedstreck importeras inte som nonstop. Direktförbindelser antas inte enbart för att orterna finns i nätet. Övriga granskade stadsavsnitt gav inga ytterligare nya scheman.

| Riktad sträcka | Nya rörelser |
|---|---:|
| AUS-MUELLER → HRL | 2 |
| AUS-MUELLER → LBB | 5 |
| AUS-MUELLER → MAF | 6 |
| HRL → AUS-MUELLER | 2 |
| HRL → SAT | 4 |
| LBB → AUS-MUELLER | 3 |
| MAF → AUS-MUELLER | 6 |
| SAT → HRL | 4 |

## Två korrigerade trafikdagar

WN906 Lubbock–Albuquerque 16:40–16:40 anges dagar 123457 på både s. 3 och 33. WN736 San Antonio–El Paso 12:05–12:30 anges dagar 123457 på både s. 20 och 49. Båda hade felaktigt importerats som dagliga (1234567). Lördag tas bort från schemana.

Detta tar bort WN736 den 1 mars 1986, tidigare 18:05–19:30 UTC. Dess ID och fullständiga tidigare post finns i corrections_batch62.json. WN906:s lördagsavgång låg redan utanför fönstret och hade därför inte skapat en rörelse. Två kvarvarande WN906-rörelser och en WN736-rörelse får endast rättade anteckningar. Ingen kvarvarande rörelse har fått ändrade tider. Tidigare rättelser av WN797, WN703 och WN717 är bevarade. Äldre rapporter och TSV-filer behålls som historiska ögonblicksbilder; den aktuella katalogen och rättelsefilen gäller.

## Datum och tidskontroll

Alla 16 nya scheman ger rörelser i fönstret. Nya lokala avgångsdatum: 27 februari fem, 28 februari sexton och 1 mars elva. Alla nya ändpunkter använder Central Standard Time UTC−6. WN939 Lubbock–Austin avgår 23:00 och anländer 00:00 nästa kalenderdag; fredagens avgång blir 1 mars 05:00–06:00 UTC. Övriga nya scheman anländer samma lokala kalenderdag. Austin avser historiska Mueller. Inga flygplatser eller koordinater tillkommer.

Intervallöverlapp mot 1986-02-27T22:21:30Z–1986-03-01T22:21:30Z avgör urvalet; hela flygtider bevaras över fönstrets gränser. Avgångar begränsas till lokala datum 27 februari–1 mars. Datum och UTC kontrolleras separat med fasta vinteroffset och ZoneInfo. Flygnummer återanvänds på delsträckor; detta är inte ett belägg för samma individflygplan.

## Täckning och fortsättning

Tillsammans med tidigare omgångar har stadsavsnitten nu gåtts igenom för uttryckliga röda Southwest N/S-rader. Detta är inte en oberoende fullständighetskontroll av alla fysiska flygningar, genomförd trafik eller fullständiga tjänstekedjor. Muse Air, eventuella ej separat redovisade delsträckor och senare ändringsblad återstår. Southwest har därför fortsatt partiell forskningsstatus. Nästa stora källa är Deltas tidtabell från 1 februari 1986, med möjliga luckor kring Atlanta, Dallas/Fort Worth och Cincinnati.

Arkivet daterar Southwest-utgåvan till 12 januari 1986; nästa katalogiserade utgåva är 18 mars enligt AirTimes. Tryckt slutdatum och fullständiga ändringsblad är inte verifierade. Tider tolkas som lokala enligt tidtabellskonvention; separat all-times-local-legend har inte återfunnits. Planerad trafik är inte belagt genomförande, och rutter ritas schematiskt.

Norden är oförändrat: 161 unika ändpunktsrörelser; Sverige 87, Norge 52, Danmark 85, Finland 4 och Island 0. Europakön behåller Cypern. Registret har 94 operatörer, 48 med rörelser och 46 utan. Inget bolag eller land är verifierat fullständigt. Antalet bolag worldwide 1986 och global täckningsprocent är inte fastställda.

## Källor och validering

- https://www.departedflights.com/WN011286intro.html — daterat tidtabellsindex.
- https://www.airtimes.com/cgat/usc/southwest.htm — katalogiserade utgivningsdatum.
- https://www.departedflights.com/WN011286p3.jpg — tryckt s. 3–4.
- https://www.departedflights.com/WN011286p5.jpg — tryckt s. 5–6.
- https://www.departedflights.com/WN011286p7.jpg — tryckt s. 7–8.
- https://www.departedflights.com/WN011286p9.jpg — tryckt s. 9–10.
- https://www.departedflights.com/WN011286p11.jpg — tryckt s. 11–12.
- https://www.departedflights.com/WN011286p13.jpg — tryckt s. 13–14.
- https://www.departedflights.com/WN011286p15.jpg — tryckt s. 15–16.
- https://www.departedflights.com/WN011286p17.jpg — tryckt s. 17–18.
- https://www.departedflights.com/WN011286p19.jpg — tryckt s. 19–20.
- https://www.departedflights.com/WN011286p23.jpg — tryckt s. 23–24.
- https://www.departedflights.com/WN011286p25.jpg — tryckt s. 25–26.
- https://www.departedflights.com/WN011286p32.jpg — tryckt s. 32–33.
- https://www.departedflights.com/WN011286p34.jpg — tryckt s. 34–35.
- https://www.departedflights.com/WN011286p36.jpg — tryckt s. 36–37.
- https://www.departedflights.com/WN011286p38.jpg — tryckt s. 38–39.
- https://www.departedflights.com/WN011286p40.jpg — tryckt s. 40–41.
- https://www.departedflights.com/WN011286p42.jpg — tryckt s. 42–43.
- https://www.departedflights.com/WN011286p46.jpg — tryckt s. 46–47.
- https://www.departedflights.com/WN011286p48.jpg — tryckt s. 48–49.
- https://www.departedflights.com/WN011286p50.jpg — tryckt s. 50–51.

Originalbilder omdistribueras inte. URL och SHA-256 finns i southwest_source_evidence_batch62.json. validation_batch62.json redovisar kalender/UTC, dubbletter, samma flygnummer, bevarande och rättelser, 20 Python-tester, tre byggkontroller och börsdatavalidering. Unreal-kompilering och Editor-körning har inte utförts här.
