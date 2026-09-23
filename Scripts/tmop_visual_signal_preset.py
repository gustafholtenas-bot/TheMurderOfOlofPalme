"""Select ONE signal controller in Unreal, set PRESET/SHOT_SECONDS, then execute.
Replaces only that controller's program. Use Undo to revert. Does not save the map.
Names describe travel direction (NB = northbound, SB = southbound).
All durations are a visual reconstruction, NOT a recovered 1986 signal plan.
"""
PRESET = 'TUNNELGATAN'  # or APELBERGSGATAN / KUNGSGATAN
SHOT_SECONDS = 23 * 3600 + 21 * 60 + 30  # MUST match the project's first-shot time


def make_program(preset, shot_seconds):
    if preset not in ('TUNNELGATAN', 'APELBERGSGATAN', 'KUNGSGATAN'):
        raise ValueError('Unknown preset')
    if not 82800 <= shot_seconds <= 85499:
        raise ValueError('First shot must be within the playable period')
    groups = ['VEH_NB_THROUGH', 'VEH_SB_THROUGH', 'VEH_NB_LEFT', 'VEH_WEST_APPROACH',
              'VEH_EAST_APPROACH', 'PED_NORTH', 'PED_SOUTH', 'PED_WEST', 'PED_EAST']
    phases = []
    def phase(seconds, active=(), state='Red'):
        phases.append({'duration': seconds, 'states': {g: state if g in active else 'Red' for g in groups}})
    def stage(active, seconds):
        phase(1, active, 'RedYellow')
        phase(seconds, active, 'Green')
        phase(3, active, 'GreenYellow')
        phase(2)
    # At Tunnelgatan: first through green at T; northbound left remains red.
    # Through-green northbound is a modelling choice to accommodate Palm, not explicit testimony.
    stage(['VEH_NB_THROUGH', 'VEH_SB_THROUGH'], 20)
    stage(['VEH_NB_LEFT'], 8)
    stage(['VEH_WEST_APPROACH', 'VEH_EAST_APPROACH'], 10)
    # Dedicated all-pedestrian stage; no claim that this was the historical arrangement.
    phase(12, [g for g in groups if g.startswith('PED_')], 'Green')
    phase(12)
    offset = {'TUNNELGATAN': 0, 'APELBERGSGATAN': -8, 'KUNGSGATAN': 1}[preset]
    return {'groups': groups, 'phases': phases, 'epoch': shot_seconds + offset - 1,
            'cycle_seconds': sum(p['duration'] for p in phases)}


def apply_selected(preset=PRESET, shot_seconds=SHOT_SECONDS):
    import unreal
    editor = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    selected = editor.get_selected_level_actors()
    if len(selected) != 1 or not isinstance(selected[0], unreal.TMOPTrafficSignalController):
        raise RuntimeError('Select exactly one TMOPTrafficSignalController for the chosen intersection.')
    controller = selected[0]
    program = make_program(preset, shot_seconds)
    # Preserve intersection identity: existing heads stay linked to the controller.
    if str(controller.get_editor_property('intersection_id')) in ('None', ''):
        raise RuntimeError('Set IntersectionId before applying the preset.')
    groups = []
    for name in program['groups']:
        g = unreal.TMOPTrafficSignalGroup()
        g.set_editor_property('group_id', name)
        g.set_editor_property('pedestrian', name.startswith('PED_'))
        groups.append(g)
    enums = {'Red': unreal.TMOPTrafficSignalState.RED,
             'RedYellow': unreal.TMOPTrafficSignalState.RED_YELLOW,
             'Green': unreal.TMOPTrafficSignalState.GREEN,
             'GreenYellow': unreal.TMOPTrafficSignalState.GREEN_YELLOW}
    phases = []
    for source in program['phases']:
        p = unreal.TMOPTrafficSignalPhase()
        p.set_editor_property('duration_seconds', source['duration'])
        states = []
        for name, state in source['states'].items():
            s = unreal.TMOPSignalGroupState()
            s.set_editor_property('signal_group_id', name)
            s.set_editor_property('state', enums[state])
            states.append(s)
        p.set_editor_property('group_states', states)
        phases.append(p)
    # Explicit conflicts between different protected stages.
    stage_groups = [program['groups'][:2], [program['groups'][2]], program['groups'][3:5], program['groups'][5:]]
    conflicts = []
    for i, a in enumerate(stage_groups):
        for b in stage_groups[i + 1:]:
            for ga in a:
                for gb in b:
                    c = unreal.TMOPTrafficSignalConflict()
                    c.set_editor_property('group_a', ga)
                    c.set_editor_property('group_b', gb)
                    conflicts.append(c)
    with unreal.ScopedEditorTransaction('TMOP visual witness signal reconstruction'):
        controller.modify()
        for name, value in {'groups': groups, 'phases': phases, 'conflicts': conflicts,
                            'stages': [], 'program_epoch_seconds': program['epoch'],
                            'cycle_offset_seconds': 0.0, 'initial_phase_index': 0,
                            'cycle_automatically': True, 'visual_clock_only': True}.items():
            controller.set_editor_property(name, value)
        controller.set_phase(0)  # validates the program
        controller.evaluate_at_time(shot_seconds)
        controller.refresh_signal_heads()
        for actor in editor.get_all_level_actors():
            if isinstance(actor, unreal.TMOPTrafficSignalDirector):
                actor.modify()
                actor.set_editor_property('control_vehicles', False)
                actor.set_editor_property('control_pedestrians', False)
    unreal.log('TMOP visual preset applied. Assign signal heads to the documented group IDs. Map not saved.')

if __name__ == '__main__':
    apply_selected()
