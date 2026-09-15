"""Run during Play in Editor; displays test actors' bubbles without changing timelines."""
import unreal
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
if world is None:
 raise RuntimeError('Start Play in Editor first.')
shown=0
for actor in unreal.GameplayStatics.get_all_actors_of_class(world,unreal.TMOPHistoricalAgent):
 profile=actor.get_component_by_class(unreal.TMOPPersonProfileComponent)
 if profile is None:continue
 row=profile.get_editor_property('profile')
 eid=str(row.get_editor_property('entity_id'))
 if eid not in ('SYSTEM_TEST_DRIVER','SYSTEM_TEST_PASSENGER'):continue
 bubble=actor.get_editor_property('speech_bubble')
 if bubble is None:
  unreal.log_error(eid+': SpeechBubble component is missing.')
  continue
 # Clear HiddenInGame for this diagnostic; show call controls normal visibility.
 bubble.set_hidden_in_game(False)
 duration=actor.show_automatic_speech('[MANUELLT BUBBELTEST] '+eid+' – denna text ska synas i 20 sekunder.',None,20.0)
 location=actor.get_actor_location()
 widget=bubble.get_user_widget_object()
 unreal.log(f'{eid}: duration={duration}, widget={widget}, location={location}')
 shown+=1
if not shown:
 unreal.log_warning('No spawned SYSTEM_TEST_DRIVER/PASSENGER found in this Play world. Check the active People table and spawn state.')
else:
 unreal.log('Manual test triggered. Look at the test actors; their locations are printed above. This tests display, not scheduled triggering.')
