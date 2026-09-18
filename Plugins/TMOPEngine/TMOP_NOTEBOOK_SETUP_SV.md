# Mina observationer

Stäng Unreal, slå ihop paketets Plugins-mapp med projektets Plugins-mapp
och bygg projektets Editor-target. Behåll det tidigare pluginet; detta är
en uppdatering med källfiler, inte ett komplett plugin.

## Spela

Öppna en grön persons informationsruta med interaktionsknappen (E som standard).
När rutan stängs läggs personen till i Mina observationer och en fyra sekunder
lång notis visas på den lokala spelarens skärm. E, Stäng, Escape och motsvarande
konfigurerade knappar använder samma stängningsflöde.

Samma EntityId samlas bara en gång. Vanliga vittnen och vanliga poliser ingår
inte. Kategorier OBSERVED_* och SUSPECT, samt OBSERVED_*-identiteter, använder
samma urval som gröna namn. THE_KILLER omfattas också och får grönt namn.

Anteckningsboken visar namn, tidpunkt och den observationssammanfattning som
fanns när personen inspekterades. Informationen är sparad även om NPC:n
despawnas. Den lägger inte till personens framtida tidslinje.

## Kategorier

People Editor → Agent Info → Kategori i Mina observationer.

- Automatisk: THE_KILLER → Skytten; aktuellt Fleeing-tillstånd → Högt suspekt;
  registrerat föremål med WalkieTalkie-grepp → Män med walkie-talkies;
  annars Andra suspekta.
- Välj en uttrycklig kategori för att klassningen ska gälla oavsett vad personen
  gör vid inspektion. Använd exempelvis Mindre suspekta för kategori 5.
- Klassningen gör inte ett vanligt vittne insamlingsbart. Personen måste först
  omfattas av det gröna urvalet.
- THE_KILLER ligger alltid under Skytten.

Detta paket omklassificerar inga datatabellsrader. Gör önskade manuella
klassningar i People Editor och spara tabellen.

## Sparning och kontroll

Samlingen sparas i varje lokal spelares data vid vanlig sparning och återställs
vid laddning. Äldre sparfiler utan fältet öppnas med tom samling.
Detta skapar ingen separat automatisk sparning.

Unreal Automation: kör TMOP.Notebook (EligibilityAndClassification och
DeduplicationAndSaveRoundTrip). Testerna ingår men har inte körts här eftersom
Unreal Engine inte finns i arbetsmiljön. Statiska menykontroller har gått igenom.

Kontrollera i PIE: inspektera ett vittne (ingen post), en grön person (en post
och en notis), samma person igen (ingen ny post/notis), spara, samla ytterligare
en person och ladda (endast de sparade posterna återställs). Kontrollera också
E-stängning och att två lokala spelare har separata listor och notiser.
