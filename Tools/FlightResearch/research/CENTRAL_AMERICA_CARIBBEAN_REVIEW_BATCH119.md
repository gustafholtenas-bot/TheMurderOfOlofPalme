# Centralamerika och Karibien – v122 / batch119

Granskat 1 oktober 2026. **51 nya planerade Pan Am-rörelser**, totalt **8 488 rörelser**. 25 nya scheman på 24 nya riktade flygplatspar. Alla **8 437 äldre rörelseobjekt och 4 945 äldre scheman är oförändrade**. Kumulativ datauppdatering från v121.

## Områden och källa

Nu tillkommer St. Thomas, St. Croix, St. Maarten, Saint Kitts, Saint Lucia, Barbados, Santo Domingo och Guatemala City, med förbindelser till Miami, New York och Port of Spain. **St. Thomas får åtta rörelser**: två från JFK, fyra till St. Croix och två från St. Croix. Urvalet är fortfarande partiellt.

Källa: [Pan Am, 11 februari–26 april 1986, University of Miami](https://digitalcollections.library.miami.edu/digital/collection/asm0341/id/37370/). De egna **Nonstop**-raderna på tryckta s.9, 44, 62–64, 71–72, 81, 88–92 och 96 har lästs i originalbilder. Bildadresser och SHA256 finns i `caribbean_source_evidence_batch119.json`. OCR används för att hitta sidor, inte för att ensam godkänna klockslag. Originalskanningarna återdistribueras inte.

## Importerade scheman

Alla tider nedan är lokala. Dagar: 1=måndag … 7=söndag. Scheman är avgränsade till relevanta datum 27 februari–2 mars och filtreras sedan mot det exakta spelfönstret. Dagurvalet är ingen fullständig veckotranskription.

| Flyg | Fysiskt ben | Lokal tid | Granskade dagar | Tryckt sida |
|---|---|---|---|---:|
| PA430 | STX → MIA | 10:25–12:10 | 1234567 | 88 |
| PA221 | STX → JFK | 16:15–18:59 | 1234567 | 88 |
| PA429 | STX → STT | 21:30–21:55 | 1234567 | 88 |
| PA430 | STT → STX | 09:30–09:55 | 1234567 | 92 |
| PA221 | STT → STX | 15:10–15:35 | 1234567 | 92 |
| PA426 | UVF → SXM | 08:10–09:05 | 1357 | 90 |
| PA426 | SKB → SXM | 08:40–09:05 | 2467 | 89 |
| PA426 | SXM → MIA | 09:35–11:25 | 1234567 | 91 |
| PA225 | SXM → JFK | 16:00–19:05 | 1234567 | 91 |
| PA425 | SXM → SKB | 18:10–18:35 | 56 | 91 |
| PA425 | SXM → UVF | 18:10–19:05 | 2467 | 91 |
| PA429 | MIA → STX | 17:30–21:05 | 1234567 | 63 |
| PA425 | MIA → SXM | 14:00–17:45 | 1234567 | 63 |
| PA405 | MIA → GUA | 18:05–19:40 | 1234567 | 63 |
| PA221 | JFK → STT | 09:45–14:20 | 1234567 | 72 |
| PA225 | JFK → SXM | 09:45–14:28 | 1234567 | 72 |
| PA219 | JFK → BGI | 09:45–15:10 | 1234567 | 71 |
| PA433 | MIA → SDQ | 18:00–21:00 | 1234567 | 64 |
| PA434 | SDQ → MIA | 10:30–11:30 | 1234567 | 96 |
| PA435 | MIA → BGI | 13:20–17:45 | 1234567 | 62 |
| PA436 | BGI → MIA | 08:45–11:30 | 1234567 | 9 |
| PA220 | BGI → JFK | 16:45–20:35 | 1234567 | 9 |
| PA435 | BGI → POS | 18:20–19:20 | 1234567 | 9 |
| PA436 | POS → BGI | 07:10–08:00 | 1234567 | 81 |
| PA404 | GUA → MIA | 08:05–11:25 | 1234567 | 44 |

Två scheman går STT–STX, därför är antalet scheman 25 men antalet riktade sträckor 24. En tidtabellsrad kan ge flera avgångar under spelfönstret. Flygnummer och tidtabell identifierar inte ett enskilt flygplan eller bekräftar att flygningen genomfördes.

## Datumvillkor och kontroll av mellanlandningar

- **PA425 SXM–SKB**: fredag används enligt MoWeFr; lördag enligt den nya MoWeFrSa-varianten från 1 mars.
- **PA425 SXM–UVF**: TuThSaSu upphör efter 27 februari. Ersättningen TuThSu börjar 2 mars. Bara torsdagsavgången överlappar här; ingen gammal lördagsavgång skapas.
- **PA426 SKB–SXM**: den nya TuThSaSu-varianten från 1 mars ger lördagsavgången. **UVF–SXM** använder fredagen i den gamla MoWeFrSu-varianten till 28 februari.
- **Guatemala**: ändringarna den 30 mars används inte. Rätt klockslag är 18:05–19:40 från Miami och 08:05–11:25 tillbaka.
- **Miami–Barbados**: förstorad originalrad bekräftar **13:20–17:45**, vilket även stämmer med avgången i den genomgående Port of Spain-raden.
- Resor STT–MIA, STT–JFK, JFK–STX och MIA–STT är genomgående resor via en annan ö. De ingår som egna tidsatta delsträckor där underlaget finns, utan extra obrutna direktflyg. Samma gäller resor via Miami och Barbados.

18 intilliggande par av delsträckor med samma flygnummer har kontrollerade markuppehåll: 25, 30, 35, 40, 45, 50 eller 92 minuter. Inga tidsöverlapp mellan ben med samma berörda Pan Am-flygnummer har påträffats. Dessa kontroller rekonstruerar inte individflygplans rotationer.

## Historiska flygplatser

| Kod | Namn i denna uppdatering | Forskningsområde |
|---|---|---|
| STT | St. Thomas – Cyril E. King | vi |
| STX | St. Croix – Alexander Hamilton (1986) | vi |
| SXM | St. Maarten – Princess Juliana | an |
| UVF | Saint Lucia – Hewanorra | lc |
| SKB | St. Kitts – Golden Rock (1986) | kn |
| SDQ | Santo Domingo – Las Américas | do |
| GUA | Guatemala City – La Aurora | gt |
| BGI | Barbados – Grantley Adams | bb |

St. Croix behåller **Alexander Hamilton**, och St. Kitts **Golden Rock**. St. Maarten placeras i **Nederländska Antillerna enligt 1986**. Saint Lucia avser uttryckligen Hewanorra. Platskontroller, myndighets-/flygplatskällor och koordinatkällor finns i `airport_location_review_batch119.json`. Koordinaterna är ungefärliga flygfältsmarkörer; moderna terminaler, bantrösklar eller ombyggnader rekonstrueras inte.

## Nicaragua och återstående arbete

**Nicaragua har fortfarande noll importerade rörelser.** Det betyder en datalucka, inte att trafik saknades. Inga läsbara, datumgiltiga klockrader har kunnat godkännas i denna omgång.

Kontrollerade spår redovisas med länkar och beslut i `caribbean_withheld_batch119.json`:

- Aeronica-posten från maj 1990 och annonsledtråden i Cuadernos del Tercer Mundo april–maj 1985 ligger utanför rätt period.
- Februariutgåvan 1986 av samma tidskrift har sökts i extraherad text och kontrollerats visuellt på inledande/avslutande sidor. Ingen tillämplig Aeronica-tabell verifierades; hela tidskriften har inte granskats visuellt sida för sida.
- AirTimes TACA-galleri gav inga användbara vinterblad för 1985/86. En händelserapport från juni 1986 används inte som februarischema.
- [Den arkiverade ordern från 1985](https://www.reaganlibrary.gov/archives/speech/executive-order-12513-prohibiting-trade-and-certain-other-transactions-involving) visar varför äldre Miami-annonser inte bör förlängas automatiskt till 1986. Den ger inga flygtider.

Fortsatt prioritet är Nicaragua, fler operatörer kring St. Thomas och övriga regionen, inklusive regional-, frakt- och chartertrafik. Militär-, stats-, privat- och specialtrafik kräver daterade rörelsehandlingar. **Inga sådana nya flyg importeras här**, och sökningen efter dem är inte färdig. Allmän historik eller en ruttkarta räcker inte för en exakt avgång. Kön finns i `central_america_caribbean_queue_batch119.json`.

## Regional täckning

Unika rörelser med minst en ändpunkt i det valda området ökar **33 → 84**. Riktade par ökar **12 → 36** och registrerade regionala flygplatser **3 → 11**. Sex ben går mellan St. Thomas och St. Croix. Täckningen för samtliga områden är fortsatt partiell.

| Område med importerad trafik, samt Nicaragua | Före v121 | Efter v122 |
|---|---:|---:|
| Nicaragua | 0 | 0 |
| Guatemala | 0 | 4 |
| Amerikanska Jungfruöarna / St. Thomas, St. Croix | 0 | 15 |
| Puerto Rico | 10 | 10 |
| Dominikanska republiken | 0 | 4 |
| Bahamas | 19 | 19 |
| Saint Kitts och Nevis | 0 | 3 |
| Saint Lucia | 0 | 2 |
| Barbados | 0 | 14 |
| Trinidad och Tobago | 4 | 9 |
| Nederländska Antillerna (1986) | 0 | 14 |

Land-/områdesraderna överlappar och får inte summeras. Alla 29 forskningsområden, även övriga nollrader, finns i `central_america_caribbean_coverage_batch119.json`. Ändpunkter identifierar inte vilka länder ett flyg faktiskt överflög. Det finns ingen verifierad totalsiffra för återstående flyg eller bolag.

## Verifiering och installation

Fönstret är **27 februari 1986 22:21:30 UTC – 1 mars 22:21:30 UTC**. Samtliga 51 nya UTC-intervall är kontrollerade mot separat klocktranskription och fasta UTC-förskjutningar: New York/Miami −5, Guatemala −6 och de importerade karibiska flygplatserna −4.

PA425 SXM–UVF torsdag börjar 22:10 UTC och överlappar starten. Torsdagens PA221 STX–JFK, PA225 SXM–JFK och PA220 BGI–JFK börjar också före fönstret men landar inom det. **PA435 BGI–POS lördag börjar 22:20 UTC, bara 90 sekunder före slutet**, och räknas med. Hela flygintervall bevaras; de kapas inte vid fönstergränserna.

**20 befintliga Python-tester och tre generatorkontroller passerar.** Totalt: **8 488 rörelser** (8 487 tidtabellslagda och en tidigare dokumenterat genomförd), **4 970 katalogscheman / 4 949 granskade**, **354 flygplatser/platser**, **105 länder/territorier**, **1 641 riktade par**. 102 registrerade operatörer, 59 med rörelser och 43 utan; 87 källposter och 29 bidragande tidtabellsutgåvor. Pan Am har nu 379 rörelser.

Stäng Unreal Editor. Slå samman ZIP-filens `Plugins/` och `Tools/` med projektroten bredvid `.uproject`, med bibehållen mappstruktur. Detta är **endast data**; `PaintAirplane`-rättelsen **f23b73a** måste redan vara kompilerad. C++-bygge, Unreal Editor och paketerat spel har inte körts här. Äldre data, kod, rättelser och hållna konflikter bevaras.
