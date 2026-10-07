# PBA Northern System – v135 / batch132

Granskat 2 oktober 2026. **252 nya planerade flygrörelser**, totalt **9 823**. Tillägget ger rörelser på **29 riktade sträckor, varav 24 är nya i databasen**. Alla tillägg är inrikes i USA och ligger utanför det prioriterade Centralamerika/Karibien-urvalet.

**144 nya granskade scheman**, varav 135 ger rörelser i spelfönstret och nio har noll utfall. **Fyra nya flygplatser**: Nantucket (ACK), Martha’s Vineyard (MVY), Provincetown (PVC) och New Bedford (EWB). PBA har nu **637 rörelser från 543 scheman i tre utgåvor**: Northern januari12 samt tidigare Southern januari15 och mars1.

Alla **9 571 äldre rörelseobjekt och 5 847 äldre scheman bevaras oförändrade**. Äldre flygplatser, länder och källposter samt andra operatörer är också oförändrade. PBA:s metadata får endast en ny källreferens och utvidgad beskrivning av publikationsunderlaget. Inga tidigare tidtabellsklockslag rättas. Äldre forskningsrapporter, rättelser och reservationer bevaras byte för byte.

## Nya rörelser per sträcka

| Riktad sträcka | Nya rörelser |
|---|---:|
| ACK–EWB | 7 |
| ACK–HYA | 20 |
| ACK–LGA | 2 |
| ACK–MVY | 3 |
| BOS–BTV | 16 |
| BOS–EWB | 6 |
| BOS–HYA | 19 |
| BOS–MVY | 4 |
| BOS–PVC | 9 |
| BTV–BOS | 17 |
| EWB–ACK | 8 |
| EWB–BOS | 6 |
| EWB–LGA | 7 |
| EWB–MVY | 9 |
| HYA–ACK | 17 |
| HYA–BOS | 21 |
| HYA–LGA | 8 |
| HYA–MVY | 4 |
| HYA–PVC | 6 |
| LGA–EWB | 8 |
| LGA–HYA | 9 |
| LGA–MVY | 6 |
| MVY–ACK | 5 |
| MVY–BOS | 3 |
| MVY–EWB | 9 |
| MVY–HYA | 4 |
| MVY–LGA | 4 |
| PVC–BOS | 9 |
| PVC–HYA | 6 |

BOS = Boston, BTV = Burlington, HYA = Hyannis, LGA = New York LaGuardia. Övriga fyra koder anges ovan. En rörelse är ett daterat fysiskt flygben. BOS–BTV, BTV–BOS, BOS–HYA, HYA–LGA och LGA–HYA fanns redan som sträckor; PBA-avgångarna är nya.

## Original och avgränsning

[PBA Northern System, giltig från 12 januari 1986](https://flypba.com/timetable/n-1986-01-12/), original ur **Gordon K. Werners samling på [flyPBA.com](https://flypba.com/timetables/)**. Tryckt datum på omslaget, båda inlagorna och kartor har granskats. Arkivindex visar nästa Northern-utgåva den 24 april. Inget tryckt slutdatum har tilldelats och ändringsblad är inte fullständigt inventerade. Importen avgränsas till projektets 27 februari–1 mars.

Fem nedladdade bildfiler, inklusive omslagsvarianter, dokumenteras med URL, byteantal och SHA-256 i `caribbean_source_evidence_batch132.json`. De två inlagorna är `N-86-01-12-inside-1.jpg` och `N-86-01-12-inside-2.jpg`. Originalskanningarna distribueras inte.

Ett flygnummer utan stopp-/anslutningsanmärkning tolkas som nonstop enligt tabellstrukturen. Genomgående resor med ett eller flera stopp och anslutningar med flera nummer räknas inte som extra direktflyg. Ingen självständig uttrycklig nonstop- eller lokaltidslegend har återfunnits; lokaltid och tom daganmärkning/dagligen följer tabellkonventionen. Noten om möjliga ändringar under helgperioder har inte omvandlats till påhittade undantagsdagar.

Flygnummer återges numeriskt som tryckta. PBA-attributionen följer publikationen. Easterns bonusreklam gör inte dessa till Eastern-flyg. Omslags- och inlageillustrationer med flygplansregistreringar belägger inte en viss avgångs utrustning. Flygplanstyp, individflygplan och eventuell faktisk underoperatör är inte fastställda. Allt nytt har status `scheduled`, inte dokumenterat genomfört.

## Två rader hålls utanför

Originalets flyg **789** visar samtidigt:

| Ben | Lokala tider | Originalposition |
|---|---|---|
| Boston → Hyannis | 17:00–17:40 | Inlaga 1, kolumn 1 |
| Hyannis → Nantucket | 17:30–18:10 | Inlaga 1, kolumn 3 |

Vidareavgången ligger **tio minuter före ankomsten**. Den genomgående BOS–ACK-raden 17:00–18:10 löser inte vilket delklockslag som är fel. **Båda delraderna lämnas utanför schemakatalogen** tills en daterad rättelse eller annan tillräcklig källa finns. Ingen tid ändras exempelvis till 17:50. Se `caribbean_withheld_batch132.json`.

## Dagar, tidszon och fönster

Fönstret är **27 februari 1986 kl. 22:21:30 UTC till 1 mars kl. 22:21:30 UTC**. Intervallöverlapp avgör om flyget tas med. Alla åtta berörda flygplatser använder **UTC−5** på dessa datum, verifierat mot `America/New_York`; gränserna motsvarar **17:21:30 lokal tid**.

Nya rörelser efter lokal avgångsdag: **torsdag 34, fredag 131, lördag 87**. Samma januariutgåva gäller för alla tre projektdata; sydsystemets separata marsutgåva kopieras inte till Northern System.

- ACK–HYA **816**, 17:00–17:20, slutar före torsdagens startgräns: fredag/lördag ingår.
- BOS–MVY **887**, 16:45–17:25; BTV–BOS **761**, 17:00–17:59; LGA–HYA **710**, 16:29–17:45, överlappar båda gränsdagarnas fönster och ger tre rörelser var.
- BOS–PVC **917** och ACK–EWB **777** avgår 17:25. De ger torsdag/fredag, inte lördag efter slutgränsen.
- **792 ACK–HYA** går enligt delraden utom tisdag/onsdag/lördag; **HYA–BOS** är fredag endast. Torsdagens första ben behålls utan att hitta på en fortsatt Boston-avgång.
- **777 ACK–EWB** är daglig; **EWB–LGA** undantar lördag. **778 LGA–EWB** undantar lördag; **EWB–MVY och MVY–ACK** är fredag endast. Skillnaderna bevaras.
- **917 BOS–PVC** är daglig; **PVC–HYA** undantar fredag. På samma sätt bevaras skillnader för 813 och 791 mellan delbenen.

Nio måndags- eller söndagsrader ger noll utfall i torsdag–lördag-fönstret. De behålls som granskade scheman, med förklaring per ID i reservationsfilen. Samtliga **144 flygtider**, även nollutfallen, och **252 UTC-intervall** är kontrollerade. Flygtiderna är 20–81 minuter.

## Flygplatsmarkörer

De fyra nya koderna och destinationerna finns i den samtida PBA-tidtabellen/kartorna. Identitet, koordinater och äldre aktiveringsmetadata har jämförts med FAA-data återgivna av AirNav:

| Kod | Flygplats | Latitud | Longitud | Koordinatkälla |
|---|---|---:|---:|---|
| ACK | Nantucket Memorial | 41.2532992 | -70.0605114 | [AirNav](https://www.airnav.com/airport/ACK) |
| MVY | Martha’s Vineyard | 41.3934175 | -70.6138744 | [AirNav](https://www.airnav.com/airport/MVY) |
| PVC | Provincetown Municipal | 42.0722778 | -70.2207222 | [AirNav](https://www.airnav.com/airport/PVC) |
| EWB | New Bedford Regional | 41.6765661 | -70.9578361 | [AirNav](https://www.airnav.com/airport/EWB) |

Dessa är **nutida uppskattade referenspunkter för samma flygfält**, inte rekonstruerade gate-, ban- eller flygplanspositioner från 1986. Aktiveringsmetadata används endast som stöd för identitetsmatchning före 1986, inte som exakta historiska öppningsdatum. Se `airport_location_review_batch132.json`.

## Kontroll av benföljder

**95 daterade följder av angränsande ben med samma flygnummer**, fördelade på **54 klockmönster/53 sträckmönster**, har granskats mot originaltiderna. Alla är inom Northern System. De flesta markuppehåll är tio minuter; **777 i EWB är 20, 889 i MVY fem och 978 i BOS 25 minuter**. Detta kontrollerar tidernas inbördes ordning, inte flygplansidentitet eller faktiska anslutningar.

| Nummer | Benföljd | Ankomst / nästa avgång | Minuter |
|---|---|---|---:|
| 707 | HYA → MVY → LGA | 11:10 / 11:20 | 10 |
| 708 | LGA → MVY → HYA | 14:10 / 14:20 | 10 |
| 771 | EWB → ACK → LGA | 06:55 / 07:05 | 10 |
| 772 | LGA → EWB → ACK | 09:50 / 10:00 | 10 |
| 773 | ACK → EWB → LGA | 10:15 / 10:25 | 10 |
| 774 | LGA → EWB → ACK | 12:45 / 12:55 | 10 |
| 775 | ACK → EWB → LGA | 14:00 / 14:10 | 10 |
| 776 | LGA → EWB → ACK | 16:35 / 16:45 | 10 |
| 777 | ACK → EWB → LGA | 17:50 / 18:10 | 20 |
| 778 | EWB → MVY → ACK | 21:20 / 21:30 | 10 |
| 778 | LGA → EWB → MVY | 20:50 / 21:00 | 10 |
| 784 | HYA → PVC → BOS | 10:05 / 10:15 | 10 |
| 785 | BOS → HYA → ACK | 11:40 / 11:50 | 10 |
| 786 | ACK → HYA → BOS | 12:50 / 13:00 | 10 |
| 787 | BOS → HYA → ACK | 14:40 / 14:50 | 10 |
| 788 | ACK → HYA → BOS | 15:50 / 16:00 | 10 |
| 790 | ACK → HYA → BOS | 18:50 / 19:00 | 10 |
| 791 | BOS → HYA → ACK | 20:40 / 20:50 | 10 |
| 792 | ACK → HYA → BOS | 21:50 / 22:00 | 10 |
| 810 | ACK → HYA → BOS | 08:20 / 08:30 | 10 |
| 811 | BOS → HYA → ACK | 10:10 / 10:20 | 10 |
| 812 | ACK → HYA → BOS | 11:20 / 11:30 | 10 |
| 813 | BOS → HYA → ACK | 13:10 / 13:20 | 10 |
| 814 | ACK → HYA → BOS | 14:20 / 14:30 | 10 |
| 815 | BOS → HYA → ACK | 16:10 / 16:20 | 10 |
| 816 | ACK → HYA → BOS | 17:20 / 17:30 | 10 |
| 817 | BOS → HYA → ACK | 19:10 / 19:20 | 10 |
| 818 | ACK → HYA → BOS | 20:20 / 20:30 | 10 |
| 880 | EWB → MVY → BOS | 06:45 / 06:55 | 10 |
| 881 | BOS → EWB → MVY | 08:35 / 08:45 | 10 |
| 881 | EWB → MVY → ACK | 09:05 / 09:15 | 10 |
| 882 | ACK → MVY → EWB | 11:00 / 11:10 | 10 |
| 882 | MVY → EWB → BOS | 11:30 / 11:40 | 10 |
| 885 | BOS → EWB → MVY | 13:10 / 13:20 | 10 |
| 886 | MVY → EWB → BOS | 14:15 / 14:25 | 10 |
| 888 | MVY → EWB → BOS | 17:55 / 18:05 | 10 |
| 889 | BOS → EWB → MVY | 19:20 / 19:30 | 10 |
| 889 | BOS → EWB → MVY | 20:00 / 20:10 | 10 |
| 889 | EWB → MVY → BOS | 19:50 / 19:55 | 5 |
| 891 | BOS → MVY → EWB | 21:35 / 21:45 | 10 |
| 908 | HYA → PVC → BOS | 06:50 / 07:00 | 10 |
| 909 | BOS → PVC → HYA | 09:55 / 10:05 | 10 |
| 912 | HYA → PVC → BOS | 13:20 / 13:30 | 10 |
| 913 | BOS → PVC → HYA | 13:35 / 13:45 | 10 |
| 917 | BOS → PVC → HYA | 17:55 / 18:05 | 10 |
| 919 | BOS → PVC → HYA | 19:55 / 20:05 | 10 |
| 978 | HYA → BOS → BTV | 06:35 / 07:00 | 25 |
| 991 | HYA → MVY → LGA | 06:20 / 06:30 | 10 |
| 994 | LGA → MVY → HYA | 12:10 / 12:20 | 10 |
| 995 | ACK → MVY → EWB | 15:20 / 15:30 | 10 |
| 995 | HYA → ACK → MVY | 14:50 / 15:00 | 10 |
| 995 | MVY → EWB → LGA | 15:50 / 16:00 | 10 |
| 996 | LGA → MVY → ACK | 18:40 / 18:50 | 10 |
| 997 | ACK → HYA → LGA | 20:00 / 20:10 | 10 |

## Transkriberade scheman

Kolumner räknas från vänster i respektive inlaga. Tider är lokala. D = tom anmärkning/dagligen; X följt av siffror = undantagna dagar; ensam siffra = endast den dagen. 1=måndag, 2=tisdag, 3=onsdag, 5=fredag, 6=lördag, 7=söndag. Rörelseantalet gäller projektfönstret.

| Flygnummer | Sträcka | Lokala tider | Dagar | Rörelser | Inlaga/kolumn |
|---|---|---|---|---:|---|
| 978 | BOS → BTV | 07:00–07:59 | X7 | 2 | 1/1 |
| 756 | BOS → BTV | 08:25–09:30 | X7 | 2 | 1/1 |
| 980 | BOS → BTV | 09:55–10:59 | X7 | 2 | 1/1 |
| 758 | BOS → BTV | 11:30–12:35 | D | 2 | 1/1 |
| 982 | BOS → BTV | 14:15–15:30 | D | 2 | 1/1 |
| 760 | BOS → BTV | 15:20–16:30 | D | 2 | 1/1 |
| 762 | BOS → BTV | 18:25–19:35 | D | 2 | 1/1 |
| 764 | BOS → BTV | 22:00–22:59 | X6 | 2 | 1/1 |
| 783 | BOS → HYA | 08:00–08:40 | X7 | 2 | 1/1 |
| 811 | BOS → HYA | 09:30–10:10 | D | 2 | 1/1 |
| 785 | BOS → HYA | 11:00–11:40 | D | 2 | 1/1 |
| 813 | BOS → HYA | 12:30–13:10 | D | 2 | 1/1 |
| 787 | BOS → HYA | 14:00–14:40 | D | 2 | 1/1 |
| 815 | BOS → HYA | 15:30–16:10 | D | 2 | 1/1 |
| 817 | BOS → HYA | 18:30–19:10 | D | 2 | 1/1 |
| 791 | BOS → HYA | 20:00–20:40 | D | 2 | 1/1 |
| 819 | BOS → HYA | 21:30–22:10 | X236 | 2 | 1/1 |
| 793 | BOS → HYA | 22:45–23:25 | 5 | 1 | 1/1 |
| 881 | BOS → MVY | 08:25–09:05 | 1 | 0 | 1/1 |
| 887 | BOS → MVY | 16:45–17:25 | D | 3 | 1/1 |
| 891 | BOS → MVY | 20:55–21:35 | 5 | 1 | 1/1 |
| 881 | BOS → EWB | 08:05–08:35 | X17 | 2 | 1/2 |
| 885 | BOS → EWB | 12:40–13:10 | D | 2 | 1/2 |
| 889 | BOS → EWB | 18:50–19:20 | 5 | 1 | 1/2 |
| 889 | BOS → EWB | 19:30–20:00 | X56 | 1 | 1/2 |
| 909 | BOS → PVC | 09:25–09:55 | X7 | 2 | 1/2 |
| 913 | BOS → PVC | 13:05–13:35 | X7 | 2 | 1/2 |
| 915 | BOS → PVC | 15:25–15:55 | D | 2 | 1/2 |
| 917 | BOS → PVC | 17:25–17:55 | D | 2 | 1/2 |
| 919 | BOS → PVC | 19:25–19:55 | 5 | 1 | 1/2 |
| 755 | BTV → BOS | 07:00–07:50 | X7 | 2 | 1/2 |
| 979 | BTV → BOS | 08:30–09:30 | X7 | 2 | 1/2 |
| 757 | BTV → BOS | 09:55–10:59 | D | 2 | 1/2 |
| 981 | BTV → BOS | 11:25–12:35 | X7 | 2 | 1/2 |
| 759 | BTV → BOS | 13:00–14:10 | D | 2 | 1/2 |
| 983 | BTV → BOS | 15:45–16:55 | D | 2 | 1/2 |
| 761 | BTV → BOS | 17:00–17:59 | D | 3 | 1/2 |
| 763 | BTV → BOS | 20:00–20:59 | X6 | 2 | 1/2 |
| 978 | HYA → BOS | 06:00–06:35 | X7 | 2 | 1/3 |
| 782 | HYA → BOS | 07:00–07:35 | X7 | 2 | 1/3 |
| 810 | HYA → BOS | 08:30–09:10 | D | 2 | 1/3 |
| 812 | HYA → BOS | 11:30–12:10 | D | 2 | 1/3 |
| 786 | HYA → BOS | 13:00–13:35 | D | 2 | 1/3 |
| 814 | HYA → BOS | 14:30–15:10 | D | 2 | 1/3 |
| 788 | HYA → BOS | 16:00–16:35 | D | 2 | 1/3 |
| 816 | HYA → BOS | 17:30–18:10 | D | 2 | 1/3 |
| 790 | HYA → BOS | 19:00–19:35 | D | 2 | 1/3 |
| 818 | HYA → BOS | 20:30–21:10 | X236 | 2 | 1/3 |
| 792 | HYA → BOS | 22:00–22:35 | 5 | 1 | 1/3 |
| 991 | HYA → MVY | 06:00–06:20 | X7 | 2 | 1/3 |
| 707 | HYA → MVY | 10:50–11:10 | D | 2 | 1/3 |
| 781 | HYA → ACK | 06:00–06:20 | 1 | 0 | 1/3 |
| 809 | HYA → ACK | 07:20–07:40 | X7 | 2 | 1/3 |
| 811 | HYA → ACK | 10:20–10:40 | D | 2 | 1/3 |
| 785 | HYA → ACK | 11:50–12:10 | D | 2 | 1/3 |
| 813 | HYA → ACK | 13:20–13:40 | X23 | 2 | 1/3 |
| 995 | HYA → ACK | 14:25–14:50 | X6 | 1 | 1/3 |
| 787 | HYA → ACK | 14:50–15:10 | D | 2 | 1/3 |
| 815 | HYA → ACK | 16:20–16:40 | D | 2 | 1/3 |
| 817 | HYA → ACK | 19:20–19:40 | D | 2 | 1/3 |
| 791 | HYA → ACK | 20:50–21:10 | X236 | 2 | 1/3 |
| 993 | HYA → LGA | 09:25–10:45 | D | 2 | 1/4 |
| 709 | HYA → LGA | 15:00–16:15 | D | 2 | 1/4 |
| 711 | HYA → LGA | 18:00–19:20 | X6 | 2 | 1/4 |
| 997 | HYA → LGA | 20:10–21:30 | X26 | 2 | 1/4 |
| 908 | HYA → PVC | 06:30–06:50 | X7 | 2 | 1/4 |
| 784 | HYA → PVC | 09:45–10:05 | D | 2 | 1/4 |
| 912 | HYA → PVC | 13:00–13:20 | D | 2 | 1/4 |
| 880 | MVY → BOS | 06:55–07:35 | X17 | 2 | 1/4 |
| 880 | MVY → BOS | 07:30–08:10 | 1 | 0 | 1/4 |
| 882 | MVY → BOS | 11:10–11:50 | 7 | 0 | 1/4 |
| 889 | MVY → BOS | 19:55–20:35 | 5 | 1 | 1/4 |
| 994 | MVY → HYA | 12:20–12:40 | D | 2 | 1/4 |
| 708 | MVY → HYA | 14:20–14:40 | D | 2 | 1/4 |
| 881 | MVY → ACK | 09:15–09:35 | X7 | 2 | 1/4 |
| 996 | MVY → ACK | 18:50–19:10 | X6 | 2 | 1/4 |
| 778 | MVY → ACK | 21:30–21:50 | 5 | 1 | 1/4 |
| 882 | MVY → EWB | 11:10–11:30 | X7 | 2 | 1/4 |
| 886 | MVY → EWB | 13:55–14:15 | D | 2 | 1/4 |
| 995 | MVY → EWB | 15:30–15:50 | X6 | 1 | 1/4 |
| 888 | MVY → EWB | 17:35–17:55 | X6 | 2 | 1/4 |
| 890 | MVY → EWB | 20:40–21:00 | X56 | 1 | 1/4 |
| 891 | MVY → EWB | 21:45–22:05 | 5 | 1 | 1/4 |
| 991 | MVY → LGA | 06:30–07:40 | X7 | 2 | 1/4 |
| 707 | MVY → LGA | 11:20–12:30 | D | 2 | 1/4 |
| 782 | ACK → HYA | 06:30–06:50 | 1 | 0 | 2/1 |
| 810 | ACK → HYA | 08:00–08:20 | X7 | 2 | 2/1 |
| 812 | ACK → HYA | 11:00–11:20 | D | 2 | 2/1 |
| 786 | ACK → HYA | 12:30–12:50 | D | 2 | 2/1 |
| 814 | ACK → HYA | 14:00–14:20 | X23 | 2 | 2/1 |
| 788 | ACK → HYA | 15:30–15:50 | D | 2 | 2/1 |
| 816 | ACK → HYA | 17:00–17:20 | D | 2 | 2/1 |
| 790 | ACK → HYA | 18:30–18:50 | D | 2 | 2/1 |
| 997 | ACK → HYA | 19:40–20:00 | X26 | 2 | 2/1 |
| 818 | ACK → HYA | 20:00–20:20 | D | 2 | 2/1 |
| 792 | ACK → HYA | 21:30–21:50 | X236 | 2 | 2/1 |
| 880 | ACK → MVY | 07:00–07:20 | 1 | 0 | 2/1 |
| 882 | ACK → MVY | 10:40–11:00 | X7 | 2 | 2/1 |
| 995 | ACK → MVY | 15:00–15:20 | X6 | 1 | 2/1 |
| 773 | ACK → EWB | 09:50–10:15 | D | 2 | 2/1 |
| 775 | ACK → EWB | 13:35–14:00 | D | 2 | 2/1 |
| 777 | ACK → EWB | 17:25–17:50 | D | 2 | 2/1 |
| 779 | ACK → EWB | 22:00–22:25 | 5 | 1 | 2/1 |
| 771 | ACK → LGA | 07:05–08:15 | X7 | 2 | 2/2 |
| 882 | EWB → BOS | 11:40–12:10 | X7 | 2 | 2/2 |
| 886 | EWB → BOS | 14:25–14:55 | D | 2 | 2/2 |
| 888 | EWB → BOS | 18:05–18:35 | X6 | 2 | 2/2 |
| 880 | EWB → MVY | 06:25–06:45 | X17 | 2 | 2/2 |
| 881 | EWB → MVY | 08:45–09:05 | X17 | 2 | 2/2 |
| 882 | EWB → MVY | 10:40–11:00 | 7 | 0 | 2/2 |
| 885 | EWB → MVY | 13:20–13:40 | D | 2 | 2/2 |
| 889 | EWB → MVY | 19:30–19:50 | 5 | 1 | 2/2 |
| 889 | EWB → MVY | 20:10–20:30 | X56 | 1 | 2/2 |
| 778 | EWB → MVY | 21:00–21:20 | 5 | 1 | 2/2 |
| 880 | EWB → ACK | 06:25–06:50 | 1 | 0 | 2/2 |
| 771 | EWB → ACK | 06:35–06:55 | X7 | 2 | 2/2 |
| 772 | EWB → ACK | 09:10–09:35 | 7 | 0 | 2/2 |
| 772 | EWB → ACK | 10:00–10:25 | X7 | 2 | 2/2 |
| 774 | EWB → ACK | 12:55–13:20 | D | 2 | 2/2 |
| 776 | EWB → ACK | 16:45–17:10 | D | 2 | 2/2 |
| 773 | EWB → LGA | 10:25–11:30 | D | 2 | 2/3 |
| 775 | EWB → LGA | 14:10–15:15 | D | 2 | 2/3 |
| 995 | EWB → LGA | 16:00–16:59 | X6 | 1 | 2/3 |
| 777 | EWB → LGA | 18:10–19:15 | X6 | 2 | 2/3 |
| 992 | LGA → HYA | 07:55–09:10 | X7 | 2 | 2/3 |
| 710 | LGA → HYA | 16:29–17:45 | D | 3 | 2/3 |
| 712 | LGA → HYA | 19:35–20:50 | X6 | 2 | 2/3 |
| 998 | LGA → HYA | 21:40–22:55 | X26 | 2 | 2/3 |
| 994 | LGA → MVY | 10:59–12:10 | D | 2 | 2/3 |
| 708 | LGA → MVY | 13:00–14:10 | D | 2 | 2/3 |
| 996 | LGA → MVY | 17:30–18:40 | X6 | 2 | 2/3 |
| 772 | LGA → EWB | 08:29–09:50 | X7 | 2 | 2/3 |
| 774 | LGA → EWB | 11:45–12:45 | D | 2 | 2/3 |
| 776 | LGA → EWB | 15:29–16:35 | D | 2 | 2/3 |
| 778 | LGA → EWB | 19:45–20:50 | X6 | 2 | 2/3 |
| 908 | PVC → BOS | 07:00–07:30 | X7 | 2 | 2/4 |
| 784 | PVC → BOS | 10:15–10:35 | D | 2 | 2/4 |
| 912 | PVC → BOS | 13:30–14:00 | D | 2 | 2/4 |
| 916 | PVC → BOS | 16:00–16:30 | D | 2 | 2/4 |
| 918 | PVC → BOS | 18:05–18:35 | 5 | 1 | 2/4 |
| 909 | PVC → HYA | 10:05–10:25 | X7 | 2 | 2/4 |
| 913 | PVC → HYA | 13:45–14:05 | X7 | 2 | 2/4 |
| 917 | PVC → HYA | 18:05–18:25 | X5 | 1 | 2/4 |
| 919 | PVC → HYA | 20:05–20:25 | 5 | 1 | 2/4 |

## Avgränsad radräkning

Visuell genomgång av båda inlagorna ger **146 enkelnummer-rader utan stopp-/anslutningsanmärkning**: **144 importerade och två reserverade 789-rader**. Det är en avslutad radräkning i denna källa, inte en komplett inventering av PBA:s trafik.

| Avgångsort | Synliga rader | Importerade | Reserverade |
|---|---:|---:|---:|
| BOS | 31 | 30 | 1 |
| BTV | 8 | 8 | 0 |
| HYA | 31 | 30 | 1 |
| MVY | 17 | 17 | 0 |
| ACK | 19 | 19 | 0 |
| EWB | 20 | 20 | 0 |
| LGA | 11 | 11 | 0 |
| PVC | 9 | 9 | 0 |
| **Totalt** | **146** | **144** | **2** |

Sydsystemets tidigare 399 scheman lämnas oförändrade. Äldre luckor för självständiga mellanlandningstider, bland annat RSW–SRQ 1208 och januari APF–RSW, kvarstår. Ändringsblad, inställda avgångar, inhyrningar och faktisk drift är fortfarande ofullständigt undersökta.

## Nicaragua, St. Thomas och regional sökning

**Ingen ny regional rörelse importeras i v135.** Samma 31 forskningsområden har **532 rörelser, 158 riktade par, 38 flygplatser och 64 inrikesrörelser**. **St. Thomas 14, Tortola 8, Nicaragua 0** är oförändrade. Angränsande Mexiko har 20 och Mellanösternurvalet 72. Noll betyder datalucka.

Denna omgång sökte vidare efter LACSA:s januari-/marsinlagor, Cubana/Aeroflot kring Managua–Havanna och Caribbean Express vid St. Thomas. Index, omslag och senare utgåvor gav inga importerbara nya klockrader för måldatumen. Valda regionala Pan Am-poster fanns redan i katalogen. Air Frances F27-rader har fortfarande olöst operatörsattribution; äldre OAG-/rapportmaterial löser inte den för februari 1986. Dessa frågor hålls öppna och inga militär- eller specialflygrörelser har lagts till.

Detaljer och granskade länkar finns i `regional_source_search_batch132.json`. Nästa prioriterade sökning följer `central_america_caribbean_queue_batch132.json`: Nicaragua, St. Thomas, LACSA/Caribbean Express-inlagor, TACA/Aviateca/TAN SAHSA, American/Eastern/BWIA samt sjöflygets ankomsttider och hamnidentiteter. Tidigare LIAT-, Challenge-, Arrow-, Airways International- och specialflygsfrågor kvarstår.

## Totalt och verifiering

**9 823 rörelser: 9 822 planerade och en tidigare bekräftad. 5 991 katalogscheman, varav 5 970 granskade. 392 flygplatser/platser, 121 länder/territorier och 1 873 riktade par. 111 operatörsposter: 68 med rörelser, 43 utan. 94 källposter och 36 bidragande tidtabellsutgåvor.** Ingen verifierad global bolagsinventering eller färdigprocent finns.

**20 befintliga Python-tester och tre generatorkontroller passerar.** Alla nya UTC-intervall matchar separat beräkning med fast UTC−5, manuellt kontrollerade flygtider och datum. Samtliga 637 PBA-rörelser har kontrollerats för dubbletter och samtidig överlappning av samma nummer; inga återstår bland importerade poster. Kontrollerna använder samma originalmaterial, inte en andra oberoende källa eller granskare.

Alla tidigare rörelse- och schemaobjekt är oförändrade. Endast tillåtna aktuella datafiler, README och index uppdateras. ZIP-kontroll och filhashar dokumenteras i manifestet. **Unreal Editor och spelet har inte körts.**

## Installation

Stäng Unreal och slå samman ZIP-filens `Plugins/` och `Tools/` med projektroten bredvid `.uproject`. **PaintAirplane-rättelsen f23b73a måste redan vara kompilerad.** Paketet är kumulativt och innehåller endast data och befintliga forskningsverktyg; ingen ny C++-kod.
