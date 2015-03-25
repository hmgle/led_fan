#include "nyancat.h"
#include "plane.h"

#include <assert.h>

typedef uint32_t Uint32;

static Uint32 nyancat_colors[256] = {
	[',']  = 0xFFFF0000,	/* Blue background */
	['.']  = 0xFFFFFFFF,	/* White stars */
	['\''] = 0xFF000000,	/* Black border */
	['@']  = 0xFFFFFFFF,	/* Tan poptart */
	['$']  = 0xFF0033AA,	/* Pink poptart */
	['-']  = 0xFF0000FF,	/* Red poptart */
	['>']  = 0xFF0000FF,	/* Red rainbow */
	['&']  = 0xFF0099FF,	/* Orange rainbow */
	['+']  = 0xFF00FFFF,	/* Yellow Rainbow */
	['#']  = 0xFF00FF00,	/* Green rainbow */
	['=']  = 0xAAAA0000,	/* Light blue rainbow */
	[';']  = 0xFFFF9900,	/* Dark blue rainbow */
	['*']  = 0x80808080,	/* Gray cat face */
	['%']  = 0xFF0033AA,	/* Pink cheeks */
};

struct plane *set_nyancat_plane(struct plane *p, int x, const char **frame)
{
	assert(p);
	assert(p->w >= FRAME_WIDTH && p->h == FRAME_HEIGHT);
	int i, j;
	char color;
	for (i = 0; i < FRAME_HEIGHT; i++) {
		for (j = 0; j < FRAME_WIDTH; j++) {
			color = frame[i][j];
			(p->pixel)[(j+x)*(p->h) + i] = nyancat_colors[(int)color];
		}
	}
	return p;
}
