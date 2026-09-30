# Köpenhamn, Stansted och Kanalöarna – omgång 24 / v25

Granskat 2026-09-28. **90 nya planerade flygrörelser** från **62 granskade nonstoprader**. Totalt **1 649 rörelser**, varav **155 med nordisk ändpunkt**. Alla **1 559 tidigare rörelseposter** är exakt oförändrade.

## Källor och avgränsning

[Air France utgåva 25](https://www.airtimes.com/cgat/fr/airfrance/pdf/1a/af851027-1a.pdf) och [Air UK vinterutgåva 1](https://www.airtimes.com/cgat/uk/klmuk/pdf/uk851027-1a.pdf) gäller 27 oktober 1985–29 mars 1986. AF har 55 PDF-sidor; hela sidföljden är inte helgranskad. UK har 13 PDF-sidor; tryckta s. 16–17 och 24–25 saknas. Källorna behåller därför status `partial_scan`.

AF-rader har pil i VIA-kolumnen, vilket betyder nonstop enligt s. 4. UK-rader har 0 i stoppkolumnen enligt s. 3. Tider är lokala och dagar 1–7 betyder måndag–söndag. Importen begränsas till 28 februari–1 mars 1986. Samtliga nya avgångar och ankomster ligger inom spelets 48-timmarsfönster. Tidtabellen belägger planerad trafik; senare ändringar och faktisk operation är inte verifierade.

| Urval | Tryckta importsidor | Nya rader | Nya rörelser |
|---|---|---:|---:|
| AF/SK Köpenhamn–Nice/Lyon | 26, 43, 56 | 4 | 6 |
| Air UK Stansted | 6, 8, 14, 19 | 20 | 24 |
| Air UK Southampton–Amsterdam | 7, 19, 20 | 4 | 6 |
| Air UK Exeter–Guernsey–Jersey–Southampton/retur | 10, 12, 13, 14, 20 | 6 | 12 |
| Air UK Guernsey–Heathrow | 12, 15 | 8 | 14 |
| Air UK Guernsey–Southampton | 12, 20 | 10 | 12 |
| Air UK Jersey–Southampton | 14, 20 | 10 | 16 |
| Totalt | | 62 | 90 |

24 riktade platspar tillkommer; totalt finns 463. Full transkription och datumundantag finns i `nordic_airuk_batch24.tsv`. Källhashar och nya rörelse-ID:n finns i `batch_24.json`.

## Sex nordiska rörelser

| Tryckt flygkod | Fysisk sträcka, lokal tid | Datum | Kataloggrupp |
|---|---|---|---|
| SKAF575 | CPH 12.10 → NCE 14.30 | 28 februari och 1 mars | SAS |
| AFSK576 | NCE 16.00 → CPH 18.20 | 28 februari och 1 mars | Air France |
| AFSK1790 | LYS 15.10 → CPH 17.15 | 28 februari | Air France |
| SKAF1791 | CPH 18.20 → LYS 20.25 | 28 februari | SAS |

Dubbelkoderna betyder samarbete enligt AF:s s. 4. Första tryckta kod styr kataloggrupp; båda koderna bevaras i respektive källnot. **Faktisk operatör är inte fastställd.** Varje avgång räknas en gång, utan en parallell partnerpost. De tidigare SAS- och AF-rörelserna är oförändrade.

AFSK1790:s Marseille–Lyon och SKAF1791:s Lyon–Marseille saknar separata mellantider. Genomgående Marseille–Köpenhamn 13.55–17.15 och Köpenhamn–Marseille 18.20–21.50 blir därför inga extra nonstoprörelser. Tryckta s. 42–45 är granskade men löser inte dessa mellantider.

Nordiska ändpunktsrörelser totalt: Sverige 87, Norge 47, Danmark 84, Finland 4, Island 0. Landtalen överlappar och ska inte summeras. Inga faktiska överflygningar har lagts till.

## Säsongsfotnoter vid månadsskiftet

De tryckta veckodagarna bevaras, med `excluded_dates` för undantag inom importperioden. Inga uppgifter om faktiskt inställda flyg skapas från säsongsuppehåll.

| Flyg | 28 februari | 1 mars | Skäl |
|---|---|---|---|
| UK130/131 | Ingår | Ingår | Lördagstrafiken återupptas 1 mars |
| UK134/135 | Ingår | Ingår | Fredagstrafiken gäller från 1 februari; lördag saknar motsvarande undantag |
| UK180/181 | Ingår | Utesluts | Lördagar endast 2 och 9 november |
| UK184/185 | Ingen fredagsrad | Ingår | Tryckta dagar 1,3,6,7; lördag undantas inte |
| UK188/189 | Utesluts | Ingår | Dagar 1,5,6,7 återupptas 1 mars |
| UK440/441 | Ingår | Ingår | Lördagar undantas i januari/februari |
| UK450/443 | Utesluts | Ingår | Fredagar endast november/mars; lördagsundantaget för UK450 slutar efter februari |
| UK444/451 | Ingår | Ingår | Fredagsundantaget gäller januari |
| UK446/447 | Ingår | Utesluts | Lördagstrafik endast till 30 november |
| UK371/372 | Ingår | Ingår | Tisdag/torsdag-undantag slutar 27 februari; lördagsundantag slutar 8 februari |

UK454/455/456/457 gäller endast till 10 november och importeras inte. UK373/374 går söndagar och faller utanför urvalet. UK184/185:s övriga fotnotsdagar behöver inte extrapoleras eftersom importen är begränsad till dessa två datum.

## Tidsatta delsträckor och markuppehåll

| Flyg | Fysiska delsträckor, lokal tid | Markuppehåll |
|---|---|---|
| UK371, fredag/lördag | SOU 11.55 → JER 12.40; JER 12.55 → GCI 13.10; GCI 13.20 → EXT 14.00 | Jersey 15 min; Guernsey 10 min |
| UK372, fredag/lördag | EXT 14.25 → GCI 15.05; GCI 15.15 → JER 15.30; JER 15.50 → SOU 16.35 | Guernsey 10 min; Jersey 20 min |
| UK214, fredag | STN 07.00 → LBA 07.50; tidigare LBA 08.00 → EDI 08.55 | Leeds/Bradford 10 min |
| UK217, fredag | Tidigare EDI 19.30 → LBA 20.25; LBA 20.35 → STN 21.25 | Leeds/Bradford 10 min |

Totalt tio markuppehåll kontrollerade, inklusive två kopplingar till tidigare importerade delsträckor. UK371/372:s separata nollstoppsrader styr animationen. Några genomgående resrader anger exempelvis Exeter 14.30, Southampton 16.30 eller Southampton 12.00; dessa avvikelser är dokumenterade och skapar inga alternativa avgångar. Flygsymbolen försvinner under markuppehållen.

UK645 Stansted–Charles de Gaulle är 10.10–12.25 lokal tid, 75 minuter efter tidszonsomräkning. Korta Kanalöflyg har låga genomsnittshastigheter beräknade över hela blocktiden; detta är inte uppmätt marschfart.

## Flygplatser

Fem nya punkter: Lyon–Satolas (LYS), London–Stansted (STN), Southampton–Eastleigh (SOU), Exeter (EXT) och Guernsey (GCI). Guernsey läggs till som eget territorium enligt samma struktur som Jersey. Totalt finns 164 flygplatser/platser och 71 länder/territorier.

Satolas namnges i AF-tabellen, Eastleigh i UK-tabellen. Historiska kontrollkällor och koordinatfilens hash finns i `airport_locations_batch24.json`. Flygfältskoordinaterna är ungefärliga; de är inte rekonstruerade terminal-, gate- eller flygplanspositioner från 1986. Flygbanorna är storcirkelillustrationer.

## Fortsatt forskning

SAS/Finnair-sökningar gav inga ytterligare kompletta avgångssidor denna omgång. [KLM:s AirTimes-galleri](https://www.airtimes.com/cgat/nl/klm/gal/klgal1a80.htm) gav inga PDF-länkar. Tidigare källspår kvarstår. Prioritet: egna SAS/Finnair-vinterutgåvor, AY873/874:s mellanlandningar, SK563 GOT–CPH, SK570 GOT–FBU, SK594/410:s förhållande till SK674 samt de saknade Marseille–Lyon-tiderna. För Air UK återstår saknade uppslag och senare ändringar. Ingen av källorna är färdiginventerad.

## Validering

20 befintliga Python-tester passerade. Genererad flyg-JSON, landindex och börsdata är kontrollerade. Alla 1 559 gamla rörelser och 22 skyddade kod-/börsfiler är oförändrade. Nya rörelser har positiva, rimliga restider och inga fysiska dubbletter eller samtidiga överlapp för samma bolag/flygnummer. Samarbetsraderna har dessutom kontrollerats för partnerdubbletter. Fjorton kontroller jämför säsongsfotnoternas förväntade datum med genererade rörelser. Tio markuppehåll är kontrollerade. De tio äldre kandidatraderna är oförändrade och animeras inte.

Egen flygtrafikmeny och roterande flygplanssilhuett ingår fortsatt. Ingen C++-ändring tillkommer i v25. Unreal-kompilering, Editor-körning och paketerad spelbuild är inte utförda här. Det maskinläsbara resultatet finns i `validation_batch24.json`.
