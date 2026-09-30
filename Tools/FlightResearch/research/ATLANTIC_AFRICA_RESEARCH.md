# Atlanten och Afrika – omgång 8 / paket v09

Granskat 26 september 2026. **37 nya tidtabellsrader ger 60 nya planerade fysiska flygsträckor**, 28 nya riktade platspar och sju nya flygplatser. Det kumulativa underlaget har 759 rörelser. Det är fortfarande ett urval, inte en fullständig trafikrekonstruktion.

## Underlag och spårbarhet

Primärkälla: [Pan Am, 11 februari–26 april 1986, University of Miami](https://digitalcollections.library.miami.edu/digital/collection/asm0341/id/37370/). Avgångstider, ankomsttider, veckodagar och undantag har lästs visuellt från skanningarna. OCR används endast som sökhjälp. Sidnummer nedan är de **tryckta sidnumren**, inte bildvisarens löpnummer.

| Tryckt sida | Granskade avsnitt | Källbild |
|---|---|---|
| 33 | Dakar | [37266](https://digitalcollections.library.miami.edu/iiif/2/asm0341:37266/full/2400,/0/default.jpg) |
| 56 | Lagos, London | [37289](https://digitalcollections.library.miami.edu/iiif/2/asm0341:37289/full/2400,/0/default.jpg) |
| 57 | London | [37290](https://digitalcollections.library.miami.edu/iiif/2/asm0341:37290/full/2400,/0/default.jpg) |
| 58 | London | [37291](https://digitalcollections.library.miami.edu/iiif/2/asm0341:37291/full/2400,/0/default.jpg) |
| 59 | Los Angeles | [37292](https://digitalcollections.library.miami.edu/iiif/2/asm0341:37292/full/2400,/0/default.jpg) |
| 66 | Monrovia | [37301](https://digitalcollections.library.miami.edu/iiif/2/asm0341:37301/full/2400,/0/default.jpg) |
| 68 | München, Nairobi | [37303](https://digitalcollections.library.miami.edu/iiif/2/asm0341:37303/full/2400,/0/default.jpg) |
| 71 | New York JFK | [37306](https://digitalcollections.library.miami.edu/iiif/2/asm0341:37306/full/2400,/0/default.jpg) |
| 72 | New York JFK | [37307](https://digitalcollections.library.miami.edu/iiif/2/asm0341:37307/full/2200,/0/default.jpg) |

`panam_batch08.tsv` innehåller alla 37 nya rader. Kolumnen `days` anger ISO-veckodagar för lokal avgång, `offset` ankomstens lokala dygnsskifte och `code` den tryckta dagkoden. `add_1mar` kräver datumundantaget 1 mars i katalogen; TSV-filen ensam är därför inte ett komplett driftsschema. Katalogens granskningsintervall 26 februari–2 mars är avsiktligt snävare än tidtabellsutgåvans giltighet.

## PA188: New York till Nairobi via Västafrika

Samtliga fyra sträckor överlappar animationens period. Tiderna är planerade, inte belägg för faktisk avgång.

| Sträcka | Avgång, lokal tid | Ankomst, lokal tid | Avgång–ankomst i UTC |
|---|---|---|---|
| JFK–Dakar Yoff | 27 feb 21.45 EST | 28 feb 09.50 GMT | 28 feb 02.45–09.50 |
| Dakar–Monrovia Roberts | 28 feb 11.00 GMT | 28 feb 12.50 GMT | 28 feb 11.00–12.50 |
| Monrovia–Lagos | 28 feb 14.05 GMT | 28 feb 17.20 WAT | 28 feb 14.05–16.20 |
| Lagos–Nairobi | 28 feb 18.30 WAT | 1 mars 01.30 EAT | 28 feb 17.30–22.30 |

Markuppehållen är 70 minuter i Dakar och 75 respektive 70 minuter i Monrovia och Lagos. Ingen nonstoplinje JFK–Nairobi skapas. Flygnumret kopplar ihop tjänstens delsträckor; registrering och flygplansidentitet är inte fastställda.

## PA189: retur och fönstrets slut

| Sträcka | Avgång–ankomst i UTC | Animation |
|---|---|---|
| Nairobi–Lagos | 1 mars 14.15–19.10 | Ingår |
| Lagos–Monrovia | 1 mars 20.20–22.35 | Ingår; planet är i luften när fönstret slutar |
| Monrovia–Dakar | 1 mars 23.50–2 mars 01.40 | Utanför fönstret |
| Dakar–JFK | 2 mars 02.45–10.55 | Utanför fönstret |

De två sista raderna finns i katalogen med källa men skapar inga rörelser i `flights.json`. Animationen slutar 1 mars 22.21.30 UTC. Lokal lördag är inte i sig tillräckligt för att en flygning ska ingå.

## Övriga tillskott och viktiga undantag

- Heathrow får fler sträckor till Amsterdam, Bryssel, Frankfurt, Hamburg, München, New York, Washington, Miami, Los Angeles, Seattle och San Francisco. München avser gamla **Riem**.
- JFK får ytterligare nattflyg till Frankfurt, Heathrow, München och Zürich. Los Angeles får Frankfurt, Heathrow och JFK; även JFK–Los Angeles ingår.
- **PA121 LHR–LAX**: onsdag/fredag i februari. Från 1 mars tillkommer bland annat lördag, vilket ger en ny avgång i vårt fönster. Både 28 februari och 1 mars finns i runtime-data.
- **PA120 LAX–LHR**: torsdag/söndag i februari. Den nya lördagsavgången 1 mars 18.30 PST motsvarar 2 mars 02.30 UTC och ligger utanför fönstret. Endast avgången 27 februari ingår här.
- **PA125**: fredag går den granskade sträckan London–Seattle; lördag går London–San Francisco nonstop. Seattle–San Francisco har ännu inte importerats. Ingen nonstopresa London–San Francisco skapas för fredagen.
- **PA100 LHR–FRA och PA2 LHR–HAM**: raderna gäller till och med 2 mars. Senare marsändringar används inte.
- **PA55 London–Detroit** börjar först 20 mars och har uteslutits.
- **No Local Traffic** för PA98 LHR–AMS, PA102 LHR–BRU och PA61 MUC–FRA bevaras i anteckningarna. Detta avser restriktion för lokal trafik, inte en inställd fysisk delsträcka.
- Flyg som redan är i luften vid periodens start räknas med. Därför kan en daglig sträcka ge tre rörelser trots ett 48-timmarsfönster.

Genomgående tjänster är grupperade där anslutande delar finns i katalogen: PA1, PA2, PA61, PA66, PA72, PA90, PA100, PA102, PA103, PA107, PA188 och PA189. Fem tidigare rader får uppdaterad gruppering, utan ändrade flygtider. Inga luckor mellan granskade delar fylls med gissade tider.

## Flygplatser och fortsatt kö

Nya flygplatser: AMS, MIA, SEA, SFO, DKR, ROB och LOS. `airport_locations_batch08.json` sparar koordinatkällor och kompletterande myndighets-/flygplatshänvisningar. Markörerna avser flygfältsområden, inte terminaler eller uppställningsplatser från 1986. Dakar är Yoff/GOOY; Monrovia är Roberts/GLRB.

Senegal, Liberia och Nigeria är tillagda i landregistret. Trafiken i denna omgång är Pan Am-trafik; inhemska operatörer i dessa tre länder återstår att inventera. Landtabellens tomma operatörskolumner betyder inte att sådana bolag saknades.

Fortsätt med återstående Pan Am-sidor och delsträckor, exempelvis Seattle–San Francisco och återresor från nygranskade destinationer. SAA:s och El Als avgångssidor från vintern 1985/86, liksom SAS och andra nordiska bolag, står kvar i kön. Nya militära flygningar med exakta tider har inte fastställts i denna omgång.

Alla nya flyg har status `scheduled`. Källorna belägger varken verklig flygbana, last, passagerare eller någon anknytning till mordet. Originalskanningarna återdistribueras inte.
