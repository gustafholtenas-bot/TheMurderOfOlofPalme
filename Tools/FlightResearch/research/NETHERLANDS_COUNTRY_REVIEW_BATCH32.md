# Nederländerna – första källurvalet, v33 / omgång 32

Granskat 2026-09-28. **35 nya planerade rörelser**, **105 nederländska ändpunktsrörelser totalt**. Täckningen är ofullständig. Alla 1 966 äldre rörelseposter är oförändrade.

22 granskade nonstoprader ger **22 avgångar fredag 28 februari och 13 lördag 1 mars**. 14 grupperas under AF och 21 under KL. AFKL/KLAF betyder samarbete enligt tabellens s. 4 och räknas en gång efter första tryckta bolagskod. Detta fastställer inte faktisk utförande operatör.

| Flygplatspar | Nya rörelser, båda riktningar |
|---|---:|
| Amsterdam Schiphol–Paris Charles de Gaulle | 27 |
| Amsterdam Schiphol–Lyon Satolas | 2 |
| Amsterdam Schiphol–Nice | 4 |
| Rotterdam–Paris Charles de Gaulle | 2 |
| **Totalt** | **35** |

Urvalet tillför åtta riktade flygplatspar. Amsterdam har nu 103 ändpunktsrörelser; Rotterdam två. Landets tidigare 70 rörelser var alla knutna till Schiphol. Rotterdam–Zestienhoven tillkommer som markör på flygfältet som öppnades 1956, inte på äldre Waalhaven. Den nyare benämningen Rotterdam The Hague Airport används inte som historiskt namn.

## Läsregler

VIA-pilen betyder nonstop. B och G vid Paristiderna betecknar Charles de Gaulle, inte Orly. Alla nya ändpunkter har UTC+1 på de två datumen. Dagmönstren sparas i `netherlands_batch32.tsv`; katalogen begränsar de nya raderna till aktuella fredag/lördag. Ingen retur skapas genom spegling.

Rotterdam tillför KLAF305 klockan 07.40–08.40 och KLAF308 klockan 19.50–20.45 på fredagen. De båda andra Rotterdam-raderna, KLAF306/307, saknar nonstop-pil och har inte importerats som direktflyg. Amsterdam–Marseille saknar också pil; översikten på s. 10 visar trafik via Lyon. Endast de separat tidsatta Amsterdam–Lyon-raderna läggs till. Inga markuppehåll eller fortsättningstider uppskattas.

KLM:s rätta World timetable har identifierats och omslaget lästs, men inga inlagor finns i det granskade museiföremålet. De nya KLM-rörelserna kommer från Air France nr 25. NLM, Martinair och Transavia finns kvar som forskningsspår; deras fullständiga vintertrafik är inte kartlagd.

LVNL:s aktuella ARP-koordinater för RTM används som ungefärlig flygfältsmarkör, tillsammans med samtida flygplatsidentifiering och flygplatsens historik. Det är inte en rekonstruktion av en gateposition från 1986. Originalskanningarna omdistribueras inte.

Tidtabeller visar planerad trafik, inte faktiskt genomförande, passagerare, last eller verklig flygbana. De 155 nordiska ändpunktsrörelserna är oförändrade. Inga faktiska överflygningar har tillkommit.

## Granskade källor och sökspår

- [contemporary_timetable](https://www.airtimes.com/cgat/fr/airfrance/pdf/1a/af851027-1a.pdf): Air France nr 25, 27 oktober 1985–29 mars 1986. Förklaringar s. 2–4, flerstoppsöversikt s. 10 och rader s. 13, 43, 47, 56, 61, 75, 83 visuellt granskade. 22 nonstoprader ger 35 rörelser, 14 grupperade under AF och 21 under KL.
- [museum_record_and_contemporary_cover](https://collection.sfomuseum.org/objects/1511940817/): KLM World timetable, SFO Museum 2009.122.408. Museets post och dess enda bild granskade. Tryckt giltighet 27 oktober 1985–29 mars 1986. Museet beskriver 114 sidor, men bara omslaget är exponerat i posten. Ingen avgång hämtas från omslaget.
- [edition_gallery](https://www.airtimes.com/cgat/nl/klm/gal/klgal1a80.htm): KLM:s gallerilista för 1980-talet granskat; rätt vinterdatum finns, men inga importklara innersidor verifierade. Separat hämtning av rå HTML misslyckades med timeout.
- [edition_listing](https://www.ebay.com/itm/127316405967): Nordamerikansk KLM-utgåva 27 oktober 1985 identifierad i en annons med en bild. Inga avgångssidor lästa eller importerade; inget köp eller kontakt har gjorts.
- [archive_index](https://northwestairlineshistory.org/timetables-klm/): Arkivets KLM-lista granskad. De äldre länkade utgåvorna hoppar från 1982 till 1993; ingen importklar vinter 1985/86-utgåva hittad i listan.
- [edition_index](https://www.airtimes.com/cgat/nl/cityhopper.htm): Indexet listar föregångaren NLM CityHopper och en utgåva 29 september 1985. Den tryckta indexraden har ett överlappande slutdatum som inte används för avgångar. Ingen avgångssida verifierad.
- [edition_index](https://www.airtimes.com/cgat/nl/martinair.htm): Martinair-index granskat. Inget läsbart vinteruppslag 1985/86 med exakta tider hittat i detta urval.
- [edition_index](https://www.airtimes.com/cgat/nl/transavia.htm): Transavia-index granskat. Första listade vinterutgåvan börjar 27 oktober 1986, efter observationsfönstret; inga tider överförs bakåt.
- [official_airfield_history](https://www.rotterdamthehagueairport.nl/en/organisation/about-us/): Flygplatsens egen historik identifierar Zestienhoven, öppnat 1 oktober 1956, skilt från tidigare Waalhaven. Namnet Rotterdam The Hague Airport infördes 2010 och används inte som 1986-namn i markören.
- [official_coordinate_reference](https://eaip.lvnl.nl/web/eaip/AIRAC%20AMDT%2008-2026_2026_08_06/eAIP/EH-AD%202%20EHRD%201-en-GB.html): RTM/EHRD: ARP 515725N 0042614E = 51.95694444, 4.43722222. LVNL:s aktuella referens används som ungefärlig flygfältsmarkör, inte som gate- eller flygplansposition 1986.

- [small_contemporary_cover_lead](https://www.airtimes.com/cgat/nl/nlm/1a/hn850929.jpg): Litet omslag från indexets utgåva 29 september 1985 visuellt granskat: NLM CityHopper står på omslaget. Datumtextens upplösning är otillräcklig för en säker komplett giltighetsavläsning. Inga innersidor eller avgångstider; ingen import.

## Kvarvarande arbete

- KLM:s egna innersidor och ändringsblad vinter 1985/86 återstår. Den nya importen är ett begränsat Frankrikeurval i Air Frances tabell.
- Inventera NLM CityHopper, Martinair, Transavia och andra dåtida operatörer med samtida källor. KLM är fortfarande den enda registrerade nederländska operatören; detta är inte en fullständig bolagslista.
- Nederländsk inrikestrafik och övriga flygplatser, interkontinental trafik, frakt, charter och privata/statliga/militära rörelser återstår.
- KLAF306/307 Rotterdam–Paris saknar nonstop-pil. Separat tidsatta delsträckor eller ytterligare samtida belägg krävs innan de kan importeras.
- Amsterdam–Marseille via Lyon samt AF1910/1911 Nice–Amsterdam får inte animeras som oavbrutna direktsträckor. Raderna saknar nonstop-pil; inga stopptider gissas.
- Publicerad tidtabell styr planeringen; faktisk flygning, avvikelser och flygväg kräver rörelsehandlingar.

Nästa land i Europakön är Belgien, därefter Luxemburg. Inget land har fullständig täckning. Landstal räknar ändpunkter och överlappar varandra.
