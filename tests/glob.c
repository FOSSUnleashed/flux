#include <snow.h>
#include <string.h>
#include <flux/str.h>

enum GlobType {
	GLOB_DEAD
	,GLOB_LITERAL
	,GLOB_WILD
	,GLOB_PREFIX // prefix*
	,GLOB_SUFFIX // *suffix
	,GLOB_MIXED // prefix*suffix
	,GLOB_MULTI // one*two*three
};

// TODO: handle multi

typedef struct glob Glob;
struct glob {
	Buffer *pattern;
	uint16_t plen, slen, cnt; // Prefix Length | Suffix Length | Wildcard Count
	uint16_t min, max; // max = 0 if glob contains *
	enum GlobType type;
};

int glob_init(Glob * g, Buffer pat);
void glob_free(Glob * g);
int glob_match(Glob * g, Buffer str);

//
// Tests
//

#define GLOB_TEST(v, str) asserteq(glob_match(&g, BUFLIT1(str)), v, str)

describe(glob) {
	Glob g;

	it("*") {
		asserteq(glob_init(&g, BUFLIT1("*")), GLOB_WILD);

		asserteq(g.plen, 0);
		asserteq(g.slen, 0);
		asserteq(g.min, 0);
		asserteq(g.max, 0);

		GLOB_TEST(1, "test");
		GLOB_TEST(1, "");
		GLOB_TEST(1, "foo");
		GLOB_TEST(1, "a test");
		glob_free(&g);
	}

	it("?*??") {
		asserteq(glob_init(&g, BUFLIT1("?*??")), GLOB_WILD);

		asserteq(g.plen, 0);
		asserteq(g.slen, 0);
		asserteq(g.max, 0);

		GLOB_TEST(1, "test");
		GLOB_TEST(0, "");
		GLOB_TEST(1, "foo");
		GLOB_TEST(1, "a test");
		glob_free(&g);
	}

	it("???") {
		asserteq(glob_init(&g, BUFLIT1("???")), GLOB_WILD);

		asserteq(g.plen, 0);
		asserteq(g.slen, 0);
		asserteq(g.max, 3);

		GLOB_TEST(0, "test");
		GLOB_TEST(0, "");
		GLOB_TEST(1, "foo");
		GLOB_TEST(0, "atest");
		glob_free(&g);
	}

	it("*test") {
		asserteq(glob_init(&g, BUFLIT1("*test")), GLOB_SUFFIX);

		asserteq(g.plen, 0);
		asserteq(g.slen, 4);
		asserteq(g.max, 0);

		GLOB_TEST(1, "test");
		GLOB_TEST(0, "");
		GLOB_TEST(0, "foo");
		GLOB_TEST(1, "a test");
		glob_free(&g);
	}

	it("pre*") {
		asserteq(glob_init(&g, BUFLIT1("pre*")), GLOB_PREFIX);

		asserteq(g.plen, 3);
		asserteq(g.slen, 0);
		asserteq(g.max, 0);

		GLOB_TEST(1, "prediction");
		GLOB_TEST(0, "");
		GLOB_TEST(0, "foo");
		GLOB_TEST(1, "pre");
		glob_free(&g);
	}

	it("mixed*media") {
		asserteq(glob_init(&g, BUFLIT1("mixed*media")), GLOB_MIXED);

		asserteq(g.plen, 5);
		asserteq(g.slen, 5);
		asserteq(g.max, 0);

		GLOB_TEST(1, "mixedmedia");
		GLOB_TEST(0, "");
		GLOB_TEST(0, "foo");
		GLOB_TEST(1, "mixed coats are not in the media");

		glob_free(&g);
	}
/*
	it("multi*meter*") {
		asserteq(glob_init(&g, BUFLIT1("multi*meter*")), GLOB_MULTI);
		glob_free(&g);
	} // */

	it("pre?") {
		asserteq(glob_init(&g, BUFLIT1("pre?")), GLOB_PREFIX);

		asserteq(g.plen, 3);
		asserteq(g.slen, 0);
		asserteq(g.max, 4);

		GLOB_TEST(0, "prediction");
		GLOB_TEST(0, "");
		GLOB_TEST(0, "foo");
		GLOB_TEST(0, "pre");
		GLOB_TEST(1, "pret");
		GLOB_TEST(1, "pre?");

		glob_free(&g);
	}

	it("?fix") {
		asserteq(glob_init(&g, BUFLIT1("?fix")), GLOB_SUFFIX);

		asserteq(g.plen, 0);
		asserteq(g.slen, 3);
		asserteq(g.max, 4);

		GLOB_TEST(1, "afix");
		GLOB_TEST(0, "");
		GLOB_TEST(0, "foo");
		GLOB_TEST(0, "fix");
		GLOB_TEST(0, "suffix");
		GLOB_TEST(1, "?fix");
		glob_free(&g);
	}

	it("question?mark") {
		asserteq(glob_init(&g, BUFLIT1("question?mark")), GLOB_MIXED);

		asserteq(g.plen, 8);
		asserteq(g.slen, 4);
		asserteq(g.max, 13);

		GLOB_TEST(0, "questionmark");
		GLOB_TEST(0, "");
		GLOB_TEST(0, "foo");
		GLOB_TEST(0, "questionable mark");
		GLOB_TEST(1, "question mark");
		GLOB_TEST(1, "question?mark");
		glob_free(&g);
	}
}
