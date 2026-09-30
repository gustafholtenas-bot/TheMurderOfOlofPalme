# Norden – Esbjerg och kompletterade Braathens-delsträckor

Granskat 27 september 2026. **Paket v15 tillför sju planerade flygsträckor och undantar tre tidigare Casair-poster med olösta tidskonflikter.** Totalt finns nu **979 rörelser**: 978 tidtabellslagda och en tidigare dokumenterad militär rörelse. **44 rörelser har nordisk start eller destination**, mot 37 i v14. Inga nya faktiskt genomförda flyg eller överflygningar har bekräftats.

## Sju nya avgångar fredag 28 februari

| Flyg | Fysisk delsträcka | Avgång lokal | Ankomst lokal | Underlag |
|---|---|---|---|---|
| BU211 | Stavanger → Haugesund | 09.35 | 09.55 | Braathens s. 11, jämför s. 10 och 20 |
| BU211 | Haugesund → Bergen | 10.10 | 10.35 | Braathens s. 10, jämför s. 11 och 20 |
| BU237 | Stavanger → Haugesund | 15.45 | 16.05 | Braathens s. 11, jämför s. 10 och 20 |
| BU237 | Haugesund → Bergen | 16.20 | 16.45 | Braathens s. 10, jämför s. 11 och 20 |
| BU271 | Kristiansand → Stavanger | 08.25 | 08.55 | Braathens s. 11, jämför s. 10 och 20 |
| BU273 | Kristiansand → Stavanger | 11.30 | 12.00 | Braathens s. 11, jämför s. 10 |
| UK620 | Humberside → Esbjerg | 11.40 | 14.40 | Humbersides avgångspanel, bild 2; Teesside PDF 2 |

Samtliga rader anger måndag–fredag. De ger därför en avgång vardera på fredagen, ingen på lördagen. UK620 är **11.40–13.40 UTC**, två timmar; de norska klockslagen är CET, UTC+1. Endast Esbjerg, EBJ, har lagts till som ny flygplats.

## Hur Braathens stopp kan delas utan att hitta på tider

Primärkälla: Braathens SAFE, *Winter ’85/’86, October 27–March 29*, s. 10, 11 och 20. [Nya fotografier av samma vinterutgåva](https://www.ebay.com/itm/198572849980) kompletterar [den tidigare hänvisningen](https://www.ebay.co.uk/itm/206172029345). Bilderna visar samma sidurval, inte nya returflygssidor.

De nya posterna är en sammanställning av **uttryckligt tryckta delsträckstider** på flera rader. De är inte sex nya rader märkta NONSTOP:

- **BU211:** s. 20 visar Torp–Stavanger 08.20–09.15 utan stopp samt Stavanger–Bergen 09.35–10.35 med ett stopp. S. 11 visar anslutningen från Kristiansand med byte i Stavanger: det andra flyget, BU211, avgår där 09.35 och når Haugesund 09.55. S. 10 visar BU211 från Haugesund 10.10 till bytespunkten Bergen 10.35. Därmed kan det enda stoppet i Stavanger–Bergen-raden identifieras och tidsättas som Haugesund.
- **BU237:** s. 20 visar Torp–Stavanger 14.30–15.25 samt Torp–Bergen 14.30–16.45 med två stopp. S. 11 anger BU237 Stavanger 15.45–Haugesund 16.05; s. 10 anger Haugesund 16.20–Bergen 16.45. De två stoppen är därmed tidsatta. Delsträckornas egna dagkoder bevaras; Torp-delens begränsning till måndag/torsdag/fredag överförs inte till senare delsträckor som anges måndag–fredag.
- **BU271:** s. 10 visar Kristiansand–Bergen 08.25–09.50 med ett stopp. Transferkolumnen på s. 11 anger ankomst till Stavanger 08.55 för BU271. Den redan importerade raden på s. 20 anger Stavanger–Bergen 09.20–09.50 utan stopp. Den nya första delsträckan kopplas till den befintliga andra; endast dess `leg_index` ändras, inga tidigare klockslag.
- **BU273:** Kristiansand–Bergen anges med ett stopp på s. 10. S. 11 visar att BU273 från Kristiansand 11.30 når bytespunkten Stavanger 12.00. Bara Kristiansand–Stavanger läggs in. Någon okänd avgångstid för BU273 vidare från Stavanger till Bergen fylls inte i.

Detta förklarar skillnaden mot v13:s första, snävare urval av endast uttryckliga NONSTOP-rader. Källans flygnummer, namngivna stopp och tryckta tider behövs tillsammans för varje ny fysisk delsträcka. Returflyg och övriga tabellsidor återstår. Braathens teckenförklaringssida saknas fortfarande; 1–7 läses som måndag–söndag enligt konvention, som tidigare dokumenterat.

## Esbjerg och rätt bolag vid rätt datum

[Humberside Airport, Winter Timetable, 27 October–29 March 1986](https://www.ebay.co.uk/itm/198565828590), omslag och onumrerad **Flight Departures**-panel, anger UK620 Humberside 11.40–Esbjerg 14.40 måndag–fredag. [Teessides vinterblad](https://www.dtvmovements.co.uk/Info/History/Documents/Programmes/Timetable_1985W.pdf), PDF 2, anger uttryckligt byte vid Humberside till UK620 och lokal ankomst i Esbjerg 14.40. Hela resan Teesside–Esbjerg har inte skapats som direktflyg. Humbersides ankomstpanel är beskuren; UK621-returen saknar fortfarande fullständig tidsättning.

**Flyv, juni 1986, tryckt s. 133 / PDF 135** i [årsvolymen](https://www.marinehist.dk/FLYV/1986-FLYV.pdf), daterar Air Ecosses övertagande av Humberside–Esbjerg till **1 april 1986**. Händelsen ska därför inte bakdateras till februari. Bara den relevanta sidan har lästs visuellt; hela årsvolymen har inte genomgåtts. Uppgiften stödjer periodiseringen av bolagsbytet men bekräftar inte att UK620 faktiskt flög den 28 februari.

## Rättning: tre Casair-poster är nu kandidater

| Post från v14 | Teessides vinterblad | Humbersides vinterblad | Åtgärd |
|---|---|---|---|
| KS500 Humberside → Teesside | 07.15–07.45 | 06.45–07.15 | Kandidat, inte animerad |
| KS500 Teesside → Glasgow | Ankomst 08.50 | Genomgående KS500 når Glasgow 08.10 | Kandidat, inte animerad |
| KS502 Teesside → Glasgow | Ankomst 18.20 | Genomgående KS502 når Glasgow 18.25 | Kandidat, inte animerad |

Detta är **olösta källkonflikter**, inte bevis på inställda flyg. Tiderna får inte väljas bara genom antagandet att ett novemberomslag måste ersätta ett oktoberomslag. Originalraderna behålls med `review_status: candidate`. Skillnader och fortsatt kontroll finns i `schedule_conflicts_batch14.json`. KS502 Humberside–Teesside 16.50–17.20 är samstämmig i båda bladen och ligger kvar. Casair bidrar nu med fem animerade rörelser i stället för åtta.

## Övriga spår och kvarstående luckor

- **Aeroflot:** sex fotografier av första vinterutgåvan 27 oktober 1985–29 mars 1986 har granskats, med tabellsidor 10, 11, 24 och 29 samt omslag/nätuppslag. Nätet nämner Copenhagen, Oslo, Stockholm, Helsinki, Tampere och Longyear, men dessa foton ger inga nordiska avgångstider. Ny separat `partial_scan`-källa; den äldre katalogpostens avvikande datum skrivs inte över. Inga flyg eller överflygningar skapas från nätlistan.
- **SE-FNZ:** ingen ankomstnotis för Halmstad den 28 februari har hittats. Ett flygplanshistoriskt forumspår var spärrat av en botkontroll och har inte lästs. Inga ankomsttider, operatörer eller lastuppgifter har tilldelats. Den ursprungliga föreningsjournalens notis ligger kvar utan animation.
- **SAS och Finnair:** de följda SAS-spåren gav omslag eller borttagna annonser; Finnairs museumspost kunde inte hämtas via API. Inga nya tillämpliga tabellsidor importerade. Tidigare forskningsspår kvarstår.
- Fortsätt med **UK621 Esbjerg–Humberside**, Braathens returflyg, SAS/Linjeflyg samt finska och isländska vintertabeller. Sverige har fortsatt 14 berörda rörelser, Norge 36 och Danmark nu en; Finland/Island noll. Landtalen överlappar och får inte summeras.

## Validering och leverans

`batch_14.json` innehåller käll-URL:er, kontrollsummor, de sju nya rad-ID:na och de tre undantagna rörelserna. De 971 övriga gamla rörelserna är oförändrade; en ytterligare gammal BU271-post har enbart ändrat delsträcksnummer. Inga tidigare avgångs-/ankomsttider har ändrats. Källbilder och den stora årsvolymen ingår inte i ZIP-filen.

Alla 20 Python-tester samt kontroller av genererad JSON, landindex, tidigare börsdata och reskedjor/tidskonflikter har passerat. Unreal-kompilering och spelkörning kan inte utföras i denna miljö.
