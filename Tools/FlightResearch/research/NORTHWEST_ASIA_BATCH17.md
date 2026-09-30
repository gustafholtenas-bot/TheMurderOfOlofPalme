# Omgång 17 / paket v18 – Asien och flygplansmarkörer

Granskat 28 september 2026. Kumulativ fortsättning från v17.

## Resultat

**47 nya planerade rörelser** från 29 granskade tidtabellsrader, på **25 nya riktade platspar**. Totalt finns 1 182 rörelser: 1 181 planerade och en tidigare dokumenterat genomförd rörelse. Alla 1 135 rörelser från v17 är exakt oförändrade.

Registret omfattar nu 789 godkända tidtabellsrader, 328 riktade platspar, 140 flygplatser/platser och 67 länder/territorier. Fyra nya geografiska poster är Sydkorea, Taiwan, Filippinerna och dåtida brittiska Hongkong. Antalet registrerade operatörer är fortsatt 79; nya länder innebär inte att deras inhemska flygbolag är inventerade. Den landvisa innehållsförteckningen är uppdaterad.

Nordiska ändpunkter är fortsatt 44 rörelser. Förnyade nordiska källsökningar gav inga nya läsbara avgångssidor i denna omgång. Inga faktiska överflygningar har tillförts.

## Källa och transkription

[Northwest Orient System Timetable, 18 december 1985](https://dlg.usg.edu/record/delta_nwa-tt_nwa-tt-19851218), Delta Flight Museum / Digital Library of Georgia.

[Original-PDF](https://dlg.galileo.usg.edu/data/delta/nwa-tt/pdfs/delta_nwa-tt_nwa-tt-19851218.pdf): tryckt s. 139 / PDF 71 för tider och trafikdagar, s. 142 / PDF 74 för fysisk delsträckeindelning och s. 150 / PDF 78 för flygplatskoder. Sidorna har granskats visuellt. PDF-kontrollsumma finns i `batch_17.json`; originalskanningen ingår inte i ZIP-paketet.

Importen begränsas till 26 februari–1 mars, samma granskade tidsperiod som tidigare Northwest-rader. Nästa arkivlistade tidtabellsutgåva börjar 2 mars. Tidtabellen belägger planerad trafik; den bekräftar inte att en viss avgång genomfördes.

| Flygnummer | Nya delar i urvalet | Nya rörelser |
|---|---|---:|
| NW1/2 | Tokyo–Taipei, Tokyo–Osaka–Taipei samt Taipei–Tokyo | 5 |
| NW3/4 | Tokyo–Manila och retur | 4 |
| NW7/8 | Tokyo–Osaka–Okinawa och retur | 4 |
| NW9/10 | Tokyo–Seoul och retur | 4 |
| NW15/16 | Honolulu–Osaka och retur | 5 |
| NW17/18 | Tokyo–Hongkong och retur | 4 |
| NW19/20 | San Diego–Los Angeles–Seattle–Seoul, Seoul–Manila samt Seoul–Seattle–Los Angeles–San Diego | 15 |
| NW23/24 | Los Angeles–Seoul och retur | 2 |
| NW27/28 | Tokyo–Manila, Tokyo–Taipei, Kuala Lumpur–Tokyo och Manila–Tokyo | 4 |
| **Totalt** | | **47** |

Exakta lokala tider, veckodagar och datumförskjutningar finns i `northwest_batch17.tsv`. Endast rader som ger minst en rörelse i fönstret har importerats. De återstående tabellraderna är inte en restlista över all trafik som saknas i världen.

## Historiska flygplatser

| Tabellens ort/kod | Plats i spelet | Kontroll |
|---|---|---|
| Osaka / OSA | Itami, ITM | Kansai öppnade för internationell trafik 1994; 1986 används Itami |
| Seoul / SEL | Kimpo/Gimpo, GMP | Internationell trafik flyttades till Incheon 2001 |
| Hong Kong / HKG | Kai Tak, HKG-KAITAK | Den dåvarande flygplatsen stängde 1998 |
| Kuala Lumpur / KUL | Subang, KUL-SUBANG | Sepang/KLIA öppnade 1998; koordinaterna avser Subang |
| Taipei / TPE | Chiang Kai-shek, dagens Taoyuan | Den internationella trafiken flyttades från Songshan 1979 |

Övriga nya platser är Manila International, Okinawa/Naha och San Diego/Lindbergh Field. Historiska namn används där moderna namn skulle vara anakronistiska. Samtliga koordinatposter och källlänkar finns i `airport_locations_batch17.json`. Positionerna är ungefärliga flygfältsmarkörer, inte rekonstruerade gate- eller flygplanspositioner.

Historiken är kontrollerad mot flygplatsoperatörernas uppgifter: [Itami](https://www.kansai-airports.co.jp/en/business/airports-itm/), [Korea Airports Corporation](https://www.airport.co.kr/wwweng/cms/frCon/index.do?MENU_ID=120), [Hongkong](https://www.hongkongairport.com/en/about-us/?section=our-history), [Subang](https://subangairport.com/en_US/about-airport/) och [Taipei Songshans jubileumshistorik](https://www.tsa.gov.tw/Content/Uploads/FileDownloadArea/fce9d32b-0671-4ab3-9fb2-457810450824.pdf).

## Tids- och ruttdetaljer

- Lokala veckodagar följer varje delsträckas egen avgång. Fortsättningar i Asien kopplas till föregående amerikanska servicedag med `service_day_offset=1`.
- Västerut över datumgränsen ligger ankomsten på följande lokala kalenderdag. Österut ligger ankomsten här på samma lokala datum.
- NW27 Tokyo–Manila på onsdagar och NW28 Manila–Tokyo på torsdagar upphör från 7 januari enligt fotnoten. Dessa turer importeras inte. Fredags- respektive lördagsturen kvarstår.
- NW19 Seoul–Manila på onsdagar och lördagar tillkommer från 7 januari enligt fotnoten. Lördagsturen ingår i fönstret.
- Japanska segment med förbud mot lokaltrafik är markerade som delar av internationella resor, inte som separata bokningsbara inrikesresor.
- Tre tidsöverlapp mellan samma flygnummer tillhör **olika servicedatum**: föregående dags långresa befinner sig fortfarande i Asien när nästa dags amerikanska segment börjar. De är inte dubletter eller samma flygplansindivid. Detaljer finns i `validation_batch17.json`; inga delsträckor överlappar inom samma resa.

## Flygplansikonen

Pilmarkören är ersatt med en fylld flygplanssilhuett med flygkropp, vingar och stjärt. Formen ritas direkt av Slate och kräver ingen ny textur, fontsymbol eller Blueprint-inställning.

Nosen följer den projicerade ruttens riktning när tiden går eller globen roteras. Riktningen beräknas även när en närliggande provpunkt ligger bakom globens kant. Flygplanet visas endast för aktiva flyg; markuppehåll och färdiga flyg ger ingen flygplansmarkör.

Cyan betecknar tidtabell, grönt dokumenterat genomförande och gult valt flyg. Vald markör är något större och ritas ovanpå de andra. En mörk kontur håller formen synlig på kartan, och klickytan är anpassad till den större symbolen.

`flight_icon_preview_v18.png` visar den faktiska kodens geometri i flera riktningar och färger. Det är en separat geometriförhandsvisning, inte en skärmbild från Unreal.

## Installation och kontroll

Byt ut både `Plugins/` och `Tools/` från paketet. **Denna version ändrar C++**, så bygg projektets Development Editor-konfiguration efter installation. Flygtrafiken ligger fortsatt i sin egen pausmenykategori, FLYGTRAFIK KRING MORDET.

Ändrade C++-filer:

- `STMOPFlightWidgets.cpp`: ikon, riktning, markeringsordning och klickyta.
- `STMOPWorldAtlas.cpp`: korrigerad högsta ritnivå för den nya markerade flygplanssymbolen.

Kontrollresultat finns i `validation_batch17.json`: Python-tester, genererad JSON, landindex, börsdata, bevarade rörelser, datumgräns, tidsöverlapp och ikonens geometri. **Unreal-kompilering och spelkörning har inte utförts i denna miljö.**
