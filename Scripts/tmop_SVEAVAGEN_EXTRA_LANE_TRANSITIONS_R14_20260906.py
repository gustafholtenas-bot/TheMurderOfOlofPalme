# Supplemental to R11. Six adjacent-lane transitions, using live lane geometry.
# Includes SVEAVAGENN_004_R1 -> SVEAVAGENN_005_R3.
APPLY_CHANGES = True
PLAN = [{'road': 'SVEAVAGEN', 'source': 'SVEAVAGENN_004_R2', 'destination': 'SVEAVAGENN_005_R1', 'connector': 'X_SVEAVAGENN_004_R2_TO_SVEAVAGENN_005_R1_STRAIGHT', 'turn': 'STRAIGHT', 'status': 'create', 'lane_change': True}, {'road': 'SVEAVAGEN', 'source': 'SVEAVAGENN_004_R1', 'destination': 'SVEAVAGENN_005_R3', 'connector': 'X_SVEAVAGENN_004_R1_TO_SVEAVAGENN_005_R3_STRAIGHT', 'turn': 'STRAIGHT', 'status': 'create', 'lane_change': True}, {'road': 'SVEAVAGEN', 'source': 'SVEAVAGENN_006_R3', 'destination': 'SVEAVAGENN_007_R1', 'connector': 'X_SVEAVAGENN_006_R3_TO_SVEAVAGENN_007_R1_STRAIGHT', 'turn': 'STRAIGHT', 'status': 'create', 'lane_change': True}, {'road': 'SVEAVAGEN', 'source': 'SVEAVAGENS_003_R2', 'destination': 'SVEAVAGENS_004_R1', 'connector': 'X_SVEAVAGENS_003_R2_TO_SVEAVAGENS_004_R1_STRAIGHT', 'turn': 'STRAIGHT', 'status': 'create', 'lane_change': True}, {'road': 'SVEAVAGEN', 'source': 'SVEAVAGENS_005_R1', 'destination': 'SVEAVAGENS_006_R2', 'connector': 'X_SVEAVAGENS_005_R1_TO_SVEAVAGENS_006_R2_STRAIGHT', 'turn': 'STRAIGHT', 'status': 'create', 'lane_change': True}, {'road': 'SVEAVAGEN', 'source': 'SVEAVAGENS_005_R3', 'destination': 'SVEAVAGENS_006_R1', 'connector': 'X_SVEAVAGENS_005_R3_TO_SVEAVAGENS_006_R1_STRAIGHT', 'turn': 'STRAIGHT', 'status': 'create', 'lane_change': True}]
import math
import unreal


def get(obj, key):
    return obj.get_editor_property(key)


def discover():
    result = {}
    for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
        for lane in actor.get_components_by_class(unreal.TMOPTrafficLaneComponent):
            lid = str(get(lane, 'lane_id'))
            if lid in ('', 'None'): continue
            if lid in result: raise RuntimeError('Duplicate Lane ID: '+lid)
            result[lid] = lane
    return result


def endpoint(lane, end):
    n = lane.get_number_of_spline_points()
    if n < 2: raise RuntimeError('Lane has fewer than two points')
    i = n-1 if end else 0
    return lane.get_location_at_spline_point(i, unreal.SplineCoordinateSpace.WORLD)


def direction(lane, end):
    n = lane.get_number_of_spline_points()
    i = n-1 if end else 0
    v = lane.get_tangent_at_spline_point(i, unreal.SplineCoordinateSpace.WORLD)
    length = v.length()
    if length < .001:
        v = endpoint(lane, True)-endpoint(lane, False)
        length = v.length()
    if length < .001: raise RuntimeError('Zero lane tangent')
    return v/length


def curve(a, b):
    start, end = endpoint(a, True), endpoint(b, False)
    distance = (end-start).length()
    if distance > 15000: raise RuntimeError('Crossing endpoints more than 150 m apart; check lane positions.')
    handle = max(100., min(900., distance*.42))
    p1, p2 = start+direction(a,True)*handle, end-direction(b,False)*handle
    pts=[]; tangents=[]
    steps=max(16, int(math.ceil(distance/50)))
    for i in range(steps+1):
        t=i/steps; u=1-t
        pts.append(start*u**3+p1*(3*u*u*t)+p2*(3*u*t*t)+end*t**3)
        tangents.append(((p1-start)*(3*u*u)+(p2-p1)*(6*u*t)+(end-p2)*(3*t*t))/steps)
    return pts,tangents


def linked(lane, target):
    return any(str(get(c,'target_lane_id'))==target and get(c,'allowed') for c in get(lane,'next_lanes'))


def link(lane, target, turn):
    values=list(get(lane,'next_lanes')); found=False
    for c in values:
        if str(get(c,'target_lane_id'))==target:
            c.set_editor_property('allowed',True); found=True
    if not found:
        c=unreal.TMOPLaneConnection()
        c.set_editor_property('target_lane_id',unreal.Name(target))
        c.set_editor_property('turn_type',getattr(unreal.TMOPTrafficTurnType,turn))
        c.set_editor_property('allowed',True); values.append(c)
    lane.modify(); lane.set_editor_property('next_lanes',values)


def main(apply=True):
    lanes=discover(); work=[]
    for p in PLAN:
        a,b,c=p['source'],p['destination'],p['connector']
        if a not in lanes or b not in lanes: raise RuntimeError('Missing road lane: '+a+' / '+b)
        if c in lanes and linked(lanes[a],c) and linked(lanes[c],b): continue
        if p.get('lane_change'):
            f=direction(lanes[a],True); g=direction(lanes[b],False)
            delta=endpoint(lanes[b],False)-endpoint(lanes[a],True)
            forward=delta.x*f.x+delta.y*f.y+delta.z*f.z
            lateral=abs(-delta.x*f.y+delta.y*f.x)
            alignment=f.x*g.x+f.y*g.y+f.z*g.z
            if not (800<=forward<=6000 and lateral<=450 and forward>=4*lateral and abs(delta.z)<=150 and alignment>=.95):
                raise RuntimeError('Lane change geometry has changed: '+c+'. No changes made.')
        points=curve(lanes[a],lanes[b]) if c not in lanes else None
        work.append((p,points))
    unreal.log('TMOP R14: {} connectors to create, {} existing chains to enable/repair.'.format(
        sum(points is not None for _,points in work),sum(points is None for _,points in work)))
    for p,points in work: unreal.log(('CREATE ' if points else 'LINK/ENABLE ')+p['connector'])
    if not apply:
        unreal.log('Preview only. No changes.'); return
    if not work:
        unreal.log('All requested connections are already enabled.'); return
    subsystem=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    # Store independent struct copies before mutating any connections.
    backups={}; created=[]
    for p,_ in work:
        for lid in (p['source'],p['connector']):
            if lid in lanes and lid not in backups:
                backups[lid]=[c.copy() for c in get(lanes[lid],'next_lanes')]
    with unreal.ScopedEditorTransaction('TMOP R14 Complete Sveavagen junction connections'):
        try:
            for p,geometry in work:
                cid=p['connector']
                if geometry is not None:
                    actor=subsystem.spawn_actor_from_class(unreal.TMOPLaneSplineActor,unreal.Vector(),unreal.Rotator())
                    if actor is None: raise RuntimeError('Could not create '+cid)
                    created.append(actor); actor.modify(); actor.set_actor_label(cid)
                    actor.set_folder_path('TMOP Traffic Network/Sveavagen Connections R14')
                    actor.set_editor_property('lane_id',unreal.Name(cid))
                    actor.set_editor_property('is_crossing',True)
                    lane=get(actor,'lane_spline'); lane.modify()
                    lane.set_editor_property('lane_id',unreal.Name(cid))
                    lane.set_editor_property('road_id',unreal.Name('CROSSING'))
                    lane.set_editor_property('direction_id',unreal.Name('CROSSING'))
                    lane.set_editor_property('right_hand_traffic',True)
                    lane.set_editor_property('speed_limit_kmh',min(get(lanes[p['source']],'speed_limit_kmh'),get(lanes[p['destination']],'speed_limit_kmh')))
                    lane.set_editor_property('next_lanes',[])
                    pts,tangents=geometry
                    lane.clear_spline_points(False)
                    for point in pts: lane.add_spline_point(point,unreal.SplineCoordinateSpace.WORLD,False)
                    for i,tangent in enumerate(tangents):
                        lane.set_spline_point_type(i,unreal.SplinePointType.CURVE_CUSTOM_TANGENT,False)
                        lane.set_tangent_at_spline_point(i,tangent,unreal.SplineCoordinateSpace.WORLD,False)
                    lane.set_closed_loop(False,False); lane.update_spline(); lane.set_draw_debug(True)
                    lanes[cid]=lane
                link(lanes[p['source']],cid,p['turn'])
                link(lanes[cid],p['destination'],'STRAIGHT')
            for p in PLAN:
                a,b,c=p['source'],p['destination'],p['connector']
                if not linked(lanes[a],c) or not linked(lanes[c],b): raise RuntimeError('Chain verification failed: '+c)
            for p,geometry in work:
                if geometry is not None:
                    c=lanes[p['connector']]
                    if (endpoint(c,False)-endpoint(lanes[p['source']],True)).length()>1 or (endpoint(c,True)-endpoint(lanes[p['destination']],False)).length()>1:
                        raise RuntimeError('Endpoint verification failed: '+p['connector'])
        except Exception:
            for lid,values in backups.items(): lanes[lid].set_editor_property('next_lanes',values)
            for actor in reversed(created): subsystem.destroy_actor(actor)
            raise
    unreal.log('TMOP R14 COMPLETE: all 6 requested chains verified. New endpoint gaps <= 1 cm. Inspect curves, then save the level. Undo is available.')


main(APPLY_CHANGES)
