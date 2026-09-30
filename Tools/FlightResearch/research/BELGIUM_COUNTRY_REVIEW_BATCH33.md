# Belgien – första källurvalet, v34 / omgång 33

Granskat 2026-09-28. **31 nya planerade rörelser**, **62 belgiska ändpunktsrörelser totalt**, alla via Bryssel National. Täckningen är ofullständig. Alla 2 001 äldre rörelseposter är oförändrade.

24 granskade nonstoprader ger **20 avgångar fredag 28 februari och 11 lördag 1 mars**. 18 grupperas under AF och 13 under SN. AFSN/SNAF betyder samarbete enligt tabellens s. 4 och räknas en gång efter första tryckta bolagskod. Detta fastställer inte faktisk utförande operatör.

| Flygplatspar | Nya rörelser, båda riktningar |
|---|---:|
| Bryssel National–Paris Charles de Gaulle | 17 |
| Bryssel National–Lyon Satolas | 6 |
| Bryssel National–Nice | 4 |
| Bryssel National–Strasbourg Entzheim | 4 |
| **Totalt** | **31** |

Urvalet tillför åtta riktade flygplatspar. Strasbourg–Entzheim tillkommer som flygfältsmarkör. De 31 tidigare Belgien-rörelserna följer med utan ändringar. Landets 62 rörelser omfattar utländska bolag; endast Sabena är hittills registrerat som belgisk operatör.

## Läsregler

VIA-pilen betyder nonstop. B vid Paristiderna betecknar Charles de Gaulle, inte Orly. Alla nya ändpunkter har UTC+1 på de två datumen. Dagmönstren sparas i `belgium_batch33.tsv`; katalogen begränsar de nya raderna till aktuell fredag/lördag. Ingen retur skapas genom spegling.

SNAF639 Bryssel–Paris är tydligt tryckt **08.01–08.50**. Minuten bevaras exakt och noteras särskilt i både katalog och transkription. Om det är ett tryckfel behöver det beläggas med en annan samtida källa innan någon ändring görs.

Bryssel–Marseille och omvänd riktning saknar nonstop-pil och importeras inte som direktflyg. Endast de separat tidsatta Bryssel–Lyon-raderna tas med här; inga markuppehåll eller fortsättningstider uppskattas. Onsdags-, torsdags- och söndagsvarianter som saknar trafik i observationsfönstret skapar inga nya rörelser.

Sabenas egen vinterutgåva nr 1 har identifierats och omslaget lästs, men inga inlagor för utgåvan har verifierats. De nya SN-rörelserna kommer från Air France nr 25. Sobelair, TEA och Air Belgium finns kvar som forskningsspår; deras vintertrafik är inte kartlagd.

Strasbourg identifieras som Entzheim i den samtida tidtabellen. SIA:s moderna ARP-koordinater används som ungefärlig flygfältsmarkör, inte som en rekonstruktion av en gateposition från 1986. Koordinatraden lästes i ett sökindexutdrag från den officiella källan; direkt PDF-hämtning misslyckades med 404. Originalskanningarna omdistribueras inte.

Tidtabeller visar planerad trafik, inte faktiskt genomförande, passagerare, last eller verklig flygbana. De 155 nordiska ändpunktsrörelserna är oförändrade. Inga faktiska överflygningar har tillkommit.

## Granskade källor och sökspår

- [contemporary_timetable](https://www.airtimes.com/cgat/fr/airfrance/pdf/1a/af851027-1a.pdf): Air France nr 25, 27 oktober 1985–29 mars 1986. Förklaringar s. 2–4 samt rader s. 21, 43, 47, 56, 63 och 88 visuellt granskade. 24 nonstoprader ger 31 rörelser, 18 grupperade under AF och 13 under SN. Rader utan nonstop-pil till/från Marseille undantas.
- [contemporary_cover](https://www.airtimes.com/cgat/be/sabena/1a/sn851027.jpg): Sabena Belgian World Airlines, tidtabell nr 1: tryckt giltighet 27 oktober 1985–29 mars 1986. Omslagsbild visuellt granskad. Inga avgångstider på omslaget; inga rörelser importerade från denna källa.
- [edition_index](https://www.airtimes.com/cgat/be/sabena.htm): Indexet anger vinterutgåva 27 oktober 1985–29 mars 1986, nr 1, i överensstämmelse med omslaget.
- [edition_gallery](https://www.airtimes.com/cgat/be/sabena/gal/sngal1a80.htm): Galleriets HTML granskad. Rätt vinteromslag finns; ingen länkad PDF för utgåvan identifierad. Den länkade PDF:en sn830601-1a.pdf gäller 1983 och används inte som 1986-tidtabell.
- [archive_index](https://www.timetableimages.com/ttimages/sn.htm): Sabena-galleriets index granskat. Inget importklart uppslag för vinter 1985/86 hittat; de listade årtalen i området hoppar från 1978 till 1987.
- [access_failed_search_lead](https://www.airtimes.com/cgat/be/sobelair.htm): Försök att läsa möjlig Sobelair-indexadress misslyckades. Ingen innersida läst och inga avgångar importerade.
- [access_failed_search_lead](https://www.airtimes.com/cgat/be/tea.htm): Försök att läsa möjlig Trans European Airways-indexadress misslyckades. Ingen innersida läst och inga avgångar importerade.
- [official_coordinate_reference_search_extract](https://www.sia.aviation-civile.gouv.fr/media/dvd/eAIP_11_JUN_2026/FRANCE/AIRAC-2026-06-11/pdf/FR-AD-2.LFST-fr-FR.pdf): SIA:s officiella sökindexutdrag anger LFST ARP 483231N 0073804E = 48.54194444, 7.63444444. Direkt PDF-hämtning gav 404. Den moderna flygfältsreferensen används endast som ungefärlig markör. Den samtida tabellen identifierar Entzheim på s. 21 och 88.

## Kvarvarande arbete

- Sabenas egna innersidor och ändringsblad vinter 1985/86 återstår. De nya SN-kodade rörelserna kommer från Air Frances tabell; omslaget ger inga avgångstider.
- Inventera Sobelair, Trans European Airways, Air Belgium och andra dåtida bolag med samtida källor. Sabena är fortfarande den enda registrerade belgiska operatören; detta är inte en fullständig bolagslista.
- Övriga internationella förbindelser, belgisk inrikestrafik, andra flygplatser, frakt, charter samt privata/statliga/militära rörelser återstår.
- Bryssel–Marseille-raderna saknar nonstop-pil. Inga genomgående direktsträckor eller stopptider får skapas utan separat tidsatta fysiska delsträckor.
- SNAF639 återges som tryckta 08.01–08.50. Kontrollera mot Sabenas egen vintertabell eller ett ändringsblad; minuten får inte tyst ändras till 08.00.
- SIA:s ARP är en modern ungefärlig flygfältsreferens. Historiska flygbilder eller flygplatskartor behövs för exakt placering av 1986 års terminal- eller gatepositioner.
- Publicerad tidtabell styr planeringen; faktisk flygning, avvikelser och flygväg kräver rörelsehandlingar.

Nästa land i Europakön är Luxemburg, därefter Västtyskland. Inget land har fullständig täckning. Landstal räknar ändpunkter och överlappar varandra.
