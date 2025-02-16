#include <flux/time.h>
#include <stdio.h>

int main(void) {
	printf("%llu\n", flux_us());

	return 0;
}
