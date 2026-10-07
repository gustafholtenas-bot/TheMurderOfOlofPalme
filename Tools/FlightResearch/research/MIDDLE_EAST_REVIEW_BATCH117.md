# Mellanöstern – v120 / batch117

Granskat 1 oktober 2026. Kumulativ uppdatering från v119: **39 nya planerade flygrörelser**, totalt **8 418**. De nya rörelserna bygger på **37 separat tidsatta nonstop-rader** och berör 36 riktade flygplatspar, varav 35 är nya i databasen. Ingen region eller operatör markeras färdig.

## Tillägg i detta paket

| Bolag | Nya scheman | Nya daterade rörelser |
|---|---:|---:|
| Air France | 23 | 23 |
| UTA | 6 | 6 |
| Middle East Airlines | 2 | 4 |
| Alia | 2 | 2 |
| Saudia | 1 | 1 |
| El Al | 1 | 1 |
| Air Djibouti | 2 | 2 |
| **Totalt** | **37** | **39** |

De sex sistnämnda bolagen får sina första rörelser i databasen. Alia, Middle East Airlines, Saudia och det historiska Air Djibouti får nya bolagsposter; UTA och El Al fanns redan registrerade. Bolagsnamn följer den samtida källans legend. En tryckt bolagskod identifierar tidtabellens tjänst, inte ett visst individflygplan eller en eventuell underleverantör på en viss dag.

Bland tillskotten finns **Bagdad, Amman, Damaskus, Beirut, Jeddah, Kuwait, Bahrain, Dubai, Muscat, Kairo och Aden**. Dhahran och Tel Aviv kompletteras. Exempel: ME211/212 Beirut–Orly, RJ103/104 Amman–Orly, SV062 Paris–Jeddah, LY324 Orly–Tel Aviv samt DJ071/072 Djibouti–Aden. UTA:s ben bildar bland annat Singapore–Bahrain–Paris och Paris–Muscat–Colombo.

## Område och täckning

Genomgången använder ett brett Mellanösternområde inklusive **Egypten, Turkiet och Cypern**. **Nordjemen och Sydjemen hålls isär enligt 1986 års förhållanden.** Tabellen nedan anger flyg med start eller mål i respektive område, oavsett bolagets hemland.

| Område | Före v119 | Efter v120 | Nya |
|---|---:|---:|---:|
| Egypten | 0 | 3 | 3 |
| Israel | 4 | 7 | 3 |
| Jordanien | 0 | 3 | 3 |
| Libanon | 0 | 5 | 5 |
| Syrien | 0 | 2 | 2 |
| Irak | 0 | 2 | 2 |
| Iran | 0 | 0 | 0 |
| Saudiarabien | 1 | 13 | 12 |
| Kuwait | 0 | 1 | 1 |
| Bahrain | 0 | 2 | 2 |
| Qatar | 0 | 0 | 0 |
| Förenade arabemiraten | 0 | 1 | 1 |
| Oman | 0 | 4 | 4 |
| Nordjemen | 0 | 0 | 0 |
| Sydjemen | 0 | 2 | 2 |
| Turkiet | 8 | 8 | 0 |
| Cypern | 1 | 1 | 0 |

För hela området är antalet **unika rörelser 14 → 53**, riktade par **8 → 43** och registrerade flygplatser **4 → 15**. Länderna överlappar: exempelvis Damaskus–Amman räknas för både Syrien och Jordanien, men bara en gång i områdestotalen. Landraderna ska därför inte summeras. Noll betyder att databasen saknar rörelser, inte att historisk trafik saknades. Verkliga överflygningar kan inte räknas från flygens ändpunkter.

**Iran, Qatar och Nordjemen har fortfarande inga importerade rörelser.** Även övriga länder har stora luckor. Inrikestrafik saknas i detta regionala urval. Det finns ingen verifierad fullständig operatörsinventering eller nämnare för antalet återstående rutter; en färdigprocent skulle därför bli missvisande.

## Källor och importregler

Alla nya avgångsrader kommer från [Air France nr 25, vinter 1985/86](https://www.airtimes.com/cgat/fr/airfrance/pdf/1a/af851027-1a.pdf), giltig **27 oktober 1985–29 mars 1986**. Boken innehåller även andra bolags tjänster. Legend s.2–4, berörda datumvillkor och originalbilder har granskats visuellt. Accepterade tryckta sidor: 11, 13, 15, 18, 25, 27–31, 43, 48–50, 56, 61–62, 65, 68, 70, 76, 86–87 och 89. Källfilens SHA256 och varje transkriberad rad följer med i forskningsfilerna; originalskanningarna distribueras inte i ZIP.

Pilen i **VIA-kolumnen betyder nonstop**. En tom ruta är inte ett sådant belägg. Varje accepterat ben har eget flygnummer, lokala klockslag, veckodagar och ankomstens dygnsförskjutning. En genomgående resa över flera flygplatser blir inte ett obrutet flyg över mellanstoppet. Det tryckta `a` betyder ankomst nästa dag.

Datumvillkor har tillämpats: RJ103 från 2 november, RJ104 från 3 november, AF144 från 3 januari, ME211 till 28 mars, AF1352:s variant från 7 februari, AF1351 från 8 februari och UT567 Muscat–Paris från 3 november. Äldre december-/januarivarianter används inte. Scheman lagras enbart för de lokala avgångsdatum som behövs för observationsfönstret.

Saudias skannade bok från [Airline Memorabilia](https://airline-memorabilia.blogspot.com/2014/07/saudia-saudi-arabian-airlines-1985.html) gäller **1 juni–26 oktober 1985**. Den är alltså för gammal för februari 1986 och ger inga importerade tider. Emirates material från augusti/oktober 1986 är för sent. En historiksida som bara anger invigningsår ger inte datumgiltiga avgångstider.

Följande granskade rader hålls utanför:

- **Saudia SV061 Jeddah–Paris**, Kuwait Airways KU162 Orly–Kuwait och Yemenia IY748/749 Sana–Paris saknar nonstop-pil. Ingen av dessa blir en felaktig direktsträcka. Det finns inga KU- eller IY-rörelser i den nya importen.
- AF1382 har tidsatta Orly–Bagdad och Jeddah–Lyon, men saknar här Bagdads avgångstid mot Jeddah. Det mellanliggande benet konstrueras inte.
- AF160:s Paris–Kuwait är tidsatt; fortsättningen via Dubai och Abu Dhabi saknar fullständiga mellantider i det granskade materialet.
- AF145 Beirut–Paris och relevanta Riyadh–Paris-/Paris–Riyadh-rader är genomgående resor utan tillräckliga bentider.
- Iran Air-raderna går på tisdagar och ligger utanför fönstret. Andra söndagsrader avgår för sent; inga trafikdagar läggs till.
- AF140:s fredagsrad till Larnaca upphörde 27 december 1985. Befintlig AF164 lördag bevaras utan dubbelimport. Lyon–Tel Aviv-raderna gäller måndag.
- Den hämtade AF-skanningen saknar tryckta s.16–17 och 38–39, vilket lämnar luckor för bland annat Bangkok samt avgångar från Kuwait, Kairo och Larnaca.

`middle_east_withheld_batch117.json` redovisar dessa beslut. Äldre rättelser och hållna konflikter, inklusive Pan Ams Istanbul-tidszonproblem, bevaras.

## Vad som återstår per bolag

Detta är en prioriteringslista över undersökta källspår, inte en komplett lista över alla bolag verksamma i regionen 1986. Bolag kan finnas registrerade utan importerade rörelser.

| Bolag / källspår | Status och nästa kontroll |
|---|---|
| Saudia | En SV062-avgång CDG–JED importeras. SV061 JED–Paris saknar nonstop-pil och hålls. Sommarboken 1 juni–26 oktober1985 gäller inte spelfönstret. |
| Alia | RJ103/104 Amman–Orly i båda riktningar. Egen fullständig vinterbok och regionaltrafik återstår. |
| Middle East Airlines | ME211/212 Beirut–Orly i båda riktningar. Nice-radernas mellanstopp och övrigt nät återstår. |
| El Al | LY324 Orly–Tel Aviv lördag kväll. Övriga LY-rader, Arkia och CAL kräver egna granskade vinterunderlag. |
| Kuwait Airways | Omslag daterat27oktober1985. Innersidor behövs. AF:s KU162 fredag saknar nonstop-pil; ingen KU-rörelse importeras. |
| Gulf Air | Utgåva1januari–29mars1986 finns i index, men läsbara vinteravgångssidor har inte återfunnits. Gulf Air ska granskas för samtliga fyra Gulfstater utan att hemlandsfilter gissas. |
| EgyptAir | Vinteromslag27oktober1985 identifierat; inga kompletta avgångssidor importerade. Kairo får här tre AF-rörelser, inga EgyptAir-rörelser. |
| Iran Air | AF:s tryckta Iran Air-rader går på tisdagar och ger inga rörelser i fönstret. Egen vintertabell och inrikesnät återstår. |
| Iraqi Airways | Index gav inget importklart vinter1985/86-underlag. Bagdad får AF-rörelser; inga Iraqi Airways-tider konstrueras. |
| Syrian Arab Airlines | Ingen tillämplig vinter1985/86-tabell med fullständiga tider har hittats i granskat index. Damaskus får AF-rörelser. |
| Emirates | 1986-materialet som hittats gäller augusti/oktober. Det kan inte flyttas till februari. Officiell historik visar trafikstart1985 men saknar exakta tider för spelfönstret. |
| Yemenia | Vinteromslag27oktober1985 finns. AF:s IY748/749 Sana–Paris saknar nonstop-pil; fullständiga bentider saknas. |
| Alyemda | Egen vintertidtabell och datumgiltiga ändringar återstår. Aden–Djibouti importeras endast från Air Djiboutis separat tidsatta rader. |
| Turkish Airlines | Fyra befintliga TK-rörelser bevaras. Ankaraanslutningar saknar mellanliggande tider; ingen uppskattad inrikessträcka importeras. |
| Cyprus Airways / KTHY | Cyprus Airways förhandsmaterial har kvarstående legend/fotnotsluckor enligt batch43. KTHY kräver egen källa. Ingen ny CY-rörelse i denna omgång. |
| UTA | Sex regionala ben via Bahrain och Muscat. Genomgående Singapore–Paris delas endast där båda ändpunkternas tider är tryckta. |
| Air Djibouti | DJ071/072 Djibouti–Aden. Andra DJ-rader och dubbelkodade AFDJ-tjänster kräver separat operatörs- och delsträckskontroll. |

För Kuwait Airways, Gulf Air, EgyptAir och Yemenia finns vinteromslag eller katalogposter, men dessa räcker inte för att skapa flyg. Direktlänkar och status finns i `middle_east_research_queue_batch117.json`. Charter-, frakt-, regional-, helikopter-, privat-, stats- och militärtrafik behöver en separat fullständighetskontroll. Forskningsnamn som Arkia, CAL, TMA, Iran Aseman, Air Sinai, Misr Overseas och Abu Dhabi Aviation är kandidater för fortsatt inventering, inte automatiskt verifierade eller importerade avgångar.

## Samtliga nya riktningar

En rörelse är ett daterat flygintervall, medan ett schema är en återkommande tidtabellsrad. Två olika bolag eller flygnummer på samma riktning kan därför ge flera rörelser.

| Fysiskt ben | Nya rörelser |
|---|---:|
| ADE–JIB | 1 |
| AMM–ORY | 1 |
| BAH–CDG | 1 |
| BEY–ORY | 2 |
| BGW–CDG | 1 |
| CDG–BEY | 1 |
| CDG–DAM | 1 |
| CDG–DHA | 2 |
| CDG–JED | 1 |
| CDG–KWI | 1 |
| CDG–MCT | 1 |
| CDG–TLV | 1 |
| CMB–MCT | 1 |
| DAM–AMM | 1 |
| DAR–JED | 1 |
| DHA–CDG | 1 |
| DHA–DEL | 1 |
| DXB–CDG | 1 |
| JED–CDG | 1 |
| JED–DAR | 1 |
| JED–LYS | 1 |
| JED–SEZ | 1 |
| JIB–ADE | 1 |
| LYS–CAI | 1 |
| MCT–CDG | 1 |
| MCT–CMB | 1 |
| MRS–CAI | 1 |
| NCE–CAI | 1 |
| NCE–JED | 1 |
| ORY–AMM | 1 |
| ORY–BEY | 2 |
| ORY–BGW | 1 |
| ORY–TLV | 1 |
| SEZ–JED | 1 |
| SIN–BAH | 1 |
| TLV–NCE | 1 |
| **Totalt** | **39** |

## Flygplatser och historiska lägen

**17 markörer** tillkommer: SIN, BGW, AMM, DAM, BEY, DEL, JED, DAR, SEZ, DXB, CMB, MCT, BAH, KWI, CAI, ADE, JIB. Elva ligger i det avgränsade Mellanösternområdet. De sex övriga är motparterna Singapore–Changi, Delhi–Palam, Colombo–Katunayake, Djibouti–Ambouli, Dar es Salaam och Mahé/Seychellerna.

De samtida flygplatsnamnen styr identifieringen: Queen Alia i Amman, Saddam International väster om Bagdad, King Abdul Aziz i Jeddah, Seeb i Muscat och Khormaksar i Aden. Changi används i Singapore, inte Paya Lebar; Katunayake i Colombo, inte Ratmalana. Dubai är DXB, inte DWC. Sydjemen får en separat landspost.

Koordinaterna är ungefärliga markörer från OurAirports, kontrollerade mot tidtabellens flygplatsnamn och de flygplatshistoriker som anges i `airport_location_review_batch117.json`. De är inte uppmätta historiska terminaler, trösklar eller exakta referenspunkter för 1986. Inga tidigare flygplatsmarkörer ändras.

## Verifiering och installation

Observationsfönstret är **27 februari 1986 kl.22:21:30 UTC – 1 mars 1986 kl.22:21:30 UTC**. Samtliga 39 nya intervall har stämts av mot en separat manuell klocktranskription och fasta historiska UTC-förskjutningar, oberoende av byggarens ZoneInfo-beräkning. Damaskus är UTC+3 under dessa datum, Amman UTC+2; flyget 18:20–18:20 tar därför en timme.

AF147 Bagdad–Paris och AF463 Jeddah–Mahé börjar strax före fönstret men överlappar det och behåller sina fulla intervall. UT567 avgår Muscat **2 mars 01:40 lokal tid = 1 mars 21:40 UTC** och räknas därför också med. LY324 avgår Orly 1 mars 21:50 UTC och överlappar slutet.

Sju sammanhängande flygnummerserier har kontrollerade markuppehåll på **59, 60, 65, 70, 70, 70 och 75 minuter**. Inga tidsöverlapp finns mellan de nya benen och andra ben med samma bolag/flygnummer. Det identifierar inte individuella flygplan.

**20 Python-tester och tre kontroller av genererade filer passerar.** Alla **8 379 äldre rörelseobjekt och 4 889 äldre scheman är oförändrade**, liksom äldre flygplats- och landsposter. Tidtabellerna visar planerad trafik; faktisk drift, inställda flyg, last och verkliga flygbanor kräver andra underlag.

Efter import: **8 418 rörelser**, varav 8 417 tidtabellslagda och en dokumenterat genomförd; **4 926 katalogscheman / 4 905 granskade**; **346 flygplatser/platser**, **98 länder/territorier**, **1 598 riktade par**. **102 registrerade operatörer**, varav **59 med rörelser** och 43 utan. Dessa 43 är inte antalet kvarvarande bolag i världen.

Stäng Unreal Editor och slå samman ZIP-filens `Plugins/` och `Tools/` med projektroten bredvid `.uproject`. Ersätt motsvarande datafiler med bibehållen struktur. Detta är ett **datapaket** och kräver att den tidigare `PaintAirplane`-rättelsen **f23b73a** redan är kompilerad. Unreal Editor, C++-bygge och paketerat spel har inte körts här.
