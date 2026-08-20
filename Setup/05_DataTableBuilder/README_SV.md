# Automatisk Appearance DataTable-byggare

Kör detta sist, efter de tre asset-scripten:

`py "FULL_SÖKVÄG/tmop_build_appearance_data_table.py"`

Scriptet:

- hittar alla `DT_TMOP_AppearanceAssets_*.csv` under `Setup`;
- kräver identiska kolumner och unika `Name`/`CatalogId`;
- skapar `DT_TMOP_AppearanceAssets` om den saknas;
- skapar en tidsstämplad säkerhetskopia före varje uppdatering;
- bevarar egna rader vars namn inte finns i de levererade CSV-filerna;
- ersätter de 64 systemhanterade raderna med paketets senaste version;
- kopplar tabellen till alla laddade `TMOPPersonRegistryDirector`-aktörer.

Målasset:

`/Game/TMOP/Characters/Appearance/Data/DT_TMOP_AppearanceAssets`

Säkerhetskopior:

`/Game/TMOP/Characters/Appearance/Data/Backups`

Om du har ändrat en systemhanterad rad manuellt bör den få ett eget nytt
Row Name. Annars ersätts den av nästa kumulativa uppdatering. Alla helt egna
rader bevaras automatiskt.
