#include <SDL.h>
#include <SDL2_gfxPrimitives.h>

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
	SDL_Renderer *renderer;
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
	renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);
	// Update the surface
	// SDL_UpdateWindowSurface(screen);

	/* Clear the screen */
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
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

		filledCircleRGBA(renderer, 100, 200, 50, 255, 255, 100, 255);
		SDL_RenderPresent(renderer);
		/* Adjust framerate */
		SDL_Delay(25);
	}
	return 0;
}
