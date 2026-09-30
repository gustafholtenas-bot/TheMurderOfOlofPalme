# Turkiet – första källurvalet, v45 / omgång 44

Åtta nya planerade Istanbul–Paris-rörelser: fyra AF mellan Atatürk och CDG, fyra TK mellan Atatürk och Orly. Turkish Airlines registreras och får sina första animerade avgångar. Alla befintliga flygplatsmarkörer är oförändrade.

## Godkända tidtabellsrader

Lokala tider: Istanbul UTC+2, Frankrike UTC+1. 1=måndag … 7=söndag. Utgåvan gäller 27 oktober 1985–29 mars 1986; inga särskilda radbegränsningar är tryckta.

| Kod | Sträcka | Avgång–ankomst | Tryckta dagar | Importdagar | Tryckt sida |
|---|---|---|---|---|---|
| AF608 | CDG–IST-ATATURK | 09:15–13:30 | `1-3-567` | 28 feb och 1 mars | 67 |
| AF609 | IST-ATATURK–CDG | 14:30–16:50 | `1-3-567` | 28 feb och 1 mars | 37 |
| TK925 | IST-ATATURK–ORY | 10:45–13:05 | `-----6-` | 1 mars | 37 |
| TK926 | ORY–IST-ATATURK | 14:35–18:40 | `-----6-` | 1 mars | 67 |
| TK927 | IST-ATATURK–ORY | 16:40–19:00 | `1---5-7` | 28 feb | 37 |
| TK928 | ORY–IST-ATATURK | 20:30–00:35 +1 lokal dag | `1---5-7` | 28 feb | 67 |

Alla sex rader har nonstop-pil i VIA-kolumnen. AF608/609 ger vardera två rörelser; de fyra TK-raderna ger en var. Totalt fyra avgångar fredag och fyra lördag. Samtliga har tryckt flygplansbeteckning 727. Importgiltigheten begränsas till observationsdatumen; utgåvans fulla giltighet bevaras i transkriptionen.

## Midnatt och fysisk flygplats

TK928 avgår fredag 28 februari 20:30 i Paris och landar lördag 1 mars 00:35 i Istanbul. I UTC ligger hela flyget på fredagen: 19:30–22:35, alltså 185 minuter. Ankomstsuffixet a skapar ingen lördagsavgång. AF608 tar 195 minuter; AF609 och TK925/927 tar 200 minuter; TK926/928 tar 185 minuter. Varje riktning har lästs separat.

Istanbul är Atatürk enligt den samtida tabellen. Den tidigare platsmarkören IST-ATATURK används oförändrad. Namnet och flygfältet kontrolleras även mot DHMİ:s historik. Paris B betyder CDG 2B; S betyder Orly Sud. Nutida flygplatskoder eller tidszoner överförs inte till 1986.

## Anslutningar utan animation

Två Ankara–Paris-rader på tryckt sida 14 kombinerar TK119 eller TK121 med AF609 via Istanbul. De har Ankara-avgång och slutlig Paris-ankomst men saknar ankomsttid för den första fysiska delsträckan. De sparas i turkey_connection_research_rows_batch44.tsv utan schemarader eller UTC-rörelser. Den separat publicerade AF609-sträckan Istanbul–CDG importeras en gång. Ingen Ankara–Paris nonstop eller uppskattad inrikesankomst skapas.

Alla nya poster är scheduled: tidtabeller visar planerad trafik, inte belagt genomförande. Storcirkeln illustrerar sträckan, inte verklig flygbana. Inget land är färdiginventerat.

## Källkontroller

- [contemporary_timetable](https://www.airtimes.com/cgat/fr/airfrance/pdf/1a/af851027-1a.pdf): Tryckta s. 37 och 67: sex nonstoprader ger åtta rörelser. Atatürk namnges; IST UTC+2. Paris B=CDG 2B och S=Orly Sud, UTC+1. AF608/609 fredag och lördag; TK927/928 fredag; TK925/926 lördag. TK928:s suffix a är nästa lokala dag.
- [connection_itineraries_not_imported](https://www.airtimes.com/cgat/fr/airfrance/pdf/1a/af851027-1a.pdf): Tryckt s. 14: TK119/AF609 lördag och TK121/AF609 dagar 1/3/5/7 via IST. Ankara–Esenboğa namnges men mellantider saknas. Två rader sparas endast som forskning; den separat tidsatta AF609-sträckan räknas en gång.
- [edition_index](https://www.airtimes.com/cgat/tr/turkish.htm): AirTimes listar sommar 1985 och sommar/vinter1986. Ingen tillämplig vinter 1985/86-inlaga verifierad.
- [edition_gallery](https://www.airtimes.com/cgat/tr/turkish/gal/tkgal1a80.htm): 1980-talsgalleriet har mars 1985, mars 1986 och oktober 1986; inga verifierade vinter 1985/86-tidssidor i denna genomgång.
- [edition_index](https://www.timetableimages.com/ttimages/tk.htm): TimetableImages går från april 1982 till oktober 1986 i detta avsnitt. Äldre fullständiga tabeller används inte för 1986-fönstret.
- [official_airport_history](https://www.dhmi.gov.tr/Sayfalar/Havalimani/Ataturk/SehirTarihcesi.aspx): DHMİ beskriver Yeşilköy-fältet och namnändringen till Atatürk 1985. Samtida Air France-tabell namnger Atatürk. Befintlig IST-ATATURK-markör återanvänds oförändrad. Senare Istanbul-flygplats används inte.

## Kvarvarande arbete

- THY:s fullständiga vinterutgåva 1985/86 och ändringar saknas i det granskade urvalet. Inga avgångar hämtas från sommar 1985, sommar 1986 eller vinter 1986/87.
- Inrikestrafik, Ankara–Esenboğa, Izmir, Antalya och andra orter samt fler utländska linjer återstår. Istanbul–Paris täcker bara en liten del av Turkiets trafik.
- Charter-, frakt-, helikopter-, stats-, privat- och militäraktörer kräver daterade källor. THY är ingen komplett operatörsinventering. Faktiskt genomförande och verkliga överflygningar är inte fastställda.
- Ankaras anslutningar via Istanbul saknar mellanliggande ankomsttid i de granskade raderna. De får inte bli Ankara–Paris nonstop eller en uppskattad inrikessträcka.

Alla 2 349 äldre rörelseposter och schemarader är oförändrade. Originalskanningarna omdistribueras inte. Nästa köpost är Sovjetunionen enligt 1986 års indelning.
