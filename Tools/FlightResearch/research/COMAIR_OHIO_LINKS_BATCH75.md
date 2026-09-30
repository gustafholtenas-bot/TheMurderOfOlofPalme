# COMAIR – fler Ohio-förbindelser, v76 / omgång 75

33 nya planerade COMAIR-rörelser: Dayton–Detroit, Cleveland–Milwaukee och Cleveland–Fort Wayne med retur, samt Detroit–Cleveland. 26 nya scheman från 60 granskade rader. Totalt 4 819 rörelser. Alla 4 786 tidigare rörelseposter, scheman och flygplatser är oförändrade. COMAIR har nu 182 rörelser. Delta är oförändrat med 910. Menyn, flygplansikonen och spelkoden bevaras.

26 godkända COMAIR-scheman på sju nya riktade platspar. Tre flygplatspar har separat granskade tider i båda riktningarna; Detroit–Cleveland tillkommer som en riktning utan härledd retur.

| Från | Till | Nya rörelser |
|---|---|---:|
| CLE | FWA | 4 |
| CLE | MKE | 6 |
| DAY | DTW | 5 |
| DTW | CLE | 2 |
| DTW | DAY | 8 |
| FWA | CLE | 3 |
| MKE | CLE | 5 |

## Kalender och tidskontroll

60 visuellt granskade rader: 26 nonstop-scheman, 30 anslutningar och fyra rader med mellanstopp. Samtliga accepterade rader är COMAIR enligt legend s. 4, har stoppkolumn 0 och saknar daterad fotnot. Frekvenserna är X67, X7 eller X6; X betyder utom och 1–7 är måndag–söndag.

MKE–CLE 1683 är 07:15–09:35; flygnumret är 1683. CLE–MKE 1693 är 17:30–17:55. DTW–DAY 1631 är 17:15–18:19. DAY–DTW 1572 går X67, trots att föregående CVG–DAY-rad med samma nummer går X7. Benens dagar kontrolleras separat.

Milwaukee använder UTC−6; Cleveland, Dayton, Detroit och Fort Wayne UTC−5. Alla ankomster sker samma lokala dag. Perioden är exakt 1986-02-27T22:21:30Z–1986-03-01T22:21:30Z; hela flygtider bevaras vid intervallöverlapp. Lokala avgångar: 1986-02-27: 6, 1986-02-28: 26, 1986-03-01: 1.

## Mellanstopp och forskningsrättelse

Nya DAY–DTW 1629, 1572 och 1643 ansluter till tidigare separat belagda CVG–DAY-ben. Nya CLE–MKE 1575 kompletterar CVG–DAY–CLE med samma nummer. DTW–DAY 1714 och 1559 fortsätter enligt tidigare DAY–CVG-rader. DTW–CLE 1603 föregår tidigare CLE–CVG. Andra kontrollerade exempel finns i valideringsfilen. Delsträckorna är lästa direkt ur tidtabellen; de härleds inte ur en genomgående rad. Servicegrupperna slås inte automatiskt samman.

Tabellerna mellan Dayton och Milwaukee samt Dayton och Fort Wayne ger inga nya nonstop-scheman i denna granskning. Anslutningar via CVG/CLE/IND och genomgående rader behålls i TSV-filen med sina uteslutningsbeslut. Detta är ingen fullständighetsförklaring för samtliga bolag på sträckorna.

En äldre utesluten forskningsrad rättas genom denna anmärkning: omgång 72, rad 30, MKE–CVG 1589/1655 via Indianapolis har avgång 13:25 och ankomst 16:40. Avgången transkriberades tidigare som 12:25. Den gamla forskningsfilen bevaras som historik; rättelsen påverkar inget importerat schema eller någon rörelse. Även den nu granskade anslutningen MKE–CLE 1589/1655 avgår 13:25 och utesluts.

Tidtabellen belägger planerad trafik, inte genomförande, förseningar, flygplansindivider eller verklig flygbana. Utgåvans slutdatum och senare ändringar är inte fastställda. Tidszonskartan och lokaltidskonvention används; separat all-times-local-text har inte återfunnits. Äldre otydliga DL475/705 kvarstår utanför importen.

239 flygplatser och 1000 riktade platspar. Norden är oförändrat: 161 unika ändpunktsrörelser (Sverige 87, Norge 52, Danmark 85, Finland 4, Island 0). Europakön behåller Cypern. 95 registrerade operatörer är ingen fullständig global inventering. Nästa prioritet är Indianapolis, Columbus och fler Cincinnati-destinationer eller Deltas Dallas/Fort Worth.

## Källor och validering

- https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201 — Original från Delta Flight Museum, utgåva 1 februari 1986.
- https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf — 136 PDF-sidor. Tryckta scheman s. 53, 67, 68, 73, 88 och 154; legend s. 4, vinterkarta PDF 2.

Originalskanningar omdistribueras inte. comair_ohio_source_evidence_batch75.json innehåller adresser, SHA-256 och sidmappning; comair_ohio_links_batch75.tsv varje granskad rad och beslut. validation_batch75.json redovisar oberoende kalender/UTC-kontroll, dubblettkontroll, kontroll mot tidigare flygnummer och bevarande av äldre data samt 20 Python-tester och fyra datakontroller. Unreal-kompilering och Editor-körning har inte utförts.
