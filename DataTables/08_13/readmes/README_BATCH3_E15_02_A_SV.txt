TMOP Batch 3 – E15-02-A
========================

Uppslag
-------
E15-02-A – Kompletterande förhör med Ahmed Zahir om Lars Jeppsson och
poliserna vid baracken.
WPU: https://wpu.nu/wiki/Uppslag:E15-02-A
Förhör: 1986-03-13 kl. 07.50 (procedurtid, inte mordnattstid).

Källgranskning
--------------
Tre publicerade bildkällor har granskats visuellt:
- primary-PDF, SHA-256:
  3f1db80c063fedfa794d9de55796046da0b69d1e6b7dd6b7037b300afdaf011b
- fristående dublett-PDF, SHA-256:
  b9e956ef719c9f047632a2a411869d2d8fbabba241b97cfaee65a65d55f3542b
- E15-02-samlingsfil, SHA-256:
  184b067987a40765e1aee0ac193b7039544185c295c3a7e6c7bbb3a2456889d9

Samlingsfilens sida 2 renderar pixelidentiskt med primary. Dubletten återger
samma sakuppgifter i en separat bildkopia.

Integrerade preciseringar
-------------------------
- Ahmed och Yvonne hade gått ett par meter västerut från Johannesgatan när de
  observerade löparen.
- De fortsatte i vanlig, lugn takt och såg sig inte om efter löparen.
- De hade ännu inte börjat gå nedför trapporna när Lars Jeppsson kom uppspringande.
- Samtalet med Lars tog endast några sekunder; paret stannade inte och Lars
  sprang därefter vidare nedför gatan.
- Vid baracken omedelbart väster om Luntmakargatan stoppades paret av flera
  poliser som kom springande från Sveavägen med dragna skjutvapen.
- Poliserna frågade om de sett någon springa förbi. Paret berättade om Lars,
  och poliserna sprang vidare efter ett samtal på cirka tio sekunder.
- Ahmed trodde då att poliserna sökte Lars och förband inte deras fråga med
  den tidigare löparen.

Datamodell och avgränsningar
----------------------------
Inga nya personer, grupper, observationer, fordon, SharedEvents eller
tidslinjepunkter har skapats. Uppslaget säger "flera polismän" och kompletterar
därför både E15-01:s fyrgrupp och E15-02:s femgrupp utan att avgöra
räkneskillnaden eller identifiera någon polis.

Källans "några sekunder" och "cirka 10 sekunder" är bevarade som textuell
varaktighet. Ingen befintlig rekonstruktionssekund har flyttats. Osäkerheten
mellan skytten och löparen gäller fortsatt.

Datatabeller
------------
- DT_TMOP_People.json: 685 poster
- DT_TMOP_Observations.json: 136 poster
- DT_TMOP_Groups.json: 124 poster
- DT_TMOP_HistoricalVehicles.json: 68 poster
- DT_TMOP_Uppslag_REGISTER.json: 3764 poster (E15-02-A uppdaterad)
- DT_TMOP_HistoricalEvents.json: 67 poster, oförändrad
- TMOP_Grand_IdentityHypotheses.json: oförändrad

Kontroller
----------
Inga dubblett-ID:n eller nya brutna referenser. Inga befintliga personers
tidslinjer har ändrats. Tolv befintliga observationer har endast fått
källreferenser och förklarande text kompletterade; tid, TimingMode,
SharedEventId, offset, ankare, radie, varaktighet, observatörer och observerad
entitet är oförändrade.
