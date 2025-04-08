#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct {
	uint8_t *start, *end;
} FluxBuffer;

#define FLUX_BUFLIT(s) ((FluxBuffer){.start = s, .end = s - 1 + sizeof(s)})
#define flux_isbadbuffer(b) (NULL == b.start || NULL == b.end || b.end <= b.start)
#define flux_bufcontinue(out, dst) ((FluxBuffer){out.end, dst.end})

#define flux_string2buffer(s, sz) ((FluxBuffer){.start = s, .end = flux_bufindex((FluxBuffer){.start = s, .end = s + sz}, 0)})

#define flux_bufend(b) flux_bufindex((b), 0)
#define flux_buferror(b, err) flux_buflcpy(b, flux_string2buffer(strerror_r(err), 512))
#define flux_bufzero(b) flux_bufset((b), 0)
#define flux_bufzeromem(s, e) flux_bufset(((Buffer){.start = (uint8_t *)(s), .end = (uint8_t *)(e)}), 0)
#define flux_bufadvance(p, end, cond) for (; p < end && (cond); ++p)

uint8_t * flux_bufindexnull(FluxBuffer b, uint8_t ch);
uint8_t * flux_bufindex(FluxBuffer b, uint8_t ch);

int flux_bufcmp(FluxBuffer a, FluxBuffer b, uint8_t ** out);
int flux_bufeq(FluxBuffer a, FluxBuffer b, uint8_t ** out);

FluxBuffer flux_bufcpy(FluxBuffer dst, FluxBuffer src);
FluxBuffer flux_buflcpy(FluxBuffer dst, FluxBuffer src);

void flux_bufset(FluxBuffer dst, uint8_t val);

uint8_t *flux_bufdump(uint8_t *out, uint8_t *eout, const uint8_t *in, const uint8_t *ein);

int flux_str_parse_ctl(const char * data, char * buf, uint32_t sz, uint32_t bsz, char **V); // Do we even still need this?

// bufprintf?

FluxBuffer flux_envbuf(const char * env);

#if !defined FLUX_DISABLE_RAW_NAMES
#define BUFLIT FLUX_BUFLIT
#define Buffer FluxBuffer
#define bufend flux_bufend
#define envbuf flux_envbuf
#define bufcontinue flux_bufcontinue
#define isbadbuffer flux_isbadbuffer
#define string2buffer flux_string2buffer
#define buferror flux_buferror
#define bufindexnull flux_bufindexnull
#define bufindex flux_bufindex
#define bufcmp flux_bufcmp
#define bufeq flux_bufeq
#define bufcpy flux_bufcpy
#define buflcpy flux_buflcpy
#define str_parse_ctl flux_str_parse_ctl
#define bufset flux_bufset
#define bufzero flux_bufzero
#define bufzeromem flux_bufzeromem
#define bufdump flux_bufdump
#endif
