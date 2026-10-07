# Internationella tillägg och Sverigeuppföljning – INT02

Kumulativ fortsättning på v141_INT01. **11 nya planerade flygrörelser från 11 granskade schemarader**, totalt **11 380 rörelser**. En schemarad ligger helt före tidsfönstret; en annan ger två rörelser. Air Canada får sina första två rörelser i urvalet. Antalet bolag med rörelser blir 69; registrerade bolag är fortsatt 111.

## Urval och bevarande

Internationellt plus svenskt inrikes omfattar nu **2 142 rörelser**, jämfört med 2 131 i INT01. Land-/territorieindelningen följer befintlig katalog. 9 238 äldre utländska inrikesrörelser ligger utanför detta urval men bevaras i det kumulativa paketet. Inga nya sådana rörelser tillförs.

Sverige är oförändrat: **87 ändpunktsrörelser = 41 internationella ankomster + 40 internationella avgångar + 6 inrikes**. Det är ett partiellt källbelagt urval, inte fullständig faktisk trafik eller en uppskattning av antalet återstående flyg.

Alla 11 369 äldre rörelseobjekt och 6 750 äldre scheman är oförändrade. Befintliga flygplatser, länder och övriga flygbolag är oförändrade. Air Canadas forskningsstatus ändras från ej sökt till partiell och får källhänvisning. Inga nya flygplatser eller koordinater införs.

## Nya rörelser

| Från | Till | Nya rörelser |
|---|---|---:|
| ANC | NRT | 2 |
| CDG | ANC | 1 |
| CDG | YYZ | 1 |
| IAH | MEX | 1 |
| KHI | CDG | 2 |
| MEX | IAH | 1 |
| NRT | ANC | 2 |
| YYZ | CDG | 1 |

- Air Canada AC880/881: Toronto–Charles-de-Gaulle och retur, två rörelser.
- Air France AF274/273: Charles-de-Gaulle–Anchorage, Anchorage–Narita och Narita–Anchorage, fem rörelser.
- Air France AF067/068: Houston Intercontinental–Mexico City och retur, två rörelser.
- Air France AF175/189: Karachi–Charles-de-Gaulle, två rörelser.

## Källunderlag

[Air France nr25, 27 oktober 1985–29 mars 1986](https://www.airtimes.com/cgat/fr/airfrance/pdf/1a/af851027-1a.pdf), teckenförklaring på tryckt s.2–4. Accepterade rader på tryckt s.14,36,37,51,61,77,90,91. Flygplatserna identifieras av samtida namn och Paris terminalkoder. Pil i VIA-kolumnen betyder nonstop. En tom kolumn används inte som belägg för nonstop.

Samtliga accepterade rader avlästes visuellt inklusive dagar, lokal tid, eventuell nästa-dagsankomst och giltighetsgräns. AF068:s slutdatum 27 mars ligger efter importperioden. De separata AC880/881-rader som börjar 5 respektive 6 mars importeras inte. Flygplansbeteckningar bevaras som källfakta; spelets generiska modell ändras inte. Originalskanningar distribueras inte. `source_rows_int02.json` innehåller faktatranskription och `new_movements_int02.json` alla nytillkomna daterade objekt.

## Datum, tidszoner och delsträckor

Fönstret är 27 februari 22:21:30–1 mars 22:21:30 UTC. Hela flygintervallet behålls när det överlappar fönstret. En oberoende utvidgning med fasta vinteroffsetar och trafikdagar matchar alla 11 nya objekt: Paris +1, Toronto −5, Anchorage −9, Narita +9, Houston/Mexico City −6 och Karachi +5.

AF274 torsdag Charles-de-Gaulle–Anchorage slutar 27 februari 20:20 UTC och ger därför ingen ny rörelse. Den separat tidsatta fortsättningen från Anchorage 21:35 UTC samma dag är däremot i luften vid fönstrets början och räknas. Fredagens CDG–ANC–NRT har ett kontrollerat uppehåll på 75 minuter. Båda fysisk delsträckorna använder samma resa och separata leg_index; animeringen har inget flyg under markuppehållet. Källans begränsning till genomresande AF-passagerare bevaras. Narita–Anchorage har samma lokala datum vid båda ändpunkterna; tidsskillnaden gör flygtiden 6 timmar 35 minuter. En fortsatt Anchorage–Paris-sträcka konstrueras inte utan dess tider.

Houston–Mexico City avgår torsdag 23:00 UTC och ingår; lördagens avgång 23:00 UTC ligger efter fönstret. Returen torsdag 20:50 lokal tid är fredag 02:50 UTC. Karachi–Paris AF175 fredag 01:10 lokal tid börjar torsdag 20:10 UTC och överlappar periodstarten. Inga tider har avrundats eller fyllts i genom antagande.

## Sverige och kvarstående källluckor

Sverigeprioriteten ligger kvar. Köpenhamn–Arlanda på s.26 jämfördes med befintliga scheman. De tydliga tillämpliga raderna är redan importerade. SK594/410 vid samma tider som SK674 förblir öppna frågor. Den extra SK428-raden 22:10–23:20 har en ofylld dagkolumn och samma tider som befintlig SK420; den förs också till fortsatt granskning utan ny rörelse. En tom dagkolumn fylls inte automatiskt från raden ovanför.

Finnair AY873/874 på s.36/67 saknar nonstop-pil och importeras inte som HEL–CDG-direktben. Deras mellanlandningar behöver separat källunderlag. Den tidigare hållningen i NORDIC_OVERFLIGHT_RESEARCH.md kvarstår. SAS/Linjeflyg/Swedair behöver fortfarande tillämpliga originaltabeller eller ändringsblad med tydliga fysiska delsträckor.

Montréal/Mirabel-tabellen anger tider för transporten med mobile lounge. Dessa behandlas inte som belagda flygplansankomster eller avgångar. Mirabel mappas inte till Dorval/YUL. AC870/871 och AF032/033 Paris–Toronto saknar nonstop-pil; deras genomresor importeras inte som direktflyg. Karachi–Peking saknar också nonstop-pil; inget direktben skapas. Tidigare Teesside-, EI456/EI457- och AF035-frågor kvarstår.

## Kontroll och installation

Alla tidigare rörelser och scheman jämförs som hela objekt. Nya objekt kontrolleras för id-dubbletter, fysisk dubblett mot den gamla databasen, internationella ändpunkter och oberoende UTC-utvidgning. Se `validation_int02.json`, `automated_checks_int02.json` och `package_preservation_int02.json`.

Paketet är en datauppdatering för befintlig flygmeny med kompilerad PaintAirplane-rättelse f23b73a. ZIP-mapparna Plugins/ och Tools/ hör till projektroten bredvid .uproject. Unreal eller spelet har inte körts här. Planerad trafik är inte bekräftad faktisk drift; individflygplan, senare ändringar och verkliga flygbanor är inte fastställda.
