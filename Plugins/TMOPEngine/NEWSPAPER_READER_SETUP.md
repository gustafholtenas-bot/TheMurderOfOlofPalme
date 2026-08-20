# TMOP newspaper items

The plugin contains a data-driven newspaper item and a native page reader.

## Create the seven newspaper assets

In Content Browser, create **Miscellaneous > Data Asset** and select
`TMOPNewspaperItemDefinition`. Suggested assets:

| Asset | Publication |
| --- | --- |
| `DA_Newspaper_Arbetet_19860228` | Arbetet |
| `DA_Newspaper_Aftonbladet_19860228` | Aftonbladet |
| `DA_Newspaper_DN_19860228` | DagensNyheter |
| `DA_Newspaper_Expressen_19860228` | Expressen |
| `DA_Newspaper_SvD_19860228` | SvenskaDagbladet |
| `DA_Newspaper_GP_19860228` | GoteborgsPosten |
| `DA_Newspaper_DI_19860228` | DagensIndustri |

Set `Item Id`, `Display Name`, `Publication`, `Publication Date`, and add the
scans to `Pages` in reading order. `Item Type`, stack size, equip/menu behavior,
and dropping defaults are configured automatically by the subclass.

Suggested scan names are `T_NP_<paper>_19860228_P001`, `P002`, etc. Keep scans
as separate `Texture2D` assets. Use the UI texture group, preserve sRGB, and
allow texture streaming. The newspaper stores soft references and loads only
the currently displayed page.

## Put a paper in the world

Place or spawn a `TMOPWorldItem` and assign the newspaper data asset as its item
definition. Interaction adds it to the normal inventory. Selecting it in the
quick inventory opens the reader instead of equipping it in the hand.

## Reader controls

- Previous/next buttons, Left/Right, A/D, Page Up/Page Down: change page.
- Plus/minus buttons, keyboard +/−, mouse wheel: zoom.
- Scrollbars: pan while zoomed.
- Escape or Close: return to the game.

Each newspaper can independently decide whether simulation time pauses while
it is being read. The default is enabled.
