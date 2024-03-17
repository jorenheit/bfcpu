# Short Introduction (~30 seconds)
Ik ben een student uit V6B en aangezien mijn passie in computer science en electronics, ik kies een onderzoek en ontwikkeling van een processor die speciale machinecode uitvoert. In feite, Ik bedank mijn natuurkundeleraar, Joren Heit, voor het aanbieden van deelname aan dit project.

# Brainfck
Voordat ik dieper inga op het doel van het project, hebben we wat achtergrondkennis nodig. In het bijzonder begrip van de esoterische programmeertaal, BF. Ondanks een verdachte naam, kun je aan deze taal denken als een set van specifieke "opdrachten" die je kunt uitvoeren en waarmee je verschillende soorten programma's kunt maken.

Stel je voor dat je 's ochtends wakker wordt en je wilt douchen. Wat zijn de specifieke handelingen of opdrachten die je uitvoert?

Je opent je ogen.
Je staat op.
Ga voorzichtig naar de badkamer.
Kleren uitdoen.
Begin met water laten stromen.
Enzovoort.
Deze mini-opdrachten zijn "operaties" die een programma vormen, dat in ons geval het "douchen in de ochtend" is.
Op dezelfde manier zijn in BF de operaties aanwezig: >, <, +, -, ., ",", [, ]
Met behulp van deze operaties kun je een programma maken.

Om BF te begrijpen, stel je een rij zogenaamde cellen voor die een getal kunnen opslaan.

***first animation***
De momenteel geselecteerde cel wordt aangewezen door de pijl.

Neem nu dit eenvoudige programma: "++>++>--<+--<+++>--<+++>-++>--<+-<+"
"+" in BF betekent dat er één wordt toegevoegd aan de momenteel geselecteerde cel, dus we hebben 1 in de 1e cel.

[1][][][][][][]

hetzelfde geldt voor de tweede plus

[2][][][][][][]

">" betekent dat de volgende cel wordt geselecteerd, dus we kijken naar de tweede cel.

"-" betekent dat er één wordt afgetrokken. Dus trekken we er één af van de tweede cel.

[2][-1][][][][][]

Door op deze manier berekeningen uit te voeren, krijgen we de uiteindelijke celconfiguratie.

De volgende voorbeeld heeft de volgende programma: ...
Nou, op tafels hebben jullie een papier waarop jullie zelf het resultaat kunt berekenen tijdens de komende 15 seconden. **De tijd begint nu**

[2][-2][][][][][]
[3][-2][][][][][]
[2][0][][][][][]
[1][0][][][][][]
[0][0][0][0][0][0][0]

Tijd is op. Op het bord zie je nu de animatie van de berekening.
Als je dit resultaat hebt, wees dan trots, want je hebt alles correct gedaan.

# Doel
Nu begrijpt jij waar BF voor staat, kunnen we ons zicht op de computer verleggen. In feite kan de computer die u gebruikt slechts enkele en heel eenvoudige handelingen begrijpen, zoals die van BF. Dit betekent dat wanneer u Chrome (of TikTok) wilt starten, u zich op gebruikersniveau bevindt en dubbelklikt op het Google Chrome-pictogram om het te openen. Op een dieper niveau heeft de computer het besturingssysteem dat het Google-programma kan vertalen in een taal die een computer kan begrijpen. En nogmaals, deze handelingen zijn net zo eenvoudig als ophogen (+ of -) en naar links (< of >) gaan in BF. Vervolgens komt het CPU-niveau aan bod, en de CPU kan deze enorme hoeveelheden bewerkingen namelijk in milliseconden uitvoeren.

Het doel van het project is het creëren van een CPU die BF-code kan uitvoeren met behulp van de eenvoudigste elektronische componenten, zoals resistors, logic chips en capacitors.

# Resultaten
Helaas is de huidige staat van de CPU nog niet klaar. Het bleek een project van langere duur te zijn, en ik kan niet aantonen hoe BF wordt uitgevoerd. Omdat de CPU volgens ons ontwerp echter uit modules bestaat (zoals geheugen, rekenmachinemodule, enz.), werken sommige modules wel, maar zijn ze nog steeds niet met elkaar verbonden. Voor meer verdieping in de werkingsprincipes kunt u een mini-documentatieboek lezen dat wij hebben geschreven. En als je uitleg en observaties zoekt voor waartoe deze CPU al in staat is, kun je naar mij toe komen en het bespreken.

# Process
Het proces om dit stuk hardware te maken was behoorlijk moeilijk maar toch interessant.

Ten eerste moest ik twee boeken over digitale en analoge elektronica gedeeltelijk hebben gelezen om een basisidee van elektronica te krijgen. Daarna keek ik naar series aangeboden door Ben Eater, die ook een CPU maakte. Daarna was het tijd om na te denken over ons eigen ontwerp, schakkelingen, componenten en zo.

Er zijn weinig momentopnamen van de voortgang op het bord. Op sommige momenten werkten de CPU-modules niet goed of helemaal niet, en er was wat debug nodig. Bij sommige gelegenheden was ik twee weken lang betrokken bij dit debugproces.

# Volgende stappen
Ik ben vastbesloten dit project tot het einde te leiden. Mijn volgende stappen zijn dus het maken van de rest van de modules, het verbinden ervan, het testen van hun werk en het vrijgeven van de eerste versie van de CPU. Als je de gang van zaken wilt volgen, kun je deze webpagina bezoeken. Daar vind je de softwarekant van het project en de documentatie die ik probeer up-to-date te houden.

# Goodbye
Nou, dat was het. Ik hoop dat je deze presentatie leuk vond en geniet van de rest van de avond!