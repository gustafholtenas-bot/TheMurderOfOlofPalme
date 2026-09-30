# Sverige, Norge, Danmark och England – omgång 6 / paket v07

Kontrollerat 26 september 2026. **28 nya tidtabellslagda fysiska flygsträckor**, totalt **693** rörelser: 692 planerade och en tidigare dokumenterat genomförd. Källorna belägger inte att de nya flygningarna faktiskt genomfördes.

| Land | Nya uppgifter i animationen | Fortsatt kö |
|---|---|---|
| Sverige | NW44 Gatwick–Arlanda, fredag 28 februari 08.05–11.20; NW37 Arlanda–Gardermoen, lördag 1 mars 12.20–13.20 | SAS vinter 1985/86, Linjeflyg, Swedair samt flygplatsjournaler |
| Norge | NW37 Arlanda–Gardermoen och Gardermoen–JFK, lördag. Den senare 14.20–16.25 | SAS, Braathens SAFE, Widerøe, Norving; militära transportjournaler för Anchor Express |
| Danmark | Inga nya animerade avgångar i denna omgång. Northwests Köpenhamnstabell är granskad men veckodagarna ligger utanför fönstret | SAS, Maersk Air, Cimber Air och gemensamma Danair-tidtabeller |
| England | Gatwick-trafik med Northwest, GB Airways och British Airways: Stockholm, Minneapolis, Boston, Frankfurt, Glasgow och Gibraltar | BA:s fullständiga vintertabell, British Caledonian, ytterligare regionala bolag och rörelsejournaler |

Tiderna ovan är lokala vid respektive flygplats. Arlanda–Gardermoen förekommer under både Sverige och Norge, så landraderna ska inte summeras. Gatwick–Glasgow är England–Skottland; båda ligger under Storbritannien i landfiltret. NW44 Gatwick–Arlanda och NW48 Gatwick–Glasgow har trafikrestriktionen **ingen lokaltrafik**; de visas som fysiska delsträckor i genomgående trafik.

## Inlästa originaltabeller

1. [Northwest Orient, 18 december 1985, Delta Flight Museum / Digital Library of Georgia](https://dlg.usg.edu/record/delta_nwa-tt_nwa-tt-19851218). Originalskanning, tryckt s. 138 / PDF 71 samt Stockholm på s. 120 / PDF 62. Atlanttabellen har granskats visuellt. 16 registrerade tidtabellsrader ger 14 rörelser i fönstret; fyra av raderna beskriver Köpenhamnstrafik utan träff i fönstret. Övriga rader kan ge en eller två rörelser. [Nästa utgåva i arkivindex börjar 2 mars 1986](https://northwestairlineshistory.org/timetables-northwest/); den används inte för mordhelgen.
2. [GB Airways / British Airways, vintern 1985–86](https://www.timetableimages.com/ttimages/complete/gt85/gt85.pdf). Båda PDF-sidorna granskade. Sidan 1 anger Gatwick; sidan 2 ger veckodagar och lokala tider. Tio tidtabellsrader ger 14 rörelser: tolv GT-kodade och två BA-kodade. Åtta av rörelserna är Gibraltar–Tanger eller retur, från samma tabell.

`nordic_uk_batch06.tsv` innehåller transkriptionerna. `batch_06.json` innehåller ID:n, antal och SHA-256 för originalfilerna. Skanningarna ingår inte i ZIP-paketet.

## Viktiga avgränsningar i Northwest-tabellen

- Oslo anges uttryckligen som **Gardermoen**, med dåtidens kod **GEN**. Befintliga **FBU/Fornebu** ersätts inte. Koordinaterna visar flygfältets område; moderna terminaler eller uppställningsplatser används inte som påstådda 1986-positioner.
- NW44/45 och 840–845 delar internationella delsträckor. De räknas en gång som 44/45, så att samma fysiska segment inte blir flera flyg i animationen.
- Fotnoten undantar NW38/39 lördag, NW48 måndag och NW49 fredag under 7 januari–6 mars. Exempelvis läggs **inte** NW49 Gatwick–Boston in fredag 28 februari.
- NW30 JFK–Köpenhamn avgår söndag och fortsätter Köpenhamn–Stockholm måndag. NW31 Stockholm–Köpenhamn–JFK går söndag. Ingen av dessa delsträckor överlappar vårt fönster som slutar lördag 1 mars 22.21.30 UTC.
- NW36 JFK–Gardermoen avgår lördag 19.05 EST, vilket är söndag 00.05 UTC och därmed utanför fönstret. NW37:s återresa på lördagen ryms däremot.
- Delsträckorna ligger som separata rader och får gemensam reseidentitet när de hör ihop över midnatt. Kurvorna anger inte verkliga flygkorridorer.

## Nordiska källor som fortfarande behöver avgångssidor

| Operatör | Kontrollerad källa | Status och nästa steg |
|---|---|---|
| SAS | [Quick Reference Timetable 27 okt 1985–29 mars 1986](https://oarnestadcollection.com/TT/TTImages/SAS-QRT-1985-27-10-25.jpg); [världsutgåvor](https://oarnestadcollection.com/TT/SAS.asp) | Rätt vinteromslag hittat. Själva tabellsidorna behövs. |
| Braathens SAFE | [Tidtabellsindex](https://www.timetableimages.com/ttimages/bu.htm) | Omslag för 27 oktober 1985 finns. Full vintertabell behövs. |
| Norving | [Tidtabellsindex](https://www.timetableimages.com/ttimages/rt1.htm) | Omslag för 27 oktober 1985 hittat. Mars 1985 och mars 1986 har fullskanningar men belägger inte den aktuella vintertrafiken. |
| Widerøe | [Tidtabellsindex](https://www.timetableimages.com/ttimages/wf.htm) | Ingen utgåva för vårt datum hittad i detta index. |
| Linjeflyg | [Tidtabellsindex](https://www.timetableimages.com/ttimages/lf.htm) | Ingen verifierbar vintertabell inläst. Sök även SAS gemensamma svenska inrikestabeller. |
| Maersk Air / Cimber Air | [Maersk-index](https://www.timetableimages.com/ttimages/dm.htm), [Danair](https://www.timetableimages.com/ttimages/dx.htm) | Två bolag tillagda i den danska kön. Sök Danairs gemensamma vintertabell samt respektive bolags utrikeslinjer. Faktisk operatör ska fastställas; en DX-beteckning är inte tillräcklig. |

Inget av länderna är färdiginventerat. De funna omslagen ger inga nya animerade flyg.

## Militärt underlag och nästa handlingar

**Norge:** [NATO:s katalogpost POLADS(86)6](https://archives.nato.int/communique-de-presse-et-politique-dinformation-du-public-proposes-pour-lexercice-anchor-express-86) identifierar en föreslagen press- och informationsplan för Anchor Express 86. Själva handlingen gick inte att läsa här. [US Naval Institutes översikt för 1986](https://www.usni.org/magazines/proceedings/1987/may/u-s-naval-operations-1986), avsnittet *Atlantic Command Exercises*, anger perioden 15 februari–18 mars och att övningen avbröts 6 mars. Detta är övningskontext, inte en lista över faktiska flyg. Den amerikanska amfibiefasen anges till 6–12 mars och får inte dateras om till mordhelgen. Nästa steg är transportorder och flygplatsjournaler med tider och ändpunkter.

**Sverige:** Förhöret om GUSTAV från förra omgången står kvar som en retrospektiv uppgift om Visby–Arlanda. Inga nya tider, flygnummer eller belägg för militärt flygplan har hittats.

**Danmark:** Stationernas rörelsejournaler och eventuella transportorder för NATO-förstärkningar behövs. Ingen militär avgång har fastställts i denna omgång.

**England:** RAF:s stations- och förbandsjournaler är nästa mål. Även [Teessides februariarkiv](https://www.dtvmovements.co.uk/Archivesmonths/1986/1986%20-%20Feb.pdf) och [marsarkiv](https://www.dtvmovements.co.uk/Archivesmonths/1986/1986%20-%20Mar.pdf) har lokaliserats. De är kvar som granskningskällor; inga flygidentiteter, rutter eller klockslag därifrån har importerats.

De militära forskningsposterna finns i `military_research.json`. De är inte egna kort i spelmenyn och läggs inte i animationen utan tillräcklig tids- och rörelsedokumentation. Inget i denna omgång belägger en koppling mellan flygtrafiken och mordet.

## Uppdatering v13, 2026-09-27

Braathens har nu 22 importerade avgångar från vinterutgåvan. SAS/Linjeflygs Malmöutgåva har fyra granskade tabellsidor, men inga importerbara fysiska delsträckor ännu. De äldre källstatusarna ovan avser den tidigare omgången. Aktuellt nuläge finns i `NORDIC_TIMETABLE_BATCH12.md` och `COUNTRY_INDEX.md`.
