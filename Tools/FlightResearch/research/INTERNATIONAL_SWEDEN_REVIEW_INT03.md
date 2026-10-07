# Internationella tillägg och Sverigeuppföljning – INT03

Kumulativ fortsättning på v141_INT02. **55 nya planerade flygrörelser från 34 granskade schemarader**, totalt **11 435 rörelser**. Air France står för26 och Air Algérie för29 av de nya rörelserna. 53 går mellan Alger och Frankrike; två kompletterar Anchorage–Paris.

## Urval och bevarande

Internationellt plus svenskt inrikes omfattar nu **2 197 rörelser**, jämfört med2 142 i INT02. Land-/territorieindelningen följer befintlig katalog. 9 238 äldre utländska inrikesrörelser ligger utanför detta urval och bevaras i det kumulativa paketet. Inga nya utländska inrikesrörelser tillförs.

Sverige är oförändrat: **87 ändpunktsrörelser = 41 internationella ankomster + 40 internationella avgångar + 6 inrikes**. Detta är ett partiellt källbelagt urval. Det fastställer varken fullständig faktisk trafik eller antalet återstående flyg.

Alla11 380 äldre rörelseobjekt och6 761 äldre scheman bevaras oförändrade. Inga nya flygplatser eller koordinater införs. Air Algérie får en ytterligare källhänvisning; bolagets status är fortsatt partiell. Antalet bolag med rörelser är fortsatt69.

## Nya rörelser

| Från | Till | Nya rörelser |
|---|---|---:|
| ALG | LIL | 2 |
| ALG | LYS | 2 |
| ALG | MRS | 6 |
| ALG | NCE | 1 |
| ALG | ORY | 14 |
| ALG | TLS | 2 |
| ANC | CDG | 2 |
| LIL | ALG | 2 |
| LYS | ALG | 2 |
| MRS | ALG | 6 |
| NCE | ALG | 1 |
| ORY | ALG | 13 |
| TLS | ALG | 2 |

Parisförbindelserna med Alger använder **Orly**, inte Charles-de-Gaulle. Den nya sträckan från Anchorage använder Charles-de-Gaulle. Lyon anges som Satolas, Marseille som Marignane, Lille som Lesquin, Nice som Côte d'Azur, Toulouse som Blagnac och Alger som Houari Boumediene. Befintliga flygplatsmarkörer återanvänds.

## Källunderlag och avläsning

[Air France nr25,27 oktober1985–29 mars1986](https://www.airtimes.com/cgat/fr/airfrance/pdf/1a/af851027-1a.pdf), teckenförklaring på tryckt s.2–4. De nya raderna finns på tryckt s.12,13,40,43,47,56,61,91. Alla accepterade rader har en nonstop-pil i VIA-kolumnen och saknar särskild datumgräns. Lokaltider, veckodagar, terminalbokstav och nästa-dagsankomst har avlästs visuellt, vid behov i förstorade utsnitt.

AH1426 Alger–Orly går enligt raden måndag,onsdag,fredag,lördag,söndag. Returen AH1427 Orly–Alger går måndag,tisdag,torsdag,lördag,söndag. Därför ger riktningarna två respektive en rörelse under fredag/lördag. AF2356 Alger–Lyon har12:00–13:40 på vardagar och19:00–20:40 på lördag/söndag. Skillnaderna bevaras.

Alger–Bordeaux AF2340/2344 saknar nonstop-pil. Endast den separat tidsatta internationella delen Alger–Toulouse importeras där den är tillämplig. Det franska inrikesbenet tillförs inte. Söndagsrader till Nice/Strasbourg samt måndags-/torsdagsrader före fönstret ger inga tillägg. Marocko importeras inte från denna källa eftersom dess varning om ogiltiga Marockotider kvarstår. Genomresor fylls inte ut med antagna mellanlandningstider.

Originalskanningar distribueras inte. `source_rows_int03.json` innehåller transkriberade fakta och sidangivelser. `new_movements_int03.json` innehåller alla55 nya daterade objekt.

## Anchorage-returen och tidskontroll

Den tidigare luckan för AF273 Anchorage–Paris löses av tryckt s.13: **10:45–05:55 nästa dag**, dagar1,3,5,6, nonstop. Med Anchorage UTC−9 och Paris UTC+1 är flygtiden9 timmar10 minuter. Fredags- och lördagsavgången ingår. Lördagens ankomst den2 mars ligger efter periodslut, men hela flygintervallet bevaras eftersom avgången överlappar fönstret.

De nya benen kopplas som leg_index2 till de redan importerade Narita–Anchorage-benen i INT02. Befintliga objekt ändras inte. Båda resorna har **70 minuters markuppehåll**, från09:35 till10:45 lokal tid i Anchorage. Detta belägger tidtabellens fortsättning, inte ett visst individflygplan. Narita–Osaka och Anchorage–Osaka-genomresan tillförs inte.

Fönstret är27 februari22:21:30–1 mars22:21:30 UTC. En oberoende utvidgning med fasta vinteroffsetar, veckodagar och dygnsskifte matchar alla55 nya rörelser. Alger och samtliga berörda franska flygplatser anges som UTC+1; inga lokala klockor behöver flyttas mellan dem. Inga tider har avrundats eller uppskattats.

## Sverigeuppföljning och öppna frågor

Sverigeprioriteten ligger kvar. En förnyad sökning gav Finnair museipost SIM M016-27845, en hopvikbar vintertidtabell för23 december1985–29 mars1986, utöver den tidigare häftesposten M016-27847. Katalogposter visar att underlag finns men de faktiska tabellbilderna kunde inte öppnas; inga rörelser importeras från dem. SAS-gallerierna gav inga nya tillämpliga flygsidor.

Den tidigare nedladdade SAS City Portrait Moscow/Leningrad1986-PDF:en granskades visuellt på båda sidorna. Den innehåller omslag/festivaltext och en Leningradkarta, inga flygtider. Spåret avförs som tidtabellsunderlag i denna form. Det innebär inte att hela originalhäftet är inventerat.

AY873/874:s mellanlandningar, SK594/410/428, Mirabels mobile-lounge-tider, Teesside, EI456/EI457 och AF035:s ofyllda lördagsrad är fortsatt öppna frågor. SAS/Linjeflyg/Swedair behöver fortfarande tillämpliga originaltabeller eller ändringsblad. Se `sweden_research_leads_int03.json` och tidigare revisionsrapporter.

## Kontroll och installation

Alla tidigare rörelser och scheman jämförs som hela objekt. Nya objekt kontrolleras för id-dubbletter, fysisk dubblett mot äldre databas, internationella ändpunkter och oberoende UTC-utvidgning. AF273:s två nya anslutningar kontrolleras separat. Se `validation_int03.json`, `automated_checks_int03.json` och `package_preservation_int03.json`.

Paketet är en datauppdatering för befintlig flygmeny med kompilerad PaintAirplane-rättelse f23b73a. ZIP-mapparna Plugins/ och Tools/ hör till projektroten bredvid .uproject. Unreal eller spelet har inte körts här. Tidtabellerna visar planerad trafik; faktisk drift, individflygplan, senare ändringar och verkliga flygbanor är inte fastställda.
