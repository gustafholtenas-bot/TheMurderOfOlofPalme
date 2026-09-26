# Flaggor och blockfärger — 28 februari 1986

## Installera denna uppdatering

1. Stäng Unreal Editor.
2. Packa upp ZIP-filen i projektroten. Slå ihop `Plugins/` och `Tools/` med projektets mappar. Paketet innehåller atlasens tidigare kod och innehåll plus denna uppdatering; det är inte hela spelprojektet.
3. Generera projektfiler vid behov och bygg projektets **Development Editor**-mål. Denna uppdatering ändrar C++ och Build.cs, så det räcker inte att byta `world.json` eller använda enbart Live Coding.
4. Öppna **Grupperingar i världen**. **Flaggor på globen** och **Blockfärger 1986** är aktiverade från början. Inga nya Blueprint-widgets, material, DataTables eller manuella PNG-importer behövs.
5. Paketerade spel måste byggas/paketeras om. De fyra nya visuella filerna följer med genom UFS/RuntimeDependencies.

## Resultat

- Alla atlasens 33 landsposter har en flagga i listan, detaljpanelen och på globen. Regionen Kurdistan och politiska aktörer får inga påhittade nationalflaggor.
- Historiska varianter används för Sovjetunionen, Afghanistan, Sydafrika, Irak och Syrien. Honduras använder den äldre blå varianten.
- Flaggan är klickbar. När flera flaggor ligger tätt provas närliggande lediga lägen med en kort linje tillbaka till den geografiska punkten. Hover ändrar inte placeringen. Vid mycket tät koncentration kan överlappning fortfarande förekomma; listan och zoom finns kvar.
- Landytor får blå, röda eller grå färger, mörkt hav och landgränser. Det valda landets gräns framhävs. Flaggor, texter och konfliktmarkörer ligger ovanpå landytorna.
- Baksidan klipps bort för både fyllningar och gränser. Landytorna följer samma koordinater, rotation, zoom och Slate-klippning som kartans övriga innehåll.
- Flaggor och blockfärger kan stängas av separat. Med blockfärger av återkommer den befintliga sfären/materialet och kustlinjelagret.
- Senare-historikreglaget ändrar inte detta fasta kartdatum. Det är alltid flaggor och klassning för 28 februari 1986.

## Vad färgerna betyder

Indelningen är en **redaktionell förenkling av regeringars politiska och säkerhetspolitiska orientering**, inte ett mått på befolkningens åsikter eller en lista över alla formella allianser.

| Färg | Klassning |
|---|---|
| Blå | NATO-medlemmarna 1986 samt ett urval västanknutna regeringar och administrerade territorier. |
| Röd | Warszawapaktens medlemmar 1986 samt ett urval sovjetanknutna regeringar. |
| Grå | Neutrala, alliansfria och övriga/inte entydigt klassificerade områden. Grått betyder **inte automatiskt neutralitet**. |

Sverige, Finland, Schweiz, Österrike och Irland är grå. Kina, Jugoslavien och Albanien görs inte automatiskt röda därför att de hade kommunistiska regeringar. Iran och Irak har inte givits någon entydig blocktillhörighet. Syriens omfattande sovjetiska stöd framgår av förklaringen, men landet ligger i övrigt/alliansfritt. Nicaraguas röda färg gäller sandinistregeringen, inte Contras. För varje befintlig landspost visas klassningens förklaring i detaljpanelen.

Utanför de uttryckligen klassificerade länderna är grått standard. Det är ingen heltäckande rangordning av alla staters utrikespolitiska närhet. Fler klassningar kan granskas och ändras i datafilen utan C++-ändring.

## Filer att redigera

Alla runtimefiler ligger i `Plugins/TMOPEngine/Content/WorldAtlas/`.

| Fil | Innehåll |
|---|---|
| `world.json` | Befintliga 134 innehållsposter, personuppgifter, händelser, konflikter och källor. Oförändrad av denna visuella uppdatering. |
| `alignments_1986.json` | Land-/territorie-ID, `bloc` (`west`, `east`, `other`), klassningsgrund och förklaring på svenska/engelska. |
| `land_1986.json` | Generaliserade landytor: koordinater `[longitud, latitud]`, triangelindex och gränslinjer. |
| `flags_1986.json` | Koppling mellan land-ID och flaggans pixelrektangel i bilden, med källuppgifter. |
| `flags_1986.png` | En gemensam bild med alla flaggikoner. Laddas av C++ vid öppning av menyn; ingen Content Browser-import. |

Exempel: ändra `bloc` för en rad i `alignments_1986.json` och öppna menyn på nytt. Uppdatera även förklaringen och källunderlaget. För översättning används `sv`, `en`, `en_source`; en gammal engelsk text faller tillbaka till svenska om svenska originalet ändras. Externa språkpaket kan använda tabell-ID `TMOP_WorldAtlasAlignments`, rad-ID landets ID och fält `description`.

Flaggorna visas endast för länder som redan har en landspost i `world.json`. Landfärger täcker 217 länder/territorier/kartområden, även där någon klickbar landprofil ännu inte har skrivits. En landyta skapar inte automatiskt en ny innehållspost.

## Kartografisk noggrannhet

Underlaget är Natural Earth 1:50m, bearbetat till en **generaliserad historisk översikt**. Sovjetunionen, Jugoslavien, Tjeckoslovakien, Sudan före Sydsudans självständighet samt Etiopien före Eritreas självständighet är sammanslagna. Tyskland och Jemen är delade.

Detta är inte en exakt digitalisering av samtliga gränsavtal den 28 februari 1986. Öst-/Västtyskland rekonstrueras från moderna delstatsytor; mindre historiska gränsjusteringar och Västberlins separata status återges inte i denna skala. Nord-/Sydjemens gräns är en ungefärlig rekonstruktion från guvernement; Al Dali-området är särskilt förenklat. Övriga yttergränser följer det moderna generaliserade basunderlaget och kan innehålla senare mindre gränsändringar. Östtimor, Namibia och palestinska områden är kartområden med egna konturer, inte påståenden om självständiga stater 1986. Hav, sjöar och små kustöar är förenklade.

Om exakta lokala gränser krävs bör de bytas mot särskilt granskade polygoner innan kartan används för sådana slutsatser. Renderingen kan använda mer detaljerad geometri utan att ändra innehållsposterna.

## Bygga om grafikdata

`build_appearance.py` är ett utvecklingsverktyg; användaren av spelet behöver inte Python.

```bash
python -m pip install shapely numpy mapbox-earcut cairosvg Pillow
python Tools/WorldAtlas/build_appearance.py --countries ne_50m_admin_0_countries.geojson --states ne_10m_admin_1_states_provinces.geojson
```

Skriptet skriver om geometri och flaggbild. Det behåller en befintlig klassningsfil; `--write-alignments` återställer uttryckligen den redaktionella grundversionen. Flagornas SVG-original, licens och källregister finns i `SourceAssets/flags/`.

Natural Earth-indata:

- https://raw.githubusercontent.com/nvkelso/natural-earth-vector/master/geojson/ne_50m_admin_0_countries.geojson
- https://raw.githubusercontent.com/nvkelso/natural-earth-vector/master/geojson/ne_10m_admin_1_states_provinces.geojson
- Licens: public domain, https://www.naturalearthdata.com/about/terms-of-use/

Inga data från CShapes ingår. Flag-icons v6.15.0 används under MIT; licenstext följer med. De fyra särskilda historiska SVG-filerna är public-domain-filer från Wikimedia Commons; respektive ursprung och hash finns i `SourceAssets/flags/sources.json`. Syriens ikon kommer från flag-icons-versionen före flaggbytet 2024. Honduras-ikonen är omfärgad till den äldre blå varianten.

Källor för blockindelningens kärna och särskilda tidsgränser:

- NATO:s medlemsdatum: https://www.nato.int/en/about-us/organization/nato-member-countries
- Warszawapakten: https://history.state.gov/milestones/1953-1960/warsaw-treaty
- ANZUS och den senare suspenderingen av USA:s förpliktelser gentemot Nya Zeeland: https://history.state.gov/milestones/1945-1952/anzus
- Samtida amerikanskt underlag om sovjetiskt/kubanskt militärt samarbete: https://www.cia.gov/readingroom/document/cia-rdp85t00287r000901590001-9
- Landprofilerna i `world.json` innehåller ytterligare källor. Färgvalen är redaktionella, inte en karta direkt utgiven av någon av källorna.

## Verifiering

18 atlas-/grafikdatatester och 23 lokaliseringstester passerar utan Unreal. De kontrollerar bland annat historiska sammanslagningar, representativa punkter i de delade länderna, alliansernas medlemslistor, flaggtäckning, triangelindex, korta triangelkanter, koordinater, översättningar och UFS-staging. Grafikdata har också renderats separat för kontroll av Europa, Amerika och Asien; detta är inte ett speltest.

**Unreal-kompilering, GPU-rendering och paketerad körning har inte kunnat verifieras här.** UE-testet `TMOP.WorldAtlas.LandClipping` kontrollerar klippning och laddning av de nya filerna när det körs i motorn.

Kontrollera i Unreal: rotera över datumlinjen och polerna, zooma till båda ytterlägena, klicka på tätt liggande flaggor, växla språk, slå av/på båda nya lagren, stäng/öppna menyn, samt kör ett paketerat spel. Färger och flaggor ska försvinna på globens baksida. Flaggor ska behålla sina klickytor när de förskjuts. Kartan ska fortsätta fungera när spelet är pausat.
