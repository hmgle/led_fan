#ifndef _ENCODING_CONVERT_H
#define _ENCODING_CONVERT_H

#include <stdint.h>
#include <stdio.h>

int get_utf8_length(const uint8_t *src);
uint16_t get_gb2312_by_utf8(const uint8_t *utf8);

#endif
