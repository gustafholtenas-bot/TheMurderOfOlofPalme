# TMOP Engine 0.0.71

## Runtime group timeline actions

People timelines now support:

- `CreateGroup`
- `JoinGroup`
- `LeaveGroup`
- `SplitGroup`
- `DissolveGroup`
- `SetGroupLeader`

Every action uses the existing timeline timing modes, including absolute time,
shared historical events and relative-to-previous-entry timing.

Followers now ignore only ordinary `MoveToAnchor` entries while a runtime group
leader controls their route. Their seating, vehicles and group lifecycle actions
still execute. Catch-up evaluates each follower's historical position, and runtime
membership is used after a person leaves or joins a group.

## Anders Björkman

The supplied People and Groups tables create Blender collection `Sällskap 1` as
`GROUP_ANDERS_BJORKMAN_SALLSKAP`, led by
`CLAES_NYBERG_BOFORS_ANSTALLD`.

`ANDERS_BJORKMAN` executes `LeaveGroup` at 23:18:25 after the group reaches
`DekorimaVaruingang`. From that point his own timeline continues independently
toward the murder scene.

## Installation

1. Close Unreal Editor and Visual Studio.
2. Replace the project's complete `Plugins/TMOPEngine` folder with this one.
3. Delete only the project's `Binaries`, `Intermediate` and `.vs` folders.
4. Right-click the `.uproject` and generate Visual Studio project files.
5. Build `Development Editor | Win64`.
6. Open the project and import the supplied People JSON into `DT_TMOP_People`
   and the Groups JSON into `DT_TMOP_Groups`.

This package contains source code only; it contains no precompiled DLL or EXE.
