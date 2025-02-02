#include <stdint.h>
#include <stddef.h>

static const char enmap32[] = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";

static const char demap32[] = {
	 0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF // 00 07
	,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF // 08 0F
	,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF // 10 17
	,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF // 18 1F
	,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF // 20 27
	,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF // 28 2F
	,0x00 ,0x01 ,0x02 ,0x03 ,0x04 ,0x05 ,0x06 ,0x07 // 30 37 0-7
	,0x08 ,0x09 ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF // 38 3F 8 9
	,0xFF ,0x0A ,0x0B ,0x0C ,0x0D ,0x0E ,0x0F ,0x10 // 40 47 @ A B C D E F G
	,0x11 ,0x01 ,0x12 ,0x13 ,0x01 ,0x14 ,0x15 ,0x00 // 48 4F H I J K L M N O
	,0x16 ,0x17 ,0x18 ,0x19 ,0x1A ,0xFF ,0x1B ,0x1C // 50 57 P Q R S T U V W
	,0x1D ,0x1E ,0x1F ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF // 58 5F X Y Z
	,0xFF ,0x0A ,0x0B ,0x0C ,0x0D ,0x0E ,0x0F ,0x10 // 60 67 ` a b c d e f g
	,0x11 ,0x01 ,0x12 ,0x13 ,0x01 ,0x14 ,0x15 ,0x00 // 68 6F h i j k l m n o
	,0x16 ,0x17 ,0x18 ,0x19 ,0x1A ,0xFF ,0x1B ,0x1C // 70 77 p q r s t u v w
	,0x1D ,0x1E ,0x1F ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF // 78 7F x y z
};

#define enbase32_map(c) enmap32[(c) >> 3]

uint8_t * flux_enbase32(uint8_t * out, uint8_t * oend, uint8_t * in, uint8_t * iend) {
	uint8_t bits, tok;

	bits = 0;

	while (in < iend && out < oend) {
		switch (bits) { // how many bits in tok
		case 0:
			*out++ = enbase32_map(*in & 0xF8); // 1111 1000
			tok	= *in++ & 0x07; // 0000 0111
			bits = 3;
			break;
		case 1:
			*out++ = enbase32_map((tok << 7) | ((*in & 0xF0) >> 1)); // 1111 0000
			tok	= *in++ & 0x0F; // 0000 1111
			bits = 4;
			break;
		case 2:
			*out++ = enbase32_map((tok << 6) | ((*in & 0xE0) >> 2)); // 1110 0000
			*out++ = enmap32[*in++ & 0x1F]; // 0001 1111
			bits = 0;
			break;
		case 3:
			*out++ = enbase32_map((tok << 5) | ((*in & 0xC0) >> 3)); // 1100 0000
			*out++ = enbase32_map((*in & 0x3E) << 2); // 0011 1110
			tok = *in++ & 0x01; // 0000 0001
			bits = 1;
			break;
		case 4:
			*out++ = enbase32_map((tok << 4) | ((*in & 0x80) >> 4)); // 1000 0000
			*out++ = enbase32_map((*in & 0x7C) << 1); // 0111 1100
			tok = *in++ & 0x03; // 0000 0011
			bits = 2;
			break;
		}
	}

	// TODO: figgure out case where we are passed exactly the buffer size we need, and bits does need writing
	if (out >= oend) {
		return NULL;
	}

	if (bits)
		*out++ = enbase32_map(tok << (8 - bits));

	return out;
}

uint8_t * flux_debase32(uint8_t * out, uint8_t * oend, uint8_t * in, uint8_t * iend) {
	uint8_t bits, tok;

	bits = 0;

	// TODO: double writes to out can exceed buffer, check it properly
	while (in < iend && *in < 0x80 && '=' != *in && out < (oend - 1)) {
		tok	= demap32[*in];

		if (0xFF == tok)
			return NULL;

		switch (bits) { // how many bits in *out
		case 0:
			*out = tok << 3;
			break;
		case 1:
			*out |= (tok << 2); // 0111 1100
			break;
		case 2:
			*out |= (tok << 1); // 0011 1110
			break;
		case 3:
			*out |= tok; // 0001 1111
			out++;
			break;
		case 4:
			*out |= (tok & 0x1E) >> 1; // 0001 1110
			out++;
			*out	= (tok & 0x01) << 7; // 0000 0001
			break;
		case 5:
			*out |= (tok & 0x1C) >> 2; // 0001 1100
			out++;
			*out = (tok & 0x03) << 6; // 0000 0011
			break;
		case 6:
			*out |= ((tok & 0x18) >> 3); // 0001 1000
			out++;
			*out = (tok & 0x07) << 5; // 0000 0111
			break;
		case 7:
			*out |= ((tok & 0x10) >> 4); // 0001 0000
			out++;
			*out	= (tok & 0x0F) << 4; // 0000 1111
			break;
		}
		++in;
		bits = (bits + 5) & 0x07;
	}

	if (out >= oend) {
		return NULL;
	}

	return out;
}
