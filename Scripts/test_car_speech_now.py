"""Run during Play in Editor; displays test actors' bubbles without changing timelines."""
import unreal
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
if world is None:
 raise RuntimeError('Start Play in Editor first.')
shown=0
seats={}
for vehicle in unreal.GameplayStatics.get_all_actors_of_class(world,unreal.TMOPVehicleBase):
 for seat in vehicle.get_components_by_class(unreal.TMOPVehicleSeatComponent):
  occupant=seat.get_occupant_character()
  if occupant is not None:seats[occupant.get_path_name()]=(vehicle.get_name(),str(seat.get_editor_property('seat_id')))
for actor in unreal.GameplayStatics.get_all_actors_of_class(world,unreal.TMOPHistoricalAgent):
 profile=actor.get_component_by_class(unreal.TMOPPersonProfileComponent)
 if profile is None:continue
 row=profile.get_editor_property('profile')
 eid=str(row.get_editor_property('entity_id'))
 if eid not in ('SYSTEM_TEST_DRIVER','SYSTEM_TEST_PASSENGER'):continue
 seat_info=seats.get(actor.get_path_name())
 if not seat_info:
  unreal.log_warning(eid+': NOT registered in a vehicle seat. This is not an in-car test yet.')
  continue
 unreal.log(f'{eid}: confirmed seat occupant: {seat_info}')
 bubble=actor.get_editor_property('speech_bubble')
 if bubble is None:
  unreal.log_error(eid+': SpeechBubble component is missing.')
  continue
 # Clear HiddenInGame for this diagnostic; show call controls normal visibility.
 bubble.set_hidden_in_game(False)
 duration=actor.show_automatic_speech('[MANUELLT BILTEST] '+eid+' – denna text ska synas i 20 sekunder.',None,20.0)
 location=actor.get_actor_location()
 widget=bubble.get_user_widget_object()
 unreal.log(f'{eid}: duration={duration}, widget={widget}, location={location}')
 shown+=1
if not shown:
 unreal.log_warning('No seated test people received a bubble. Check spawn and boarding; run after 23:00:15.')
else:
 unreal.log('Manual test triggered. Look at the test actors; their locations are printed above. This tests display, not scheduled triggering.')
