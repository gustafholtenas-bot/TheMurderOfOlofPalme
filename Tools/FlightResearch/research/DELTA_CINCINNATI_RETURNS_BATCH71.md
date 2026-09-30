# Delta – returer till Cincinnati, v72 / omgång 71

54 nya planerade rörelser från 24 scheman på tolv nya riktade platspar. Totalt 4 624 rörelser: 4 623 scheduled och en tidigare confirmed. Delta har nu 897 rörelser. Alla 4 570 tidigare rörelseposter, scheman och flygplatser är oförändrade. Menyn, flygplansikonen och spelkoden bevaras.

87 visuellt granskade rader: 24 nya nonstop-scheman, 52 anslutningar, tre rader med mellanstopp och åtta COMAIR-rader som väntar på import under rätt operatör. Varje returtid är läst separat i källan.

| Från | Till | Nya rörelser |
|---|---|---:|
| BOS | CVG | 6 |
| DFW | CVG | 4 |
| DTW | CVG | 5 |
| LAX | CVG | 3 |
| LGA | CVG | 9 |
| MCO | CVG | 3 |
| MIA | CVG | 2 |
| MSY | CVG | 2 |
| ORD | CVG | 9 |
| PHL | CVG | 5 |
| SFO | CVG | 1 |
| TPA | CVG | 5 |

## Frekvenser, fotnoter och tidskontroll

BOS–CVG DL849 06:40–08:48 och SFO–CVG DL850 12:05–19:05 har fotnot 7: trafik från 1 mars. Endast lördagens avgångar importeras. SFO-flyget landar efter fönstrets slut, men avgår före det och ingår därför med hela sin flygtid. Fotnot 4 för DTW–CVG DL619 och TPA–CVG DL680 betyder start 12 februari.

BOS–CVG DL457 har X6 (utom lördag). PHL–CVG DL505 och LGA–CVG DL809 har X7 (utom söndag). Övriga accepterade rader är dagliga. Samtliga accepterade ankomster är samma lokala kalenderdag. LGA-flyget 10:30–12:18 är DL419. Los Angeles-flyget DL810 är 12:20–19:03.

CVG och de östliga avgångsflygplatserna använder UTC−5; ORD/DFW/MSY UTC−6; LAX/SFO UTC−8. Perioden är exakt 1986-02-27T22:21:30Z–1986-03-01T22:21:30Z. Hela flygtider bevaras vid intervallöverlapp. Lokala avgångar: 27 februari 12, 28 februari 22, 1 mars 20. Flygtider 51–240 minuter.

## Avgränsning och fortsättning

MCO94/324 är en anslutning som enligt fotnot 14 körs sista gången 1 mars. Den ersättande anslutningen MCO158/324 börjar 2 mars enligt fotnot 6. LAX68/1140 börjar också 2 mars. Ingen av dessa tre anslutningar importeras. ONT–CVG DL498, MIA–CVG DL806 och MSY–CVG DL1110 har ett mellanstopp och importeras inte som obrutna sträckor. Mellanliggande delsträckor härleds inte.

De åtta COMAIR-raderna från Detroit sparas med operatören comair i forskningsfilen. Tillsammans med tio uppskjutna COMAIR-rader i föregående omgång ger det 18 granskade rader i båda riktningarna för separat import under rätt operatör. Vissa är genomgående med mellanstopp och kan inte importeras direkt som nonstop. Nästa steg kan också vara fler destinationer från Cincinnati eller Dallas/Fort Worth.

New York-tabellens L betyder LaGuardia; Los Angeles-tabellens L/O betyder LAX/Ontario; M/F i Florida-tabellen betyder Miami/Fort Lauderdale. Alla använda flygplatser finns sedan tidigare. De tolv utresesträckorna i v71 har nu minst ett separat källbelagt returschema, men ingen fullständighet för bolaget hävdas.

Tidtabellen belägger planerad trafik, inte genomförande, förseningar, individflygplan eller verklig flygbana. Utgåvans slutdatum och senare ändringar är inte fastställda. Tidszonskartan och lokaltidskonvention används; separat all-times-local-text har inte återfunnits. Servicegrupper mellan omgångar har inte slagits samman. Otydliga DL475/705 från v64 hålls fortsatt utanför.

236 flygplatser och 975 riktade platspar. Norden oförändrat: 161 unika ändpunktsrörelser (Sverige 87, Norge 52, Danmark 85, Finland 4, Island 0). Europakön behåller Cypern. Registret med 94 operatörer är ingen fullständig global inventering.

## Källor och validering

- https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201 — Digital Library of Georgia, original från Delta Flight Museum, utgåva 1 februari 1986.
- https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf — 136 PDF-sidor. Tryckta s. 31, 47, 64, 73, 135, 150, 170, 173, 183, 191, 215 och 230. Fotnoter s. 31/73/135/183/215/231; legend s. 4; vinterkarta PDF 2.

Originalskanningar omdistribueras inte. delta_source_evidence_batch71.json innehåller källadresser, SHA-256 och sidmappning. delta_cincinnati_returns_batch71.tsv innehåller varje granskad rad och beslut. validation_batch71.json redovisar oberoende kalender/UTC-kontroll, dubblettkontroll, bevarande av äldre data, 20 Python-tester och fyra datakontroller. Unreal-kompilering och Editor-körning har inte utförts.
