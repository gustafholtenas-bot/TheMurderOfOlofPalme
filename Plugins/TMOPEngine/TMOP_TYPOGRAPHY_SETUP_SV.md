# TMOP Typography Director

## Importera den detaljerade tabellen

1. Öppna `DT_TMOP_Typography` i Unreal.
2. Välj **Reimport** om tabellen redan pekar på JSON-filen, annars **Import**.
3. Använd `Plugins/TMOPEngine/Data/DT_TMOP_Typography_IMPORT.json`.
4. Kontrollera att radstrukturen är `TMOPTypographyStyleRow`.
5. Sätt tabellen i `Typography Table` på `TMOPTypographyDirector`.

Varje rad har nu fältet **Used By**, som förklarar exakt var stilen används.
Samma kompletta lista visas även i `Style Usage Reference` på directorn.

## Stilgrupper

- `MainMenu...`: startmeny och dess laddningslista.
- `Intro...`: introkort och SKIP-knapp.
- `PauseMenu...`: pausmeny, källor/uppslag och sparfiler.
- `Dialog...`, `Subtitle`: dialog, radio och undertexter.
- `Interaction...`: sikte, målnamn, hjälp och E-uppmaning.
- `QuickInventory...`: det radiella snabbinventariet.
- `Newspaper...`: tidningsläsaren.
- `AgentInfo...`: det stora personkortet.
- `Map...`: stora kartans etiketter, teckenförklaring och hjälp.
- `AgentName3D`, `VehicleName3D`: Text Render ovanför personer och fordon.

Om en ny specifik rad saknas använder koden fortfarande en kompatibel generell
reservstil (`Heading`, `Body`, `Caption` eller `MenuButton`). Det gör att gamla
tabeller fortsätter fungera medan du fyller i de nya raderna.

## Varför personnamnen försvann

3D-namnen använder `TextRenderComponent`, som inte använder samma fontformat som
Slate/UMG. En vanlig Composite Font ska därför inte automatiskt skrivas in som
3D-font. Directorn skrev dessutom tidigare över world size och kategori-färg var
0,5 sekund.

Raderna `AgentName3D` och `VehicleName3D` har nu tre separata val:

- **Override World Font**: av som standard. Aktivera bara med en legacy `UFont`
  som fungerar med Text Render.
- **Override World Size**: av som standard. När den är av behålls personens
  storleksval Liten/Medium/Stor.
- **Override World Color**: av som standard. När den är av behålls färgerna för
  Palmefamilj, polis, observerad och misstänkt.

Directorn återställer också namnets text och synlighet innan en tillåten 3D-stil
appliceras. Det gör att en aktiverad Typography Director inte längre kan dölja
namnskyltarna.

## Menyfärger

På `TMOPTypographyDirector` finns kategorin **Menu Colors**:

- `Menu Background`
- `Panel Background`
- `Button Background`
- `Main Menu Button Text`
- `Pause Menu Button Text`
- `Status Text`
- `Accent Text`
- `Intro Card Background`

Textfärgen för varje enskild textroll kan fortfarande anges i respektive rad i
typografitabellen. Paletten styr de gemensamma bakgrunderna och knapparna.

## Blueprint-texter

`Widget Name Style Overrides` används endast för Blueprint-skapade `TextBlock`.
Nyckeln är widgetens exakta namn i Designer och värdet är Style ID. Exempel:

- `ClockText` -> `HUDClock`
- `MurderCountdownText` -> `HUDCountdown`
- `ObjectiveText` -> `HUDObjective`

Detta är rätt sätt att ge projektunika Blueprint-texter egna rader utan att
alla okända texter faller tillbaka på samma `Body`-stil.
