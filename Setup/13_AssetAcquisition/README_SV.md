# Assetanskaffning och produktionsordning

Den konkreta arbetslistan finns i
`Planning/TMOP_Appearance_Asset_Plan.xlsx`, på bladet **Acquisition List**.
Listan genereras från runtime-katalogens 64 rader och innehåller 39 unika
skeletal meshes samt 23 unika material.

## Prioritet

- **P0 Critical:** obscured-fallbackmaterialen och de sex grundkropparna.
- **P1 Core:** de vanligaste ansiktena, håren och vinterkläderna som behövs för
  att snabbt få användbar variation i folksamlingar.
- **P2 Expansion:** accessoarer, mer specifika plagg och ytterligare varianter.

## Arbetsflöde per asset

1. Fyll i `Source Candidate` och en direkt `Source URL`.
2. Kontrollera licensen innan filen köps, hämtas eller bearbetas.
3. Sätt `Commercial Use` till `Yes` först när kommersiell användning är tydligt
   tillåten för spelet.
4. Spara licenstext, kvitto eller skärmbild och skriv platsen i
   `License / Proof`.
5. Ändra `Status` från `Not Acquired` genom produktionsflödet till `Ready`.
6. Importera asseten till exakt den sökväg som står i `Unreal Target Path`.
7. Kör `Setup/12_AssetReadinessAudit` och åtgärda fel innan status sätts till
   `Ready`.

Samma mesh eller material kan användas av flera katalograder. Kolumnerna
`Catalog Rows` och `Reuse Count` visar därför hur stor täckning varje färdig
asset ger.
