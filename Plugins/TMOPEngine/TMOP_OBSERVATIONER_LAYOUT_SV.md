# Mina observationer – kortvy enligt observationer.jpg

Paketet är kumulativt och innehåller de tidigare levererade ändringarna för
Teoribygge och anteckningsboken. Kopiera `Plugins` över projektets befintliga
`Plugins`, med Unreal Editor stängd. Behåll övriga pluginfiler. Bygg sedan
Development Editor / Win64 normalt; nya sparfält kräver Unreal Header Tool.
Paketet innehåller källkod, inte färdigkompilerade DLL-filer.

## Utseende

- Vit sida med centrerad rubrik **Mina observationer**.
- Två grå, svart inramade kolumner: **PERSONER** och **FORDON**.
- Varje kolumn har egen rullning. Menyn till vänster är oförändrad.
- Mörkare grå kort med vit text och svarta ramar.
- Modellbild till vänster, namn/händelser/signalement/**Sedd av** i mitten.
- Personkort har separat bildruta till höger för fantombilder och skisser.
  Pilar bläddrar mellan flera bilder; bildtext och källhänvisning visas under.
- Långa texter radbryts och kortets höjd följer innehållet.
- Sidan har en fast designyta som skalas ned i små eller delade spelvyer,
  så textkolumnerna inte pressas ihop mellan bilderna.

De fem tidigare personkategorierna behålls, även om exempelbilden bara visar tre:
**Gärningsmannen**, **Mycket misstänkta**, **Män med walkie-talkies**,
**Andra misstänkta**, **Mindre misstänkta**.

Fordon delas i **Mycket misstänkta** och **Mindre misstänkta**. På
`TMOPVehicleBase` finns nya inställningar under **TMOP → Vehicle → Notebook**:
`Notebook Suspicion` och `Notebook Signalement`. De kan anges på fordonets
Blueprint/instans före insamling. Utan klassificering används **Mindre misstänkta**.
Detta lägger inte till motsvarande fält i fordonstabellens specialeditor.

## Modellbilder

Rutorna visar statiska ögonblicksbilder av de faktiska 3D-modellerna, inklusive
deras synliga meshdelar och material. Bilderna skapas med en engångsrendering av
enbart modellen och komprimeras i sparfilen. Det är inte roterbara eller animerade
3D-vyer. Ingen separat widget, kamera eller render target behöver läggas ut i banan.

PNG-bilden finns kvar när personen/bilen har despawnat och efter spara/ladda.
Bildfångst kan ge en kort engångskostnad när en ny akt öppnas. Bilder renderas
inte fortlöpande. Listorna skapar bildresurser endast för de kort som visas.

Äldre sparfiler behåller sina insamlade observationer. Text och bildreferenser
kompletteras från registren. En saknad modellbild kompletteras när kortet visas,
om originalfiguren fortfarande finns och kan renderas. Annars står det
**Modellbild saknas** tills figuren inspekteras igen. Ingen ersättningsmodell gissas.

## Händelser, signalement och vittnen

Händelser hämtas från de direkta eller länkade observationerna i ObservationDirector,
fram till observationens insamlingstid. Den tidigare sammanfattningen behålls som
reserv när ingen sådan händelsetext finns. Insamlingstid visas separat, så den inte
förväxlas med tiden för en beskriven händelse.

Signalement använder befintliga källtexter för hår, ansikte, kroppsbyggnad och kläder,
samt vittnenas signalementsammanfattningar. För fordon används även det nya
signalementsfältet och registreringsnumret när de finns. **Sedd av** använder
vittneskopplingar i de tillgängliga observationerna och samma namnformatering som
personakterna. Saknade uppgifter markeras som saknade; ingen text från exempelbilden
läggs in som historisk uppgift.

För bilder till höger: använd personens **Evidence Images** i People-data med
typen **Phantom Image**, **Sketch**, **Reconstruction** eller **Other**.
Bildtext och källhänvisning följer med. Vanliga referensfotografier ersätter inte
fantombilder automatiskt.

## Sparning och Teoribygge

Modellbild, signalement, vittnesnamn, bildreferenser och fordonsklassificering sparas
med respektive lokal spelares observationer. Upprepad inspektion skapar inte en
dubblett eller en extra insamlingsnotis, men kan komplettera en saknad modellbild.
Teoribygge använder samma insamlade personer och fordon som tidigare.

## Verifiering

De tre körbara kontrollerna för trädmodell och menynavigering passerar efter ändringen.
Sparfilstestet `TMOP.Notebook.DeduplicationAndSaveRoundTrip` har utökats med
signalement, vittnesnamn, bilddata och bildreferenser.

Unreal Engine saknas i arbetsmiljön. Pluginen har därför **inte kompilerats eller
visuellt testats i Unreal här**, och Unreal Automation-testerna har inte körts.
Kontrollera efter byggning en person med flera skisser, ett observerat fordon,
långa texter, separat rullning och spara/ladda efter att figuren har despawnat.

Bildkodens motor-API: [Epic – FImageUtils](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/FImageUtils).
