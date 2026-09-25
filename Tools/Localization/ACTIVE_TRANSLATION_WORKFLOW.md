# Latest dated tables and spawn-filtered translation

The repository uses MM_DD export folders. Resolve the newest file separately for
**each named table**, not by taking every file from the newest folder. The folder
names contain no year; this convention needs an explicit year policy if exports
span New Year. Supplementary *_IMPORT / *_UPDATED / delta files are not treated as
replacements for canonical table names.

The input list now uses People, HistoricalVehicles and HistoricalEvents from
09_21. Other named inputs retain their individually newest dated versions.
`--latest-dated` resolves current file dates against that explicit table list;
it does not automatically add unrelated new tables.

```text
python Tools/Localization/tmop_localization.py export --table-list Tools/Localization/current_tables.json --latest-dated --spawn-only --merge TranslationTemplates/en.catalog.json --output TranslationTemplates/en.updated.json --audit TranslationTemplates/selection_audit.json
python Tools/Localization/tmop_localization.py compile TranslationTemplates/en.updated.json --output Saved/LanguagePacks/en.json
```

`--spawn-only` filters People and HistoricalVehicles, leaving other table types
unchanged. It selects rows with bSpawnInSimulation enabled, a nonempty EntityId or
VehicleId, and an InitialPlacement or Spawn action. People actions marked
DocumentationOnly do not qualify. The audit contains selected counts and every
excluded row ID. Historical reference-only people and cars are not translated.

This is a **candidate set**, not an observed live actor list. Vehicles can ignore
row flags through director settings or be spawned explicitly; inactive/invalid
anchors, automatic spawning toggles, time conditions, map assignments and runtime
state also affect actual spawning. The current Git exports yield 796 people and
130 vehicle candidates (816 / 133 rows have the flags enabled).

Before translating every in-game line, also resolve cross-row dependencies such
as shared MeetingDialogues owned by inactive rows but referenced by active
participants. This filter does not claim to compute that dependency closure.
For an exact scene-specific set, export IDs of spawned actors over the full
simulation window from Unreal and match those IDs to the catalogue.

The 2026-09-25 English catalogue supersedes the pilot and retains its 21 reviewed
fields. See `ENGLISH_TRANSLATION_STATUS.md` for current counts and review status.
The runtime pack contains only reviewed translations; machine drafts remain in
`TranslationTemplates/en.catalog.json` for review.
The integration assumes the conventional People / HistoricalVehicles asset and
row names match the exports. Original IDs, names and source data are unchanged.

The table list now also includes intro cards, item labels and appearance assets.
Appearance assets currently contribute no prose fields. Only person Timeline
Notes are added to the prose selection, because the inspector displays them;
other Notes fields and player-authored notes remain excluded.
