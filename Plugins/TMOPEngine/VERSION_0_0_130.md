# TMOPEngine 0.0.130

- Egna ansikten i `AppearanceProfile.Face` har fortsatt högsta prioritet.
- Personer utan eget ansikte får automatiskt närmaste standardhuvud för kön och
  ålder: man/kvinna, 18/30/45/65.
- `AgeAtEvent=0` använder 30-årshuvudet; okänt kön behåller obscured-fallback.
- Standardhuvuden döljer kroppens Head-region via appearance-katalogen.
- Saknade eller feltypade exakta katalograder faller tillbaka säkert och ger
  diagnostik i stället för ett osynligt modular mesh.
- Explicit face override ignoreras inte längre när `UnknownPartStyle=Hidden`.
