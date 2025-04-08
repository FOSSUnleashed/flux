#include <flux/str.h>
#include <newniine.h>

uint8_t *bufFile_write(BufFile * file, uint8_t *buf, uint8_t *be) {
	Buffer a = {.start = bufFile_getpos(file), .end = file->be}, b = {.start = buf, .end = be}, p;

	p = flux_bufcpy(a, b);

	file->f.st.size = p.end - file->b;

	return p.end;
}

uint8_t *bufFile_linewrite(BufFile * file, uint8_t *buf, uint8_t *be) {
	uint8_t *b;

	b = bufFile_write(file, buf, be);

	if (be > b && NULL != b) {
		*b++ = '\n';
		file->f.st.size++;
	}

	return b;
}

void bufFile_clear(BufFile * file) {
	file->f.st.size = 0;
}
