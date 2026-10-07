# Delta – västra USA, Honolulu och Sarasota, v115 / batch112

Granskat 1 oktober 2026. Kumulativ datauppdatering från v114.

71 nya planerade Delta-rörelser från 34 granskade nonstop-scheman på 16 riktade flygplatspar, varav 14 är nya i databasen. Portland i Oregon till/från Seattle och Salt Lake City, San Diego till/från Ontario och Phoenix, Phoenix–Cincinnati, Los Angeles–New Orleans, Dallas–Honolulu och Atlanta–Sarasota i båda riktningar. PDX och SRQ tillkommer som flygplatser. Totalt **8 142 rörelser**, varav 8 141 tidtabellslagda och en dokumenterat genomförd. Alla 8 071 äldre rörelseobjekt och 4 721 äldre scheman är oförändrade. Se `research/DELTA_WEST_HONOLULU_SARASOTA_BATCH112.md`.

## Sträckor

Varje riktning räknas separat. **SAN–PHX och PHX–SAN** fanns redan med Southwest och får nu Delta-trafik. Övriga 14 riktade ändpunktspar är nya. Alla 34 nya scheman ger rörelser i det exakta spelfönstret.

| Riktad sträcka | Nya scheman | Nya rörelser |
|---|---:|---:|
| ATL → SRQ | 6 | 12 |
| CVG → PHX | 1 | 2 |
| DFW → HNL | 1 | 3 |
| HNL → DFW | 1 | 2 |
| LAX → MSY | 1 | 2 |
| MSY → LAX | 1 | 2 |
| ONT → SAN | 2 | 4 |
| PDX → SEA | 2 | 4 |
| PDX → SLC | 2 | 5 |
| PHX → CVG | 1 | 2 |
| PHX → SAN | 2 | 4 |
| SAN → ONT | 2 | 4 |
| SAN → PHX | 2 | 4 |
| SEA → PDX | 2 | 4 |
| SLC → PDX | 2 | 4 |
| SRQ → ATL | 6 | 13 |
| **Totalt** | **34** | **71** |

## Källa och urval

[Delta Air Lines systemtidtabell från 1 februari 1986, Digital Library of Georgia](https://dlg.usg.edu/record/delta_dal-tt_dal-tt-19860201). Accepterade rader har granskats visuellt på tryckta sidor **17, 51, 65, 102, 136, 137, 171, 194, 195, 199, 209, 214, 218 och 224**. Operatörsnyckeln på s.4, sammanhang på s.198/213, daterade fotnoter på s.17/219 och flygplatskoder på s.261 används också.

Samtliga 34 rader anger **0 stopp**, har Delta-mainline-nummer under 1200 och ett tomt frekvensfält, vilket betyder dagligen. I transkriptionen skrivs detta D. Stjärna betyder lågtrafikpris, inte datumskifte. Generiska utrustningssymboler ger inget belägg för ett individflygplan.

Genomgående rader och anslutningsförslag skapar inga extra nonstop-rörelser. Exempelvis går **PDX–ATL 314 via SLC**, **PDX–DFW 424 via SEA**, **PDX–DFW 310 via SLC** och **HNL–IAH 16 via DFW**. Deras separat tidsatta ben har granskats i respektive avreseortstabell. SAN–ONT förekommer under både Los Angeles-samlingsrubriken på s.213 och Ontario-rubriken på s.214; avgångarna importeras bara en gång.

**Fotnot 4 gäller från 12 februari 1986** och finns på **ATL–SRQ 1119** samt **SRQ–ATL 396**. Datumet ligger före hela importperioden. Definitionerna på s.17 och s.219 är samstämmiga.

SLC–PDX 565 avgår **11:28**. SRQ–ATL 560 anländer **22:13**. HNL–DFW är flyg **16**. Förstorade originalutsnitt användes vid granskningen. Hela rader inklusive högra fotnotsfältet granskades; uppslagens varierande bredd hanterades utifrån faktiska sidgränser.

Utgåvans slutdatum och senare ändringar är inte verifierade. Data avser planerad trafik, inte belagt genomförande eller verklig flygbana. Nätet är fortfarande partiellt. Käll-PDF och nya sidbilder ingår inte i paketet. PDF-hash, sidmappning och granskningsdetaljer finns i `delta_west_honolulu_sarasota_source_evidence_batch112.json`.

## Nya flygplatser

| Kod | Historiskt namn i paketet | Tidszon i perioden | Kartmarkör latitud, longitud |
|---|---|---|---|
| PDX | Portland International, Oregon (1986) | UTC−8 | 45.588699, −122.598000 |
| SRQ | Sarasota–Bradenton Airport (1986) | UTC−5 | 27.394631, −82.554359 |

Tidtabellens s.261 identifierar PDX som Portland i **Oregon**, skilt från PWM i Maine, och SRQ som Sarasota/Bradenton. [Port of Portlands historik, PDF-sida 3](https://cdn.portofportland.com/pdfs/Pub_Portside_Fall_15.pdf) bekräftar flygplatsens nuvarande område före 1986. [SRQ:s egen historik](https://flysrq.com/history) beskriver det äldre flygfältet, terminalbytet 1989 och namntillägget International 1992. Det senare namnet används därför inte för 1986.

Koordinaterna kommer från [OurAirports PDX](https://ourairports.com/airports/KPDX/) och [OurAirports SRQ](https://ourairports.com/airports/KSRQ/). De används som **ungefärliga markörer för samma flygfält**, inte som rekonstruerade koordinater för 1986 års terminaler, gater, bantrösklar eller flygplanspositioner. Fullständiga platsnoteringar och källor finns i `airport_location_review_batch112.json` och i katalogens flygplatsposter. Inga nya länder eller operatörer tillkommer.

## Tider, dygnsskiften och markuppehåll

| Ändpunkter | UTC-förskjutning under importdatumen |
|---|---:|
| Honolulu | −10 timmar |
| Portland, Seattle, San Diego, Ontario, Los Angeles | −8 timmar |
| Salt Lake City, Phoenix | −7 timmar |
| Dallas/Fort Worth, New Orleans | −6 timmar |
| Cincinnati, Atlanta, Sarasota | −5 timmar |

**SAN–PHX 504 22:25–00:21** anländer nästa lokala kalenderdag. Det följs av det äldre PHX–ATL-benet 00:55, ett markuppehåll på 34 minuter. **HNL–DFW 16 19:10–06:03** anländer också nästa lokala kalenderdag, med 57 minuter till DFW–IAH 07:00.

**PHX–SAN 259 20:35–20:30** och **487 23:45–23:40** tar båda 55 minuter; ankomstklockan är tidigare eftersom San Diego ligger en tidszon västerut. Inget dygnsskifte läggs på dessa rader.

33 par av gamla och nya separat tidsatta ben med samma flygnummer har granskats. 32 par har två matchande övergångar i de genererade rörelserna. **DFW–SEA–PDX 831** har en, den 28 februari: på den första fönsterdagen anländer det äldre benet 22:00 UTC, före startgränsen, och på sista dagen avgår det nya benet 22:35 UTC, efter slutgränsen. Varje flygintervall prövas självständigt mot fönstret. Detta är väntat och innebär ingen saknad rörelse. Samma flygnummer och rimlig marktid fastställer inte samma individflygplan.

Fönstret är **1986-02-27 22:21:30 UTC till 1986-03-01 22:21:30 UTC**. Hela flygintervallet behålls när någon del överlappar det. **PDX–SLC 310, DFW–HNL 17 och SRQ–ATL 372** ger tre rörelser vardera eftersom de första flygningarna påbörjas före startgränsen och fortfarande pågår när fönstret börjar. Övriga 31 nya scheman ger två rörelser vardera. Lokala avgångsdatum: **27 februari: 17; 28 februari: 34; 1 mars: 20**.

## Validering och bevarande

En separat 24-timmarstranskription av alla **34 rader** och oberoende kalenderberäkning med fasta vinterförskjutningar stämmer med samtliga **71 nya UTC-intervall**. Inga tidsöverlapp hittas i jämförelser mellan berörda gamla och nya rörelser med samma operatör och flygnummer.

Alla **8 071 äldre rörelseobjekt, 4 721 scheman, 317 flygplatser, 82 länder och 98 operatörer** är oförändrade. Två nya flygplatser läggs till. Endast Delta-källposten kompletteras med granskningsnoteringar. Tidigare rättelser och de tre hållna källkonflikterna bevaras.

Alla **20 Python-tester** och de tre kontrollerna av flygdata, landindex och Europaindex passerar. Exakta utdata finns i `automated_checks_batch112.json`; UTC-resultat och markuppehåll finns i `validation_batch112.json`. Verktygskoden är oförändrad. Unreal-kompilering, Editor och paketerat spel har inte körts. Paketet förutsätter redan kompilerad PaintAirplane-rättelse från `f23b73a`.

## Fortsatt arbete

Fler Delta-rutter och separat tidsatta mellanliggande ben återstår. Källkonflikter kräver oberoende underlag. Ingen global nämnare eller säker återstående mängd är verifierad. Europeiska landkön återupptas vid Cypern.
