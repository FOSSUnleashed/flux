#pragma once

//#include <flux/factotum.h>
#include <cfg.h>
#include <dill/core.h>
#include <flux/list.h>

#define TWITCH_MAX_SIZE (1 << 7)

typedef struct {
	char clientId[TWITCH_MAX_SIZE], clientSecret[TWITCH_MAX_SIZE], userToken[TWITCH_MAX_SIZE], appToken[TWITCH_MAX_SIZE], userId[TWITCH_MAX_SIZE];
	char host[TWITCH_MAX_SIZE], base[TWITCH_MAX_SIZE];
	Config cfg;
	struct ipaddr addr;
	handle h;
	Bit ssl : 1;
} Twitch;

Twitch *flux_twitch_create();
Twitch *flux_twitch_mock_create(const char * host, uint32_t port);

// GET
// POST
// PATCH
// PUT
// DELETE
