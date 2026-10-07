# PBA kring Tampa och norra Florida – v133 / batch130

Granskat 2 oktober 2026. **159 nya planerade flygrörelser**, totalt **9 488**. Alla nya rörelser är **inrikes i USA**. **22 riktade sträckor** får nya rörelser; **21 är nya i databasen**. Jacksonville–West Palm Beach fanns redan med annan trafik.

**176 nya granskade utgåvespecifika scheman**, varav **136 ger rörelser** och 40 har noll utfall i projektfönstret. Källraderna omfattar 24 riktade par, men **Jacksonville–Melbourne och Melbourne–Jacksonville ger inga rörelser** och räknas därför inte som nya animerade sträckor.

PBA har nu sammanlagt **302 rörelser från 323 granskade scheman** i v132 och v133. Bolagets nät är fortfarande partiellt importerat. Alla **9 329 äldre rörelseobjekt och 5 595 äldre scheman är oförändrade**. Inga flygplatser, länder, operatörer eller källposter läggs till eller ändras. Äldre forskningsrapporter och rättelser bevaras byte för byte.

## Sträckor med nya rörelser

| Riktad sträcka | Rörelser | Databasens sträckregister |
|---|---:|---|
| APF–TPA | 10 | Ny |
| DAB–JAX | 3 | Ny |
| GNV–JAX | 3 | Ny |
| GNV–TPA | 3 | Ny |
| JAX–DAB | 3 | Ny |
| JAX–GNV | 3 | Ny |
| JAX–PBI | 7 | Fanns tidigare |
| JAX–TLH | 8 | Ny |
| JAX–TPA | 9 | Ny |
| PBI–JAX | 8 | Ny |
| PBI–TPA | 8 | Ny |
| RSW–TPA | 10 | Ny |
| SRQ–TPA | 10 | Ny |
| TLH–JAX | 6 | Ny |
| TLH–TPA | 8 | Ny |
| TPA–APF | 11 | Ny |
| TPA–GNV | 4 | Ny |
| TPA–JAX | 10 | Ny |
| TPA–PBI | 8 | Ny |
| TPA–RSW | 10 | Ny |
| TPA–SRQ | 10 | Ny |
| TPA–TLH | 7 | Ny |

APF = Naples i Florida, DAB = Daytona Beach, GNV = Gainesville, JAX = Jacksonville, PBI = Palm Beach/West Palm Beach, RSW = Fort Myers, SRQ = Sarasota–Bradenton, TLH = Tallahassee, TPA = Tampa. MLB = Melbourne i Florida, inte Melbourne i Australien. En rörelse är ett daterat fysiskt flygben.

## Original och tolkning

Samma två originalutgåvor som i v132 används:

- [PBA Southern System, giltig från 15 januari 1986](https://flypba.com/timetable/s-1986-01-15/): **80 nya scheman, 103 nya rörelser**. Inlagor 1, 3 och 4. Filnamnet `01-01` ändrar inte omslagets uttryckliga datum 15 januari. Tillämpningen avslutas den 28 februari därför att nästa utgåva börjar den 1 mars; slutdagen är härledd, inte tryckt.
- [PBA Southern System, giltig från 1 mars 1986](https://flypba.com/timetable/s-1986-03-01/): **96 nya scheman, 56 nya rörelser**. Inlagor 1–3. Utgåvan används endast på lördagen i projektfönstret. Nya nattflyg förs inte tillbaka till februari.

Original från **Gordon K. Werners samling, [flyPBA.com](https://flypba.com/timetables/)**. Skanningarna distribueras inte i paketet. De 13 originalbildfilerna har samma verifierade SHA-256 som i v132; URL och hash finns i `caribbean_source_evidence_batch130.json`. Ingen ny källutgåva tillkommer i registret. Ändringsblad och faktisk drift är inte fullständigt verifierade.

Ett nummer utan stopp- eller anslutningsanmärkning tolkas som nonstop enligt tabellstrukturen. Explicit **1 Stop/2 Stops**, flera flygnummer och anslutningsflygplats ger inga extra direktflyg. D betyder tom daganmärkning/daglig trafik enligt tabellkonvention; X6 = utom lördag, X7 = utom söndag, X67 = utom lördag och söndag. Separat uttrycklig nonstop-/lokaltidslegend har inte återfunnits.

Alla klockslag tolkas som lokala enligt tidtabellskonvention. **Samtliga tio berörda flygplatser har UTC−5 på importdatumen**, även Tallahassee. Det västligare Floridas andra tidszon får inte användas för TLH. Befintliga historiskt granskade flygplatsposter bevaras; RSW är inte FMY och APF är inte italienska NAP. Koordinaterna är ungefärliga flygfältsmarkörer.

Flygnummer är de tryckta numeriska numren. PBA-attributionen följer publikationen, inte Easterns bonusreklam. Flygplanstyp, individflygplan, faktisk operatör vid eventuell inhyrning och juridisk drifthistorik är inte fastställda. Tidtabellen visar planering, inte bekräftat genomförande.

## Datum, nattider och fönstergränser

Fönstret är **27 februari 1986 kl. 22:21:30 UTC till 1 mars kl. 22:21:30 UTC**. Lokalt är båda gränserna **17:21:30**. Ett flyg räknas om dess intervall överlappar fönstret. Avgången behöver inte ligga inom det och ankomsten kan ligga efter slutet.

Nya rörelser efter lokal avgångsdag: **torsdag 23, fredag 80, lördag 56**. Alla januariutgåvans rörelser ligger på torsdag/fredag; alla marsutgåvans på lördag.

Sex mönster ger tre rörelser vardera över de två utgåvorna genom att överlappa båda gränserna: **1036 Naples–Tampa, 1075 Tampa–Fort Myers, 1053 Tampa–Sarasota, 1212 Tampa–Jacksonville, 807 Tallahassee–Tampa och 1270 Palm Beach–Jacksonville**. På lördagen räknas också **943 Jacksonville–Tallahassee 16:25–17:25**, trots att landningen sker efter slutgränsen.

**940 Tallahassee–Jacksonville 05:15–06:15** har X67 och ger ingen lördagsrörelse. **959 Jacksonville–Daytona Beach**, **971 Jacksonville–Gainesville** och **973 Jacksonville–Melbourne** landar efter midnatt följande lokala dygn. De ligger efter fönstret och ger inga rörelser. De får inte bakdateras till fredagen eftersom de kommer från marsutgåvan. **972 Melbourne–Jacksonville 21:55–23:00** ligger också utanför. Alla 40 nollutfall behålls som granskade scheman med dokumenterad orsak.

**1208 Sarasota–Tampa** trycks som **07:25–07:40 i januari**, men **07:15–07:40 i mars**. Båda är X67. Utgåveskillnaden bevaras; marsraden ger noll rörelser. Det är inte en rättelse av v132-data.

## Markuppehåll och anslutning till v132

**44 daterade anslutningar med samma flygnummer** har kontrollerats mot de tryckta klockslagen. Åtta ansluter till bevarade Key West/Naples-ben från v132. De 30 numrernas markuppehåll är:

| Nummer | Minuter mellan benen |
|---|---:|
| 800 | 30 |
| 801 | 30 |
| 802 | 20 |
| 803 | 20 |
| 804 | 30 |
| 805 | 20 |
| 806 | 50 |
| 807 | 25 |
| 1021 | 25 |
| 1024 | 30 |
| 1029 | 15 |
| 1030 | 15 |
| 1033 | 15 |
| 1036 | 30 |
| 1037 | 25 |
| 1047 | 35 |
| 1048 | 15 |
| 1051 | 16 |
| 1052 | 30 |
| 1053 | 25 |
| 1054 | 40 |
| 1074 | 65 |
| 1075 | 20 |
| 1076 | 40 |
| 1207 | 10 |
| 1208 | 25 |
| 1209 | 45 |
| 1211 | 15 |
| 1212 | 25 |
| 1213 | 30 |

Kontrollen styrker tidsmässig förenlighet mellan publicerade ben. Den bevisar inte att samma individflygplan utförde dem. Genomgående rader läggs inte ovanpå de fysiska benen. Exempelvis får **1208 Fort Myers–Tampa**, **1051 Jacksonville–Tampa**, **1042 Naples–Tampa** och **1213 Tampa–Naples** med uttryckligt stopp inte bli extra nonstop-rörelser. Se `caribbean_withheld_batch130.json`.

## Transkriberade klockslag

Inlaga avser bildfilens `inside-N`; kolumn räknas från vänster. Januariinlagorna har fyra kolumner, mars inlaga 2 har sex. Rörelseantal är utfallet i projektfönstret, inte all trafik under utgåvans livslängd.

### Januariutgåvan – tillämpad 27–28 februari

| Flygnummer | Sträcka | Lokala tider | Dagar | Rörelser | Inlaga/kolumn |
|---|---|---|---|---:|---|
| 1021 | TPA → APF | 08:15–09:20 | X67 | 1 | 4/2 |
| 1025 | TPA → APF | 11:50–12:45 | D | 1 | 4/2 |
| 1029 | TPA → APF | 13:20–14:15 | D | 1 | 4/2 |
| 1033 | TPA → APF | 15:40–16:35 | D | 1 | 4/2 |
| 1037 | TPA → APF | 18:23–19:20 | D | 2 | 4/2 |
| 1043 | TPA → APF | 22:25–23:15 | X6 | 2 | 4/2 |
| 1020 | APF → TPA | 06:50–07:40 | X67 | 1 | 3/1 |
| 1024 | APF → TPA | 10:00–10:55 | D | 1 | 3/1 |
| 1030 | APF → TPA | 12:00–12:55 | D | 1 | 3/1 |
| 1032 | APF → TPA | 13:45–14:40 | D | 1 | 3/1 |
| 1036 | APF → TPA | 17:00–17:50 | D | 2 | 3/1 |
| 1207 | TPA → RSW | 08:00–08:44 | X67 | 1 | 4/1 |
| 1073 | TPA → RSW | 10:55–11:39 | D | 1 | 4/1 |
| 1211 | TPA → RSW | 14:45–15:30 | D | 1 | 4/1 |
| 1075 | TPA → RSW | 17:05–17:50 | D | 2 | 4/1 |
| 1213 | TPA → RSW | 20:10–20:55 | X6 | 2 | 4/1 |
| 1072 | RSW → TPA | 09:45–10:30 | D | 1 | 1/2 |
| 1074 | RSW → TPA | 12:00–12:45 | D | 1 | 1/2 |
| 1212 | RSW → TPA | 16:00–16:45 | D | 1 | 1/2 |
| 1076 | RSW → TPA | 18:05–18:50 | X6 | 2 | 1/2 |
| 1042 | RSW → TPA | 20:50–21:35 | X6 | 2 | 1/2 |
| 1047 | TPA → SRQ | 08:25–08:50 | X67 | 1 | 4/2 |
| 1209 | TPA → SRQ | 11:20–11:45 | D | 1 | 4/2 |
| 1051 | TPA → SRQ | 13:15–13:40 | D | 1 | 4/2 |
| 1053 | TPA → SRQ | 17:15–17:40 | D | 2 | 4/2 |
| 1091 | TPA → SRQ | 20:05–20:30 | X6 | 2 | 4/2 |
| 800 | SRQ → TPA | 07:00–07:25 | X7 | 1 | 3/4 |
| 1208 | SRQ → TPA | 07:25–07:40 | X67 | 1 | 3/4 |
| 1048 | SRQ → TPA | 09:00–09:25 | X67 | 1 | 3/4 |
| 1088 | SRQ → TPA | 11:55–12:20 | D | 1 | 3/4 |
| 1052 | SRQ → TPA | 14:00–14:25 | D | 1 | 3/4 |
| 1054 | SRQ → TPA | 18:00–18:25 | D | 2 | 3/4 |
| 1208 | TPA → JAX | 08:05–09:10 | X67 | 1 | 4/1 |
| 1210 | TPA → JAX | 11:15–12:20 | D | 1 | 4/1 |
| 1074 | TPA → JAX | 13:50–14:55 | D | 1 | 4/1 |
| 1212 | TPA → JAX | 17:10–18:15 | D | 2 | 4/1 |
| 1076 | TPA → JAX | 19:30–20:35 | X6 | 2 | 4/1 |
| 1207 | JAX → TPA | 06:45–07:50 | X67 | 1 | 1/4 |
| 1209 | JAX → TPA | 09:30–10:35 | D | 1 | 1/4 |
| 1211 | JAX → TPA | 13:25–14:30 | D | 1 | 1/4 |
| 1075 | JAX → TPA | 15:40–16:45 | D | 1 | 1/4 |
| 1213 | JAX → TPA | 18:35–19:40 | X6 | 2 | 1/4 |
| 800 | TPA → TLH | 07:55–08:59 | X67 | 1 | 4/2 |
| 802 | TPA → TLH | 11:25–12:30 | D | 1 | 4/2 |
| 804 | TPA → TLH | 14:00–15:05 | D | 1 | 4/2 |
| 806 | TPA → TLH | 18:05–19:10 | D | 2 | 4/2 |
| 801 | TLH → TPA | 06:45–07:50 | X67 | 1 | 4/1 |
| 803 | TLH → TPA | 09:35–10:40 | D | 1 | 4/1 |
| 805 | TLH → TPA | 13:05–14:10 | D | 1 | 4/1 |
| 807 | TLH → TPA | 16:20–17:25 | D | 2 | 4/1 |
| 801 | TPA → PBI | 08:20–09:20 | X7 | 1 | 4/2 |
| 803 | TPA → PBI | 11:00–11:59 | D | 1 | 4/2 |
| 805 | TPA → PBI | 14:30–15:30 | D | 1 | 4/2 |
| 807 | TPA → PBI | 17:50–18:50 | D | 2 | 4/2 |
| 802 | PBI → TPA | 10:00–11:05 | D | 1 | 4/4 |
| 804 | PBI → TPA | 12:25–13:30 | D | 1 | 4/4 |
| 806 | PBI → TPA | 16:10–17:15 | D | 1 | 4/4 |
| 808 | PBI → TPA | 19:15–20:20 | X6 | 2 | 4/4 |
| 1048 | TPA → GNV | 09:40–10:30 | X67 | 1 | 4/1 |
| 1052 | TPA → GNV | 14:55–15:45 | D | 1 | 4/1 |
| 1054 | TPA → GNV | 19:05–19:55 | D | 2 | 4/1 |
| 1047 | GNV → TPA | 07:00–07:50 | X67 | 1 | 1/3 |
| 1051 | GNV → TPA | 12:10–12:59 | D | 1 | 1/3 |
| 1053 | GNV → TPA | 16:00–16:50 | D | 1 | 1/3 |
| 1265 | JAX → PBI | 06:20–07:55 | X67 | 1 | 1/4 |
| 1267 | JAX → PBI | 10:20–11:55 | D | 1 | 1/4 |
| 1269 | JAX → PBI | 14:20–15:55 | D | 1 | 1/4 |
| 1271 | JAX → PBI | 18:20–19:55 | X6 | 2 | 1/4 |
| 1266 | PBI → JAX | 08:20–09:55 | X67 | 1 | 4/2 |
| 1268 | PBI → JAX | 12:20–13:55 | D | 1 | 4/2 |
| 1270 | PBI → JAX | 16:20–17:55 | D | 2 | 4/2 |
| 1272 | PBI → JAX | 20:20–21:55 | X6 | 2 | 4/2 |
| 1356 | JAX → TLH | 06:35–07:35 | X67 | 1 | 1/4 |
| 1302 | JAX → TLH | 10:10–11:10 | D | 1 | 1/4 |
| 1358 | JAX → TLH | 14:30–15:30 | D | 1 | 1/4 |
| 1304 | JAX → TLH | 18:10–19:10 | D | 2 | 1/4 |
| 1301 | TLH → JAX | 08:40–09:40 | X67 | 1 | 3/4 |
| 1357 | TLH → JAX | 12:55–13:55 | D | 1 | 3/4 |
| 1303 | TLH → JAX | 16:15–17:15 | D | 1 | 3/4 |
| 1359 | TLH → JAX | 20:35–21:35 | D | 2 | 3/4 |
### Marsutgåvan – tillämpad endast 1 mars

| Flygnummer | Sträcka | Lokala tider | Dagar | Rörelser | Inlaga/kolumn |
|---|---|---|---|---:|---|
| 1021 | TPA → APF | 08:15–09:20 | X67 | 0 | 2/6 |
| 1025 | TPA → APF | 11:50–12:45 | D | 1 | 2/6 |
| 1029 | TPA → APF | 13:20–14:15 | D | 1 | 2/6 |
| 1033 | TPA → APF | 15:40–16:35 | D | 1 | 2/6 |
| 1037 | TPA → APF | 18:23–19:20 | D | 0 | 2/6 |
| 1043 | TPA → APF | 22:25–23:15 | X6 | 0 | 2/6 |
| 1020 | APF → TPA | 06:50–07:40 | X67 | 0 | 2/4 |
| 1024 | APF → TPA | 10:00–10:55 | D | 1 | 2/4 |
| 1030 | APF → TPA | 12:00–12:55 | D | 1 | 2/4 |
| 1032 | APF → TPA | 13:45–14:40 | D | 1 | 2/4 |
| 1036 | APF → TPA | 17:00–17:50 | D | 1 | 2/4 |
| 1207 | TPA → RSW | 08:00–08:44 | X67 | 0 | 2/6 |
| 1073 | TPA → RSW | 10:55–11:39 | D | 1 | 2/6 |
| 1211 | TPA → RSW | 14:45–15:30 | D | 1 | 2/6 |
| 1075 | TPA → RSW | 17:05–17:50 | D | 1 | 2/6 |
| 1213 | TPA → RSW | 20:10–20:55 | X6 | 0 | 2/6 |
| 1072 | RSW → TPA | 09:45–10:30 | D | 1 | 1/2 |
| 1074 | RSW → TPA | 12:00–12:45 | D | 1 | 1/2 |
| 1212 | RSW → TPA | 16:00–16:45 | D | 1 | 1/2 |
| 1076 | RSW → TPA | 18:05–18:50 | X6 | 0 | 1/2 |
| 1042 | RSW → TPA | 20:50–21:35 | X6 | 0 | 1/2 |
| 1047 | TPA → SRQ | 08:25–08:50 | X67 | 0 | 2/6 |
| 1209 | TPA → SRQ | 11:20–11:45 | D | 1 | 2/6 |
| 1051 | TPA → SRQ | 13:15–13:40 | D | 1 | 2/6 |
| 1053 | TPA → SRQ | 17:15–17:40 | D | 1 | 2/6 |
| 1091 | TPA → SRQ | 20:05–20:30 | X6 | 0 | 2/6 |
| 800 | SRQ → TPA | 07:00–07:25 | X7 | 1 | 2/5 |
| 1208 | SRQ → TPA | 07:15–07:40 | X67 | 0 | 2/5 |
| 1048 | SRQ → TPA | 09:00–09:25 | X67 | 0 | 2/5 |
| 1088 | SRQ → TPA | 11:55–12:20 | D | 1 | 2/5 |
| 1052 | SRQ → TPA | 14:00–14:25 | D | 1 | 2/5 |
| 1054 | SRQ → TPA | 18:00–18:25 | D | 0 | 2/5 |
| 1208 | TPA → JAX | 08:05–09:10 | X67 | 0 | 2/6 |
| 1210 | TPA → JAX | 11:15–12:20 | D | 1 | 2/6 |
| 1074 | TPA → JAX | 13:50–14:55 | D | 1 | 2/6 |
| 1212 | TPA → JAX | 17:10–18:15 | D | 1 | 2/6 |
| 1076 | TPA → JAX | 19:30–20:35 | X6 | 0 | 2/6 |
| 1207 | JAX → TPA | 06:45–07:50 | X67 | 0 | 1/3 |
| 1209 | JAX → TPA | 09:30–10:35 | D | 1 | 1/3 |
| 1211 | JAX → TPA | 13:25–14:30 | D | 1 | 1/3 |
| 1075 | JAX → TPA | 15:40–16:45 | D | 1 | 1/3 |
| 1213 | JAX → TPA | 18:35–19:40 | X6 | 0 | 1/3 |
| 800 | TPA → TLH | 07:55–08:59 | X67 | 0 | 2/6 |
| 802 | TPA → TLH | 11:25–12:30 | D | 1 | 2/6 |
| 804 | TPA → TLH | 14:00–15:05 | D | 1 | 2/6 |
| 806 | TPA → TLH | 18:05–19:10 | D | 0 | 2/6 |
| 801 | TLH → TPA | 06:45–07:50 | X67 | 0 | 2/5 |
| 803 | TLH → TPA | 09:35–10:40 | D | 1 | 2/5 |
| 805 | TLH → TPA | 13:05–14:10 | D | 1 | 2/5 |
| 807 | TLH → TPA | 16:20–17:25 | D | 1 | 2/5 |
| 801 | TPA → PBI | 08:20–09:20 | X7 | 1 | 2/6 |
| 803 | TPA → PBI | 11:00–11:59 | D | 1 | 2/6 |
| 805 | TPA → PBI | 14:30–15:30 | D | 1 | 2/6 |
| 807 | TPA → PBI | 17:50–18:50 | D | 0 | 2/6 |
| 802 | PBI → TPA | 10:00–11:05 | D | 1 | 3/2 |
| 804 | PBI → TPA | 12:25–13:30 | D | 1 | 3/2 |
| 806 | PBI → TPA | 16:10–17:15 | D | 1 | 3/2 |
| 808 | PBI → TPA | 19:15–20:20 | X6 | 0 | 3/2 |
| 1265 | JAX → PBI | 06:20–07:55 | X67 | 0 | 1/3 |
| 1267 | JAX → PBI | 10:20–11:55 | D | 1 | 1/3 |
| 1269 | JAX → PBI | 14:20–15:55 | D | 1 | 1/3 |
| 1271 | JAX → PBI | 18:20–19:55 | X6 | 0 | 1/3 |
| 1266 | PBI → JAX | 08:20–09:55 | X67 | 0 | 3/1 |
| 1268 | PBI → JAX | 12:20–13:55 | D | 1 | 3/1 |
| 1270 | PBI → JAX | 16:20–17:55 | D | 1 | 3/1 |
| 1272 | PBI → JAX | 20:20–21:55 | X6 | 0 | 3/1 |
| 939 | JAX → TLH | 02:50–03:59 | D | 1 | 1/3 |
| 941 | JAX → TLH | 12:25–13:25 | D | 1 | 1/3 |
| 943 | JAX → TLH | 16:25–17:25 | D | 1 | 1/3 |
| 945 | JAX → TLH | 20:05–21:05 | D | 0 | 1/3 |
| 940 | TLH → JAX | 05:15–06:15 | X67 | 0 | 2/5 |
| 942 | TLH → JAX | 14:50–15:50 | D | 1 | 2/5 |
| 944 | TLH → JAX | 18:35–19:35 | D | 0 | 2/5 |
| 946 | TLH → JAX | 22:00–23:00 | D | 0 | 2/5 |
| 950 | DAB → JAX | 05:15–05:50 | D | 1 | 1/1 |
| 952 | DAB → JAX | 11:20–11:55 | D | 1 | 1/1 |
| 954 | DAB → JAX | 15:15–15:50 | D | 1 | 1/1 |
| 956 | DAB → JAX | 19:00–19:35 | D | 0 | 1/1 |
| 958 | DAB → JAX | 22:25–23:00 | D | 0 | 1/1 |
| 951 | JAX → DAB | 02:50–03:25 | D | 1 | 1/2 |
| 953 | JAX → DAB | 12:25–13:00 | D | 1 | 1/2 |
| 955 | JAX → DAB | 16:25–17:00 | D | 1 | 1/2 |
| 957 | JAX → DAB | 20:05–20:40 | D | 0 | 1/2 |
| 959 | JAX → DAB | 23:35–00:10 (+1 dag) | D | 0 | 1/2 |
| 962 | GNV → JAX | 05:30–06:00 | D | 1 | 1/2 |
| 964 | GNV → JAX | 11:25–11:55 | D | 1 | 1/2 |
| 966 | GNV → JAX | 15:20–15:50 | D | 1 | 1/2 |
| 968 | GNV → JAX | 19:05–19:35 | D | 0 | 1/2 |
| 970 | GNV → JAX | 22:30–23:00 | D | 0 | 1/2 |
| 963 | JAX → GNV | 02:50–03:25 | D | 1 | 1/3 |
| 965 | JAX → GNV | 12:25–12:55 | D | 1 | 1/3 |
| 967 | JAX → GNV | 16:30–17:00 | D | 1 | 1/3 |
| 969 | JAX → GNV | 20:10–20:40 | D | 0 | 1/3 |
| 971 | JAX → GNV | 23:40–00:10 (+1 dag) | D | 0 | 1/3 |
| 972 | MLB → JAX | 21:55–23:00 | D | 0 | 2/1 |
| 973 | JAX → MLB | 23:35–00:40 (+1 dag) | D | 0 | 1/3 |

## Regional prioritet och nästa källor

Samma **31 forskningsområden för Centralamerika/Karibien** ligger kvar på **532 rörelser, 158 riktade par och 38 flygplatser**, varav 64 inrikesrörelser. **St. Thomas 14, Tortola 8 och Nicaragua 0** är oförändrade. Mexiko, separat angränsande område, har 20 rörelser. Mellanösternurvalet ligger kvar på 72. De 159 nya Florida-rörelserna räknas som USA-trafik.

Nicaragua och St. Thomas är fortsatt prioriterade. Den här omgångens kompletterande sökning gav ingen ny komplett, tillämplig klocktabell för dem. Noll är en datalucka, inte historisk trafikfrånvaro.

- [TACA-indexet](https://www.timetableimages.com/ttimages/ta.htm) gav ingen tillämplig 1986-inlaga bland det granskade materialet.
- [Aviateca-indexet](https://www.airtimes.com/cgat/gt/aviateca.htm) går i de närliggande systemposterna från 1979 till 1987. Det innebär en underlagslucka, inte att 1986-trafik saknades.
- Ett sökresultat ur [Museum of Flights arkivförteckning](https://archives.museumofflight.org/repositories/2/archival_objects/18066) pekar på TAN SAHSA-material från 1986. Själva sidan gav timeout och inga klockskanningar hämtades. Detta är endast ett arkivspår.
- Ett Eastern-kort från januari 1986 gällde enligt annonsmetadata Kanada; det gav inga Nicaragua-/St. Thomas-tider. Ingen beställning gjordes.

LACSA:s januari-/marsinlagor, Caribbean Express februariutgåva, sjöflygets ankomsttider och hamnidentiteter samt American/Eastern/BWIA kvarstår. Äldre LIAT-, Challenge-, Arrow- och Airways International-frågor ändras inte. Inga militär- eller specialrörelser tillkommer.

PBA:s nästa återstående urval är bland annat Miami till/från Daytona Beach, Melbourne, Sarasota och Palm Beach, MLB–PBI samt januariutgåvans fysiska ben kring västra Florida och New Orleans. Nätet måste fortsätta delas vid den 1 mars. Forskningskö: `central_america_caribbean_queue_batch130.json`.

## Totalt och kontroller

**9 488 rörelser: 9 487 planerade och en tidigare bekräftad. 5 771 katalogscheman, varav 5 750 granskade. 388 flygplatser/platser, 121 länder/territorier och 1 834 riktade par. 111 operatörsposter: 68 med rörelser och 43 utan. 93 källposter, 35 bidragande tidtabellsutgåvor.** Ingen verifierad global bolagsinventering eller färdigprocent finns.

**20 befintliga Python-tester och tre generatorkontroller passerar.** Alla 159 nya UTC-intervall matchar separat fast-offset-beräkning med manuellt granskade flygtider och datum. Alla 176 schemars flygtider har kontrollerats, inklusive nollutfallen. Samtliga 302 PBA-rörelser har kontrollerats för dubbletter och flygnummeröverlappning. Detta är kontroll av samma originalmaterial, inte en andra oberoende källa eller granskare.

Alla äldre rörelse- och schemaobjekt samt flygplats-, land-, operatörs- och källposter är oförändrade. Befintliga forskningsfiler/rättelser bevaras, medan aktuell README och genererade index uppdateras. Unreal Editor och spelet har inte körts.

Paketet är kumulativt. Stäng Unreal och slå samman `Plugins/` och `Tools/` med projektroten bredvid `.uproject`. **PaintAirplane-rättelsen f23b73a måste redan vara kompilerad.** Ingen ny C++-kod ingår.
