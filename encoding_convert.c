#include "encoding_convert.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifndef MAX_LINE
#define MAX_LINE	1024
#endif

#ifndef GB2312_MEM_SIZE
#define GB2312_MEM_SIZE		(7580 * 4)
#endif

int
get_utf8_length(const uint8_t *src)
{
	switch (*src) {
	case 0x0 ... 0x7f:
		return 1;
	case 0xC0 ... 0xDF:
		return 2;
	case 0xE0 ... 0xEF:
		return 3;
	case 0xF0 ... 0xF7:
		return 4;
	default:
		return -1;
	}
}

static int
utf8tounicode(const uint8_t *src, uint8_t *dst)
{
	int length;
	uint8_t unicode[2] = {0}; /* 小端序 */

	length = get_utf8_length(src);
	if (length < 0)
		return -1;
	switch (length) {
	case 1:
		*dst = *src;
		*(dst + 1) = 0;
		return 1;
		break;
	case 2:
		unicode[0] = *(src + 1) & 0x3f;
		unicode[0] += (*src & 0x3) << 6;
		unicode[1] = (*src & 0x7 << 2) >> 2;
		break;
	case 3:
		unicode[0] = *(src + 2) & 0x3f;
		unicode[0] += (*(src + 1) & 0x3) << 6;
		unicode[1] = (*(src + 1) & 0xF << 2) >> 2;
		unicode[1] += (*src & 0xf) << 4;
		break;
	case 4:
		/* not support now */
		return -1;
	}
	*dst = unicode[0];
	*(dst + 1) = unicode[1];
	return length;
}

static uint16_t
unicode_to_gb2312(uint16_t unicode, const uint16_t *mem_gb2312, int gb2312_num)
{
	int i;

	for (i = 0; i < gb2312_num; i++)
		if (mem_gb2312[2 * i] == unicode)
			return mem_gb2312[2 * i + 1];
	return -1;
}

static uint16_t *MEM_GB2312;
static int GB2312_NUM;

static inline uint8_t
hex_ch_to_val(char hex_ch)
{
	if (hex_ch >= '0' && hex_ch <= '9')
		return hex_ch - '0';
	else if (hex_ch >= 'A' && hex_ch <= 'F')
		return hex_ch - 'A' + 10;
	else if (hex_ch >= 'a' && hex_ch <= 'f')
		return hex_ch - 'a' + 10;
	return -1;
}

static uint16_t *
mem_gb2312(const char *gb2312filename, int *gb2312_num)
{
	FILE *gb2312_fp;
	uint16_t *ptrmem;
	char *ptrch;
	char buf[MAX_LINE];
	int i;

	gb2312_fp = fopen(gb2312filename, "r");
	if (gb2312_fp == NULL)
		return NULL;
	ptrmem = malloc(GB2312_MEM_SIZE);
	if (!ptrmem) {
		perror("malloc");
		exit(1);
	}
	memset(ptrmem, 0, GB2312_MEM_SIZE);
	i = 0;
	while (fgets(buf, MAX_LINE, gb2312_fp) != NULL) {
		if (strstr(buf, "/x") == NULL)
			continue;

		/* unicode */
		ptrch = strchr(buf, 'U');
		ptrch++;
		*(ptrmem + i * 2) =  hex_ch_to_val(ptrch[0]) * 0x1000
				    + hex_ch_to_val(ptrch[1]) * 0x100
				    + hex_ch_to_val(ptrch[2]) * 0x10
				    + hex_ch_to_val(ptrch[3]);

		/* gb2312 */
		ptrch = strstr(ptrch, "/x");
		ptrch += 2;
		if (ptrch[2] != '/') { /* 单字节 */
			*(ptrmem + i * 2 + 1) = hex_ch_to_val(ptrch[1])
						+ hex_ch_to_val(ptrch[0])*0x10;
		} else { /* 两个字节 */
			*(ptrmem + i * 2 + 1) = hex_ch_to_val(ptrch[5]) * 0x100
						+ hex_ch_to_val(ptrch[4])*0x1000
						+ hex_ch_to_val(ptrch[1])
						+ hex_ch_to_val(ptrch[0])*0x10;
		}
		i++;
	} /* i should be 7573 */
	*gb2312_num = i;

	fclose(gb2312_fp);
	return ptrmem;
}

uint16_t
get_gb2312_by_utf8(const uint8_t *utf8)
{
	uint8_t unicode[2] = {0};
	int ret;
	if (MEM_GB2312 == NULL) {
		MEM_GB2312 = mem_gb2312("./GB2312", &GB2312_NUM);
		if (MEM_GB2312 == NULL) {
			fprintf(stderr, "mem_gb2312() failed!\n");
			exit(1);
		}
	}
	ret = utf8tounicode(utf8, unicode);
	assert(ret > 0);
	return unicode_to_gb2312(unicode[0] + unicode[1] * 0x100,
				 MEM_GB2312, GB2312_NUM);
}
