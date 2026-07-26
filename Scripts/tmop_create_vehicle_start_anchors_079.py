"""Create/update TMOP vehicle start anchors in the loaded Unreal map.

Run from Unreal Editor Python console:
exec(open(r"C:\\Users\\User\\Documents\\Unreal Projects\\TheMurderOfOlofPalme\\Scripts\\tmop_create_vehicle_start_anchors_076.py", encoding="utf-8").read())

Save the level after checking the generated anchors.
"""

import unreal

# The generated anchor block uses JSON boolean spelling. Keep these aliases so
# the embedded data remains valid Python when the script is executed directly.
false = False
true = True

ANCHORS = [
    {
        "vehicle_id": "VEHICLE_BMW_LJUS_METALLIC_KAN_HA_VARIT_DEN_HAR_STORA_BMW_N_3_2_L",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_BMW_LJUS_METALLIC_KAN_HA_VARIT_DEN_HAR_STORA_BMW_N_3_2_L_DisplayName\", \" BMW ljus Metallic. kan ha varit den här stora BMW:n 3,2 L\")",
        "anchor_id": "VEHICLE_BMW_LJUS_METALLIC_KAN_HA_VARIT_DEN_HAR_STORA_BMW_N_3_2_L_START",
        "location_cm": {
            "x": 35856.552,
            "y": -16935.812,
            "z": -327.533
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": -9.987,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "X_SNICKARBACKENE_R1_TO_BIRGERJARLSGATAN_006_R2_LEFT",
        "nearest_lane_distance_cm": 98.9,
        "lane_surface_z_cm": -387.533,
        "source_z_cm": -383.161,
        "z_correction_cm": 55.628,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_ANDERS_DELBOMS_TAXI_BIL_JARFALLA_TAXI_MITSUBISHI_GALANT",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_ANDERS_DELBOMS_TAXI_BIL_JARFALLA_TAXI_MITSUBISHI_GALANT_DisplayName\", \"Anders Delboms taxi bil Järfälla taxi Mitsubishi Galant\")",
        "anchor_id": "VEHICLE_ANDERS_DELBOMS_TAXI_BIL_JARFALLA_TAXI_MITSUBISHI_GALANT_START",
        "location_cm": {
            "x": 1940.779,
            "y": 953.438,
            "z": 99.711
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": -115.781,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "SVEAVAGENN_003_R1",
        "nearest_lane_distance_cm": 381.1,
        "lane_surface_z_cm": 39.711,
        "source_z_cm": 60.573,
        "z_correction_cm": 39.138,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_ANNETTE_KOHUTS_BIL",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_ANNETTE_KOHUTS_BIL_DisplayName\", \"Annette Kohuts bil\")",
        "anchor_id": "VEHICLE_ANNETTE_KOHUTS_BIL_START",
        "location_cm": {
            "x": 9706.169,
            "y": 2161.275,
            "z": 905.29
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": -25.149,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "APELBERGSGATANE_003_R1",
        "nearest_lane_distance_cm": 25.3,
        "lane_surface_z_cm": 845.29,
        "source_z_cm": 872.256,
        "z_correction_cm": 33.034,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_ANONYMTVITTNE_EAC13558_BIL_P_BOT",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_ANONYMTVITTNE_EAC13558_BIL_P_BOT_DisplayName\", \"Anonymtvittne EAC13558 Bil P bot\")",
        "anchor_id": "VEHICLE_ANONYMTVITTNE_EAC13558_BIL_P_BOT_START",
        "location_cm": {
            "x": 39540.814,
            "y": -13504.462,
            "z": -379.927
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": 46.996,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "X_BIRGERJARLSGATAS_004_R1_TO_BIRGERJARLSGATAS_005_R1_STRAIGHT",
        "nearest_lane_distance_cm": 705.0,
        "lane_surface_z_cm": -439.927,
        "source_z_cm": -128.142,
        "z_correction_cm": -251.785,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_ANONYMTVITTNE_EAE119_00_BIL",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_ANONYMTVITTNE_EAE119_00_BIL_DisplayName\", \"Anonymtvittne EAE119-00 Bil \")",
        "anchor_id": "VEHICLE_ANONYMTVITTNE_EAE119_00_BIL_START",
        "location_cm": {
            "x": 16055.161,
            "y": -34860.095,
            "z": 1799.758
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": -118.38,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "JOHANNESGATANN_R1",
        "nearest_lane_distance_cm": 187.8,
        "lane_surface_z_cm": 1739.758,
        "source_z_cm": 1918.167,
        "z_correction_cm": -118.409,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_ANONYMTVITTNE_L1100_BIL",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_ANONYMTVITTNE_L1100_BIL_DisplayName\", \"Anonymtvittne L1100 bil\")",
        "anchor_id": "VEHICLE_ANONYMTVITTNE_L1100_BIL_START",
        "location_cm": {
            "x": -26711.145,
            "y": -42294.47,
            "z": 137.548
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": 65.042,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "SALTMASTAREGATANS_001_R1",
        "nearest_lane_distance_cm": 1555.5,
        "lane_surface_z_cm": 77.548,
        "source_z_cm": 258.389,
        "z_correction_cm": -120.841,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": true
    },
    {
        "vehicle_id": "VEHICLE_ANONYMTVITTNE_L837S_BIL",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_ANONYMTVITTNE_L837S_BIL_DisplayName\", \"Anonymtvittne L837s bil\")",
        "anchor_id": "VEHICLE_ANONYMTVITTNE_L837S_BIL_START",
        "location_cm": {
            "x": -27555.328,
            "y": -27080.615,
            "z": 758.546
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": 64.915,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "TEGNERGATANE_001_R1",
        "nearest_lane_distance_cm": 483.7,
        "lane_surface_z_cm": 698.546,
        "source_z_cm": 206.303,
        "z_correction_cm": 552.243,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_BENGT_PALME_MERCEDES_GUL",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_BENGT_PALME_MERCEDES_GUL_DisplayName\", \"Bengt Palme Mercedes gul\")",
        "anchor_id": "VEHICLE_BENGT_PALME_MERCEDES_GUL_START",
        "location_cm": {
            "x": 1766.442,
            "y": 139.899,
            "z": 70.226
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": -115.568,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "SVEAVAGENN_003_R1",
        "nearest_lane_distance_cm": 576.5,
        "lane_surface_z_cm": 10.226,
        "source_z_cm": -48.117,
        "z_correction_cm": 118.343,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_BIRGIT_DAHLSTROMS_BIL",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_BIRGIT_DAHLSTROMS_BIL_DisplayName\", \"Birgit Dahlströms bil\")",
        "anchor_id": "VEHICLE_BIRGIT_DAHLSTROMS_BIL_START",
        "location_cm": {
            "x": 47077.881,
            "y": -2603.399,
            "z": -532.64
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": 108.969,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "X_BIRGERJARLSGATAS_006_R1_TO_NORRLANDSGATANS_001_R1_RIGHT",
        "nearest_lane_distance_cm": 77.9,
        "lane_surface_z_cm": -592.64,
        "source_z_cm": 1110.839,
        "z_correction_cm": -1643.479,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_BJORN_ROSENGRENS_BIL",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_BJORN_ROSENGRENS_BIL_DisplayName\", \"Björn Rosengrens bil\")",
        "anchor_id": "VEHICLE_BJORN_ROSENGRENS_BIL_START",
        "location_cm": {
            "x": 9865.746,
            "y": 19846.799,
            "z": 393.125
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": 64.095,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "SVEAVAGENS_007_R1",
        "nearest_lane_distance_cm": 1323.1,
        "lane_surface_z_cm": 333.125,
        "source_z_cm": -41.124,
        "z_correction_cm": 434.249,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": true
    },
    {
        "vehicle_id": "VEHICLE_DANICA_NAJIC_BIL_INBROTT_MERCEDES_VINROD_KANSKE",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_DANICA_NAJIC_BIL_INBROTT_MERCEDES_VINROD_KANSKE_DisplayName\", \"Danica Najic bil inbrott mercedes vinröd kanske\")",
        "anchor_id": "VEHICLE_DANICA_NAJIC_BIL_INBROTT_MERCEDES_VINROD_KANSKE_START",
        "location_cm": {
            "x": -9275.523,
            "y": -10513.241,
            "z": 70.048
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": -26.579,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "ADOLFFREDRIKSKYRKOGE_003_R1",
        "nearest_lane_distance_cm": 441.5,
        "lane_surface_z_cm": 10.048,
        "source_z_cm": 4.558,
        "z_correction_cm": 65.49,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_ENWALL_BILD",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_ENWALL_BILD_DisplayName\", \"Enwall bild\")",
        "anchor_id": "VEHICLE_ENWALL_BILD_START",
        "location_cm": {
            "x": 9643.776,
            "y": 19293.036,
            "z": 393.125
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": 64.095,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "SVEAVAGENS_007_R1",
        "nearest_lane_distance_cm": 739.5,
        "lane_surface_z_cm": 333.125,
        "source_z_cm": 218.632,
        "z_correction_cm": 174.493,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_FINARE_PARETS_BIL",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_FINARE_PARETS_BIL_DisplayName\", \"finare parets bil\")",
        "anchor_id": "VEHICLE_FINARE_PARETS_BIL_START",
        "location_cm": {
            "x": -19756.728,
            "y": -44441.455,
            "z": -123.661
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": -115.691,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "SVEAVAGENN_007_R2",
        "nearest_lane_distance_cm": 579.7,
        "lane_surface_z_cm": -183.661,
        "source_z_cm": -196.898,
        "z_correction_cm": 73.237,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_HANS_HONGELINS_TAXI_BIL_VOLVO_740_MELLANBLA",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_HANS_HONGELINS_TAXI_BIL_VOLVO_740_MELLANBLA_DisplayName\", \"Hans Hongelins  taxi bil Volvo 740, mellanblå\")",
        "anchor_id": "VEHICLE_HANS_HONGELINS_TAXI_BIL_VOLVO_740_MELLANBLA_START",
        "location_cm": {
            "x": 58711.707,
            "y": 8164.446,
            "z": -585.232
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": -120.383,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "BIRGERJARLSGATAN_001_R1",
        "nearest_lane_distance_cm": 90.9,
        "lane_surface_z_cm": -645.232,
        "source_z_cm": -259.092,
        "z_correction_cm": -326.14,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_HANS_JOHANSSON_TAXI",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_HANS_JOHANSSON_TAXI_DisplayName\", \"Hans Johansson taxi\")",
        "anchor_id": "VEHICLE_HANS_JOHANSSON_TAXI_START",
        "location_cm": {
            "x": 1447.688,
            "y": -724.107,
            "z": 42.746
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": -118.294,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "X_SVEAVAGENN_003_R1_TO_SVEAVAGENN_004_R1_STRAIGHT",
        "nearest_lane_distance_cm": 682.2,
        "lane_surface_z_cm": -17.254,
        "source_z_cm": -10.069,
        "z_correction_cm": 52.815,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_INGE_MORELIUS_MORK_GUL_OPEL_REKORD_73_A",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_INGE_MORELIUS_MORK_GUL_OPEL_REKORD_73_A_DisplayName\", \"Inge Morelius mörk gul Opel Rekord -73:a.\")",
        "anchor_id": "EnterKungsgatanE_Car",
        "location_cm": {
            "x": 57722.121,
            "y": 2813.398,
            "z": -630.216
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": 0.024,
            "roll": 0.0
        },
        "already_exists": true,
        "nearest_lane_id": "KUNGSGATANW_001_R1",
        "nearest_lane_distance_cm": 91.4,
        "lane_surface_z_cm": -690.216,
        "source_z_cm": -478.0,
        "z_correction_cm": -152.216,
        "anchor_local_z_offset_cm": -152.216,
        "placement_type": "IncomingEnter",
        "manual_rotation_review": false,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_JAN_AKE_SVENSSON_S_BIL_BMW_GRA",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_JAN_AKE_SVENSSON_S_BIL_BMW_GRA_DisplayName\", \"Jan-Åke Svensson s bil BMW GRÅ\")",
        "anchor_id": "VEHICLE_JAN_AKE_SVENSSON_S_BIL_BMW_GRA_START",
        "location_cm": {
            "x": 1584.752,
            "y": -249.438,
            "z": 59.304
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": -116.661,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "X_SVEAVAGENN_003_R1_TO_SVEAVAGENN_004_R1_STRAIGHT",
        "nearest_lane_distance_cm": 583.2,
        "lane_surface_z_cm": -0.696,
        "source_z_cm": -48.117,
        "z_correction_cm": 107.421,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_JAN_AKE_SVENSSONS_BIL",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_JAN_AKE_SVENSSONS_BIL_DisplayName\", \"Jan-Åke Svenssons bil \")",
        "anchor_id": "VEHICLE_JAN_AKE_SVENSSONS_BIL_START",
        "location_cm": {
            "x": 1447.688,
            "y": -724.107,
            "z": 42.746
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": -118.294,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "X_SVEAVAGENN_003_R1_TO_SVEAVAGENN_004_R1_STRAIGHT",
        "nearest_lane_distance_cm": 682.2,
        "lane_surface_z_cm": -17.254,
        "source_z_cm": -10.069,
        "z_correction_cm": 52.815,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_KJELL_AKE_JANSSONS_BIL",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_KJELL_AKE_JANSSONS_BIL_DisplayName\", \"Kjell åke Janssons Bil\")",
        "anchor_id": "VEHICLE_KJELL_AKE_JANSSONS_BIL_START",
        "location_cm": {
            "x": 10414.306,
            "y": 20146.841,
            "z": 393.125
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": 64.095,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "SVEAVAGENS_007_R1",
        "nearest_lane_distance_cm": 1800.1,
        "lane_surface_z_cm": 333.125,
        "source_z_cm": -484.564,
        "z_correction_cm": 877.689,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": true
    },
    {
        "vehicle_id": "VEHICLE_LARS_ERIC_ERIKSSON_PAPPAS_BIL",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_LARS_ERIC_ERIKSSON_PAPPAS_BIL_DisplayName\", \"Lars Eric Eriksson  pappas bil\")",
        "anchor_id": "VEHICLE_LARS_ERIC_ERIKSSON_PAPPAS_BIL_START",
        "location_cm": {
            "x": 0.0,
            "y": 0.0,
            "z": 11.577
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": -54.314,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "X_TUNNELGATANE_003_R1_TO_SVEAVAGENN_004_R2_LEFT",
        "nearest_lane_distance_cm": 1.5,
        "lane_surface_z_cm": -48.423,
        "source_z_cm": 0.0,
        "z_correction_cm": 11.577,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_LARS_ERIC_ERIKSSON_SILVERGRA_VOLVO_GLT",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_LARS_ERIC_ERIKSSON_SILVERGRA_VOLVO_GLT_DisplayName\", \"Lars Eric Eriksson silvergrå Volvo GLT\")",
        "anchor_id": "VEHICLE_LARS_ERIC_ERIKSSON_SILVERGRA_VOLVO_GLT_START",
        "location_cm": {
            "x": -30037.064,
            "y": -38380.072,
            "z": -123.596
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": 64.962,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "HOLLANDAREGATANS_001_R1",
        "nearest_lane_distance_cm": 548.8,
        "lane_surface_z_cm": -183.596,
        "source_z_cm": 466.207,
        "z_correction_cm": -589.803,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_LARS_KNUBBS_BIL",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_LARS_KNUBBS_BIL_DisplayName\", \"Lars Knubbs Bil\")",
        "anchor_id": "LarsKnubbBil",
        "location_cm": {
            "x": -19395.278,
            "y": -31162.887,
            "z": 66.112
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": -53.675,
            "roll": 0.0
        },
        "already_exists": true,
        "nearest_lane_id": "TEGNERGATANE_003_R1",
        "nearest_lane_distance_cm": 273.7,
        "lane_surface_z_cm": 6.112,
        "source_z_cm": 79.472,
        "z_correction_cm": -13.36,
        "anchor_local_z_offset_cm": -13.36,
        "placement_type": "Parked",
        "manual_rotation_review": false,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_LEIF_LJUNGQVIST_CHEVROLET_SUBURBAN",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_LEIF_LJUNGQVIST_CHEVROLET_SUBURBAN_DisplayName\", \"Leif Ljungqvist Chevrolet Suburban\")",
        "anchor_id": "VEHICLE_LEIF_LJUNGQVIST_CHEVROLET_SUBURBAN_START",
        "location_cm": {
            "x": 1883.243,
            "y": 438.391,
            "z": 82.849
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": -115.568,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "SVEAVAGENN_003_R1",
        "nearest_lane_distance_cm": 553.0,
        "lane_surface_z_cm": 22.849,
        "source_z_cm": -48.117,
        "z_correction_cm": 130.966,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_MARCO_NEESERS_KOMPIS_BIL_MORKROD",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_MARCO_NEESERS_KOMPIS_BIL_MORKROD_DisplayName\", \"Marco Neesers Kompis bil mörkröd\")",
        "anchor_id": "EnterSveavagenN_Car",
        "location_cm": {
            "x": -20210.166,
            "y": -42634.14,
            "z": -156.147
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": 0.0,
            "roll": 0.0
        },
        "already_exists": true,
        "nearest_lane_id": "SVEAVAGENS_001_R1",
        "nearest_lane_distance_cm": 266.6,
        "lane_surface_z_cm": -216.147,
        "source_z_cm": -96.915,
        "z_correction_cm": -59.232,
        "anchor_local_z_offset_cm": -59.232,
        "placement_type": "IncomingEnter",
        "manual_rotation_review": false,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_MERCEDES_LIMOSINE_JAN_NILSSON",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_MERCEDES_LIMOSINE_JAN_NILSSON_DisplayName\", \"Mercedes Limosine Jan Nilsson\")",
        "anchor_id": "VEHICLE_MERCEDES_LIMOSINE_JAN_NILSSON_START",
        "location_cm": {
            "x": -848.923,
            "y": 530.036,
            "z": 22.5
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": -29.295,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "TUNNELGATANE_003_R1",
        "nearest_lane_distance_cm": 116.7,
        "lane_surface_z_cm": -37.5,
        "source_z_cm": -48.117,
        "z_correction_cm": 70.617,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_MERCEDES_TAXI_BEIGE",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_MERCEDES_TAXI_BEIGE_DisplayName\", \"Mercedes-taxi  beige\")",
        "anchor_id": "VEHICLE_MERCEDES_TAXI_BEIGE_START",
        "location_cm": {
            "x": 35053.876,
            "y": -17277.753,
            "z": -308.574
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": 160.836,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "X_BIRGERJARLSGATAS_003_R2_TO_SNICKARBACKENE_R1_LEFT",
        "nearest_lane_distance_cm": 7.2,
        "lane_surface_z_cm": -368.574,
        "source_z_cm": -318.764,
        "z_correction_cm": 10.19,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_MIKAEL_LINDBLAD_TAXI_BIL",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_MIKAEL_LINDBLAD_TAXI_BIL_DisplayName\", \"Mikael Lindblad taxi bil\")",
        "anchor_id": "EnterKammakargatanW_Car",
        "location_cm": {
            "x": -22978.876,
            "y": -18748.229,
            "z": 378.283
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": 0.026,
            "roll": 0.0
        },
        "already_exists": true,
        "nearest_lane_id": "KAMMAKAREGATANW_005_R1",
        "nearest_lane_distance_cm": 150.5,
        "lane_surface_z_cm": 318.283,
        "source_z_cm": 455.0,
        "z_correction_cm": -76.717,
        "anchor_local_z_offset_cm": -76.717,
        "placement_type": "IncomingEnter",
        "manual_rotation_review": false,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_MIKEAL_ASTROMS_BIL_ORANGE_FORD_TANUS",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_MIKEAL_ASTROMS_BIL_ORANGE_FORD_TANUS_DisplayName\", \"Mikeal Åströms bil orange ford Tanus\")",
        "anchor_id": "VEHICLE_MIKEAL_ASTROMS_BIL_ORANGE_FORD_TANUS_START",
        "location_cm": {
            "x": -15274.431,
            "y": 7643.04,
            "z": -123.595
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": 153.213,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "TUNNELGATANW_003_R1",
        "nearest_lane_distance_cm": 402.3,
        "lane_surface_z_cm": -183.595,
        "source_z_cm": 1475.029,
        "z_correction_cm": -1598.624,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_PB_SAAB_900I_Y63AM35_1983_BLA",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_PB_SAAB_900I_Y63AM35_1983_BLA_DisplayName\", \"PB SAAB 900I Y63AM35 1983 BLÅ\")",
        "anchor_id": "VEHICLE_PB_SAAB_900I_Y63AM35_1983_BLA_START",
        "location_cm": {
            "x": 37651.233,
            "y": -17775.62,
            "z": -405.458
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": -121.695,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "X_BIRGERJARLSGATAN_005_R1_TO_BIRGERJARLSGATAN_006_R1_STRAIGHT",
        "nearest_lane_distance_cm": 97.3,
        "lane_surface_z_cm": -465.458,
        "source_z_cm": -486.161,
        "z_correction_cm": 80.702,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_SIGGES_BIL_SAAB_900_BRUN",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_SIGGES_BIL_SAAB_900_BRUN_DisplayName\", \"Sigges bil Saab 900 brun\")",
        "anchor_id": "VEHICLE_SIGGES_BIL_SAAB_900_BRUN_START",
        "location_cm": {
            "x": -9084.106,
            "y": -39355.557,
            "z": -176.115
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": -115.031,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "LUNTMAKARGATANN_005_R1",
        "nearest_lane_distance_cm": 43.6,
        "lane_surface_z_cm": -236.115,
        "source_z_cm": 872.256,
        "z_correction_cm": -1048.37,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_THEODOROS_ATHANASIOU_BIL",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_THEODOROS_ATHANASIOU_BIL_DisplayName\", \"Theodoros Athanasiou Bil\")",
        "anchor_id": "VEHICLE_THEODOROS_ATHANASIOU_BIL_START",
        "location_cm": {
            "x": -1637.496,
            "y": 237.916,
            "z": 8.177
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": 151.121,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "TUNNELGATANW_001_R1",
        "nearest_lane_distance_cm": 170.7,
        "lane_surface_z_cm": -51.823,
        "source_z_cm": -47.735,
        "z_correction_cm": 55.912,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_VIT_VOLVO",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_VIT_VOLVO_DisplayName\", \"Vit Volvo\")",
        "anchor_id": "VEHICLE_VIT_VOLVO_START",
        "location_cm": {
            "x": 8050.735,
            "y": -2963.367,
            "z": 200.591
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": -115.417,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "LUNTMAKARGATANN_001_R1",
        "nearest_lane_distance_cm": 128.4,
        "lane_surface_z_cm": 140.591,
        "source_z_cm": 872.256,
        "z_correction_cm": -671.665,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_VAKTARE_BIL",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_VAKTARE_BIL_DisplayName\", \"Väktare bil\")",
        "anchor_id": "VEHICLE_VAKTARE_BIL_START",
        "location_cm": {
            "x": -5496.945,
            "y": -27982.907,
            "z": -153.596
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": 153.231,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "KAMMAKAREGATANW_002_R1",
        "nearest_lane_distance_cm": 292.9,
        "lane_surface_z_cm": -213.596,
        "source_z_cm": -219.12,
        "z_correction_cm": 65.524,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": false
    },
    {
        "vehicle_id": "VEHICLE_AKE_LARSSONS_BIL_FORD_ESCORT_GRAMETALLIC",
        "display_name": "NSLOCTEXT(\"DT_TMOP_HistoricalVehicles [F26B2D7BC347BDFA5B39EE2098F3C814]\", \"VEHICLE_AKE_LARSSONS_BIL_FORD_ESCORT_GRAMETALLIC_DisplayName\", \"Åke Larssons bil  Ford Escort, gråmetallic\")",
        "anchor_id": "VEHICLE_AKE_LARSSONS_BIL_FORD_ESCORT_GRAMETALLIC_START",
        "location_cm": {
            "x": -19954.533,
            "y": 10292.1,
            "z": -123.595
        },
        "rotation_deg": {
            "pitch": 0.0,
            "yaw": 153.213,
            "roll": 0.0
        },
        "already_exists": false,
        "nearest_lane_id": "TUNNELGATANW_003_R1",
        "nearest_lane_distance_cm": 5764.4,
        "lane_surface_z_cm": -183.595,
        "source_z_cm": -10.069,
        "z_correction_cm": -113.526,
        "anchor_local_z_offset_cm": 0.0,
        "placement_type": "Parked",
        "manual_rotation_review": true,
        "manual_location_review": true
    }
]


def prop(obj, name, value):
    obj.set_editor_property(name, value)


anchor_class = getattr(unreal, "TMOPHistoricalAnchor", None)
if anchor_class is None:
    raise RuntimeError("TMOPHistoricalAnchor is unavailable. Build TMOPEngine and restart Unreal.")

subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = subsystem.get_all_level_actors()
existing = {}
for actor in actors:
    if not isinstance(actor, anchor_class):
        continue
    try:
        anchor_id = str(actor.get_anchor_id())
    except Exception:
        anchor_id = ""
    # A previous interrupted run may have assigned the actor label before the
    # identity component. Reuse that actor instead of spawning a duplicate.
    if anchor_id in ("", "None"):
        anchor_id = actor.get_actor_label()
    if anchor_id not in ("", "None"):
        existing[anchor_id] = actor

created = 0
updated = 0
skipped = 0
for item in ANCHORS:
    anchor_id = item["anchor_id"]
    if item["already_exists"] and anchor_id in existing:
        unreal.log("TMOP vehicle anchor kept: " + anchor_id)
        skipped += 1
        continue

    location_data = item["location_cm"]
    rotation_data = item["rotation_deg"]
    location = unreal.Vector(
        location_data["x"], location_data["y"], location_data["z"]
    )
    # Unreal Python's positional Rotator order is Roll, Pitch, Yaw. Use named
    # arguments so the street heading cannot accidentally become vehicle pitch.
    rotation = unreal.Rotator(
        roll=rotation_data["roll"],
        pitch=rotation_data["pitch"],
        yaw=rotation_data["yaw"],
    )

    actor = existing.get(anchor_id)
    if actor is None:
        actor = subsystem.spawn_actor_from_class(anchor_class, location, rotation)
        created += 1
    else:
        actor.set_actor_location_and_rotation(
            location, rotation, False, False
        )
        updated += 1

    actor.set_actor_label(anchor_id)
    prop(actor, "display_name", unreal.Text(anchor_id))
    # Unreal Python drops the C++ boolean b-prefix from reflected properties.
    prop(actor, "can_be_used_for_routing", True)
    prop(actor, "hard_historical_anchor", True)
    identity = actor.get_editor_property("entity_identity")
    identity.set_entity_identity(anchor_id, "Anchor")
    existing[anchor_id] = actor

unreal.log(
    "TMOP vehicle anchors complete: {} created, {} updated, {} kept. "
    "Review rotations, then Save All.".format(created, updated, skipped)
)
