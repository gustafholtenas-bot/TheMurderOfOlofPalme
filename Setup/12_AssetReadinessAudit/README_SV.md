# Asset readiness audit

Detta steg inventerar de faktiska Unreal-assets som katalogens 64 rader pekar
på. Scriptet ändrar ingenting och kan därför köras efter varje importomgång.

## Körning

1. Kompilera projektet och kör först `Setup/05_DataTableBuilder`.
2. Aktivera **Python Editor Script Plugin**.
3. Kör i Unreal Output Log:
   `py "FULL_SÖKVÄG/tmop_audit_appearance_assets.py"`
4. Öppna rapporten som skapas under
   `Saved/TMOP/AppearanceAudit/TMOP_Appearance_Audit_DATUM_TID.csv`.

## Kontroller

- Att varje katalograd har en existerande mesh när delen inte är `NONE_*`.
- Att mesh-filen verkligen är en `SkeletalMesh`.
- Att obscured-rader har ett material och att materialreferenser fungerar.
- Att alla sex kroppar använder samma skeleton.
- Att modulära delar använder kropparnas skeleton.
- Att kropparna har de sju `TMOP_*`-morph targets som krävs.
- Att varje mesh har minst tre LOD-nivåer för folksamlingar.

`ERROR` måste lösas innan asseten används. `WARNING` betyder att systemet kan
fungera men att morphning eller prestanda inte är färdig. `INFO` betyder att
den installerade Unreal-versionens Python-API inte kunde läsa just den
egenskapen; kontrollera den då manuellt i Skeletal Mesh Editor.

Testa slutligen kropparna vid 120 och 205 cm samt extrema vikt- och
muskelvärden. Audit-scriptet kan kontrollera att morphnamnen finns, men visuell
deformation och klädpenetrering måste bedömas i Unreal.
