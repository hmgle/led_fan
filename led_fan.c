#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL2_framerate.h>
#include <sys/time.h>
#include <math.h>
#include <assert.h>

#include "dotfont.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#ifndef PI
#define PI M_PI
#endif

#define TAU 6.28318530717958647693

#define N 291.0 /* 扇页数 */
#define DN 256.0 /* 圆周像素数 */

/* Return the UNIX time in microseconds */
static long long ustime(void)
{
	struct timeval tv;
	long long ust;

	gettimeofday(&tv, NULL);
	ust = ((long long)tv.tv_sec)*1000000;
	ust += tv.tv_usec;
	return ust;
}

/* Return the UNIX time in milliseconds */
static long long mstime(void)
{
	return ustime()/1000;
}

struct plane {
	int w;
	int h;
	uint8_t *pixel;
};

struct plane *create_plane(int w, int h)
{
	struct plane *p = malloc(sizeof(*p));
	p->w = w;
	p->h = h;
	/* TODO */
	p->pixel = calloc(1, w * h);
	/* p->pixel = calloc(1, (w * h + 7) / 8); */
	return p;
}

int plane_add_font(struct plane *p, int x, int y, struct font_data_s *f)
{
	/* TODO */
	int i, j;
	uint8_t *data = f->data;
	assert(f->h == p->h);
	// for (i = x; i < x + f->w; i++) {
	for (i = 0; i < f->h; i++) {
		for (j = 0; j < f->w; j++) {
			// (p->pixel)[i*(p->w)+x+j] = data[i] & (0x1 << (f->w-j));
			(p->pixel)[i*(p->w)+x+j] = data[i] & (0x1 << (j));
			// fprintf(stderr, "%d:%d -  %d\n", i, j, data[i] & (0x1 << j));
			// (p->pixel)[i*(p->w)+x+j] = 1;
		}
	}
	return 0;
}

struct point_s {
	int x;
	int y;
};

struct led_s {
	struct point_s center;
	struct point_s currpo; /* current position */
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

struct rend_pl_s {
	SDL_Renderer *renderer;
	struct plane *pl;
	double angle;
};

void disp_font(struct led_s *led, void *p)
{
	struct rend_pl_s *r_p = p;
	SDL_Renderer *renderer = r_p->renderer;
	struct plane *pl = r_p->pl;
	double angle = r_p->angle;

	// int x = (pl->w * (angle / TAU)) % pl->w;
	int x = fmod((pl->w * (angle / TAU)), pl->w);
	int y = (led->r - 130) / 2;

	// fprintf(stderr, "%d, x: %d, y: %d\n", __LINE__, x, y);
	if (pl->pixel[y * pl->w + x] > 0) {
		filledCircleColor(renderer,
				  led->currpo.x, led->currpo.y, led->w, 0xAAAAAAFF);
	} else {
		filledCircleColor(renderer,
				  led->currpo.x, led->currpo.y, led->w, 0x3A003AFF);
	}
}

void run_led(SDL_Renderer *renderer, struct led_s *led, struct plane *pl)
{
	int elapsed_ms = mstime() - led->start_ms;
	double angle = TAU * elapsed_ms / led->period + led->start_angle;

	int i;
	double tmp_angle;
	struct rend_pl_s r_p = {renderer, pl, 0};
	for (i = 0; i < N; i++) {
		tmp_angle = angle + i * TAU / N;
		led->currpo.x = led->center.x + led->r * cos(tmp_angle);
		led->currpo.y = led->center.y + led->r * sin(tmp_angle);
	// 	filledCircleColor(renderer, led->currpo.x, led->currpo.y,
	// 			  led->w, led->color);
		r_p.angle = tmp_angle;
		if (led->cb)
			led->cb(led, &r_p);
	}
}

static const char h_font[] = {
	________,
	________,
	XXX_____,
	_XX_____,
	_XX_____,
	_XX_XX__,
	_XXX_XX_,
	_XX__XX_,
	_XX__XX_,
	_XX__XX_,
	_XX__XX_,
	XXX__XX_,
	________,
	________,
	________,
	________,
};

void dump_plane(SDL_Renderer *renderer, const struct plane *pl)
{
	int i, j;
	for (i = 0; i < pl->h; i++) {
		for (j = 0; j < pl->w; j++) {
			if (pl->pixel[i * pl->w + j] > 0) {
				// filledCircleColor(renderer, j, i, 1, 0xAAAAAAFF);
				pixelColor(renderer, j, i, 0xAAAAAAFF);
			} else {
				// filledCircleColor(renderer, j, i, 1, 0x0);
				pixelColor(renderer, j, i, 0x3AF003FF);
			}
		}
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
	int w = 640, h = 480;
	struct plane *pl = create_plane(w, 16);
	struct font_data_s font;
	font.h = 16, font.w = 8;
	font.data = malloc(16);
	memcpy(font.data, h_font, 16);
	(void)plane_add_font(pl, 0, 0, &font);

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "SDL_Init() failed: %s\n", SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);
	screen = SDL_CreateWindow("led fan",
				  SDL_WINDOWPOS_UNDEFINED,
				  SDL_WINDOWPOS_UNDEFINED,
				  w, h, SDL_WINDOW_SHOWN);
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
		led[i] = create_led(w/2, h/2, 130 + i*2, 1, 0xFFFFFFFF, 0, 551,
				    NULL);
		led[i]->cb = disp_font;
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
			run_led(renderer, led[i], pl);
		}
		// dump_plane(renderer, pl);
		SDL_RenderPresent(renderer);
		/* Adjust framerate */
		SDL_framerateDelay(&fps_mgr);
		boxRGBA(renderer, 0, 0, w - 1, h - 1, 0, 0, 0, 255);
	}
	return 0;
}
