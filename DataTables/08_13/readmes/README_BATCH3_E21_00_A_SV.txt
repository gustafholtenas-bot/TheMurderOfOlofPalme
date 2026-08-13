TMOP – Batch 3 – E21-00-A
=========================

Källa
-----
WPU: https://wpu.nu/wiki/Uppslag:E21-00-A
Original: Pol-1986-04-08_E21-00-A_Jan_Nilsson.pdf
SHA-256: a6902c053c0252fcf5d45be173636140438d6fe5b907c618faecf34c570d489b

Källgranskning
--------------
Original-PDF:en består av 16 sidor. Samtliga originalblad har granskats visuellt och jämförts med PDF-textlagret samt WPU-renskriften. Originalet styr vid avvikelse. Personnummer, adress och telefonnummer har inte kopierats.

Förhöret hölls 1986-04-08 kl. 13.00–13.35 med Jan Nilsson av kriminalinspektör Lars Jonsson utan förhörsvittne. Tiderna är procedurdata.

Viktig källrättelse
-------------------
E21-00-A klargör att E21-00:s löpare sprang österut uppför Tunnelgatan mot Malmskillnadsgatan. Den tidigare nyskapade västliga ID-benämningen var därför fel. Följande kontrollerade namnbyten har gjorts utan att skapa dubbla roller:

- UNKNOWN_RUNNING_MAN_WEST_TUNNELGATAN_E21_00 → UNKNOWN_RUNNING_MAN_EAST_TUNNELGATAN_E21_00
- OBS_E21_00_JAN_SEES_UNKNOWN_MAN_RUN_WEST → OBS_E21_00_JAN_SEES_UNKNOWN_MAN_RUN_EAST

Löparen har inte slagits ihop med THE_KILLER. Jan såg honom endast bakifrån, visste inte vem han var och sade att han möjligen verkade följa efter någon.

Personer och grupper
--------------------
- Den tidigare kollektiva kundrollen UNKNOWN_LIMOUSINE_CUSTOMERS_E21_00 har ersatts av exakt fyra separata kvinnliga passagerare, eftersom E21-00-A uttryckligen anger fyra.
- Passagerarna saknar namn och säten; Jans uppfattning om deras ursprung har inte lagts som faktisk nationalitet.
- Separata roller har skapats för Aladdin-vakten, taxiföraren och kunden i betalningsbråket, tre olika kvinnor vid offret, Limousine Service-operatören, två unga män mitt emot Dekorima, den blodige mannen ur polisbussen, den lugnt gående Luntmakargatanmannen och en kollektiv oidentifierad polisnärvaro.
- CPR-kvinnan, kvinnan bredvid offret och den hukade kvinnan hålls som tre skilda personer.
- Den blodige mannen har inte slagits ihop med Stefan Glantz.
- Luntmakargatanmannen har inte slagits ihop med löparen, THE_KILLER eller Lars Jeppsson.
- Gruppen Jan + fyra passagerare har uppdaterats och en separat tvåmannagrupp mitt emot Dekorima har skapats.

Fordon
------
- Jans befintliga fordon källbeläggs nu som en svart Mercedes 300. Registreringsnummer saknas.
- En separat taxi för betalningsbråket vid Aladdin har skapats.
- En separat, osäker U-svängstaxi vid Sveavägen har skapats utan förarperson, eftersom Jan aldrig såg föraren.
- En separat oidentifierad polisbuss vid Jans återkomst har skapats.
- Inget av de nya fordonen har slagits ihop med kända taxi- eller polisfordon utan uttryckligt stöd.

Observationer och tider
-----------------------
Tolv E21-00-A-relevanta observationer ingår: två korrigerade/kompletterade E21-00-observationer och tio nya observationer för taxin, kvinnorna, passagerarna, de två unga männen, den blodige mannen, polisbussen och Luntmakargatanmannen.

Jan uppskattade första passagen till 23.21–23.25 och gissade själv 23.23 utan att ha tittat på klockan. Observationerna använder därför 23.23 som WitnessUncertain-rekonstruktionsankare, inte som exakt källtid. Återkomsten och Luntmakargatanmannen ligger omkring 23.35 och är också osäkra.

Avgränsningar
-------------
- Befintliga person- och fordonstidslinjer, lane-ID:n, startavstånd, UE-transformer och ModelData är oförändrade.
- Jans fullständiga rutt behöver en separat samlad lane-revision.
- Säkra projektankare saknas för restaurang Aladdin, King Creole och Embassy; därför har inga påhittade ankarpositioner eller passagerarsäten skapats.
- Negativa uppgifter om walkie-talkie och övriga personer/bilar har inte skapat entiteter.

Validering
----------
- Exakt diffomfång och de tre kontrollerade ersättningarna har verifierats.
- Inga gamla ID-referenser återstår och inga dubbla ID:n har skapats.
- Inga brutna E21/E21-00/E21-00-A-referenser.
- Befintliga person- och fordonstidslinjer oförändrade.
- DT_TMOP_HistoricalEvents.json och TMOP_Grand_IdentityHypotheses.json bibehållna oförändrade.
- Paketets JSON-filer levereras som UTF-16LE med BOM för Unreal Engine-import.
