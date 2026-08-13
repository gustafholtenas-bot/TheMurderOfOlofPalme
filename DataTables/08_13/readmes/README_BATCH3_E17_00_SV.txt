TMOP Batch 3 – E17-00
=====================

Uppslag
-------
E17-00 – Förhör med Anders Delsborn om skottlossningen vid
Sveavägen–Tunnelgatan.
WPU: https://wpu.nu/wiki/Uppslag:E17-00
Dokumentdatum: 1986-03-01.

Källgranskning
--------------
Båda originalsidorna har granskats visuellt.

- Original-PDF, SHA-256:
  b6e4e5007e65e0e587f5df1d97db3433a94e544ce6068c9a6dc73dfcb2cc40f7
- Originalblanketten anger förhörsstart 00.20 och avslut 01.30.
- WPU:s metadataruta anger felaktigt 00.00 som starttid. Originalet styr.
- WPU anger registrerat 1991-01-22, ad acta 1991-05-21 och
  PU-anteckningen ”reed”. Dessa uppgifter bevaras som arkivmetadata.

Integrerat innehåll
-------------------
- Anders Delsborn körde för Järfälla Taxi med tre yngre kvinnliga
  passagerare och stannade för rött i mittfilen vid korsningen.
- Han såg ett par och en man stå och samtala på Sveavägens östra sida.
- När taxin började köra hörde han den första kraftiga smällen.
- Vid den andra smällen såg han en rök-/mynningsflamma från vapnet i
  mannens högra hand och såg mannen i paret falla.
- Skytten sprang in på Tunnelgatan i riktning mot Luntmakargatan.
- Delsborns signalementsvariant: cirka 180–185 cm, grå herrhatt långt
  neddragen över pannan, gråaktig normalång ulster med svarta stänk,
  något långsamt steg samt ett vapen med ovanligt lång pipa, typ Colt.
- Kriminalinspektör H. Karlsson har bevarats som en separat ospawnad
  procedurperson eftersom endast initial och efternamn framgår.

Datamodell och avgränsningar
----------------------------
- Det äldre EntityId:t ANDERS_DELBOM behålls för referensstabilitet, men
  visningsnamnet har rättats från Anders Delbom till Anders Delsborn.
- De tre passagerarna namnges inte i E17-00. Uppslaget har därför inte
  ensamt använts för individuella identitetskopplingar till de redan
  associerade passagerarna.
- Delsborns intryck att paret och mannen samtalade har inte gjorts till en
  social trepersonsgrupp med THE_KILLER.
- Den fallande mannen namnges inte i själva iakttagelsen. Kopplingen till
  Olof Palme följer av händelsekontexten och innebär inte samtidig
  namnigenkänning.
- Signalementet är bevarat som en vittnesvariant och skriver inte över
  andra vittnens motstridiga uppgifter eller hårda kategori-, längd- eller
  vapenegenskaper.
- Privat personnummer, bostadsadress och telefonnummer har inte kopierats.
- Förhörs- och arkivtider har inte gjorts till mordnattstider. Skottiden
  23.21.30 är oförändrad.

Datatabeller
------------
- DT_TMOP_People.json: 698 poster (+1 H. Karlsson)
- DT_TMOP_Observations.json: 141 poster (+2)
- DT_TMOP_Groups.json: 125 poster
- DT_TMOP_HistoricalVehicles.json: 68 poster
- DT_TMOP_Uppslag_REGISTER.json: 3764 poster (E17 och E17-00 uppdaterade)
- DT_TMOP_HistoricalEvents.json: 67 poster, oförändrad
- TMOP_Grand_IdentityHypotheses.json: oförändrad

Kontroller
----------
Inga dubblett-ID:n, borttagna poster eller nya brutna referenser. Endast
ANDERS_DELBOM:s två befintliga fordonsrelaterade tidslinjerader har fått
källreferens och förklarande text; deras handling, tid, TimingMode, offset,
ankare, mål och övriga scenariovärden är oförändrade. Fordonets befintliga
tidslinje har på samma sätt endast källkompletterats. Alla äldre
observationer är byte-oförändrade. HistoricalEvents och identitetshypoteserna
är oförändrade.
