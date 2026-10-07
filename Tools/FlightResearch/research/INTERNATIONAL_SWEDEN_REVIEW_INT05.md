# Internationella tillägg och Sverigeuppföljning – INT05

Kumulativ fortsättning på v141_INT04. **26 nya planerade flygrörelser från 24 granskade schemarader**, alla med Malta som ändpunkt. Air Malta får sina första 18 rörelser i databasen; British Airways får fyra, Alitalia två och Lufthansa två. Totalt **11 495 rörelser**.

## Urval och bevarande

Internationellt plus svenskt inrikes omfattar nu **2 257 rörelser**, jämfört med 2 231 i INT04. De 9 238 äldre utländska inrikesrörelserna bevaras men ligger utanför detta urval. Inga nya utländska inrikesrörelser tillförs. Land-/territorieindelningen följer katalogen.

Sverige är oförändrat: **87 ändpunktsrörelser = 41 internationella ankomster + 40 internationella avgångar + 6 inrikes**. Urvalet är partiellt. Antalet ännu saknade flyg kan inte fastställas från dessa tidtabeller.

Alla 11 469 äldre rörelseobjekt och 6 823 äldre scheman är oförändrade. Tidigare flygplatser, länder, observationer och rörelsehandlingar bevaras som hela objekt. Fyra befintliga bolag får ytterligare källhänvisning; Air Malta går från katalogfynd till partiellt granskat. Det gamla Air Malta-indexets bedömning bevaras som historik med en tillagd hänvisning till den nu lästa skanningen.

71 bolag har nu minst en rörelse. Registret har 112 bolag, 122 länder, 399 flygplatser, 96 källor och 6 847 scheman. Ingen fullständig bolags- eller landtäckning påstås.

## Nya rörelser

| Från | Till | Nya rörelser |
|---|---|---:|
| AMS | MLA | 1 |
| CAI | MLA | 1 |
| FCO | MLA | 2 |
| FRA | MLA | 2 |
| LGW | MLA | 3 |
| LHR | MLA | 2 |
| MAN | MLA | 1 |
| MLA | AMS | 1 |
| MLA | CAI | 1 |
| MLA | FCO | 2 |
| MLA | FRA | 2 |
| MLA | LGW | 3 |
| MLA | LHR | 2 |
| MLA | MAN | 1 |
| MLA | TIP | 1 |
| TIP | MLA | 1 |

## Ny originalkälla och tolkning

[Air Malta, Winter Timetable 27.10.85–29.3.86](https://www.airtimes.com/cgat/mt/airmalta/pdf/km851027-1a.pdf) har granskats visuellt i sin helhet: 11 PDF-sidor med omslag, tryckt s.1–16 och annonser. Utgåvan finns nu länkad som fullskanning i arkivets galleri. Den tidigare katalogposten innehöll inga importerade avgångar.

Omslaget och s.1 bekräftar perioden. S.1 säger att tiderna är lokala. De nya raderna finns på s.8,10,11,12,14. Alla tider, veckodagar, datumgränser och flygnummer har lästs i originalet och kontrollerats i förstorade tabellbilder.

**Nonstop är en uttryckligt redovisad tolkning:** tabellerna har inte någon nollstoppskolumn. Enstaka flygnummer utan via-markering behandlas som nonstop eftersom genomresor på andra rader särskilt märks via Lyon, Paris, Zürich eller München. D och N betyder dag- respektive nattflyg, inte direktflyg. Plus betyder nästa lokala ankomstdygn. Australienresorna med flera flygnummer och angivna mellanlandningar används inte som nonstop.

Tryckta nollor i KM024/023 bevaras. Ingen flygplanstyp står i de importerade raderna, och ingen utrustning tilldelas från reklamfoton. Originalskanningen distribueras inte. `source_rows_int05.json` innehåller transkriberade fakta och tolkningsgrund; `source_provenance_int05.json` innehåller URL, filstorlek och SHA-256.

## Datumgränser och UTC

Projektfönstret är **27 februari1986 kl.22:21:30–1 mars kl.22:21:30 UTC**. Brittiska flygplatser använder UTC, Malta/Rom/Frankfurt/Amsterdam/Tripoli UTC+1 och Kairo UTC+2 under dessa datum. En oberoende utvidgning med fasta vinteroffsetar matchar generatorns IANA-tidszoner för samtliga 26 rörelser.

Heathrow har andra lördagstider än övriga dagar. KM131 från Gatwick och KM141 från Manchester avgår fredag och landar efter lokal midnatt i Malta; båda dygnsskiften kontrolleras. KM100/101:s julundantag 25 december1985 ligger utanför fönstret.

Frankfurt-tabellen byter giltighetsblock **1 mars**. Lördagens KM114/115 läses från marsblocket. Dess nytillkomna fredagsrader tillämpas inte på fredagen 28 februari. Amsterdam-raderna KM024/023 kommer från blocket 1 januari–25 mars.

Frakt KM191F från Gatwick på lördagen har avgång **22.30 UTC**, alltså **8 minuter30 sekunder efter fönstrets slut**. Den tas inte med. KM190F är en söndagsavgång och ligger också utanför. Torsdagens Cataniaflyg och BA550/551 avslutas före fönstret; Paris, Lyon, München och Zürich har inga tillämpliga rörelser i de granskade raderna under fredag/lördag. Singapore Airlines Australienresor saknar separat tidsatta mellanben och hålls utanför.

## Flygplatser

**Malta/Luqa (MLA)** och **Tripoli International (TIP)** tillkommer. Air Malta s.4 och14 identifierar Luqa uttryckligen. [Transport Malta](https://www.transport.gov.mt/aviation/civil-aviation-directorate/history-654) beskriver flygfältets kontinuitet och att den äldre terminalen användes till1992. [OurAirports LMML](https://ourairports.com/airports/LMML/) ger den ungefärliga flygfältspositionen 35.845932,14.491546. Den avser inte 1986 års gate eller en flygplansindivid.

S.4 anger uttryckligen Tripoli International Airport. [OurAirports LY-0019](https://ourairports.com/airports/LY-0019/) bevarar identifierarna TIP/HLLT och positionen 32.663502,13.159000. Detta är det historiska internationella flygfältet vid Ben Ghashir. Mitiga används inte som ersättningsplats. Registrets moderna stängd-status förs inte tillbaka till1986.

London-flygplatserna anges uttryckligen som Heathrow respektive Gatwick. Rom kopplas till befintliga Fiumicino/FCO med stöd av flygplatskontoret på s.3; Amsterdam till Schiphol med stöd av s.5. Manchester, Frankfurt och Kairo kopplas till motsvarande befintliga historiska flygplatsposter. Inga tidigare koordinater ändras.

## Sverigeuppföljning och fortsättning

En ny sökning efter SAS/Linjeflyg och vinterutgåvor från British Airways och KLM gav inga nya läsbara Sverige-rader. British Airways 1980-talsgalleri kontrollerades: posten 27 oktober1985 har omslagsbild men ingen länkad fullskanning. Sökträffar om fullständiga vinter-PDF:er på den långa uppdateringssidan tillhör andra bolag; bolaget kontrollerades före användning.

Air Maltas samtliga tidtabellssidor innehåller inga Sverigeflyg. Detta säger inte att all charter eller alla andra bolags flyg till Malta är inventerade. Sverigeprioriteten och tidigare öppna SAS/Linjeflyg/Swedair/Finnair-spår kvarstår. Dakar/Abidjan-spåren från INT04 är ännu inte importerade. Se `sweden_research_leads_int05.json` och föregående rapporter.

## Kontroll och installation

Alla äldre rörelser och scheman jämförs som hela objekt. Nya rörelser kontrolleras för id-dubbletter, fysisk dubblett mot äldre data, samma ändpunkter/tider oberoende av bolagskod, datumgränser, dygnsskifte och oberoende UTC-utvidgning. Resultaten finns i `validation_int05.json`, `automated_checks_int05.json` och `package_preservation_int05.json`.

Detta är en datauppdatering för befintlig flygmeny med kompilerad PaintAirplane-rättelse f23b73a. Slå samman ZIP-filens Plugins/ och Tools/ med projektroten bredvid .uproject. Inga C++-ändringar ingår. Unreal/spel har inte körts här. Tidtabellerna visar planerad trafik; faktisk drift och senare ändringar är inte verifierade.
