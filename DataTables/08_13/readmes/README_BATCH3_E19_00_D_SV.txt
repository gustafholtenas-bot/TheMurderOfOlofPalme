TMOP BATCH 3 – E19-00-D
========================

Källa
-----
Uppslag: E19-00-D
WPU: https://wpu.nu/wiki/Uppslag:E19-00-D
Publicerad fil: Pol-YYYY-MM-DD_E19-00-I_Forhor_med_Anna_Hage.pdf
SHA-256: 05b71485ce5dd550554bb6da9c8fe95dbb15024888e0d209abe41c9eb5c69ec2

Källkontroll
------------
Alla tre originalsidor har granskats visuellt:

1. Karta i skala 1:2000, markerad observationsplats och porträtt av Anna Hage.
2. Fotografi av händelseförloppet från Annas position i bilen.
3. Fotografi från Annas position när hon närmade sig den liggande mannen.

WPU:s publicerade PDF heter E19-00-I och saknar datum i filnamnet, medan
dokumentinnehållet och WPU-sidans placering behandlar materialet som E19-00-D.
Avvikelsen är registrerad och filnamnet har inte fått styra över innehållet.

Infört
------
* Rekonstruktionskarta, porträtt och fotografiska siktlägen.
* Bekräftad norrgående färd på Sveavägen.
* Stopp för rött i vänstra körfältet vid Tunnelgatan.
* Anna längst till höger i baksätet; Åke förare och Karin, Elisabeth samt
  Cilla/Cecilia i bilen.
* Samtidig rekonstruktion av den liggande mannen och den springande mannen.
* Annas rörelse från bilen mot den liggande medan kvinnan böjde sig över honom
  och den springande mannen fortsatte österut in i Tunnelgatan.
* Wincent Lange som undertecknare vid tekniska roteln.

Rekonstruktionsfiguranter
-------------------------
Tre separata ospawnade bildroller har skapats:

* RECONSTRUCTION_FIGURANT_E19_00_D_LYING_PERSON
* RECONSTRUCTION_FIGURANT_E19_00_D_BENDING_WOMAN
* RECONSTRUCTION_FIGURANT_E19_00_D_RUNNING_MAN

Originalet identifierar inte de fysiska figuranterna. De har därför inte
slagits ihop med OLOF_PALME, LISBET_PALME eller THE_KILLER. Deras kläder är
rekonstruktionskläder och inte historiska signalement.

Avgränsningar
-------------
Fotografierna styrker siktlinje och händelseordning men inte exakt
mordnattstid, observationsvaraktighet, UE-transform eller komplett lane-rutt.
WPU:s metadata 00.00 har inte använts som tid. Inga nya observationer behövdes;
de tre befintliga observationerna har endast fått källa och rekonstruktionsnot.

Exakt datadiff
--------------
DT_TMOP_People.json
* 3 tillagda rekonstruktionsfiguranter.
* 9 befintliga personer källkompletterade.

DT_TMOP_Observations.json
* Ändrade: OBS_E19_00_ANNA_SEES_KILLER_FLEE,
  OBS_E19_00_ANNA_SEES_OLOF_LYING,
  OBS_E19_00_ANNA_SEES_LISBET_BESIDE_OLOF.
* Endast SourceReference och Notes ändrades.

DT_TMOP_Groups.json
* Ändrad: GROUP_E19_00_FIVE_CAR_COMPANIONS.

DT_TMOP_HistoricalVehicles.json
* Ändrade: VEHICLE_AKE_LARSSONS_BIL_FORD_ESCORT_GRAMETALLIC och
  OBSERVED_VEHICLE_E19_00_FIVE_FRIENDS_CAR.

DT_TMOP_Uppslag_REGISTER.json
* Ändrade: E19 och E19-00-D.

Inga poster har tagits bort och inga dublett-ID:n har skapats. Befintliga
hårda tidslinje-, observations- och lane-fält är oförändrade.

Resultat efter uppdateringen
----------------------------
People: 735
Observations: 168
Groups: 132
HistoricalVehicles: 72
Uppslag_REGISTER: 3764
HistoricalEvents: 67 (oförändrad)

Kvarstående arbete
------------------
Rekonstruktionsmaterialet ger ingen exakt UE-transform eller stoppsekund.
Åke Larssons alternativa tillfart via Mäster Samuelsgatan eller Hamngatan
kvarstår från E19-00-C. Nästa uppslag i registerordning är E19-00-E.
