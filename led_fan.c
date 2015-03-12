#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL2_framerate.h>
#include <sys/time.h>
#include <math.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#ifndef PI
#define PI M_PI
#endif

#define TAU 6.28318530717958647693

#define N 91.0 /* 扇页数 */
#define DN 256.0 /* 圆周像素数 */

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
	void (*cb)(struct led_s *led, void *p); /* callback */
};

struct led_s *create_led(int cx, int cy, int r, int w, Uint32 color,
			 double st_angle, int period,
			 void (*cb)(struct led_s *, void *))
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
	led->cb = cb;
	return led;
}

void run_led(SDL_Renderer *renderer, struct led_s *led)
{
	int elapsed_ms = mstime() - led->start_ms;
	int x;
	int y;
	double angle = TAU * elapsed_ms / led->period + led->start_angle;

	int i;
	double tmp_angle;
	for (i = 0; i < N; i++) {
		tmp_angle = angle + i * TAU / N;
		x = led->center.x + led->r * cos(tmp_angle);
		y = led->center.y + led->r * sin(tmp_angle);
		if (led->cb)
			led->cb(led, renderer);
		filledCircleColor(renderer, x, y, led->w, led->color);
	}
}

int main(int argc, char **argv)
{
	SDL_Window *screen;
	SDL_Renderer *renderer;
	SDL_Event event;
	FPSmanager fps_mgr;
	struct led_s *led[16];
	int i;

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
	SDL_initFramerate(&fps_mgr);
	renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);

	/* Clear the screen */
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	for (i = 0; i < ARRAY_SIZE(led); i++) {
		led[i] = create_led(320, 240, 50 + i*2, 1, 0xFFFFFFFF, 0, 551,
				    NULL);
	}
	SDL_setFramerate(&fps_mgr, 200);
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

		for (i = 0; i < ARRAY_SIZE(led); i++) {
			run_led(renderer, led[i]);
		}
		SDL_RenderPresent(renderer);
		/* Adjust framerate */
		SDL_framerateDelay(&fps_mgr);
		boxRGBA(renderer, 0, 0, 639, 479, 0, 0, 0, 255);
	}
	return 0;
}
