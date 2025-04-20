
/*
; #2124 2627 2829 2a2b 2c3b 3d 3a2f3f23 5b5d 40
//workstation4k/R /home/share/Rpub/flux (:1 /tmp/ns.R.:0 fo.github.com:FOSSUnleashed/flux)
; #41-5a == A-Z
//workstation4k/R /home/share/Rpub/flux (:1 /tmp/ns.R.:0 fo.github.com:FOSSUnleashed/flux)
; xxd tmptmp
00000000: 6765 6e2d 6465 6c69 6d73 2020 3d20 3a2f  gen-delims  = :/
00000010: 3f23 5b5d 400a 7375 622d 6465 6c69 6d73  ?#[]@.sub-delims
00000020: 2020 3d20 2124 2627 2829 2a2b 2c3b 3d0a    = !$&'()*+,;=.
//workstation4k/R /home/share/Rpub/flux (:1 /tmp/ns.R.:0 fo.github.com:FOSSUnleashed/flux)
; echo \x5c
\
//workstation4k/R /home/share/Rpub/flux (:1 /tmp/ns.R.:0 fo.github.com:FOSSUnleashed/flux)
; #21-5d == 3d 48 + 13 == 61 || 61 - 10 (numeric) - 26 (alpha) == 25 - 5 == 20
//workstation4k/R /home/share/Rpub/flux (:1 /tmp/ns.R.:0 fo.github.com:FOSSUnleashed/flux)
; echo \x22 \x2d \x2e \x3c \x3e \x5c
" - . < > \
//workstation4k/R /home/share/Rpub/flux (:1 /tmp/ns.R.:0 fo.github.com:FOSSUnleashed/flux)
; echo \x22 \x25 \x2d \x2e \x3c \x3e \x5c
" % - . < > \
//workstation4k/R /home/share/Rpub/flux (:1 /tmp/ns.R.:0 fo.github.com:FOSSUnleashed/flux)
; echo \x22 \x2d \x2e \x3c \x3e \x5c
" - . < > \

(0x20 < c && 0x5f < c) && !(A-Z) && !(0-9)
*/

// https://www.rfc-editor.org/rfc/rfc3986#section-2.1
#define NEED_PERCENT(c) ((0x21 == c) || (0x22 < c && 0x2d > c) || (0x2f == c) || (0x3a == c) || (0x3b == c) || (0x3d == c) || (0x3f == c) || (0x40 == c) || (0x5b == c) || (0x5d == c) || (0x7f < c))

#include <flux/str.h>
#include "str.h"

#include <stdio.h>
#include <stdint.h>

void test_per() {
	uint8_t c = 0;
	int n = 0;

	for (; c < 0x80; ++c) {
		if (NEED_PERCENT(c)) {
			printf("%02x:%c\n", c, c);
			++n;
		}
	}

	printf("%d\n", n);
}

Buffer flux_http_urlencode(Buffer dst, Buffer key, Buffer value) {
	if (isbadbuffer(dst) || isbadbuffer(key) || isbadbuffer(value)) {
		goto error;
	}

	if (BUFSZ(dst) < (BUFSZ(key) + BUFSZ(value) + 3)) { // ?&=
		goto error;
	}

	while (NOTEND(dst) && NOTEND(key)) {
		if (NEED_PERCENT(*key.start)) {
			if (3 > BUFSZ(dst)) {
				goto error;
			}
			*dst.start++ = '%';
			dst.start = writehexchar(dst, *key.start++);
		} else {
			*dst.start++ = *key.start++;
		}
	}

	if (NOTEND(dst)) {
		*dst.start++ = '=';
	}

	while (NOTEND(dst) && NOTEND(value)) {
		if (NEED_PERCENT(*value.start)) {
			if (3 > BUFSZ(dst)) {
				goto error;
			}
			*dst.start++ = '%';
			dst.start = writehexchar(dst, *value.start++);
		} else {
			*dst.start++ = *value.start++;
		}
	}

	if (dst.end == dst.start) {
		goto error;
	}

	return dst;
	error:
	errno = EINVAL;
	return badbuffer;
}
