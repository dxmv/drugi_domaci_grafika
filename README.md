# Dokumentacija

## Opis projekata
Proceduralni "terrain viewer" je realizovan kao RAFGL igra u prozoru 800x600 px i služi za prikaz dinamički generisanog pejzaža. `main_state.c` upravlja inicijalizacijom, renderovanjem i čišćenjem resursa, dok izdvojeni moduli (`terrain.c`, `noise.c`, `camera.c`, `tree.c`, `texture.c`) implementiraju logiku za mrežu terena, šum, kameru i dodatne objekte.

## Tehnike i efekti
- **Proceduralna generacija visine** – `noise.c` implementira 2D Perlin šum i fBm, a `terrain_init`/`terrain_generate_vertices` koriste te vrednosti za visinsku mapu, normalizaciju i centriranje mreže.
- **LODs sa "patch" pristupom** – teren se deli na patrčeve (`TerrainPatch`), za koje se generišu EBO-i po nivou detalja (različiti koraci uzorkovanja i "skirts" trake). Tokom renderovanja se bira odgovarajući LOD na osnovu distance kamere (`main_state_render`).
- **Mešanje tekstura prema visini i nagibu** – GLSL fragment ( `res/shaders/terrain/frag.glsl` ) uzorkuje pesak, travu, stenu i sneg i meša ih `smoothstep` funkcijama zavisno od visine, dok se nagib (dot sa Y normalom) koristi da se strmim delovima doda više stene.
- **Osvetljenje** – jednostavna usmerena svetlost sa prigušenim ambijentom se računa u shaderu za teren i drveće (`u_light_dir`, `u_light_color`, `u_ambient_color`).
- **Skybox** – kubna mapa (šest tekstura u `res/textures/skybox`) se crta pomoću posebnog šejdera i matrice pogleda bez translacije kako bi simulirala beskonačno nebo.
- **Voda** – `water.c` dodaje veliki kvad na fiksnoj visini sa sopstvenim šejderom (`res/shaders/water`) koji uzima refleksiju iz iste skybox kubne mape, kombinuje je sa baznom bojom i blago providnom alfa vrednošću.
- **Sistem za drveće** – `tree_system_init` nasumično bira verteksa pogodna za vegetaciju (opseg visine i mali nagib), učitava OBJ mrežu i crta više instanci sa različitim skalama/rotacijama uz gradijent boje krošnje u shaderu.
- **Kontrole kamere** – slobodna FPS kamera (`camera.c`) podržava W/A/S/D kretanje po XZ ravni, Q/E po Y osi, a rotacija se aktivira desnim tasterom miša. Taster `T` prelazi u wireframe mod, a `ESC` zatvara aplikaciju.

### Napomene za vodu
- Visina vode se trenutno postavlja u `main_state.c` (promenljiva `water_level`), vrednost je u jedinicama sveta; promeni je da podesiš nivo mora.
- Površina se prostire preko cele mreže (`water_init` dobija ukupni `terrain_extent`).
- Boju i intenzitet refleksije možeš da prilagodiš u `water.c` preko `DEFAULT_WATER_COLOR` i `DEFAULT_REFLECTION`, ili iz koda postavljanjem `water.color`/`water.reflection_strength` posle inicijalizacije.

## Pokretanje
Implementacija se oslanja na RAFGL infrastrukturu i GLFW. Projekat dolazi sa gotovim izvorom GLAD-a i shaderima, tako da su dodatne zavisnosti samo `gcc` i `glfw` (na macOS-u se linkuju potrebni framework-ovi kroz Makefile).

```bash
# iz korena repozitorijuma
make        # čisti, kompajlira i pokreće main.out
make build  # samo kompajlira
make run    # pokreće prethodno izgrađeni binar
```

Ukoliko `make` ne pronađe GLFW, proveriti da li je instaliran (npr. `brew install glfw`). Eksperimentalne opcije poput promena tekstura ili modela moguće je izvršiti zamenom fajlova u `res/`.
