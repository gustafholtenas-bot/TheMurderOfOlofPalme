# TMOP – första obscured-assets

Detta paket skapar den första materialdelen av `UNKNOWN_*`-systemet.

## Körning

1. Aktivera Unreal-pluginen **Python Editor Script Plugin**.
2. Starta om Unreal Editor.
3. Välj **Tools → Execute Python Script**.
4. Kör `tmop_create_unknown_appearance_assets.py`.
5. Kompilera appearance-koden innan du skapar datatabellen.
6. Importera `DT_TMOP_AppearanceAssets_Unknown.csv` som en DataTable med radtypen `FTMOPAppearanceAssetRow`.
7. Tilldela tabellen till `AppearanceAssetTable` i `TMOPPersonRegistryDirector`.

Skriptet skapar:

- `/Game/TMOP/Characters/Appearance/Materials/Unknown/M_TMOP_Obscured`
- åtta materialinstanser för ansikte, hår och klädslots,
- mappar för kommande fallback-mesher och datatabeller.

Materialet är texturlöst och unlit med en mjuk Fresnel-gradient. Ansikts- och
kläddetaljer försvinner medan silhuetten fortfarande går att läsa. Det är
billigare och stabilare än en äkta skärmbaserad blur för stora folkmängder.

## Viktigt om mesherna

CSV-radernas `Mesh`-fält är avsiktligt tomma. Nästa steg är att skapa eller importera fallback-mesher som använder exakt samma skelett och benstruktur som kroppssystemet. När de finns fyller vi in deras `/Game/...`-sökvägar i datatabellen.

Scriptet raderar aldrig asset-filer. Om master-materialet redan finns byggs dess
expressions om till den senaste kumulativa versionen och materialinstanserna
uppdateras.
