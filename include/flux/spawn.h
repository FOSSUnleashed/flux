#pragma once

#include <dill/core.h>

int flux_mspawn(const char ** argv, const char ** env, dill_handle *hout);
int flux_bspawn(const char ** argv, const char ** env, dill_handle *hout);

#if !defined FLUX_DISABLE_RAW_NAMES
#define mspawn flux_mspawn
#define bspawn flux_bspawn
#endif
