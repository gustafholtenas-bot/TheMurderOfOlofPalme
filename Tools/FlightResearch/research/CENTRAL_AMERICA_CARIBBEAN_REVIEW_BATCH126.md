# Grenadinerna och Inter-Island Air Services – v129 / batch126

Granskat 2 oktober 2026. **53 nya planerade flygrörelser**, totalt **9 042 rörelser**. **12 nya riktade sträckor och tre nya flygplatser**: Carriacou, Union Island och Mustique. Alla **8 989 tidigare rörelseobjekt och 5 335 tidigare scheman är oförändrade**.

42 scheman har accepterats från Inter-Island Air Services (IAS). 26 ger rörelser i tidsfönstret; 16 gäller söndagar och ger inga. 20 riktade flygplatspar har granskats, varav 16 har nya rörelser. Fyra av de 16 aktiva paren fanns redan: GND–POS, GND–SVD, SLU–SVD och SVD–SLU.

## Nya rörelser

| Riktad sträcka | Rörelser |
|---|---:|
| BGI–MQS | 2 |
| CRU–GND | 8 |
| CRU–UNI | 2 |
| GND–CRU | 6 |
| GND–POS | 2 |
| GND–SVD | 3 |
| MQS–UNI | 2 |
| SLU–SVD | 4 |
| SLU–UVF | 2 |
| SVD–SLU | 4 |
| SVD–UNI | 4 |
| SVD–UVF | 2 |
| UNI–CRU | 6 |
| UNI–SVD | 2 |
| UVF–SLU | 2 |
| UVF–SVD | 2 |

Varje rörelse är ett daterat fysiskt flygben. Flera avgångar kan tillhöra samma riktade sträcka. Tidsatta mellanlandningar delas i separata ben; en genomgående direktsträcka läggs inte till över dem.

## Källa och attribution

Primärkälla: [LIAT W.T.No.40, skanning 7](https://www.timetableimages.com/ttimages/li/li8512/li8512.pdf#page=7), med rubriken **Inter-Island Air Services**. Sidan saknar tryckt sidnummer. Kolumnerna betecknas N1–N7 för northbound och S1–S8 för southbound, räknat från vänster inom respektive tabell.

Tryckt giltighet är **13 december**. **1985 är arkivets datering**, även antecknat för hand i utgåvan. Inget slutdatum för hela utgåvan är tryckt och täckningen av ändringsblad är inte fullständig. Importen återger publicerade planer som bedömts tillämpliga på projektets fönster; den bevisar inte genomförande.

Flygnumren har prefixet **LI**, vilket bevaras i källnoterna. Operatörsposten följer IAS-sidans uttryckliga rubrik. Detta fastställer inte vilken juridisk innehavare av trafiktillstånd eller vilket individflygplan som utförde en viss avgång. Samtliga nya rörelser har status `scheduled`.

[BNAPS News, juli 2020, tryckt sida 20](https://wightmagic.com/BNAPS_Images/BNAPS_News-July_2020.pdf) beskriver i sin flygplanshistorik en försäljning till Inter Island Air Services i Antigua, före ett senare ägarbyte i april 1986. Det används enbart som stöd för landanknytningen Antigua i registret. **Flygplanet i artikeln knyts inte till någon av dessa tidtabellsrörelser.** Juridisk operatör och ägarkronologi är fortsatt ofullständigt fastställda.

## Rättelse: LI117 och kvarstående konflikt LI126

Tidigare forskningsrapporter läste LI117 i huvudtabellen som St. Vincent–Grenada. **Det var vår felläsning av radriktningen.** På skanning 5 står Barbados avgång **15:40**, Mustique ankomst **16:35**, dagligen utom söndag, DHT. Detta stämmer med IAS-tabellens första ben på skanning 7. Ingen sådan felaktig SVD–GND-rörelse hade importerats, så inga äldre rörelseobjekt behöver rättas.

Följande LI117-ben accepteras från IAS-sidan:

| Ben | Lokal tid | Beslut |
|---|---|---|
| Barbados–Mustique | 15:40–16:35 | Accepterat; bekräftat på två sidor i samma utgåva |
| Mustique–Union Island | 16:45–17:00 | Accepterat |
| Union Island–Carriacou | 17:10–17:20 | Accepterat |
| Carriacou–Grenada | 17:30–17:15 | Hålls utanför; ankomst före avgång |

De första tre sammanhängande benen ger sex rörelser. **LI117 är inte rekonstruerad som en fullständig reskedja.** Den felaktiga sista ankomsten ändras inte till ett gissat klockslag eller till nästa dag. Barbados–Mustique importeras bara en gång under IAS; huvudtabellens upprepning skapar ingen extra LIAT-rörelse. Rättelsen dokumenteras i `errata_batch126.json`. Äldre rapporter bevaras som historiska versioner; deras tidigare LI117-tolkning ersätts av denna rättelse.

**LI126 hålls helt utanför.** Huvudtabellen på skanning 3 har Mustique **14:20** → Fort-de-France **15:15**, BNI, dagligen utom söndag. IAS-sidan har i stället DHT och kedjan Trinidad–Grenada–Carriacou–Union Island–Mustique–St. Vincent–Barbados, **11:30–15:15**. Varken en rättad sträcka, ett tillståndsinnehav eller en utrustning gissas fram.

## Accepterade scheman

Alla klockslag är lokala. D=dagligen; X7=utom söndag; 7=söndag. BNI och DHT är källans utrustningskoder, inte identifierade flygplansindivider. IAS-sidan skriver flygplats-/önamn, inte IATA-koder; koderna nedan är normaliserade mot flygplatsregistret. St. Lucia Vigie är **SLU**, medan källans felskrivna “Henanorra” avser **Hewanorra/UVF**.

| Flyg | Fysiskt ben | Lokal tid | Dagar | Utrustning | Kolumn | Rörelser |
|---|---|---|---|---|---|---:|
| LI112 | GND → CRU | 06:30–06:50 | D | BNI | N1 | 2 |
| LI110 | SVD → SLU | 07:00–07:30 | D | DHT | N2 | 2 |
| LI114 | GND → CRU | 07:35–07:55 | D | BNI | N3 | 2 |
| LI114 | CRU → UNI | 08:05–08:15 | D | BNI | N3 | 2 |
| LI114 | UNI → SVD | 08:25–08:45 | D | BNI | N3 | 2 |
| LI114 | SVD → SLU | 08:55–09:25 | D | BNI | N3 | 2 |
| LI120 | GND → UNI | 11:00–11:25 | 7 | DHT | N4 | 0 |
| LI120 | UNI → MQS | 11:35–11:50 | 7 | DHT | N4 | 0 |
| LI120 | MQS → SVD | 12:00–12:20 | 7 | DHT | N4 | 0 |
| LI120 | SVD → UVF | 13:00–13:30 | 7 | DHT | N4 | 0 |
| LI120 | UVF → SLU | 13:40–13:55 | 7 | DHT | N4 | 0 |
| LI124 | GND → CRU | 16:40–17:00 | D | BNI | N6 | 2 |
| LI118 | GND → SVD | 18:20–18:55 | D | DHT | N7 | 3 |
| LI118 | SVD → UVF | 19:05–19:35 | D | DHT | N7 | 2 |
| LI118 | UVF → SLU | 20:15–20:30 | D | DHT | N7 | 2 |
| LI113 | CRU → GND | 07:00–07:20 | D | BNI | S1 | 2 |
| LI125 | SLU → UVF | 07:40–07:55 | X7 | DHT | S2 | 2 |
| LI125 | UVF → SVD | 08:10–08:40 | X7 | DHT | S2 | 2 |
| LI125 | SVD → UNI | 08:50–09:10 | X7 | DHT | S2 | 2 |
| LI125 | UNI → CRU | 09:20–09:30 | X7 | DHT | S2 | 2 |
| LI125 | CRU → GND | 09:40–10:00 | X7 | DHT | S2 | 2 |
| LI125 | GND → POS | 10:10–11:00 | X7 | DHT | S2 | 2 |
| LI125 | SLU → UVF | 07:40–07:55 | 7 | DHT | S3 | 0 |
| LI125 | UVF → SVD | 08:10–08:40 | 7 | DHT | S3 | 0 |
| LI125 | SVD → UNI | 08:50–09:10 | 7 | DHT | S3 | 0 |
| LI125 | UNI → CRU | 09:20–09:30 | 7 | DHT | S3 | 0 |
| LI125 | CRU → GND | 09:40–10:00 | 7 | DHT | S3 | 0 |
| LI115 | SLU → SVD | 11:00–11:30 | D | BNI | S4 | 2 |
| LI115 | SVD → UNI | 11:40–12:00 | D | BNI | S4 | 2 |
| LI115 | UNI → CRU | 12:10–12:20 | D | BNI | S4 | 2 |
| LI115 | CRU → GND | 12:30–12:50 | D | BNI | S4 | 2 |
| LI121 | SLU → UVF | 14:30–14:45 | 7 | DHT | S5 | 0 |
| LI121 | UVF → SVD | 15:00–15:30 | 7 | DHT | S5 | 0 |
| LI121 | SVD → MQS | 15:40–16:00 | 7 | DHT | S5 | 0 |
| LI121 | MQS → UNI | 16:10–16:25 | 7 | DHT | S5 | 0 |
| LI121 | UNI → CRU | 16:35–16:45 | 7 | DHT | S5 | 0 |
| LI121 | CRU → GND | 16:55–17:15 | 7 | DHT | S5 | 0 |
| LI117 | BGI → MQS | 15:40–16:35 | X7 | DHT | S6 | 2 |
| LI117 | MQS → UNI | 16:45–17:00 | X7 | DHT | S6 | 2 |
| LI117 | UNI → CRU | 17:10–17:20 | X7 | DHT | S6 | 2 |
| LI123 | CRU → GND | 17:10–17:30 | D | BNI | S7 | 2 |
| LI119 | SLU → SVD | 20:40–21:10 | D | DHT | S8 | 2 |

LI125:s söndagskolumn slutar i Grenada. Kolumnen för övriga dagar fortsätter till Trinidad. Varianterna är separata och ingen söndagsvariant skapar trafik på fredag/lördag. LI120 och LI121 gäller enbart söndag och ger också noll rörelser i fönstret.

## Tid, markuppehåll och dubbletter

Projektets fönster är **27 februari 1986 22:21:30 UTC – 1 mars 22:21:30 UTC**. Alla berörda flygplatser använder UTC−4 på måldatumen. En separat källtranskription med uttryckliga flygdatum och varaktigheter stämmer med samtliga **53 nya UTC-intervall**: fyra lokala avgångar den 27 februari, 26 den 28 februari och 23 den 1 mars.

**LI118 Grenada–St. Vincent 18:20–18:55** överlappar både torsdagens start och lördagens slut och skapar därför tre rörelser. Dess senare två ben och LI119:s kvällsflyg skapar rörelser torsdag/fredag; lördagens avgångar ligger efter fönstret. Intervallöverlapp, inte enbart avgångsdatum, styr urvalet.

**30 daterade markövergångar på 15 benpar** har kontrollerats mot skanning 7: 26 med tio minuter, två med femton minuter vid UVF i LI125 och två med fyrtio minuter vid UVF i LI118. Inga överlappande tider finns mellan accepterade ben med samma operatör/flygnummer. Flygnummer bevisar inte ett gemensamt individflygplan.

En extra kontroll av källutgåva, flygnummer, sträcka och UTC-tider över bolagsrubrikerna visar att inga nya IAS-rörelser dubblerar tidigare accepterade rörelser.

## Flygplatser och historisk geografi

| Kod | Benämning i databasen | Koordinatpost |
|---|---|---|
| CRU | Carriacou – Lauriston (1986) | TGPZ |
| UNI | Union Island – Clifton (1986) | TVSU |
| MQS | Mustique (1986) | TVSM |

Koordinaterna kommer från sparade [OurAirports-data](https://ourairports.com/data/) och används som **ungefärliga flygfältsmarkörer**, inte inmätta banor eller terminaler från 1986. Inga nya länder/territorier tillkommer och äldre flygplatsposter ändras inte.

För **Carriacou** ger [Bill Camerons förstahandsbilder och återgivna öppningsprogram](https://www.carriacou1968.com/opening-lauriston-airstrip/grand-opening-of-lauriston-airstrip/) belägg för Lauriston i mars 1968. Den geografiska benämningen Lauriston används utan ett senare hedersnamn.

**Union Islands exakta äldre banläge är inte rekonstruerat.** Den [moderna flygplatssidan](https://www.svg-airport.com/airports/union-island) anger öppnande 1994, men den samtida tidtabellen visar tidigare trafik. [Scott och Horrocks, UNEP/CEP Technical Report 27, 1993, avsnitt 3.1](https://www.ais.unwater.org/ais/aiscm/getprojectdoc.php?docid=546) beskriver att en befintlig bana på Union Island förlängdes genom ingrepp i revet. Denna passage var tillgänglig i sökindex; direkt nedladdning gav 502 vid slutkontrollen. Tillsammans stöder källorna slutsatsen att det fanns ett tidigare flygfält. Den moderna Clifton-positionen används med geografisk approximation; vi påstår inte att dagens förlängda bana fanns 1986.

För **Mustique** beskriver [Mustique Companys egen historik](https://www.mustique-island.com/history) anläggandet av Bamboo Airport under utvecklingen från 1968. LIAT:s huvudtabell på skanning 3 har felskrivningen **MSQ** vid Mustique, medan skanning 5 skriver **MQS**. Det uttryckliga önamnet styr; ingen flygning kopplas till Minsk.

## Regional och global täckning

Samma **31 forskningsområden** som i v128 används: **461 → 514 rörelser**, **138 → 150 riktade par**, **33 → 36 flygplatser** och **38 → 64 inrikesrörelser** inom samma land/territorium.

| Område med trafik, samt Nicaragua | v128 | v129 |
|---|---:|---:|
| Nicaragua | 0 | 0 |
| Guatemala | 4 | 4 |
| Amerikanska Jungfruöarna / St. Thomas, St. Croix | 31 | 31 |
| Brittiska Jungfruöarna | 8 | 8 |
| Puerto Rico | 26 | 26 |
| Jamaica | 4 | 4 |
| Haiti | 6 | 6 |
| Dominikanska republiken | 4 | 4 |
| Bahamas | 27 | 27 |
| Caymanöarna | 38 | 38 |
| Turks- och Caicosöarna | 4 | 4 |
| Antigua och Barbuda | 115 | 115 |
| Saint Kitts och Nevis | 70 | 70 |
| Dominica | 30 | 30 |
| Saint Lucia | 53 | 69 |
| Saint Vincent och Grenadinerna | 32 | 65 |
| Grenada | 27 | 54 |
| Barbados | 53 | 55 |
| Trinidad och Tobago | 27 | 29 |
| Guadeloupe och dåvarande franska karibiska områden | 48 | 48 |
| Martinique | 58 | 58 |
| Nederländska Antillerna (1986) | 55 | 55 |
| Anguilla | 2 | 2 |
| Montserrat | 41 | 41 |

Områdessummor överlappar och får inte adderas. Ändpunkter fastställer inte överflugna länder. **St. Thomas är oförändrat 14 rörelser, Tortola åtta. Nicaragua saknar verifierade klockrader.** Noll betyder en lucka i underlaget. Inga nya militär-, stats-, privat-, charter- eller fraktflyg har tillkommit. Mellanösternurvalet är oförändrat 72 rörelser.

Globalt finns **9 042 rörelser**, varav **9 041 planerade och en tidigare bekräftad**; **5 377 katalogscheman / 5 356 granskade**; **381 flygplatser/platser**, **119 länder/territorier** och **1 772 riktade par**. **109 registrerade bolags-/operatörsposter**, varav **66 med rörelser och 43 utan**. Registret är inte en verifierad inventering av självständiga juridiska flygbolag. Oförändrat 90 källposter och 32 bidragande tidtabellsutgåvor. Ingen global färdigprocent är fastställd.

## Fortsättning

Nästa granskning bör söka fler tillämpliga källor för Nicaragua och St. Thomas, Bahamasairs vintertabell, Air Jamaicas större nät och Cayman Express. LI126, LI117:s sista ben, Four Island Airs LI168-asterisk och juridiska operatörsfrågor ligger kvar. Ändringsblad för LIAT-utgåvan och daterade specialflyghandlingar behövs fortsatt. Se `central_america_caribbean_queue_batch126.json`.

## Kontroller och installation

**20 befintliga Python-tester och tre generatorkontroller passerar.** Samtliga äldre rörelser, scheman, bolag, länder och flygplatser är oförändrade. En tidigare källpost får utökad titel, sidlista och noter, med före/efter i `source_metadata_update_batch126.json`. Äldre kod, rapporter och rättelser bevaras. Nya original-PDF:er och källbilder återdistribueras inte.

Stäng Unreal Editor och slå samman ZIP-filens `Plugins/` och `Tools/` med projektroten bredvid `.uproject`. Paketet är kumulativt och innehåller data. **PaintAirplane-rättelsen f23b73a måste redan vara kompilerad.** C++-bygge, Unreal Editor och paketerat spel har inte körts här.
