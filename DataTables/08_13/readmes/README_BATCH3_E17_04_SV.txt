TMOP Batch 3 – E17-04
======================

Källa
-----
WPU: https://wpu.nu/wiki/Uppslag:E17-04

Granskade original
------------------
1. Pol-1986-03-03_E17-04_Forhor_med_Charlotte_Liljedahl.pdf
   SHA-256: af2d4b636d43c8a3cf118d213f1e66061316cef2b89a8d37fe857f6fa4759784
   10 av 10 sidor har renderats och granskats visuellt.

2. Pol-1986-03-03_E17-04_Charlotte_Liljedahl_DUPLICATE.pdf
   SHA-256: 180723aa1639dbe793ae1ca2622a13084163600eeb607ad5d8b5994274b8917b
   7 av 7 sidor motsvarar pixelmässigt de första sju sidorna i huvudfilen och har därför integrerats endast en gång.

Integrerat innehåll
-------------------
- Förhöret med Charlotte Eva Liljedal den 3 mars 1986, inklusive hennes placering till höger i baksätet i Anders Delsborns taxi.
- Charlottes iakttagelser av andra skottet, gärningsmannens vapen, klädsel, ungefärliga ålder/längd och flykt österut in på Tunnelgatan.
- Separata entiteter för den okände mannen vid Dekorima och den okände person som kort följde efter gärningsmannen. Ingen av dem har osäkert slagits samman med andra vittnen.
- Iakttagelser av Olof Palmes fall, Lisbet Palme vid skotten och senare hjärt-lungräddning utan spekulativa identiteter för hjälparna.
- Tomas Carlsson, kriminalinspektör, har lagts in enligt stavningen i originalhandlingen.
- Ann-Mari Sjöbloms utskriftsbestyrkande och Wincent Langes rekonstruktionsmaterial har källkompletterats.
- Två visuellt åtskiljbara, oidentifierade rekonstruktionsfiguranter har lagts in separat. Rekonstruktionsfotot används inte som belägg för klädsel under mordkvällen.
- Charlottes namn har korrigerats från den äldre databasstavningen ”Liledahl” till ”Charlotte Eva Liljedal”; EntityId har bevarats för kompatibilitet.
- Gruppen med Charlotte, Ann-Charlott Holmgren och Lena Schödin samt taxifordonet har källkompletterats.

Avgränsningar och granskningspunkter
-----------------------------------
- Den röda bilen på rekonstruktionskartan används endast som positionsmarkör och ger inte stöd för taxins färg eller modell.
- Charlotte anger höger körfält närmast trottoaren, medan Anders Delsborn i E17-00 anger mittfilen med hög säkerhet. Båda utsagorna bevaras som en uttrycklig källkonflikt.
- De tre passagerarnas äldre importerade urstigningstid 23:21:28 ligger före den låsta skotttiden, trots att E17-04 visar att de var kvar i taxin vid skotten. Inga exakta tider, TimingMode-värden, offsetar, ankare eller efterföljande rutter har ändrats utan ett säkert gemensamt tidsunderlag. E17-04 och moderposten E17 är därför markerade för fortsatt granskning.
- Endast Charlottes sätesplacering har korrigerats från REAR_LEFT till REAR_RIGHT i de två relevanta tidslinjeposterna.
- Förhörs-, rekonstruktions- och dokumenttider har inte omvandlats till tider under mordnatten.

Datatabeller efter integration
------------------------------
- People: 707
- Observations: 151
- Groups: 127
- HistoricalVehicles: 69
- Uppslag register: 3764
- HistoricalEvents: 67 (oförändrad)

Validering
----------
- Inga poster har tagits bort.
- Inga dubbla primär-ID:n har tillkommit.
- Exakt diffomfång har kontrollerats mot föregående E17-00-A-checkpoint.
- Alla äldre observationsposter är oförändrade.
- Endast de två uttryckligen motiverade sätesfälten har ändrats bland hårda tidslinjefält.
- Inga nya brutna referenser har skapats.
- HistoricalEvents och TMOP_Grand_IdentityHypotheses är oförändrade.
