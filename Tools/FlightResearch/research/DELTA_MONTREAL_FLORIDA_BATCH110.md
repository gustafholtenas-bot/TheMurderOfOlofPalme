# Delta – Montreal, Bangor och Florida, v113 / batch110

Granskad 1 oktober 2026. Kumulativ datauppdatering från v112.

195 nya planerade Delta-rörelser från 94 granskade nonstop-scheman på 44 nya riktade flygplatspar. Montreal–Dorval/YUL och Bangor/BGR tillkommer. Fler linjer mellan New York/Boston och Florida, Montreal till/från Boston och Miami, Washington till/från Memphis och New Orleans samt separat tidsatta lördagsben mellan Miami och Fort Lauderdale. Totalt **7 915 rörelser**, varav 7 914 tidtabellslagda och en dokumenterat genomförd. Alla 7 720 äldre rörelseobjekt och 4 552 äldre scheman är oförändrade. Se `research/DELTA_MONTREAL_FLORIDA_BATCH110.md`.

## Sträckor

Alla 44 riktade ändpunktspar är nya i databasen. Varje rad avser endast den angivna riktningen. Samma stadsrubrik kan innehålla flera separata flygplatser.

| Riktad sträcka | Nya scheman | Nya rörelser |
|---|---:|---:|
| BGR → BOS | 4 | 8 |
| BOS → BGR | 4 | 9 |
| BOS → FLL | 3 | 7 |
| BOS → MCO | 3 | 7 |
| BOS → MIA | 2 | 4 |
| BOS → PBI | 2 | 5 |
| BOS → TPA | 2 | 5 |
| BOS → YUL | 5 | 10 |
| DCA → MEM | 3 | 6 |
| DCA → MSY | 1 | 2 |
| EWR → FLL | 2 | 4 |
| EWR → MCO | 1 | 2 |
| FLL → BOS | 3 | 7 |
| FLL → EWR | 2 | 4 |
| FLL → JFK | 4 | 8 |
| FLL → LGA | 3 | 7 |
| FLL → MIA | 1 | 1 |
| JFK → FLL | 4 | 9 |
| JFK → MCO | 1 | 3 |
| JFK → TPA | 1 | 2 |
| LGA → FLL | 3 | 6 |
| LGA → MCO | 1 | 2 |
| LGA → MSY | 1 | 3 |
| LGA → PBI | 3 | 7 |
| LGA → PWM | 2 | 4 |
| LGA → TPA | 1 | 2 |
| MCO → BOS | 2 | 4 |
| MCO → EWR | 1 | 2 |
| MCO → JFK | 1 | 2 |
| MCO → LGA | 1 | 2 |
| MEM → DCA | 2 | 4 |
| MIA → BOS | 2 | 5 |
| MIA → FLL | 1 | 1 |
| MIA → YUL | 2 | 2 |
| MSY → DCA | 1 | 2 |
| MSY → LGA | 1 | 2 |
| PBI → BOS | 2 | 4 |
| PBI → LGA | 3 | 6 |
| PWM → LGA | 2 | 4 |
| TPA → BOS | 2 | 5 |
| TPA → JFK | 1 | 3 |
| TPA → LGA | 1 | 2 |
| YUL → BOS | 5 | 9 |
| YUL → MIA | 2 | 2 |
| **Totalt** | **94** | **195** |

## Källa och urval

[Delta Air Lines systemtidtabell från 1 februari 1986, Digital Library of Georgia](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201). Tryckta sidor **24, 31–34, 81, 83, 148, 150–152, 163, 172, 174–176, 182, 184, 198, 230–231, 246 och 248–249** har granskats visuellt. Operatörs- och symbolnyckeln samt flygplatsförteckningen på s.260–261 används. PDF-hash och sidmappning finns i `delta_montreal_florida_source_evidence_batch110.json`.

Alla 94 importerade rader anger **0 stopp**. Urvalet omfattar endast Delta mainline. Genomgående rader med stopp och anslutningsförslag importeras inte som ytterligare nonstop-flyg. Originalets separata Fort Lauderdale-rubriker och samlingsrubrikerna Miami/Fort Lauderdale kan återge samma avgång; den räknas då endast en gång.

Skannade PDF-uppslag har varierande bredd. Hela relevant rad, inklusive högra trafikdags- och fotnotskolumnen, granskas mot sidans faktiska gränser. Textutvinning används endast som stöd när skanningen är tät eller fetstil sammanflyter. Original-PDF och nya sidbilder ingår inte i paketet.

Utgåvans slutdatum och senare ändringar är inte verifierade. Tidtabellen beskriver planerad trafik, inte bevisat genomförande, individflygplan eller verkliga flygbanor. Generella flygplanssymboler används inte för att tillskriva varje avgång en viss typ eller registrering.

## Fotnoter och trafikdagar

Sex accepterade rader har **fotnot 4, gäller från 12 februari 1986**: **JFK–FLL 721, FLL–JFK 720, BOS–FLL 189, FLL–BOS 594, BOS–PBI 891 och PBI–BOS 818**. Alla börjar före importperioden 27 februari–1 mars. Fotnoten och dess startdatum finns uttryckligen i revisionsunderlaget. Inga andra daterade fotnoter finns på de accepterade raderna.

- **YUL–MIA 215** går **08:00–11:15 utom lördag**, men **08:30–11:45 på lördag**. Dessa är två veckodagsvarianter, inte två samtidiga dagliga flyg.
- **MIA–YUL 214** går utom lördag. **MIA–YUL 468** går lördag **14:30–17:33**.
- Lördagens fortsättning **MIA–FLL 215 12:15–12:40** har en egen nonstop-rad. Returen **FLL–MIA 468 13:35–13:56** har också en egen rad, följd av MIA–YUL. Den genomgående YUL–FLL-raden med stopp räknas inte som ett extra flyg.
- **MSY–LGA 216** går dagligen, medan fortsättningen **LGA–PWM 216** undantar lördag. Varje delsträcka behåller sina egna dagar.
- **BGR–BOS 339** undantar lördag medan **BOS–PHL 339** från v112 går dagligen. Samma princip gäller här.
- Flygnummer **494** förekommer i båda riktningarna **LGA–PBI 08:00–10:39** och **PBI–LGA 19:55–22:21**. De har separata rader och överlappar inte i tid.

**J=JFK, L=LGA och E=EWR** skiljer New York-flygplatserna. **N=Washington National/DCA**. **M=Miami/MIA och F=Fort Lauderdale/FLL** i samlingsrubriker. Montreal-rubriken avser **Dorval/YUL**, inte Mirabel. Stjärnan avser lågtrafikpris, inte datumskifte. D kodar tomt frekvensfält/dagligen; X undantar angivna dagar, 1=måndag och 7=söndag.

Exakta minuter bevaras: **BOS–FLL 189 19:55–22:56**, **BOS–PBI 891 15:59–18:55**, **LGA–PWM 216 21:29–22:25**, **MCO–JFK 328 12:01–14:15** och **BOS–BGR 562 08:57–09:45**. Den sena FLL–BOS-avgången är flyg **136**, medan 138 är ett annat befintligt flygnummer.

## Tidszoner och 48-timmarsfönster

Memphis och New Orleans använder **UTC−6**, övriga nya ändpunkter **UTC−5** under importdatumen. Det ger exempelvis **DCA–MEM 733 08:00–09:03 lokal tid** ett UTC-intervall **13:00–15:03**, alltså 2 timmar 3 minuter. Lokal klockskillnad används inte som flygtid. Inga nytillagda ben passerar lokal midnatt.

Fönstret är **1986-02-27 22:21:30 UTC till 1986-03-01 22:21:30 UTC**. Hela flygintervallet behålls när det överlappar fönstret. **BOS–BGR 356 16:52–17:40 lokal tid** överlappar både starten på torsdagen och slutet på lördagen och ger därmed tre rörelser.

Alla 94 scheman ger minst en rörelse: 7 ger en, 73 ger två och 14 ger tre. Lokala avgångsdatum ger 38 rörelser den 27 februari, 90 den 28 februari och 67 den 1 mars.

## Nya flygplatsmarkörer

| Kod | Historiskt namn i tidtabellen | Ungefärlig markör | Underlag |
|---|---|---|---|
| BGR | Bangor International | 44.8074444, −68.8281389 | Tidtabell s.260; FAA-referenspunkt återgiven av AirNav |
| YUL | Montréal–Dorval International | 45.4680556, −73.7413889 | Tidtabell s.261; NAV CANADA:s moderna aerodromkoordinat |

[AirNav BGR](https://www.airnav.com/airport/BGR) återger uppskattade FAA-koordinater vid det namngivna flygfältet och aktiveringsmetadata före 1986. Aktiveringsfältet används inte som ett exakt historiskt invigningsdatum.

[NAV CANADA:s tjänstemeddelande](https://www.navcanada.ca/en/flight-planning/service-notices/2026-05-14-montreal.aspx) anger en aerodromkoordinat för dagens CYUL som här används enbart som ungefärlig markör. [Montreals kommunala historik](https://ville.montreal.qc.ca/memoiresdesmontrealais/laeroport-montreal-trudeau) dokumenterar Dorval före 1986. Det senare Trudeau-namnet förs inte tillbaka till 1986.

Koordinaterna rekonstruerar inte 1986 års terminaler, uppställningsplatser, bantrösklar eller flygplanspositioner. Fulla källor och avgränsningar finns i `airport_locations_batch110.json`. Kanada fanns redan som land; inga nya länder eller bolag tillkommer.

## Validering och bevarande

Separat manuell 24-timmarsinmatning av **94 rader** jämförs med tolkningen av originaltiderna. En oberoende kalenderberäkning med fasta vintertidsförskjutningar stämmer med **alla 195 nya UTC-intervall**. **31 markuppehåll** mellan separat tidsatta ben har granskats; samtliga har matchande rörelser i fönstret. Detta visar tidtabellsmässig kontinuitet, inte vilket individflygplan som användes.

Äldre och nya rörelser för berörda operatör/flygnummer jämförs där minst ett ben är nytillagt; inga tidsöverlapp hittas. Den tidigare hållna konflikten för Delta 597 MEM–LIT lämnas kvar, även när PWM–LGA 597 nu kompletterar det tidigare LGA–MEM-benet.

Alla **7 720 äldre rörelseobjekt**, **4 552 scheman**, **315 flygplatser**, **82 länder** och **98 operatörer** är oförändrade. Endast Delta-källposten får kompletterande granskningsnoteringar. Tidigare rättelser och tre hållna källkonflikter bevaras. Verktyg och äldre forskningsfiler är oförändrade utom dokumenterade aktuella index och statusfiler.

Alla **20 Python-tester** och kontrollerna av genererad flygdata, landindex och Europaindex passerar. Exakta testutdata finns i `automated_checks_batch110.json`. Unreal-kompilering, Editor och paketerat spel har inte körts. Paketet förutsätter redan kompilerad PaintAirplane-rättelse från `f23b73a`.

## Fortsatt arbete

Fler Delta-linjer kring Hartford, Florida och de stora sjöarna återstår att granska. Tidigare hållna källkonflikter behöver oberoende underlag innan de kan lösas. Bolagsnät och världsinventering är fortsatt ofullständiga. Europeiska landkön återupptas vid Cypern.
