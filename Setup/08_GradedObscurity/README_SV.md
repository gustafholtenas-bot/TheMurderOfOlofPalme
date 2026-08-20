# Graderad obscurity

Obscured-systemet har nu en flyttalsnivå mellan 0 och 1 på varje löst
appearance-del:

- 0.00: dokumenterad eller vanlig genererad del;
- 0.15: rekonstruerad uppgift;
- 0.30: infererad uppgift;
- 0.55: spekulativ uppgift;
- minst 0.65: känd beskrivning som ännu saknar matchande asset;
- 1.00: informationen är helt okänd i källorna.

Runtime-komponenten skickar nivån till materialparametern
`TMOP_ObscurityAmount`. Unknown-materialet är texturlöst och unlit och använder
en mjuk Fresnel-gradient, vilket tar bort identifierande ytdetaljer utan dyr
screen-space blur eller translucency.

`UnknownPartStyle` fungerar nu så här:

- `Obscured`: visar den detaljreducerade fallback-delen;
- `Neutral`: visar fallback-delen utan unknown-markering;
- `Hidden`: renderar inte okända delar alls.

Kända kläder använder fortfarande sina vanliga material. En okänd jacka kan
alltså vara featureless samtidigt som kända byxor och skor visas tydligt.
