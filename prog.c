#include <SDL2/SDL.h>

Uint32 surface_diff(SDL_Surface *a, SDL_Surface *b)
{
	Uint32 magnitude = 0;

	int i;
	for(i=0; i< a->w * a->h * 3; ++i) {
		magnitude += abs(((Uint8*)(a->pixels))[i] - ((Uint8*)(b->pixels))[i]);
	}

	return magnitude;
}

SDL_Surface *surface_dupe(SDL_Surface *sur)
{
	SDL_Surface *new = SDL_CreateRGBSurface(
		0, sur->w, sur->h, 24, 0, 0, 0, 0);

	SDL_BlitSurface(sur, NULL, new, NULL);

	return new;
}

void modify(SDL_Surface *sur, FILE *random)
{
	SDL_Rect r;
	Uint8 block = fgetc(random) / 8 + 8;

	r.x = (fgetc(random) << 8 | fgetc(random)) % (sur->w - block);
	r.y = (fgetc(random) << 8 | fgetc(random)) % (sur->h - block);
	r.w = block;
	r.h = block;

	Uint32 color =
		(fgetc(random) << 8 | fgetc(random)) << 16 |
		(fgetc(random) << 8 | fgetc(random));

	SDL_FillRect(sur, &r, color);
}

SDL_Window *win;
SDL_Renderer *ren;

int main(int argc, char *argv[])
{
	char *filename;

	if(argc < 2)
		filename = "image.bmp";
	else
		filename = argv[1];


    SDL_Init(SDL_INIT_VIDEO);

	FILE *urandom = fopen("/dev/urandom", "rb");

	SDL_Surface *target = SDL_LoadBMP(filename);
	SDL_Surface *sur = SDL_CreateRGBSurface(
		0, target->w, target->h, 24, 0, 0, 0, 0);

	SDL_Rect rect = {0, 0, sur->w, sur->h};
	SDL_FillRect(sur, &rect, 0xFF888888);

	win = SDL_CreateWindow("Test", 0, 0, target->w, target->h, 0);
	ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_PRESENTVSYNC);

	SDL_Event e;
	int loop = 1;
	Uint32 frames = 0;
	int oldTicks = 0;
	while (loop) {
		while(SDL_PollEvent(&e))
			if(e.type == SDL_QUIT)
				loop = 0;
		++frames;

		SDL_Surface *new = surface_dupe(sur);
		modify(new, urandom);
		Uint32 old_diff = surface_diff(target, sur);
		Uint32 new_diff = surface_diff(target, new);

		if(new_diff < old_diff) {
			SDL_Surface *tmp = sur;
			sur = new;
			new = tmp;
		}

		SDL_FreeSurface(new);

		int ticks = SDL_GetTicks();
		if(ticks - oldTicks >= 60) { // 30ms/f ~= 15fps
			oldTicks = ticks;

			SDL_Texture *tex;
			tex = SDL_CreateTextureFromSurface(ren, sur);

			SDL_RenderClear(ren);
			SDL_RenderCopy(ren, tex, NULL, NULL);
			SDL_DestroyTexture(tex);

			SDL_RenderPresent(ren);
		}
	}
	
	SDL_Quit();
    return 0;
}
