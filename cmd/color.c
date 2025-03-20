/*
 * This file contains the function from SocketMUD which
 * is used to convert #color codes to ANSI color sequences
 * for use in terminal programs.  Example usage:
 * echo '#g::.::#n     #p::.::#n' | color
 */

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <sys/ioctl.h>
#include <errno.h>

#include <stdint.h>

#define	bool	uint8_t
#define	TRUE	1
#define	FALSE	0

enum {
	eTHIN
	,eBOLD
};

#define MAX_BUFFER (1 << 13)

void process_text_color(uint32_t *sz, const char *in_txt, char *out_txt);

int main(void) {
	int32_t sz;
	char in_txt[MAX_BUFFER], out_txt[MAX_BUFFER << 1];

	while (sz = read(0, in_txt, MAX_BUFFER)) {
		// TODO: fix bug where a # might be at the end of a read chunk
		in_txt[sz] = 0;

		process_text_color(&sz, in_txt, out_txt);

		write(1, out_txt, sz);
	}

	return 0;
}

/*
 * Text_to_buffer()
 *
 * Stores outbound text in a buffer, where it will
 * stay untill it is flushed in the gameloop.
 *
 * Will also parse ANSI colors and other tags.
 */
void process_text_color(uint32_t *sz, const char *txt, char *output) {
	bool underline = FALSE, bold = FALSE;
	int iPtr = 0, last = -1, j, k;
	int length = *sz;
	*sz = 0;

  /* the color struct */
  struct sAnsiColor {
    const char    cTag;
    const char  * cString;
    int           aFlag;
  };

  /* the color table... */
  const struct sAnsiColor ansiTable[] =
  {
    { 'd',  "30",  eTHIN },
    { 'D',  "30",  eBOLD },
    { 'r',  "31",  eTHIN },
    { 'R',  "31",  eBOLD },
    { 'g',  "32",  eTHIN },
    { 'G',  "32",  eBOLD },
    { 'y',  "33",  eTHIN },
    { 'Y',  "33",  eBOLD },
    { 'b',  "34",  eTHIN },
    { 'B',  "34",  eBOLD },
    { 'p',  "35",  eTHIN },
    { 'P',  "35",  eBOLD },
    { 'c',  "36",  eTHIN },
    { 'C',  "36",  eBOLD },
    { 'w',  "37",  eTHIN },
    { 'W',  "37",  eBOLD },

    /* the end tag */
    { '\0',  "",   eTHIN }
  };

  if (length >= MAX_BUFFER) {
    /*log_string("text_to_buffer: buffer overflow."); */
    return;
  }

  /* always start with a leading space */
/*  if (dsock->top_output == 0)
  {
    dsock->outbuf[0] = '\n';
    dsock->outbuf[1] = '\r';
    dsock->top_output = 2;
  } */

	while (*txt != '\0') {
		/* simple bound checking */
		if (iPtr > (MAX_BUFFER - 15))
			break;

		switch (*txt) {
			default:
				output[iPtr++] = *txt++;
			break;
			case '#':
				txt++;

				/* toggle underline on/off with #u */
				if (*txt == 'u') {
					txt++;
					if (underline) {
						underline = FALSE;
						output[iPtr++] =  27; output[iPtr++] = '['; output[iPtr++] = '0';
						if (bold) {
							output[iPtr++] = ';'; output[iPtr++] = '1';
						}
						if (last != -1) {
							output[iPtr++] = ';';
							for (j = 0; ansiTable[last].cString[j] != '\0'; j++) {
								output[iPtr++] = ansiTable[last].cString[j];
							}
						}
						output[iPtr++] = 'm';
					} else {
						underline = TRUE;
						output[iPtr++] =  27; output[iPtr++] = '[';
						output[iPtr++] = '4'; output[iPtr++] = 'm';
					}

				} else if ('-' == *txt) { // [38;5;248m
					++txt;
					output[iPtr++]	= 27;
					output[iPtr++]	= '[';
					output[iPtr++]	= '3';
					output[iPtr++]	= '8';
					output[iPtr++]	= ';';
					output[iPtr++]	= '5';
					output[iPtr++]	= ';';
					for (; '0' <= *txt && *txt <= '9'; txt++) {
						output[iPtr++]	= *txt;
					}
					if ('-' == *txt) {
						++txt;
					}
					output[iPtr++]	= 'm';
				} else if ('=' == *txt) { // [48;5;248m
					++txt;
					output[iPtr++]	= 27;
					output[iPtr++]	= '[';
					output[iPtr++]	= '4';
					output[iPtr++]	= '8';
					output[iPtr++]	= ';';
					output[iPtr++]	= '5';
					output[iPtr++]	= ';';
					for (; '0' <= *txt && *txt <= '9'; txt++) {
						output[iPtr++]	= *txt;
					}
					if ('=' == *txt) {
						++txt;
					}
					output[iPtr++]	= 'm';

				/* parse ## to # */
				} else if (*txt == '#') {
					txt++;
					output[iPtr++] = '#';

				/* #n should clear all tags */
				} else if (*txt == 'n') {
					txt++;
					//if (last != -1 || underline || bold) {  
						underline = FALSE;
						bold = FALSE;
						output[iPtr++] =  27; output[iPtr++] = '[';
						output[iPtr++] = '0'; output[iPtr++] = 'm';
					//}

					last = -1;
				/* check for valid color tag and parse */
				} else {
					bool validTag = FALSE;

					for (j = 0; ansiTable[j].cString[0] != '\0'; j++) {
						if (*txt == ansiTable[j].cTag) {
							validTag = TRUE;

							/* we only add the color sequence if it's needed */
							if (last != j) {
								bool cSequence = FALSE;

								/* escape sequence */
								output[iPtr++] = 27; output[iPtr++] = '[';

								/* remember if a color change is needed */
								if (last == -1 || last / 2 != j / 2)
									cSequence = TRUE;

								/* handle font boldness */
								if (bold && ansiTable[j].aFlag == eTHIN) {
									output[iPtr++] = '0';
									bold = FALSE;

									if (underline) {
										output[iPtr++] = ';'; output[iPtr++] = '4';
									}

									/* changing to eTHIN wipes the old color */
									output[iPtr++] = ';';
									cSequence = TRUE;
								} else if (!bold && ansiTable[j].aFlag == eBOLD) {
									output[iPtr++] = '1';
									bold = TRUE;

									if (cSequence)
										output[iPtr++] = ';';

								}
								/* add color sequence if needed */
								if (cSequence) {
									for (k = 0; ansiTable[j].cString[k] != '\0'; k++) {
										output[iPtr++] = ansiTable[j].cString[k];
									}
								}

								output[iPtr++] = 'm';
							}

							/* remember the last color */
							last = j;
						}
					}

					/* it wasn't a valid color tag */
					if (!validTag)
						output[iPtr++] = '#';
					else
						txt++;
				}
			break;   
		}
	}

	/* and terminate it with the standard color */
	if (last != -1 || underline || bold) {
		output[iPtr++] =  27; output[iPtr++] = '[';
		output[iPtr++] = '0'; output[iPtr++] = 'm';
	}

	output[iPtr++] = 27;
	output[iPtr++] = '[';
	output[iPtr++] = '0';
	output[iPtr++] = 'm';

	output[iPtr] = '\0';
	*sz = iPtr;

  /* check to see if the socket can accept that much data */
/*  if (dsock->top_output + iPtr >= MAX_OUTPUT)
  {
    bug("Text_to_buffer: ouput overflow on %s.", dsock->hostname);
    return;
  } */

  /* add data to buffer */
/*  strcpy(dsock->outbuf + dsock->top_output, output);
  dsock->top_output += iPtr; */
}
