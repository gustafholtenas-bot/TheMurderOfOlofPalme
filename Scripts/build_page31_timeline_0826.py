#!/usr/bin/env python3
"""Build the Page 31 witness-anchor/timeline package from the 08_25 tables.

The web animation uses 23:21:22 for the first shot.  The game uses the shared
event Palme_shot_1, normally 23:21:30.  Every new person entry is therefore
relative to Palme_shot_1; the clock fields are only editor-friendly previews.
"""

from __future__ import annotations

import copy
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
PEOPLE_IN = ROOT / "DataTables/08_25/DT_TMOP_People.json"
VEHICLES_IN = ROOT / "DataTables/08_25/DT_TMOP_HistoricalVehicles.json"
OUT = ROOT / "Page31Package"
EVENT = "Palme_shot_1"
SOURCE = "itdemokrati.nu/page31.html; user marker identification; supplied 70 s capture"

MARKER_LEGEND = {
    "people": {
        "black/brown":"PER_VALLIN", "white/brown":"CHRISTINA_VALLIN", "pink/brown":"SIRPA_LINDGREN",
        "green/gray":"CARINA_PETTERSSON", "green/yellow":"ULRIKA_RYTTERSTAL", "lightblue/red":"SUSANNE_KARLSSON",
        "blue/black":"ANDERS_BJORKMAN", "black/gray":"THE_KILLER", "brown/black":"OLOF_PALME",
        "red/white":"LISBET_PALME", "blue/black_large_inner":"CHRISTER_PETTERSSON", "purple/gray":"STIG_ENGSTROM",
        "black/white_large":"SVEN_ERIK_ROLFART_FODD_59", "black/white_small":"SUSANNE_LARSSON",
        "black/orange":"HELENA_LAHDE", "black/peach":"SVEN + BERIT", "yellow/red_late_car":"KARIN_JOHANSSON",
        "pink/red_late_car":"ANNE_HAGE", "gray/darkgray_late_car":"STEFAN_GLANTZ", "pink/lightgray_late_car":"LENA_BASEN",
        "blue/gray_late_car":"GORAN_ISRAELSSON (Bengt Göran Israelsson)", "black/gray_late_car":"ANDERS_ERSSON",
        "blue/black_late_right":"EGON_ENOCKSSON", "yellow/red_late_right":"ANNIKA_BLOMQVIST", "blue_marker_late_left":"ALF_LUNDIN"
    },
    "vehicles": {
        "yellow_tunnelgatan":"VEHICLE_INGE_MORELIUS_MORK_GUL_OPEL_REKORD_73_A",
        "white_left_of_lights":"VEHICLE_ANDERS_DELBOMS_TAXI_BIL_JARFALLA_TAXI_MITSUBISHI_GALANT",
        "gray_waiting_right":"VEHICLE_JAN_AKE_SVENSSON_S_BIL_BMW_GRA", "white_far_right":"VEHICLE_HANS_JOHANSSON_TAXI",
        "gray_later_from_left":"VEHICLE_LEIF_LJUNGQVIST_CHEVROLET_SUBURBAN", "gray_later_from_right":"VEHICLE_AKE_LARSSONS_BIL_FORD_ESCORT_GRAMETALLIC",
        "yellow_later_from_right":"VEHICLE_BENGT_PALM_MERCEDES_GUL", "limo_from_tunnelgatan":"VEHICLE_MERCEDES_LIMOSINE_JAN_NILSSON"
    }
}

# Screen-to-Unreal affine calibration.  Control points: Olof/Mordplatsen,
# Stig's existing shot anchor and Per Vallin's existing shot anchor.
H = (
    (9.79323360, 21.1905008),
    (-9.30450680, -1.45565371),
    (-3251.07505, -17768.4180),
)


def world(px: float, py: float) -> tuple[float, float]:
    return (
        px * H[0][0] + py * H[1][0] + H[2][0],
        px * H[0][1] + py * H[1][1] + H[2][1],
    )


def clock(offset: int) -> dict[str, int]:
    sec = 23 * 3600 + 21 * 60 + 30 + offset
    sec %= 24 * 3600
    return {"Hour": sec // 3600, "Minute": (sec % 3600) // 60, "Second": sec % 60}


def slug_offset(offset: int) -> str:
    return "T0" if offset == 0 else (f"TM{abs(offset):02d}" if offset < 0 else f"TP{offset:02d}")


# (offset, screen x, screen y, semantic, confidence).  Pixel readings are from
# the supplied capture: video seconds 4/19/24/31/69 correspond to T-20/-5/0/+7/+43.
TRACKS: dict[str, list[tuple[int, float, float, str, str]]] = {
    "PER_VALLIN": [(-20,543,664,"standard","Reconstructed"),(-5,693,664,"standard","Reconstructed"),(0,743,664,"shot","Reconstructed"),(7,773,664,"standard","Reconstructed"),(43,786,348,"crime_scene_final","Inferred")],
    "CHRISTINA_VALLIN": [(-20,558,674,"standard","Reconstructed"),(-5,708,674,"standard","Reconstructed"),(0,758,674,"shot","Reconstructed"),(7,788,674,"standard","Reconstructed"),(43,792,353,"crime_scene_final","Inferred")],
    "SIRPA_LINDGREN": [(-20,664,664,"standard","Reconstructed"),(-5,784,664,"standard","Reconstructed"),(0,824,664,"shot","Reconstructed"),(7,880,664,"standard","Reconstructed"),(43,800,357,"crime_scene_final","Inferred")],
    "CARINA_PETTERSSON": [(-20,664,676,"standard","Reconstructed"),(-5,784,676,"standard","Reconstructed"),(0,824,676,"shot","Reconstructed"),(7,880,676,"standard","Reconstructed"),(43,806,360,"crime_scene_final","Inferred")],
    "ULRIKA_RYTTERSTAL": [(-20,965,664,"standard","Reconstructed"),(-5,1085,664,"standard","Reconstructed"),(0,1125,664,"shot","Reconstructed"),(7,1168,664,"standard","Inferred"),(43,814,363,"crime_scene_final","Inferred")],
    "SUSANNE_KARLSSON": [(-20,965,675,"standard","Reconstructed"),(-5,1085,675,"standard","Reconstructed"),(0,1125,675,"shot","Reconstructed"),(7,1168,675,"standard","Inferred"),(43,820,359,"crime_scene_final","Inferred")],
    "ANDERS_BJORKMAN": [(-20,549,359,"standard","Reconstructed"),(-5,699,359,"standard","Reconstructed"),(0,749,359,"shot","Reconstructed"),(2,782,351,"takes_cover","Inferred"),(7,809,312,"doorway_cover","Reconstructed"),(18,795,337,"looks_out","Inferred"),(43,783,344,"crime_scene_final","Inferred")],
    "THE_KILLER": [(-20,795,344,"decorima_wait","Inferred"),(-12,790,345,"leaves_dekorima","Inferred"),(-5,786,344,"behind_palmes","Reconstructed"),(0,795,343,"shot","Reconstructed"),(2,805,334,"turns_to_tunnelgatan","Inferred"),(7,810,312,"escape_tunnelgatan","Reconstructed"),(15,815,275,"escape_east","Inferred")],
    "OLOF_PALME": [(-20,609,360,"standard","Reconstructed"),(-5,759,356,"standard","Reconstructed"),(0,798,346,"shot","Reconstructed"),(2,798,346,"falls","Inferred"),(7,798,346,"body","Reconstructed"),(43,798,346,"crime_scene_final","Reconstructed")],
    "LISBET_PALME": [(-20,610,368,"standard","Reconstructed"),(-5,759,362,"standard","Reconstructed"),(0,808,352,"shot","Reconstructed"),(2,812,350,"reacts","Inferred"),(7,808,355,"beside_olof","Reconstructed"),(43,804,351,"crime_scene_final","Inferred")],
    "CHRISTER_PETTERSSON": [(-20,798,331,"alternative_standard","Speculative"),(-5,798,331,"alternative_standard","Speculative"),(0,798,331,"alternative_shot","Speculative"),(7,811,305,"alternative_escape","Speculative")],
    "STIG_ENGSTROM": [(-20,394,364,"standard","Reconstructed"),(-5,543,364,"standard","Reconstructed"),(0,593,364,"shot","Reconstructed"),(7,663,364,"standard","Reconstructed"),(20,754,351,"arrives_scene","Inferred"),(43,787,344,"crime_scene_final","Reconstructed")],
    "SVEN_ERIK_ROLFART_FODD_59": [(-20,809,683,"standard","Reconstructed"),(-5,809,683,"standard","Reconstructed"),(0,809,683,"shot","Reconstructed"),(7,812,675,"standard","Inferred"),(43,828,355,"crime_scene_final","Inferred")],
    "SUSANNE_LARSSON": [(-20,808,692,"standard","Reconstructed"),(-5,808,692,"standard","Reconstructed"),(0,808,692,"shot","Reconstructed"),(7,812,684,"standard","Inferred"),(43,833,350,"crime_scene_final","Inferred")],
    "HELENA_LAHDE": [(-20,558,688,"standard","Reconstructed"),(-5,708,688,"standard","Reconstructed"),(0,758,688,"shot","Reconstructed"),(7,790,688,"standard","Inferred"),(43,824,346,"crime_scene_final","Inferred")],
    "SVEN": [(-20,758,700,"standard","Reconstructed"),(-5,758,700,"standard","Reconstructed"),(0,758,700,"shot","Reconstructed"),(7,775,693,"standard","Inferred"),(43,831,342,"crime_scene_final","Inferred")],
    "BERIT": [(-20,766,700,"standard","Reconstructed"),(-5,766,700,"standard","Reconstructed"),(0,766,700,"shot","Reconstructed"),(7,783,693,"standard","Inferred"),(43,836,347,"crime_scene_final","Inferred")],
    # The late walkers only receive anchors from their first visible/reconstructed point.
    "EGON_ENOCKSSON": [(24,1668,653,"enters_frame_east","Reconstructed"),(35,1460,640,"walks_west","Inferred"),(43,825,364,"crime_scene_final","Inferred")],
    "ANNIKA_BLOMQVIST": [(24,1668,660,"enters_frame_east","Reconstructed"),(35,1460,648,"walks_west","Inferred"),(43,838,360,"crime_scene_final","Inferred")],
    "ALF_LUNDIN": [(28,274,684,"enters_frame_west","Reconstructed"),(36,500,675,"walks_east","Inferred"),(43,780,360,"crime_scene_final","Inferred")],
}

# Vehicle screen positions are inspection/failsafe anchors. Occupants remain
# attached to seats and are not moved to these anchors individually.
VEHICLE_TRACKS: dict[str, list[tuple[int, float, float, str]]] = {
    "VEHICLE_INGE_MORELIUS_MORK_GUL_OPEL_REKORD_73_A": [(-20,962,765,"waiting"),(-5,962,765,"waiting"),(0,962,765,"shot"),(7,962,765,"waiting")],
    "VEHICLE_ANDERS_DELBOMS_TAXI_BIL_JARFALLA_TAXI_MITSUBISHI_GALANT": [(-20,1068,499,"waiting"),(-5,1068,499,"waiting"),(0,1068,499,"shot"),(7,1068,499,"waiting")],
    "VEHICLE_JAN_AKE_SVENSSON_S_BIL_BMW_GRA": [(-20,699,580,"waiting"),(-5,699,580,"waiting"),(0,699,580,"shot"),(7,769,580,"moving")],
    "VEHICLE_HANS_JOHANSSON_TAXI": [(-20,1439,584,"waiting"),(-5,1439,584,"waiting"),(0,1439,584,"shot"),(7,1534,584,"moving")],
    "VEHICLE_LEIF_LJUNGQVIST_CHEVROLET_SUBURBAN": [(0,545,580,"enters_frame_west"),(7,796,580,"at_scene")],
    "VEHICLE_AKE_LARSSONS_BIL_FORD_ESCORT_GRAMETALLIC": [(0,1200,499,"approaches_east"),(7,1140,499,"at_scene"),(12,1060,505,"stops")],
    "VEHICLE_BENGT_PALM_MERCEDES_GUL": [(-5,1135,459,"approaches_east"),(0,809,459,"at_shot"),(7,324,459,"continues_west")],
    "VEHICLE_MERCEDES_LIMOSINE_JAN_NILSSON": [(7,936,685,"enters_tunnelgatan"),(18,936,610,"northbound"),(26,930,545,"passes_scene")],
    "VEHICLE_GLANTZ_BASEN_TAXI_E9979": [(18,1000,610,"approaches"),(22,930,590,"stops"),(35,850,560,"scene")],
}

SEATED = {
    "INGE_MORELIUS_G": ("VEHICLE_INGE_MORELIUS_MORK_GUL_OPEL_REKORD_73_A", "DRIVER", 999),
    "ANDERS_DELBOM": ("VEHICLE_ANDERS_DELBOMS_TAXI_BIL_JARFALLA_TAXI_MITSUBISHI_GALANT", "DRIVER", 999),
    "JAN_AKE_SVENSSON": ("VEHICLE_JAN_AKE_SVENSSON_S_BIL_BMW_GRA", "DRIVER", 999),
    "HANS_JOHANSSON": ("VEHICLE_HANS_JOHANSSON_TAXI", "DRIVER", 999),
    "LEIF_LJUNGQVIST": ("VEHICLE_LEIF_LJUNGQVIST_CHEVROLET_SUBURBAN", "DRIVER", 999),
    "AKE_LARSSON": ("VEHICLE_AKE_LARSSONS_BIL_FORD_ESCORT_GRAMETALLIC", "DRIVER", 999),
    "BENGT_PALM": ("VEHICLE_BENGT_PALM_MERCEDES_GUL", "DRIVER", 999),
    "JAN_NILSSON": ("VEHICLE_MERCEDES_LIMOSINE_JAN_NILSSON", "DRIVER", 999),
    "ANNE_HAGE": ("VEHICLE_AKE_LARSSONS_BIL_FORD_ESCORT_GRAMETALLIC", "REAR_RIGHT", 8),
    "KARIN_JOHANSSON": ("VEHICLE_AKE_LARSSONS_BIL_FORD_ESCORT_GRAMETALLIC", "REAR_CENTER", 8),
    "STEFAN_GLANTZ": ("VEHICLE_GLANTZ_BASEN_TAXI_E9979", "REAR_LEFT", 24),
    "LENA_BASEN": ("VEHICLE_GLANTZ_BASEN_TAXI_E9979", "REAR_RIGHT", 24),
    "GORAN_ISRAELSSON": ("VEHICLE_GLANTZ_BASEN_TAXI_E9979", "FRONT_PASSENGER", 25),
    "ANDERS_ERSSON": ("VEHICLE_GLANTZ_BASEN_TAXI_E9979", "REAR_CENTER", 25),
}


def anchor_id(entity: str, offset: int, semantic: str, vehicle: bool = False) -> str:
    prefix = "ANCHOR_PAGE31_VEHICLE" if vehicle else "ANCHOR_PAGE31"
    suffix = "_CRIME_SCENE_FINAL" if semantic == "crime_scene_final" else f"_{slug_offset(offset)}"
    if semantic not in {"standard", "shot", "crime_scene_final"}:
        suffix += "_" + semantic.upper()
    return f"{prefix}_{entity}{suffix}"


def redundant_position(previous, current) -> bool:
    if previous is None or math.hypot(current[1]-previous[1], current[2]-previous[2]) >= 2:
        return False
    # These carry a distinct physical/action state even without XY movement.
    return current[3] not in {"falls", "body", "reacts", "beside_olof", "crime_scene_final"}


def make_anchor(entity: str, point, next_point=None, vehicle=False):
    offset, px, py, semantic = point[:4]
    confidence = point[4] if len(point) > 4 else "Reconstructed"
    x, y = world(px, py)
    z = 60.0 if vehicle or py < 500 or semantic == "crime_scene_final" else 51.0
    yaw = 0.0
    if next_point:
        nx, ny = world(next_point[1], next_point[2])
        if math.hypot(nx-x, ny-y) > 1:
            yaw = math.degrees(math.atan2(ny-y, nx-x))
    return {
        "anchor_id": anchor_id(entity, offset, semantic, vehicle),
        "display_name": f"{entity} {slug_offset(offset)} {semantic}",
        "category": "VEHICLE_POSITION" if vehicle else "WITNESS_POSITION",
        "confidence": confidence.upper(),
        "event_id": EVENT,
        "event_offset_seconds": offset,
        "source_time": f"{EVENT}{offset:+d}s",
        "source_video_seconds": offset + 24,
        "source_screen_px": {"x": px, "y": py},
        "source_object": entity,
        "notes": "Page 31 marker position calibrated into Unreal centimetres. Pixel precision is limited by the supplied raster capture; confidence records reconstructed/inferred points.",
        "unreal_location_cm": {"x": round(x,3), "y": round(y,3), "z": z},
        "blender_rotation_euler_degrees": {"x": 0.0, "y": 0.0, "z": round(-yaw,3)},
    }


def timeline_template(person: dict) -> dict:
    if person.get("Timeline"):
        return copy.deepcopy(person["Timeline"][0])
    return {
        "EntryId":"", "Action":"Wait", "Time":clock(0), "TimingMode":"Relative",
        "SharedEventId":EVENT, "EventOffsetSeconds":0, "bTimeIsArrival":False,
        "TravelSpeedOverrideCmPerSecond":0, "LocationType":"Unknown", "TargetAnchorId":"None",
        "PassAnchorIds":[], "TargetEntityId":"None", "TargetSeatId":"None", "TargetStopId":"None",
        "OrderedLaneIds":[], "VehicleRouteMode":"ManualLaneRoute", "DrivingDestinationAnchorId":"None",
        "VehicleStartDistanceAlongFirstLaneCm":0, "TargetGroupId":"None",
        "GroupDefinition":{"GroupId":"None","MemberEntityIds":[],"LeaderEntityId":"None","Formation":"SideBySide","FormationSpacing":110},
        "SplitGroupDefinitions":[], "NewGroupLeaderEntityId":"None",
        "WorldTransform":{"Rotation":{"X":0,"Y":0,"Z":0,"W":1},"Translation":{"X":0,"Y":0,"Z":0},"Scale3D":{"X":1,"Y":1,"Z":1}},
        "ActivityState":"Standing", "LifeState":"Alive", "bTeleportDuringCatchUp":True,
        "Confidence":"Reconstructed", "SourceReference":SOURCE, "Notes":""
    }


def entry(person: dict, offset: int, action: str, target="None", *, semantic="", activity="Standing",
          confidence="Reconstructed", vehicle="None", seat="None", life="Alive", notes="") -> dict:
    e = timeline_template(person)
    e.update({
        "EntryId": f"PAGE31_{person['Name']}_{slug_offset(offset)}_{semantic or action.upper()}",
        "Action": action, "Time": clock(offset), "TimingMode": "Relative", "SharedEventId": EVENT,
        "EventOffsetSeconds": offset, "bTimeIsArrival": action == "MoveToAnchor",
        "LocationType": "VehicleSeat" if vehicle != "None" else ("Anchor" if target != "None" else "Unknown"),
        "TargetAnchorId": target, "PassAnchorIds": [], "TargetEntityId": vehicle,
        "TargetSeatId": seat, "ActivityState": activity, "LifeState": life,
        "Confidence": confidence, "SourceReference": SOURCE,
        "Notes": notes or f"Page 31 reconstruction, {slug_offset(offset)} relative to {EVENT}. CRIME_SCENE_FINAL is a milestone and does not terminate later timeline actions.",
    })
    return e


def effective_seconds(e: dict) -> int | None:
    if e.get("TimingMode") == "Relative" and e.get("SharedEventId") == EVENT:
        return 23*3600+21*60+30+int(e.get("EventOffsetSeconds",0))
    t=e.get("Time") or {}
    if all(k in t for k in ("Hour","Minute","Second")):
        return int(t["Hour"])*3600+int(t["Minute"])*60+int(t["Second"])
    return None


def clean_window(person: dict) -> None:
    kept=[]
    seated = SEATED.get(person["Name"])
    is_driver = seated is not None and seated[2] >= 900
    for e in person.get("Timeline",[]):
        target=str(e.get("TargetAnchorId",""))
        eid=str(e.get("EntryId",""))
        sec=effective_seconds(e)
        rel_to_shot = e.get("SharedEventId")==EVENT
        rel_offset = int(e.get("EventOffsetSeconds",0)) if rel_to_shot else 0
        passenger_replacement = seated is not None and not is_driver and e.get("Action") in {"ExitVehicle","MoveToAnchor"} and rel_offset <= 120
        old_shot = target.startswith("ANCHOR_SHOT1_") or (rel_to_shot and rel_offset <= 43) or passenger_replacement
        in_window = sec is not None and 23*3600+21*60+10 <= sec <= 23*3600+23*60+30
        protected_driver_action = is_driver and e.get("Action") in {
            "EnterVehicle", "BeginDriving", "StopDriving", "Stop", "ExitVehicle"
        }
        # Replace the short reconstruction window only.  Non-Blender actions
        # after T+43 (departures, interviews, ambulance actions, etc.) survive.
        before_final = sec is not None and sec <= 23*3600+22*60+13
        replaceable = in_window and ("BLENDER" in eid or (before_final and (person["Name"] in TRACKS or person["Name"] in SEATED)))
        if protected_driver_action or (not old_shot and not replaceable):
            kept.append(e)
    person["Timeline"]=kept


def add_foot_timeline(person: dict, anchors_by_id: dict) -> None:
    points=TRACKS[person["Name"]]
    previous=None
    for p in points:
        offset, px, py, semantic, confidence=p
        aid=anchor_id(person["Name"],offset,semantic)
        if redundant_position(previous,p):
            continue
        action="Wait" if semantic in {"crime_scene_final","body"} else "MoveToAnchor"
        activity="Running" if semantic in {"escape_tunnelgatan","escape_east","takes_cover"} else "Walking"
        if semantic in {"shot","falls","body","reacts","beside_olof","crime_scene_final","decorima_wait"}:
            activity="Standing"
        person["Timeline"].append(entry(person,offset,action,aid,semantic=semantic,activity=activity,confidence=confidence))
        previous=p
    if person["Name"]=="OLOF_PALME":
        person["Timeline"].append(entry(person,0,"ChangeLifeState",semantic="death",life="Dead",notes="Olof Palmes life state changes at the shared first-shot event; position remains at the T0/body anchor."))


def add_seated_timeline(person: dict, anchors_by_id: dict) -> None:
    vehicle, seat, exit_at=SEATED[person["Name"]]
    for off in (-20,-5,0,7):
        if off >= exit_at:
            break
        person["Timeline"].append(entry(person,off,"Wait",semantic="seated",activity="Sitting",vehicle=vehicle,seat=seat,
            notes=f"Remains attached to {vehicle} seat {seat}; the vehicle route owns world movement."))
    if exit_at >= 900:
        return
    person["Timeline"].append(entry(person,exit_at,"ExitVehicle",semantic="exit_vehicle",activity="Standing",vehicle=vehicle,seat=seat,
        notes=f"Leaves {vehicle} after it reaches the crime-scene area. Exit timing reconstructed from the supplied animation."))
    # Shared door/run/final anchors, offset slightly per person to avoid stacking.
    lane=list(SEATED).index(person["Name"]) % 4
    if exit_at < 20:
        pts=[(exit_at+1,1035+lane*6,570+lane*5,"door_side","Reconstructed"),(exit_at+5,900+lane*5,455+lane*4,"runs_to_scene","Inferred"),(exit_at+12,790+lane*8,342+lane*5,"crime_scene_final","Inferred")]
    else:
        pts=[(exit_at+1,925+lane*6,565+lane*5,"door_side","Reconstructed"),(exit_at+5,875+lane*5,455+lane*4,"runs_to_scene","Inferred"),(exit_at+14,805+lane*7,350+lane*5,"crime_scene_final","Inferred")]
    for i,p in enumerate(pts):
        a=make_anchor(person["Name"],(*p,"Inferred"),pts[i+1] if i+1<len(pts) else None)
        anchors_by_id[a["anchor_id"]]=a
        activity="Running" if p[3]=="runs_to_scene" else "Walking"
        action="Wait" if p[3]=="crime_scene_final" else "MoveToAnchor"
        person["Timeline"].append(entry(person,p[0],action,a["anchor_id"],semantic=p[3],activity=activity,confidence="Inferred"))


def sort_timeline(person: dict) -> None:
    priority={"EnterVehicle":0,"BeginDriving":1,"MoveToAnchor":2,"ExitVehicle":3,"ChangeLifeState":4,"ChangeActivity":5,"Wait":6}
    def key(e):
        s=effective_seconds(e)
        return (s if s is not None else 10**9, priority.get(e.get("Action"),5), e.get("EntryId",""))
    person["Timeline"].sort(key=key)


def main() -> None:
    OUT.mkdir(parents=True,exist_ok=True)
    people=json.loads(PEOPLE_IN.read_text(encoding="utf-8-sig"))
    vehicles=json.loads(VEHICLES_IN.read_text(encoding="utf-8-sig"))
    people_by={p["Name"]:p for p in people}
    vehicle_by={v["Name"]:v for v in vehicles}
    missing_people=sorted((set(TRACKS)|set(SEATED))-set(people_by))
    missing_vehicles=sorted(set(VEHICLE_TRACKS)-set(vehicle_by))
    if missing_people or missing_vehicles:
        raise SystemExit(f"Missing rows: people={missing_people}; vehicles={missing_vehicles}")

    anchors_by_id={}
    for entity,points in TRACKS.items():
        previous=None
        for i,p in enumerate(points):
            if redundant_position(previous,p):
                continue
            a=make_anchor(entity,p,points[i+1] if i+1<len(points) else None)
            anchors_by_id[a["anchor_id"]]=a
            previous=p
    for entity,points in VEHICLE_TRACKS.items():
        previous=None
        for i,p in enumerate(points):
            if redundant_position(previous,p):
                continue
            a=make_anchor(entity,p,points[i+1] if i+1<len(points) else None,True)
            anchors_by_id[a["anchor_id"]]=a
            previous=p

    changed=[]
    for name in sorted(set(TRACKS)|set(SEATED)):
        p=people_by[name]
        clean_window(p)
        if name in TRACKS:
            add_foot_timeline(p,anchors_by_id)
        else:
            add_seated_timeline(p,anchors_by_id)
        sort_timeline(p)
        changed.append(name)
        if name=="GORAN_ISRAELSSON":
            p["FullName"]='NSLOCTEXT("DT_TMOP_People [5FF9126A2E58348990CC01ABAA09CC1A]", "GORAN_ISRAELSSON_FullName", "Bengt Göran Israelsson")'
            p["FirstName"]="Bengt Göran"
            p["LastName"]="Israelsson"
            p["Notes"]=(p.get("Notes","")+" Page31 marker identity confirmed as Bengt Göran Israelsson; canonical EntityId GORAN_ISRAELSSON retained for reference stability.").strip()

    page_entries=[e for p in people for e in p.get("Timeline",[]) if str(e.get("EntryId","")).startswith("PAGE31_")]
    duplicate_entry_rows=[]
    missing_anchor_refs=[]
    for p in people:
        ids=[e.get("EntryId") for e in p.get("Timeline",[])]
        if len(ids)!=len(set(ids)):
            duplicate_entry_rows.append(p["Name"])
        for e in p.get("Timeline",[]):
            if not str(e.get("EntryId","")).startswith("PAGE31_"):
                continue
            target=e.get("TargetAnchorId","None")
            if target!="None" and target not in anchors_by_id:
                missing_anchor_refs.append({"person":p["Name"],"anchor":target})
    if duplicate_entry_rows or missing_anchor_refs:
        raise SystemExit(f"Validation failed: duplicate entries={duplicate_entry_rows}; missing anchors={missing_anchor_refs}")

    anchors={
        "format":"TMOP_PAGE31_RELATIVE_TIMELINE_ANCHORS_V1",
        "source_page":"http://www.itdemokrati.nu/page31.html",
        "source_capture":"7abdcac5-cdc2-4cfa-b6cb-263072af1f12.mp4",
        "source_shot_time":"23:21:22",
        "game_shared_event_id":EVENT,
        "nominal_game_event_time":"23:21:30",
        "coordinate_method":"2D affine calibration from three verified simulation-marker/Unreal-anchor control points",
        "precision_notice":"The capture is rasterised and markers overlap. Reconstructed and inferred coordinates are explicitly labelled; they are not claimed as surveyed historical coordinates.",
        "anchor_count":len(anchors_by_id),
        "anchors":list(anchors_by_id.values()),
    }
    (OUT/"Content/TMOP/Data").mkdir(parents=True,exist_ok=True)
    (OUT/"DataTables/08_25").mkdir(parents=True,exist_ok=True)
    (OUT/"Content/TMOP/Data/TMOP_PAGE31_WITNESS_TIMELINE_ANCHORS.json").write_text(json.dumps(anchors,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
    # Unreal exports commonly use UTF-16. Keep the downloadable replacement files import-ready.
    (OUT/"DataTables/08_25/DT_TMOP_People.json").write_text(json.dumps(people,ensure_ascii=False,indent=2)+"\n",encoding="utf-16")
    (OUT/"DataTables/08_25/DT_TMOP_HistoricalVehicles.json").write_text(json.dumps(vehicles,ensure_ascii=False,indent=2)+"\n",encoding="utf-16")
    report={
        "base_people":"08_25/DT_TMOP_People.json",
        "base_vehicles":"08_25/DT_TMOP_HistoricalVehicles.json",
        "uppslag":"08_26 is intentionally untouched and not packaged",
        "changed_people_count":len(changed),
        "changed_people":changed,
        "vehicle_rows_changed":0,
        "vehicle_anchor_entities":sorted(VEHICLE_TRACKS),
        "anchor_count":len(anchors_by_id),
        "page31_timeline_entry_count":len(page_entries),
        "duplicate_anchor_ids":len(anchors_by_id)!=len(set(anchors_by_id)),
        "duplicate_timeline_entry_rows":duplicate_entry_rows,
        "missing_page31_anchor_references":missing_anchor_refs,
        "missing_people":missing_people,
        "missing_vehicles":missing_vehicles,
        "identity_note":"Confirmed full name Bengt Göran Israelsson; canonical EntityId GORAN_ISRAELSSON retained for reference stability.",
        "validation_status":"PASS",
    }
    (OUT/"VALIDATION.json").write_text(json.dumps(report,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
    (OUT/"PAGE31_MARKER_LEGEND.json").write_text(json.dumps(MARKER_LEGEND,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")


if __name__=="__main__":
    main()
