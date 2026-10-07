# PBA i Florida Keys – v132 / batch129

Granskat 2 oktober 2026. **143 nya planerade flygrörelser**, totalt **9 329**. Alla nya rörelser är **inrikes i USA**, på **16 riktade sträckor som är nya i databasen**. Key West/EYW, Marathon/MTH och Naples/APF tillkommer som flygplatser. PBA tillkommer som operatörspost.

**147 granskade utgåvespecifika scheman**, varav **121 ger rörelser** och 26 ligger utanför det tillämpade lördagsfönstret eller undantar lördag. Schemarader som har samma klockslag i två utgåvor har separata, icke överlappande giltighetsperioder. Det är alltså inte 147 olika linjer.

Alla **9 186 äldre rörelseobjekt, 5 448 äldre scheman, 385 äldre flygplatsposter och samtliga äldre källor/operatörer/länder är oförändrade**. Äldre forskningsrapporter och rättelser följer med byte för byte.

## Nya rörelser per sträcka

| Riktad sträcka | Rörelser |
|---|---:|
| APF–EYW | 7 |
| APF–MIA | 12 |
| APF–RSW | 1 |
| EYW–APF | 7 |
| EYW–MIA | 20 |
| EYW–RSW | 3 |
| EYW–TPA | 6 |
| MIA–APF | 13 |
| MIA–EYW | 21 |
| MIA–MTH | 13 |
| MIA–RSW | 8 |
| MTH–MIA | 12 |
| RSW–APF | 1 |
| RSW–EYW | 3 |
| RSW–MIA | 9 |
| TPA–EYW | 7 |

EYW = Key West, MTH = Marathon, APF = Naples i Florida, RSW = Fort Myers, MIA = Miami, TPA = Tampa. En rörelse är ett daterat fysiskt flygben, inte en hel resa med anslutningar.

## Originalutgåvor och datumbyte

- [PBA Southern System, giltig från 15 januari 1986](https://flypba.com/timetable/s-1986-01-15/): **68 scheman och 90 rörelser**. Originalets filnamn innehåller `01-01`, men omslaget anger uttryckligen **January 15, 1986**. Omslagets datum styr. Sista tillämpningsdag **28 februari** härleds från marsutgåvans startdatum; detta är inte ett tryckt slutdatum.
- [PBA Southern System, giltig från 1 mars 1986](https://flypba.com/timetable/s-1986-03-01/): **79 scheman och 53 rörelser**. Utgåvan används endast den 1 mars i projektets fönster. Arkivet listar därefter en majutgåva; eventuella ändringsblad är inte fullständigt inventerade.

Original från **Gordon K. Werners samling, [flyPBA.com](https://flypba.com/timetables/)**. Skanningarna distribueras inte i datapaketet. URL, filstorlek och SHA-256 för alla 13 hämtade bilder finns i `caribbean_source_evidence_batch129.json`. Decemberutgåvan 1985 är ersatt och används inte.

Ett flygnummer utan stopp- eller anslutningsanmärkning tolkas som ett fysiskt nonstop-ben. Rader med **1 Stop**, **2 Stops**, flera nummer eller anslutningsflygplats läggs inte in som extra direktflyg. Detta bygger på tabellens struktur; en separat uttrycklig nonstop-legend har inte återfunnits. D betyder tom daganmärkning/daglig trafik enligt samma konvention. X6 = utom lördag, X7 = utom söndag, X67 = utom lördag och söndag.

A/p-markeringarna följs, och tiderna tolkas som lokala enligt tidtabellskonvention. En uttrycklig all-times-local-legend har inte återfunnits. Samtliga sex flygplatser har **UTC−5** på importdatumen enligt den historiska tidszonskontrollen. Ingen utrustning anges per flygning; omslagets flygplansbild är inte tilldelning till alla rader. Easterns bonusreklam är inte belägg för Eastern som operatör. PBA-attributionen följer den samtida publikationen; juridisk drift-/ägarhistorik och individflygplan är inte fastställda.

## Gränskontroller

Projektfönstret är **27 februari 1986 kl. 22:21:30 UTC till 1 mars kl. 22:21:30 UTC**, alltså **17:21:30 lokal tid** vid båda gränserna. Flygintervall som överlappar fönstret räknas, även om avgången ligger före start eller ankomsten efter slut.

- Avgångsdatum lokalt: **22 torsdag, 68 fredag och 53 lördag**.
- Januariutgåvan skapar inga lördagsrörelser. Marsutgåvan skapar inga februariavgångar.
- **1172 Key West–Miami** undantar söndag. Dess vidare ben **Miami–Fort Myers** undantar både lördag och söndag i **båda** utgåvorna. Lördagens första ben får därför inte skapa det andra.
- **1156 Miami–Naples** undantar lördag och söndag i båda utgåvorna.
- **975 Miami–Key West** avgår **01:00 den 1 mars**, inte 13:00. **976**, **977** och **960** behåller också tryckta natt-/morgontider. De förs inte tillbaka till februari.
- **974 Key West–Miami 23:30–00:25 nästa dygn** finns bara i marsutgåvan och ger noll rörelser i fönstret. En fredagsnatt får inte läggas till från den senare utgåvan.
- **1129 Miami–Marathon**, **1128 Miami–Fort Myers**, **1163 Naples–Miami** och **1379 Tampa–Key West** överlappar båda fönstergränserna och ger vardera tre rörelser sammanlagt över de två utgåvorna.
- **988 Key West–Naples** på lördagen räknas eftersom det avgår före slutgränsen. Det efterföljande Naples–Fort Myers-benet börjar 17:40 och ligger utanför.

Tio daterade anslutningar med samma nummer har kontrollerade markuppehåll: **1172: 21 minuter; 1124: 50; 1126: 15; 1128: 25; 1179: 35; 961 och 984: 10 minuter**. Detta kontrollerar tidtabellernas förenlighet och bevisar inte samma individflygplan. Naples i januari–Fort Myers har inte rekonstruerats ur genomgående rader eftersom ett självständigt klockblock saknas i det granskade materialet.

## Transkriberade klockslag

Inlaga avser bildfilens `inside-N`, kolumn räknas från vänster. Januariinlagorna har fyra kolumner; mars inlaga 2 har sex. Tabellen återger fakta ur originalet, med 24-timmarsformat. Rörelser visar utfallet inom projektfönstret.

### 15 januari 1986 – tillämpad 27–28 februari

| Flygnummer | Sträcka | Lokala tider | Dagar | Rörelser | Inlaga/kolumn |
|---|---|---|---|---:|---|
| 1172 | EYW → MIA | 07:00–07:44 | X7 | 1 | 2/1 |
| 1186 | EYW → MIA | 08:30–09:25 | D | 1 | 2/1 |
| 1174 | EYW → MIA | 10:00–10:55 | D | 1 | 2/1 |
| 1188 | EYW → MIA | 11:30–12:25 | D | 1 | 2/1 |
| 1176 | EYW → MIA | 13:00–13:55 | D | 1 | 2/1 |
| 1190 | EYW → MIA | 14:30–15:25 | D | 1 | 2/1 |
| 1178 | EYW → MIA | 16:00–16:55 | D | 1 | 2/1 |
| 1192 | EYW → MIA | 17:30–18:25 | D | 2 | 2/1 |
| 1180 | EYW → MIA | 19:00–19:55 | D | 2 | 2/1 |
| 1194 | EYW → MIA | 20:30–21:25 | X6 | 2 | 2/1 |
| 1024 | EYW → APF | 08:45–09:30 | D | 1 | 2/1 |
| 1030 | EYW → APF | 11:00–11:45 | D | 1 | 2/1 |
| 1036 | EYW → APF | 15:45–16:30 | D | 1 | 2/1 |
| 1042 | EYW → APF | 18:45–19:30 | D | 2 | 2/1 |
| 1376 | EYW → TPA | 09:45–10:59 | D | 1 | 2/2 |
| 1378 | EYW → TPA | 14:20–15:35 | D | 1 | 2/2 |
| 1380 | EYW → TPA | 18:00–19:15 | X6 | 2 | 2/2 |
| 1120 | MTH → MIA | 07:00–07:40 | D | 1 | 2/2 |
| 1122 | MTH → MIA | 09:15–09:55 | D | 1 | 2/2 |
| 1124 | MTH → MIA | 11:15–11:55 | D | 1 | 2/2 |
| 1126 | MTH → MIA | 14:15–14:55 | D | 1 | 2/2 |
| 1128 | MTH → MIA | 16:15–16:55 | D | 1 | 2/2 |
| 1130 | MTH → MIA | 18:15–18:55 | D | 2 | 2/2 |
| 1173 | MIA → EYW | 08:30–09:25 | X7 | 1 | 2/4 |
| 1187 | MIA → EYW | 10:00–10:55 | D | 1 | 2/4 |
| 1175 | MIA → EYW | 11:45–12:40 | D | 1 | 2/4 |
| 1189 | MIA → EYW | 13:00–13:55 | D | 1 | 2/4 |
| 1177 | MIA → EYW | 14:30–15:25 | D | 1 | 2/4 |
| 1191 | MIA → EYW | 16:00–16:55 | D | 1 | 2/4 |
| 1179 | MIA → EYW | 17:30–18:25 | D | 2 | 2/4 |
| 1193 | MIA → EYW | 19:00–19:55 | D | 2 | 2/4 |
| 1181 | MIA → EYW | 20:30–21:25 | D | 2 | 2/4 |
| 1195 | MIA → EYW | 22:05–22:55 | X6 | 2 | 2/4 |
| 1121 | MIA → MTH | 08:15–08:55 | D | 1 | 2/4 |
| 1123 | MIA → MTH | 10:15–10:55 | D | 1 | 2/4 |
| 1125 | MIA → MTH | 13:15–13:55 | D | 1 | 2/4 |
| 1127 | MIA → MTH | 15:15–15:55 | D | 1 | 2/4 |
| 1129 | MIA → MTH | 17:15–17:55 | D | 2 | 2/4 |
| 1131 | MIA → MTH | 19:55–20:35 | D | 2 | 2/4 |
| 1156 | MIA → APF | 08:35–09:20 | X67 | 1 | 2/4 |
| 1158 | MIA → APF | 12:05–12:45 | D | 1 | 2/4 |
| 1160 | MIA → APF | 14:05–14:45 | D | 1 | 2/4 |
| 1162 | MIA → APF | 16:05–16:45 | D | 1 | 2/4 |
| 1164 | MIA → APF | 18:05–18:45 | D | 2 | 2/4 |
| 1166 | MIA → APF | 20:05–20:45 | X6 | 2 | 2/4 |
| 1112 | MIA → APF | 21:05–21:45 | X6 | 2 | 2/4 |
| 1172 | MIA → RSW | 08:05–08:45 | X67 | 1 | 2/3 |
| 1124 | MIA → RSW | 12:45–13:30 | D | 1 | 2/3 |
| 1126 | MIA → RSW | 15:10–15:55 | D | 1 | 2/3 |
| 1128 | MIA → RSW | 17:20–17:59 | D | 2 | 2/3 |
| 1101 | RSW → MIA | 07:00–07:40 | X67 | 1 | 1/2 |
| 1103 | RSW → MIA | 09:50–10:30 | D | 1 | 1/2 |
| 1107 | RSW → MIA | 14:00–14:40 | D | 1 | 1/2 |
| 1179 | RSW → MIA | 16:15–16:55 | D | 1 | 1/2 |
| 1111 | RSW → MIA | 18:15–18:59 | X6 | 2 | 1/2 |
| 1155 | APF → MIA | 07:00–07:40 | X67 | 1 | 3/1 |
| 1157 | APF → MIA | 09:40–10:25 | D | 1 | 3/1 |
| 1159 | APF → MIA | 13:05–13:45 | D | 1 | 3/1 |
| 1161 | APF → MIA | 15:05–15:45 | D | 1 | 3/1 |
| 1163 | APF → MIA | 17:05–17:45 | D | 2 | 3/1 |
| 1165 | APF → MIA | 19:05–19:45 | X6 | 2 | 3/1 |
| 1021 | APF → EYW | 09:45–10:30 | D | 1 | 3/1 |
| 1029 | APF → EYW | 14:30–15:15 | D | 1 | 3/1 |
| 1033 | APF → EYW | 16:50–17:35 | D | 2 | 3/1 |
| 1037 | APF → EYW | 19:45–20:30 | X6 | 2 | 3/1 |
| 1375 | TPA → EYW | 08:10–09:25 | X7 | 1 | 4/1 |
| 1377 | TPA → EYW | 12:45–13:59 | D | 1 | 4/1 |
| 1379 | TPA → EYW | 16:15–17:30 | D | 2 | 4/1 |
### 1 mars 1986 – tillämpad endast 1 mars

| Flygnummer | Sträcka | Lokala tider | Dagar | Rörelser | Inlaga/kolumn |
|---|---|---|---|---:|---|
| 1172 | EYW → MIA | 07:00–07:44 | X7 | 1 | 1/4 |
| 1186 | EYW → MIA | 08:30–09:25 | D | 1 | 1/4 |
| 1174 | EYW → MIA | 10:00–10:55 | D | 1 | 1/4 |
| 1188 | EYW → MIA | 11:30–12:25 | D | 1 | 1/4 |
| 1176 | EYW → MIA | 13:00–13:55 | D | 1 | 1/4 |
| 1190 | EYW → MIA | 14:30–15:25 | D | 1 | 1/4 |
| 1178 | EYW → MIA | 16:00–16:55 | D | 1 | 1/4 |
| 1192 | EYW → MIA | 17:30–18:25 | D | 0 | 1/4 |
| 1180 | EYW → MIA | 19:00–19:55 | D | 0 | 1/4 |
| 1194 | EYW → MIA | 20:30–21:25 | X6 | 0 | 1/4 |
| 974 | EYW → MIA | 23:30–00:25 (+1 dag) | D | 0 | 1/4 |
| 984 | EYW → APF | 10:00–10:45 | D | 1 | 1/4 |
| 988 | EYW → APF | 16:45–17:30 | D | 1 | 1/4 |
| 1376 | EYW → TPA | 09:45–10:59 | D | 1 | 1/4 |
| 1378 | EYW → TPA | 14:20–15:35 | D | 1 | 1/4 |
| 1380 | EYW → TPA | 18:00–19:15 | X6 | 0 | 1/4 |
| 1120 | MTH → MIA | 07:00–07:40 | D | 1 | 2/1 |
| 1122 | MTH → MIA | 09:15–09:55 | D | 1 | 2/1 |
| 1124 | MTH → MIA | 11:15–11:55 | D | 1 | 2/1 |
| 1126 | MTH → MIA | 14:15–14:55 | D | 1 | 2/1 |
| 1128 | MTH → MIA | 16:15–16:55 | D | 1 | 2/1 |
| 1130 | MTH → MIA | 18:15–18:55 | D | 0 | 2/1 |
| 975 | MIA → EYW | 01:00–01:55 | D | 1 | 2/2 |
| 1173 | MIA → EYW | 08:30–09:25 | X7 | 1 | 2/2 |
| 1187 | MIA → EYW | 10:00–10:55 | D | 1 | 2/2 |
| 1175 | MIA → EYW | 11:45–12:40 | D | 1 | 2/2 |
| 1189 | MIA → EYW | 13:00–13:55 | D | 1 | 2/2 |
| 1177 | MIA → EYW | 14:30–15:25 | D | 1 | 2/2 |
| 1191 | MIA → EYW | 16:00–16:55 | D | 1 | 2/2 |
| 1179 | MIA → EYW | 17:30–18:25 | D | 0 | 2/2 |
| 1193 | MIA → EYW | 19:00–19:55 | D | 0 | 2/2 |
| 1181 | MIA → EYW | 20:30–21:25 | D | 0 | 2/2 |
| 1195 | MIA → EYW | 22:05–22:55 | X6 | 0 | 2/2 |
| 1121 | MIA → MTH | 08:15–08:55 | D | 1 | 2/2 |
| 1123 | MIA → MTH | 10:15–10:55 | D | 1 | 2/2 |
| 1125 | MIA → MTH | 13:15–13:55 | D | 1 | 2/2 |
| 1127 | MIA → MTH | 15:15–15:55 | D | 1 | 2/2 |
| 1129 | MIA → MTH | 17:15–17:55 | D | 1 | 2/2 |
| 1131 | MIA → MTH | 19:55–20:35 | D | 0 | 2/2 |
| 1156 | MIA → APF | 08:35–09:20 | X67 | 0 | 2/3 |
| 1158 | MIA → APF | 12:05–12:45 | D | 1 | 2/3 |
| 1160 | MIA → APF | 14:05–14:45 | D | 1 | 2/3 |
| 1162 | MIA → APF | 16:05–16:45 | D | 1 | 2/3 |
| 1164 | MIA → APF | 18:05–18:45 | D | 0 | 2/3 |
| 1166 | MIA → APF | 20:05–20:45 | X6 | 0 | 2/3 |
| 1112 | MIA → APF | 21:05–21:45 | X6 | 0 | 2/3 |
| 1172 | MIA → RSW | 08:05–08:45 | X67 | 0 | 2/2 |
| 1124 | MIA → RSW | 12:45–13:30 | D | 1 | 2/2 |
| 1126 | MIA → RSW | 15:10–15:55 | D | 1 | 2/2 |
| 1128 | MIA → RSW | 17:20–17:59 | D | 1 | 2/2 |
| 1101 | RSW → MIA | 07:00–07:40 | X67 | 0 | 1/1 |
| 1103 | RSW → MIA | 09:50–10:30 | D | 1 | 1/1 |
| 1107 | RSW → MIA | 14:00–14:40 | D | 1 | 1/1 |
| 1179 | RSW → MIA | 16:15–16:55 | D | 1 | 1/1 |
| 1111 | RSW → MIA | 18:15–18:59 | X6 | 0 | 1/1 |
| 1155 | APF → MIA | 07:00–07:40 | X67 | 0 | 2/4 |
| 1157 | APF → MIA | 09:40–10:25 | D | 1 | 2/4 |
| 1159 | APF → MIA | 13:05–13:45 | D | 1 | 2/4 |
| 1161 | APF → MIA | 15:05–15:45 | D | 1 | 2/4 |
| 1163 | APF → MIA | 17:05–17:45 | D | 1 | 2/4 |
| 1165 | APF → MIA | 19:05–19:45 | X6 | 0 | 2/4 |
| 961 | APF → EYW | 08:30–09:15 | D | 1 | 2/3 |
| 989 | APF → EYW | 19:00–19:45 | D | 0 | 2/3 |
| 1375 | TPA → EYW | 08:10–09:25 | X7 | 1 | 2/6 |
| 1377 | TPA → EYW | 12:45–13:59 | D | 1 | 2/6 |
| 1379 | TPA → EYW | 16:15–17:30 | D | 1 | 2/6 |
| 976 | EYW → RSW | 02:05–02:55 | D | 1 | 1/4 |
| 960 | EYW → RSW | 05:25–06:15 | D | 1 | 1/4 |
| 986 | EYW → RSW | 12:45–13:50 | D | 1 | 1/4 |
| 977 | RSW → EYW | 03:30–04:25 | D | 1 | 1/1 |
| 985 | RSW → EYW | 11:35–12:25 | D | 1 | 1/1 |
| 987 | RSW → EYW | 13:55–14:45 | D | 1 | 1/1 |
| 961 | RSW → APF | 08:00–08:20 | D | 1 | 1/2 |
| 989 | RSW → APF | 18:30–18:50 | D | 0 | 1/2 |
| 1213 | RSW → APF | 21:05–21:25 | X6 | 0 | 1/2 |
| 984 | APF → RSW | 10:55–11:15 | D | 1 | 2/3 |
| 988 | APF → RSW | 17:40–18:00 | D | 0 | 2/3 |
| 1042 | APF → RSW | 20:10–20:35 | X6 | 0 | 2/3 |
| 1112 | APF → RSW | 21:55–22:20 | X6 | 0 | 2/3 |

## Flygplatsidentitet

Koderna **EYW, MTH och APF står uttryckligen i originalrubrikerna**. Samma sak gäller Fort Myers **RSW**; det ersätts inte av det äldre fältet FMY.

- **Key West/EYW:** [Monroe Countys historik](https://www.monroecounty-fl.gov/921/Key-West-International-Airport-Fire-Resc) beskriver Meacham-fältet på sydöstra Key West och dess civila identitet före 1986. Det är inte Boca Chica/NQX eller en sjöflygbas.
- **Marathon/MTH:** [länets flygplatshistorik](https://www.monroecounty-fl.gov/109/Florida-Keys-Marathon-International-Airp) beskriver fältets öppnande 1943 och efterkrigstidens civila användning. Senare tullbyggnader återprojiceras inte till 1986.
- **Naples/APF:** [flygplatsmyndigheten](https://www.flynaples.com/about-naples-airport/) beskriver fältet från 1943 och myndigheten från 1969. Detta är Florida, inte Neapel/NAP i Italien.

Koordinater från [OurAirports](https://ourairports.com/data/) är ungefärliga flygfältsmarkörer, inte rekonstruerade terminaler eller banor från 1986. Källdatafilens hash sparas i flygplatsgranskningen.

## Fortsatt forskning och geografisk avgränsning

Samma **31 forskningsområden för Centralamerika/Karibien** ligger kvar på **532 rörelser, 158 riktade par och 38 flygplatser**, varav 64 inrikesrörelser. **St. Thomas 14, Tortola 8, Nicaragua 0** är oförändrade. Mexiko, separat angränsande område, ligger kvar på 20 rörelser. Mellanösternurvalet ligger kvar på 72. Florida Keys redovisas som USA-trafik; de höjer inte den regionala summan.

Nicaragua och St. Thomas är fortsatt prioriterade. Noll betyder en datalucka, inte historisk trafikfrånvaro. Gull Airs februariomslag/karta och Bahamasairs decemberutgåva gav inga kompletta nya klocktabeller. Aeronica saknar fortfarande tillämplig full inlaga. Inga Bahamas-, militär- eller specialrörelser tillkommer här.

Kvarstående spår omfattar LACSA:s januari-/marsinlagor 1986, Caribbean Express februariutgåva, sjöflygets ankomsttider och hamnidentiteter, samt American, Eastern och BWIA. Tidigare frågor om LIAT, Airways International, Challenge och Arrow bevaras. PBA:s resterande sydliga nät är också partiellt granskat; januari- och marsnätet får inte behandlas som identiskt. Se `central_america_caribbean_queue_batch129.json`.

## Globalt läge och validering

**9 329 rörelser: 9 328 planerade och en tidigare bekräftad. 5 595 katalogscheman, varav 5 574 granskade. 388 flygplatser/platser, 121 länder/territorier, 1 813 riktade par. 111 operatörsposter: 68 med rörelser och 43 utan. 93 källposter och 35 bidragande tidtabellsutgåvor.** Ingen verifierad global bolagsinventering eller färdigprocent finns.

**20 befintliga Python-tester och tre generatorkontroller passerar.** Alla 143 nya UTC-intervall matchar separat fast-offset-beräkning, manuellt granskade flygtider och väntade datum. Det är en kontroll av samma källmaterial, inte en andra oberoende källa eller granskare. Kontroll omfattar datumbytet, veckodagar, nattider, båda fönstergränserna, dubbletter, flygnummeröverlappning, tio markuppehåll och äldre objekts bevarande. Unreal Editor och spelet har inte körts.

README:s inaktuella löpande registertal och prioritetsstycke uppdateras till aktuellt läge. Historiska paketnotiser och äldre forskningsfiler bevaras.

Stäng Unreal Editor och slå samman ZIP-filens `Plugins/` och `Tools/` med projektroten bredvid `.uproject`. Paketet är kumulativt och innehåller endast data och forskningsverktyg. **PaintAirplane-rättelsen f23b73a måste redan vara kompilerad.**
