#pragma once

#include <stdint.h>
#include <stddef.h>

#define flux_bufend(in, end) flux_bufindex(in, end, 0)
#define flux_buferror(in, end, err) flux_bufwrite(in, end, strerror_r(err), 512)
#define flux_bufzero(in, end) flux_bufset((uint8_t *)(in), (uint8_t *)(end), 0)

uint8_t * flux_bufindexnull(const uint8_t * in, const uint8_t * end, uint8_t ch);
uint8_t * flux_bufindex(const uint8_t * in, const uint8_t * end, uint8_t ch);

int flux_bufcmp(const uint8_t * a, const uint8_t * aend, const uint8_t * b, const uint8_t * bend);
int flux_bufeq(const uint8_t * a, const uint8_t * aend, const uint8_t * str, uint8_t ** out);

uint8_t * flux_bufcpy(uint8_t * dst, const uint8_t * dstend, const uint8_t * src, const uint8_t * srcend);
uint8_t * flux_buflcpy(uint8_t * dst, const uint8_t * dstend, const uint8_t * src, const uint8_t * srcend);

uint8_t * flux_bufwrite(uint8_t * dst, const uint8_t * dstend, const uint8_t * src, size_t sz);
uint8_t * flux_buflwrite(uint8_t * dst, const uint8_t * dstend, const uint8_t * src, size_t sz);

void flux_bufset(uint8_t * dst, const uint8_t * dstend, uint8_t val);

int flux_str_parse_ctl(const char * data, char * buf, uint32_t sz, uint32_t bsz, char **V);

#if !defined FLUX_DISABLE_RAW_NAMES
#define bufend flux_bufend
#define buferror flux_buferror
#define bufindexnull flux_bufindexnull
#define bufindex flux_bufindex
#define bufcmp flux_bufcmp
#define bufeq flux_bufeq
#define bufcpy flux_bufcpy
#define buflcpy flux_buflcpy
#define bufwrite flux_bufwrite
#define buflwrite flux_buflwrite
#define str_parse_ctl flux_str_parse_ctl
#define bufset flux_bufset
#define bufzero flux_bufzero
#endif
