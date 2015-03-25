#ifndef _PLANE_H
#define _PLANE_H

#include <stdint.h>

typedef uint32_t Uint32;

struct plane {
	int w;
	int h;
	Uint32 *pixel;
};

struct plane *create_plane(int w, int h, Uint32 color);

#endif
