# TMOP – polis- och ambulansimport

Detta paket bygger vidare på den senaste polisgrundversionen och innehåller
fullständiga importfiler, inte endast de nya raderna.

## Innehåll

- 322 personer.
- 45 historiska fordon.
- 40 grupper.
- 25 befintliga SharedEvents.

## Nya personer

- `PETER_ANDERSSON_AMB` – förare i A951.
- `CHRISTER_ERIKSSON_AMB` – ambulanssjukvårdare i A951.
- `MARIA_DEGERMAN` – förare i 912.
- `KENNETH_LAVRELL` – kommer med 912 och byter till A951.
- `EVA_LANTZ` – åker bak i 912 till mordplatsen och fram på återvägen.

Maria Degerman och Eva Lantz har `CategoryId=POLICE` eftersom de var poliser.
Övrig ambulanspersonal har `CategoryId=AMBULANCE`.

## Fordon

- `AMBULANCE_A951`
- `AMBULANCE_912`

Fordonen har ännu `VehicleClass=None`, `ModelData=None` och
`bSpawnInSimulation=false`. Ange rätt ambulans-Blueprint och modell innan de
aktiveras.

A951 spawnas preliminärt vid `EnterKungsgatanE_Car`. 912 spawnas vid
`EnterTunnelgatanW_Car`. Förarnas `BeginDriving` använder
`AutomaticToAnchor`.

## Befintliga SharedEvents

Inga nya SharedEvents har skapats. Endast dessa fyra befintliga rader har
ändrats:

| Event | Tidsfönster eller beroende |
|---|---|
| `AMBULANCE_951_ARRIVES_CRIME_SCENE` | 23:26:00–23:26:30 |
| `AMBULANCE_912_ARRIVES_CRIME_SCENE` | 23:27:00–23:29:00 |
| `AMBULANCE_951_LEAVES_CRIME_SCENE` | 90 sekunder efter 912:s ankomst; motsvarar 23:28:30–23:30:30 |
| `AMBULANCE_912_LEAVES_CRIME_SCENE` | 30–60 sekunder efter A951:s avfärd; motsvarar 23:29:00–23:31:30 |

Beroendena hindrar att A951 lämnar innan Kenneth Lavrell har kommit med 912
och bytt ambulans.

## Olof och Lisbeth Palme

Deras befintliga tidslinjer är bevarade och har kompletterats:

- `OLOF_PALME` går in i `AMBULANCE_A951` på
  `PATIENT_STRETCHER`, åtta sekunder före A951:s avfärd.
- `LISBET_PALME` går in i `AMBULANCE_A951` på `FRONT_RIGHT`,
  åtta sekunder före avfärden.

Olof behåller `LifeState=Dead`. Lisbeth behåller `LifeState=Alive`.

## Sätes-ID:n som ambulansens Blueprint måste ha

- `FRONT_LEFT`
- `FRONT_RIGHT`
- `PATIENT_STRETCHER`
- `PATIENT_BENCH_1`
- `PATIENT_BENCH_2`

Om ambulans-Blueprinten använder andra socket-/seat-ID:n måste de här värdena
bytas i personernas tidslinjer före aktivering.

## Lars Jeppsson möter 3230-poliserna

Det tidigare platshållareventet `LARS_MEET_POLICE` har delats upp i:

- `LARS_MEETS_POLICE_FIRST_PASS`, cirka 23:25:20–23:25:50. Claes
  Djurfeldt och Jan Hermansson passerar Lars på David Bagares gata och Lars
  blir kvar.
- `LARS_MEETS_POLICE_RETURN_TO_CRIME_SCENE`, cirka
  23:26:00–23:27:00. Jan Hermansson tar med Lars tillbaka ned till
  mordplatsen. Deras tidslinjer når `ANCHOR_TEST_END` 90 sekunder efter
  eventet.

Lars, Claes och Jan använder samma SharedEvents så att mötena förblir
synkroniserade när eventtiderna varierar.

## RB 1170 – Anders Pettersson och Mats Eriksson

Mats Eriksson används tills vidare som teknisk förare (`FRONT_LEFT`) och
Anders Pettersson som passagerare (`FRONT_RIGHT`). Förarvalet är rekonstruerat
och kan bytas senare utan att möteshändelserna påverkas.

Följande SharedEvents har lagts till:

| Event | Tidsfönster |
|---|---|
| `POLICE_RB1170_ARRIVES_SNICKARBACKEN` | 23:27:45–23:28:15 |
| `POLICE_RB1170_MEETS_GEDDA_WIKSTROM_SNICKARBACKEN` | 23:32:30–23:33:30 |
| `POLICE_RB1170_CHECKS_INTOXICATED_MAN_SNICKARBACKEN` | 23:33:00–23:35:00 |

Anders och Mats använder alla tre händelserna. Klas Gedda och Peter Wikström
är kopplade till kollegmötet. Den berusade mannen är ännu inte skapad som en
egen People-rad och kan kopplas till sitt SharedEvent senare.

Mötet med paret vid Oxtorgsgatan/Malmskillnadsgatan utelämnas eftersom platsen
ligger utanför spelplanen. RB 1170, Mats och Anders spawnas i stället klockan
23:26:55 vid `EnterMalmskillnadsgatanS_Car`. Den exakta lane-rutten därifrån
till Snickarbacken kopplas från lane-underlaget.

`GROUP_POLICE_RB1170` innehåller Anders och Mats, med Anders som gruppledare.
Gruppen skapas vid scenariostart men medlemmarna spawnas först tillsammans med
bilen. Mats har tjänstgöringsnummer 97 och tillhör VD 1, C-turen.

## Christer Persson och pikét 1230

Christer Persson är chef för `GROUP_POLICE_PIKET1230`. Gruppen innehåller
Christer Persson, Kent Bäcklund, Ulf Hellman, Lena Löhr och Per Borg.
Hans-Erik Rehnstam tillhörde organisatoriskt gruppen men färdades mordkvällen
i RB 1210 och ingår därför inte som passagerare i 1230.

Pikét 1230 spawnas 23:24:30 vid `EnterTunnelgatanW_Car` och kör på:

`TUNNELGATANE_001_R1` → `TUNNELGATANE_002_R1` →
`TUNNELGATANE_003_R1` → `SVEAVAGENN_004_R2`.

Per Borg används tills vidare som teknisk förare. Christer sitter
`FRONT_RIGHT`. Föraridentiteten kan bytas senare utan att SharedEvents
påverkas.

Christer använder följande SharedEvents:

| Event | Funktion |
|---|---|
| `POLICE_PIKET_1230_ARRIVES_CRIME_SCENE` | 1230 anländer och Christer lämnar bilen |
| `POLICE_1230_FIRST_CORDON_STARTS` | 3–4 minuter efter att A951 lämnar |
| `POLICE_1230_FIRST_CORDON_COMPLETE` | första avspärrningen färdig inom högst fem minuter |
| `POLICE_1230_CORDON_EXTENDED_TO_LUNTMAKARGATAN` | östra gränsen flyttas till Tunnelgatan/Luntmakargatan |

De återstående medlemmarnas individuella tidslinjer är nu ifyllda:

| Person | Individuell uppgift |
|---|---|
| Lena Löhr | frågar efter vittnen, uppmärksammas av Leif Ljungqvist och stoppar Anders Björkman |
| Kent Bäcklund | tar hand om Stefan Glantz efter mun-mot-mun-insatsen |
| Ulf Hellman | ordnar och bevakar avspärrningen |
| Per Borg | ordnar och håller avspärrningen och stannar hela natten |

Nya SharedEvents:

- `POLICE_LENA_LOHR_ALERTED_BY_LEIF_LJUNGQVIST`
- `POLICE_LENA_LOHR_INTERVIEWS_ANDERS_BJORKMAN`
- `POLICE_KENT_BACKLUND_ASSISTS_STEFAN_GLANTZ`

Leif Ljungqvist, Anders Björkman och Stefan Glantz har motsvarande referenser
i sina egna tidslinjer. Kopplingen mellan Bäcklunds uppgift och Stefan Glantz
är markerad som rekonstruerad eftersom det citerade förhöret inte namnger
mannen.
