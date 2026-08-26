# TMOP Engine 0.0.108

Uppslag coverage redesign:

- Adds `bHighPriorityForGame` to the uppslag row type.
- Displays global totals and per-section counts for added, online-not-added,
  police-only and police-only/high-priority records.
- Uses green, yellow, grey and red coverage segments respectively.
- Keeps unknown availability visually neutral and reports it separately.
- Scales each section bar linearly by its document count relative to the
  largest section, so large sections are visibly longer than small sections.
- Shows the registered section description, with a parent-section fallback
  when a child section lacks its own description.

