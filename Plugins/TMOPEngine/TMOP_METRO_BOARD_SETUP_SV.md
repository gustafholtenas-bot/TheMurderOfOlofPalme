# Tunnelbaneskyltar – norrgående trafik

Systemet skapar automatiskt en flytande ankomstskylt ovanför alla
`ATMOPHistoricalAnchor` vars AnchorId börjar med `MetroHotorget` eller
`MetroRadmansgatan`. Ankare vars id innehåller `_inside` hoppas över.

Skylten visar station, riktning, linje, destination och nedräkning mot de tre
närmaste tågen. Vid ankomst visas `NU`. Följande disclaimer visas tills en
verifierad södergående körplan har lagts in:

> Södergående tidtabell saknas och visas därför inte.

## Importera tabellen

1. Kompilera projektet så att `FTMOPMetroArrivalRow` blir tillgänglig.
2. Importera `DataTables/09_13/DT_TMOP_MetroArrivals_Northbound.json` som en
   Data Table med radtypen `FTMOPMetroArrivalRow`.
3. Spara asseten som
   `/Game/TMOP/Data/DT_TMOP_MetroArrivals_Northbound`.

Koden har samma 14 rader som inbyggd reservdata. Därför fungerar skyltarna
även innan JSON-filen har importerats. Den importerade tabellen tar automatiskt
över när den finns på den angivna sökvägen.

Tiderna gäller körriktning norrut måndag–fredag. Hötorgets ankomsttid är
T-Centralens avgång + 00:30. Rådmansgatans ankomsttid är T-Centralens avgång
+ 02:10 enligt den tillhandahållna stationskörningen.

