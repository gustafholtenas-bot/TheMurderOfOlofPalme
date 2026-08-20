# TMOP – sex grundkroppar

Det här paketet skapar sex kroppsaliaser: smal, normal och kraftig för man
respektive kvinna. Kroppskategorierna `Athletic` och `Strong` använder den
normala grundkroppen och får sin form genom runtime-morpharna.

## Installation

1. Lägg de två uppdaterade C++-filerna på motsvarande plats i projektets
   `Plugins/TMOPEngine` och kompilera projektet.
2. Kontrollera sökvägarna `MALE_SOURCE_MESH` och `FEMALE_SOURCE_MESH` högst upp
   i `tmop_create_six_base_bodies.py`. Standard är Manny och Quinn.
3. Starta Unreal Editor och aktivera pluginen **Python Editor Script Plugin**.
4. Kör scriptet via Output Log:
   `py "FULL_SÖKVÄG/tmop_create_six_base_bodies.py"`
5. Importera `DT_TMOP_AppearanceAssets_Bodies.csv` till samma DataTable som
   fallback-raderna. Row Struct ska vara `FTMOPAppearanceAssetRow`.
6. Sätt denna DataTable som `AppearanceAssetTable` på
   `TMOPPersonRegistryDirector` och kör valideringen.

## Viktigt om de första mesh-filerna

Scriptet duplicerar Manny/Quinn eller de två mesh-filer du väljer. Det ger rätt
assetstruktur och katalogkoppling direkt, men inte färdiga realistiska
kroppsformer. De riktiga kropparna måste senare ha morph targets med namnen:

- `TMOP_BodyWeight`
- `TMOP_Muscularity`
- `TMOP_HeadScale`
- `TMOP_ShoulderScale`
- `TMOP_TorsoLength`
- `TMOP_ArmLength`
- `TMOP_LegLength`

Tills dessa finns fungerar längd, kapsel, ögonhöjd, namnskylt och
rörelsehastighet. Morph-anrop mot saknade morph targets påverkar inte meshen.

## Automatisk kroppstolkning

- Thin: vikt -0.75
- Slim: vikt -0.40
- Average: neutral
- Athletic: vikt -0.10, muskler 0.45
- Strong: vikt 0.15, muskler 0.80
- Heavy: vikt 0.75, muskler 0.10

Manuellt satta morphvärden tar över standardvärdena.
