# Västtyskland – första källurvalet, v35 / omgång 34

Granskat 2026-09-28. **89 nya planerade rörelser**, **307 västtyska ändpunktsrörelser totalt**. Täckningen är ofullständig.

56 granskade nonstoprader ger **54 avgångar fredag 28 februari och 35 lördag 1 mars**. 45 grupperas under AF och 44 under LH. Källan är Air France nr 25. Lufthansas eget omslag ger inga avgångstider.

| Flygplatspar | Nya rörelser, båda riktningar |
|---|---:|
| Frankfurt–Paris Charles de Gaulle | 29 |
| Düsseldorf–Paris Charles de Gaulle | 17 |
| München–Riem–Paris Charles de Gaulle | 13 |
| Hamburg–Paris Charles de Gaulle | 10 |
| Köln/Bonn–Paris Charles de Gaulle | 6 |
| Frankfurt–Lyon Satolas | 4 |
| Frankfurt–Nice | 4 |
| Stuttgart–Paris Charles de Gaulle | 4 |
| München–Riem–Nice | 2 |
| **Totalt** | **89** |

Urvalet tillför 18 riktade flygplatspar och flygfältsmarkören Köln/Bonn–Wahn. München använder fortsatt Riem, enligt den samtida tabellen. Den moderna Münchenflygplatsen används inte för dessa rörelser.

## Läsregler

VIA-pilen betyder nonstop. B/G betecknar Charles de Gaulle. Alla nya ändpunkter har UTC+1 på de två datumen. Tryckta veckodagar och giltighetsstarter sparas i `west_germany_batch34.tsv`. Vid ändring 1 november används den senare raden; raden som slutar 31 oktober används inte. München–Nice går här endast på lördagen.

Frankfurt–Marseille och Frankfurt–Toulouse saknar nonstop-pil och importeras inte som direktflyg. AF565 Frankfurt–Lyon är separat tidsatt och tas med. Den får en egen segmentgrupp eftersom katalogen redan har AF565 Toulouse–Madrid medan mellanliggande Lyon–Toulouse inte läggs till i denna omgång. Äldre poster förblir oförändrade; inga tider för det saknade segmentet gissas.

Endast AF- och LH-rader i det angivna Frankrikeurvalet har importerats. Nürnberg, Hannover, Västberlintrafik och andra förbindelser återstår att granska. Västra och östra Tyskland samt Västberlin hålls isär enligt 1986 års indelning.

Köln/Bonn-markören identifieras som Wahn i den samtida tabellen och stöds av flygplatsens egen historik. Registerkoordinaten är en ungefärlig flygfältsreferens; en historisk gateposition är inte rekonstruerad.

Tidtabeller visar planerad trafik, inte faktiskt genomförande, passagerare, last eller verklig flygbana. De 155 nordiska ändpunktsrörelserna är oförändrade. Inga faktiska överflygningar har tillkommit.

Nästa land i Europakön är Östtyskland, därefter Västberlin.

## Granskade källor och sökspår

- [contemporary_timetable](https://www.airtimes.com/cgat/fr/airfrance/pdf/1a/af851027-1a.pdf): Air France nr 25, 27 oktober 1985–29 mars 1986. Visuellt granskade nonstoprader på tryckta s. 25, 32, 34, 35, 43, 54, 56, 57, 64, 66, 67, 71, 76 och 89 samt teckenförklaringar s. 2–4. 56 rader ger 89 rörelser, 45 AF och 44 LH.
- [edition_gallery](https://www.airtimes.com/cgat/de/lufthansa/gal/lhgal1a80.htm): Galleriets HTML läst. Vinteromslag 27 oktober 1985 identifierat. Ingen PDF för detta datum hittad i granskad lista; PDF för 30 mars 1986 gäller för sent och används inte.
- [contemporary_cover](https://www.airtimes.com/cgat/de/lufthansa/1a/lh851027.jpg): Omslag visuellt läst: 27 Oct 85–29 Mar 86. Ingen avgång hämtas från omslaget; nya LH-rader kommer från Air France nr 25.
- [coordinate_reference](https://ourairports.com/airports/EDDK/): CGN/EDDK: 50.865898, 7.142740. Nutida registerkoordinat används endast som ungefärlig flygfältsmarkör.
- [official_airfield_history](https://www.cologne-bonn-airport.com/en/company/newsroom/press-releases/detail/airport-celebrates-75-years-of-civil-aviation.html): Flygplatsens egen historik identifierar den nuvarande platsen vid Wahner Heide, skild från äldre Butzweilerhof. Samtida tabell anger Wahn på s. 25 och 64.

## Kvarvarande arbete

- Lufthansas egen vintertabell och ändringsblad återstår; importen gäller endast ett urval av Frankrikeförbindelser i Air France nr 25.
- Inventera Condor, LTU, DLT, Nürnberger Flugdienst och andra dåtida operatörer med samtida källor. Lufthansa är fortfarande den enda registrerade västtyska operatören; bolagslistan är ofullständig.
- Övriga inrikes- och utrikesrutter, fler flygplatser, frakt, charter, privata/statliga/militära rörelser återstår.
- Nürnbergs NS-rader behöver operatörsidentifiering och fortsatt granskning. Hannoverrader och trafiken till Västberlin återstår också; de ingår inte i denna import.
- Frankfurt–Marseille/Toulouse saknar nonstop-pil. Endast Frankfurt–Lyon importeras här. AF565 Frankfurt–Lyon får en separat segmentgrupp från äldre Toulouse–Madrid; ingen saknad mellansträcka eller marktid uppskattas.
- Faktiskt genomförande, avvikelser och flygväg kräver rörelsehandlingar. Historiska terminal-/gatepositioner är inte rekonstruerade.

Alla 2 032 äldre rörelseposter är oförändrade. Inget land har fullständig täckning. Landstal räknar ändpunkter och överlappar varandra. Originalskanningar omdistribueras inte.
