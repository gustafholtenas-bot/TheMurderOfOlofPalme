# Centralamerika, Karibien och Florida – v124 / batch121

Granskat 1 oktober 2026. **132 nya planerade flygrörelser** med **Republic Airlines (1986)**. Totalt **8 667 rörelser**. 68 nya granskade scheman på 28 riktade flygplatspar, varav 19 par är nya i databasen. 65 scheman skapar rörelser inom spelfönstret; tre sena lördagsscheman ligger utanför. Alla **8 535 äldre rörelseobjekt och 5 013 äldre scheman är oförändrade**.

## Vad som tillkommer

| Urval | Nya rörelser |
|---|---:|
| Miami ↔ Grand Cayman | 8 |
| Memphis ↔ Cancún | 2 |
| USA: Florida-förbindelser och Orlando ↔ Tampa | 122 |
| Totalt | 132 |

Florida-urvalet omfattar Fort Lauderdale, Miami, Orlando, Sarasota/Bradenton och Tampa med separat tidsatta jetben till/från Detroit, Memphis och Minneapolis. Det innebär inte 132 nya karibiska flygningar. Caymanöarna får sin första importerade trafik, medan Cancún tillkommer som angränsande mexikansk destination. Republic registreras som ett nytt historiskt bolag; nätet är endast delvis inläst.

## Primärkälla och läsregler

[Republic Airlines Employee System Timetable, 14 februari–1 mars 1986](https://northwestairlineshistory.org/wp-content/uploads/2018/07/REP-employee-schedule-1986-02-14.pdf), bevarad av [Northwest Airlines History Center](https://northwestairlineshistory.org/timetables-republic/). Omslaget anger uttryckligen hela giltighetsperioden. PDF-filen har sju skanningar: omslag 1, tabelluppslag 2–6 samt stadsförteckning 7. Sidorna saknar tryckta nummer; hänvisningarna nedan är **skanning/kolumn från vänster**.

Originalbilderna har granskats visuellt, med förstoring av små klockslag. OCR används endast för att hitta rader. PDF-adress, kontrollsumma och bildhänvisningar finns i `caribbean_source_evidence_batch121.json`; originalskanningarna återdistribueras inte.

- **ST = antal mellanstopp.** Endast tom ST-kolumn accepteras som nonstop. Rader med 1 eller 2 stopp blir inte extra obrutna flyg.
- **FRQ:** tomt tolkas som dagligen, 6=lördag, 67=lördag/söndag, X6=alla dagar utom lördag. Nyckeln på skanning 6 anger X=except och 1=måndag … 7=söndag.
- **A/P = AM/PM.** Tiderna tolkas som lokala enligt tidtabellskonvention, med kontroll mot tidszoner. Någon uttrycklig all-times-local-text har inte återfunnits i denna utgåva; detta är en dokumenterad tolkningsbegränsning.
- **DC9, D9S, D95 och 72S** bevaras som tryckta utrustningskoder, utan antagande om registrering eller individflygplan.
- **EXP** är andra operatörer enligt den tryckta nyckeln: Express Airlines I för 1400–1699 och Simmons för 1700–1899. Inga EXP-rader importeras under Republic i denna omgång.

## Importerade scheman

Tiderna är lokala; +1 betyder nästa lokala kalenderdag. Importdatum är 27 februari–1 mars, begränsade av källans veckodagar och exakt intervallöverlapp. De tryckta D9S- och D95-koderna hålls isär.

| Flyg | Fysiskt ben | Lokal tid | FRQ | Skanning/kolumn | Utrustning |
|---|---|---|---|---|---|
| RC6 | CUN → MEM | 15:40–18:00 | 67 | 2/2 | DC9 |
| RC212 | DTW → FLL | 09:50–12:40 | Daglig | 2/4 | D95 |
| RC142 | DTW → FLL | 16:10–19:00 | Daglig | 2/4 | D9S |
| RC445 | DTW → MCO | 09:40–11:58 | Daglig | 3/1 | 72S |
| RC420 | DTW → MCO | 13:05–15:25 | Daglig | 3/1 | D95 |
| RC418 | DTW → MCO | 16:00–18:22 | Daglig | 3/1 | D95 |
| RC416 | DTW → MIA | 08:30–11:30 | Daglig | 3/1 | D95 |
| RC807 | DTW → MIA | 17:00–19:55 | Daglig | 3/1 | D9S |
| RC446 | DTW → SRQ | 09:30–12:05 | Daglig | 3/1 | D95 |
| RC139 | DTW → TPA | 09:35–12:05 | Daglig | 3/1 | D95 |
| RC279 | DTW → TPA | 19:35–22:05 | Daglig | 3/1 | D95 |
| RC23 | FLL → DTW | 09:30–12:30 | Daglig | 3/2 | D9S |
| RC375 | FLL → DTW | 16:40–19:40 | Daglig | 3/2 | D95 |
| RC233 | FLL → MEM | 10:20–11:25 | Daglig | 3/2 | D9S |
| RC455 | FLL → MEM | 13:40–14:45 | Daglig | 3/2 | D95 |
| RC465 | GCM → MIA | 07:40–08:55 | Daglig | 3/2 | D9S |
| RC457 | GCM → MIA | 15:05–16:20 | Daglig | 3/2 | D95 |
| RC546 | MCO → DTW | 10:00–12:30 | Daglig | 4/1 | D9S |
| RC442 | MCO → DTW | 12:40–15:05 | Daglig | 4/1 | 72S |
| RC423 | MCO → DTW | 16:00–18:30 | Daglig | 4/1 | D95 |
| RC421 | MCO → DTW | 18:55–21:25 | Daglig | 4/1 | D9S |
| RC453 | MCO → MEM | 10:20–11:20 | Daglig | 4/1 | D95 |
| RC383 | MCO → MEM | 13:45–14:45 | Daglig | 4/1 | D9S |
| RC385 | MCO → MEM | 17:45–18:45 | Daglig | 4/1 | D95 |
| RC459 | MCO → MEM | 20:55–21:55 | Daglig | 4/1 | D95 |
| RC439 | MCO → MSP | 16:20–18:35 | 6 | 4/1 | D95 |
| RC431 | MCO → MSP | 17:40–20:00 | Daglig | 4/1 | D9S |
| RC265 | MCO → TPA | 09:30–10:00 | Daglig | 4/1 | DC9 |
| RC430 | MCO → TPA | 13:00–13:30 | 6 | 4/1 | D95 |
| RC382 | MCO → TPA | 15:50–16:20 | Daglig | 4/1 | D95 |
| RC5 | MEM → CUN | 12:15–14:35 | 67 | 4/1 | DC9 |
| RC232 | MEM → FLL | 12:25–15:25 | Daglig | 4/1 | D9S |
| RC290 | MEM → FLL | 19:40–22:40 | Daglig | 4/1 | D9S |
| RC384 | MEM → MCO | 09:10–11:50 | Daglig | 4/2 | D9S |
| RC382 | MEM → MCO | 12:35–15:15 | Daglig | 4/2 | D95 |
| RC454 | MEM → MCO | 15:45–18:25 | Daglig | 4/2 | D95 |
| RC523 | MEM → MCO | 19:50–22:30 | Daglig | 4/2 | D95 |
| RC696 | MEM → MCO | 22:40–01:20 +1 | 6 | 4/2 | DC9 |
| RC712 | MEM → MCO | 22:40–01:20 +1 | X6 | 4/2 | DC9 |
| RC680 | MEM → MIA | 09:00–12:02 | Daglig | 4/2 | D95 |
| RC452 | MEM → MIA | 12:45–15:47 | Daglig | 4/2 | D95 |
| RC688 | MEM → MIA | 15:55–18:57 | Daglig | 4/2 | D9S |
| RC456 | MEM → MIA | 19:55–22:57 | Daglig | 4/2 | D95 |
| RC521 | MEM → SRQ | 19:55–22:45 | Daglig | 5/1 | DC9 |
| RC476 | MEM → TPA | 09:00–11:38 | Daglig | 5/1 | D9S |
| RC480 | MEM → TPA | 19:55–22:30 | 6 | 5/1 | D95 |
| RC462 | MEM → TPA | 19:55–22:30 | X6 | 5/1 | D9S |
| RC417 | MIA → DTW | 09:15–12:15 | Daglig | 5/1 | D95 |
| RC379 | MIA → DTW | 16:50–19:50 | Daglig | 5/1 | D95 |
| RC680 | MIA → GCM | 13:15–14:33 | Daglig | 5/1 | D95 |
| RC688 | MIA → GCM | 20:25–21:40 | Daglig | 5/1 | D9S |
| RC465 | MIA → MEM | 10:00–11:22 | Daglig | 5/1 | D9S |
| RC687 | MIA → MEM | 13:30–14:52 | Daglig | 5/1 | D95 |
| RC457 | MIA → MEM | 17:30–18:52 | Daglig | 5/1 | D95 |
| RC689 | MIA → MEM | 20:35–21:57 | Daglig | 5/1 | D9S |
| RC430 | MSP → MCO | 08:25–12:20 | 6 | 5/3 | D95 |
| RC64 | MSP → MCO | 13:10–17:05 | Daglig | 5/3 | D9S |
| RC433 | SRQ → DTW | 12:40–15:15 | Daglig | 6/2 | D95 |
| RC141 | SRQ → MEM | 10:15–11:13 | Daglig | 6/2 | DC9 |
| RC123 | TPA → DTW | 09:50–12:20 | Daglig | 6/2 | D95 |
| RC525 | TPA → DTW | 12:45–15:15 | Daglig | 6/2 | D95 |
| RC439 | TPA → MCO | 15:20–15:49 | 6 | 6/2 | D95 |
| RC385 | TPA → MCO | 16:45–17:15 | Daglig | 6/2 | D95 |
| RC480 | TPA → MCO | 23:00–23:29 | 6 | 6/2 | D95 |
| RC462 | TPA → MCO | 23:00–23:29 | X6 | 6/2 | D9S |
| RC448 | TPA → MEM | 07:00–07:50 | Daglig | 6/2 | DC9 |
| RC265 | TPA → MEM | 10:30–11:20 | Daglig | 6/2 | DC9 |
| RC405 | TPA → MEM | 14:00–14:50 | Daglig | 6/2 | D9S |

Fullständiga maskinläsbara rader finns i `caribbean_source_rows_batch121.json`. En schemarad kan ge flera daterade avgångar. Därför skiljer sig antalet scheman, rörelser och flygplatspar.

## Mellanstopp och tidsgränser

Republics MEM–GCM och GCM–MEM-rader har ett stopp. De separat tidsatta benen **MEM–MIA–GCM** respektive **GCM–MIA–MEM** importeras; totalsammanställningarna dubbleras inte som nonstop. Motsvarande gäller resorna via Orlando och Tampa.

**16 daterade övergångar** inom tio benkedjor har kontrollerats mot originalraderna. Markuppehållen är 30, 31, 35, 40, 65, 70, 73 eller 88 minuter. Samma flygnummer fastställer inte att samma individflygplan användes.

- RC712 Memphis–Orlando 22:40 landar **01:20 nästa lokala dag**. Källans `120A` betyder 1:20 AM, inte 00:20.
- RC430 Minneapolis–Orlando på lördag landar **12:20**. Fortsättningen Orlando–Tampa avgår 13:00 efter 40 minuter.
- RC385 Tampa–Orlando på torsdag landar **22:15 UTC**, 6 minuter 30 sekunder före fönstrets början, och ger ingen torsdagsrörelse här. Fortsättningen Orlando–Memphis avgår 22:45 UTC och ingår.
- RC696 Memphis–Orlando samt RC480 Memphis–Tampa och Tampa–Orlando är sena lördagsrader. De finns kvar som granskade scheman men skapar **noll** rörelser inom fönstret.
- Cancún använder **UTC−6 i februari 1986**, enligt den historiska tidszonskonverteringen; dagens UTC−5 har inte tillämpats bakåt. Grand Cayman, Florida och Detroit använder UTC−5; Memphis och Minneapolis UTC−6.

Fönstret är **27 februari 1986 22:21:30 UTC – 1 mars 22:21:30 UTC**. Samtliga 132 nya UTC-intervall matchar en separat transkription av varaktigheter och avgångsdatum med fasta tidszonsförskjutningar. Hela flygintervallet behålls när det överlappar fönstret. Inga tidsöverlapp finns mellan de importerade benen med samma Republic-flygnummer.

## Flygplatser och geografisk avgränsning

| Kod | Flygplats | Forskningsområde | Historisk tidszon |
|---|---|---|---|
| GCM | Grand Cayman – Owen Roberts | Caymanöarna | America/Cayman, UTC−5 |
| CUN | Cancún International | Mexiko | America/Cancun, UTC−6 |

Koderna identifieras i den samtida stadsförteckningen. [Cayman Islands Airports Authority](https://www.caymanairports.com/overview-of-owen-roberts-international-airport) daterar kommersiell trafik vid Owen Roberts till 1952. [Quintana Roos regering](https://cgc.qroo.gob.mx/veda_electoral/efemeride-30-marzo/) daterar överlämnandet av Cancúns internationella flygplatsanläggningar till 1975. Den äldre stadsflygplatsen används inte.

OurAirports-koordinater är ungefärliga flygfältsmarkörer, inte inmätta terminal- eller banpositioner från 1986. Senare terminaler, banor och förlängningar rekonstrueras inte. Alla äldre flygplats- och landobjekt är oförändrade. Se `airport_location_review_batch121.json`.

Den ursprungliga regionala avgränsningen med **29 forskningsområden** behålls: **131 → 139 unika ändpunktsrörelser**, 65 → 67 riktade par, 18 → 19 regionala flygplatser. De två nya Cancún-rörelserna och 122 amerikanska inrikesrörelserna redovisas separat och räknas inte in i 139. Caymanöarna är ett territorium, inte en ny självständig stat. Den tidigare anslutande täckningen av Franska Guyana är oförändrad.

| Område med trafik, samt Nicaragua | v123 | v124 |
|---|---:|---:|
| Nicaragua | 0 | 0 |
| Guatemala | 4 | 4 |
| Amerikanska Jungfruöarna / St. Thomas, St. Croix | 15 | 15 |
| Puerto Rico | 18 | 18 |
| Haiti | 6 | 6 |
| Dominikanska republiken | 4 | 4 |
| Bahamas | 27 | 27 |
| Caymanöarna | 0 | 8 |
| Turks- och Caicosöarna | 4 | 4 |
| Antigua och Barbuda | 1 | 1 |
| Saint Kitts och Nevis | 3 | 3 |
| Saint Lucia | 2 | 2 |
| Barbados | 14 | 14 |
| Trinidad och Tobago | 9 | 9 |
| Guadeloupe och dåvarande franska karibiska områden | 17 | 17 |
| Martinique | 19 | 19 |
| Nederländska Antillerna (1986) | 14 | 14 |

Områdesraderna överlappar och får inte summeras. Ändpunkter fastställer inte vilka länder flygen faktiskt överflög.

## Luckor och nästa steg

**Nicaragua har fortfarande noll importerade rörelser eftersom verifierade tidtabellsklockslag saknas. St. Thomas har fortsatt åtta rörelser.** Detta är dataluckor, inte belägg för att historisk trafik saknades.

Air Jamaicas utgåva 26 november 1985–26 april 1986 har identifierats via omslag och katalog, men inga användbara inre klockrader har erhållits. Arrow Airs index/URL anger november 1985 medan introduktionssidan skriver 1984; originalutgåva och giltighet måste klarläggas före import. BWIA-genomgången gav inga nya tillämpliga klockrader. Dessa spår och en samtida Nicaragua-ledtråd finns i `caribbean_withheld_batch121.json`.

Fortsätt med Nicaragua, fler operatörer kring St. Thomas och Centralamerika/Karibien. Republics återstående nät, andra Florida-flygplatser, Puerto Vallarta, EXP-operatörer och eventuella ändringsblad återstår. Air Frances tidigare hållna F27-operatörsfråga är fortsatt öppen. Inga nya militär-, stats-, privat-, charter- eller specialflyg läggs till. Sådan trafik kräver daterade rörelsehandlingar.

Sökningen är inte uttömmande. Inget verifierat globalt slutantal eller procent färdig finns. Se `central_america_caribbean_queue_batch121.json`.

## Kontroller och installation

**20 befintliga Python-tester och tre generatorkontroller passerar.** Äldre rörelser, scheman, flygplatser, bolag, källposter, kod, rättelser och hållna konflikter är bevarade. Den tidigare allmänna statusraden i verktygens README har också uppdaterats till aktuella antal.

Totalt **8 667 rörelser**: 8 666 planerade och en tidigare bekräftad. **5 081 katalogscheman / 5 060 granskade**, **364 flygplatser/platser**, **112 länder/territorier**, **1 689 riktade par**. 103 registrerade operatörer; 60 med rörelser (59 civila och US Marine Corps), 43 utan. 88 källposter och 30 bidragande tidtabellsutgåvor. All täckning är partiell.

Stäng Unreal Editor och slå samman ZIP-filens `Plugins/` och `Tools/` med projektroten bredvid `.uproject`, med bibehållen mappstruktur. Detta är **endast data**; `PaintAirplane`-rättelsen **f23b73a** måste redan vara kompilerad. C++-bygge, Unreal Editor och paketerat spel har inte körts här. Tidtabeller visar planerad trafik, inte bekräftat genomförande eller identifierade individflygplan.
