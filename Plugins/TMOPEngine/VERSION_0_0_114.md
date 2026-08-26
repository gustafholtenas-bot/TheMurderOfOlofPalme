# TMOP Engine 0.0.114

- Fixes C2445 in `TMOPPlayerCharacter.cpp` when selecting the forced native
  target-information widget class.
- Replaces the ambiguous conditional expression with an explicit
  `TSubclassOf<UTMOPInteractionPromptWidget>` assignment.
- Functionality is otherwise unchanged from 0.0.113.
