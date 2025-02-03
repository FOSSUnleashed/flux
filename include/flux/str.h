#pragma once

#include <stdint.h>
#include <stddef.h>

#define flux_bufend(in, end) flux_bufindex(in, end, 0)
#define flux_buferror(in, end, err) flux_bufwrite(in, end, strerror_r(err), 512)

uint8_t * flux_bufindexnull(const uint8_t * in, const uint8_t * end, uint8_t ch);
uint8_t * flux_bufindex(const uint8_t * in, const uint8_t * end, uint8_t ch);

int flux_bufcmp(const uint8_t * a, const uint8_t * aend, const uint8_t * b, const uint8_t * bend);

uint8_t * flux_bufcpy(uint8_t * dst, const uint8_t * dstend, const uint8_t * src, const uint8_t * srcend);
uint8_t * flux_buflcpy(uint8_t * dst, const uint8_t * dstend, const uint8_t * src, const uint8_t * srcend);

uint8_t * flux_bufwrite(uint8_t * dst, const uint8_t * dstend, const uint8_t * src, size_t sz);
uint8_t * flux_buflwrite(uint8_t * dst, const uint8_t * dstend, const uint8_t * src, size_t sz);

int flux_str_parse_ctl(const char * data, char * buf, uint32_t sz, uint32_t bsz, char **V);
