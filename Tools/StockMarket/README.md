# Börsen före och efter

Ny sida i pausmenyn, efter **Grupperingar i världen / Grupperingar i Sverige**:
**BÖRSEN FÖRE OCH EFTER**. Sidan jämför 28 februari med 3 mars 1986.

## Installation

1. Stäng Unreal Editor.
2. Packa upp ZIP-filen i projektets rot, där `TheMurderOfOlofPalme.uproject` finns.
   Mapparna `Plugins` och `Tools` ska slås samman med projektets mappar.
3. Generera projektfiler igen om utvecklingsmiljön kräver det och bygg **Development Editor**.
   Använd en full kompilering efter enum-/headerändringen, inte enbart Live Coding.
4. Starta spelet och öppna pausmenyn. Välj **BÖRSEN FÖRE OCH EFTER**.

Ingen ny Blueprint, DataTable eller widget behöver skapas i Content Browser.
Den befintliga C++-pausmenyn skapar sidan. En helt egen Blueprint-meny som ersätter
denna pausmeny behöver förstås kopplas till motsvarande sida separat.

## Innehåll och kontroller

- 35 indexrader för 19 marknader och ett världsindex. 34 förändringar kan beräknas.
- Sökruta för marknad/index, knapp som växlar region och knapp som växlar mellan
  alfabetisk ordning, störst uppgång och störst nedgång.
- Filter för rader med båda användbara värdena och för att dölja branschindex.
- Välj rad med mus eller tangentbord/handkontroll för källor och kommentarer.
  Knappen vid källan öppnar den aktuella tidningen i webbläsaren.
- Grönt/rött visar riktning. Tecken och procenttal finns också, så färgen inte är
  den enda informationsbäraren. Okända förändringar visas sist vid procentsortering.
- Den svarta bakgrunden förbättrar läsbarheten. Raderna radbryts efter panelens
  bredd. Avdelaren mellan lista och detaljer går att dra.
- Varje lokal spelare har eget urval, filter och scrolläge. Sidan ändrar inte klockan
  eller spelets befintliga paus-/multiplayerregler.

## Data och källor

`Plugins/TMOPEngine/Content/StockMarket/markets.json` innehåller hela jämförelsen.
Filen inkluderas i paketerade byggen genom `TMOPEngine.Build.cs`. JSON-filen läses
när sidan öppnas. Stäng och öppna sidan efter dataändringar i editorn.

Värdena kommer från den granskade FT-jämförelsen. Källor: *Financial Times*,
4 mars 1986, s. 42–43 och 46, samt 5 mars 1986, s. 43 och 46.
USA och Toronto använder måndagens slutvärden från det senare numret.
Danmarks motsägelsefulla tabellvärden är markerade. Montréal behålls i listan
men får inget måndagsvärde eftersom källans skalor inte har kunnat förenas.
Japan hade också lördagshandel. Två observationer innebär ingen beräknad
statistisk avvikelse eller slutsats om mordets ekonomiska effekter.

Ingen aktiekurs, totalavkastning, handelsvolym, tidsserie eller fördelning över
enskilda köpare finns ännu i denna version. Indexpunkter ska inte summeras över
olika index. Världsindexets valutabas är inte fastställd i det använda utdraget.

### Schema 1

- `before_date`, `after_date`: verkliga observationsdatum i `YYYY-MM-DD`, inte tidningens utgivningsdatum.
- `sources`: stabilt `id`, `title`, `published` och `https`-URL.
- `entries`: stabilt `id`, `region`, `kind`, `status`, `decimals`, `before`, `after`, flerspråkiga `market`, `name`, `notes` och `citations`.
- Regioner: `europe`, `north_america`, `asia_pacific`, `africa`, `world`.
- Kategorier: `market`, `sector`, `world`.
- Status: `daily`, `close`, `provisional`, `source_conflict`, `missing`.
- En saknad/osäker observation är `null`, aldrig `0`. En faktiskt noterad nolla
  får ligga kvar, men ett nollvärde före perioden ger ingen procentberäkning.
- Procent = `(after - before) / before * 100`. Resultat beräknas av koden och ska
  inte lagras separat i JSON. Det förhindrar att procenten blir inaktuell.
- Lägg till rader med samma jämförelsedatum genom att följa befintlig struktur.
  Sidan har inga datumval utan underliggande data. För flera jämförelseperioder
  behöver datamodellen och periodvalet utökas tillsammans.

## Svenska och engelska

Sidan följer spelets språkval. UI-strängar finns i `TMOPMenuTranslations.inl`.
Innehållets språk ligger i `markets.json`, med samma källkontroll som atlasen:

```json
{"sv":"Svensk text","en":"English text","en_source":"Svensk text"}
```

Ändras `sv` utan att engelskan granskas och `en_source` uppdateras visas svenskan.
Externa språkpaket kan överstyra fälten via `TMOP_StockMarket/<id>/<field>`
genom befintlig `FTMOPLocalization::TableText`. Metodtexten har rad-ID
`comparison` och fält `method`. Nya språk måste även finnas i spelets språkval.

## Verifiering

```sh
python Tools/StockMarket/validate_markets.py
```

Valideringen kontrollerar data, källhänvisningar, datum, språkversioner och alla
menysträngars engelska motsvarigheter. Koden innehåller Unreal-automationstester:
**TMOP.StockMarket.Change**, **TMOP.StockMarket.Data**, **TMOP.StockMarket.LanguageFallback**.

Unreal Editor/UnrealBuildTool finns inte i leveransmiljön. C++-kompilering,
Unreal-tester och visuell körning i spelet är därför inte verifierade här.

Kontrollera i editorn och därefter i en paketerad Development-build:

1. Öppna sidan och kontrollera Stockholm +0,72 %, Dow Jones −0,72 % och Madrid +4,26 %.
2. Välj Montréal: ”Saknas”, källavvikelse och ingen påhittad procent. Välj Danmark och läs källkonflikten.
3. Sortera i båda riktningarna, sök på Sverige och välj ett filter som ger noll träffar.
4. Växla svenska/engelska. Kontrollera både UI, länder, anmärkningar och källornas sidetikett.
5. Öppna/stäng sidan och byt pausmenysida upprepade gånger. Prova smalt fönster
   och två lokala spelare med olika filter. Kontrollera att bakgrund och radbrytning är läsbara.
6. Öppna en källänk och testa tangentbords-/handkontrollfokus i den befintliga menyinmatningen.
7. Paketera spelet och kontrollera att börsdata fortfarande laddas utan editorn.
