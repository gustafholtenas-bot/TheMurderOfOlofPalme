# COMAIR Buffalo, Akron–Canton och Evansville – omgång 86 / v89

Kontrollerad 2026-09-30. Kumulativ fortsättning från v88.

## Resultat

**35 nya planerade COMAIR-rörelser** från 20 nya nonstop-scheman. Totalt **5 508 rörelser**, varav 5 507 tidtabellslagda och en tidigare dokumenterat genomförd. Alla 5 473 tidigare rörelse-ID:n bevaras; 5 472 rörelseobjekt och 3 385 äldre scheman är helt oförändrade. Ett äldre schema och dess enda rörelse får rättat flygnummer, med oförändrade tider och trafikdagar.

| Förbindelse | Nya scheman | Nya rörelser |
|---|---:|---:|
| Buffalo–Cincinnati, båda riktningar | 4 | 7 |
| Akron–Canton–Cincinnati, båda riktningar | 6 | 11 |
| Evansville–Cincinnati, båda riktningar | 9 | 16 |
| Evansville → Indianapolis | 1 | 1 |
| **Totalt** | **20** | **35** |

Tre flygplatser tillkommer: BUF, CAK och EVV. Databasen omfattar 252 platser och 1 097 riktade ändpunktspar. COMAIR har 381 rörelser; Delta har fortsatt 1 400. Av 95 registrerade operatörer har 49 rörelser (48 civila samt US Marine Corps), medan 46 saknar rörelser. Inget bolag är verifierat fullständigt. Ingen global täckningsprocent kan anges.

## Originalkälla och urval

[Delta, tidtabell giltig från 1 februari 1986, Digital Library of Georgia](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201). [Original-PDF](https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf), 136 PDF-sidor. Skanningen återdistribueras inte.

| Tryckta sidor | PDF-sidor | Innehåll |
|---|---|---|
| 4 | 5 | Frekvenser och operatörsnyckel |
| 6 | 6 | Akron–Canton–Cincinnati |
| 36 | 21 | Buffalo–Cincinnati |
| 48 | 27 | Rättelse Chicago O’Hare–Milwaukee |
| 49 | 27 | Cincinnati–Akron–Canton/Buffalo |
| 50 | 28 | Cincinnati–Evansville |
| 79 | 42 | Evansville–Cincinnati/Indianapolis |
| 111 | 58 | Indianapolis–Evansville, forskning utan import |
| 264–265 | 135 | Kontroll av flygnummer och linjeföljder |

Åtta kompletta valda riktningstabeller omfattar **30 rader**: 20 nonstoprader importeras, medan nio anslutningsrader och en genomgående rad med ett stopp sparas enbart som forskning. Detta är ett urval av nätet, inte en fullständig inventering av städerna eller bolaget.

COMAIR identifieras genom nummerserien 1525–1749 i operatörsnyckeln. Frekvenserna är X6 (utom lördag), X7 (utom söndag), X67 (måndag–fredag) och X56 (utom fredag och lördag). Ingen accepterad rad har en daterad fotnot.

## Tider, kalender och delsträckor

Evansville använder UTC−6 under perioden. Buffalo, Akron–Canton, Cincinnati och Indianapolis använder UTC−5. Alla accepterade ankomster är på samma lokala kalenderdag som avgången. Cincinnati–Evansville 1552, **17:00–16:59 lokal tid**, tar därför 59 minuter. Evansville–Indianapolis 1563, **06:00–07:40**, tar 40 minuter.

Förstorade rader bekräftar Buffalo–Cincinnati **1660**, Cincinnati–Akron–Canton 1530 **09:22–10:27** och Evansville–Cincinnati 1553 **17:10–19:09**. Cincinnati–Evansville 1595 **19:55–19:55** har X56. Den ger endast en torsdagsrörelse den 27 februari i det aktuella fönstret, trots att föregående Knoxville–Cincinnati har andra trafikdagar.

Fönstret är **1986-02-27 22:21:30 UTC till 1986-03-01 22:21:30 UTC**. Rörelser tas med vid intervallöverlapp, med hela flygtider bevarade. De nya rörelsernas lokala avgångsdatum är nio den 27 februari, 19 den 28 februari och sju den 1 mars. Inget nytt schema ger tre rörelser.

Tretton övergångar mellan separat källbelagda ben med samma flygnummer har kontrollerats, bland annat Indianapolis–Cincinnati–Buffalo (35 minuter), Evansville–Cincinnati–Akron–Canton (31 minuter) och Evansville–Indianapolis–Milwaukee (20 minuter). Evansville–Cincinnati–Detroit 1594 och Knoxville–Cincinnati–Evansville 1595 kontrolleras den 27 februari, då båda delarna har trafik. Varje ben behåller sina egna veckodagar. Kontrollerna belägger tidtabellens linjeföljd, inte en viss flygplansindivid.

Evansville–Indianapolis 1527 är en genomgående rad med ett stopp i Cincinnati. Den skapar ingen extra nonstoprörelse; dess fysiska ben är Evansville–Cincinnati och det tidigare importerade Cincinnati–Indianapolis. Indianapolis–Evansville-tabellens sex rader är samtliga anslutningar. Ingen nonstopretur skapas.

## Rättelse och bevarande

Chicago O’Hare–Milwaukee **11:15–11:50** rättas från flygnummer **1530 till 1580**. Stadstabellen på s. 48 och linjeförteckningen på s. 264 visar båda 1580 för ORD–MKE; 1530 avser CMH–CVG–CAK. Endast flygnummer och anmärkningar ändras i ett schema och en rörelse. Stabila ID:n, servicegrupp, tider och trafikdagar behålls. Därför innehåller det stabila ID:t fortfarande texten `1530`.

`errata_batch86.json` ersätter den felaktiga etiketten i första dataraden i `comair_toronto_stl_chicago_batch73.tsv` och äldre rapporthänvisningar till den. Historiska forskningsfiler bevaras som revisionsspår.

V87:s rättelse till COMAIR 1582 Cincinnati–Detroit bevaras. Delta 597 Memphis–Little Rock från omgång 80 och 1871/1671 Columbus–Louisville från omgång 82 är fortsatt hållna källkonflikter och importeras inte.

## Historiska flygplatsmarkörer

[Buffalos flygplatsmyndighet](https://www.buffaloairport.com/airport-info/celebrating-100-years) beskriver platsen i Cheektowaga, öppnad 1926, och namnet Greater Buffalo International från 1959. Namnet Buffalo Niagara och den senare terminalen är från 1997. [OurAirports BUF](https://ourairports.com/airports/KBUF/) anger 42.940498 / −78.732201.

[Akron–Cantons flygplatsmyndighet](https://www.akroncantonairport.com/home/business/about-cak/history/) beskriver den invigda platsen 1946 och flygbolagens flytt från den separata Akron Municipal 1948. CAK ska inte förväxlas med Akron Fulton/AKC. [OurAirports CAK](https://ourairports.com/airports/KCAK/) anger 40.916100 / −81.442200.

[Evansvilles masterplan, introduktion juli 2023, utkast, s. 1-4–1-5](https://s3.us-east-1.amazonaws.com/s3.flyevv.com/documents/master-plan/evv-airport-master-plan-Introduction.pdf) beskriver flygplatsen från 1928 längs US41 i norra Vanderburgh County. Den senare tiogateterminalen från 1988 projiceras inte tillbaka till 1986. [OurAirports EVV](https://ourairports.com/airports/KEVV/) anger 38.036999 / −87.532402.

Koordinaterna är ungefärliga markörer för flygplatsområdena. De rekonstruerar inte terminaler, gater, rullbanor eller flygplanspositioner för 1986.

## Validering och fortsättning

En separat transkription med 24-timmarsklockor, explicita veckodagar och fasta UTC-offsetar matchar alla 35 nya rörelser. Äldre poster jämförs fält för fält med endast den dokumenterade flygnummerrättelsen tillåten. Inga nya tidsöverlapp finns för berörda bolags- och flygnummer. Alla 20 Python-tester och aktualitetskontroller passerar. Körresultaten redovisas i `automated_checks_batch86.json`; detaljer finns i `validation_batch86.json`.

UE-kompilering, spelkörning och paketerad build har inte utförts här. Paketet innehåller data och forskning för befintlig flygmeny med tidigare kompilerad PaintAirplane-rättelse i f23b73a.

Nästa prioritet är återstående fysiska delsträckor kring Buffalo/Akron/Evansville, saknade Jackson/Shreveport-ben, Memphis–LaGuardia och genomgående Delta-ben kring Chattanooga/Atlanta. Europakön behåller Cypern.

Tidtabellen beskriver planerad trafik. Faktiskt genomförande, passagerare, last, flygvägar, senare ändringar och utgåvans slutdatum är inte verifierade.
