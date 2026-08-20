# Automatisk signalementsmatchning

Den här etappen tolkar svensk fri signalementstext och omvandlar den till samma
taggar som används i `DT_TMOP_AppearanceAssets`.

Exempel:

- `lång mörkblå yllerock` → `Long`, `DarkBlue`, `Dark`, `Wool`, `Coat`
- `svart stickad mössa` → `Black`, `Dark`, `KnitCap`, `Cap`
- `bruna skinnhandskar` → `Brown`, `Leather`, `Gloves`
- `mörk täckjacka` → `Dark`, `Jacket`

Systemet använder även `HairColorCategory`, `OuterwearCategory` och
`HeadwearCategory` när fritexten är tom. Handskar kan hittas i
`OtherCharacteristics` eftersom den äldre persontabellen saknar ett eget
signalementsfält för handskar.

`TMOP_Appearance_Swedish_Tag_Mapping.csv` dokumenterar den nuvarande ordlistan.
Själva runtime-ordlistan finns i `TMOPAppearanceResolver.cpp` och är avsiktligt
deterministisk: samma person och samma källuppgifter ger samma assetval.
