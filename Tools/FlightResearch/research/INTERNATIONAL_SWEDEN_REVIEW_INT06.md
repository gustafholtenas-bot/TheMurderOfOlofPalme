# Internationella tillägg – INT06

Kumulativ fortsättning på INT05. **11 nya planerade rörelser från 11 granskade schemarader**: tio till/från Dakar och en Paris–Abidjan. Air France får fem, Air Afrique fem och UTA en. Air Afrique tillkommer i bolagsregistret.

Totalt **11 506 rörelser**, varav **2 268 internationella eller svenska inrikesrörelser**. Alla 11 495 äldre rörelseobjekt och 6 847 äldre scheman är oförändrade. De 9 238 äldre utländska inrikesrörelserna bevaras i paketet men ligger utanför urvalet. Inga nya utländska inrikesrörelser tillförs.

Sverige är oförändrat: **41 internationella ankomster, 40 internationella avgångar och 6 inrikesrörelser**, totalt87. Detta är dokumenterad partiell täckning; antalet saknade flyg är inte känt. Inga nya Sverige-rader har hittats i denna omgång. Tidigare öppna SAS/Linjeflyg/Swedair/Finnair-spår och prioriteten på Sverige kvarstår; ingen ny inventering av dessa källor påstås här.

## Originalkälla och transkription

[Air France nr25, vinter27okt1985–29mars1986](https://www.airtimes.com/cgat/fr/airfrance/pdf/1a/af851027-1a.pdf), tryckt s.20,27,48,60,65. Teckenförklaringen på s.2–4 har kontrollerats igen. Tiderna är lokala; a betyder nästa lokala dygn; pil i VIA-kolumnen betyder uttryckligen nonstop. Alla elva importerade rader har pil. Tom VIA-kolumn behandlas inte som nonstop.

| Flygkod | Från | Till | Lokal avgång | Lokal ankomst | Veckodag | Sida |
|---|---|---|---|---|---|---|
| RK004 | DKR | BOD | 23:45 | 05:35 nästa dag | 5 | 27 |
| AF302 | DKR | MRS | 23:59 | 06:05 nästa dag | 5 | 27 |
| AF300 | DKR | BSL | 23:45 | 06:25 nästa dag | 4 | 27 |
| RK018 | DKR | CDG | 10:00 | 16:35 | 5 | 27 |
| AF306 | DKR | CDG | 15:10 | 21:40 | 6 | 27 |
| RK017 | CDG | DKR | 23:59 | 04:50 nästa dag | 4 | 65 |
| AF307 | CDG | DKR | 09:00 | 13:45 | 6 | 65 |
| AF303 | MRS | DKR | 17:35 | 21:55 | 5 | 48 |
| RKAF015 | MRS | DKR | 13:40 | 17:45 | 6 | 48 |
| RK003 | BOD | DKR | 17:00 | 21:05 | 5 | 20 |
| UT805 | CDG | ABJ | 15:30 | 20:40 | 6 | 60 |

Veckodag4=torsdag,5=fredag,6=lördag. RK017 avgår uttryckligen **torsdag**, inte fredag: 27feb23.59 i Paris =22.59 UTC, med ankomst28feb04.50 UTC i Dakar. Den gäller från12december. AF300 gäller från19december och RK018 från13december. Ingen utgången decembervariant används.

RKAF015 är samarbetskod enligt s.4 och räknas en gång under första tryckta kod RK. Den faktiska operatören fastställs inte. Tryckta utrustningskoder bevaras i forskningsunderlaget men ingen flygplansindivid eller runtime-modell tilldelas.

DKR är befintliga Dakar/Yoff, inte den senare flygplatsen DSS. ABJ är Port Bouët, BOD Mérignac och MRS Marignane. Paris A/G avser Charles de Gaulle. Tryckt MLH/Bâle–Mulhouse kopplas till befintliga BSL för samma flygfält. Inga flygplatser, koordinater eller tidigare landkopplingar ändras.

## Air Afrique

Originalets s.4 identifierar RK som Air Afrique. [Världsbankens rapport, ruta1.3, PDF-sida28](https://thedocs.worldbank.org/en/doc/416451434652908214-0190022009/original/AirTransportchallenges.pdf) beskriver bolaget som multinationellt med huvudkontor i Abidjan. Katalogkopplingen ci avser huvudkontoret; den är inte en exklusiv nationalitet eller fullständig kartläggning av 1986 års ägarstater. Operatörsinventeringen markeras inte som komplett. Rapporten används enbart för bolagsbakgrund, aldrig för1986 års flygtider.

Registret innehåller113 bolag, varav72 med rörelser, samt122 länder,399 flygplatser,96 tidtabells-/övriga katalogkällor och6 858 scheman. Bakgrundsreferensen för Air Afrique ligger i bolagsposten och proveniensfilen, inte som ny flygtidtabell.

## Avgränsningar och gränsfall

Projektfönstret är27feb1986 kl.22:21:30–1mars kl.22:21:30 UTC. Frankrike/Bâle–Mulhouse har UTC+1, Dakar/Abidjan UTC. En separat utvidgning med fasta offsetar matchar generatorns IANA-zoner för samtliga elva rörelser.

AF301 Bâle–Mulhouse–Dakar på torsdagen landar22.20 UTC,90sekunder före fönstrets start och ingår inte. RKAF016 Dakar–Marseille avgår lördag22.30 UTC,510sekunder efter fönstrets slut och ingår inte. UTA806 Abidjan–Paris avgår lördag23.00 UTC och ligger också utanför.

Genomresor Paris–Dakar via Bordeaux/Marseille/Bâle–Mulhouse utan nonstop-pil importeras inte som långa nonstopben. De separat tidsatta internationella benen tas med där originalet ger dem. Franska inrikesben tillförs inte. Övriga Paris–Abidjan-rader utan pil lämnas utanför; UT805 på lördagen har uttrycklig pil. Söndags- och måndagsrader utanför projektfönstret ingår inte.

## Kontroller och installation

Alla äldre dataobjekt har jämförts med INT05. Nya poster kontrolleras mot gamla för id, normaliserat flygnummer, ändpunkter och UTC-tider; dessutom söks dubbletter oberoende av bolagskod. Elva oberoende beräknade rörelser matchar exakt generatorn. Befintliga20 tester och tre generatorers --check körs, resultaten sparas i automated_checks_int06.json. Originalskanningen distribueras inte. Transkription, källhash och validering finns i source_rows_int06.json, source_provenance_int06.json och validation_int06.json.

Slå samman ZIP-filens Plugins/ och Tools/ med projektroten bredvid .uproject. Endast data och forskningsdokumentation; inga C++- eller verktygskodsändringar. PaintAirplane-rättelsen f23b73a måste redan vara kompilerad. Unreal/spel har inte körts här. Tidtabellstrafik är planerad, inte bekräftat faktiskt genomförd. Ändringsblad och fullständig trafik är inte inventerade.
