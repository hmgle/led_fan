#include "plane.h"
#include <stdlib.h>

struct plane *create_plane(int w, int h, Uint32 color)
{
	struct plane *p = malloc(sizeof(*p));
	p->w = w;
	p->h = h;
	p->pixel = calloc(1, w * h * sizeof(*p->pixel));
	int i;
	for (i = 0; i < w * h; i++)
		p->pixel[i] = color;
	return p;
}
