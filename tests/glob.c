#include <snow.h>
#include <string.h>

enum GlobType {
	GLOB_DEAD
	,GLOB_PREFIX // prefix*
	,GLOB_SUFFIX // *suffix
	,GLOB_MIXED // prefix*suffix
	,GLOB_MULTI // one*two*three
};

// TODO: handle multi

typedef struct glob Glob;
struct glob {
	char * pattern;
	int type, len, plen, slen, cnt;
};

int glob_init(Glob * g, const char * pat);
void glob_free(Glob * g);
int glob_match(Glob * g, const char * const str);

//
// Tests
//

#define GLOB_TEST(v, str) asserteq(glob_match(&g, str), v, str)

describe(glob) {
	Glob g;

	it("*test") {
		asserteq(glob_init(&g, "*test"), GLOB_SUFFIX);

		asserteq(g.plen, 0);
		asserteq(g.slen, 4);

		GLOB_TEST(1, "test");
		GLOB_TEST(0, "");
		GLOB_TEST(0, "foo");
		GLOB_TEST(1, "a test");
		glob_free(&g);
	}

	it("pre*") {
		asserteq(glob_init(&g, "pre*"), GLOB_PREFIX);

		asserteq(g.plen, 3);
		asserteq(g.slen, 0);

		GLOB_TEST(1, "prediction");
		GLOB_TEST(0, "");
		GLOB_TEST(0, "foo");
		GLOB_TEST(1, "pre");
		glob_free(&g);
	}

	it("mixed*media") {
		asserteq(glob_init(&g, "mixed*media"), GLOB_MIXED);

		asserteq(g.plen, 5);
		asserteq(g.slen, 5);

		GLOB_TEST(1, "mixedmedia");
		GLOB_TEST(0, "");
		GLOB_TEST(0, "foo");
		GLOB_TEST(1, "mixed coats are not in the media");

		glob_free(&g);
	}

	it("multi*meter*") {
		asserteq(glob_init(&g, "multi*meter*"), GLOB_MULTI);
		glob_free(&g);
	}

	it("pre?") {
		asserteq(glob_init(&g, "pre?"), GLOB_PREFIX);

		asserteq(g.plen, 3);
		asserteq(g.slen, 0);

		GLOB_TEST(0, "prediction");
		GLOB_TEST(0, "");
		GLOB_TEST(0, "foo");
		GLOB_TEST(0, "pre");
		GLOB_TEST(1, "pret");
		GLOB_TEST(1, "pre?");

		glob_free(&g);
	}

	it("?fix") {
		asserteq(glob_init(&g, "?fix"), GLOB_SUFFIX);

		asserteq(g.plen, 0);
		asserteq(g.slen, 3);

		GLOB_TEST(1, "afix");
		GLOB_TEST(0, "");
		GLOB_TEST(0, "foo");
		GLOB_TEST(0, "fix");
		GLOB_TEST(0, "suffix");
		GLOB_TEST(1, "?fix");
		glob_free(&g);
	}

	it("question?mark") {
		asserteq(glob_init(&g, "question?mark"), GLOB_MIXED);

		asserteq(g.plen, 8);
		asserteq(g.slen, 4);

		GLOB_TEST(0, "questionmark");
		GLOB_TEST(0, "");
		GLOB_TEST(0, "foo");
		GLOB_TEST(0, "questionable mark");
		GLOB_TEST(1, "question mark");
		GLOB_TEST(1, "question?mark");
		glob_free(&g);
	}
}

snow_main();
