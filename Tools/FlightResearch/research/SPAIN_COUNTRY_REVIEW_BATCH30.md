# Spanien – första källurvalet, v31 / omgång 30

Granskat 2026-09-28. **50 nya planerade rörelser**, **59 ändpunktsrörelser totalt**. Täckningen är ofullständig. Alla 1 901 äldre rörelseposter är oförändrade.

42 visuellt granskade nonstoprader ur Air France nr 25 ger **50 nya rörelser**: 29 den 28 februari och 21 den 1 mars. 27 grupperas under AF och 23 under IB. Samarbetskoder AFIB/IBAF räknas en gång efter första tryckta bolagskod; detta fastställer inte faktisk utförande operatör.

| Spansk ändpunkt | Nya rörelser |
|---|---:|
| Madrid–Barajas | 30 |
| Barcelona–El Prat | 8 |
| Málaga | 4 |
| Palma–Son Sant Joan | 4 |
| Bilbao | 2 |
| Valencia–Manises | 2 |
| **Totalt** | **50** |

Urvalet avser Frankrikeförbindelser, med Orly, Bordeaux, Marseille, Nice och Toulouse som andra ändpunkter. Det tillför 22 riktade flygplatspar. Bilbao, Palma och Valencia är nya markörer. Spanien hade nio tidigare ändpunktsrörelser och har nu 59, samtliga internationella. Detta är 50 individuella avgångar, inte 50 unika flyglinjer.

## Läsregler

Pilen i VIA-kolumnen betyder nonstop enligt s. 4. Endast rader med denna markering importeras. Palma–Paris IB668/669 och Lyon–Madrid saknar pil och har inte lagts in som nonstop. Barcelona får bara de belagda inkommande raderna i detta urval; ingen retur skapas automatiskt. S vid Paristiden betyder Orly Sud, inte Charles de Gaulle. Samtliga importerade ändpunkter har UTC+1 på de två datumen.

Råtranskriptionen i `spain_batch30.tsv` sparar tryckt veckomönster och flightkod. Schemakatalogen begränsar importen till 5=fredag och 6=lördag inom 28 februari–1 mars. Andra dagars tid-, nummer- och flygplansvarianter är inte automatiskt återanvändbara. IB652 och IB654 Madrid–Paris samt IB651 och IB655 Paris–Madrid är olika dagvarianter och har hållits isär.

ENAIRE:s aktuella ARP-koordinater används som ungefärliga flygfältsmarkörer. Aenas historik och den samtida tabellen används för att kontrollera flygfältets identitet. Koordinaterna är inte belagda gate- eller flygplanspositioner från 1986. Originalskanningarna omdistribueras inte.

Tidtabeller belägger planerad trafik, inte faktiskt genomförande, passagerare, last eller verklig flygbana. Något nytt faktiskt överflygningsbelägg tillkommer inte.

## Granskade källor och sökspår

- [contemporary_timetable](https://www.airtimes.com/cgat/fr/airfrance/pdf/1a/af851027-1a.pdf): Nr 25 gäller 27 oktober 1985–29 mars 1986. Förklaringar s. 4–5 samt spanska nonstoprader på s. 18, 20, 45–46, 48–49, 56–57, 60, 62, 70, 73, 77, 91, 93 lästa visuellt. 42 scheman ger 50 rörelser.
- [edition_gallery](https://www.airtimes.com/cgat/es/iberia/gal/ibgal1a80.htm): Rätt vinterutgåva hittad via omslagslänk. Ingen inlaga för utgåvan verifierad.
- [contemporary_cover](https://www.airtimes.com/cgat/es/iberia/1a/ib851027.jpg): Omslag nr 2: 27 oktober 1985–29 mars 1986. Bevarat som källspår; inga avgångar hämtas från omslaget.
- [edition_gallery](https://www.timetableimages.com/ttimages/ib.htm): Index granskat; inget läsbart tidtabellsuppslag för rätt vinter hittat där.
- [edition_index](https://www.airtimes.com/cgat/es/aviaco.htm): Aviacos index granskat. Ingen avgångssida verifierad för 28 februari–1 mars 1986.
- [different_season_lead](https://airline-memorabilia.blogspot.com/2010/08/aviaco-1986-1987-edicion-local-galicia.html): Bloggmaterial med Aviaco 1986/1987 och bilder märkta verano 86; inte verifierat som vinter 1985/86. Inga rörelser importerade.
- [official_coordinate_reference](https://aip.enaire.es/AIP/contenido_AMDT/LE_Amdt_A_2026_10_AD_2_LEBB_en.html): BIO/LEBB: ARP 431804N 0025438W = 43.30111111, -2.91055556. Nuvarande referenskoordinater används endast för ungefärlig flygfältsmarkör.
- [official_airfield_history](https://www.aena.es/es/bilbao/conocenos/historia.html): Bilbaos historia identifierar Sondika och utvecklingen före 1986; ny terminal år 2000 används inte som historisk gate.
- [official_coordinate_reference](https://aip.enaire.es/AIP/contenido_AMDT/LE_Amdt_A_2026_10_AD_2_LEPA_LESJ_en.html): PMI/LEPA: ARP 393306N 0024420E = 39.55166667, 2.73888889. Nuvarande referenskoordinater används endast för ungefärlig flygfältsmarkör.
- [official_airfield_history](https://www.aena.es/es/palma-de-mallorca/conocenos/historia.html): Son Sant Joan öppnades för nationell och internationell trafik 1960. PMI markeras där, inte på Son Bonet.
- [official_coordinate_reference](https://aip.enaire.es/AIP/contenido_AIP/AD/AD2/LEVC/LE_AD_2_LEVC_en.html): VLC/LEVC: ARP 392922N 0002854W = 39.48944444, -0.48166667. Nuvarande referenskoordinater används endast för ungefärlig flygfältsmarkör.
- [official_airfield_history](https://www.aena.es/es/valencia/conocenos/historia.html): Manises/Valencias historik omfattar terminalen från 1983. Den samtida tidtabellens VLC hänförs till detta flygfält.

## Kvarvarande arbete

- Iberias egna inlagor och ändringsblad för vinter 1985/86 återstår, liksom Aviacos samtidiga tabeller och en fullständig operatörsinventering.
- Inrikestrafik, övriga utrikeslinjer, Kanarieöarna och återstående destinationer ingår inte i detta första urval.
- Barcelonas avgångssidor är inte transkriberade i denna omgång. Returriktning eller delsträcka skapas inte genom spegling.
- Palma–Paris IB668/669, Lyon–Madrid och andra rader utan nonstop-pil kräver separat belagda delsträckor. Inga stopp eller klockslag gissas.
- Charter, frakt, militärflyg, helikopterflyg och faktiskt genomförande behöver andra daterade källor.

Monaco, Andorra och Spanien har nu granskats i ett första riktat urval, alla med luckor. Nästa land i kön är Gibraltar, därefter Portugal. Äldre länder återbesöks när nytt underlag hittas. Landstal räknar ändpunkter och överlappar varandra.
