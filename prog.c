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
	Uint8 block = fgetc(random) / 4 + 8;

	r.x = (fgetc(random) << 8 | fgetc(random)) % (sur->w) - (block/2);
	r.y = (fgetc(random) << 8 | fgetc(random)) % (sur->h) - (block/2);
	r.w = (fgetc(random) << 8 | fgetc(random)) % (sur->w);
	r.h = (fgetc(random) << 8 | fgetc(random)) % (sur->h);
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
	char *filename2;

	if(argc > 1)
		filename = argv[1];
	else
		filename = "image.bmp";

	if(argc > 2)
		filename2 = argv[2];
	else
		filename2 = NULL;

    SDL_Init(SDL_INIT_VIDEO);

	FILE *urandom = fopen("/dev/urandom", "rb");

	SDL_Surface *target = SDL_LoadBMP(filename);
	SDL_Surface *sur;
	if(filename2)
		sur = SDL_LoadBMP(filename2);
	else
		sur = SDL_CreateRGBSurface(
			0, target->w, target->h, 24, 0, 0, 0, 0);

	SDL_Rect rect = {0, 0, sur->w, sur->h};
//	SDL_FillRect(sur, &rect, 0xFF888888);

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

		double percent;
		static double curPercent = 10;

		int ticks = SDL_GetTicks();
//		if(new_diff < old_diff) { // display every change
		if(ticks - oldTicks >= 60) { // display every 30ms/f ~= 15fps
			oldTicks = ticks;
			
			percent = 100 - 100.0 *
				(old_diff / (sur->w * sur->h * 3 * 255.0));

			
			++frames;
			if(frames%30 == 0) // Revert to 150
				printf("%.1f%%\n",  percent);

			SDL_Texture *tex;
			tex = SDL_CreateTextureFromSurface(ren, sur);

			SDL_RenderClear(ren);
			SDL_RenderCopy(ren, tex, NULL, NULL);
			SDL_DestroyTexture(tex);

			SDL_RenderPresent(ren);

		}

		if(percent > curPercent) {
			char string[] = " .bmp";
			string[0] = curPercent/10 - 1 + '0';
			SDL_SaveBMP(sur, string);
			curPercent += 10;
			if(curPercent < 100)
				break;
		}
	}

	
	SDL_Quit();
    return 0;
}
