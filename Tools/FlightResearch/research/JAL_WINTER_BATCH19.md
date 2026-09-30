# Omgång 19 / paket v20 – JAL:s vintertrafik

Granskat 28 september 2026. Kumulativ fortsättning från v19.

## Resultat

**33 nya planerade flygrörelser** från **24 visuellt granskade tidtabellsrader**. JAL tillkommer som animerat bolag. Tio riktade platspar är nya för projektet. Totalt finns **1 280 rörelser: 1 279 planerade och en tidigare dokumenterat genomförd**. Alla 1 247 rörelser från v19 är exakt oförändrade.

Registret omfattar nu 845 godkända tidtabellsrader, 368 riktade platspar, 148 flygplatser/platser, 30 animerade bolag och 38 källposter. De 79 registrerade operatörerna och 69 länderna/territorierna är oförändrade. Åtta tidigare kandidatrader är fortsatt undantagna från animation.

| Flygnummer | Granskat urval | Rörelser i fönstret |
|---|---|---:|
| JL418 | Amsterdam–Köpenhamn–Anchorage, lördag | 2 |
| JL412 | Amsterdam–Anchorage–Narita, torsdag | 2 |
| JL436 | Frankfurt–Düsseldorf–Anchorage–Narita, fredag | 3 |
| JL5 | New York/JFK–Narita | 3 |
| JL11 | Vancouver–Narita | 1 |
| JL9 | Chicago/O’Hare–Seattle | 1 |
| JL61 | San Francisco–Narita | 3 |
| JL3 | San Francisco–Honolulu–Narita fredag; San Francisco–Narita lördag | 3 |
| JL1 | Vancouver–Narita | 3 |
| JL77/JL73/JL75 | Honolulu–Narita; JL75:s reguljära fredagskolumn | 7 |
| JL41 | Fraktflyg New York/JFK–Anchorage–Narita | 5 |
| **Totalt** | | **33** |

## Källans status och spårbarhet

Källan är **Japan Air Lines, Passenger–Cargo Timetable, Advance Edition**, giltig **1 november 1985–31 mars 1986**. Exakta datum står på Stillahavsuppslaget. Omslaget säger förhandsutgåva, och förbehåll om ändringar och myndighetsgodkännande står i den tryckta tabellen.

**Detta är publicerad planerad trafik enligt en förhandsutgåva. En senare slutlig utgåva eller ändringsblad har inte verifierats.** Den begränsningen finns också i varje ny flygposts svenska och engelska anmärkning. Ingen ny post har status `confirmed`.

Tre fotografier identifierades i den offentliga eBay-kategorisidan som bilder för [annons 198537995729](https://www.ebay.co.uk/itm/198537995729):

- [Omslag](https://i.ebayimg.com/images/g/OIEAAOSw4UpmxkVu/s-l1600.jpg).
- [Europe: polar route / via Moscow](https://i.ebayimg.com/images/g/5WoAAOSwe6JmxkVv/s-l1600.jpg).
- [Transpacific / Europe: southern route](https://i.ebayimg.com/images/g/42kAAOSwoQtmxkVx/s-l1600.jpg).

Tider, flightnummer, kolumngränser, veckodagar och symboler har lästs visuellt. Panelrubrik och bild-ID används som sidhänvisning eftersom tryckta sidnummer inte är läsbara. Raderna i de granskade inåtgående tabellerna läses **nedifrån och upp**: ankomst och efterföljande avgång på en mellanlandning får inte byta plats.

`jal_batch19.tsv` innehåller transkriptionen och `batch_19.json` bildlänkar, storlekar och SHA-256 för de granskade 1 600-pixelsbilderna. Fotografierna ingår inte i ZIP-filen. Importerade avgångsrader är avgränsade till **26 februari–1 mars 1986**; hela vintersäsongen är inte rekonstruerad.

## Norden och fönstrets gränser

JL418:s kolumn anger **lördag**. Lördagen den 1 mars ger två rörelser:

| Delsträcka | Avgång lokal tid | Ankomst lokal tid | UTC-intervall |
|---|---|---|---|
| Amsterdam–Köpenhamn | 12.30 CET | 13.50 CET | 11.30–12.50 |
| Köpenhamn–Anchorage | 15.05 CET | 13.40 AKST | 14.05–22.40 |

Markuppehållet i Köpenhamn är 75 minuter. Anchorageflyget är fortfarande i luften när fönstret slutar 22.21.30 UTC och ska därför ingå. Nästa del, Anchorage–Narita 14.55–16.15 nästa dag, är granskad men startar **23.55 UTC**, efter fönstrets slut, och animeras inte.

Nu finns **46 rörelser med nordisk ändpunkt**: Sverige 14, Norge 36, Danmark 3, Finland 0 och Island 0. Landtalen överlappar. JL416 via Köpenhamn anges som måndag och ger ingen rörelse i detta fönster. Utgående JAL-polarrutter är bara delvis synliga och har inte fyllts ut genom antaganden.

Övriga tre granskade rader som inte överlappar fönstret är JL412 Rom–Amsterdam, JL11 Mexico City–Vancouver och JL9 Seattle–Narita. Den sista avgår lördag **22.25 UTC**, bara 3 minuter och 30 sekunder efter slutgränsen; den är korrekt utesluten.

SAS-annonsen [375823217337](https://www.ebay.com/itm/375823217337) gav bara ett omslag. Aeroflots vinterindex och Icelandairs omslag gav inga ytterligare läsbara avgångsrader. Tidningssökningen via Tímarit.is kunde inte hämtas. Dessa luckor har inte fyllts med påhittade tider. Inga faktiska överflygningar har belagts.

## Delsträckor och avgränsningar

- Narita är uttryckligen angivet i tabellen. Anchorage använder historisk **UTC−9**, Vancouver och Seattle UTC−8, Honolulu UTC−10 och Narita UTC+9. Ankomster över datumlinjen har rätt lokalt ankomstdygn.
- JL3:s fredag går via Honolulu; lördagsvarianten är nonstop från San Francisco. De har separata resegrupper. Fortsättningen Narita–Hongkong på söndagen ligger utanför fönstret och ingår inte i urvalet.
- JL75:s fredag har inga december–januari-begränsningar i kolumnen. De intilliggande, datumbegränsade JL75-varianterna har inte lagts in för februari/mars.
- Anslutningarna JL51/52 till/från Osaka och JL615/JL715 har inte tolkats som extra långdistansavgångar. Inte heller São Paulo–Rio med VASP, gemensamma AF/BA/LH-fraktnummer eller oklara variantkolumner har importerats som separata JAL-flyg.
- Trafikrättsbegränsningen Europa–Anchorage gäller rätten att resa enbart på delsträckan; den tar inte bort det fysiska flygsegmentet.
- JL41 är märkt frakt och B747F i tabellen. Dess markuppehåll i Anchorage är 60 minuter.
- Storcirklarna är en illustration av tidtabellens ändpunkter. De visar inte belagda färdvägar, territoriepassager eller överflygningar.

## Flygplatser

**Anchorage International (ANC)** och **Vancouver International (YVR)** tillkommer. Det historiska Anchorage-namnet används i stället för det senare tillägget Ted Stevens. Koordinater och länkar finns i `airport_locations_batch19.json`.

Flygplatsmyndigheten i Alaska beskriver [Anchorageflygplatsens etablering 1951](https://dot.alaska.gov/anc/passengers-about.shtml). YVR:s egen historik anger [JAL-trafik sedan september 1968](https://news.yvr.ca/jal-celebrates-50-years-of-service-to-vancouver/). Kopplingen New York–JFK stöds av [JAL:s egen historik](https://press.jal.co.jp/en/release/201608/003886.html), och Chicago–O’Hare av [Chicago Department of Aviation](https://www.linkedin.com/posts/chicago-department-of-aviation_on-this-day-in-1983-japan-airlines-flew-activity-7048028581687398400-5zm9). Dessa historikkällor identifierar flygplatserna; exakta flygtider kommer från den tryckta vintertabellen.

## Gränssnitt och kontroll

Den egna pausmenykategorin **FLYGTRAFIK KRING MORDET** och flygplanssilhuetten från v18 följer med. Silhuetten följer färdriktningen. V20 ändrar inga C++-/Python-kodfiler eller börsdata.

20 Python-tester passerade. Genererad JSON, landindex och börsdata stämmer. Kontrollen bekräftar 1 247 oförändrade äldre rörelser och 22 oförändrade kod-/börsfiler. Inga nya fysiska dubbletter eller tidsöverlapp med samma JAL-flygnummer hittades. Markuppehåll, datumlinjen, gränsfallen vid fönstrets slut och rimliga flygtider kontrollerades. Detaljer finns i `validation_batch19.json`.

Paketet är kumulativt. Lägg `Plugins/` och `Tools/` i projektroten enligt `LAS_MIG_v20.txt`. **Unreal-kompilering och spelkörning har inte utförts i denna miljö.**
