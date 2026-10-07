# Delta Jackson och Shreveport – omgång 87 / v90

Kontrollerad 2026-09-30. Kumulativ fortsättning från v89.

## Resultat

**67 nya planerade Delta-rörelser** från 33 nya nonstop-scheman. Totalt **5 575 rörelser**, varav 5 574 tidtabellslagda och en tidigare dokumenterat genomförd. Alla 5 508 tidigare rörelseobjekt, 3 406 tidigare scheman och samtliga flygplatsuppgifter bevaras oförändrade.

| Förbindelse, båda riktningar | Nya scheman | Nya rörelser |
|---|---:|---:|
| Jackson–Atlanta | 10 | 21 |
| Jackson–Dallas/Fort Worth | 7 | 14 |
| Shreveport–Atlanta | 3 | 6 |
| Shreveport–Dallas/Fort Worth | 11 | 22 |
| Jackson–Shreveport | 2 | 4 |
| **Totalt** | **33** | **67** |

Databasen omfattar 252 platser och 1 107 riktade ändpunktspar. Delta har 1 467 rörelser och COMAIR fortsatt 381. Av 95 registrerade operatörer har 49 rörelser (48 civila samt US Marine Corps), medan 46 saknar rörelser. Inget bolag är verifierat fullständigt och ingen global täckningsprocent kan anges.

## Originalkälla och urval

[Delta, tidtabell giltig från 1 februari 1986, Digital Library of Georgia](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201). [Original-PDF](https://dlg.galileo.usg.edu/data/delta/dal-tt/pdfs/delta_dal-tt_dal-tt-19860201.pdf), 136 PDF-sidor. Skanningen återdistribueras inte.

| Tryckta sidor | PDF-sidor | Innehåll |
|---|---|---|
| 4 | 5 | Frekvensnyckel |
| 15 | 10 | Atlanta–Jackson |
| 17 | 11 | Atlanta–Shreveport |
| 65 | 35 | Dallas/Fort Worth–Jackson |
| 66 | 36 | Dallas/Fort Worth–Shreveport |
| 112 | 59 | Jackson–Atlanta |
| 113 | 59 | Jackson–Dallas/Fort Worth |
| 115 | 60 | Jackson–Shreveport |
| 224 | 115 | Shreveport–Atlanta |
| 225 | 115 | Shreveport–Dallas/Fort Worth/Jackson |
| 262–264 | 134–135 | Kontroll av flygnummer och linjeföljder |

Tio kompletta valda riktningstabeller omfattar **51 rader**: 33 nonstoprader importeras, medan 16 genomgående rader med ett stopp och två anslutningsrader sparas enbart som forskning. Varje fysisk delsträcka har ett eget källbelägg. Detta är ett urval av nätet, inte en fullständig genomgång av bolaget.

Jackson–Atlanta 844 och Atlanta–Shreveport 833 har X6 (utom lördag). Atlanta–Jackson 357 har X7 (utom söndag). Övriga accepterade rader är dagliga, med tomt frekvensfält. Ingen accepterad rad har en daterad fotnot.

## Nattflyg, tidszoner och periodgränser

Jackson, Shreveport och Dallas/Fort Worth använder UTC−6 i perioden; Atlanta använder UTC−5. Jackson–Shreveport 531 avgår **23:40** och anländer **00:24 nästa lokala kalenderdag**. Flygtiden är 44 minuter och `arrival_day_offset` är 1. De importerade avgångarna är den 27 och 28 februari, med ankomst den 28 februari respektive 1 mars.

Atlanta–Jackson 531 avgår **23:12 Eastern** och anländer **23:15 Central**, vilket motsvarar 63 minuter. Därefter följer Jackson–Shreveport med 25 minuter i Jackson. Den genomgående Atlanta–Shreveport-raden har ett stopp och skapar ingen extra nonstoprörelse. Även den uteslutna anslutningen Shreveport–Atlanta 931/386 anländer nästa dag; detta är dokumenterat i forskningsfilen.

Fönstret är **1986-02-27 22:21:30 UTC till 1986-03-01 22:21:30 UTC**. Rörelser tas med vid intervallöverlapp, med hela flygtider bevarade. Atlanta–Jackson 1117 går **16:51 Eastern–17:00 Central**, alltså **21:51–23:00 UTC**. Den överlappar fönstrets början den 27 februari och dess slut den 1 mars, och ger därför tre rörelser. Övriga 32 nya scheman ger två rörelser vardera.

De nya rörelsernas lokala avgångsdatum fördelas på 15 den 27 februari, 33 den 28 februari och 19 den 1 mars.

## Förstorad källkontroll och linjeföljder

Jackson–Dallas/Fort Worth 941 har **ett stopp**, vilket bekräftas av linjeförteckningen ATL–JAN–MLU–DFW–AUS. Den raden importeras inte som nonstop. De fysiska benen via Monroe behöver egna tider i en senare omgång. Jackson–Dallas/Fort Worth 557 har ankomst **17:49**, verifierad i förstoring.

Shreveport–Atlanta 537 har däremot **noll stopp** och tiden **17:25–19:45**. Linjeförteckningen ORD–MEM–SHV–ATL–AGS stöder den fysiska direktsträckan. Även 1088 Shreveport–Atlanta **10:15–12:35** är nonstop.

Sju övergångar mellan separat källbelagda ben har kontrollerats:

| Flyg | Övergång | Marktid | Kontrollerade avgångsdatum |
|---|---|---:|---|
| 248 | Shreveport–Jackson–Atlanta | 26 min | 28 feb, 1 mars |
| 248 | Jackson–Atlanta–Philadelphia | 49 min | 28 feb, 1 mars |
| 531 | Atlanta–Jackson–Shreveport | 25 min | 27 och 28 feb |
| 1051 | Louisville–Dallas/Fort Worth–Shreveport | 41 min | 28 feb |

Tidtabellens linjeföljd är kontrollerad; den belägger inte en viss flygplansindivid. Tidigare importerade fortsättningar skapas inte en gång till.

## Bevarande och validering

Tidigare rättelser till COMAIR 1582 Cincinnati–Detroit och COMAIR 1580 Chicago O’Hare–Milwaukee bevaras oförändrade, inklusive stabila ID:n. De hållna konflikterna Delta 597 Memphis–Little Rock från omgång 80 och 1871/1671 Columbus–Louisville från omgång 82 kvarstår och importeras inte.

Befintliga flygplatsmarkörer och historiska platsbedömningar för JAN, SHV, ATL och DFW används oförändrade. Inga nya flygplatser, koordinater eller bolag läggs till.

En separat transkription med 24-timmarsklockor, explicita veckodagar, ankomstdagar och fasta UTC-offsetar matchar alla 67 nya rörelser. Alla äldre rörelser, scheman och flygplatser jämförs fält för fält. Inga nya tidsöverlapp finns för berörda bolags- och flygnummer. Alla 20 Python-tester och aktualitetskontroller passerar. Körresultaten redovisas i `automated_checks_batch87.json`; detaljer finns i `validation_batch87.json`.

UE-kompilering, spelkörning och paketerad build har inte utförts här. Paketet innehåller data och forskning för befintlig flygmeny med tidigare kompilerad PaintAirplane-rättelse i f23b73a.

Nästa prioritet är saknade fysiska ben via Monroe på bland annat 941, 1036, 471, 584, 662 och 583, därefter Birmingham/Mobile-ben kring Jackson/Shreveport och Memphis–LaGuardia. Europakön behåller Cypern.

Tidtabellen beskriver planerad trafik. Faktiskt genomförande, passagerare, last, flygvägar, senare ändringar och utgåvans slutdatum är inte verifierade.
