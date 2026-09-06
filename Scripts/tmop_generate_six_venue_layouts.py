"""Generate supplemental venue anchors for TMOPVenueLayoutImporter.

Run with Python 3 outside Unreal. Only the supplemental JSON is written.
These are adjustable gameplay layouts, not historical floor plans.
"""
import json
from pathlib import Path

VENUES = {
    'KlubbOxen': 'KlubbOxen_inside',
    'RestaurangRiviera': 'RestaurangRiviera_inside',
    'Tegnerkallaren': 'Tegnerkallaren_inside',
    'newgeneration': 'newgeneration_inside',
    'MonteCarlo': 'MonteCarlo_inside',
    'KingCreole': 'KingCreole_inside',
}

def build():
    anchors = []
    for venue, parent in VENUES.items():
        def add(suffix, role, x, y, yaw):
            name = venue + suffix
            anchors.append(dict(anchor_id=name, display_name=name,
                parent_anchor_id=parent, role=role,
                relative_offset_cm=dict(x=x, y=y, z=0),
                relative_yaw_degrees=yaw,
                notes='Gameplay-rekonstruktion: justerbar lokalplan, inte historiskt belagd möblering. Scenplatser är valfria reservpunkter.'))
        # Four rows of four tables, four inward-facing seats per table.
        for table in range(16):
            x, y = (table % 4 - 1.5) * 300, (table // 4) * 300
            for seat, (dx, dy, yaw) in enumerate(
                    ((0,-90,90),(90,0,180),(0,90,-90),(-90,0,0)),1):
                add(f'_table_{table+1:02d}_seat_{seat:02d}',
                    'TableSeat',x+dx,y+dy,yaw)
        for seat in range(12):
            add(f'_bar_seat_{seat+1:02d}','BarSeat',
                (seat-5.5)*100,-350,-90)
        for staff in range(4):
            add(f'_bar_staff_{staff+1:02d}','BarStaffStanding',
                (staff-1.5)*220,-550,90)
        for musician in range(6):
            add(f'_stage_{musician+1:02d}','MusicianStanding',
                (musician-2.5)*150,1250,-90)
    assert len(anchors)==516
    assert len({a['anchor_id'].lower() for a in anchors})==516
    assert len({(a['parent_anchor_id'], *a['relative_offset_cm'].values()) for a in anchors})==516
    return dict(format='TMOP_VENUE_LAYOUT_ANCHORS_V1', units='centimeters',
        venue_count=len(VENUES),anchor_count=len(anchors),
        notes='Supplement only. Verify Riviera and Tegnerkallaren parent IDs in the level before import.',
        anchors=anchors)

if __name__=='__main__':
    output=Path(__file__).resolve().parents[1]/'Content/TMOP/Data/TMOP_VENUE_LAYOUT_ANCHORS_EXTRA_6.json'
    output.parent.mkdir(parents=True,exist_ok=True)
    output.write_text(json.dumps(build(),ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(output)
