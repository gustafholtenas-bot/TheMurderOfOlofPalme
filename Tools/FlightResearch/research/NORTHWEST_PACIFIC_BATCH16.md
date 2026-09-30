# Omgång 16 / paket v17 – Northwest Stillahavstrafik och Prestwick

Granskat 27 september 2026. Kumulativ fortsättning från v16.

## Resultat

- **55 nya planerade rörelser** från 23 dagliga, fysiska delsträckor.
- **1 135 rörelser totalt:** 1 134 planerade och en tidigare dokumenterat genomförd rörelse.
- **23 nya riktade platspar** genom de nya flygen. En tidigare Glasgowsträcka flyttas dessutom från GLA till PIK; totalt 303 riktade platspar.
- Åtta nya flygplatser: PIK, DCA, DFW, DTW, PHL, TPA, HNL och NRT. Totalt 132 flygplatser/platser.
- 1 079 tidigare rörelser är exakt oförändrade. En tidigare rörelse har rättad destination och källhänvisning, med samma tider.
- Nordiska ändpunkter är fortsatt **44 rörelser**. Inga nya faktiska överflygningar är belagda.

## Tidtabell och kontroll

[Northwest Orient System Timetable, 18 december 1985](https://dlg.usg.edu/record/delta_nwa-tt_nwa-tt-19851218), förvarad hos Delta Flight Museum och publicerad av Digital Library of Georgia.

[Original-PDF](https://dlg.galileo.usg.edu/data/delta/nwa-tt/pdfs/delta_nwa-tt_nwa-tt-19851218.pdf).

| Tryckt sida / PDF-sida | Användning |
|---|---|
| 139 / 71 | Lokala tider, daglig trafik, datumgräns och fullständigt tidsatta delsträckor |
| 142 / 74 | Kontroll att de importerade sträckorna är fysiska flygsegment |
| 138 / 71 | Befintlig NW48 London–Glasgow och dess lördagsankomst |
| 43 / 23 | Uttrycklig uppgift att samtliga Northwestflyg till Glasgow använder Prestwick |
| 150 / 78 | Flygplatskoder, lokaltid och teckenförklaring |

PDF-filen har 81 sidor. Tryckta sidnummer och PDF-sidnummer skiljer sig. SHA-256 finns i `batch_16.json`; originalskanningen ingår inte i ZIP-paketet.

Importen begränsas till 26 februari–1 mars, samma granskade period som de tidigare Northwest-raderna. Nästa arkivlistade utgåva börjar 2 mars. Senare faktiska ändringar eller genomförande är inte belagda av denna tidtabell.

Alla 23 nya rader är dagliga. Västerut från USA till Japan ligger ankomsten på följande lokala kalenderdag; österut ligger den på samma lokala datum trots den lägre klocktiden vid ankomst. Historiska tidszoner används vid UTC-omräkning. Varje fysisk delsträcka har egna tider och markuppehållen skapar inga flygrörelser. Ett gemensamt flygnummer belägger inte samma flygplansindivid genom hela resan.

Råtranskription: `northwest_batch16.tsv`. Genererade rörelse-ID:n, kontrollsumma och rättelse: `batch_16.json`. Flygplatskällor: `airport_locations_batch16.json`.

## Rättad Glasgowflygplats

NW48 lördagen 1 mars, Gatwick 08.05–09.15 lokal tid, ska sluta på **Prestwick (PIK)**. V16 använde Glasgow Abbotsinch (GLA). Northwests egen stationsrubrik på s. 43 och kodlista på s. 150 avgör flygplatsen. Avgång, ankomst och antal rörelser är oförändrade. Rättelsen ersätter ett rörelse-ID; den räknas inte som en ny avgång.

## Avgränsningar

Urvalet omfattar USA–Tokyo i båda riktningarna och de fullständigt tidsatta amerikanska delsträckorna för NW1/2, 3/4, 7/8 och 27/28. Honolulu–Tokyo NW9/10 och JFK–Tokyo NW17/18 ingår också.

Fort Lauderdale–Tampa, ytterligare Asiensegment, Guam, Seoul, Osaka, NW77/88 och Atlanttabellens amerikanska matarsträckor är inte importerade i denna omgång. Översiktens ensamma avgångs- eller ankomsttid för ett stopp räcker inte för att skapa en fullständig delsträcka. Fotnoterna om NW19/20 och NW27/28 gäller särskilda fortsättningssegment i Asien; dessa segment ingår inte i urvalet. USA–Tokyo-sträckorna är markerade som dagliga.

NRT följer tabellens Tokyokod och avser Narita, då New Tokyo International Airport. DCA benämns Washington National och HNL Honolulu International enligt perioden. Flygplatsmarkörerna är ungefärliga platser för flygfälten, inte historiska gate- eller flygplanspositioner.

## Nordiska arkivspår

Två originalhandlingar har granskats visuellt. De ger forskningshänvisningar men inga importerbara fullständiga flygtider:

- [A6774-01](https://wpu.nu/wiki/Uppslag:A6774-01): ensidig anteckning om en SAS-tidtabell för perioden som där anges till 25 september 1985–29 mars 1986, förvarad bland originalhandlingarna. Själva tidtabellen saknas i den publicerade ensidiga PDF:en. Startdatumet behöver kontrolleras mot tidtabellens omslag.
- [A6774-00](https://wpu.nu/wiki/Uppslag:A6774-00): PM den 6 maj 1986 hänvisar till insamlade passagerarlistor för avgångar från Arlanda 1–7 mars och charter 1–15 mars. PM beskriver uppgifter om avgångstid, flygnummer och destination. Listbilagorna ingår inte i den granskade ensidiga PDF:en.

Exakta dokumentlänkar och nästa steg finns i `nordic_priority.json`. Inga passageraruppgifter har importerats och ingen arkivförfrågan har skickats. Handlingarnas förekomst i en utredning belägger ingen anknytning mellan ett flyg och mordet.

## Validering

Kontrollresultat sparas i `validation_batch16.json`. Kontrollerna omfattar befintliga Python-tester, genererad JSON, landindex, börsdata, bevarade rörelser, tidsordning och samtidig användning av nya flygnummer. Unreal-kompilering och spelkörning har inte utförts i denna miljö.
