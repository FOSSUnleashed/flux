#include <flux/str.h>
#include <snow.h>
#include <stdio.h>

#define SZ (1 << 15)

Buffer gBuf = BUFLIT("This is a test");

#define BUF(s, e) (Buffer){.start = s, .end = e}

describe(str) {
	uint8_t buffer[SZ], *p, *q, *qe;
	int16_t r;
	Buffer s, t;

	t = BUFLIT(buffer);

	it ("sanity") {
		s.start = gBuf.start;
		s.end	= gBuf.start + 512;

		asserteq(gBuf.end, flux_bufend(s), "BUFLIT macro works correctly");
	}

	it ("???") {
#if 0
		flux_buflwrite(buffer, buffer + SZ, "This is a testing test", 64);

		asserteq_str(buffer, "This is a testing test", "buflwrite()");

		asserteq(flux_buflwrite(NULL, NULL, "LOL WTF", 3), NULL, "Bail early");
#endif

		p = "WTF";

		asserteq(flux_bufend(BUF(p, p + 1)), NULL, "bufend() overflow check (1)");
		asserteq(flux_bufend(BUF(p, p    )), NULL, "bufend() overflow check (0)");
		asserteq(flux_bufend(BUF(p, p + 4)), p + 3, "bufend() overflow check (4)");
		asserteq(flux_bufend(BUF(p, p + 8)), p + 3, "bufend() overflow check (8)");

		q = "FOO";
		s = BUF(p, p + 3);

		assertneq(flux_bufcmp(s, BUF(q, q + 3), NULL), 0, "FOO == WTF");
		asserteq(flux_bufcmp(s, BUF(q, q), NULL), -257, "Short cmp");
	}


	it ("bufeq") {
		q = "FOO";

		asserteq(r = flux_bufeq(BUF(q, q + 3), BUFLIT("FOOBAR"), NULL), -257, "eq too long");

		q = "FOOBAR";

		asserteq(flux_bufeq(BUF(q, q + 7), BUFLIT("FOOBAR"), NULL), 0, "eq sized correctly");
	}

	it ("bufeqp") {
		s = BUFLIT("This is a testing test");

		asserteq(flux_bufeq(s, BUFLIT("This is a "), &p), 0, "short eq");
		asserteq(p - s.start, 9, "Result point at incorrect location");
	}

	it ("lcpy") {
		s = BUFLIT("This is a test");

		s = flux_buflcpy(t, s);
		assert(s.end > s.start, "End is after start");
		assertneq(s.start, s.end, "Result buffer has a length");
		asserteq(s.start, t.start, "Result buffer is pointing to correct address");
		asserteq_str(s.start, buffer);
		asserteq(s.end, buffer + 14, "Return pointer at correct position");
	}

	it ("bufeq p player") {
		s = BUFLIT("p");

		assertneq(flux_bufeq(s, BUFLIT("player"), NULL), 0);
	}
}
