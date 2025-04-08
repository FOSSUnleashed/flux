#include <flux/gate.h>
#include <assert.h>
#include <stdio.h>

int i = 0;

coroutine void thing(handle gate) {
	yield();

	for (; i < 10; ++i) {
		if (7 == i) {
			flux_gateopen(gate);
			return;
		}

		printf("CR: %d\n", i);
	}
}

int main(int argc, char **argv) {
	handle gate;

	gate = flux_mgate();

	assert(-1 != gate);

	go(thing(gate));

	flux_gatehold(gate, -1);

	for (; i < 10; ++i) {
		printf("MAIN: %d\n", i);
	}

	return 0;
}
