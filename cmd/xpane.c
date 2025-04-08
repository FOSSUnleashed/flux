#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include <sys/utsname.h>

void writeNum(uint8_t *b, uint8_t *be, uint16_t num) {
	uint8_t *p = be - 1;

	*p-- = 0; // NUL terminator

	while (b < p) {
		*p-- = (num % 10) + '0';
		num /= 10;
		if (0 == num) {
			break;
		}
	}

	// 600 [- - - - - - - - - - - - - 6 0 0]
	// [ 6 0 0 \0 ...]

	if (p == b) {
		return;
	}

	p++;
	while (p < be) {
		*b++ = *p++;
	}
}

typedef struct {
	uint8_t *chars;
	uint16_t nchars;
	uint32_t color;
} TextBlock;

void writeText(Display *dpy, Window win, GC gc, XFontStruct *font, int x_offset, int y_offset, TextBlock *tb, uint16_t ni) {
	XTextItem tx = {.font = font->fid};
	XGCValues gcv = {};

	for (; ni; --ni, ++tb) {
		tx.chars	= tb->chars;
		tx.nchars	= tb->nchars;

		gcv.foreground = tb->color;
		XChangeGC(dpy, gc, GCForeground, &gcv);

		XDrawText(dpy, win, gc, x_offset, y_offset, &tx, 1);

		x_offset += XTextWidth(font, tx.chars, tx.nchars);
	}
}

struct utsname sname;  
bool uname_ok;

TextBlock *writeBlock(TextBlock *tx, uint8_t *str, uint32_t color) {
	tx->chars	= str;
	tx->nchars	= strlen(str);
	tx->color	= color;

	return tx + 1;
}

void render(TextBlock *text, Display *dpy, Window win, GC gc, XFontStruct *hacks) {
	uint8_t xbuf[16], ybuf[16];
	int spacing = hacks->ascent + hacks->descent, y_offset = hacks->ascent;
	XWindowAttributes wa;
	TextBlock *tx;

	tx	= text;
	tx	= writeBlock(tx, "X11 test app under Linux", 0xFF000000);

	writeText(dpy, win, gc, hacks, 10, y_offset, text, 1);
	y_offset += spacing + 5;


	XGetWindowAttributes(dpy, win, &wa);


	tx	= text;
	tx	= writeBlock(tx, "Font ascent: ", 0xFF000000);
	tx	= writeBlock(tx, "TESTING", 0xFF0000FF);
	tx	= writeBlock(tx, "Font ascent: ", 0xFF000000);
	tx	= writeBlock(tx, "TESTING", 0xFF0000FF);
	writeText(dpy, win, gc, hacks, 10, y_offset, text, 4);
	y_offset += spacing + 5;

	if (uname_ok) {
		text[0].chars	= "System information:";
		text[0].nchars	= strlen(text[0].chars);

		writeText(dpy, win, gc, hacks, 10, y_offset, text, 1);
		y_offset += spacing;

		text[0].chars	= "- System: ";
		text[0].nchars	= strlen(text[0].chars);
		text[1].chars	= sname.sysname;
		text[1].nchars	= strlen(text[1].chars);

		writeText(dpy, win, gc, hacks, 10, y_offset, text, 2);
		y_offset += spacing;

		text[0].chars	= "- Release: ";
		text[0].nchars	= strlen(text[0].chars);
		text[1].chars	= sname.release;
		text[1].nchars	= strlen(text[1].chars);

		writeText(dpy, win, gc, hacks, 10, y_offset, text, 2);
		y_offset += spacing;

		text[0].chars	= "- Version: ";
		text[0].nchars	= strlen(text[0].chars);
		text[1].chars	= sname.version;
		text[1].nchars	= strlen(text[1].chars);

		writeText(dpy, win, gc, hacks, 10, y_offset, text, 2);
		y_offset += spacing;

		text[0].chars	= "- Machine: ";
		text[0].nchars	= strlen(text[0].chars);
		text[1].chars	= sname.machine;
		text[1].nchars	= strlen(text[1].chars);

		writeText(dpy, win, gc, hacks, 10, y_offset, text, 2);
		y_offset += spacing + 5;
	}

	writeNum(xbuf, xbuf + sizeof(xbuf), wa.width);
	writeNum(ybuf, ybuf + sizeof(ybuf), wa.height);

	tx	= text;
	tx	= writeBlock(tx, "Current window size: ", 0xFF000000);
	tx	= writeBlock(tx, xbuf, 0xFF0000FF);
	tx	= writeBlock(tx, " x ", 0xFF000000);
	tx	= writeBlock(tx, ybuf, 0xFF0000FF);
//	tx	= writeBlock(tx, ".", 0xFF000000);

	writeText(dpy, win, gc, hacks, 10, y_offset, text, 4);
	y_offset += spacing + 5;

	writeNum(xbuf, xbuf + sizeof(xbuf), hacks->ascent);
	writeNum(ybuf, ybuf + sizeof(ybuf), hacks->descent);
	tx	= text;
	tx	= writeBlock(tx, "Font ascent: ", 0xFF000000);
	tx	= writeBlock(tx, xbuf, 0xFF0000FF);
	tx	= writeBlock(tx, "; descent: ", 0xFF000000);
	tx	= writeBlock(tx, ybuf, 0xFF0000FF);
//	tx	= writeBlock(tx, ".", 0xFF000000);

	writeText(dpy, win, gc, hacks, 10, y_offset, text, 4);
	y_offset += spacing + 5;
}

int main(int argc, char** argv) {
	Display* dpy = XOpenDisplay(NULL);

	if (NULL == dpy) {
		fprintf(stderr, "Cannot open display\n");
		return 1;
	}

	int s = DefaultScreen(dpy);
	GC gc = DefaultGC(dpy, s);
	XFontStruct *hacks;
	TextBlock text[16];
	char *screenmem;
	XVisualInfo vi;
	XConfigureEvent *cfgev, cfgev_ = {.width = 1220, .height = 600};
	cfgev = &cfgev_;

	XSetWindowAttributes  wsa = {.background_pixmap = None, .background_pixel = 0x3fafe9af};

	hacks = XLoadQueryFont(dpy, "-*-hack-*-*-*-*-36-*-*-*-*-*-*-*");

	int mask = CWBackPixmap | CWEventMask | CWBackPixel;

	wsa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

	Window win = XCreateWindow(dpy, RootWindow(dpy, s), 10, 10, cfgev->width, cfgev->height, 1, CopyFromParent, InputOutput, CopyFromParent, mask, &wsa);

	if (0 == XMatchVisualInfo(dpy, s, 24, TrueColor, &vi)) {
		printf("ERROR: No matching visual info\n");
		return 1;
	}

	screenmem = malloc(4 * cfgev->width * cfgev->height);
	XImage *winbuf = XCreateImage(dpy, vi.visual, vi.depth, ZPixmap, 0, screenmem, cfgev->width, cfgev->height, 32, 0);

	XMapWindow(dpy, win);

	XStoreName(dpy, win, "LOLWUT");
  
	Atom WM_DELETE_WINDOW = XInternAtom(dpy, "WM_DELETE_WINDOW", False); 
	XSetWMProtocols(dpy, win, &WM_DELETE_WINDOW, 1);

	uname_ok = -1 != uname(&sname);
 
	XEvent e;
	while (1) {
		XNextEvent(dpy, &e);

		if (e.type == Expose) {
//			XClearWindow(dpy, win);
			render(text, dpy, win, gc, hacks);
		} else if (e.type == KeyPress) {
			char buf[128] = {0};
			KeySym keysym;
			int len = XLookupString(&e.xkey, buf, sizeof buf, &keysym, NULL);
			if (keysym == XK_Escape)
				break;
		} else if (ConfigureNotify == e.type) {
			cfgev = (XConfigureEvent *)&e;

			// NOTE: XDestroyImage also frees Memory, so no need to free from our side
			XDestroyImage(winbuf);

			screenmem = malloc(4 * cfgev->width * cfgev->height);

			winbuf = XCreateImage(dpy, vi.visual, vi.depth, ZPixmap, 0, screenmem, cfgev->width, cfgev->height, 32, 0);

			render(text, dpy, win, gc, hacks);
		} else if ((e.type == ClientMessage) && ((unsigned int)(e.xclient.data.l[0]) == WM_DELETE_WINDOW)) {
			break;
		}

	//	XPutImage(dpy, win, gc, winbuf, 0, 0, 0, 0, cfgev->width, cfgev->height);
  }

	XDestroyImage(winbuf);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
	return 0;
}
