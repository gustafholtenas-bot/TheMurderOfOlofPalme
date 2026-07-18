"""Import TMOP Unreal anchor/seat JSON as Blender empties.

Blender: Scripting workspace -> open this file -> set JSON_PATH -> Run Script.
"""

import json
import os
import bpy


JSON_PATH = r"C:\Users\User\Documents\Unreal Projects\TheMurderOfOlofPalme\Saved\TMOP\Exports\TMOP_AnchorsAndSeats.json"
CLEAR_OLD_COLLECTIONS = True


def _collection(name):
    existing = bpy.data.collections.get(name)
    if existing and CLEAR_OLD_COLLECTIONS:
        for obj in list(existing.objects):
            bpy.data.objects.remove(obj, do_unlink=True)
        bpy.data.collections.remove(existing)
        existing = None
    if existing is None:
        existing = bpy.data.collections.new(name)
        bpy.context.scene.collection.children.link(existing)
    return existing


def _make_empty(collection, name, location, display_type, size, properties):
    obj = bpy.data.objects.new(name, None)
    obj.empty_display_type = display_type
    obj.empty_display_size = size
    obj.location = (location["x"], location["y"], location["z"])
    collection.objects.link(obj)
    for key, value in properties.items():
        if isinstance(value, (str, int, float, bool)):
            obj[key] = value
    return obj


def import_tmop_anchors_and_seats():
    path = bpy.path.abspath(JSON_PATH)
    if not os.path.isfile(path):
        raise FileNotFoundError("TMOP JSON not found: " + path)
    with open(path, "r", encoding="utf-8") as source:
        data = json.load(source)
    if data.get("format") != "TMOP_ANCHORS_AND_SEATS_V1":
        raise ValueError("Unsupported TMOP JSON format")

    anchor_collection = _collection("TMOP_Anchors")
    seat_collection = _collection("TMOP_CinemaSeats")

    for item in data.get("anchors", []):
        _make_empty(
            anchor_collection,
            "ANCHOR__" + item["anchor_id"],
            item["blender_location_m"],
            "ARROWS",
            0.35,
            {
                "tmop_type": "HistoricalAnchor",
                "anchor_id": item["anchor_id"],
                "display_name": item.get("display_name", ""),
                "category": item.get("category", ""),
                "parent_place_id": item.get("parent_place_id", ""),
                "source_level": data.get("level", ""),
            },
        )

    for item in data.get("cinema_seats", []):
        _make_empty(
            seat_collection,
            "SEAT__" + item["seat_id"],
            item["blender_location_m"],
            "CUBE",
            0.22,
            {
                "tmop_type": "CinemaSeat",
                "seat_id": item["seat_id"],
                "venue_id": item.get("venue_id", ""),
                "auditorium_id": item.get("auditorium_id", ""),
                "row_number": item.get("row_number", -1),
                "seat_number": item.get("seat_number", -1),
                "source_level": data.get("level", ""),
            },
        )

    print("TMOP import complete: {} anchors, {} seats".format(
        len(data.get("anchors", [])), len(data.get("cinema_seats", []))))


import_tmop_anchors_and_seats()

