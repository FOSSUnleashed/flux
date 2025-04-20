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

#define TWITCH_USER	0x01
#define TWITCH_APP	0x02

#define TWITCH_API \
	X(start_commercial, POST, /channels/commercial, USER, {"channel:edit:commercial"}, broadcaster_id, length)\ // str, int
	X(get_ads, GET, /channels/ads, USER, {"channel:read:ads"}, broadcaster_id)\ // str
	X(snooze_ad, POST, /channels/ads/schedule/snooze, USER, {"channel:manage:ads"}, broadcaster_id)\ // str
	X(get_bits_leaderboard, GET, /bits/leaderboard, USER, {"bits:read"}, count, period, started_at, user_id)\ // ?int, ?str, ?str, ?str

