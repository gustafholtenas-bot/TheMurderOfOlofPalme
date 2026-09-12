# Fix för C4150 i Killer Branch Runtime

Felet uppstod eftersom `TUniquePtr` försökte radera den framåtdeklarerade typen
`FTMOPKillerBranchRuntime` i kod där typens destruktor inte var synlig.

Fixen använder en egen deleter som deklareras i headern men implementeras i cpp-filen
efter den fullständiga definitionen av `FTMOPKillerBranchRuntime`. Runtime-typen kan
därmed fortsätta vara privat för cpp-filen och raderas alltid där den är komplett.

Filer som ändrats:

- `Source/TMOPEngine/Public/Killer/TMOPKillerBranchDirector.h`
- `Source/TMOPEngine/Private/Killer/TMOPKillerBranchDirector.cpp`

`MSB3073` med exit code 6 är följdfelet från det avbrutna Unreal-bygget och ska
försvinna när C4150-felet är borta.

Om Visual Studio fortfarande visar den gamla deklarationen efter att filerna har
ersatts: stäng Unreal Editor och Visual Studio, rensa projektets genererade
`Binaries`/`Intermediate` samt pluginens genererade `Binaries`/`Intermediate`,
generera Visual Studio-projektfilerna igen och bygg om projektet.
