#include <snow.h>
#include <stdlib.h>
#include <stdint.h>
#include <new.h>

#define c2(out, src) do { json_buffer_copy(out, src); asserteq_str(out, src); } while (0)
#define c3(out, src, fin) do { json_buffer_copy(out, src); asserteq_str(out, fin); } while (0)

describe(json) {
	char out[1 << 16];

	it("simple") {
		c2(out, "This is a test");
		c2(out, "foobar");
		c2(out, "\\");
		c3(out, "\\n", "\n");
	}
}

snow_main();
