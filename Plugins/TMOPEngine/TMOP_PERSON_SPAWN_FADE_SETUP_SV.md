# TMOP – fade vid spawn och despawn

Koden sköter tidslinjen automatiskt. Varje person fadear in vid spawn och fadear
ut innan den förstörs. Alla `SkeletalMeshComponent` och `StaticMeshComponent`
söks upp under animationen, så även profilskapade kläder, hår och burna föremål
följer med.

## Inställningar i nivån

Markera `TMOPPersonRegistryDirector` och öppna:

`TMOP > People > Spawn Fade`

Standardvärden:

- `Enable Person Spawn Fade`: på
- `Enable Person Despawn Fade`: på
- `Person Spawn Fade Duration Seconds`: 0,75
- `Person Despawn Fade Duration Seconds`: 0,75
- `Person Visibility Fade Material Parameter`: `TMOP_VisibilityFade`
- `Write Person Fade To Custom Primitive Data`: på
- `Person Fade Custom Primitive Data Index`: 0
- `Write Person Fade Material Parameter`: av

En valfri `Curve Float` kan anges i `Person Visibility Fade Curve`. Utan kurva
används en mjuk ease-in/ease-out.

## Materialkoppling – rekommenderad metod

Gör detta en gång i varje gemensamt master-material som används av kropp,
ansikte, hår, kläder och burna föremål:

1. Sätt `Blend Mode` till `Masked`.
2. Lägg till noden `Custom Primitive Data`.
3. Sätt dess `Data Index` till `0`.
4. Koppla värdet till `Alpha Threshold` på `DitherTemporalAA`.
5. Koppla utgången från `DitherTemporalAA` till `Opacity Mask`.
6. Spara materialet.

För äldre master-material kan `Write Person Fade Material Parameter` aktiveras.
Då skriver directorn även scalar-parametern `TMOP_VisibilityFade`, som kan
kopplas på samma sätt. Custom Primitive Data är rekommenderat för stora
folkmängder eftersom det inte kräver en unik dynamisk materialinstans för varje
person.

Värdet är `0` när personen är osynlig och `1` när personen är helt synlig.

## Vad koden gör

- Spawn: 0 → 1 under vald tid.
- Timeline `Despawn`: 1 → 0, därefter förstörs personen.
- Person eller grupp som når en `MapExit`: samma fade-out används.
- Namnskylt och pratbubbla döljs under övergången.
- AI-rörelse, collision och interaktion stängs av när despawn börjar.
- Vid omstart, tidshopp och misslyckad spawn rensas gamla aktörer direkt för att
  inte lämna dubbla personer i världen.

## Test

Skapa en person med `Spawn`, vänta några sekunder och lägg sedan en `Despawn`-
rad. Kör PIE. Personen ska lösas in under 0,75 sekunder och lösas ut under 0,75
sekunder vid despawn-tiden.
