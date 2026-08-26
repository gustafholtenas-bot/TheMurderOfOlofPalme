"""Load and pin existing Page31 anchors without changing their data.

Run inside Unreal Editor. This script deliberately does NOT spawn, delete,
rename, move, rotate, modify, or save any actor. It only asks World Partition
to load and pin existing actor descriptors matching ANCHOR_PAGE31_.
"""

import unreal


PREFIX = "ANCHOR_PAGE31_"


def read_desc_value(desc, names):
    for name in names:
        try:
            value = desc.get_editor_property(name)
            if value is not None:
                return value
        except Exception:
            pass
        try:
            value = getattr(desc, name)
            if value is not None:
                return value
        except Exception:
            pass
    return None


descs = unreal.WorldPartitionBlueprintLibrary.get_actor_descs() or []
matching_guids = []
matching_labels = []

for desc in descs:
    label_value = read_desc_value(
        desc,
        ("actor_label", "label", "actor_name", "name"),
    )
    folder_value = read_desc_value(
        desc,
        ("folder_path", "folder", "actor_folder_path"),
    )
    label = str(label_value or "")
    folder = str(folder_value or "")

    is_page31 = label.upper().startswith(PREFIX)
    if not is_page31:
        normalized_folder = folder.replace("\\", "/").upper()
        is_page31 = "/PAGE31" in normalized_folder or "PAGE31TIMELINE" in normalized_folder
    if not is_page31:
        continue

    guid = read_desc_value(desc, ("guid", "actor_guid"))
    if guid is None:
        unreal.log_warning(
            "TMOP recovery: Page31 descriptor has no readable GUID: {}".format(label)
        )
        continue

    matching_guids.append(guid)
    matching_labels.append(label or folder)

if not matching_guids:
    unreal.log_error(
        "TMOP recovery: found no Page31 actor descriptors. Nothing was changed. "
        "Keep the editor open and inspect the Output Log."
    )
else:
    unreal.WorldPartitionBlueprintLibrary.load_actors(matching_guids)
    unreal.WorldPartitionBlueprintLibrary.pin_actors(matching_guids)
    unreal.log(
        "TMOP recovery: loaded and pinned {} existing Page31 actors. "
        "No actor data or transforms were changed and nothing was saved.".format(
            len(matching_guids)
        )
    )

