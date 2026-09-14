# Systemtest vid Grand – två personer och en testbil

Detta är ett avsiktligt fiktivt diagnostikscenario. Raderna är märkta
`SYSTEM_TEST` och `FictionalGameplay` och får inte användas som historiskt
material.

## Rader

| Typ | ID | Synligt namn |
|---|---|---|
| Förare | `SYSTEM_TEST_DRIVER` | David Testperson |
| Passagerare | `SYSTEM_TEST_PASSENGER` | Eva Testperson |
| Bil | `VEHICLE_SYSTEM_TEST_GRAND` | SYSTEMTEST – turkos Volvo vid Grand |
| Delad dialoghändelse | `SYSTEM_TEST_GRAND_TIMED_DIALOGUE` | Fyra växlande pratbubblor |

## Automatisk sekvens

| Tid | Förväntat resultat |
|---|---|
| 23:00:00 | Den turkosa Volvon spawnar vid `EnterSveavagenN_Car`. |
| 23:00:05 | David och Eva spawnar bredvid bilen utan markgenomslag eller överlapp. |
| 23:00:08 | Båda visar en tydligt märkt automatisk testpratbubbla. |
| 23:00:15 | David sätter sig `FRONT_LEFT`, Eva `FRONT_RIGHT`. |
| 23:00:30 | Bilen startar först när båda är ombord. Den manuella lane-rutten går söderut mot Grand. |
| 23:02:00 | Bilen anländer och stannar vid `GrandOutside_4_Curb`. |
| 23:02:05–15 | Båda kliver ur och går till var sin sida av `GrandOutside_2_Entrance`. |
| 23:02:18 | Ankomstrepliker visas på rätt personer. |
| 23:02:25 | Personerna tittar på varandra. |
| 23:02:30–40 | David spelar stående tal-animation och Eva telefon/radio-animation; båda återgår sedan till vanlig animation. |
| 23:03:00–15 | `[TIDSDIALOG 1/4]` till `4/4` växlar mellan personerna var femte sekund. Lyssnaren tittar på talaren. |
| 23:03:35–40 | Båda joggar tillbaka och sätter sig i samma säten. |
| 23:03:45–04:10 | Bilen gör en långsam, högersvängd `AnchorManeuver` till Grandparkeringen vid Smala gränd. |
| 23:04:20–45 | Bilen provar en vänstersvängd backmanöver tillbaka till Grand. |
| 23:04:45 | Bilen parkeras vid `GrandOutside_4_Curb`; inga fler automatiska fordonskommandon följer. |
| 23:04:50–05:00 | Personerna kliver ur och lämnar plats för spelarens övertagande. |

## Kontroller att göra i Play

### Personer och animation

- Bekräfta att båda spawnar på marken och inte i varandra eller bilen.
- Kontrollera rätt förare/passagerarsäte, instignings- och urstigningsanimation,
  dold kropp i bilen och återställd kropp efter urstigning.
- Kontrollera normal gång, jogging tillbaka till bilen och att fotstegens takt
  följer hastigheten.
- Vid 23:02:30 ska båda unika animationerna börja utan att locomotion startas om
  varje bildruta. Vid 23:02:40 ska `DefaultSlot` sluta och vanlig idle återkomma.
- Sikta på båda personerna och kontrollera namn, target-markering och
  informationsfönster.

### Synlig tidsdialog

- Fyra repliker ska synas i ordningen 1, 2, 3, 4 på växelvis David och Eva.
- Bara aktuell talare ska ha pratbubblan; den andra ska titta på talaren.
- Ingen replik får visas två gånger vid normal gång.
- Pausa mitt i en replik: bubblan och dialogordningen ska återupptas korrekt.
- Sök klockan bakåt före 23:03 och kör igen: hela sekvensen ska kunna spelas om
  efter korrekt loop-/seek-återställning.
- Sök direkt till 23:03:08: systemet ska inte teleportera personerna fel eller
  spela framtida repliker i fel ordning.
- Starta 2–4 lokala spelare. Alla ska se samma världspratbubbla, men en manuell
  E-dialog ska bara öppnas och styras i den spelares viewport som initierade den.

### Manuell E-dialog

- Före skottgränsen: sikta på David respektive Eva och tryck Interact. Texten ska
  börja `[SYSTEMTEST – FÖRE SKOTTET]`.
- Efter skottgränsen: upprepa och kontrollera texten
  `[SYSTEMTEST – EFTER SKOTTET]`.
- Kontrollera dialogkamera, stängning, återställd kamerarotation och att rörelse
  inte ligger kvar efter att dialogen stängts.
- I tvåspelarläge: öppna dialog med spelare 2 medan spelare 1 rör sig. Bara rätt
  spelarvy ska ta emot menyinput; om dialogen pausar spelet ska pausen vara gemensam.

### Bil och rutt

- Bilen får inte starta 23:00:30 om en listad person ännu inte sitter i bilen.
- Kontrollera att `ManualLaneRoute` följer de tre angivna lane-segmenten från
  norra Sveavägen till Grand, följer trafikregler och når stoppet utan teleport.
- Kontrollera att ankomsttiden 23:02:00 inte förväxlas med avgångstiden 23:00:30.
- Första ankarmanövern ska vara en mjuk kurva utan lane-snap. Backmanövern ska
  verkligen köras baklänges utan att bilens rotation vänds 180 grader.
- Kontrollera att stopp/parkering inte placerar bilen under marken, i fasaden
  eller ovanpå en annan bil.
- Efter 23:05: ta över bilen med spelaren och prova gas, broms, back, styrning,
  handbroms, high-speed-läge, kamera, shoulder swap, zoom, exit och återinträde.
- Prova att två spelare försöker ta förarplatsen samtidigt. Bara en får bli
  förare; den andra ska kunna använda ett ledigt passagerarsäte eller nekas rent.
- Spara vid 23:01 under färd och vid 23:05 efter parkering. Ladda båda sparningarna
  och kontrollera biltransform, säten, personer, klocka och kvarvarande tidslinje.

## Import

Extrahera först `DT_TMOP_People.json` ur `DT_TMOP_People.zip`. Importera/reimportera:

1. `DT_TMOP_HistoricalEvents.json`
2. `DT_TMOP_HistoricalVehicles.json`
3. `DT_TMOP_People.json`

Ordningen gör att dialoghändelsen och bilen finns när personkopplingarna granskas.
Starta därefter om PIE så att direktörernas runtime-cache byggs från de nya raderna.

Om en ankarmanöver saknas, kontrollera att följande ankare är laddade och exakt
stavade: `EnterSveavagenN_Car`, `GrandOutside_4_Curb`,
`GrandOutside_2_Entrance` och `EAE46_FORD_SMALA_GRAND_PARKED`.
