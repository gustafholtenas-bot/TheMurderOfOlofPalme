# Internationella tillägg och Sverigeuppföljning – INT04

Kumulativ fortsättning på v141_INT03. **34 nya planerade flygrörelser från 28 granskade schemarader**, alla mellan Tunis och Frankrike. Totalt **11 469 rörelser** i databasen.

## Urval och bevarande

Internationellt plus svenskt inrikes omfattar nu **2 231 rörelser**, jämfört med 2 197 i INT03. De 9 238 äldre utländska inrikesrörelserna bevaras i det kumulativa paketet men ligger utanför urvalet. Inga nya utländska inrikesrörelser tillförs. Land-/territorieindelningen följer katalogen.

Sverige är oförändrat: **87 ändpunktsrörelser = 41 internationella ankomster + 40 internationella avgångar + 6 inrikes**. Detta är ett partiellt källbelagt urval och ger inte ett känt antal ännu saknade flyg.

Alla 11 435 äldre rörelseobjekt och 6 795 äldre scheman är oförändrade. Tidigare flygplatser, länder, bolag, observationer och rörelsehandlingar bevaras som hela objekt. Tunisien, Tunis/Carthage och Tunisair tillförs med partiell täckning. 70 bolag har nu minst en rörelse; bolagsregistret har 112 poster, flygplatsregistret 397, landregistret 122 och källregistret 95.

## Nya rörelser

| Från | Till | Nya rörelser |
|---|---|---:|
| BSL | TUN | 1 |
| LYS | TUN | 3 |
| MRS | TUN | 4 |
| NCE | TUN | 3 |
| ORY | TUN | 5 |
| TLS | TUN | 1 |
| TUN | BSL | 1 |
| TUN | LYS | 3 |
| TUN | MRS | 4 |
| TUN | NCE | 3 |
| TUN | ORY | 5 |
| TUN | TLS | 1 |

Air France står för 14 och Tunisair för 20 nya rörelser enligt första tryckta bolagskod. Samarbetskoderna AFTU/TUAF bevaras i underlaget. Varje rad räknas en gång; samarbetet används inte för att fastställa faktisk operatör.

## Källor och flygplatser

[Air France nr 25, 27 oktober 1985–29 mars 1986](https://www.airtimes.com/cgat/fr/airfrance/pdf/1a/af851027-1a.pdf), teckenförklaring tryckt s.2–4, tabeller s.45,49,53,57,77,92. Alla accepterade rader har en nonstop-pil i VIA-kolumnen och saknar särskild datumgräns. Tider, dagar, flygnummer och terminaler har lästs visuellt i originalskanningen. Transkriberade fakta finns i `source_rows_int04.json`; originalskanningar distribueras inte.

Tunis anges uttryckligen som **Carthage**, åtta kilometer från staden. [OurAirports DTTA](https://ourairports.com/airports/DTTA/) ger koordinaterna 36.851002,10.227200 för samma flygfält. De används som ungefärlig platsmarkör, inte som en rekonstruerad gate eller flygplansposition 1986. OACA:s flygplatssida kunde inte hämtas; inga uppgifter påstås ha verifierats där.

Paris använder **Orly Sud**, enligt bokstaven S. De andra franska flygplatserna är Lyon/Satolas, Marseille/Marignane, Nice/Côte d'Azur, Toulouse/Blagnac och Bâle-Mulhouse. Tidtabellens MLH/Bâle-Mulhouse hänförs till befintliga **BSL**. Ingen extra flygplats skapas. Den binationala flygplatsen ligger geografiskt i Frankrike och behåller katalogens landkod fr.

[Tunisairs egen historik](https://www.tunisair.com/fr/histoire/le-lancement-de-la-compagniee) samt [Tunisiens transportministeriums bolagspost](https://catalogue-data.transport.tn/organization/about/tunisair) stöder bolagsidentiteten och landanknytningen. De används inte som källor för 1986 års avgångar.

## Avgränsningar och tidskontroll

Fönstret är **27 februari 1986 kl.22:21:30–1 mars kl.22:21:30 UTC**. Tunis och samtliga franska ändpunkter anges som UTC+1. En oberoende kalenderutvidgning med denna fasta vinteroffset och de tryckta veckodagarna matchar samtliga 34 nya rörelser. Alla tillägg faller på fredag eller lördag och har samma lokala ankomstdygn. Ingen tid har uppskattats eller avrundats.

TU770/771 mellan Tunis och Marseille har olika lördags- och vardagstider. TU760/761 Tunis–Nice har samma klockslag men olika utrustningsrader för måndag/torsdag respektive fredag/lördag; enbart den senare raden ger rörelser i fönstret. TU720/721 Orly–Tunis är lördagsrader. Sådana skillnader bevaras.

Tunis–Bordeaux via Toulouse, Tunis–Metz via Mulhouse och Tunis–Strasbourg via Lyon har inte importerats som nonstop. Endast separat tidsatta internationella flygben tas med. Söndagsrader, även vissa verkliga nonstop-rader, ligger utanför fönstret. Inga franska eller tunisiska inrikesben tillförs.

## Sverige och fortsatta källspår

Sverigeprioriteten ligger kvar. En Aeroflot-annons med sex fotografier av vintern 1985/86 kontrollerades. De läsbara uppslagen var samma sidor 10/11,24,29 och nätverkskartan som redan finns i forskningshistoriken, tillsammans med omslag/baksideskalender. Inga nya Stockholmstider framkom. Att Stockholm finns på en nätverkskarta räcker inte för att skapa en rörelse.

Ett Aeroflot-galleri beskriver en 52-sidig utgåva, men den åtkomliga posten har bara omslagsminiatyr och tom bildlista. Den är inte en tillgänglig fullskanning. En LOT-annons för vintern 1985/86 hittades i sökresultat men kunde inte öppnas; inga tider har lästs eller importerats från den. Se `sweden_research_leads_int04.json`.

SAS/Linjeflyg/Swedair originaltabeller och ändringsblad samt Finnair M016-27845/27847 är fortsatt relevanta. Tidigare öppna frågor om mellanlandningar och saknade tider kvarstår; detta tillägg löser inte dem. Dakar/Abidjan-spår som lästes under arbetet lämnas till en separat genomgång och ingår inte i INT04.

## Kontroll och installation

Alla äldre dataobjekt jämförs före och efter. Nya rörelser kontrolleras för id-dubbletter, fysisk dubblett mot äldre data och samma ändpunkter/tider oberoende av bolagskod. 20 befintliga tester och alla tre generatorkontroller ska passera; resultaten finns i `automated_checks_int04.json`. UTC-utvidgning, urval och bevarande dokumenteras i `validation_int04.json` och `package_preservation_int04.json`.

Paketet är en datauppdatering för befintlig flygmeny med kompilerad PaintAirplane-rättelse f23b73a. Slå samman ZIP-filens Plugins/ och Tools/ med projektroten bredvid .uproject. Inga C++-ändringar ingår. Unreal eller spelet har inte körts här. Tidtabellerna belägger planerad trafik; faktisk drift och verkliga flygbanor är inte fastställda.
