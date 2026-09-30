# Nordiska anslutningar och Teesside – omgång 13, paket v14

Granskat 27 september 2026. **29 nya planerade delsträckor** ger **975 rörelser totalt**: 974 tidtabellslagda och en tidigare dokumenterad militär rörelse. De 946 tidigare posterna är oförändrade. De nya animerade sträckorna ligger i Storbritannien samt mellan Teesside och Amsterdam. Antalet animerade rörelser med nordisk ändpunkt är därför fortfarande **37**; inga nya överflygningar har belagts.

## Importerade avgångar

Primärkälla: [Tees-side Airport, Air Services Timetable, Winter 1985–86](https://www.dtvmovements.co.uk/Info/History/Documents/Programmes/Timetable_1985W.pdf), PDF 1–2. Båda sidorna och teckenförklaringen har lästs visuellt. Omslaget anger november–mars, utan exakta gränsdagar. Importen är begränsad till de granskade dagarna 28 februari och 1 mars 1986.

| Operatör enligt källans kod | Delsträckor | Fredag | Lördag | Totalt |
|---|---|---:|---:|---:|
| British Midland, BD | Teesside ↔ Heathrow | 10 | 6 | 16 |
| Casair, KS | Humberside ↔ Teesside ↔ Glasgow | 8 | 0 | 8 |
| Jersey European, JY | Teesside ↔ Blackpool | 2 | 0 | 2 |
| Dan-Air, DA | Teesside ↔ Amsterdam | 0 | 2 | 2 |
| Air UK, UK | Teesside → Norwich | 1 | 0 | 1 |
| **Summa** | | **21** | **8** | **29** |

De 25 transkriberade raderna finns i `teesside_batch13.tsv`. Inget markerat byte eller mellanlandningsalternativ har ritats som nonstop. Blank stoppkolumn läses tillsammans med tabellens uttryckliga markeringar ”Ops via” och ”Chng at”. Casairs genomgående nummer har delats vid Teesside med respektive tryckta tider. JY102/105 förekommer på båda PDF-sidorna men räknas bara en gång. Flygplansbeteckningarna sparas som källuppgifter, inte som verifierade flygplansindivider.

Fem flygplatser har registrerats: MME, BLK, HUY, NWI och ABZ. Aberdeen är endast en granskad bytespunkt i denna omgång och får inga nya animerade avgångar. Koordinatunderlag finns i `airport_locations_batch13.json`. Flygfältsmarkörerna rekonstruerar inte terminaler eller flygplanspositioner 1986.

## Samtida ändringar som påverkar importen

[Teesside Aviation News, vol. 3 nr 3, mars 1986](https://www.dtvmovements.co.uk/Archivesmonths/1986/1986%20-%20Feb.pdf), tryckt s. 24 / PDF 4, beskriver ändringar under februari:

- Air UK:s UK204/205 ändras till UK101/106. Den exakta ändringsdagen framgår inte. Dessa nummerberörda rader undantas i stället för att det nya numret bakdateras på antagande.
- Dan-Airs DA813/814 på måndag/onsdag/fredag dras in. De importerades inte.
- DA812/819 beskrivs som kvarvarande, men deras mellanlandning i Newcastle saknar fullständiga delsträckstider i vinterbladet. Inte heller de animeras här.

[Aprilnumrets marsöversikt](https://www.dtvmovements.co.uk/Archivesmonths/1986/1986%20-%20Mar.pdf), s. 34–35 / PDF 6–7, har också lästs. Senare ändringar hos Dan-Air och Air Ecosse bakdateras inte till mordhelgen. Rörelserna för 1 mars ger inte kompletta start- och sluttider för nya fysiska flygsträckor.

## Dokumenterad nordisk notis: SE-FNZ till Halmstad

Februarijournalen, **tryckt s. 26 / PDF 6**, anger följande för **SE-FNZ, Be55**:

| Händelse vid Teesside | Datum | Klockslag som tryckt | Uppgiven annan ort |
|---|---|---|---|
| Ankomst | 27 februari 1986 | 13.56 | Från Billund |
| Avgång | 28 februari 1986 | 15.02 | Till Halmstad |

Raden står under den 27:e; avgången är uttryckligen märkt **28/2**. Detta är två stationstider vid Teesside, inte båda ändtiderna för ett flyg Billund–Halmstad. Ankomsten den 27:e är bakgrund före vårt tidsfönster. Journalen är en samtida föreningspublikation, inte en officiell flygledningslogg. Tidszon, Billunds avgångstid, Halmstads ankomsttid, operatör, passagerare och last saknas. Uppgiften belägger ingen anknytning till mordet.

Posten ligger strukturerad i **`nordic_movement_notes.json`**, med `runtime_import: false` och `animate: false`. Den visas alltså inte som ett tidsatt flyg på globen. Nästa kontroll är Halmstads ankomstjournal den 28 februari, Billunds avgångslista den 27 februari och flygplans-/operatörslogg. Ingen flygtid eller UTC-konvertering har hittats på.

## Nordiska tidtabellsspår som återstår

| Källa | Lästa delar | Resultat och nästa steg |
|---|---|---|
| Teessides vintertabell | PDF 1–2 | Bergen via Aberdeen, Stavanger via Aberdeen och Esbjerg via Humberside. Saknar kompletta tider för nordiska fysiska delsträckor. Sök Air UK, SAS och Air Ecosse samt ändringsblad. |
| [LOT, vinter 27 okt 1985–29 mars 1986](https://www.ebay.com/itm/198449943966) | Omslag, s. 2–3 och 8–9 | Transferförslag med LO362 från Köpenhamn, LO351 till Stockholm och LO371 till Helsingfors. Sök direktflygsidorna 4–7 och transferfotnoterna på s. 11. Helsinkis tryckta GMT-offset behöver kontrolleras. Inga hela anslutningsresor eller härledda delsträckor importerade. |
| [Finnair, två utgåvor i samma annons](https://www.ebay.com/itm/205166838524) | Tre fotografier | Vinteromslaget anger rätt period, men det öppna uppslaget har sommarens datumgränser. Vinterkälla registrerad som endast omslag; inga tider importerade. Fortsätt med vinterutgåvan från 23 december. |
| [Swissair Benelux, vinter 1985/86](https://www.ebay.com/itm/198568921278) | Omslag och fotograferade s. 24–25 | Giltig vinterutgåva men inga nordiska tabellavsnitt i dessa bilder. Sparat som kompletteringsspår; inga importerade flyg. |
| [Air UK, Winter 1985/86 Issue 1](https://www.ebay.com/itm/206570442564) | Tillgängligt omslagsfoto | Tabellsidor behövs, särskilt Norge och Danmark samt ändringsblad. |
| [Dan-Air inflight magazine, Winter 1985/86](https://www.danairremembered.com/1985.php) | Omslag samt s. 50–52 | Dessa slutblad innehåller telefon-/ombordinformation och reklam, inte avgångstider. Hela magasinet har inte lästs. |

`nordic_priority.json` innehåller nu 29 prioriterade operatörer, inklusive Air UK och brittiska Dan-Air. Dan-Air är skilt från danska Danair. LOT har ändrats till `scan_found`, men inga egna avgångar har ännu importerats. Braathens returflyg samt svenska, danska, finska och isländska systemtabeller återstår fortfarande.

## Spårbarhet och kontroll

`batch_13.json` anger nya rad-ID:n, bild-/PDF-URL:er och SHA-256. Källbilderna distribueras inte i ZIP-paketet. Det genererade landregistret och den svenska/engelska spelinformationen är uppdaterade. Källregistret har 32 poster och operatörsregistret 77 poster; inget land eller bolag är färdiginventerat.

Valideringen kontrollerar oförändrade tidigare rörelser, 21 nya fredags- och åtta lördagsrörelser, unika fysiska sträckor, UTC-omräkning och undantagna anslutningar. Unreal-kompilering och spelkörning har inte utförts i denna miljö.
