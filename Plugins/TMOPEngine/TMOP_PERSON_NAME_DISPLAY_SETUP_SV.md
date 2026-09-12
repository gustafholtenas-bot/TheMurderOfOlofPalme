# Efternamn som initialer i spelet

Namnvisningen använder nu initialer för efternamn. Regeln gäller alla personer,
även Palme-familjen. Exempel: **Anders Björkman → Anders B.**, **Olof Palme →
Olof P.**, **Jan-Åke Svensson → Jan-Åke S.**

Kopplingen finns i namn ovanför personerna, target-rutan, interaktionsprompten,
dialogens talarnamn och personaktens namnrubrik. Spelarens befintliga vy- och
zoomfunktioner ingår i samma uppdatering.

Adresslistor behåller sin tidigare förkortning av förnamnet och förkortar nu
även efternamnet: **J. Andersson → J. A.** Bekräftade familjer visas exempelvis
som **Familjen A.** Manuellt angivna adressnamn passerar också formateringen.

## Installera

Stäng Unreal, packa upp ZIP-filens `Plugins` och `Scripts` i projektroten och
bygg **Development Editor**. Starta om editorn. Inga datatabeller behöver
importeras om. Paketet innehåller tidigare kamera-, adress-, informationspunkts-
och spökvägsuppdateringar.

De ursprungliga namnuppgifterna och personernas EntityId finns kvar i
datatabellerna för redigering och källkopplingar. Det är visningsnamnen under
Play som förkortas.

## Namn i ett eller flera fält

- När **FirstName** och **LastName** är ifyllda visas hela FirstName följt av
  en initial för LastName, även om efternamnet består av flera ord.
- Om bara **FullName** finns förkortas namndelarna efter det första förnamnet.
  Det undviker att en del av ett dubbelt efternamn visas fullt, exempelvis
  **Elisabeth Lönn Sunde → Elisabeth L. S.** Fyll i FirstName/LastName för att
  styra exakt vilka ord som är förnamn respektive efternamn.
- Rollnamn som **Gärningsmannen** och **Okänd man vid Grand**, samt ensamma
  förnamn som **Kerstin**, behålls. Om ett ensamt namn faktiskt är ett efternamn,
  fyll i **LastName**, exempelvis Bondestam, för att visa **B.**

Detta är formatering av personernas namnvisning. Löptext i förhör, dialoger,
tidningar, källhänvisningar och egna informationspunkter är inte automatiskt
genomsökt eller anonymiserad. Beskrivande namnposter som innehåller hela
meningar behöver också granskas om de nämner andra personer.

## Egna Blueprint-widgetar

Använd **Get In Game Display Name** på `TMOPHistoricalAgent` eller
`TMOPPersonProfileComponent`. För en profilrad kan du använda
**Format Person Name** med FullName, FirstName och LastName.

Befintliga widgetar som läser agentens **DisplayName** får det formaterade
namnet när namnvisningen uppdateras under Play. Den äldre funktionen
**Get Full Name** på profilkomponenten ger också det förkortade namnet i
spelvärlden, och det fullständiga i editorvärlden. Egna widgetar som läser
**Profile.FullName direkt** behöver byta till visningsfunktionen.

## Kontroll

25 fristående C++-fall har kontrollerat bland annat vanliga namn, svenska
bokstäver, bindestreck, sammansatta namn, namngivna par, initialer och rollnamn.
De befintliga adress-testernas förväntningar är uppdaterade till den nya regeln.
**Unreal-bygge och visuell kontroll i Play återstår; Unreal Editor/SDK finns
inte i denna arbetsmiljö.** Kontrollera namn ovanför personerna, target,
dialog, personakt och adresslistor efter ombyggnaden.
