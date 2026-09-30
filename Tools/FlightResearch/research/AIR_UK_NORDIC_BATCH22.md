# Air UK och Norden – omgång 22 / paket v23

Granskat 2026-09-28. **97 nya planerade rörelser**, varav **10 med nordisk ändpunkt**. Totalt **1 467 rörelser**. Alla **1 370 tidigare rörelseposter** är exakt oförändrade.

## Källa och avgränsning

[Air UK Winter 1985/86, Issue 1](https://www.airtimes.com/cgat/uk/klmuk/pdf/uk851027-1a.pdf) gäller 27 oktober 1985–29 mars 1986. PDF:en har 13 sidor med uppslag och omslag. Tryckta s. 16–17 och 24–25 saknas. Den är registrerad som partiell. Alla 13 PDF-sidor har granskats visuellt. Endast valda rader med **0 mellanlandningar** importeras, med särskild prioritet för Norden. Storbritannien, Amsterdam och Paris kompletterar urvalet.

Sidan 3 anger lokala tider, veckodagar 1=måndag till 7=söndag, och UK som standardflygkod. Fotnoter anger uttryckligen SAS, KLM och Airbusiness A/S när dessa bolag utför flygen. Importdagar begränsas till 26 februari–1 mars 1986; den genererade tidsperioden använder intervallets överlappning, 27 februari 22.21.30–1 mars 22.21.30 UTC. Senare ändringar och genomförande är inte verifierade.

| Bolag | Nya rörelser | Nya nordiska rörelser |
|---|---:|---:|
| Air UK | 90 | 5 |
| SAS | 4 | 4 |
| KLM | 2 | 0 |
| Air Business A/S | 1 | 1 |
| Totalt | 97 | 10 |

83 granskade tidtabellsrader ger de 97 rörelserna. De fullständiga transkriptionerna finns i `air_uk_batch22.tsv`. URL:er, filstorlekar och SHA-256 finns i `batch_22.json`. Källbilder/PDF återpubliceras inte i paketet.

## Nordiska nytillskott

Tider är lokala. Fredag betyder 28 februari, lördag 1 mars.

| Flyg | Sträcka | Avgång–ankomst | Dag |
|---|---|---|---|
| UK602 | Aberdeen–Stavanger | 08.50–11.20 | Fredag |
| UK601 | Stavanger–Aberdeen | 16.50–17.20 | Fredag |
| SK532 | Aberdeen–Stavanger | 13.15–15.15 | Fredag |
| SK531 | Stavanger–Aberdeen | 12.25–12.25 | Fredag |
| SK534 | Aberdeen–Stavanger | 13.15–15.15 | Lördag |
| SK533 | Stavanger–Aberdeen | 12.25–12.25 | Lördag |
| UK660 | Newcastle–Köpenhamn | 11.30–14.55 | Fredag |
| UK661 | Köpenhamn–Newcastle | 15.25–17.00 | Fredag |
| UK621 | Esbjerg–Humberside | 15.10–16.10 | Fredag |
| 8A163 | Esbjerg–Århus | 15.10–15.50 | Fredag |

SK531/533 tar 60 minuter: Norge är en timme före Storbritannien. UK621 tar 120 minuter. UK620 Humberside–Esbjerg finns redan och har inte lagts in igen. Air Business A/S är ny i operatörsregistret. Århus–Tirstrup är den enda nya flygplatsen; identiteten kontrollerades mot [flygplatsens historia](https://www.aar.dk/aarhus-airports-historie/), inklusive terminalen från 1981. Koordinaterna anger ungefärligt flygfält, inte en rekonstruerad gate.

## Tidsatta delsträckor och luckor

UK201/202 och UK209/210 är uppdelade via Edinburgh och Humberside. UK052/053/056/057 är uppdelade via Norwich. UK204/205 är uppdelade via Teesside. UK552/553 är uppdelade via Norwich till/från Amsterdam. De 18 granskade markuppehållen ligger mellan separat tidsatta flygsträckor; flygsymbolen försvinner under uppehållen.

UK660/661 har kända ändsegment men saknar separat tidtabellsrad för sträckan Newcastle–Leeds/Bradford i PDF:en. Delsträcksnummer 2 lämnas därför som en avsiktlig lucka. Ingen rörelse skapas mellan dessa punkter. Andra genomgående resor med 1 eller 2 stopp förs inte över som direktflyg.

Bergen–Stavanger saknar mellantider. SAS fortsättningar till/från Oslo och Köpenhamn nämns, men är inte tidsatta här. De läggs inte till. Söndagsflyg och tisdag-/torsdagsflyg utan överlappning av fönstret läggs inte in i denna omgång. Kvarvarande Stansted- och Kanalörader, med datumundantag, behöver en egen genomgång.

UK211 Teesside–Norwich finns sedan tidigare med 20.35–21.30. Air UK s. 21 stöder detta; s. 19 visar avvikande 21.00. Den tidigare Teesside-tidtabellen stöder också 21.30. Posten är oförändrad, och ingen ny 21.00-post skapas. Det nya Aberdeen–Newcastle-segmentet med samma flygnummer överlappar inte den gamla posten. Återstående mellansträcka har inte uppskattats.

## Övriga källspår

[British Caledonians North & Midland Region-utgåva](https://www.ebay.co.uk/itm/206234854200) har läsbart omslag, s. 1 och 26–27. SK516 Gatwick–Bergen samt SK524 Gatwick–Århus visas som anslutningar. Innan fysiska delsträckor kan importeras behövs kontroll av mellanlandningar, helst via SAS-tabell eller flygplanerna på s. 39. Inga rörelser från denna källa är importerade.

Air UK-annonsen hos Astralines gav inget nytt läsbart exemplar. AirTimes PDF hittades via galleriets faktiska PDF-länk. De nya SAS-/Finnair-sökningarna gav huvudsakligen tidigare omslag, katalogposter eller fel period.

## Validering

20 befintliga Python-tester passerade. Genererad flyg-JSON, landindex och börsdata är kontrollerade. 1 370 gamla rörelser och 22 skyddade kod-/börsfiler är oförändrade. Nya flyg har kontrollerade tidszoner, positiva och rimliga restider, inga fysiska dubbletter och inga överlapp för samma bolag/flygnummer. Två avsiktliga luckor för UK660/661 är dokumenterade; 18 markuppehåll är verifierade. De tio äldre kandidatraderna är fortsatt utan animation.

Gränssnittet har kvar egen flygtrafikmeny och roterande flygplanssilhuett. Unreal-kompilering, spelkörning och paketerad build är inte utförda här. Storcirkelanimation är en illustration av tidtabellen; faktiska flygbanor och överflygningar är inte fastställda.
