# Belize, Honduras och Jamaica – v130 / batch127

Granskat 2 oktober 2026. **18 nya planerade flygrörelser**, totalt **9 060 rörelser**. **Åtta nya riktade sträckor**, två nya flygplatser och två nya länder i registret. Alla **9 042 äldre rörelseobjekt och 5 377 äldre scheman är oförändrade**.

Tio scheman från **Challenge International Airlines** är granskade. Åtta ger rörelser i projektets tidsfönster, 27 februari1986 kl22:21:30 UTC till1 mars kl22:21:30 UTC. Två onsdagsvarianter ger inga rörelser. Tiderna visar publicerade planer, inte bekräftat genomförande, passagerare eller last.

## Nya sträckor och tider

| Riktad sträcka | Nya rörelser |
|---|---:|
| BZE–MIA | 3 |
| KIN–MIA | 2 |
| MBJ–MIA | 2 |
| MIA–BZE | 2 |
| MIA–KIN | 2 |
| MIA–MBJ | 2 |
| MIA–SAP | 2 |
| SAP–MIA | 3 |

Varje rörelse är en daterad avgång, inte en egen linje. Åtta riktade sträckor motsvarar fyra förbindelser räknade utan riktning.

| Tryckt flygnummer | Sträcka | Lokala tider | Dagar | Rörelser |
|---|---|---|---|---:|
| VV105 | MIA → BZE | 13:15–14:15 | X3 | 2 |
| VV106 | BZE → MIA | 16:15–19:10 | X3 | 3 |
| VV301 | MIA → KIN | 07:45–09:20 | D | 2 |
| VV308 | KIN → MIA | 22:40–00:15 (+1 dag) | D | 2 |
| VV307 | MIA → MBJ | 20:00–21:30 | D | 2 |
| VV302 | MBJ → MIA | 10:30–12:00 | D | 2 |
| VV101 | MIA → SAP | 12:55–14:05 | X3 | 2 |
| VV103 | MIA → SAP | 12:55–14:05 | 3 | 0 |
| VV102 | SAP → MIA | 16:10–19:10 | X3 | 3 |
| VV104 | SAP → MIA | 16:10–19:10 | 3 | 0 |

D = dagligen, X3 = alla dagar utom onsdag, 3 = onsdag. Raderna finns på den onumrerade tidtabellssidan, skanning2, i kolumnerna From Miami och To Miami. **Samtliga accepterade rader har uttryckligt Stops=0.**

## Källa och giltighet

Primärkälla: [Challenge International Airlines, System Timetable,20december1985](https://www.timetableimages.com/ttimages/3c8512.htm), med [omslag/karta](https://www.timetableimages.com/ttimages/3c/3c8512/3c8512-1.jpg) och [tidtabell](https://www.timetableimages.com/ttimages/3c/3c8512/3c8512-2.jpg). Omslaget trycker hela datumet. **Inget slutdatum anges.** Arkivets nästa listade utgåva är1987; det bevisar inte att inga mellanliggande ändringar fanns. Importen följer projektets hantering av publicerade, öppna tidtabellsperioder och har ofullständig ändringstäckning.

Operatörsnamnet följer omslaget. Tryckt flygprefix **VV** bevaras i källnoter och transkription, trots arkivmappens kod3C. Juridiskt bolag, namnbyteskronologi och trafiktillstånd är inte verifierade. Posten slås inte samman med Challenge Air Cargo. Utrustning och individflygplan är ospecificerade.

Tiderna tolkas som **lokala enligt tidtabellskonvention**, med kontroll mot historiska tidszoner; någon uttrycklig all-times-local-legend har inte återfunnits. Miami och Jamaica är UTC−5, Belize och Honduras UTC−6 för importdatumen. **VV308 landar00:15 följande dygn. VV302 landar12:00 på dagen.** VV106 frånBelize ochVV102 frånSanPedroSula pågår vid både fönstrets första och sista gräns, vilket ger tre datum vardera. Övriga aktiva scheman ger två. Fördelning efter lokal avgångsdag: torsdag4, fredag8, lördag6.

Tio enstoppsrader hålls utanför. Exempelvis MIA–KIN medVV307 ochMIA–MBJ medVV301 är genomgående alternativ via den andra jamaicanska orten. En tid vid mellanlandningen saknas. På motsvarande sätt saknas interna klockslag förSanSalvador ochTegucigalpa. Inga avgångar, ankomster eller markuppehåll fylls i genom gissning. Se `caribbean_withheld_batch127.json`.

## Flygplatser

**BZE: Belize City – Belize International, Ladyville.** Flygplatsoperatörens [historik](https://www.pgiabelize.com/about-us/) beskriver Ladyville och terminalens invigning1945. Den internationella Miami-trafiken normaliseras till detta flygfält, inte kommunalaTZA. Ett senare hedersnamn läggs inte tillbaka i1986.

**SAP: San Pedro Sula – La Mesa.** [La Gaceta,8juni1964, dekret81, sidor1–2](https://tzibalnaah.unah.edu.hn/bitstream/handle/123456789/8158/19640608.pdf?isAllowed=y&sequence=2) beskriver finansiering avLaMesa-flygplatsen förSanPedroSula. Challenge-tabellen är samtida belägg för stadens internationella flygförbindelse. NamnetLaMesa används; senare terminalutbyggnader rekonstrueras inte.

Flygplatskoderna är vår normalisering av tryckta stadsnamn, inte tryckta koder. Moderna [MZBZ](https://ourairports.com/airports/MZBZ/) och[MHLM](https://ourairports.com/airports/MHLM/) används endast som ungefärliga flygfältsmarkörer. Exakta1986-banor och terminalpositioner är inte fastställda. Omslaget namngerNormanManley förKingston ochSangster förMontegoBay; befintligaKIN/MBJ-poster behålls.

## Andra källor granskade

**Virgin Islands Seaplane Shuttle,14september1985:** [hela tabellen](https://www.timetableimages.com/ttimages/3g8509.htm) har avgångstider men inga ankomster eller flygtider. Den ger därför inga animerade rörelser. Sjöflyghamnar kringStThomas/StCroix får inte ersättas med landflygplatsernaSTT/STX. Sök kompletta klockrader och hamnidentiteter.

**Caribbean Express,15juli1985:** [utgåvan](https://www.timetableimages.com/ttimages/wh8507.htm) har användbara klockslag, men[AirTimes publiceringsförteckning](https://www.airtimes.com/cgat/usa/caribbeanexpress.htm) listar en senare utgåva från**1februari1986**. Dess inlaga behöver hämtas. Juli-tabellen läggs därför inte in för slutet avfebruari.

**Airways International,15december1985:** [tvåsidig tabell](https://www.timetableimages.com/ttimages/4a/4a8512/4a8512.pdf) ger dagliga tider, men saknar uttrycklig stoppkolumn/nonstop-angivelse. Den allmänna reservationen om ändrade stopp räcker inte för att avgöra de fysiska benen. Tiderna hålls i forskningskön tills stoppmönstret är belagt.

**Arrow Air:** originalbilderna i[14november1985-utgåvan](https://www.timetableimages.com/ttimages/jw8511.htm) löser den tidigare datumförvirringen:1985 är tryckt. Däremot återger[Vita husets nyhetssammanställning12februari1986, trycktB-6/PDF-sida18](https://www.reaganlibrary.gov/sites/default/files/2025-03/40-399-5730359-386-018-2025.pdf#page=18) att reguljär passagerartrafik hade ställts in, medan annan charter och frakt skulle fortsätta. Den äldre passagerartabellen projiceras därför inte på projektets datum. Domstolens[Arrow Air v.United States](https://law.justia.com/cases/federal/district-courts/FSupp/649/993/1631302/) behandlar specifiktMAC-uppdragens avbrott7februari och återkomst med endast frakt10april. Det säger inte att all privat charter eller frakt var stoppad. Inga enskilda sådana avgångar har belagda kompletta tider här. Se `errata_batch127.json`.

ÄldreLI126-konflikt, LI117:s sista tidsmässigt omöjliga ben, LI168:s asterisk och operatörernas juridiska kronologi ligger kvar. Äldre rapporter bevaras som historik; rättelsen i `errata_batch126.json` omLI117:s första ben gäller fortsatt.

## Täckning

Samma31 forskningsområden används: **514→532 rörelser**, **150→158 riktade par**, **36→38 flygplatser**, oförändrat64inrikesrörelser. Belize ochHonduras fanns redan i forskningsomfånget men får nu poster i spelets landsregister.

| Utvalda områden | v129 | v130 |
|---|---:|---:|
| Nicaragua | 0 | 0 |
| Belize | 0 | 5 |
| Honduras | 0 | 5 |
| Amerikanska Jungfruöarna / St. Thomas, St. Croix | 31 | 31 |
| Brittiska Jungfruöarna | 8 | 8 |
| Jamaica | 4 | 12 |

Områdessummor kan överlappa. **St. Thomas14, Tortola8 och Nicaragua0 är oförändrade.** Noll visar en datalucka, inte historisk frånvaro av flygtrafik. Inga nya militär-, stats-, privat-, charter- eller fraktflyg tillkommer. Mellanösternurvalet är oförändrat72rörelser.

Globalt: **9 060 rörelser**, varav9 059planerade och en tidigare bekräftad; **5 387 katalogscheman/5 366 granskade**; **383 flygplatser/platser**, **121 länder/territorier**, **1 780 riktade par**. **110 operatörsposter**, varav67med rörelser och43utan. Registret är inte en verifierad global inventering av juridiska flygbolag. **91 källposter**, varav33tidtabellsutgåvor bidrar med rörelser. Ingen färdigprocent är fastställd.

## Kontroller och installation

**20 befintliga Python-tester och tre generatorkontroller passerar.** Samtliga18nyaUTC-intervall är jämförda mot en separat klocktranskription med beräknade flygtider och väntade avgångsdatum. Tidszoner, midnatt, onsdagsvarianter, gränsöverlappning och dubbletter är kontrollerade. Alla äldre rörelser, scheman, operatörer, källposter, länder och flygplatser är oförändrade. Äldre kod, rapporter och rättelser bevaras. Nya källbilder/PDF:er återdistribueras inte; derasURL:er och kontrollsummor dokumenteras.

Stäng Unreal Editor och slå samman ZIP-filens `Plugins/` och `Tools/` med projektroten bredvid`.uproject`. Paketet är kumulativt och innehåller data. **PaintAirplane-rättelsen f23b73a måste redan vara kompilerad.** C++-bygge, Unreal Editor och paketerat spel har inte körts här. Fortsatt kö: `central_america_caribbean_queue_batch127.json`.
