# Slutmeny vid 23:45

Funktionen är aktiv utan Blueprint-arbete när spelaren är `ATMOPPlayerCharacter`
eller en Blueprint-underklass till den.

Vid den sluttid som anges i **Project Settings > Plugins > TMOP Simulation >
Scenario End Time** stannar klockan på själva sluttiden. Standardvärdet är
23:45:00. HUD och spelinput döljs, spelet pausas och den inbyggda menyn visar:

- SPELA OM FRÅN BÖRJAN
- GÅ TILL HUVUDMENYN
- AVSLUTA SPELET

**Spela om** anropar klockans vanliga `RestartLoop`. Alla system som redan
lyssnar på `OnLoopRestarted` återställs alltså på samma sätt som vid den gamla
automatiska omstarten. Spelarens egen plats och permanenta upptäckter nollställs
inte av den funktionen; det är avsiktligt för tidsloopens kontinuitet.

**Huvudmeny** laddar om den aktuella leveln. Då skapar levelns befintliga
`ATMOPMainMenuIntroDirector` huvudmenyn på nytt. Leveln måste därför fortfarande
innehålla den director som projektets huvudmeny redan använder.

**Avsluta** använder Unreals vanliga `QuitGame`.

Escape, Gamepad B och Start kan inte stänga slutmenyn. Ett av de tre valen måste
göras. Tangentbord, mus och vanlig gamepad-navigation fungerar via Slate.

## Valfri egen design

På `BP_TMOPPlayerCharacter` kan `Loop End Widget Class` sättas till en
Blueprint-underklass av `UTMOPLoopEndWidget`. Om fältet lämnas tomt används den
kompletta inbyggda C++-menyn. Låt `Create Loop End Widget` vara aktiverad.

## Snabbtest

1. Starta spelet och välj nytt spel.
2. Öppna pausmenyns **MOVE IN TIME** och flytta till `23:44:58`.
3. Fortsätt spelet. Klockan ska visa 23:45 och slutmenyn ska öppnas en gång.
4. Kontrollera först **Spela om**: klockan ska bli 23:00 och världen ska få
   `OnLoopRestarted`.
5. Upprepa och kontrollera **Huvudmeny** samt **Avsluta spelet** i ett paketerat
   bygge. PIE kan behandla avslut annorlunda än ett paketerat spel.
