#include <snow.h>
#include <flux/time.h>

#define DO(str, res) asserteq(flux_parsetime(str), res, str)

describe(time) {
	it("time") {
		DO("555", 555);
		DO("1m40", 100);
		DO("1h40m", 6000);
		DO("h", 0); // TODO: this should be an error
		DO("10d", 36000 * 24);
		DO("10x", -3);
		asserteq(flux_parsetime(NULL), -1, "NULL");
	}
}
