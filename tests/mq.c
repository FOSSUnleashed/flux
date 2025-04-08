#include <flux/mq.h>
#include <snow.h>

#define SZ (1 << 15)

describe(mq) {
	flux_mq *mq = flux_mq_alloc(1 << 14);
	char buffer[SZ];
	ssize_t sz;

	flux_mq_reader *rA, *rB, *rC;

	it("alloc") {
		assertneq(mq, NULL, "Allocation successful");
	}

	it("write") {
		asserteq(flux_mq_write(mq, "This is a test", -1), 0, "Send a test message");

		rA = flux_mq_get_reader(mq);

		assertneq(rA, NULL, "Allocate a reader");

		assertneq(sz = flux_mq_read(rA, buffer, SZ), -1, "Read message");

		buffer[sz] = 0;

		asserteq_str(buffer, "This is a test", "We have correct message");
	}

#define write(str, sz) flux_mq_write(mq, str, sz)
#define read(rdr) flux_mq_read(rdr, buffer, SZ)

#define read_cmp(rdr, res, cmt) read(rdr); asserteq_str(buffer, res, cmt)

	it("Multi-Write") {
		write("ABC", 4);
		write("123", 4);
		write("foobar", 7);

		sz = read_cmp(rA, "ABC", "First");

		sz = read_cmp(rA, "123", "Second");

		sz = read_cmp(rA, "foobar", "Third");

		asserteq(read(rA), 0, "No message to read");

		write("this", 5);

		read(rA); // advance the position pointer
	}

	it("Multi-read") {
		rB = flux_mq_get_reader(mq);

		read_cmp(rB, "this", "Read");

		sz = read(rB);
		asserteq(sz, 0, "Read to end");

		write("fizzbuzz", 9);
		write("gondor", 7);

		rC = flux_mq_get_reader(mq);
		flux_mq_reader_skip(rC);

		write("twitch", 7);

		read_cmp(rC, "twitch", "Last message");
	}
}
