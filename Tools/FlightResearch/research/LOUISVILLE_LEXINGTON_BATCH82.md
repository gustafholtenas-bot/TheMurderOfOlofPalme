# Louisville och Lexington – omgång 82 / v85

Kontrollerad 2026-09-30. Kumulativ fortsättning från v84.

## Resultat

**58 nya planerade flygrörelser** från 36 nya nonstop-scheman: 26 Delta och 32 COMAIR. Totalt **5 281 rörelser**, varav 5 280 tidtabellslagda och en tidigare dokumenterat genomförd. Alla 5 223 äldre rörelseobjekt och 3 254 äldre schemarader är oförändrade.

| Förbindelse, båda riktningar | Nya scheman | Nya rörelser |
|---|---:|---:|
| Louisville–Chicago O’Hare | 5 | 10 |
| Louisville–Detroit | 10 | 14 |
| Louisville–Columbus | 4 | 4 |
| Louisville–Lexington | 2 | 4 |
| Lexington–Cincinnati | 15 | 26 |
| **Totalt** | **36** | **58** |

LEX tillkommer som flygplatsmarkör. Databasen omfattar nu 246 platser och 1 066 riktade ändpunktspar. Delta har 1 255 rörelser, COMAIR 299. Av 95 registrerade operatörer har 49 rörelser (48 civila samt US Marine Corps), 46 saknar rörelser. Inget bolag är verifierat fullständigt; världens totala antal operatörer är inte fastställt.

## Originalkälla och avgränsning

[Deltas tidtabell, giltig från 1 februari 1986, Digital Library of Georgia](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201). [Original-PDF](https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf), 136 PDF-sidor. Originalskanningen återdistribueras inte.

| Tryckta sidor | PDF-sidor | Innehåll |
|---|---|---|
| 4 | 5 | Frekvens- och operatörsnyckel |
| 48, 51 | 27, 28 | Chicago–Louisville och Cincinnati–Lexington |
| 62–63 | 34 | Columbus–Louisville och stöd för flygnummerkonflikten |
| 73–74 | 39–40 | Detroit–Louisville samt definition av fotnot 4 |
| 128–129 | 67 | Lexington–Cincinnati och Lexington–Louisville |
| 138–139 | 72 | Louisville–Chicago/Detroit/Columbus/Lexington |
| 262–265 | 134–135 | Flygnummer och linjeföljder som kompletterande kontroll |

Tio hela valda stadsriktnings-tabeller innehåller 62 rader: 36 importeras, 23 anslutningsrader och två genomgående enstoppsrader utesluts som direktsträckor, en rad hålls för flygnummerkonflikt. Samtliga accepterade rader har stoppkolumn 0. Ingen accepterad rad har daterad fotnot. Varje ben behåller sina egna tryckta veckodagar.

Tomt frekvensfält betyder dagligen. X6 utesluter lördag, X7 söndag och X67 lördag/söndag. X56 på uteslutna anslutningar betyder ej fredag/lördag. En ensam 6 betyder endast lördag: COMAIR1679 Cincinnati–Lexington ger en rörelse den 1 mars. De två uteslutna Detroit–Louisville-anslutningarna med första flygnummer 619 har fotnot 4, giltig från 12 februari, definierad på tryckt s. 73.

Operatörsnyckeln identifierar 1525–1749 som COMAIR. Numren 292, 956, 311, 493, 541, 1172, 1051, 520, 288, 983, 497, 419 och 475 i de accepterade raderna tillhör Delta.

## Flygnummer som hålls utanför

Columbus–Louisville, s. 62: **14:50–15:40, X6, stopp 0** har tryckt nummer **1871**. På samma sida visar anslutningen Columbus–Dallas/Fort Worth och på s. 63 Columbus–Shreveport samma avgång via Louisville med **1671/1051**. Itinerariet på s. 265 anger COMAIR1671 CMH–SDF men Ransome1871 BOS–BTV. Operatörsnyckeln skiljer också dessa nummerserier åt.

Detta talar för ett tryckfel, men inget nummer eller bolag gissas i databasen. Raden finns bara i forskningen tills en daterad rättelse eller oberoende tidtabell kan styrka flygnummer och tider. Den tidigare konflikten Delta597 Memphis–Little Rock från omgång 80 är fortsatt oförändrad och hålls utanför.

SDF–DTW1564 och254 är genomgående resor med ett stopp i Cincinnati. Deras fysiska delsträckor finns redan separat och importeras inte på nytt.

## Flygplats och tid

LEX använder **Lexington–Blue Grass Airport (1986)**, 38.035066 / −84.606738. [Flygplatsens egen masterplan, tryckt s. 1-3–1-4](https://www.bluegrassairport.com/wp-content/uploads/2025/08/MasterPlan_Chapter1_DIGITAL.pdf) beskriver den etablerade platsen vid Versailles Road och namnbytet från Blue Grass Field 1984. Terminalen öppnade 1976. [OurAirports](https://ourairports.com/airports/KLEX/) ger en modern koordinat som ungefärlig markör för området; den är ingen exakt gate-, ban- eller flygplansposition för 1986.

Chicago är UTC−6. Louisville, Detroit, Columbus, Lexington och Cincinnati är UTC−5 under perioden. Lexington använder zonen America/New_York. Alla accepterade ankomster sker samma lokala kalenderdag. Louisville–Chicago292 och956 tar 62 minuter trots att de lokala klockslagen bara skiljer två minuter.

Flygfönstret är **1986-02-27 22:21:30 UTC till 1986-03-01 22:21:30 UTC**. Hela avgångs- och ankomsttider behålls när flyget överlappar en fönstergräns. De 58 nya rörelserna har lokala avgångsdatum fördelade 10 den 27 februari, 35 den 28 februari och 13 den 1 mars.

## Validering och leverans

En separat transkription med 24-timmarsklockor, explicita veckodagar och fasta UTC-offsetar matchar varje ny rörelses UTC-tider. Tidigare rörelseobjekt och scheman jämförs fält för fält. Inga nya tidsöverlapp har hittats bland berörda bolags- och flygnummer. Femton övergångar mellan separat källbelagda delsträckor har kontrollerade markuppehåll den 28 februari; det belägger inte flygplansindivid.

De 20 befintliga Python-testerna och aktualitetskontrollerna för flygdata, landindex och Europaindex passerar. Körresultaten finns i `validation_batch82.json` och `automated_checks_batch82.json`. UE-kompilering, spelkörning och paketerad build har inte utförts här.

Paketet är en datauppdatering för befintlig flygmeny och tidigare kompilerad PaintAirplane-rättelse i f23b73a. C++-kod ingår inte. Källa, råtranskription, radbeslut, konflikt, flygplatsgranskning och validering finns i respektive `*batch82*`-fil.

Tidtabellen belägger planerad trafik, inte faktiskt genomförande, passagerare, last eller verkliga flygvägar. Senare tidtabellsändringar och utgåvans slutdatum är inte verifierade.

Nästa prioritet: Lexington–Atlanta och Lexington–Detroit, Louisville–Dallas/Fort Worth samt saknade fysiska ben genom Jackson och Shreveport. Memphis–LaGuardia kvarstår. Europakön återupptas vid Cypern när den prioriteras igen.
