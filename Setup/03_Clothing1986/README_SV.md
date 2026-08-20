# TMOP – första modulära klädpaketet för 1986

Paketet förbereder 25 katalogvarianter och 12 färgmaterial för vinterkläder i
Stockholm 1986. Det innehåller inte upphovsrättsskyddade mesh-filer.

## Det som skapas direkt

- Ett enkelt master-material för kända kläder.
- 12 färginstanser: svart, kolgrå, mörkblå, marinblå, mörkgrå, grå, brun,
  mörkbrun, beige, oliv, röd och vit.
- Katalograder med tidsperiod, ålder, typ, färg och matchningstaggar.
- Målsökvägar och standardnamn för 19 modulära mesh-former.

## Körning

1. Öppna `tmop_create_1986_clothing_assets.py`.
2. Fyll i sökvägarna i `SOURCE_MESHES` när motsvarande mesh finns.
3. Säkerställ att mesh-filerna använder samma skeleton som TMOP-kropparna.
4. Aktivera **Python Editor Script Plugin** i Unreal.
5. Kör via Output Log:
   `py "FULL_SÖKVÄG/tmop_create_1986_clothing_assets.py"`
6. Importera `DT_TMOP_AppearanceAssets_Clothing1986.csv` till projektets
   Appearance DataTable med Row Struct `FTMOPAppearanceAssetRow`.
7. Kör `Validate Appearance Asset Table` på `TMOPPersonRegistryDirector`.

Tomma `SOURCE_MESHES` hoppas över och rapporteras. Materialen skapas ändå, så
scriptet kan köras nu och sedan köras igen varje gång en ny mesh har importerats.

## Mesh-former som behövs

- Lång och kort rock
- Standardjacka, skinnjacka, parkas och seglarjacka
- Skjorta och tröja
- Raka byxor, jeans och kostymbyxor
- Skor, stövlar och vinterstövlar
- Handskar
- Stickad mössa, pälsmössa, brättad hatt och keps

Kända kläder får vanliga material. Endast helt okända signalementsdelar använder
det tidigare obscured-materialet, vilket gör skillnaden tydlig för spelaren.
