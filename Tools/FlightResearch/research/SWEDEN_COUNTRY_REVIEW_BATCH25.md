# Sverige – första landgenomgången / v26, omgång 25

Granskat 2026-09-28. **Inga nya importklara avgångar hittades i de kompletterande källorna.** Paketet har fortsatt **1 649 rörelser**, varav **87 med svensk start eller destination**. Sverige är delvis granskat; detta är inte en fullständig inventering av landets trafik.

Användarens nya arbetsordning är land för land i Europa. Första omgången börjar med Sverige. **Norge är nästa fördjupning**, följt av Danmark, Finland och Island. Hela arbetsordningen och aktuella rörelsetal finns i `EUROPE_COUNTRY_INDEX.md`. Tidigare importer i andra länder följer med men innebär inte att deras landsgenomgångar är färdiga.

## Vad Sverige redan har

| Bolagskod | Rörelser med svensk ändpunkt |
|---|---:|
| SAS | 61 |
| Golden Air | 12 |
| Air France | 12 |
| Northwest | 2 |
| Totalt | 87 |

Sex är inrikesrörelser och 81 gränsöverskridande. Fem svenska flygplatser är inlagda: Arlanda, Bromma, Landvetter, Karlskoga och gamla Karlstad–Jakobsberg. Detta anger innehållet i paketet, inte alla flygplatser som fanns eller trafikerades 1986.

| Riktad fysisk sträcka | Inlagda rörelser |
|---|---:|
| ARN → CDG | 5 |
| ARN → CPH | 27 |
| ARN → GEN | 1 |
| ARN → HEL | 2 |
| BMA → KSK | 3 |
| CDG → ARN | 5 |
| CDG → GOT | 1 |
| CPH → ARN | 27 |
| FBU → GOT | 2 |
| FBU → KSD-OLD | 3 |
| GOT → CDG | 2 |
| HEL → ARN | 2 |
| KSD-OLD → FBU | 3 |
| KSK → BMA | 3 |
| LGW → ARN | 1 |

GEN är den befintliga historiska Gardermoen-punkten; FBU är Fornebu. KSD-OLD är Karlstad–Jakobsberg. Flygplatsbyten och nutida flygplatsnamn får inte ändra de historiska ändpunkterna.

SAS och Golden Air har importerade rader. Linjeflyg har en delvis läsbar utgåva men inga importklara avgångar här; Swedair saknar granskad tidtabell. Utländska bolags Sverigeavgångar ingår i landets rörelsetal även om de inte har Sverige som hemland. Svenska bolag kan omvänt ha rörelser helt utanför Sverige. Därför skiljer sig denna sammanställning från hemlandsgrupperingen i `COUNTRY_INDEX.md`.

## Källkontroller i denna omgång

- [index_checked](https://www.airtimes.com/cgat/se/sas/gal/skgal1a80.htm): 1985/86-utgåvor finns i indexet; inga PDF-länkar hittades i detta galleri.
- [index_checked](https://www.timetableimages.com/ttimages/lf.htm): Inga läsbara avgångssidor för vintern 1985/86 hittades på denna Linjeflyg-indexsida.
- [unavailable_listing](https://www.ebay.com/itm/157857969262): Tidigare känt SAS edition 4-spår gav HTTP 404 vid öppning; ingen ny tidtabell kunde läsas.
- [existing_evidence_carried_forward](https://www.ebay.co.uk/itm/198453305668): Tidigare granskade Malmöblad 14,15,22,23 kvarstår som reseförslag utan tillräcklig delsträcks- och flygnummerinformation.
- [contemporary_pages_rechecked](https://www.airtimes.com/cgat/fr/airfrance/pdf/1a/af851027-1a.pdf): Tryckta s. 87–88 återkontrollerade visuellt. Importerbara fredags-/lördagsrader finns redan; övriga resor/veckodagar blir inga nya avgångar.
- [index_checked](https://www.airtimes.com/cgat/de/lufthansa/gal/lhgal1b-scandinavia.htm): Inga PDF-länkar hittades i detta Skandinaviengalleri.
- [index_checked](https://www.airtimes.com/cgat/ch/swissair/gal/srgal1b-scan.htm): Inga PDF-länkar hittades i detta Skandinaviengalleri.
- [index_checked](https://www.airtimes.com/cgat/ch/swissair/gal/srgal1b-scan-ukie.htm): Inga PDF-länkar hittades i detta regionala galleri.
- [unavailable_page](https://www.linjeflyg.info/linsidor.htm): HTTP 404 vid hämtning av länksidan.
- [index_checked](https://www.airtimes.com/cgat/indexse.htm): Namnindex ger ytterligare forskningsspår men fastställer inte verksamhet eller avgångar den aktuella helgen.

Sökningar efter SAS, Linjeflyg, Swedair och Sverigeavgångar gav därutöver främst äldre källspår, fel period eller allmän historik. Det är inget bevis för att fler tabeller saknas i andra arkiv. Air France-sidorna visar både fysiska avgångar och resor via Köpenhamn; genomgående resor dupliceras inte. Många andra rader går på veckodagar utanför fredag/lördag eller har redan importerats.

Namnen Scanair, Transwede, AMA Norving/Salair och Air Nordic/Arosflyg sparas som ytterligare sökmål. Deras exakta operatörsstatus den aktuella helgen måste kontrolleras mot samtida register innan de tas in som verifierade operatörer. Inga avgångar, flygnummer eller klockslag skapas från namnindex. Forskningskön behöver också frakt, charter, privat- och militärflyg.

## Konkreta luckor

1. Komplett SAS/Linjeflyg-vinterutgåva med giltighet, teckenförklaring, flygnummer, stopp och tider per fysisk sträcka. Malmöbladen räcker inte för att härleda hela inrikesnätet.
2. Swedairs vintertabell och belägg för vilka avgångar som publicerades under eget respektive annat bolags kod.
3. SK563 Göteborg–Köpenhamn och SK570 Göteborg–Fornebu saknar separata mellantider. SK594/410 behöver jämföras med SK674:s samma tryckta Köpenhamn–Arlanda-tider. De är inte fastställda som dubbletter eller inställda.
4. Golden Air Karlskoga–Karlstad behöver egen ankomst-/avgångstid; hela Karlskoga–Oslo-resan får inte ritas som nonstop.
5. Utländska bolags direktsträckor till/från Sverige, inklusive datumundantag och vinterändringar.
6. Daterade rörelsejournaler/flygledningshandlingar för faktiskt genomförande och överflygningar. Nuvarande storcirkelanimation belägger ingen verklig färdväg.

## Leverans och återupptagning

V26 tillför landöversikten, en maskinläsbar Europakö och denna granskningslogg. **Flygdata och befintlig spelkod är oförändrade från v25.** Den egna flygtrafikmenyn och flygplanssilhuetten ingår fortsatt. Alla 1 649 rörelser följer med i det kumulativa paketet.

`europe_country_queue.json` innehåller arbetsordning, status och nästa uppgift. `build_europe_index.py` genererar `EUROPE_COUNTRY_INDEX.md` och `europe_country_index.json` från aktuell katalog och aktuella avgångar. Länder som ännu saknas i runtime-registret kan finnas som sökområden med `catalog_country_id: null`; detta skapar inga flyg.

```sh
python Tools/FlightResearch/build_europe_index.py
python Tools/FlightResearch/build_europe_index.py --check
```

Ingen landrad markeras fullständigt färdig. Noll i ett land betyder inga inlagda poster, inte noll faktisk trafik. En inrikesrörelse räknas en gång; en internationell kan förekomma i två länders ändpunktstal. Landsiffrorna ska inte summeras till världstotalen.

Kontrollresultat finns i `validation_batch25.json`. Unreal-kompilering och Editor-körning är inte utförda här.

20 befintliga Python-tester passerade. Flyg-JSON, båda landöversikterna och börsdata är kontrollerade. Alla tidigare pluginfiler och 22 skyddade kod-/börsfiler är identiska med v25. Alla 40 landrader har kontrollerats mot sina unika rörelse-ID:n.
