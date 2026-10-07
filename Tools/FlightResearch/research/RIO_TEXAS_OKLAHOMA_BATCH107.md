# Rio Airways – v110 / batch107

Granskad 1 oktober 2026. Kumulativ datauppdatering från v109.

170 nya planerade rörelser med det nytillagda bolaget Rio Airways. 92 nya granskade nonstop-scheman på 26 nya riktade sträckor; 88 scheman ger rörelser i fönstret. Nio regionala DFW-förbindelser med returer samt separat tidsatta ben mellan Abilene/San Angelo, College Station/Waco, Killeen/Waco och Lawton/Wichita Falls. Totalt **7 435 rörelser**, varav 7 434 tidtabellslagda och en dokumenterat genomförd. Alla 7 265 äldre rörelseobjekt och 4 272 äldre scheman är oförändrade. Nio flygplatser tillkommer, inklusive historiska Killeen Municipal/ILE. Separat UTC-kontroll stämmer med alla nya rörelser; 25 markuppehåll mellan källbelagda ben är kontrollerade. Se `research/RIO_TEXAS_OKLAHOMA_BATCH107.md`.

## Sträckor

Alla förbindelser nedan har båda riktningarna. Antal och veckodagar skiljer mellan riktningarna.

| Förbindelse | Nya scheman | Nya rörelser |
|---|---:|---:|
| Dallas/Fort Worth–Abilene | 9 | 14 |
| Dallas/Fort Worth–Bryan/College Station | 7 | 15 |
| Dallas/Fort Worth–Killeen Municipal | 5 | 8 |
| Dallas/Fort Worth–Lawton | 10 | 17 |
| Dallas/Fort Worth–San Angelo | 7 | 10 |
| Dallas/Fort Worth–Temple | 4 | 7 |
| Dallas/Fort Worth–Texarkana | 10 | 21 |
| Dallas/Fort Worth–Waco | 11 | 22 |
| Dallas/Fort Worth–Wichita Falls | 7 | 15 |
| Abilene–San Angelo | 6 | 10 |
| Bryan/College Station–Waco | 2 | 4 |
| Killeen Municipal–Waco | 8 | 17 |
| Lawton–Wichita Falls | 6 | 10 |
| **Totalt** | **92** | **170** |

## Källa och operatör

[Delta Air Lines systemtidtabell från 1 februari 1986, Digital Library of Georgia](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201). Tryckta sidor **5, 35, 36, 63–67, 120, 127, 209, 232, 233, 243 och 250** har granskats visuellt. Sidan 4 definierar operatörer, frekvenser och symboler; sidorna 260–261 identifierar flygplatserna. PDF-hash och sidmappning finns i `rio_texas_oklahoma_source_evidence_batch107.json`.

Teckenförklaringen tilldelar **flygnummer 1875–1999 till Rio Airways**. De registreras därför under ett separat bolag, `rio-airways`, även om tidtabellen är Deltas och triangeln anger Delta Connection. Det angränsande intervallet 1750–1874 hör till Ransome Airlines. Den generella flygplansförklaringen listar BE1 för Rio, men används inte för att tilldela en flygplansindivid eller en viss typ till varje enskild rad.

Urvalet omfattar 92 explicit nonstop-rader i 26 riktningsrubriker. Ingen accepterad rad har en daterad fotnot. Genomgående rader med stopp och anslutningsförslag ingår inte i transkriptionsantalet eller som extra nonstop-flyg. Nätet är fortfarande partiellt. Utgåvans slutdatum och senare ändringsblad är inte verifierade. Underlaget beskriver planerad trafik, inte belagt genomförande. Original-PDF och sidbilder ingår inte i leveransen.

## Klockslag och mellanlandningar

- **DFW–Lawton 1918** går **15:40–16:25 dagligen**. Flyg 1908 och 1932 under samma rubrik har ett stopp och importeras inte som nonstop; deras separat tidsatta ben via Wichita Falls finns i omgången.
- **DFW–Lawton 1933** och **Lawton–Wichita Falls 1933** går bara lördag, liksom **DFW–San Angelo 1922**. Alla tre börjar efter fönstrets slut på lördagen och ger noll rörelser. **DFW–Killeen 1934** går bara söndag och ger också noll. De fyra raderna behålls som granskade scheman.
- **DFW–Abilene 1914** går dagligen, medan fortsättningen **Abilene–San Angelo 1914** bara går lördag. Egna veckodagar behålls per ben. SJT–ABI–DFW 1974 går också lördag och får sina separat tryckta tider.
- **DFW–Waco 1912/1917/1930/1944** följs av separat tidsatta Waco–Killeen-ben med tio minuter på marken. Returbenen ILE–ACT–DFW 1954/1955/1958/1960 har också tio minuter. Stjärnan på ACT–ILE 1944 är en prismarkering, inte ankomst nästa dygn.
- **DFW–Lawton–Wichita Falls–DFW 1900** och **DFW–Wichita Falls–Lawton–DFW 1908/1932** består av tre egna fysiska ben. Tiderna överlappar inte. Samma flygnummer och sammanhängande tider bevisar inte flygplansindividens identitet.
- **ACT–CLL 1929** importeras inte. DFW–CLL-tabellen har en genomgående rad, men den granskade lokala ACT–CLL-rubriken anger bara morgonflyg 1893. Ingen mellanliggande avgångstid härleds från genomgående tider. Det separat tryckta DFW–ACT 1929 tas med.

Alla tio berörda ändpunkter, inklusive DFW, använder **UTC−6** under importdatumen. Tom frekvens, kodad `D`, betyder dagligen; X undantar angivna dagar. 1=måndag, 7=söndag. Inga importerade ben passerar lokal midnatt.

Flygfönstret är **1986-02-27 22:21:30 UTC till 1986-03-01 22:21:30 UTC**. Hela intervall bevaras om avgång sker före slutet och ankomst efter början. Exempelvis DFW–SPS 1921 på lördagen avgår 16:10 lokalt och landar 16:50: den ingår trots att landningen ligger efter fönstrets slut 16:21:30 lokalt. Fördelningen är fyra scheman med noll, elva med en, 72 med två och fem med tre rörelser. Lokala avgångsdatum ger 31 rörelser den 27 februari, 85 den 28 februari och 54 den 1 mars.

## Nio nya historiska flygplatser

Tidtabellens flygplatsförteckning är primärkällan för 1986 års koder och namn. Moderna uppskattade FAA-koordinater, återgivna av AirNav, används som ungefärliga markörer vid samma flygfält. De återskapar inte dåtidens terminaler, uppställningsplatser, bantrösklar eller flygplanspositioner. Varje markör har individuella källänkar och en avgränsning i `airport_locations_batch107.json`.

| Kod | Markör för 1986 | Kontroll |
|---|---|---|
| ABI | Abilene Municipal | Tidtabell s.260; flygplatsens masterplan anger nuvarande flygfält från 1953, skilt från den tidigare platsen norrut. |
| CLL | Bryan/College Station–Easterwood | Namn och kod på s.260; FAA-posten anger driftplats från 1940. |
| ILE | Killeen Municipal | S.260 samt officiell historik; flytt av linjetrafiken till GRK och namnändring till Skylark Field hör till 2004. |
| LAW | Lawton Municipal | S.260; dagens flygfält har FAA-aktivering 1948 och operatören beskriver service sedan 1950. Olika milstolpar, inget exakt öppningsdatum tillskrivs. |
| SJT | San Angelo–Mathis Field | S.261 anger Mathis Field; samma namn och flygfält i FAA-posten. |
| TPL | Temple–Draughon-Miller | S.261 listar TPL som Municipal; officiell historik skiljer det efterkrigstida Draughon-Miller från den äldre kommunala platsen. |
| TXK | Texarkana Municipal | S.261; Arkansas Aeronautics identifierar TXK/Webb Field på Arkansas-sidan. |
| ACT | Waco–Madison Cooper | S.260 anger Madison Cooper; flygplatsens historik beskriver övergång från krigstida till kommunal användning. |
| SPS | Wichita Falls Municipal–Sheppard | S.261 och FAA:s gemensamma Sheppard/Municipal-identitet; detta är SPS, inte Kickapoo/CWC. |

Kompletterande primärkällor: [Killeens officiella historik](https://flygrk.com/our-history/), [Abilenes masterplan kapitel 2](https://abilene.airportplans.com/files/ABI_02_Inventory_Chapter.pdf), [Temple](https://flytemple.com/draughon-miller-central-texas-regional-airport/about-the-temple-airport/), [Waco](https://www.flywaco.com/Airport/History) och [Arkansas Aeronautics](https://fly.arkansas.gov/texarkana.html).

## Validering och bevarande

Separat manuell 24-timmarsinmatning av alla 92 rader kontrollerar originaltidernas tolkning. En separat beräkning med fast vintertidsförskjutning och kalenderdagar stämmer med samtliga **170 nya UTC-intervall**. Alla 25 markuppehåll mellan separat tidsatta ben stämmer; 24 har även matchande rörelser i fönstret, medan lördagskedjan 1933 ligger utanför. Inga tidsöverlapp förekommer mellan nya ben med samma Rio-flygnummer.

Alla **7 265 äldre rörelseobjekt**, **4 272 scheman**, **299 flygplatser**, **82 länder** och **96 operatörer** är oförändrade. Endast Delta-källposten får kompletterande granskningsnoteringar. Tidigare COMAIR-rättelser 1582/1580, den uteslutna RIC–ATL 1125-rättelsen och alla tre hållna källkonflikter bevaras. Alla äldre forskningsfiler och verktyg bevaras byte för byte, utom de aktuella genererade indexen och dokumenterade statusfilerna.

De 20 befintliga Python-testerna och kontrollerna av genererad flygdata, landindex och Europaindex passerar. Fullständiga utdata finns i `automated_checks_batch107.json`. Unreal-kompilering, Editor och paketerat spel har inte körts. Paketet förutsätter redan kompilerad PaintAirplane-rättelse från `f23b73a`.

## Revisionsunderlag och nästa urval

- `rio_texas_oklahoma_batch107.tsv` och `independent_clock_transcription_batch107.txt`: källtranskription och separat tidsinmatning.
- `rio_texas_oklahoma_source_evidence_batch107.json` och `airport_locations_batch107.json`: källor och tolkningsbeslut.
- `batch_107.json`, `validation_batch107.json`, `automated_checks_batch107.json`, `package_preservation_batch107.json`: antal, ID:n, beräkningar, tester och bevarande.
- `worldwide_coverage_batch107.json`: operatörsvis täckning.

Nästa större urval kan omfatta Delta/Ransome i nordöstra USA och ytterligare separat tidsatta mellanliggande ben. ACT–CLL 1929 kräver en egen källbelagd avgångstid. Världsinventeringen och representerade nät är ofullständiga. Europeiska landkön återupptas fortsatt vid Cypern.
