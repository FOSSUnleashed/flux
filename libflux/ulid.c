#include <flux/time.h>
#include <flux/ulid.h>

uint8_t * flux_ulid32(uint8_t * out, uint8_t * oend, flux_isaac32_ctx * ctx) {
	uint64_t s	= flux_ms();
	uint32_t r;
	int i;
	uint8_t buffer[16];

	for (i = 0; i < 6; ++i) {
		s = s & 0xFFFFFFFFFFFF;
		buffer[i] = ((s & 0xFF0000000000) >> 40);
		s = s << 8;
	}

	r = rand32(ctx);

	for (; i < 10; ++i) {
		buffer[i] = ((r & 0xFF000000) >> 24);
		r = (r & 0x00FFFFFF) << 8;
	}

	r = rand32(ctx);

	for (; i < 14; ++i) {
		buffer[i] = ((r & 0xFF000000) >> 24);
		r = (r & 0x00FFFFFF) << 8;
	}

	r = rand32(ctx);

	for (; i < 16; ++i) {
		buffer[i] = ((r & 0xFF000000) >> 24);
		r = (r & 0x00FFFFFF) << 8;
	}

	return flux_enbase32(out, oend, buffer, buffer + 16);
}

uint8_t * flux_ulid64(uint8_t * out, uint8_t * oend, flux_isaac64_ctx * ctx) {
	uint64_t s	= flux_ms();
	uint64_t r;
	int i;
	uint8_t buffer[16];

	for (i = 0; i < 6; ++i) {
		s = s & 0xFFFFFFFFFFFF;
		buffer[i] = ((s & 0xFF0000000000) >> 40);
		s = s << 8;
	}

	r = rand64(ctx);

	for (; i < 14; ++i) {
		buffer[i] = ((r & 0xFF00000000000000) >> 56);
		r = (r & 0x00FFFFFFFFFFFFFF) << 8;
	}

	r = rand64(ctx);

	for (; i < 16; ++i) {
		buffer[i] = ((r & 0xFF00000000000000) >> 56);
		r = (r & 0x00FFFFFFFFFFFFFF) << 8;
	}

	return flux_enbase32(out, oend, buffer, buffer + 16);
}
