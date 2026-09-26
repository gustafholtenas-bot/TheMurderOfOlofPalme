# Befattningsträd och konfliktflaggor

Världsmenyn har en övre, utfällbar befattningspanel och en separat undre panel
för händelser, reaktioner, historiskt sammanhang, relaterade poster och källor.
Dra avdelaren för att ändra höjderna. Båda panelerna kan scrollas oberoende;
breda/djupa träd kan även scrollas i sidled. Svenska och engelska stöds.

## Redigera `world.json`

Alla 33 befintliga landsprofiler har ett träd. Varje nod har ett stabilt lokalt
`id`, ett `parent` (tom sträng för en rot), ett `label` som pekar på ett fält i
postens `text`, och en `relation`. Flera rötter är tillåtna, så en oberoende
befattning inte behöver placeras under en påhittad chef. Ordningen i `nodes`
bestämmer syskonens ordning. Nya nivåer kräver ingen C++-ändring.

Exempel på en ny underordnad befattning (schematiskt, inte historiskt innehåll):

```json
{
  "hierarchy": {
    "as_of": "1986-02-28",
    "nodes": [
      {"id": "agency", "parent": "", "label": "office-agency", "relation": "group"},
      {"id": "department", "parent": "agency", "label": "office-department", "relation": "reports_to", "sources": ["https://example.org/replace-with-actual-source"]}
    ]
  },
  "text": {
    "office-agency": {"sv": "Myndighetschef\nNamn", "en": "Agency director\nName", "en_source": "Myndighetschef\nNamn"},
    "office-department": {"sv": "Avdelningschef\nNamn", "en": "Department head\nName", "en_source": "Avdelningschef\nNamn"}
  }
}
```

- `reports_to`: belagd rapporteringsväg, hel blå linje. Kräver förälder och
  minst en HTTPS-källa. `sources` styr nodens källhänvisning i hovringstexten.
- `group`: streckad grå linje. Organisatorisk gruppering eller ännu inte
  kartlagd rapporteringsväg; anger **inte** att personen ovanför var chef.
- Valfritt `note`: ytterligare fältnamn under `text`, visas på kortet.
- `label`/`note` använder samma översättningsmekanism som övriga atlasfält:
  `TMOP_WorldAtlas`, post-ID, fältnamn. Inbyggd engelska används endast när
  `en_source` matchar aktuell svenska; annars används svenska.
- Uppdatera kortens `office-*`-fält när namn eller befattningar ändras. De äldre
  profiltexterna (`leaders`, `ministers`, `personnel` m.fl.) är separat
  bakgrundsinformation under **Regering, tjänster och källanmärkningar**.
  De parsas inte automatiskt till rapporteringsvägar. Håll båda aktuella.
- Max 512 noder och 32 nivåer per post. Dubbletter, okända föräldrar,
  cirkelreferenser, saknade texter och osourcade chefslinjer avvisas.

## Källstatus

Referensdatumet är **28 februari 1986**. USA-exemplet i den bifogade bilden är
daterat 2003 och används enbart som layoutreferens. USA har grenar för
regeringsmedlemmar, CIA, NSA under försvarsministern och FBI under
justitieministern. Vicepresidenten har en separat befattning; Gates är DDI,
McMahon är DDCI. CIA:s Latinamerikaavdelning har en egen mellanliggande nod
före Fiers. Dess chefsnamn lämnas öppet här. Trädet förenklar organisationen;
DCI:s ansvar inför både presidenten och NSC anges på kortet.

Övriga länders kända befattningshavare visas också som noder. Där källorna
inte fastställer rapporteringsvägen visas gruppering. Detta är ännu inte en
fullständig kartläggning av alla länders kommandokedjor. Schweiz visar ett
kollektivt förbundsråd; presidenten ges inte chefsansvar över övriga ledamöter.
Osäkra chefsnamn och Egyptens ministerskifte på referensdagen behåller sina
källanmärkningar.

Nya organisationskällor och befintligt underlag:

- [EO 12333, ursprunglig version från 1981](https://www.reaganlibrary.gov/archives/speech/executive-order-12333-united-states-intelligence-activities), §§1.5, 1.11, 1.14.
- [Reagans regeringsmedlemmar](https://www.reaganlibrary.gov/reagans/reagan-administration/cabinet-members-during-reagan-administration).
- [Gates nominering den 4 mars 1986](https://www.reaganlibrary.gov/archives/speech/nomination-robert-m-gates-be-deputy-director-central-intelligence).
- [Walsh, kapitel 17](https://irp.fas.org/offdocs/walsh/chap_17.htm) och [19](https://irp.fas.org/offdocs/walsh/chap_19.htm).
- Nicaragua och Norge: befintliga källor i respektive lands `sources` och på noderna.

## Flaggor vid konfliktpunkter

En deltagare får exempelvis `"flag": "cn"` eller `"flag": "vn"`. Referensen
pekar på `flags_1986.json`, **inte** på en landsprofil som måste existera.
Samma flagga syns vid konfliktpunkten och deltagarnamnet i detaljpanelen.
Hovra för namnet; klick behåller konfliktens sammanhang och pilar.
Flaggornas träffytor följer deras synliga rektanglar. Vald konflikts punkter
ritas och träfftestas över andra markörer. Flagglagret kan stängas av.

54 statliga deltagarpunkter använder flaggor, med 54 flaggbilder totalt
(33 tidigare + 21 nya). Icke-statliga aktörer, civila och blandade
koalitionspunkter behåller numrerade symboler. En ny nationalflagga behöver
en SVG och käll-/licenspost under `Tools/WorldAtlas/SourceAssets/flags/`,
varefter `build_appearance.flags()` bygger om den paketerade PNG-atlasen.
Libyen, Burma och Etiopien använder historiska flaggvarianter. Se
`SourceAssets/flags/sources.json` för ursprung och SHA-256.

## Kontroll

```sh
python Tools/WorldAtlas/validate_hierarchy.py
python -m unittest discover -s Tools/WorldAtlas
python -m unittest discover -s Tools/Localization
```

Bygg sedan **Development Editor** med Unreal stängd. Öppna världsmenyn,
kontrollera Kina/Vietnam-konfliktens flaggor och pilar, USA:s träd, öppna/stäng
grenar, dra avdelaren, scrolla båda panelerna och byt svenska/engelska.
Kontrollera även filtrering och landsbyte samt ett paketerat spel.
Unreal finns inte i leveransmiljön; kompilering och interaktion måste därför
verifieras i projektets editor.
