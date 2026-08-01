L1639 – KUMULATIV UPPDATERING

Tillagt:
- VITTNE_L1639_1 och VITTNE_L1639_2 (anonyma kvinnor).
- GROUP_L1639_TWO_WOMEN.
- VEHICLE_L1639_WOMENS_CAR, Volvo 240-placeholder vid Luntmakargatan_51.
- OBSERVED_L1639_YOUNG_MAN_JUMPS_ROW, arkiverad utan spawn eftersom tiden saknas.
- Tre observationsposter: yngre mannen, osäker Björn Rosengren-identifikation och polisbil vid Tunnelgatan.

Tidslogik:
- Kvinnornas sena utgång och bilstart är relativa till GRAND_1_PROGRAM_END_WITH_CREDITS.
- Polisbilspassagen är rekonstruerad som PALME_SHOT_1 +150 sekunder.
- Alla nya aktiva personrörelser har bTeleportDuringCatchUp=False.

Kontroll i Unreal:
- Kontrollera SEAT_GRAND_1_125 och SEAT_GRAND_1_126.
- Luntmakargatan_51 och TunnelXSvea_NE återanvänds; inga nya anchors krävs.
- Importera INTE över din fungerande manuellt sammanslagna HistoricalVehicles-UAsset blint. Den nya fordonsraden måste läggas in/mergeas kontrollerat så tidigare fordonsdata bevaras.
- RB2520-kopplingen är en rekonstruktion eftersom källan bara säger 'polisbil'.
