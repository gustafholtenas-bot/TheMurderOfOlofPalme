# Air France och SAS i Norden – omgång 23 / paket v24

Granskat 2026-09-28. **92 nya planerade flygrörelser**, samtliga med nordisk ändpunkt. Totalt **1 559 rörelser**, varav **149 nordiska**. Alla **1 467 tidigare rörelseposter** är exakt oförändrade.

## Källa och läsning

[Air France utgåva 25](https://www.airtimes.com/cgat/fr/airfrance/pdf/1a/af851027-1a.pdf) gäller 27 oktober 1985–29 mars 1986. Den faktiska PDF-länken hittades i [AirTimes galleri](https://www.airtimes.com/cgat/fr/airfrance/gal/afgal1a80.htm). PDF:en har 55 sidor. Arkivet märker den komplett, men hela sidföljden är inte helgranskad och källan behandlas konservativt som partiell.

Omslaget, förklaringar på s. 2–4, flygplats- och resvägsförklaringar på s. 8–10 samt följande importsidor har granskats visuellt. PDF-sida betyder sidans ordningsnummer i filen.

| Tryckt sida | PDF-sida | Importerat urval |
|---|---:|---|
| 26 | 14 | Köpenhamn till Arlanda och Paris |
| 35 | 18 | Landvetter till Paris |
| 36 | 19 | Vantaa till Arlanda |
| 59 | 31 | Fornebu till Landvetter och Paris |
| 64 | 34 | Paris till Köpenhamn |
| 67 | 35 | Paris till Landvetter |
| 73 | 38 | Paris till Fornebu |
| 76 | 40 | Paris till Arlanda |
| 87 | 45 | Arlanda till Köpenhamn |
| 88 | 46 | Arlanda till Paris och Vantaa |

Pil i VIA-kolumnen betyder nonstop. Rader med via-flygplats, genomgående restid eller utan tillräckliga mellanlandningsuppgifter blir inte direktflyg. Dagar 1–7 betyder måndag–söndag. Alla tider är lokala. Importen begränsas till 26 februari–1 mars; byggaren räknar endast flyg vars tidsintervall överlappar 27 februari 22.21.30–1 mars 22.21.30 UTC. Senare ändringar och faktiskt genomförande är inte verifierade.

## Nytillskott

| Flygkod/bolag | Nya tidtabellsrader | Nya rörelser |
|---|---:|---:|
| SK / SAS | 43 | 70 |
| AF / Air France | 11 | 22 |
| Totalt | 54 | 92 |

13 nya riktade platspar tillkommer. Alla nya rörelser har minst en nordisk ändpunkt. Källans flygkod anger bolag i registret; eventuell inhyrd faktisk operatör är inte belagd. Full transkription finns i `air_france_nordic_batch23.tsv`; filhash, nya rörelse-ID:n och platspar finns i `batch_23.json`.

| Land som start eller mål | Totalt i v24 |
|---|---:|
| Sverige | 87 |
| Norge | 47 |
| Danmark | 78 |
| Finland | 4 |
| Island | 0 |

Landtalen överlappar. De fyra rörelserna Arlanda–Vantaa/retur är paketets första finska ändpunktsrörelser. Vantaa och Landvetter är nya i flygplatsregistret, med namn verifierade i den samtida tabellen. Koordinaterna är ungefärliga flygfältspositioner från den dokumenterade koordinatfilen. Paris terminal B/G betyder Charles de Gaulle. OSL i denna källa avser uttryckligen Fornebu och mappas till den befintliga historiska punkten FBU.

## Tidsatta reskedjor

Tider nedan är lokala; varje kedja förekommer fredag 28 februari och lördag 1 mars.

| Flyg | Fysiska delsträckor | Kontrollerade markuppehåll |
|---|---|---|
| AF790 | CDG 10.40 → ARN 13.05; ARN 13.45 → HEL 15.40 | Arlanda 40 min |
| AF791 | HEL 16.25 → ARN 16.20; ARN 17.05 → CDG 19.30 | Arlanda 45 min |
| AF796 | CDG 10.30 → FBU 12.40; FBU 13.25 → GOT 14.10; GOT 14.45 → CDG 16.45 | Fornebu 45 min; Landvetter 35 min |

AF791 Vantaa–Arlanda tar 55 minuter; Finland ligger en timme före Sverige. Totalt åtta markuppehåll har kontrollerats. Flygplanssymbolen försvinner mellan de tidsatta delsträckorna. Inga genomgående CDG–HEL, HEL–CDG, CDG–GOT eller FBU–CDG-poster duplicerar dessa AF-kedjor.

## Datum och kvarstående frågor

SK401/402/403/418 använder de tryckta raderna från 23 december. SK593 anges med dagar 1,2,4,6,7 på en rad och dagar 3,5 på raden 8 januari–21 mars; tiderna är identiska och har sammanförts enbart inom granskningsperioden. SK730/731 går fredag medan SK406/409/596 här ger lördagsflyg. SK420:s torsdagstur slutar 22.20 UTC, 90 sekunder före fönstrets start, och räknas därför inte.

SK674 Köpenhamn–Arlanda 14.15–15.25 är importerad. SK594 på fredag och SK410 på lördag har samma tryckta tider på samma sträcka. Dessa två rader lämnas tills vidare till separat granskning av SAS egen utgåva eller ändringsblad. De har inte fastställts som dubbletter eller inställda; urvalet är medvetet ofullständigt.

Finnairs AY873/874 Helsingfors–Paris/retur saknar nonstopmarkering. Inga delsträckor skapas utan underlag för mellanlandningar. SK563:s Göteborg–Köpenhamn och SK570:s Göteborg–Fornebu saknar separata mellantider i urvalet. Endast SK563 Köpenhamn–Paris och SK570 Paris–Göteborg importeras. Samarbetsraderna AFSK/SKAF från Nice/Lyon/Marseille kvarstår för fortsatt granskning.

Air UK:s återstående Stansted- och Kanalörader ligger kvar i kön. Den nyfunna Air France-källan prioriterades eftersom den ger läsbara nordiska avgångar. Finnair- och SAS-sökningarna gav i övrigt inga nya egna avgångssidor som importerades denna omgång. Inga faktiska överflygningar har belagts. Storcirkelanimationen illustrerar tidtabellen och fastställer ingen verklig flygbana.

## Validering

20 befintliga Python-tester passerade. Genererad flyg-JSON, landindex och börsdata är kontrollerade. Alla 1 467 gamla rörelser och 22 skyddade kod-/börsfiler är oförändrade. Nya rörelser har positiva, rimliga restider, inga fysiska dubbletter och inga samtidiga överlapp för samma bolag och flygnummer. Åtta markuppehåll är kontrollerade. De tio äldre kandidatraderna är oförändrade och animeras inte.

Den separata flygtrafikmenyn och v18:s roterande flygplanssilhuett följer med. Ingen C++-ändring tillkommer i v24. Unreal-kompilering, körning i Editor och paketerad spelbuild är inte utförda här.
