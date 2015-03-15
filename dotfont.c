#include "dotfont.h"

#include <string.h>

/*
 * const int ASCII_W = 8;
 * const int ASCII_H = 16;
 * static const unsigned char __font_bitmap_8x16__[]
 */

static uint8_t *
get_ascii_8x16font(uint8_t c, uint8_t *buf)
{
	memcpy(buf, &__font_bitmap_8x16__[c * ASCII_H], ASCII_H);
	return buf;
}

struct font_data_s *
create_ascii_8x16font(uint8_t c)
{
	struct font_data_s *font = malloc(sizeof(*font));
	assert(font);
	font->h = ASCII_H;
	font->w = ASCII_W;
	font->type = ASCII;
	font->ascii = c;
	font->data = malloc(ASCII_H);
	assert(font->data);
	(void)get_ascii_8x16font(c, font->data);
	return font;
}

struct font_data_s *
set_font_data(struct font_data_s *font, int h, int w, encode_type_t t,
	      const uint8_t *v)
{
	/* TODO */
	return NULL;
}
