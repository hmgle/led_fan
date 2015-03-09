#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL2_framerate.h>
#include <sys/time.h>
#include <math.h>

#ifndef PI
#define PI 3.14159
#endif

/* Return the UNIX time in microseconds */
long long ustime(void)
{
	struct timeval tv;
	long long ust;

	gettimeofday(&tv, NULL);
	ust = ((long long)tv.tv_sec)*1000000;
	ust += tv.tv_usec;
	return ust;
}

/* Return the UNIX time in milliseconds */
long long mstime(void)
{
	return ustime()/1000;
}

struct point_s {
	int x;
	int y;
};

struct led_s {
	struct point_s center;
	int r;
	int w;
	Uint32 color;		/* 发光颜色 */
	double start_angle;	/* 起始角度 */
	int period;		/* 旋转周期: ms */
	long long start_ms;
	FPSmanager fps_mgr;
};

struct led_s *create_led(int cx, int cy, int r, int w, Uint32 color,
			 double st_angle, int period)
{
	struct led_s *led = malloc(sizeof(*led));

	led->center.x = cx;
	led->center.y = cy;
	led->r = r;
	led->w = w;
	led->color = color;
	led->start_angle = st_angle;
	led->period = period;
	led->start_ms = mstime();
	SDL_initFramerate(&led->fps_mgr);
	return led;
}

void run_led(SDL_Renderer *renderer, struct led_s *led)
{
	int elapsed_ms = mstime() - led->start_ms;
	led->start_angle += 2.0 * PI * elapsed_ms / led->period;
	int x = led->center.x + led->r * cos(led->start_angle);
	int y = led->center.y + led->r * sin(led->start_angle);
	filledCircleColor(renderer, x, y, led->w, led->color);
}

int main(int argc, char **argv)
{
	SDL_Window *screen;
	SDL_Renderer *renderer;
	SDL_Event event;
	struct led_s *led = create_led(320, 240, 80, 4, 0xFFFFFFFF, 0, 24);

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

		run_led(renderer, led);
		SDL_RenderPresent(renderer);
		/* Adjust framerate */
		SDL_framerateDelay(&led->fps_mgr);
	}
	return 0;
}
