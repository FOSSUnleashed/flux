#pragma once

#include <stdint.h>

typedef uint64_t Time;

int64_t flux_parsetime(const char * const s);

// flux_time
uint64_t flux_ms();
uint64_t flux_us();
uint64_t flux_s();

uint32_t flux_sit();
//uint32_t flux_sit_from(time_t);

uint8_t flux_time_gen8();
uint16_t flux_time_gen16();
uint32_t flux_time_gen32();
uint64_t flux_time_gen64();
