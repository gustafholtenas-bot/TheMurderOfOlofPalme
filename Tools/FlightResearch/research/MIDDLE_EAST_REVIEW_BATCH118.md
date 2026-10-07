# Mellanöstern – v121 / batch118

Granskat 1 oktober 2026. **19 nya planerade El Al-rörelser**, totalt **8 437 rörelser**. Paketet är kumulativt från v120. De nya rörelserna bygger på 19 separat tidsatta scheman och tillför 19 riktade flygplatspar. Alla **8 418 äldre rörelseobjekt och 4 926 äldre scheman är oförändrade**. Inga tidigare flygplatser, länder, bolag, rättelser eller hållna konflikter ändras.

## Den nya källan

Läsbara originalfotografier har hittats av **El Al, Winter timetable 1985/6, issue one**, giltig **27 oktober 1985–29 mars 1986**. [Källpost med fotografier](https://www.ebay.co.uk/itm/198672080349). Den befintliga omslagsposten `el-al-19851027-cover` har uppgraderats till partiell skanning; dess id behålls av kompatibilitetsskäl. Det är samma utgåva, så ingen extra källpost skapas. Antalet bidragande tidtabellsutgåvor ökar däremot från 28 till 29.

Fotografiets två hela, onumrerade paneler **from tel-aviv** och **to tel-aviv** har granskats visuellt. Bilden visar även en avskuren Eilat-panel, som inte används. Omslaget och informationspanelen har kontrollerats. Bildadresser, filstorlekar och SHA256 finns i `middle_east_source_evidence_batch118.json`; originalbilderna återdistribueras inte i ZIP.

Dagkolumnerna går **söndag–lördag**, medan databasens dagar är **1=måndag … 7=söndag**. Stjärna, cirkel, fyrkant och triangel betecknar Boeing 747, 767, 707 respektive 737. Not 1 betyder ankomst nästa dag. Alla klockslag är lokala. Rader med angiven ort i **VIA** är genomgående resor över en mellanlandning. Denna källa har ett annat tabellformat än Air France-boken; dess tomma VIA-fält ska inte förväxlas med AF-bokens nonstop-pil.

Dagurvalet nedan är det granskade urvalet för spelfönstret. På rader där utrustningssymbolen varierar mellan dagar har urvalet avgränsats; tabellen är inte en fullständig veckotranskription. En tryckt flygplanstyp identifierar inte individflygplanet som faktiskt användes.

## Nya scheman

Varje rad ger en rörelse som överlappar spelfönstret. Koderna ATH och IST avser de historiska fälten **Ellinikon** och **Atatürk**. Inga moderna ersättningsflygplatser används.

| Flyg | Fysiskt ben | Lokala tider | Granskade dagar (1=mån) |
|---|---|---|---|
| LY005 | TLV → AMS | 23:30–03:25 +1 dygn | 26 |
| LY541 | TLV → ATH-ELLINIKON | 07:40–09:45 | 5 |
| LY357 | TLV → FRA | 10:00–13:25 | 5 |
| LY581 | TLV → IST-ATATURK | 09:50–11:50 | 5 |
| LY315 | TLV → LHR | 10:25–13:35 | 1235 |
| LY511 | TLV → NBO | 22:00–04:00 +1 dygn | 4 |
| LY001 | TLV → JFK | 01:00–05:50 | 45 |
| LY385 | TLV → FCO | 07:05–09:45 | 5 |
| LY363 | TLV → VIE | 06:50–09:35 | 5 |
| LY323 | TLV → ZRH | 09:30–12:40 | 5 |
| LY542 | ATH-ELLINIKON → TLV | 10:45–12:35 | 5 |
| LY334 | BRU → TLV | 08:30–13:35 | 5 |
| LY358 | FRA → TLV | 19:45–00:40 +1 dygn | 6 |
| LY582 | IST-ATATURK → TLV | 12:50–14:45 | 5 |
| LY318 | LHR → TLV | 22:00–04:35 +1 dygn | 6 |
| LY004 | JFK → TLV | 18:00–11:10 +1 dygn | 4 |
| LY386 | FCO → TLV | 11:25–15:35 | 5 |
| LY364 | VIE → TLV | 10:45–15:00 | 5 |
| LY348 | ZRH → TLV | 09:20–14:05 | 5 |

Tel Aviv får nya riktningar till Amsterdam, Aten, Frankfurt, Istanbul, London, Nairobi, New York, Rom, Wien och Zürich samt återresor från flera av dessa och Bryssel. El Al har nu **20 rörelser** i databasen inklusive tidigare LY324 från Orly.

## Datum, dubbletter och luckor

- **LY358 Frankfurt–Tel Aviv lördag** används med **19:45–00:40 nästa dag från 1 februari**. Varianten 18:15–23:10 till 25 januari utesluts.
- **LY324 Orly–Tel Aviv lördag 22:50–04:05 nästa dag** bekräftas av denna utgåva. Den finns redan i batch117 från Air France-boken och läggs inte in igen.
- **LY323 Tel Aviv–Orly fredag** går via Zürich. Endast den separat tidsatta raden **Tel Aviv–Zürich 09:30–12:40** läggs till; avgångstid från Zürich till Orly uppskattas inte.
- Resor till Chicago/Los Angeles via Amsterdam och eventuellt Chicago, samt Johannesburg–Tel Aviv via Nairobi, blir inte obrutna direktflyg. Mellantider som inte framgår lämnas öppna.
- Kairo-raderna överlappar inte fönstret. Torsdagens Kairo–Tel Aviv landar 21:20 UTC, före fönstrets start 22:21:30. Veckodagar utvidgas inte.
- LY008 från New York lördag 22:45 börjar först söndag 03:45 UTC. LY512 från Nairobi söndag 04:50 börjar 01:50 UTC. De ligger efter fönstrets slut.
- Eilat-kolumnerna är avskurna; flyg, tider och mellanlandningar kan inte läsas komplett. Ingen Eilat-trafik skapas.

Besluten finns i `middle_east_withheld_batch118.json`. Äldre hållna rader i batch117 och tidigare omgångar bevaras, inklusive Pan Ams Istanbul-konflikt. El Al-källan anger lokala tider utan Pan Ams problematiska UTC+3-rubrik; historisk ZoneInfo och den oberoende beräkningen använder UTC+2 för Istanbul vid dessa datum.

De Gulf Air-publikationer som kontrollerades i denna omgång gav inget tillämpligt tidtabellsunderlag: månadstidningens omslag anger **1981**, och den kontrollerade öppningssidan i nyhetsbrevssamlingen anger **8 oktober 1977**. TWA:s utgåva 12 januari 1986 finns katalogiserad, men läsbara avgångssidor kunde inte hämtas där. Inga tider tas från dessa spår.

## Regional täckning

Mellanöstern räknas här brett, inklusive Egypten, Turkiet och Cypern. Nord- och Sydjemen hålls isär enligt 1986. Unika rörelser med en ändpunkt i området ökar **53 → 72**, riktade par **43 → 62**. Antalet registrerade regionala flygplatser är fortsatt 15. Inrikestrafik saknas fortfarande i detta urval.

| Område | Före v120 | Efter v121 | Nya |
|---|---:|---:|---:|
| Egypten | 3 | 3 | 0 |
| Israel | 7 | 26 | 19 |
| Jordanien | 3 | 3 | 0 |
| Libanon | 5 | 5 | 0 |
| Syrien | 2 | 2 | 0 |
| Irak | 2 | 2 | 0 |
| Iran | 0 | 0 | 0 |
| Saudiarabien | 13 | 13 | 0 |
| Kuwait | 1 | 1 | 0 |
| Bahrain | 2 | 2 | 0 |
| Qatar | 0 | 0 | 0 |
| Förenade arabemiraten | 1 | 1 | 0 |
| Oman | 4 | 4 | 0 |
| Nordjemen | 0 | 0 | 0 |
| Sydjemen | 2 | 2 | 0 |
| Turkiet | 8 | 10 | 2 |
| Cypern | 1 | 1 | 0 |

Landraderna överlappar: Tel Aviv–Istanbul berör både Israel och Turkiet men räknas bara en gång i regiontotalen. Noll betyder en lucka i databasen, inte att historisk trafik saknades. Ändpunkter visar inte vilka länder flygningen faktiskt överflög. **Iran, Qatar och Nordjemen har fortfarande inga importerade rörelser.** Inget lands eller bolags nät är färdigställt, och någon verifierad totalsiffra för alla återstående flyg finns inte.

## Nästa område: Nicaragua, St. Thomas och regionen

Användarens nya prioritet efter denna Mellanösternomgång finns i **`central_america_caribbean_queue_batch118.json`**. Först granskas **Nicaragua** och **St. Thomas i Amerikanska Jungfruöarna**, därefter övriga Centralamerika och Karibien samt regionala länkar till södra Mexiko, Florida och norra Sydamerika.

Både Nicaragua och St. Thomas har i nuläget **0 importerade rörelser och inga flygplatsmarkörer** i katalogen. Befintlig trafik för bland annat Nassau, San Juan och Port of Spain ska kontrolleras före ny import. Pan Ams februaribok och Deltas februaribok är tillgängliga källspår; ännu ogranskade sidor räknas inte som importerad trafik.

Arbetet omfattar passagerar-, regional-, frakt- och charterflyg samt dokumenterad privat-, stats-, militär- och specialtrafik. Individflygplan kräver registrering eller rörelsehandlingar; tidtabellerna räcker inte. Exakta flygningar får inte härledas från allmän politisk historia eller antaganden om flygplanens verksamhet. Kön skapar inga nya flyg i spelet. Mellanösternluckorna och Europakön finns kvar för fortsatt arbete.

## Verifiering och leverans

Fönstret är **27 februari 1986 kl.22:21:30 UTC – 1 mars kl.22:21:30 UTC**. Samtliga 19 nya UTC-intervall har kontrollerats mot separat manuell transkription och fasta historiska UTC-förskjutningar, oberoende av byggarens ZoneInfo-omvandling.

**LY511 Tel Aviv–Nairobi** börjar 27 februari 20:00 UTC, före fönstret, men är fortfarande i luften vid starten. Hela intervallet bevaras. **LY001 Tel Aviv–New York** och **LY004 New York–Tel Aviv** börjar båda 27 februari 23:00 UTC trots olika lokala datum. **LY005** från Tel Aviv börjar 1 mars 21:30 UTC och **LY318** från Heathrow 22:00 UTC; båda överlappar slutet. Inga tidsöverlapp finns mellan ben med samma El Al-flygnummer. Ingen individuell flygplansrotation rekonstrueras.

**20 befintliga Python-tester och tre generatorkontroller passerar.** Bevarandekontroll jämför äldre objekt och filer. Efter import: **8 437 rörelser** (8 436 tidtabellslagda och en dokumenterat genomförd), **4 945 katalogscheman / 4 924 granskade**, **346 flygplatser/platser**, **98 länder/territorier**, **1 617 riktade par**. 102 registrerade operatörer, varav 59 med rörelser och 43 utan. Dessa 43 är inte ett globalt återstående bolagsantal.

Stäng Unreal Editor och slå samman ZIP-filens `Plugins/` och `Tools/` med projektroten bredvid `.uproject`, med bibehållen struktur. Paketet innehåller **endast data** och kräver att `PaintAirplane`-rättelsen **f23b73a** redan är kompilerad. Unreal Editor, C++-bygge och paketerat spel har inte körts här.
