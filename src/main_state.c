#include <main_state.h>
#include <glad/glad.h>
#include <math.h>

#include <rafgl.h>

static int w, h;
static rafgl_raster_t raster;
static rafgl_texture_t tex;

void main_state_init(GLFWwindow *window, void *args, int width, int height)
{
//  upamtimo vrednosti sirine i visine prozora
//koje smo prosledili pozivom main_state_init() funkcije u okviru main.c fajla
    w = width;
    h = height;
//    inicijalizacija rastera na w*h piksela (u ovom primeru na 256*256)
    rafgl_raster_init(&raster, w, h);
//     inicijalizujemo slot za teksturu koji kasnije koristimo za prikazivanje rastera
    rafgl_texture_init(&tex);

}

void main_state_update(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void *args)
{
    int x,y;
//    dvostrukom for petljom prolazimo kroz svaki piksel rastera
//    tacka (0, 0) je gornji levi ugao slike a tacka (w-1, h-1) je donji desni ugao
    for(y=0; y<h; y++){
        for(x=0;x<w;x++){
//            uz pomoc pixel_at_m makroa mozemo pristupiti pikselu na koordinatama x, y unutar rastera r (makro pixel_at_pm koristimo ako imamo pokazivac na raster)

                /*
                pixel je union i mozemo ga koristiti na vise nacina. ako imamo pixel pix i zelimo da mu promenimo jednu komponentu, na primer crvenu
                mozemo da uradimo pix.r = 255; isto to mozemo uraditi kao pix.components[0] = 255; ako zelimo da promenimo sve komponente istovremeno
                (zadamo neku specificnu boju, na primer zutu) mozemo da uradimo pix.rgba = rafgl_RGB(255, 255, 0);
                */

            /* u ovom primeru je zadato da se raster popuni flat bojom (ista boja na celom rasteru),
            tako da jednostavno mozemo svaki piksel postaviti da ima istu vrednost */
            pixel_at_m(raster, x, y).rgba = rafgl_RGB(255, 0, 0);
        }
    }
}

void main_state_render(GLFWwindow *window, void *args)
{
    /* update-uj teksturu */
    /* ova funkcija sluzi da primeni promene rastera na samu teksturu (spusta sliku iz rama u vram graficke kartice) */
    rafgl_texture_load_from_raster(&tex, &raster);
    /* prikazi teksturu */

    /* prikazujemo teksturu razvucenu preko celog prozora. Prozor i raster ne moraju biti istih dimenzija,
     ali ako nisu, moze doci do razvlacenja ili skupljanja rastera. */
    rafgl_texture_show(&tex, 0);
}

void main_state_cleanup(GLFWwindow *window, void *args)
{

}
