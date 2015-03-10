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
	led->start_angle = 2.0 * PI * elapsed_ms / led->period;
	int x = led->center.x + led->r * cos(led->start_angle);
	int y = led->center.y + led->r * sin(led->start_angle);
	filledCircleColor(renderer, x, y, led->w, led->color);
}

int main(int argc, char **argv)
{
	SDL_Window *screen;
	SDL_Renderer *renderer;
	SDL_Event event;
	struct led_s *led0 = create_led(320, 240,  50, 2, 0xFFFFFFFF, 0, 1151);
	struct led_s *led1 = create_led(320, 240,  54, 2, 0xFFFFFFFF, 0, 1151);
	struct led_s *led2 = create_led(320, 240,  58, 2, 0xFFFFFFFF, 0, 1151);
	struct led_s *led3 = create_led(320, 240,  62, 2, 0xFFFFFFFF, 0, 1151);
	struct led_s *led4 = create_led(320, 240,  66, 2, 0xFFFFFFFF, 0, 1151);
	struct led_s *led5 = create_led(320, 240,  70, 2, 0xFFFFFFFF, 0, 1151);
	struct led_s *led6 = create_led(320, 240,  74, 2, 0xFFFFFFFF, 0, 1151);
	struct led_s *led7 = create_led(320, 240,  78, 2, 0xFFFFFFFF, 0, 1151);
	struct led_s *led8 = create_led(320, 240,  82, 2, 0xFFFFFFFF, 0, 1151);
	struct led_s *led9 = create_led(320, 240,  86, 2, 0xFFFFFFFF, 0, 1151);
	struct led_s *leda = create_led(320, 240,  90, 2, 0xFFFFFFFF, 0, 1151);
	struct led_s *ledb = create_led(320, 240,  94, 2, 0xFFFFFFFF, 0, 1151);
	struct led_s *ledc = create_led(320, 240,  98, 2, 0xFFFFFFFF, 0, 1151);
	struct led_s *ledd = create_led(320, 240, 102, 2, 0xFFFFFFFF, 0, 1151);
	struct led_s *lede = create_led(320, 240, 106, 2, 0xFFFFFFFF, 0, 1151);
	struct led_s *ledf = create_led(320, 240, 110, 2, 0xFFFFFFFF, 0, 1151);

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
	SDL_setFramerate(&led0->fps_mgr, 200);
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

		run_led(renderer, led0);
		run_led(renderer, led1);
		run_led(renderer, led2);
		run_led(renderer, led3);
		run_led(renderer, led4);
		run_led(renderer, led5);
		run_led(renderer, led6);
		run_led(renderer, led7);
		run_led(renderer, led8);
		run_led(renderer, led9);
		run_led(renderer, leda);
		run_led(renderer, ledb);
		run_led(renderer, ledc);
		run_led(renderer, ledd);
		run_led(renderer, lede);
		run_led(renderer, ledf);
		SDL_RenderPresent(renderer);
		/* Adjust framerate */
		SDL_framerateDelay(&led0->fps_mgr);
		boxRGBA(renderer, 0, 0, 639, 479, 0, 0, 0, 255);
	}
	return 0;
}
