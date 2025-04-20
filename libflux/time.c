#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>

struct delta {
	uint8_t sec, min, hour, days;
};

uint64_t flux_s() {
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return (uint64_t)tv.tv_sec;
}

uint64_t flux_ms() {
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

uint64_t flux_us() {
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

struct delta delta(time_t t) {
	struct delta d;

	d.sec = t % 60;
	t /= 60;
	d.min = t % 60;
	t /= 60;
	d.hour = t % 24;
	t /= 24;
	d.days = t;

	return d;
}

uint32_t flux_sit_from(time_t t) {
	struct tm x;
	uint32_t r;
	gmtime_r(&t, &x);

	r = 3600 * ((x.tm_hour + 1) % 24) + 60 * x.tm_min + x.tm_sec;

	r = r * 1000 / 864;

	return r;
}

uint32_t flux_sit() {
	return flux_sit_from(time(NULL));
}

int64_t flux_parsetime(const char * const s) {
	int64_t res = 0, tmp = 0, i;

	if (!s) {
		return -1;
	}

	for (i = 0; s[i]; ++i) {
		if ('0' <= s[i] && '9' >= s[i]) {
			tmp *= 10;
			tmp += s[i] - '0';
		} else {
			switch (s[i]) {
			case 'm':
				res	+= tmp * 60;
				tmp	= 0;
				break;
			case 'h':
				res	+= tmp * 60 * 60;
				tmp	= 0;
				break;
			case 'd':
				res	+= tmp * 60 * 60 * 24;
				tmp	= 0;
				break;
			default:
				return -(i + 1);
			}
		}
	}

	if (tmp) {
		res += tmp;
	}

	return res;
}

#define INT(X) uint ## X ## _t
#define GEN(X) \
INT(X) flux_time_gen ## X  () { \
	INT(X) r = 0, bits = 0, tmp; \
\
	while (bits < X) { \
		tmp = (flux_us() == flux_us()) | ((flux_us() == flux_us()) << 1); \
\
		switch (tmp) {\
			case 0:\
			case 3:\
				break;\
			case 1:\
			case 2:\
				r = (r << 1) | (tmp & 1);\
				bits++;\
		}\
	}\
\
	return r;\
}

GEN(8)
GEN(16)
GEN(32)
GEN(64)
