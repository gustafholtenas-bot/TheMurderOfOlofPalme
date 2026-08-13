TMOP Batch 3 - WPU-tillgänglighetsrevision för E-uppslag
========================================================

Syfte
-----
`DT_TMOP_Uppslag_REGISTER.json` hade redan fältet `Availability`, men vissa
poster var markerade `Available` enbart därför att en WPU-sida fanns. En
WPU-sida kan bestå av bara liggarinformation och samtidigt sakna bilagd
handling. Detta gjorde att E107-04 och E107-05 behandlades i onödan innan
frånvaron av dokument upptäcktes.

Ny betydelse för Availability
-----------------------------
- `Available`: WPU kategoriserar sidan som `Uppslag med dokument`.
- `NotReleased`: WPU kategoriserar sidan som `Uppslag utan dokument`.
- `Unknown`: WPU-sidan saknas, har motstridiga kategorier eller saknar en
  uttrycklig dokumentstatuskategori.
- `Partial` bevaras för avsnitts-/föräldraposter och beskriver projektets
  delvisa integrering, inte en direkt dokumentsida.

Genomförd kontroll
------------------
Alla direkta registerposter i serie E vars SourceUrl pekar på en WPU-
uppslagssida har kontrollerats mot WPU:s MediaWiki-kategorier den 2026-08-02.
Resultatet finns i `WPU_E_AVAILABILITY_AUDIT_2026-08-02.json`, inklusive varje
uppslags tidigare status, verifierade status, WPU-kategori och sid-ID.

Arbetsregel framöver
--------------------
Nästa uppslag väljs bland poster där:

1. Posten ligger efter det senast behandlade uppslaget i registerordningen.
2. `SeriesId` är `E`.
3. `Availability` är `Available`.
4. `bAddedToProject` är `false`.

Tidigare uttryckliga projektbeslut gäller fortfarande. Resten av E63 ska
exempelvis inte återupptas bara för att äldre poster där är `Available`.

Poster med `NotReleased` hoppas över utan separat batch. De kontrolleras igen
endast vid en senare tillgänglighetsrevision eller om WPU publicerar nytt
material. `bRetrieved` och `bAddedToProject` fortsätter att beskriva vårt eget
arbete och ska inte användas som ersättning för dokumentstatus.

Efter den aktuella arbetsmarkören E107-05 hoppas E107-06 över eftersom den
saknar dokument. Nästa faktiska uppladdade och ännu ej införda uppslag är
E167-00.

Datatabeller
------------
Endast `DT_TMOP_Uppslag_REGISTER.json` har ändrats. Övriga sex JSON-tabeller är
byte-identiska med föregående paket. Registerfilen är fortsatt UTF-16LE med
BOM.

Låsta filer
-----------
`DT_TMOP_HistoricalEvents.json` och `TMOP_Grand_IdentityHypotheses.json` är
oförändrade. Alla tidigare README-filer ingår oförändrade i det kumulativa
paketet.
