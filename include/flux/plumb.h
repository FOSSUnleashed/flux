#pragma once

#include <stdint.h>

typedef struct {
	uint8_t *src, *dst, *cwd, *type, *attr, *size, *data;
	uint8_t *esrc, *edst, *ecwd, *etype, *eattr, *edata; // end pointers
	uint32_t sz;
} pmsg;

// -1 == error; 0 == success; 1 == another buffer in queue
int parse_pmsg(pmsg *p, uint8_t * data, uint8_t *data_end);
