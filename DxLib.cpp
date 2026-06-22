#include "DxLib.h"

#define SYOBON_COLOR_KEY(img) SDL_MapRGB(img, 9 * 16 + 9, 255, 255)

SDL_Joystick* joystick;
SDL_JoystickID joystick_id = -1;

std::map<SDL_Keycode, bool> keysHeld;
bool sound = true;
void deinit();
int DxLib_Init()
{
    atexit(deinit);
    setlocale(LC_CTYPE, "ja_JP.UTF-8");

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
	fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
	return -1;
    }

    SDL_DisplayMode dm;
    int screen_w = 480;
    int screen_h = 420;
    if (SDL_GetDesktopDisplayMode(0, &dm) == 0) {
        screen_w = dm.w;
        screen_h = dm.h;
    }

    window = SDL_CreateWindow("Syobon Action (しょぼんのアクション)",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              screen_w, screen_h,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        fprintf(stderr, "Unable to create window: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!renderer) {
        fprintf(stderr, "Unable to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    SDL_RenderSetLogicalSize(renderer, 480, 420);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 480, 420);
    if (!texture) {
        fprintf(stderr, "Unable to create texture: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    screen = SDL_CreateRGBSurface(0, 480, 420, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    if (!screen) {
        fprintf(stderr, "Unable to create screen surface: %s\n", SDL_GetError());
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    SDL_ShowCursor(SDL_DISABLE);

    if(IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
    {
        fprintf(stderr, "Unable to init SDL_img: %s\n", IMG_GetError());
        return -1;
    }

    //Initialize font
    if (TTF_Init() == -1) {
	fprintf(stderr, "Unable to init SDL_ttf: %s\n", TTF_GetError());
	return -1;
    }

    //Audio Rate, Audio Format, Audio Channels, Audio Buffers
#define AUDIO_CHANNELS 2
    if (sound && Mix_OpenAudio(22050, AUDIO_S16SYS, AUDIO_CHANNELS, 1024)) {
        fprintf(stderr, "Unable to init SDL_mixer: %s\n", Mix_GetError());
        sound = false;
        }
    //Try to get a joystick
    //
    /* Batocera 會亂掉。就用 PADtoKey
    joystick = SDL_JoystickOpen(0);
    if (joystick) {
        joystick_id = SDL_JoystickInstanceID(joystick);
    }
    */
    keysHeld.clear();
    for (int i = 0; i < FONT_MAX; i++)
	font[i] = NULL;
    srand(time(NULL));

    return 0;
}

//Main screen
SDL_Surface *screen = NULL;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;

//Fonts
byte fontsize = 0;
TTF_Font *font[FONT_MAX];

//Strings
void SetFontSize(byte size)
{
    fontsize = size;
    if (font[size] == NULL) {
	font[size] = TTF_OpenFont("res/sazanami-gothic.ttf", size);
	if (font[size] == NULL) {
	    printf("Unable to load font: %s\n", TTF_GetError());
	    exit(1);
	}
    }
}

byte fontType = DX_FONTTYPE_NORMAL;
void ChangeFontType(byte type)
{
    fontType = type;
}

void DrawString(int a, int b, const char *x, Uint32 c)
{
    SDL_Color color = { (Uint8)(c >> 16), (Uint8)(c >> 8), (Uint8)(c) };
    SDL_Surface *rendered = TTF_RenderUTF8_Solid(font[fontsize], x, color);
    if (fontType == DX_FONTTYPE_EDGE) {
	SDL_Color blk = { 0, 0, 0 };
	SDL_Surface *shadow = TTF_RenderUTF8_Solid(font[fontsize], x, blk);
	DrawGraphZ(a - 1, b - 1, shadow);
	DrawGraphZ(a, b - 1, shadow);
	DrawGraphZ(a + 1, b - 1, shadow);
	DrawGraphZ(a - 1, b, shadow);
	DrawGraphZ(a + 1, b, shadow);
	DrawGraphZ(a - 1, b + 1, shadow);
	DrawGraphZ(a, b + 1, shadow);
	DrawGraphZ(a + 1, b + 1, shadow);
	SDL_FreeSurface(shadow);
    }
    DrawGraphZ(a, b, rendered);
    SDL_FreeSurface(rendered);
}

void DrawFormatString(int a, int b, Uint32 color, const char *str, ...)
{
    va_list args;
    char *newstr = new char[strlen(str) + 16];
    va_start(args, str);
    vsnprintf(newstr, sizeof(newstr), str, args);
    va_end(args);
    DrawString(a, b, newstr, color);
    delete[] newstr;
}

//void DrawFormatString(int a, int b, int c

//Key Aliases
#define KEY_INPUT_ESCAPE SDLK_ESCAPE

bool ex = false;

void UpdateKeys()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
	switch (event.type) {
	case SDL_KEYDOWN:
	    keysHeld[event.key.keysym.sym] = true;
	    break;
	case SDL_KEYUP:
	    keysHeld[event.key.keysym.sym] = false;
	    break;
	case SDL_JOYAXISMOTION:
	    if(event.jaxis.which == joystick_id)
	    {
		if(event.jaxis.axis == JOYSTICK_XAXIS)
		{
		    if(event.jaxis.value < 0) keysHeld[SDLK_LEFT] = true;
		    else if(event.jaxis.value > 0) keysHeld[SDLK_RIGHT] = true;
		    else {
			keysHeld[SDLK_LEFT] = false;
			keysHeld[SDLK_RIGHT] = false;
		    }
		}
		else if(event.jaxis.axis == JOYSTICK_YAXIS)
		{
		    if(event.jaxis.value < 0) keysHeld[SDLK_UP] = true;
		    else if(event.jaxis.value > 0) keysHeld[SDLK_DOWN] = true;
		    else {
			keysHeld[SDLK_UP] = false;
			keysHeld[SDLK_DOWN] = false;
		    }
		}
	    }
	    break;
	case SDL_QUIT:
	    ex = true;
	    break;
	}
    }
}

byte ProcessMessage()
{
    return ex;
}

byte CheckHitKey(int key)
{
    if(key == SDLK_z && keysHeld[SDLK_SEMICOLON]) return true;
    return keysHeld[key];
}

int WaitKey()
{
    SDL_Event event;
    while (SDL_WaitEvent(&event)) {
	if (event.type == SDL_KEYDOWN)
	    return event.key.keysym.sym;
	if (event.type == SDL_QUIT) {
	    ex = true;
	    return SDLK_ESCAPE;
	}
    }
    return 0;
}

void ScreenFlip()
{
    if (renderer && texture && screen) {
        SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
}

/*Uint32 GetColor(byte r, byte g, byte b)
{
    return r << 8 * 3 | g << 8 * 2 | b << 8 | 0xFF;
}*/

void DrawGraphZ(int a, int b, SDL_Surface * mx)
{
    if(mx)
    {
        SDL_Rect offset;
        offset.x = a;
        offset.y = b;
        SDL_BlitSurface(mx, NULL, screen, &offset);
    }
}

void DrawTurnGraphZ(int a, int b, SDL_Surface * mx)
{
    if(mx)
    {
        SDL_Rect srcrect;
        srcrect.x = srcrect.y = 0;
        srcrect.w = mx->w;
        srcrect.h = mx->h;

        SDL_Rect offset;
        offset.x = a;
        offset.y = b;

        SDL_Surface *flipped = zoomSurface(mx, -1, 1, 0);
        SDL_SetColorKey(flipped, SDL_TRUE,
                SYOBON_COLOR_KEY(flipped->format));
        SDL_BlitSurface(flipped, &srcrect, screen, &offset);
        SDL_FreeSurface(flipped);
    }
}

void DrawVertTurnGraph(int a, int b, SDL_Surface * mx)
{
    if(mx)
    {
        SDL_Rect srcrect;
        srcrect.x = srcrect.y = 0;
        srcrect.w = mx->w;
        srcrect.h = mx->h;

        SDL_Rect offset;
        offset.x = a - mx->w / 2;
        offset.y = b - mx->h / 2;

        SDL_Surface *flipped = zoomSurface(mx, -1, -1, 0);
        SDL_SetColorKey(flipped, SDL_TRUE,
                SYOBON_COLOR_KEY(flipped->format));
        SDL_BlitSurface(flipped, &srcrect, screen, &offset);
        SDL_FreeSurface(flipped);
    }
}

SDL_Surface *DerivationGraph(int srcx, int srcy, int width, int height,
			     SDL_Surface * src)
{
    SDL_Surface *img =
	SDL_CreateRGBSurface(0, width, height,
			     screen->format->BitsPerPixel,
			     src->format->Rmask, src->format->Gmask,
			     src->format->Bmask, src->format->Amask);

    SDL_Rect offset;
    offset.x = srcx;
    offset.y = srcy;
    offset.w = width;
    offset.h = height;

    SDL_BlitSurface(src, &offset, img, NULL);
    SDL_SetColorKey(img, SDL_TRUE,
		    SYOBON_COLOR_KEY(img->format));
    return img;
}

//Noticably different than the original
SDL_Surface *LoadGraph(const char *filename, bool fix)
{
    SDL_Surface *image = IMG_Load(filename);

    if (image)
    {
        if(fix)
        {
            static SDL_PixelFormat fmt;
            static char setfmt = 0;
            if(!setfmt)
            {
                fmt = *(image->format);
                setfmt = 1;
            }
            SDL_PixelFormat newfmt = *(image->format);
            
            if(newfmt.BytesPerPixel != 1)
            {
                printf("WARNING: %s pixel format is not the one required, trying to fix...\n", filename);

                SDL_Surface *newimage = SDL_ConvertSurface(image, &fmt, 0);
                if(newimage)
                {
                    printf("Successfully converted\n");
                    SDL_FreeSurface(image);
                    image = newimage;
                    newimage = nullptr;
                }
                else
                {
                    printf("Conversion failed: %s\n", SDL_GetError());
                }
            }
        }

        if(image)
        {
            return image;
        }
    }
	fprintf(stderr, "Error: Unable to load %s: %s\n", filename, IMG_GetError());
	exit(1);
}

void PlaySoundMem(Mix_Chunk* s, int l)
{
    if(sound) Mix_PlayChannel(-1, s, l);
}

Mix_Chunk* LoadSoundMem(const char* f)
{
    if(!sound) return NULL;

    Mix_Chunk* s = Mix_LoadWAV(f);
    if(s) return s;
    fprintf(stderr, "Error: Unable to load sound %s: %s\n", f, Mix_GetError());
    return NULL;
}

Mix_Music* LoadMusicMem(const char* f)
{
    if(!sound) return NULL;

    Mix_Music* m = Mix_LoadMUS(f);
    if(m) return m;
    fprintf(stderr, "Error: Unable to load music %s: %s\n", f, Mix_GetError());
    return NULL;
}

