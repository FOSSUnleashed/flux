#include <stdio.h>
#include <flux/ulid.h>
#include <flux/time.h>

// TODO: move this into the library
uint32_t gen() {
	uint32_t r = 0, bits = 0, tmp;

	while (bits < 32) {
		tmp = (flux_us() == flux_us()) | ((flux_us() == flux_us()) << 1);

		switch (tmp) {
			case 0:
			case 3:
				break;
			case 1:
			case 2:
				r = (r << 1) | (tmp & 1);
				bits++;
		}
	}

	return r;
}

int main(int argc, char **argv) {
	flux_isaac32_ctx ctx;
	uint8_t b[64], *be = b + 64;
	int i;

	for (i = 0; i < FLUX_ISAAC_RANDSIZ; i++) {
		ctx.randrsl[i] = gen();
	}

	isaac32_init(&ctx, 1);

	be = flux_ulid32(b, be, &ctx);

	if (be) {
		*be = 0;
		printf("%s\n", b);
	}

	return 0;
}
