# Hiss och rulltrappa – installation

## Anchors

Importera eller placera följande fyra `TMOPHistoricalAnchor`:

- `D21659_TUNNELGATAN_ELEVATOR_LOWER`
- `D21659_TUNNELGATAN_ELEVATOR_UPPER`
- `D21659_TUNNELGATAN_ESCALATOR_LOWER`
- `D21659_TUNNELGATAN_ESCALATOR_UPPER`

Placeringsfilen innehåller preliminära ändpunkter baserade på befintligt nedre hissankare och trappkrön. Finjustera X/Y i Unreal mot den faktiska hissdörren och rulltrappans första/sista steg.

## Hiss

1. Placera en `TMOPVerticalTransport`.
2. Sätt `Transport Type = Elevator`.
3. Lower/Upper Anchor ID = hissankarna ovan.
4. Rekommenderat: Boarding Delay 1.5 s, Travel Duration 8 s.
5. Tilldela hisskorgen till `Moving Visual` om korgen ska röra sig visuellt.

## Rulltrappa

1. Placera en andra `TMOPVerticalTransport`.
2. Sätt `Transport Type = Escalator`.
3. Lower/Upper Anchor ID = rulltrappsankarna ovan.
4. Rekommenderat: Boarding Delay 0 s, Travel Duration 15 s.
5. Ett valfritt steg/root kan tilldelas `Moving Visual`.

Historiska agenter använder transporten automatiskt när två anslutna ändpunkter ligger efter varandra i `PassAnchorIds`/`TargetAnchorId`. För spelaren kan `Request Transport` anropas från hissknapp, dörr eller rulltrappans overlap-Blueprint.
