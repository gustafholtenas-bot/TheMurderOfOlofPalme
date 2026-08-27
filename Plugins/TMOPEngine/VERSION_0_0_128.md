# TMOPEngine 0.0.128

## Main menu and limousine intro

- Adds an editor-toggleable `TMOPMainMenuIntroDirector`.
- Uses an assigned CineCameraActor as the live main-menu background.
- Native logo menu: new game, load game, settings and quit.
- Creates a separate temporary Jan Nilsson intro limousine.
- Automatically routes from a Kungsgatan East anchor to a Grand anchor.
- Seats the player as passenger and Jan Nilsson as the visual driver.
- Supports placed cameras and vehicle-relative follow-camera shots with blends.
- Intro text and images are driven by `DT_TMOP_IntroCards`.
- Hands control to the player at Grand and starts the simulation clock.
- Existing quick-save loading is available from the main menu.
