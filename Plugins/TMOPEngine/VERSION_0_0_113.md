# TMOP Engine 0.0.113

## Nearby target information

- Adds a native target HUD for nearby historical people, vehicles and world items.
- The object nearest the camera centre is preferred within a ten-degree cone.
- If the player is not looking directly at a valid object, the nearest visible
  object inside the character's forward 180-degree hemisphere is selected.
- Displays target name, object type and distance.
- Interaction remains stricter and only shows the E prompt at interaction range.
- The native widget is forced by default so stale Player Blueprint widget values
  cannot silently disable the target HUD.

## Recorded-call overlay

- Recorded calls no longer hide the entire overlay when a time-coded
  `SpeechSegments` entry is missing.
- The call title and known participants remain visible, together with an honest
  missing-transcription notice, while audio continues.
- Existing time-coded transcripts continue to use the typewriter display.

No anchors, historical people timelines or vehicle timelines were changed.
