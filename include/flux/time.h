#pragma once

#include <stdint.h>

int64_t flux_parsetime(const char * const s);

// flux_time
uint64_t flux_ms();
uint64_t flux_us();
uint64_t flux_s();

uint32_t flux_sit();
//uint32_t flux_sit_from(time_t);

