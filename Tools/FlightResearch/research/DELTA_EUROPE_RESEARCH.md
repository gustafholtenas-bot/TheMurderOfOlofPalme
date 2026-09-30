# Omgång 10 / paket v11 – Delta och kompletterade Europasträckor

Granskad 26 september 2026. **45 nya godkända tidtabellsrader ger 92 nya planerade rörelser** i fönstret 27 februari 22:21:30–1 mars 22:21:30 UTC: Delta 80 och Pan Am 12. Totalt 924 rörelser, varav 923 planerade och en tidigare dokumenterad militär transport. Delta är nu den femtonde operatören med importerade rörelser. Ingen ny militär rörelsehandling har tillkommit.

## Delta – källa och avgränsning

[Delta Air Lines System Timetable, effective February 1, 1986](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201), Delta Flight Museum / Digital Library of Georgia. [Full PDF](https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf). PDF:en har 136 sidor. Den tryckta sidnumreringen skiljer sig från PDF-numreringen.

| Tryckt sida | PDF-sida | Granskat och importerat urval |
|---|---:|---|
| 4 | 5 | Teckenförklaring: 1=måndag, 7=söndag, X=utom; om ingen dagkod finns gäller daglig trafik. |
| 14–15 | 10 | Atlanta till Boston, Chicago O’Hare och Frankfurt. |
| 16–17 | 11 | Atlanta till Gatwick, Orly, Los Angeles, Miami, Minneapolis, San Francisco och Seattle. |
| 90 | 48 | Frankfurt till Atlanta. |
| 133 | 69 | Gatwick till Atlanta. |
| 187 | 96 | Orly till Atlanta. |

Endast rader med **0 stopp** har importerats. PDF-renderingar är visuellt lästa; OCR användes för att hitta rätt avsnitt och som stöd, inte som ensam grund för import. Transkriptionen finns i `delta_batch10.tsv`. Källfilens SHA-256 finns i `batch_10.json`.

De sex internationella tidtabellsraderna är DL14/15 Atlanta–Frankfurt, DL10/11 Atlanta–Gatwick och DL20/21 Atlanta–Orly. Dessa ger tio rörelser inom tidsfönstret. De 33 utvalda inrikesraderna gäller avgångar från Atlanta; deras returtrafik är ännu inte transkriberad.

### Datum, flygplatser och undantag

- Parisrubriken anger uttryckligen **Orly**, Londonrubriken **Gatwick**. Flygplatserna blandas inte ihop med CDG eller Heathrow. Atlanta/Hartsfield och Orly är nya platsmarkörer i denna omgång.
- DL10 går utom måndag; DL11 utom tisdag. DL20 går utom tisdag/torsdag/söndag; DL21 utom måndag/onsdag/fredag. Dagkoden hör till den lokala avgångsdagen, inte ankomstdagen i Europa.
- DL455 Atlanta–Miami går endast lördag/söndag. Endast lördagen 1 mars finns därför i fönstret.
- DL181 Atlanta–Los Angeles har fotnot 6 på s. 16–17: start **2 mars**. Den är inte importerad. DL20:s lördagsavgång ligger också utanför fönstret, eftersom den börjar 1 mars 22:35 UTC.
- Stjärnan anger ett priserbjudande vid tidiga eller sena flygtider; den gör inte ett flyg till charter eller en särskild faktisk rörelse.
- Rader märkta med Ontario eller Fort Lauderdale räknas inte som LAX respektive Miami. Anslutningar, osplittrade enstoppsrader, Delta Connection och LH-kodade flyg har inte importerats i denna omgång.
- Källan är uttryckligen giltig från 1 februari. Utgåvans slutdatum är inte fastställt; granskningsintervallet i de importerade raderna är därför begränsat till 26 februari–1 mars. Sökningen efter en ersättningsutgåva gav inget verifierat slutdatum; avsaknad av en träff är inte bevis för att inga ändringar fanns. Framtida fotnoter i själva utgåvan har kontrollerats för urvalet.

## Pan Am – sex nya fysiska delsträckor

[Pan Am, 11 februari–26 april 1986](https://digitalcollections.library.miami.edu/digital/collection/asm0341/id/37370/), University of Miami Libraries. Sex rader ger tolv nya rörelser. `panam_batch10.tsv` innehåller tider och lokala trafikdagar.

| Tryckt sida | Bild-ID | Delsträckor |
|---|---:|---|
| 3 | 37236 | PA98 Amsterdam–Hamburg, PA99 Amsterdam–Heathrow |
| 19 | 37252 | PA125 Bryssel–Heathrow |
| 20 | 37253 | PA102 Bryssel–Nürnberg |
| 44 | 37277 | PA99 Hamburg–Amsterdam |
| 46 | 37279 | PA101 Hamburg–Heathrow |

Exakt URL och kontrollsumma för varje bild finns i `batch_10.json`. Nummerändringarna från 6 mars och tidsändringarna från 30 mars används inte för mordhelgen. PA101:s dagliga Hamburg–Heathrow-rad gäller genom 2 mars. Bryssel–Heathrow och Amsterdam–Heathrow har lokala passagerarrestriktioner, som behålls i de svenska och engelska anmärkningarna.

Elva redan importerade rader får uppdaterad resegruppering. Deras tider, flygnummer, status och ID:n bevaras. PA125 binds ihop Berlin–Nürnberg–Bryssel–Heathrow, med Seattle–San Francisco på fredagen och Heathrow–San Francisco direkt på lördagen. PA102 binds ihop JFK–Heathrow–Bryssel–Nürnberg–Berlin; Europasträckorna hör till dagen efter JFK-avgången. PA98, PA99 och PA101 binds också ihop över sina granskade stopp. Samma flygnummer belägger inte att samma flygplan användes hela resan.

Istanbul-radernas tidigare tidszonskonflikt är fortfarande olöst. De förblir kandidater utan animation.

## Fortsatt kö

| Bolag/land | Nästa steg |
|---|---|
| Delta / USA | Läs returavgångarna till Atlanta från de sju importerade inrikesdestinationerna; därefter fler Atlanta-destinationer och Dallas/Fort Worth. |
| Lufthansa / Västtyskland | Kontrollera LH418/438/419/439 i Delta-tabellen och helst mot Lufthansa-utgåvan. Dessa är inte Delta-flyg och får inte räknas som sådana. |
| Pan Am / USA, Europa | Komplettera PA98:s första Atlantsträcka och fortsätt med olästa nonstoprader i den stora utgåvan. |
| SAS, Finnair och övriga bolag | Fortsätt vintertidtabellskön i landregistret. Ett lands markerade bolag är inte en fullständig inventering. |

Koordinater och historiska flygplatskällor finns i `airport_locations_batch10.json`. Animationen använder schematiska flygfältslägen och storcirklar, inte rekonstruerade flygvägar eller terminaler från 1986. Inga originalskanningar redistribueras.
