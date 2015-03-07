#include <SDL.h>

struct point_s {
	double x;
	double y;
};

struct led_s {
	struct point_s center;
	double r;
	double start_angle;	/* 起始角度 */
	double period;		/* 旋转周期 */
	uint32_t color;		/* 发光颜色 */
};

int main(int argc, char **argv)
{
	SDL_Window *screen;
	SDL_Surface *surface;
	SDL_Event event;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "SDL_Init() failed: %s\n", SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);
	screen = SDL_CreateWindow("led fan",
				  SDL_WINDOWPOS_UNDEFINED,
				  SDL_WINDOWPOS_UNDEFINED,
				  640, 480,
				  SDL_WINDOW_SHOWN);
	if (screen == NULL) {
		fprintf(stderr, "SDL_CreateWindow() failed: %s\n",
				SDL_GetError());
		exit(1);
	}
	surface = SDL_GetWindowSurface(screen);
	// Fill the surface black
	SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0x0, 0x0, 0x0));
	// Update the surface
	SDL_UpdateWindowSurface(screen);
	while (1) {
		while (SDL_PollEvent(&event) != 0) {
			switch (event.type) {
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
				case SDLK_q:
					return 0;
					break;
				}
			}
		}
		// SDL_SetRenderDrawColor();
	}
	return 0;
}
