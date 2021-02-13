#include "dotfont.h"
#include "encoding_convert.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>

/*
 * const int ASCII_W = 8;
 * const int ASCII_H = 16;
 * static const unsigned char __font_bitmap_8x16__[]
 */

static uint32_t
gb2312code_to_fontoffset(uint32_t gb2312code, size_t font_h, size_t font_w)
{
	uint32_t fontoffset;

	fontoffset = (gb2312code % 0x100 - 0xA1) * 94
		     + (gb2312code / 0x100 - 0xA1);
	fontoffset *= font_h * font_w / 8;
	return fontoffset;
}

#ifdef __APPLE__
static uint8_t *MEM_GB2312_FONT;
#else
extern uint8_t MEM_GB2312_FONT[] asm("_binary_gb2312_hzk_start");
#endif

uint8_t *
get_gb2312font(const uint8_t *fontdata, size_t font_h, size_t font_w,
	       uint32_t gb2312code, uint8_t *buf)
{
	uint32_t offset = gb2312code_to_fontoffset(gb2312code, font_h, font_w);
	memcpy(buf, fontdata + offset, font_h * font_w / 8);
	return buf;
}

#ifdef __APPLE__
uint8_t *
init_fontdata(const char *fontpath)
{
	int fd = open(fontpath, O_RDONLY);
	if (fd < 0) {
		perror("open");
		return NULL;
	}
	struct stat fd_stat;
	int ret = fstat(fd, &fd_stat);
	if (ret == -1) {
		perror("fstat");
		return NULL;
	}
	uint8_t *fontdata_start;
	fontdata_start = mmap(NULL, (size_t)fd_stat.st_size,
			      PROT_READ, MAP_PRIVATE, fd, (off_t)0);
	close(fd);
	return fontdata_start;
}
#endif

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
create_utf8_16x16font(const uint8_t *utf8)
{
	struct font_data_s *font = malloc(sizeof(*font));
	uint16_t gb2312;
	assert(font);
	font->h = 16;
	font->w = 16;
	font->type = UTF8;
	memcpy(font->utf8, utf8, 4);
	font->data = malloc(16 * 2);
	assert(font->data);
	gb2312 = get_gb2312_by_utf8(utf8);
#ifdef __APPLE__
	if (MEM_GB2312_FONT == NULL) {
		MEM_GB2312_FONT = init_fontdata("./gb2312.hzk");
		assert(MEM_GB2312_FONT);
	}
#else
	assert(MEM_GB2312_FONT);
#endif
	get_gb2312font(MEM_GB2312_FONT, font->h, font->w, gb2312, font->data);
	return font;
}

struct font_data_s *
set_font_data(struct font_data_s *font, int h, int w, encode_type_t t,
	      const uint8_t *v)
{
	/* TODO */
	return NULL;
}
