# Indianapolis och Columbus – v79 / forskningsomgång 76

64 nya planerade flygrörelser: 52 COMAIR och 12 Delta. 43 nya nonstop-scheman från 62 granskade rader. Totalt 4 883 rörelser. Indianapolis och Port Columbus tillkommer som flygplatser. Alla 4 819 tidigare rörelseposter och kodrättelserna från v77/v78 bevaras.

| Förbindelse | Nya planerade rörelser, båda riktningar |
|---|---:|
| Cincinnati–Columbus | 30 |
| Cincinnati–Indianapolis | 22 |
| Indianapolis–Milwaukee | 10 |
| Indianapolis–Columbus | 2 |
| **Totalt** | **64** |

## Källgranskning

Original: Delta Air Lines System Timetable, effective February 1, 1986,
Delta Flight Museum / Digital Library of Georgia.

- Arkivpost: https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201
- PDF: https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf
- Tryckta avgångssidor: 50, 61, 62, 110, 111 och 154.
- Teckenförklaring och operatörer: s. 4. Flygnummer 1525–1749 är COMAIR.
- Flygplatsnamn/koder: s. 260. Tidszonskarta: PDF-sida 2.

Alla 62 rader i de åtta utvalda riktningstabellerna har granskats mot bilderna.
43 nonstop-scheman godtas (37 COMAIR, sex Delta). 16 anslutningar, en genomgående
rad med ett stopp och två söndagsvarianter utan trafik under de aktuella dagarna
hålls utanför importen. Detta är ett urval, inte hela bolagens trafik.

## Kalender, klockslag och fysiska delsträckor

- Perioden är 1986-02-27 22:21:30 UTC till 1986-03-01 22:21:30 UTC.
- Cincinnati, Columbus och Indianapolis använder UTC−5 under dessa datum;
  Milwaukee UTC−6. Alla accepterade ankomster sker samma lokala kalenderdag.
- CVG–IND DL1140 avgår 00:35 och landar 01:05. Ingen tolvslagstolkning på dagen.
- IND–CVG 1547, 1574 och 1699 är fredagsflyg: det tryckta 5 är veckodag,
  inte en fotnot. Stjärnan vid 1699 betyder off-peak-prissättning.
- CVG–CMH 1542 har X7-varianten 15:00 och söndagsvarianten 15:01. Bara X7-raden
  importeras. CVG–IND 1568 är också en söndagsrad som inte importeras.
- IND–CVG 1734 går X6; fortsättningen CVG–CMH 1734 går X56. Fredagens ben till
  Cincinnati skapar därför ingen antagen fortsättning till Columbus.
- CVG–IND 1567 är 12:45–13:30. Flygnumret har lästs i förstoring och kontrollerats
  mot separat tryckta IND–MKE 1567, 13:55–14:10. Benen grupperas inte automatiskt.
- MKE–IND 1589, 13:25–15:40, följs av separat IND–CMH 1589, 16:00–16:59,
  och CMH–CVG 1589, 17:10–17:50. Ingen direkt MKE–CVG-rad härleds ur detta.
- IND–MKE 1568 den 27 februari avgår före periodens start men är då i luften;
  hela flygtiden behålls. Sena lördagsavgångar efter periodens slut utesluts.

Nya rörelser per lokal avgångsdag: 27 februari 14, 28 februari 42, 1 mars 8.
Veckodagar och UTC-tider är kontrollerade separat med fasta vinteroffsetar mot
byggverktygets historiska IANA-tidszoner. Ingen returriktning är härledd.

## Flygplatser och täckning

IND registreras som Indianapolis International och CMH som Port Columbus
International, enligt 1986 års flygplatsförteckning. Dagens John Glenn-namn
används inte för CMH. Koordinater avser schematiska flygplatslägen, inte belagda
1986-terminaler, gates eller uppställningsplatser.

Koordinater: https://ourairports.com/airports/KIND/ och
https://ourairports.com/airports/KCMH/ . Namnhistorik för Columbus:
https://flycolumbus.com/business/history/ .

Paketet har 241 flygplatser/platser och 1008 riktade platspar. COMAIR har 234
rörelser och Delta 922. Norden behåller 161 unika ändpunktsrörelser. Fortfarande
95 registrerade operatörer: 49 med rörelser (48 civila + US Marine Corps), 46 utan.
Inget bolag är verifierat fullständigt; global täckningsprocent kan inte anges.

Tidtabellen belägger planerad trafik, inte faktiskt genomförande, förseningar,
passagerare, last, individflygplan eller verklig flygbana. Utgåvans slutdatum
och senare ändringar är inte verifierade. Originalskanningar omdistribueras inte.

## Filer och fortsatt kö

- `comair_indianapolis_columbus_batch76.tsv`: varje rad, trafikdagar och beslut.
- `comair_indianapolis_source_evidence_batch76.json`: källor, checksumma, sidmappning.
- `batch_76.json` och `validation_batch76.json`: nya ID:n, antal och kontroller.
- `WORLDWIDE_COVERAGE_BATCH76.md`: uppdaterad bolagsinventering.

Nästa användbara urval är Cleveland–Columbus samt Indianapolis–Cleveland/Detroit,
fler Cincinnati-destinationer eller Deltas Dallas/Fort Worth. Europakön behåller
Cypern; ofullständiga nordiska vintertidtabeller behöver fortsatt källarbete.

Spelkoden är identisk med v78, inklusive båda krasch-/kompileringsrättelserna.
Full UE 5.8-kompilering och Editor-körning har inte utförts här.

Validering: 20 Python-tester och fyra datakontroller passerar. Alla 16 källkodsfiler från v78 är oförändrade. Inga överlappande delsträckor med samma operatör/flygnummer hittades för de nya rörelserna.
