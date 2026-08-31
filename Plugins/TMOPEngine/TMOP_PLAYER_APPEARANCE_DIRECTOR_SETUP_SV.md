# TMOP Player Appearance Director

`TMOPPlayerAppearanceDirector` placeras en gång i spelbanan och ger den
spelbara karaktären kläder från samma `DT_TMOP_AppearanceAssets` som NPC:erna.

## Lägg till i leveln

1. Kompilera pluginen och öppna leveln.
2. Sök efter `TMOPPlayerAppearanceDirector` i Place Actors.
3. Dra in en director i leveln.
4. Lämna `Target Character Override` tomt för Player 0.
5. Lämna `Body Mesh Override` tomt om spelarens vanliga `Character Mesh`
   är helkroppen. Om spelaren har flera meshkomponenter väljs den riktiga
   helkroppsmeshen här.
6. Sätt `Appearance Asset Table Override` till `DT_TMOP_AppearanceAssets`,
   eller lämna tomt om `TMOPPersonRegistryDirector` redan konfigurerar tabellen.

## Två sätt att välja kläder

### Egen spelarprofil i directorn

Låt `Use Person Profile Row` vara av. Expandera `Inline Appearance Profile` och
ange samma Catalog Id som används på NPC:er, exempelvis:

- Outerwear: `Police_jacket`
- Headwear: `Police_hat`

### Återanvänd en personrad

Aktivera `Use Person Profile Row` och välj `DT_TMOP_People` samt önskad rad.
Spelaren får då samma appearanceprofil som personen.

## Runtime

Directorn hittar Player Character efter BeginPlay och försöker igen en kort
stund om spelaren spawnas senare. `Refresh Player Appearance` kan anropas från
Blueprint efter respawn, klädbyte eller byte av spelbar Pawn.

Alla modulara delar skapas som Skeletal Mesh Components, använder samma
Skeleton som spelarens Body Mesh och får Body Mesh som Leader Pose. Den
befintliga Anim Blueprinten på kroppen behålls.

Kvinnlig profil använder Quinn och manlig profil Manny när ingen uttrycklig
Body-mesh har valts. MetaHuman-kroppar skrivs inte över.
