# Klädpassning, Leader Pose och kroppsmaskering

Runtime-systemet gör nu tre saker när ett plagg väljs:

1. Plaggets mesh får kroppens `Leader Pose Component`, så animationerna följer
   samma skelett.
2. Kroppens sju TMOP-morphvärden skickas vidare till alla modulära mesh-delar.
   Morph targets som saknas ignoreras säkert av Unreal.
3. Alla valda plaggs `HiddenBodyRegions` kombineras och skickas till kroppens
   dynamiska material.

## Regionvärden i assettabellen

- Head = 1
- Neck = 2
- Torso = 4
- Arms = 8
- Hands = 16
- Hips = 32
- Legs = 64
- Feet = 128

Värden kan kombineras. Exempel:

- Jacka eller tröja: Torso + Arms = 12
- Handskar: Hands = 16
- Byxor: Hips + Legs = 96
- Skor: Feet = 128

De 25 medföljande klädraderna har redan fått dessa värden.

## Materialparametrar på kroppens material

Koden sätter följande scalar-parametrar till 0 eller 1:

- `TMOP_HideHead`
- `TMOP_HideNeck`
- `TMOP_HideTorso`
- `TMOP_HideArms`
- `TMOP_HideHands`
- `TMOP_HideHips`
- `TMOP_HideLegs`
- `TMOP_HideFeet`

För att den visuella maskeringen ska synas måste det slutliga kroppsmaterialet
använda parametrarna tillsammans med regionmasker och ansluta resultatet till
`Opacity Mask`. Materialet ska då använda blend mode `Masked`.

Koden och katalogvärdena är alltså färdiga; regiontexturerna eller
vertexmaskerna skapas när de riktiga kroppsmaterialen tas fram.
