#pragma once

#include <dill/core.h>

// gate handles

handle flux_mgate(void);

int flux_gatehold(handle gate, int64_t deadline);
int flux_gateopen(handle gate);

int flux_gateisheld(handle gate);
