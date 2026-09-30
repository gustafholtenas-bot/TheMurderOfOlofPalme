# Vintertabeller – Swissair, Dublin och Bryssel

Granskat 26 september 2026. **Paket v16 tillför 101 planerade flygrörelser**, vilket ger **1 080 totalt: 1 079 tidtabellslagda och en tidigare dokumenterad militär rörelse**. De 979 tidigare rörelserna är oförändrade, inklusive tider, status, grupperingar och anmärkningar. Inga nya faktiskt genomförda flyg eller överflygningar har fastställts.

| Primärkälla | Nya godkända tabellrader | Nya rörelser i fönstret |
|---|---:|---:|
| Swissair Benelux, 27 okt 1985–29 mars 1986, s. 24–25 | 34 | 55 |
| Dublin Airport/Aer Rianta, Winter 1985/86, ankomstuppslag Glasgow–Newcastle | 27 | 38 |
| British Airways Worldwide, 27 okt 1985–29 mars 1986, s. 56–57 | 5 | 8 |
| **Totalt** | **66** | **101** |

Ytterligare två transkriberade rader är kandidater. Omgången berör 21 riktade flygplatspar, varav 20 är nya i paketet. Totalt finns 280 riktade platspar, 124 registrerade flygplatser/platser och trafik från 29 bolag/operatörer. Crossair och Air Algérie är nya i bolagsregistret. Länderna Saudiarabien, Algeriet och Elfenbenskusten tillkommer; ingen bolagsinventering markeras som fullständig.

## Swissair: uttryckliga nonstoprader

[Fem fotografier av Beneluxutgåvan](https://www.ebay.co.uk/itm/198568921278) visar omslag, sittplatsplaner, nätkartor och s. 24–25. De importerade raderna är uttryckligen märkta **Nonstop**. Urvalet omfattar Dhahran–Zürich, Dublin–Zürich, Düsseldorf/Frankfurt–Genève/Zürich och Genève–Abidjan/Alger/Amsterdam/Aten/Barcelona/Basel. Även LH-, EI-, KL-, AH-, OA-, IB- och LX-kodade flyg ingår enligt källan; kodtillhörigheten belägger inte vilken operatör eller vilket flygplan som faktiskt utförde flygningen.

- KLM318 gäller 21 december–29 mars. Swissair296 till Aten använder raden 1 februari–29 mars. Äldre alternativ utanför forskningsdatumen förs inte in parallellt.
- Dagkoderna läses enligt konventionen 1=måndag till 7=söndag. Swissairs egen teckenförklaring s. 4–5 saknas i fotourvalet; detta är uttryckligt angivet i källan och flyganmärkningarna.
- SR385 avgår Dhahran fredag 28 februari 01.55 lokal tid, alltså **27 februari 22.55 UTC**, och anländer Zürich 05.05 UTC. Det skapas ingen påhittad nattförskjutning i lokal avgångsdag.
- LX979 Genève–Basel torsdag 27 februari 22.50–23.35 CET överlappar fönstrets början 22.21.30 UTC. Den och fredagens avgång ingår; källan anger ingen lördagstrafik.
- Resor med byte, flera flygnummer, buss eller otidsatta mellanlandningar utesluts. De tvetydiga LH/SR-numren Düsseldorf–Basel utesluts också.

## Dublin: lokala tider, dagar och källkonflikter

[Flygplatsens vinterguide](https://www.ebay.co.uk/itm/198568921327) har fyra fotografier: omslag, brittisk nätkarta, ankomstuppslaget **TO DUBLIN FROM** och teckenförklaring/Europakarta. Guiden anger uttryckligen lokaltid och dagkodernas betydelse. Importerade ortpar är Glasgow, Leeds/Bradford, Liverpool, Gatwick, Heathrow, Malaga, Manchester och Newcastle till Dublin.

Omslaget anger endast **Winter 1985/86**. Exakt start/slut för hela utgåvan lämnas därför som `null`. Oktober som verksam månad stöds av de daterade Dan-Air-raderna 28 oktober–28 mars och registreras som månad, inte som ett påstått exakt publiceringsdatum. De nya Dublinradernas import begränsas till **28 februari–1 mars** inom den angivna vintersäsongen. Källans hela giltighetsintervall har inte rekonstruerats.

Tabellen har ingen separat stoppkolumn. De valda direkta sträckorna har granskats mot anmärkningar och guidens nätkartor. Las Palmas-raden IB702 tas inte in som nonstop: kartan visar Madrid emellan, och samma nummer återkommer från Madrid. EI679 från Milan lämnas utanför eftersom flygplatsen inte kan identifieras säkert från uppslaget. Kartor används här som stöd för sträckornas struktur, aldrig för faktisk flygväg eller överflygning.

| Flyg Heathrow–Dublin | Dublin Airport | BA:s anslutningsförslag, s. 56 | Beslut |
|---|---|---|---|
| EI157 | 11.00–12.10, dagligen | 09.55–11.05, måndag–lördag | Kandidat; varken alternativ animeras |
| EI165 | 14.45–15.55, dagligen | 13.45–14.55, dagligen | Kandidat; varken alternativ animeras |
| EI179 | 21.20–22.30, fredag | 19.45–20.55, **söndag** | Olika dagar; fredagsraden importeras |

Konflikterna är inte bevis på inställda flyg. En Aer Lingus-utgåva eller daterad ändringslista som gäller forskningsdatumen behövs för att avgöra EI157/EI165. Detaljer finns i `schedule_conflicts_batch15.json`. EI179 kontrollerades särskilt mot BA:s dagkolumn; dess söndagsrad får inte användas för att avfärda en fredagsavgång.

BA804:s skilda avgångstider 08.15 fredag och 08.10 lördag bevaras som två tabellrader med åtskilda veckodagar. De skapar inga dubbletter.

## Bryssel: flygplatsen identifieras i transferkolumnen

[British Airways vinterutgåva](https://www.ebay.co.uk/itm/206458321023), s. 57, anger noll stopp för BA371, 375, 377, 379 och 385 från Bryssel till London. Anslutningsförslagen på s. 56–57 identifierar samma BA-flygs ankomstflygplats som **LHR**. Endast Bryssel–Heathrow importeras från dessa rader, inte hela resor vidare till Dublin eller brittiska städer. Lika klockslag i Bryssel och London innebär en timmes flygtid, inte noll minuter.

Sabenas London-rader och Paris-rader lämnas utanför eftersom de synliga uppslagen inte ger motsvarande säkra flygplatsidentifiering. Sidorna 208–209 och sittplatsplanerna 292–293 har också kontrollerats men ger inga ytterligare importerade rader i denna omgång.

## Historiska platser och fortsatt nordisk prioritet

13 flygplatser tillkommer. Aten använder **Ellinikon**, inte den senare flygplatsen vid Spata. Dhahran använder den äldre civila flygplatsplatsen, i dag OEDR, inte King Fahd International. Basel/Mulhouse registreras geografiskt i Frankrike trots sin gränsöverskridande funktion. Koordinatkällor och platsanmärkningar finns i `airport_locations_batch15.json`; koordinaterna avser flygfält, inte exakta terminaler eller flygplanspositioner 1986.

Norden har fortfarande **44 unika rörelser med nordisk ändpunkt**: Sverige 14, Norge 36, Danmark 1, Finland 0, Island 0. Landtalen överlappar. Inga nya nordiska avgångar eller faktiska överflygningar är belagda här.

- Följda SAS-spår gav vinteromslag, borttagna annonser eller en felaktigt träffad 1978-utgåva. Dessa ger inga nya tillämpliga tider.
- Finnairs vinterutgåva, Icelandairs vinteromslag och Air UK:s vinterspår saknade användbara nya avgångssidor. Tidigare blandade sommar-/vinterbilder används inte.
- Olympic- och Dan-Air-samlingarna gav omslag eller andra säsonger. Inga rader importerades från dem.
- **Nytt nordiskt spår:** Dublinguidens Europakarta visar Copenhagen, men tidsatta sidor för denna trafik saknas. Sök Dublin–Köpenhamn i Aer Lingus/SAS vinterutgåvor.
- UK621 Esbjerg–Humberside, Braathens returflyg, SAS/Linjeflyg och finska/isländska tidtabeller kvarstår i kön. Flygplatsändpunkter eller storcirkelmodeller bekräftar inga överflygningar.

## Validering

Alla 20 befintliga Python-tester passerar, liksom kontrollerna av genererad flyg-JSON, landindex och börsdata. En jämförelse mot det uppladdade v15-paketet visar att alla 979 gamla rörelser är identiska. Inga nya samtidiga poster med samma bolag och flygnummer har hittats. Nya tider använder historiska IANA-tidszoner och fönstrets intervallöverlapp.

`europe_batch15.tsv` innehåller transkriptionen; `batch_15.json` innehåller nya rad-/rörelse-ID:n, källbildernas URL:er och SHA-256-kontrollsummor. Originalbilder återdistribueras inte. Transkriptionen är visuellt kontrollerad men inte oberoende dubbelgranskad. Unreal-kompilering och spelkörning har inte kunnat utföras här.
