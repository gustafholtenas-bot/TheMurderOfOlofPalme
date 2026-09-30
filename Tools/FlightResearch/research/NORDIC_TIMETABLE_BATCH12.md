# Nordiska vintertidtabeller – Braathens och SAS/Linjeflyg

Granskat 27 september 2026. Leverans v13 tillför **22 tidtabellslagda flygningar** från Braathens vinterutgåva: 15 fredag 28 februari och sju lördag 1 mars 1986. Det finns nu **946 rörelser** totalt, varav **37 med nordisk start eller destination**. Inget faktiskt genomförande eller ny överflygning har bekräftats i denna omgång.

## Importerade sträckor

| Sträcka | Fredag 28 feb | Lördag 1 mars | Totalt |
|---|---:|---:|---:|
| Røros → Oslo–Fornebu | 1 | 1 | 2 |
| Røros → Trondheim–Værnes | 1 | 1 | 2 |
| Sandefjord–Torp → Stavanger–Sola | 3 | 0 | 3 |
| Stavanger–Sola → Bergen–Flesland | 4 | 2 | 6 |
| Haugesund–Karmøy → Stavanger–Sola | 2 | 0 | 2 |
| Kristiansand–Kjevik → Oslo–Fornebu | 4 | 3 | 7 |
| **Summa** | **15** | **7** | **22** |

Sju flygplatser har lagts till: RRS, TRD, TRF, SVG, BGO, HAU och KRS. Kartpunkterna anger flygfälten, inte belagda gater eller flygplanspositioner. Flygplatsernas befintliga koordinater har hämtats från OurAirports; hänvisningar finns i `airport_locations_batch12.json`.

## Tider i källan

Alla tider nedan är lokal norsk tid, CET (UTC+1) under dessa två dagar. F och L betyder att raden ger en avgång fredag respektive lördag i vårt tidsfönster.

| Flyg | Från → till | Avgång | Ankomst | I tidsfönstret |
|---|---|---|---|---|
| BU135 | RRS → FBU | 11:25 | 12:10 | F |
| BU137 | RRS → FBU | 13:25 | 14:10 | L |
| BU134 | RRS → TRD | 11:55 | 12:20 | L |
| BU136 | RRS → TRD | 12:40 | 13:05 | F |
| BU211 | TRF → SVG | 08:20 | 09:15 | F |
| BU237 | TRF → SVG | 14:30 | 15:25 | F |
| BU231 | TRF → SVG | 20:20 | 21:15 | F |
| BU221 | SVG → BGO | 06:45 | 07:15 | F, L |
| BU223 | SVG → BGO | 07:00 | 07:30 | F |
| BU271 | SVG → BGO | 09:20 | 09:50 | F |
| BU275 | SVG → BGO | 09:20 | 09:50 | L |
| BU233 | SVG → BGO | 11:15 | 11:45 | F |
| BU238 | HAU → SVG | 11:40 | 12:00 | F |
| BU210 | HAU → SVG | 18:25 | 18:45 | F |
| BU070 | KRS → FBU | 07:55 | 08:30 | F, L |
| BU202 | KRS → FBU | 10:20 | 10:55 | F |
| BU204 | KRS → FBU | 11:50 | 12:25 | L |
| BU272 | KRS → FBU | 12:35 | 13:10 | F |
| BU274 | KRS → FBU | 15:05 | 15:40 | L |
| BU052 | KRS → FBU | 16:50 | 17:25 | F |

## Källor och avgränsningar

Primärunderlaget är fotografier av **Braathens SAFE, Time Table, Winter '85/'86, October 27–March 29**, publicerade i [annons 206172029345](https://www.ebay.co.uk/itm/206172029345). Omslaget och tryckta sidor 10, 11 och 20 har granskats visuellt. Bild-URL:er och SHA-256 finns i `batch_12.json`; fotografierna distribueras inte i ZIP-paketet.

- Endast rader som uttryckligen säger **NONSTOP** har importerats. Rader med ett eller flera stopp, samt anslutningsförslag, har inte ritats som direktflyg.
- BU223, BU070 och BU052 har olika klassbeteckningar på skilda veckodagar. Dessa rader har slagits ihop för samma flygnummer och tider, så att de inte räknas flera gånger.
- Dagkoderna läses enligt konventionen 1=måndag till 7=söndag. Själva teckenförklaringssidan finns inte bland de tillgängliga fotografierna. Originalkoderna sparas i `braathens_batch12.tsv`; framtida kompletteringar kan kontrolleras mot dem.
- Det tryckta **Oslo (OSL)** har för dessa norska inrikesrader kopplats till historiska **Fornebu (FBU)**. Gardermoen-markören GEN för Northwest är oförändrad. Flygplatsfördelningen under perioden behandlas i [Stortingets Dokument 18, 2000–2001](https://www.stortinget.no/Global/pdf/Dokumentserien/2000-2001/dok18-200001.pdf), tryckta s. 173–175. Kopplingen av dessa Oslo-rader till Fornebu är vår historiska platsnormalisering.
- BU-koden anger hur flyget marknadsförs i källan. Den belägger inte vilket bolag eller vilken flygplansindivid som faktiskt utförde flygningen.
- Dan-Air-kartan bredvid s. 20 är en annons med ruttlinjer; den ger inga importerbara avgångstider.
- Resterande tabellsidor, returflyg och eventuella ändringsblad behöver fortfarande hittas. Inga returflyg har konstruerats från de enkelriktade raderna.

Braathens källpost `braathens-19851027` har uppgraderats från omslag till `partial_scan`; bolaget har status `partial`. Det är ännu inte färdiginventerat.

## Nytt svenskt underlag som inte kan animeras ännu

**Flygtider Inrikes Malmö, vinter/vårtidtabell med prislista, 7 januari–26 mars 1986**, gemensamt utgiven av Linjeflyg och SAS, finns fotograferad i [annons 198453305668](https://www.ebay.co.uk/itm/198453305668). Omslaget och tryckta sidor **14, 15, 22 och 23** har lästs. Källan finns som `sas-linjeflyg-malmo-19860107` och kopplas till båda bolagen.

Sidorna innehåller resor till/från bland annat Kramfors/Sollefteå, Linköping, Luleå, Örebro, Örnsköldsvik och Östersund. De visar veckodagar, restider, prisklasser och några giltighetsfotnoter, men inte flygnummer eller tillräcklig information om resans fysiska delsträckor. Därför har **inga nya direktflyg, mellanlandningstider eller operatörstilldelningar härletts från dessa sidor**. Röda siffror betyder låg-/minipris, inte inställt eller militärt flyg.

Nästa steg är att hitta resten av Malmöutgåvan och SAS/Linjeflygs systemtabeller med flygnummer och separata tider för varje fysisk sträcka. SAS och Linjeflyg har nu status `scan_found`, fortfarande utan importerade egna avgångar.

## Fortsatt prioritering

1. Komplettera Braathens med resten av vintertabellen, särskilt returflyg och teckenförklaringen.
2. Fortsätt från den nu identifierade SAS/Linjeflyg-utgåvan och sök Arlandas fysiska avgångar/ankomster.
3. Fortsätt med Danmark, Finnair och Icelandair, där inga rörelser ännu importerats.
4. Sök historiska färdplaner och flygledningshandlingar parallellt. Inget nytt faktiskt överflygningsbelägg har hittats här; globens storcirklar är fortfarande schematiska.

`nordic_priority.json` och den genererade `COUNTRY_INDEX.md` är uppdaterade. Sverige har fortsatt 14 berörda rörelser, Norge nu 30, och Danmark/Finland/Island noll. Summera inte landtalen eftersom gränsöverskridande sträckor ingår i båda länderna.

De tidigare 924 rörelserna har jämförts och är oförändrade. Valideringen kontrollerar även 15 nya fredagsavgångar, sju lördagsavgångar, klassradernas sammanslagning, källhänvisningar och UTC-omräkning. Unreal-kompilering och spelkörning har inte kunnat utföras i denna miljö.
