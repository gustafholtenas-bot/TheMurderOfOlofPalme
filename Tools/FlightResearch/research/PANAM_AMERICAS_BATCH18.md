# Omgång 18 / paket v19 – Pan Am i Amerika

Granskat 28 september 2026. Kumulativ fortsättning från v18.

## Resultat

**65 nya planerade flygrörelser** från 32 visuellt granskade tidtabellsrader, fördelade på **30 nya riktade platspar**. Totalt finns **1 247 rörelser: 1 246 planerade och en tidigare dokumenterat genomförd rörelse**. Samtliga 1 182 rörelser från v18 är exakt oförändrade.

Nu finns 821 godkända tidtabellsrader, 358 riktade platspar, 146 flygplatser/platser, 69 länder/territorier och 79 registrerade operatörer. Pan Am har 321 rörelser i urvalet. Venezuela samt Trinidad och Tobago tillkommer som geografiska poster; deras inhemska flygbolag är ännu inte inventerade. Landindexet är uppdaterat.

| Flygnummer | Nytillkomna delar | Rörelser |
|---|---|---:|
| PA201/202 | New York/JFK–Rio/Galeão–Buenos Aires/Ezeiza och retur | 9 |
| PA440/441 | Rio–Miami–Los Angeles och retur | 9 |
| PA453/454 | Miami–Buenos Aires–Santiago och retur | 6 |
| PA217/218 | New York/JFK–Caracas–Port of Spain och retur | 8 |
| PA442/445 | Caracas–Miami och retur | 4 |
| PA409/410 | Miami–Maracaibo och retur | 4 |
| PA463/464/498/499 | Miami–Mexico City och retur, samt Washington/National–Miami | 10 |
| PA456/457 | Mexico City–Washington/Dulles–New York/JFK och retur | 9 |
| PA467/468 | Miami–Tampa–Mexico City samt Mexico City–Tampa | 6 |
| **Totalt** | | **65** |

## Tidtabell och kontroll

Källan är [Pan Am Schedules, 11 februari–26 april 1986](https://digitalcollections.library.miami.edu/digital/collection/asm0341/id/37370/), University of Miami Libraries, Pan American World Airways Records, ASM0341. Samma utgåva finns sedan tidigare i källregistret; ingen extra källpost har skapats.

Tryckta sidor: **23, 28, 59, 60, 62, 63, 64, 71, 72, 81, 84, 85, 95, 100, 107, 112 och 113**. Avgångstabellernas tider, flygnummer, veckodagar och fotnoter har lästs från sidbilderna. OCR användes för att hitta rätt uppslag, men flera OCR-flygnummer var fel och har kontrollerats visuellt. Sidan 64 är en bred utvikningsbild.

Varje rad i `panam_batch18.tsv` anger tryckt sida och arkivets bild-ID. `batch_18.json` innehåller originalbildlänkar, SHA-256 och storlek för den granskade bildrepresentationen (2 000 pixlar bred). Skanningarna ingår inte i ZIP-filen.

Importen avgränsas till **26 februari–1 mars 1986**. För PA217 gäller importen till och med 28 februari, enligt raden som undantar lördag. Den utgåvan innehåller många senare datumändringar som inte gäller vårt fönster.

## Tidszoner, delsträckor och avgränsningar

- Alla tider är lokala. Rio har **UTC−2** i detta fönster enligt historisk sommartid. Sidrubrikens allmänna GMT−3 har inte använts som ett fast datumoberoende offset. Flygtiderna stämmer med ruttöversiktens angivna varaktighet. Buenos Aires och Santiago har UTC−3; Caracas och Port of Spain UTC−4; Mexico City UTC−6.
- PA201 Rio–Buenos Aires, PA440 Miami–Los Angeles och PA453 Buenos Aires–Santiago kopplas till föregående lokala servicedag för respektive långresa. Markuppehåll är separata från flygtiden.
- Fönstret använder intervallöverlapp. Exempelvis ingår PA202 Buenos Aires–Rio och PA441 Los Angeles–Miami redan vid fönstrets början. PA454:s lördagsavgång från Santiago ligger efter fönstrets slut och räknas inte in.
- **Avgångstabellerna styr exakta tider.** PA440 Rio–Miami anger 23:59–05:15 nästa dag, vilket blir en minut längre än ruttöversiktens avrundade 8:15. PA456/457:s fyra delsträckor har konsekvent 15 minuter längre varaktighet i avgångstabellerna än i den allmänna ruttöversikten. Dessa avvikelser är dokumenterade i valideringsfilen; inga tider har justerats för att passa översikten.
- Washingtons **N** är National/DCA och **D** är Dulles/IAD. PA499 startar vid National; PA456/457 använder Dulles. New Yorks **K** är JFK.
- PA211/212 och andra alternativa nummer i genomgående anslutningsförslag har inte räknats som extra fysiska avgångar på redan tidsatta sträckor. São Paulo–Rio-rader som uttryckligen opereras av Transbrasil har inte lagts in som Pan Am-opererade flyg. PA453:s genomgående JFK-resa används inte för att skapa en extra JFK–Miami-avgång ovanpå avgångstabellens fysiska rad.
- PA468:s vidareförbindelse från Tampa till Miami har inte härletts ur en genomgående ankomsttid. Endast den uttryckliga nonstopraden Mexico City–Tampa ingår.
- Montevideos måndags-/torsdagsdelsträckor i detta urval överlappar inte fönstret och ger därför inga nya rörelser.

## Historiska flygplatser

Sex flygplatser tillkommer: **Ezeiza (EZE), Galeão (GIG), Maiquetía (CCS), La Chinita (MAR), Piarco (POS) och Mexico City (MEX)**. Namnen undviker senare tillägg som Tom Jobim och Benito Juárez.

La Chinita används för Maracaibo, inte det äldre Grano de Oro. [Flygplatsoperatören BAER](https://baer.gob.ve/aeropuerto-internacional-la-chinita/) anger öppnandet 1969. Galeãos historik stöds av [Brasiliens nationalarkiv](https://www.flickr.com/photos/arquivonacionalbrasil/52488276722/), Ezeiza av [ACUMAR:s arkivpost](https://centrodocumental.acumar.gob.ar/items/show/8854), Maiquetía av [INAC:s historik](https://inacvenezuela.wordpress.com/2016/02/05/historia-de-la-aviacion-en-venezuela/) och Mexico City av [AICM:s historik](https://www.aicm.com.mx/aicm-cumple-96-anos-de-operaciones/05-11-2024). Piarcos identitet och plats har jämförts med [flygplatsmyndighetens sida](https://tntairports.com/piarco-international-airport/) och dess publicerade historikuppgifter i den länkade referensposten.

Koordinatposter, tidszoner och granskningslänkar finns i `airport_locations_batch18.json`. Koordinaterna anger ungefärlig flygfältsplats; historiska gate- eller flygplanspositioner har inte rekonstruerats.

## Norden

Norden prioriterades i källsökningen. Pan Ams vinterutgåva har ett uppslag som annonserar en europeisk utökning **27 april 1986**, med Helsingfors och Oslo bland de nya orterna: [bild 37231](https://digitalcollections.library.miami.edu/digital/collection/asm0341/id/37231/) och [bild 37232](https://digitalcollections.library.miami.edu/digital/collection/asm0341/id/37232/). Dessa linjer kan inte användas för februaris fönster. Sommarutgåvans nordiska anslutningsförslag är inte importerade.

Air Canadas omslagskatalog anger en USA-utgåva från 16 februari 1986, men de funna bilderna gav inga nya kompletta avgångsrader. Sökningar efter El Als vintertabell och nordiska kommunikationstidtabeller gav heller inga ytterligare importerbara nordiska flyg i denna omgång. Tidigare SAS-/Finnair-/Air UK-spår kvarstår i forskningskön.

**44 tidigare rörelser med nordisk ändpunkt kvarstår. Inga nya faktiska överflygningar har belagts.** Kartans storcirklar är illustrationer av tidtabellsresor, inte dokumenterade färdvägar.

## Gränssnitt, installation och validering

Flygtrafiken ligger fortsatt i den egna pausmenykategorin **FLYGTRAFIK KRING MORDET**. V18:s flygplanssilhuett, riktningsberäkning, färger, markeringsordning och klickyta följer med. **V19 ändrar inga kodfiler eller börsdata.**

Paketet är kumulativt: lägg `Plugins/` och `Tools/` i projektroten. Vid uppgradering från en version före v18 behöver dess C++-ändringar byggas i Unreal. Se `LAS_MIG_v19.txt`.

20 Python-tester passerade, och genererad flyg-JSON, landindex och börsdata stämmer. Kontrollerna bekräftar oförändrade äldre rörelser, 22 oförändrade kod-/börsfiler, korrekta datumkopplingar, rimliga markuppehåll samt inga nya dubbletter eller samtidiga överlapp med samma flygnummer. Resultat och redovisade tidsavvikelser finns i `validation_batch18.json`.

**Unreal-kompilering och spelkörning har inte utförts i denna miljö.** Alla nya flyg har status `scheduled`; tidtabellen bevisar inte faktiskt genomförande.
