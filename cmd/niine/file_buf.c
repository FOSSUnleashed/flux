#include <flux/str.h>
#include <newniine.h>

uint8_t *bufFile_write(BufFile * file, uint8_t *buf, uint8_t *be) {
	uint8_t *b;

	b = flux_bufcpy(bufFile_getpos(file), file->be, buf, be);

	file->f.st.size = b - file->b;

	return b;
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
