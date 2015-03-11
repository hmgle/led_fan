#ifndef _DOTFONT_H
#define _DOTFONT_H

#include <stdint.h>

typedef enum {
	UTF8 = 0,
	GBK,
} encode_type_t;

struct font_data_s {
	int h;
	int w;
	encode_type_t type;
	union {
		uint8_t utf8[4];
		uint8_t gbk[2];
	}
	uint8_t *data;
};

struct font_data_s *set_font_data(struct font_data_s *font,
				  int h, int w, encode_type_t t, uint8_t v,
				  const uint8_t *ptrfontdata);

#endif
