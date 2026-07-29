# TMOPEngine 0.0.99 – Uppslag register

Version 0.0.99 adds the DataTable row type `TMOPUppslagRow`.

## Create the table

1. Rebuild and start Unreal Editor.
2. Create a DataTable with row structure `TMOPUppslagRow`.
3. Name the asset `DT_TMOP_Uppslag`.
4. Import `Plugins/TMOPEngine/Data/DT_TMOP_Uppslag_REGISTER.json`.

The combined register contains the supplied L/LA, DB, O and all supplied E
indexes: E, EA, EB, ED, EE, EF, EG and EH, including their lettered
subseries. Every document subcase reported by WPU's MediaWiki API is included.
The decorative `Curved-arrow-centered.png` text from copied source pages is
intentionally ignored.
The register has also been expanded from WPU's MediaWiki API with every
available L/LA subcase. `TMOP_Uppslag_L_WPU_STATUS_REPORT.json` contains an
audit summary per main case, including the exact available and missing IDs.
`TMOP_Uppslag_ED_EA_EH_EG_WPU_STATUS_REPORT.json` provides the corresponding
audit for the added E sections.
`TMOP_Uppslag_E_REMAINING_WPU_STATUS_REPORT.json` audits the remaining
E, EB, EE and EF sections.
`TMOP_Uppslag_DB_WPU_STATUS_REPORT.json` audits DB - Restaurangen.
`TMOP_Uppslag_O_WPU_STATUS_REPORT.json` audits O - Inledande dörrknackningar.

The normal DataTable editor shows the checklist booleans as checkboxes.
Use one row per canonical WPU reference. The row name and `Uppslag ID`
should normally match.

`Serie` can be used to filter related registers, for example `L` and `LA`.
`Huvuduppslag` connects document rows to their parent, for example
`L261-00-A` to `L261`.

## Checklist meaning

- `Tillgänglighet`: whether the document is released/available on WPU.
- `Uthämtat av oss`: the available document has been obtained for the project.
- `Inlagt i projektet`: all game-relevant information is represented.
- `Delvis inlagt`: some information is represented, but work remains.
- `Behöver granskas`: interpretation, identity, time, or implementation
  remains uncertain.
- `Relevant för spelet`: the document affects the current game area or
  simulation scope.

Do not check both `Inlagt i projektet` and `Delvis inlagt`.
