/* © 2021 David Given.
 * WordGrinder is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include <string.h>
#include <SDL2/SDL.h>
#include "SDL_FontCache.h"

#define VKM_SHIFT       0x10000
#define VKM_CTRL        0x20000
#define VKM_CTRLASCII   0x40000
#define VK_RESIZE     0x80000
#define VK_TIMEOUT    0x80001

static SDL_Window* window;
static SDL_Renderer* renderer;
static FC_Font* font;
static int cursorx;
static int cursory;
static bool cursor_shown;
static int charwidth;
static int charheight;

enum
{
    COLOUR_BLACK = 0,
    COLOUR_DIM,
    COLOUR_NORMAL,
    COLOUR_BRIGHT,

    NUM_COLOURS
};

static SDL_Color colourmap[NUM_COLOURS] =
{
    [COLOUR_BLACK] =  {0x00, 0x00, 0x00, 0xff},
    [COLOUR_DIM] =    {0x55, 0x55, 0x55, 0xff},
    [COLOUR_NORMAL] = {0x88, 0x88, 0x88, 0xff},
    [COLOUR_BRIGHT] = {0xff, 0xff, 0xff, 0xff},
};

static void fatal(const char* s, ...)
{
    va_list ap;
    va_start(ap, s);
    fprintf(stderr, "error: ");
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(1);
}

void dpy_init(const char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        fatal("could not initialize sdl2: %s", SDL_GetError());

    window = SDL_CreateWindow(
            "WordGrinder",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            640, 480,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    if (!window)
        fatal("could not create window: %s", SDL_GetError());
    SDL_StartTextInput();

    renderer = SDL_CreateRenderer(window, -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
        fatal("could not create renderer: %s", SDL_GetError());
    
    font = FC_CreateFont();  
    if (!FC_LoadFont(font, renderer, "/System/Library/Fonts/SFNSMono.ttf", 20, FC_MakeColor(255, 255, 255, 255), TTF_STYLE_NORMAL))
        fatal("could not load font: %s", SDL_GetError());
    charwidth = FC_GetWidth(font, "m");
    charheight = FC_GetLineSpacing(font);
fprintf(stderr, "%d %d\n", charwidth, charheight);

    cursorx = cursory = 0;
    cursor_shown = false;
}

void dpy_start(void)
{
}

void dpy_shutdown(void)
{
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void dpy_clearscreen(void)
{
    SDL_RenderClear(renderer);
}

void dpy_getscreensize(int* x, int* y)
{
    int screenwidth;
    int screenheight;
    SDL_GetWindowSize(window, &screenwidth, &screenheight);

    *x = screenwidth / charwidth;
    *y = screenheight / charheight;
}

void dpy_sync(void)
{
    SDL_RenderPresent(renderer);
    SDL_UpdateWindowSurface(window);
}

void dpy_setcursor(int x, int y, bool shown)
{
    cursorx = x;
    cursory = y;
    cursor_shown = shown;
}

void dpy_setattr(int andmask, int ormask)
{
}

void dpy_writechar(int x, int y, uni_t c)
{
    char buffer[8];
    char* p = &buffer[0];
    writeu8(&p, c);
    *p = '\0';

    FC_Draw(font, renderer, x*charwidth, y*charheight, buffer);
}

void dpy_cleararea(int x1, int y1, int x2, int y2)
{
}

uni_t dpy_getchar(double timeout)
{
    SDL_Event e;
    int expiry_ms = SDL_GetTicks() + timeout*1000.0;
    if (timeout == -1)
        expiry_ms = INT_MAX;
    for (;;)
    {
        int now_ms = SDL_GetTicks();
        if (now_ms >= expiry_ms)
            return 0;
        if (SDL_WaitEventTimeout(&e, expiry_ms - now_ms))
        {
            switch (e.type)
            {
                case SDL_QUIT:
                    break;

                case SDL_TEXTINPUT:
		        {
                    const char* p = &e.text.text[0];
                    uni_t key = readu8(&p);
                    if (!key)
                        break;

                    return key;
                }

                case SDL_KEYDOWN:
                {
                    uni_t key = e.key.keysym.sym;
                    if ((key >= 0x20) && (key < 0x80))
                    {
                        if (e.key.keysym.mod & KMOD_CTRL)
                        {
                            key = toupper(key);
                            if (key == ' ')
                            {
                                key = -(VKM_CTRLASCII | 0);
                                goto check_shift;
                            }
                            if ((key >= 'A') && (key <= 'Z'))
                            {
                                key = (key & 0x1f) | VKM_CTRLASCII;
                                goto check_shift;
                            }
                        }
                        break;
                    }
                    
                    if (e.key.keysym.mod & KMOD_CTRL)
                        key |= VKM_CTRL;
                check_shift:
                    if (e.key.keysym.mod & KMOD_SHIFT)
                        key |= VKM_SHIFT;
                    key = -key;
                    return key;
                }
            }
        }
    }

    return VK_TIMEOUT;
}

const char* dpy_getkeyname(uni_t k)
{
    switch (-k)
    {
        case VK_RESIZE:      return "KEY_RESIZE";
        case VK_TIMEOUT:     return "KEY_TIMEOUT";
    }

    int mods = -k;
    int key = (-k & 0xfff0ffff);
    static char buffer[32];

    if (mods & VKM_CTRLASCII)
    {
        sprintf(buffer, "KEY_%s^%c",
                (mods & VKM_SHIFT) ? "S" : "",
                key + 64);
        return buffer;
    }

    const char* template = NULL;
    switch (key)
    {
        case SDLK_DOWN:        template = "DOWN"; break;
        case SDLK_UP:          template = "UP"; break;
        case SDLK_LEFT:        template = "LEFT"; break;
        case SDLK_RIGHT:       template = "RIGHT"; break;
        case SDLK_HOME:        template = "HOME"; break;
        case SDLK_END:         template = "END"; break;
        case SDLK_BACKSPACE:   template = "BACKSPACE"; break;
        case SDLK_DELETE:      template = "DELETE"; break;
        case SDLK_INSERT:      template = "INSERT"; break;
        case SDLK_PAGEUP:      template = "PGDN"; break;
        case SDLK_PAGEDOWN:    template = "PGUP"; break;
        case SDLK_TAB:         template = "TAB"; break;
        case SDLK_RETURN:      template = "RETURN"; break;
        case SDLK_ESCAPE:      template = "ESCAPE"; break;
    }

    if (template)
    {
        sprintf(buffer, "KEY_%s%s%s",
                (mods & VKM_SHIFT) ? "S" : "",
                (mods & VKM_CTRL) ? "C" : "",
                template);
        return buffer;
    }

    if ((key >= SDLK_F1) && (key <= (SDLK_F24)))
    {
        sprintf(buffer, "KEY_%s%sF%d",
                (mods & VKM_SHIFT) ? "S" : "",
                (mods & VKM_CTRL) ? "C" : "",
                key - SDLK_F1 + 1);
        return buffer;
    }

    sprintf(buffer, "KEY_UNKNOWN_%d", -k);
    return buffer;
}

// vim: sw=4 ts=4 et

