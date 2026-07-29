# TMOP Engine 0.0.96

## Observation system

- Added separate DataTable row types for sourced observations and theoretical
  observation links.
- Observation links contain their main interpolated route and alternative
  routes; no third inferred-track DataTable is required.
- Added `ATMOPObservationDirector`.
- Canonical observation times can be absolute or calculated from an existing
  major shared event.
- Observations never alter actor movement, timing or spawning.
- Runtime checks observer presence, observed-entity presence, anchor distance
  and optional line of sight.
- Added test imports for Civil 2733's Passat scene and red-jacket control.
- Added `OBSERVED_UNKNOWN`; those agents keep independent timelines and
  profiles but have no ordinary overhead name label.
- Classified `THOMAS_PILTZ_MED_WALKIE_TALKIE` as `OBSERVED_UNKNOWN` and
  documented Mauno Luukas's later identification claim without treating it as
  established identity in the canonical observation.
- Observed unknown historical vehicles use the same category and label rule.
- Added Mauno Luukas and Kicki J as a documented side-by-side group with Mauno
  as leader and a corrected shared route through the crime-scene visit.
- Added separate sourced observations for the WT man at Adolf Fredriks
  kyrkogata 13 and the unknown man running from the entrance at 5-7.
- Added separate Anneli Korhonen, Margareta Storhök and Mårten Palme
  observations of two anonymous Grand figures, plus a non-canonical
  `ProbableSamePerson` link.
- Corrected Mårten Palme's post-film route to visit Kulturcirkeln before
  returning north past Sandins.
- Includes the 0.0.95 overhead-name size, position and category colors.
