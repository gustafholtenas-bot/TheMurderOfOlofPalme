# Tidningsgenomgång: 1–2 mars 1986

Granskad 2026-09-25. Satsen innehåller **18 PDF-filer, sammanlagt 623 PDF-sidor**. Detta är en riktad genomgång för världsöversikten kring morddygnet, inte en fullständig radläsning av 623 sidor. Alla filer har inventerats. Tillgänglig OCR har sökts tematiskt; bildskanningarna har dessutom granskats i sidöversikter. Utvalda nyhetsartiklar och de citat som importerats har lästs i förstoring. Annonser, börstabeller, sportresultat och skönlitteratur har inte transkriberats.

PDF-sidor räknas från 1. När tidningens sidnummer avviker anges båda. Samtliga använda källor är användarens skanningar. Flera artiklar bygger på nyhetsbyråmaterial; upprepning i olika tidningar räknas inte automatiskt som oberoende bekräftelse.

## Ändringar i spelets data

`world.json` har kompletterats i **27 befintliga poster** och fått **en ny händelsepost** för Brasiliens Cruzadoplan. Totalt 45 texttillägg, inklusive samma sakuppgift under flera berörda länder/konflikter. Alla tillägg finns på svenska och engelska med aktuell `en_source`, filnamn, publiceringsdatum, sida och fältkoppling i `sources`.

- `news_day`: händelser uttryckligen daterade till 28 februari.
- `news_after`: senare händelser eller tydligt märkta förhandsrapporter om kommande åtgärder.
- `body`: nyhetsläge där exakt händelsedatum inte är fastställt, samt historisk bakgrund.
- `reactions`: uttalanden efter mordet, med namn, befattning och citat-/referatmarkering.

Ledningsprofilerna avser fortsatt den 28 februari före mordet. Efterföljande uttalanden gör inte talaren till regeringschef på ett tidigare datum. Pågående konflikters datum och kartpilar har inte ändrats av nyhetsimporten.

### Händelser och bakgrund

| Poster i world.json | Tidningskälla | Infört innehåll och avgränsning |
|---|---|---|
| `fr`, `de` | Il Piccolo 1/3, s. 12 | Utfallet av toppmötet i Paris 28/2: konsultationer om eventuell användning av franska vapen på tyskt territorium. Kompletterar tidigare rapport om det då pågående mötet. |
| `dk`, `it`, `gr` | La Stampa 1/3, s. 4 | Undertecknandet av Europeiska enhetsakten i Haag 28/2. Undertecknande hålls skilt från ikraftträdande. |
| `nl`, `nato` | The Times 1/3, s. 5 | Underhusets beslut 28/2 om planerad stationering av 48 kryssningsrobotar. Överhusets behandling och utplaceringen återstod. |
| `us` | Times-News 1/3, B3 (PDF 13) | WTI-prisfallet och publiceringen 28/2 av handelsstatistik för januari. Statistikperiod och publiceringsdag skiljs åt. |
| `gb` | The Times 1/3, s. 1, 23 | Oljeprisfallet och nedgången för pundet 28/2. |
| `es` | El Diario Palentino 1/3, s. 26 | Bränsleprissänkning vid midnatt 28/2–1/3; samtidigt aviserade höjda eltariffer. Inte placerat tidigare på mordkvällen. |
| `us` | Times-News 1/3, A1 | Automatiska budgetnedskärningar enligt Gramm–Rudman från 1/3, enligt rapporten den dagen. |
| `gb` | The Times 1/3, s. 1 | Youngers parlamentssvar 28/2 om telefonavlyssning och säkerhet. Allmän varning, ingen belagd Palmeanknytning. |
| `su` | The Times 1/3, s. 1; jämför La Stampa s. 4 | Tjebrikovs offentliga påstående 28/2 om gripna spioner i statsapparaten. Namn och gripandedatum okända i rapporten; anklagelserna är inte oberoende fastställda. |
| `su`, `us` | Il Piccolo 1/3, s. 12 | Zamjatins offentliga invändningar 28/2 mot Reagans robotförslag. Förhandlingsposition, inget avtal. |
| `eg`, `conflict-egypt-mutiny` | Jerusalem Post 2/3, s. 2 | Zaki Badr svors in 28/2; separat uppföljning om tolv timmars lättnad i Kairos utegångsförbud 1/3. |
| `eg`, `us` | Jerusalem Post 2/3, s. 2 | Amerikanskt besked 28/2 att Bright Star inte planerades ställas in. Fortfarande en planerad övning. |
| `fr`, `us`, `conflict-chad` | La Stampa 1/3, s. 4 | Amerikanska transportflygplan inhyrda av Frankrike anlände med luftvärnsmateriel till Tchad 28/2. Rapport om militärt stöd, inte belägg för smuggling. |
| `za`, `gb` | The Times 1/3, s. 6 | Fortsatt diplomatisk oenighet 28/2 efter brittisk kritik av våldet i Alexandra. Kritik och svar från föregående dag skiljs åt. |
| `conflict-uganda` | The Times 1/3, s. 6 | NRA:s intagande av Lira 28/2 enligt tidningsrapporten. Ingen ny exakt frontlinje ritad. |
| `conflict-philippines-npa`, `conflict-people-power` | The Times 1/3, s. 6; jämför La Stampa s. 4 | Aquinos frigivningsorder och bankoro 28/2. Ordern innebär inte att alla fångar redan frigivits. People Power-postens befintliga konfliktperiod har inte förlängts; uppföljningen läses i detaljtexten. |
| `ni`, `conflict-nicaragua` | Amigoe 1/3, s. 1 | Förslag från åtta latinamerikanska länder om civila observatörer vid Nicaragua–Costa Rica-gränsen. Tillkännagivandets exakta dag lämnas öppen. |
| `conflict-northern-ireland` | The Times 1/3, s. 2 | Unionistisk protest planerad till måndag 3/3 mot det anglo-irländska avtalet. Inte beskriven som redan genomförd. |
| `is` | Amigoe 1/3, s. 1 | Avtal mellan regering, arbetsgivare och fack mot inflationen. Exakt avtalsdatum saknas i notisen. |
| `se`, `ir`, `iq`, `conflict-iraniraq` | Stampa Sera 1/3, PDF 33 | Pérez de Cuéllar sade att han rådgjort med Palme under veckan. Enligt hans medarbetare ingick Iran–Irakkriget bland ämnena. Exakt dag och kontaktform anges inte. |
| `news-brazil-cruzado` (ny) | The Times 1/3, s. 5 | Sarneys valutareform och prisstopp tillkännagivna 28/2. En händelsepunkt vid Brasília, inte en ny ofullständig landsprofil. |

### Nya reaktioner på Palmes död

Sex korta citatfragment och ett referat har lagts till. De italienska fragmenten nedan är **tidningens återgivning**, inte belägg för vilket språk talaren använde. Översättningarna till svenska och engelska är märkta i spelet. Inget längre sammansatt citat har konstruerats av separata fragment.

| Talare / befattning | Tryckt fragment eller referat | Placering |
|---|---|---|
| Laurent Fabius, Frankrikes premiärminister | «un grande statista ed un grande amico»; telegram till Lisbeth Palme | Frankrike |
| Brian Mulroney, Kanadas premiärminister | «Piangiamo la perdita di uno statista coraggioso» | Kanada |
| Yasuhiro Nakasone, Japans premiärminister | «uno statista di coscienza» | Sveriges avsnitt om internationella reaktioner |
| Rajiv Gandhi, Indiens premiärminister | «Io ho perso un buon amico» | Sverige |
| Javier Pérez de Cuéllar, FN:s generalsekreterare | «l’amico personale e il sincero sostenitore delle Nazioni Unite» | Sverige |
| Viktor Tjebrikov, KGB:s ordförande | «Siamo oltraggiati e scossi»; kongressens femte dag 1/3 | Sovjetunionen |
| Helmut Kohl, Västtysklands förbundskansler | Referat om Palmes bidrag till dialog mellan industri- och utvecklingsländer; inga citattecken | Västtyskland |

De första sex återges i Stampa Sera, PDF 33. Kohl återges i Amigoe, s. 1. Exakt uttalandedatum lämnas öppet utom där kongressammanträdet uttryckligen ger datum. Tidningens publiceringsdatum anges för alla. Befintliga reaktioner från Peres, Herzog, Reagan och Carlsson har behållits utan dubbletter. Japans och Indiens reaktioner ligger tills vidare under Sverige eftersom dessa länder ännu inte har egna källgranskade regeringsprofiler.

## Lässtatus för samtliga 18 filer

”Sidöversikt” betyder granskning av rubriker, layout och ämnesområden i miniatyrer; det är inte radläsning av brödtext. ”OCR-sökning” avser hela filens tillgängliga text men kan missa namn och ord. De två metoderna används för urval, inte som bevis för att en uppgift saknas.

| Fil | Sidor | Genomgång och bedömning |
|---|---:|---|
| `stampa-sera_1986-03-01.pdf` | 33 | OCR-sökning i hela filen; fördjupad läsning av förstasidan, utrikesmaterial PDF 11 samt reaktionerna PDF 28/33. PDF 28 och 33 återger delvis samma material i olika layout/edition; de räknas inte som två oberoende belägg. Tidiga motsägande morduppgifter uteslutna. |
| `The_Times_News_Idaho_Newspaper_1986_03_01.pdf` | 22 | OCR-sökning; förstasidan och ekonomi B3/PDF 13 visuellt kontrollerade. Tidiga morduppgifter osäkra. Lokala nyheter, annonser och sport inte uttömmande lästa. |
| `w5yireport1986ma00unse_2.pdf` | 10 | Förstasida och OCR-genomgång av sektionerna: amatörradio, FCC, provregler och satellit-TV. Specialisttidskrift, inte en rapport om radiokommunikation vid Palmemordet. Inget importerat. |
| `78001178-78001178-1986-03-01-ed-1(1).pdf` | 212 | Lancaster Farming. Förstasida kontrollerad och hela OCR-texten sökt tematiskt. Jordbruk, auktioner, annonser och regionalt material dominerar. Conservation Reserve Program är redan importerat från tidigare genomgång och dupliceras inte. Inte 212 sidor radlästa. |
| `AMIGOE-1986-03-01.pdf` | 10 | Förstasidans internationella notiser och reaktioner lästa; OCR-sökning i filen. Tillför Kohl, gränsobservatörsförslag och Islands ekonomiska avtal. |
| `ElDiarioPalentino_19860301_13889(1).pdf` | 32 | OCR-sökning; förstasida samt s. 26 och 29 kontrollerade. S. 9:s Nato-kampanjmaterial ingick i tidigare pass. Ny import av energiåtgärder; ingen dubblering av äldre fynd. |
| `lastampa_1986-03-01(1).pdf` | 42 | OCR-sökning; ny fördjupning s. 4. Förstasida samt s. 2, 6–7 hade också kontrollerats tidigare. EG-undertecknande och Tchadtransport importerade; tidigare CGIL- och Egyptenmaterial behållet. |
| `mangayar_malar1986-03-01.pdf` | 100 | Samtliga sidor i sidöversikt, omslag och kolofon identifierade. Tamilsk kvinno-/kulturtidskrift från Madras, märkt mars 1986; filnamnets 1/3 är inte belagt som utgivningsdag. Ingen fullständig översättning av de tamilska artiklarna genomförd. Inget dagsdaterat internationellt fynd importerat. |
| `Mar 01 1986, The Times, #62393, UK (en).pdf` | 32 | Samtliga sidor i sidöversikt; s. 1–8, 16 och 23 förstorade för nyheter, fortsättningar och ekonomi. Bland annat Nederländerna, Brasilien, Uganda, Filippinerna, Nordirland och telefonvarning. Opinionsartiklar hålls skilda från nyhetsrapportering. |
| `Mar 02 1986, The Jerusalem Post, #16154, Israel (en)(1).pdf` | 8 | Samtliga tillgängliga sidor i översikt; s. 1–3 och 6 särskilt lästa, tillsammans med tidigare genomgång. PDF 3 och 4 duplicerar tryckt s. 3; tryckt s. 4 saknas i den här filen. Ny Egypten-/Bright Star-information importerad; tidigare Demjanjuk-, Zarit-, Taba- och reaktionsposter inte dubblerade. |
| `Mar 02 1986, הארץ (Haaretz), #20316, Israel (he)(1).pdf` | 12 | Samtliga sidor i översikt; PDF 1, 2, 5, 6 och 12 särskilt granskade för nyheter och fortsättningar. Tidigare Peres-material behållet. Inga ytterligare säkert avlästa nya uppgifter valda för import; svårlästa hebreiska detaljer lämnas öppna. |
| `NPTKP19860301.pdf` | 20 | Ta Kung Pao, Hongkong. Alla sidor i översikt; PDF 1–3 lästa för internationella och kinesiska nyheter. Oljepris, Brasilien, Filippinerna och investeringar. Kina-statistiken avser 1985 och har inte omdaterats till en händelse 28/2. |
| `NPWK19860301.pdf` | 28 | Wah Kiu Yat Po, Hongkong. Alla sidor i översikt; PDF 1–2 samt 26–27 förstorade. Internationella nyheter, lokala bränslepriser, handel och finans. Inget separat säkert nytt Palmefynd; brödtexten i hela filen inte transkriberad. |
| `O-0760-1986-0009-32693.pdf` | 4 | Gazette of India, Part IV. OCR och sidinnehåll identifierat: privata namn-/dokumentkungörelser, tryckta s. 35–37 och en blank sida. Inte en internationell dagstidning. Inget importerat. |
| `Piccolo_1986-03-01.pdf` | 18 | Il Piccolo, Trieste. Hela OCR-texten sökt; förstasida och utrikessidan 12 visuellt kontrollerade. Toppmöte och nedrustningsförslag importerade. |
| `pub_el-popular_1986-03-01_16_103.pdf` | 8 | Toronto, spanskspråkig lördags-/kulturbilaga. OCR-genomgång av sektionerna och visuell kontroll av s. 1 och 6. S. 6 är en kommenterande text om bland annat Contra-stöd och Contadoraprocessen, inte en oberoende bekräftelse på en hemlig operation. |
| `pub_latvija-amerika_1986-03-01_36_9.pdf` | 20 | Toronto, lettisk veckotidning. OCR-sökning och kontroll av omslag samt s. 11:s internationella kommentarer. Omslaget anger uttryckligen postning 26 februari. Får inte behandlas som en tidning skriven efter mordet bara för att numret är daterat 1/3. |
| `SCO_1986030101.pdf` | 12 | Scandinavian News, Edmonton. Omslag och OCR-sektioner kontrollerade. Omslaget anger mars, medan inre sidhuvuden anger april: osäker datering i skanningen. Danska folkomröstnings-/EG-uppgifter bekräftas bättre i La Stampa/The Times och har inte daterats efter filnamnet. |

## Källkritiska observationer och ej importerade uppgifter

1. **Tidig mordrapportering:** olika uppgifter om skott, kaliber, patronhylsor, flyktfordon och möjliga organisationer förekommer i de första rapporterna. Stampa Sera återger också polisens avståndstagande från säkra organisationsanklagelser. Dessa tidiga uppgifter har inte blivit nya fastslagna mordfakta i spelet.
2. **FN-kontakten:** Palme nämns uttryckligen i rapporten om samrådet med Pérez de Cuéllar. Det är ett konkret uppslag för fortsatt kontroll i FN:s och Palmes arkiv, men varken mötesdag, kontaktform eller ett samband med mordet är fastställt. Skriv inte ”telefonsamtal den 28 februari” utifrån denna artikel.
3. **Frankrike/Iran:** The Times s. 5 beskriver iranska åtgärder mot franska tekniker och en AFP-korrespondent; tidigare tidningsmaterial beskriver franska utvisningar av iranier. De får inte slås ihop till samma händelse. Spionageanklagelser är partsuppgifter och någon Palmeanknytning framgår inte.
4. **Militär materiel:** transporten till Tchad och en aviserad amerikansk robotförsäljning till Saudiarabien är inte i sig bevis för vapensmuggling. Inga nya smugglingpilar har skapats av de artiklarna.
5. **Iran–Irak/tankfartyg:** rapporterna om tankern Castor, döda sjömän och attacker kring Kharg ger krigskontext, men det exakta attackdatumet har inte fastställts tillräckligt i detta pass för `news_day`. Inga förlustsiffror införda som oberoende verifierade.
6. **Haiti:** krav/planer på att begära Duvalier utlämnad från Frankrike återges i flera tidningar. Detta betyder inte att en utlämning skedde. Materialet noterades men någon ny landsprofil skapades inte.
7. **Egypten:** arresteringstalen varierar mellan tidningar och tidpunkter. Importen använder ingen sammanräkning av dessa siffror.
8. **Ekonomiska siffror:** oljepriser avser olika kontrakt, marknader och tidpunkter. Kinas handelsuppgifter avser olika redovisningar för 1985. Skillnaderna har inte jämnats ut till en falskt exakt gemensam siffra.
9. **Äldre händelser:** Latvija Amerikā s. 11 om familjen Randperes försök till återförening och Scandinavian News om Wallenberg/föreningsbesök är inte nyheter om reaktioner på Palmes död.

Ingen ny belagd koppling mellan en misstänkt aktör och själva mordet har identifierats i de granskade artiklarna. Fördjupningsuppslag och samtida misstankar hålls åtskilda från belagda samband. En tematisk OCR-sökning eller sidöversikt kan inte utesluta att ytterligare relevant material finns i återstående brödtext.

## Validering

Atlasens 12 Pythonkontroller och lokaliseringens 23 kontroller passerar. De kontrollerar bland annat JSON-struktur, ID:n, koordinater, hänvisningar och aktuella översättningar; de verifierar inte historiska sakuppgifter. Ingen Unreal-kompilering eller visuell körning i editorn har utförts i denna miljö.
