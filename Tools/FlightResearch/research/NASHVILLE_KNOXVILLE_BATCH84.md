# Nashville och Knoxville – omgång 84 / v87

Kontrollerad 2026-09-30. Kumulativ fortsättning från v86.

## Resultat

**72 nya planerade flygrörelser** från 38 nya nonstop-scheman: 38 Delta och 34 COMAIR. Totalt **5 431 rörelser**, varav 5 430 tidtabellslagda och en tidigare dokumenterat genomförd.

| Förbindelse, båda riktningar | Nya scheman | Nya rörelser |
|---|---:|---:|
| Nashville–Cincinnati | 11 | 17 |
| Nashville–Knoxville | 3 | 6 |
| Knoxville–Cincinnati | 9 | 17 |
| Knoxville–Atlanta | 15 | 32 |
| **Totalt** | **38** | **72** |

Knoxville–McGhee Tyson (TYS) tillkommer som flygplatsmarkör. Databasen omfattar 248 platser och 1 084 riktade ändpunktspar. Delta har 1 369 rörelser och COMAIR 335. Av 95 registrerade operatörer har 49 rörelser (48 civila samt US Marine Corps), medan 46 saknar rörelser. Inget bolag är verifierat fullständigt; någon global täckningsprocent kan inte anges.

Samtliga 5 359 tidigare rörelse-ID:n behålls. 5 357 tidigare rörelseobjekt och 3 325 tidigare schemarader är helt oförändrade. En tidigare schemarads flygnummer och anteckningar rättas, vilket ändrar motsvarande fält på två rörelser. Alla tidigare flygtider och flygplatsposter bevaras.

## Originalkälla och urval

[Delta, tidtabell giltig från 1 februari 1986, Digital Library of Georgia](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201). [Original-PDF](https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf), 136 PDF-sidor. Skanningen återdistribueras inte.

| Tryckta sidor | PDF-sidor | Innehåll |
|---|---|---|
| 4 | 5 | Frekvenser och operatörsnyckel |
| 16 | 11 | Atlanta–Knoxville |
| 50–51 | 28 | Cincinnati–Knoxville/Nashville samt kontroll av äldre Detroit-rad |
| 121 | 63 | Knoxville–Atlanta/Cincinnati |
| 123 | 64 | Knoxville–Nashville |
| 166–167 | 86 | Nashville–Cincinnati/Knoxville |
| 263–265 | 134–135 | Kompletterande kontroll av flygnummer och linjeföljder |

Åtta kompletta valda riktningstabeller omfattar **67 rader**. Av dessa importeras 38 nonstoprader. 27 anslutningsrader och två genomgående rader med ett stopp sparas enbart som forskning. Detta är ett urval av nätet, inte en komplett genomgång av varje berörd stad eller operatör.

Samtliga accepterade Delta-rader har tomt frekvensfält och går dagligen enligt källan. COMAIR identifieras genom nummerserien 1525–1749 i operatörsnyckeln. COMAIR-raderna använder tomt fält (dagligen), X6 (utom lördag), X7 (utom söndag), X67 (måndag–fredag) eller 6 (endast lördag). Ingen accepterad rad har en daterad fotnot.

## Veckodagar, tider och delsträckor

COMAIR 1619 Knoxville–Cincinnati har två separata rader: **13:25–14:35 måndag–fredag** och **13:25–14:38 på lördag**. Båda bevaras med olika schema-ID:n. Ingen söndagstrafik står i den granskade tabellen. Veckodagar följer varje fysisk delsträckas egen rad; exempelvis har den tidigare Cincinnati–Milwaukee-delen av 1619 en annan veckodagsbegränsning.

Förstorade originalrader bekräftar bland annat COMAIR 1646 Nashville–Cincinnati **19:10–21:25**, 1628 Cincinnati–Nashville **15:05–15:15**, 1725 Knoxville–Cincinnati ankomst **07:36** samt 1571 Cincinnati–Knoxville **15:20–16:30**. Delta 948 Atlanta–Knoxville avgår **08:49**, och 758 avgår **23:04**.

Nashville använder UTC−6. Knoxville, Cincinnati och Atlanta använder UTC−5 under perioden. Därför kan det lokala ankomstklockslaget till Nashville vara tidigare än avgångsklockslaget. Delta 903 Knoxville–Nashville **16:16–15:58** tar 42 minuter. COMAIR 1600 Cincinnati–Nashville **09:22–09:22** tar 60 minuter. Alla accepterade flyg anländer samma lokala kalenderdag.

Fönstret är **1986-02-27 22:21:30 UTC till 1986-03-01 22:21:30 UTC**. Intervallöverlapp avgör vilka rörelser som visas. Tre dagliga scheman ger därför tre rörelser vardera: COMAIR 1716 TYS–CVG, Delta 1144 TYS–ATL och Delta 319 ATL–TYS. Flygens fulla tider behålls. De nya rörelsernas lokala avgångsdatum fördelas 13 den 27 februari, 37 den 28 februari och 22 den 1 mars.

25 övergångar mellan separat källbelagda ben med samma flygnummer har kontrollerats för den 28 februari. Delta 1144 Dallas/Fort Worth–Nashville–Knoxville–Atlanta har 20 minuter i både Nashville och Knoxville. Delta 903 Atlanta–Knoxville–Nashville–Dallas/Fort Worth har samma uppehåll. COMAIR 1592 St Louis–Cincinnati–Knoxville har 25 minuter i Cincinnati. Detta kontrollerar tidtabellens linjeföljd och belägger inte en viss flygplansindivid.

## Rättelse av tidigare flygnummer

Den tidigare COMAIR-raden Cincinnati–Detroit **09:31–10:41**, importerad i omgång 72, hade felaktigt flygnummer 1592. Förstorad stadstabell på tryckt s. 50 visar **1582**. Linjeförteckningen på s. 264 stöder detta: 1582 går RIC–ROA–CVG–DTW, medan 1592 går STL–CVG–TYS.

Rättelsen ändrar flygnummer och anteckningar i ett schema och två rörelser. ID:n, teknisk servicegrupp, veckodagar och samtliga tider behålls. Därför innehåller det äldre stabila ID:t fortfarande 1592, medan flygnummerfältet och visningstexten anger 1582. Ingen rörelse tas bort eller läggs till på grund av rättelsen. Exakt redovisning finns i `errata_batch84.json`. Äldre forskningsfiler behålls som historik och den angivna raden ersätts av denna rättelse.

## Forskning utan import

Den uteslutna lördagsanslutningen Cincinnati–Nashville 1679/615 på s. 51 anger ankomst **12:29**, medan den separat granskade fysiska delsträckan Lexington–Nashville 615 på s. 129 anger **12:28**. Den tidigare nonstopraden behålls; anslutningen skapar inga rörelser och motiverar ingen ändring av delsträckan.

Två uteslutna anslutningar, Nashville–Cincinnati 787/1140 och Knoxville–Cincinnati 540/1140, anländer **00:10 nästa dag**. Nästa-dagsflaggan finns i forskningsfilen. Delta 983 och COMAIR 1623 Cincinnati–Nashville har ett mellanstopp och importeras inte som nonstopflyg. Deras tidigare separat granskade delsträckor via Lexington bevaras.

Den tidigare tidskonflikten Delta 597 Memphis–Little Rock från omgång 80 och flygnummerkonflikten 1871/1671 Columbus–Louisville från omgång 82 är oförändrade. Ingen av dessa hållna rader importeras i v87.

## Knoxville som historisk markör

[Flygplatsmyndighetens översikt av 2006 års masterplan](https://flyknoxville.com/wp-content/uploads/2024/11/MasterPlanExec.pdf), tryckt s. 2–3, beskriver inköpet av nuvarande flygplatsområde 1935 och skiljer det från den tidigare flygplatsen i West Knoxville. Markören avser McGhee Tyson, inte den äldre platsen eller Downtown Island Airport.

[OurAirports](https://ourairports.com/airports/KTYS/) anger 35.811001 / −83.994003. Koordinaten används som en ungefärlig områdesmarkör och rekonstruerar inte terminal, gate, rullbana eller flygplansposition för 1986. Tidszon: America/New_York.

## Validering och nästa steg

En separat transkription med 24-timmarsklockor, explicita veckodagar och fasta UTC-offsetar matchar alla 72 nya rörelser. Tidigare rörelser, scheman och flygplatser jämförs fält för fält med redovisat undantag för flygnummerrättelsen. Inga nya tidsöverlapp finns för berörda bolags- och flygnummer. Alla 20 Python-tester och aktualitetskontrollerna passerar. Körresultaten finns i `validation_batch84.json` och `automated_checks_batch84.json`.

UE-kompilering, spelkörning och paketerad build har inte utförts här. Paketet innehåller data och forskning för befintlig flygmeny med tidigare kompilerad PaintAirplane-rättelse i f23b73a. C++-kod ingår inte.

Nästa prioritet är delsträckor Chattanooga–Knoxville/Lexington, återstående COMAIR-förbindelser kring Nashville/Cincinnati och Buffalo samt saknade ben genom Jackson och Shreveport. Memphis–LaGuardia kvarstår. Europakön återupptas vid Cypern när den prioriteras igen.

Tidtabellerna belägger planerad trafik, inte faktiskt genomförande, passagerare, last eller verkliga flygvägar. Senare tidtabellsändringar och utgåvans slutdatum är inte verifierade.
