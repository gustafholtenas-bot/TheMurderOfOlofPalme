# Southwest – Dallas–Love Field, v58 / omgång 57

415 nya planerade Southwest-rörelser. Totalt 3 003 rörelser: 3 002 scheduled och en tidigare confirmed. Alla 2 588 tidigare rörelser, äldre schemarader och 21 kandidater är oförändrade. Flygtrafikmenyn och flygplanssilhuetten är bevarade.

Användarens nya prioritet är att hitta många nya rutter där bra källor finns. Därför granskas USA nu. Europakön behåller Cypern som återupptagningspunkt; ingen lucka markeras som löst genom prioriteringsbytet.

## Importerat urval

Alla 235 röda Southwest-rader märkta N/S i Dallas-avsnittet på tryckta s. 13–16 är transkriberade. De ger 26 riktade platspar, alltså 13 förbindelser med avgångar i båda riktningarna. Blå Muse Air-rader, rader med en eller flera mellanlandningar och anslutningar med flygnummerpar ingår inte. Corpus Christi och Harlingen saknar N/S-rader i detta avsnitt och räknas därför inte som direktlinjer. Alla anslutningar behöver egna fysiska delsträckstider före import.

| Destination från Dallas–Love Field | Till destinationen | Till Dallas | Totalt |
|---|---:|---:|---:|
| Albuquerque International | 4 | 7 | 11 |
| Amarillo International | 14 | 14 | 28 |
| Austin–Robert Mueller Municipal (1986) | 21 | 25 | 46 |
| El Paso International | 4 | 5 | 9 |
| Houston–William P. Hobby | 45 | 41 | 86 |
| Houston Intercontinental (1986) | 17 | 15 | 32 |
| Little Rock–Adams Field | 11 | 12 | 23 |
| Lubbock International (1986) | 16 | 14 | 30 |
| Midland/Odessa International | 14 | 11 | 25 |
| New Orleans International (1986) | 4 | 5 | 9 |
| Oklahoma City–Will Rogers World | 15 | 16 | 31 |
| San Antonio International | 27 | 26 | 53 |
| Tulsa International | 15 | 17 | 32 |

## Kalender, tid och flygplatser

Utgåvan dateras till 12 januari 1986 av DepartedFlights. AirTimes listar nästa utgåva den 18 mars. Det är stöd för tillämpning under mordhelgen, inte bevis för att alla senare ändringsblad har bevarats. Källans slutdatum lämnas därför öppet. Importens lokala avgångsdatum begränsas till 27 februari–1 mars.

Sidfoten anger 1=måndag, 2=tisdag, 3=onsdag, 4=torsdag, 5=fredag, 6=lördag, 7=söndag samt N/S=nonstop. Rött flygnummer betecknar Southwest och blått Muse Air. Lokal tid följer tidtabellskonventionen och zonövergångarnas tidsmönster; separat all-times-local-text har inte återfunnits i de granskade sidorna. Central Time är UTC−6 och Mountain Time i Albuquerque/El Paso UTC−7 under importdatumen.

Rörelser per lokalt avgångsdatum: {'1986-02-27': 97, '1986-02-28': 214, '1986-03-01': 104}. Av de granskade raderna ger 12 ingen rörelse inom tidsfönstret, bland annat måndags-/söndagsrader och torsdagens morgonrader. De finns dokumenterade i transkriptionen och batch_57.json men skapar inga falska avgångar. Ett flyg räknas när dess intervall överlappar 1986-02-27T22:21:30Z–1986-03-01T22:21:30Z. Flyg redan i luften vid början eller fortfarande i luften vid slutet behåller sina hela tider. WN395 Lubbock–Dallas och WN206 San Antonio–Dallas anländer nästa lokala kalenderdag.

14 nya flygplatsmarkörer tillkommer. Dallas är Love Field, inte DFW. Houston Hobby och Houston Intercontinental hålls åtskilda. Austin AUS är historiska Robert Mueller Municipal, med separat ID AUS-MUELLER och koordinater för det nedlagda fältet. Austin–Bergstrom öppnade först i maj 1999 enligt flygplatsens egen historik. Övriga nya koordinater är ungefärliga flygfältspositioner, inte rekonstruerade 1986-gater eller bantrösklar. Senare hedersnamn används inte som 1986-namn.

## Återstående arbete

Southwest är fortfarande delvis kartlagt. Dallas-avsnittets röda N/S-rader är granskade, men övriga nätets nonstoprader, Muse Air, fullständiga genomgående fysiska tjänstekedjor och ändringsblad återstår. Houston Hobby, Phoenix och Las Vegas är lämpliga nästa större urval. Deltas kompletta 1 februari 1986-utgåva är också tillgänglig för fortsatt utökning. Detta är planerad trafik, inte verifierat genomförande, flygplansindivider eller verkliga flygbanor. Inga nya faktiska överflygningar påstås.

Norden är oförändrat: 161 unika ändpunktsrörelser; Sverige 87, Norge 52, Danmark 85, Finland 4, Island 0. Landstalen överlappar. Alla tidigare Air Inter-konflikter och Interflug-forskningsrader bevaras. Inget bolag eller land betecknas fullständigt. Globalt totalantal bolag 1986 och global täckningsprocent är fortfarande inte fastställda.

## Källor och kontroll

- https://www.departedflights.com/WN011286intro.html — daterat systemtidtabellsindex.
- https://www.departedflights.com/WN011286p13.jpg — tryckta s. 13–14, tidtabellsrader och legend.
- https://www.departedflights.com/WN011286p15.jpg — tryckta s. 15–16, tidtabellsrader och legend.
- https://www.airtimes.com/cgat/usc/southwest.htm — utgivningsdatum; 12 januari och därefter 18 mars 1986.
- https://raw.githubusercontent.com/datasets/airport-codes/main/data/airport-codes.csv — koordinatregister; använda post-ID:n finns i katalogen.
- https://ourairports.com/airports/US-12600/ — nedlagda Mueller-fältets läge.
- https://www.flyaustin.com/history-airport — flygplatsmyndighetens historik över flytten 1999.
- https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201 — tillgänglig full Delta-utgåva för senare arbete; inga nya Delta-rader importerade i denna omgång.

Originalskanningar omdistribueras inte. URL, SHA-256 och granskade sidor finns i southwest_source_evidence_batch57.json. southwest_dallas_batch57.tsv innehåller alla 235 transkriberade rader. validation_batch57.json redovisar kalender/UTC, dubblettkontroll och filbevarande samt projektets 20 Python-tester, tre byggkontroller och börsdatavalidering. Unreal-kompilering och Editor-körning har inte utförts här.
