#include <flux/str.h>
#include <snow.h>

#define SZ (1 << 15)

describe(str) {
	uint8_t buffer[SZ], *p, *q;

	flux_buflwrite(buffer, buffer + SZ, "This is a testing test", 64);

	asserteq_str(buffer, "This is a testing test", "buflwrite()");

	asserteq(flux_buflwrite(NULL, NULL, "LOL WTF", 3), NULL, "Bail early");

	p = "WTF";

	asserteq(flux_bufend(p, p + 1), NULL, "bufend() overflow check (1)");
	asserteq(flux_bufend(p, p    ), NULL, "bufend() overflow check (0)");
	asserteq(flux_bufend(p, p + 3), p + 3, "bufend() overflow check (3)");
	asserteq(flux_bufend(p, p + 8), p + 3, "bufend() overflow check (8)");

	q = "FOO";

	assertneq(flux_bufcmp(p, p + 3, q, q + 3), 0, "FOO == WTF");
	asserteq(flux_bufcmp(p, p + 3, q, q), -257, "Short cmp");
}

snow_main();
