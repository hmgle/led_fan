#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL2_framerate.h>
#include <sys/time.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>

#include "dotfont.h"
#include "plane.h"
#include "nyancat.h"
#include "encoding_convert.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#ifndef PI
#define PI M_PI
#endif

#define TAU 6.28318530717958647693

#define PI_3_2 4.71238898038468985769 /* 1.5 x PI */

#define N 227.0 /* 扇页数 */
#define DN 256.0 /* 圆周像素数 */

#define BIT(nr) (1 << (nr))

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

int plane_add_font(struct plane *p, int x, int y, struct font_data_s *f)
{
	int i, j;
	uint8_t *data = f->data;
	assert(f->h == p->h);
	for (i = 0; i < f->h; i++)
		for (j = 0; j < f->w; j++)
			if ((data[i*f->w/8 + j/8] & (0x1 << (7 - j%8))) > 0)
				(p->pixel)[(j+x)*(p->h) + i] = 0xFFFFFFFF;
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
	double angle = r_p->angle + PI_3_2 - led->start_angle;

	int x = fmod((pl->w * (angle / TAU)), pl->w);
	int y = (70+16*4-0.001 - led->r) / 4;

	filledCircleColor(renderer, led->currpo.x, led->currpo.y,
			  led->w, pl->pixel[x*pl->h + y]);
}

void dump_plane(SDL_Renderer *renderer, const struct plane *pl);
static struct plane *nyancat_pl[12];

void cat_run_led(SDL_Renderer *renderer, struct led_s *led)
{
	int elapsed_ms = mstime() - led->start_ms;
	double angle = TAU * elapsed_ms / led->period + led->start_angle;

	int i;
	double tmp_angle;
	i = elapsed_ms / led->period;
	struct plane *pl = nyancat_pl[i % 12];
	struct rend_pl_s r_p = {renderer, pl, 0};
	for (i = 0; i < N; i++) {
		tmp_angle = angle + i * TAU / N;
		led->currpo.x = led->center.x + led->r * cos(tmp_angle);
		led->currpo.y = led->center.y + led->r * sin(tmp_angle);
		r_p.angle = tmp_angle;
		if (led->cb)
			led->cb(led, &r_p);
	}
	dump_plane(renderer, pl);
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
		r_p.angle = tmp_angle;
		if (led->cb)
			led->cb(led, &r_p);
	}
}

static Uint32 get_plane_bit(const struct plane *pl, int x, int y)
{
	int w = pl->w;
	int h = pl->h;
	assert(w > 0 && h > 0 && x >= 0 && x < w && y >= 0 && y < h);
	assert(h % 8 == 0);
	return pl->pixel[x*h + y];
}

void dump_plane(SDL_Renderer *renderer, const struct plane *pl)
{
	int i, j;
	for (i = 0; i < pl->h; i++)
		for (j = 0; j < pl->w; j++)
			pixelColor(renderer, j, i, get_plane_bit(pl, j, i));
}

static int get_string_len(const uint8_t *string)
{
	const uint8_t *p = string;
	int count = 0;

	while (*p) {
		count++;
		p += get_utf8_length(p);
	}
	return count;
}

static void usage(char **argv)
{
	fprintf(stderr, "Usage: %s [Options] [string]\n"
			"\n"
			"Options:\n"
			" -f --fps  <fps_val>            Frame per Second\n"
			" -T --period  <Millisecond>     Set Period\n"
			" -W --width   <pix>             Set Width\n"
			" -H --height  <pix>             Set Height\n"
			" -c --bgcolor <color>           Set background color\n"
			"\n", argv[0]);
}

int main(int argc, char **argv)
{
	SDL_Window *screen;
	SDL_Renderer *renderer;
	SDL_Event event;
	FPSmanager fps_mgr;
	Uint32 fps_rate = 200;
	struct led_s *led[16];
	int i;
	int w = 640, h = 480;
	struct plane *pl;
	int font_num = 0;
	struct font_data_s *font[32];
	int is_cat = 0;
	double start_angle = M_PI_2;
	Uint32 bg_color = 0x3F00FF00;
	int period = 551; /* 旋转周期: ms */
	int opt, index;

	static struct option long_opts[] = {
		{"help",	no_argument,       0, 'h'},
		{"fps",		required_argument, 0, 'f'},
		{"period",	required_argument, 0, 'T'},
		{"width",	required_argument, 0, 'W'},
		{"height",	required_argument, 0, 'H'},
		{"bgcolor",	required_argument, 0, 'c'},
		{0, 0, 0, 0}
	};

	while ((opt = getopt_long(argc, argv, "hf:T:W:H:c:",
				  long_opts, &index)) != -1) {
		switch (opt) {
		case 'f':
			fps_rate = atoi(optarg);
			break;
		case 'T':
			period = atoi(optarg);
			break;
		case 'W':
			w = atoi(optarg);
			break;
		case 'H':
			h = atoi(optarg);
			break;
		case 0:
		case 'h':
		default:
			usage(argv);
			exit(0);
		}
	}
	pl = create_plane(w/2, 16, bg_color);
	if (argc > optind) {
		const uint8_t *font_p = (const uint8_t *)argv[optind];
		start_angle =
			(1-(get_string_len((const uint8_t *)argv[optind])-0.5)/20.0) * PI;
		while (*font_p) {
			if (*font_p < 0x80) {
				/* ascii */
				font[font_num] = create_ascii_8x16font(*font_p);
				(void)plane_add_font(pl, font_num * 16, 0,
						     font[font_num]);
				font_num++;
				font_p++;
			} else if (*font_p > 0xA0 && *font_p < 0xFF) {
				/* utf-8 */
				font[font_num] =
					create_utf8_16x16font((const uint8_t *)font_p);
				(void)plane_add_font(pl, font_num * 16, 0,
						     font[font_num]);
				font_num++;
				font_p += get_utf8_length(font_p);
			}
		}
	} else {
		is_cat = 1;
		for (i = 0; i < 12; i++) {
			nyancat_pl[i] = create_plane(w/2, 16, 0xFFFF0000);
			(void)set_nyancat_plane(nyancat_pl[i], 48, cat_frames[i]);
		}
	}

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

	if (is_cat)
		period = 97;
	for (i = 0; i < ARRAY_SIZE(led); i++) {
		led[i] = create_led(w/2, h/2, 70 + i*4, 1, 0xFFFFFFFF,
				    start_angle, period, disp_font);
	}
	SDL_setFramerate(&fps_mgr, fps_rate);
	while (1) {
		while (SDL_PollEvent(&event) != 0) {
			switch (event.type) {
			case SDL_KEYDOWN:
			case SDL_QUIT:
				return 0;
				break;
			}
		}
		if (is_cat) {
			for (i = 0; i < ARRAY_SIZE(led); i++) {
				cat_run_led(renderer, led[i]);
			}
		} else {
			for (i = 0; i < ARRAY_SIZE(led); i++) {
				run_led(renderer, led[i], pl);
			}
		}
		// dump_plane(renderer, pl);
		SDL_RenderPresent(renderer);
		/* Adjust framerate */
		SDL_framerateDelay(&fps_mgr);
		boxRGBA(renderer, 0, 0, w - 1, h - 1, 0, 0, 0, 255);
	}
	return 0;
}
