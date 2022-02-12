/* See LICENSE file for copyright and license details. */
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <png.h>

#include "drw.h"
#include "util.h"

#define UTF_INVALID 0xFFFD
#define UTF_SIZ     4

static const unsigned char utfbyte[UTF_SIZ + 1] = {0x80,    0, 0xC0, 0xE0, 0xF0};
static const unsigned char utfmask[UTF_SIZ + 1] = {0xC0, 0x80, 0xE0, 0xF0, 0xF8};
static const long utfmin[UTF_SIZ + 1] = {       0,    0,  0x80,  0x800,  0x10000};
static const long utfmax[UTF_SIZ + 1] = {0x10FFFF, 0x7F, 0x7FF, 0xFFFF, 0x10FFFF};


// Icon path to search for 32x32 png files -> statusbar
static char* iconpath = "/usr/local/share/phyos/dwm/icons/";
static XImage* icon_ximg[10];


static long
utf8decodebyte(const char c, size_t *i)
{
	for (*i = 0; *i < (UTF_SIZ + 1); ++(*i))
		if (((unsigned char)c & utfmask[*i]) == utfbyte[*i])
			return (unsigned char)c & ~utfmask[*i];
	return 0;
}

static size_t
utf8validate(long *u, size_t i)
{
	if (!BETWEEN(*u, utfmin[i], utfmax[i]) || BETWEEN(*u, 0xD800, 0xDFFF))
		*u = UTF_INVALID;
	for (i = 1; *u > utfmax[i]; ++i)
		;
	return i;
}

static size_t
utf8decode(const char *c, long *u, size_t clen)
{
	size_t i, j, len, type;
	long udecoded;

	*u = UTF_INVALID;
	if (!clen)
		return 0;
	udecoded = utf8decodebyte(c[0], &len); if (!BETWEEN(len, 1, UTF_SIZ))
		return 1;
	for (i = 1, j = 1; i < clen && j < len; ++i, ++j) {
		udecoded = (udecoded << 6) | utf8decodebyte(c[i], &type);
		if (type)
			return j;
	}
	if (j < len)
		return 0;
	*u = udecoded;
	utf8validate(u, len);

	return len;
}

Drw *
drw_create(Display *dpy, int screen, Window root, unsigned int w, unsigned int h)
{
	Drw *drw = ecalloc(1, sizeof(Drw));

	drw->dpy = dpy;
	drw->screen = screen;
	drw->root = root;
	drw->w = w;
	drw->h = h;
	drw->drawable = XCreatePixmap(dpy, root, w, h, DefaultDepth(dpy, screen));
	drw->gc = XCreateGC(dpy, root, 0, NULL);
	XSetLineAttributes(dpy, drw->gc, 1, LineSolid, CapButt, JoinMiter);

	return drw;
}

void
drw_resize(Drw *drw, unsigned int w, unsigned int h)
{
	if (!drw)
		return;

	drw->w = w;
	drw->h = h;
	if (drw->drawable)
		XFreePixmap(drw->dpy, drw->drawable);
	drw->drawable = XCreatePixmap(drw->dpy, drw->root, w, h, DefaultDepth(drw->dpy, drw->screen));
}

void
drw_free(Drw *drw)
{
	XFreePixmap(drw->dpy, drw->drawable);
	XFreeGC(drw->dpy, drw->gc);
	free(drw);
}

/* This function is an implementation detail. Library users should use
 * drw_fontset_create instead.
 */
static Fnt *
xfont_create(Drw *drw, const char *fontname, FcPattern *fontpattern)
{
	Fnt *font;
	XftFont *xfont = NULL;
	FcPattern *pattern = NULL;

	if (fontname) {
		/* Using the pattern found at font->xfont->pattern does not yield the
		 * same substitution results as using the pattern returned by
		 * FcNameParse; using the latter results in the desired fallback
		 * behaviour whereas the former just results in missing-character
		 * rectangles being drawn, at least with some fonts. */
		if (!(xfont = XftFontOpenName(drw->dpy, drw->screen, fontname))) {
			fprintf(stderr, "error, cannot load font from name: '%s'\n", fontname);
			return NULL;
		}
		if (!(pattern = FcNameParse((FcChar8 *) fontname))) {
			fprintf(stderr, "error, cannot parse font name to pattern: '%s'\n", fontname);
			XftFontClose(drw->dpy, xfont);
			return NULL;
		}
	} else if (fontpattern) {
		if (!(xfont = XftFontOpenPattern(drw->dpy, fontpattern))) {
			fprintf(stderr, "error, cannot load font from pattern.\n");
			return NULL;
		}
	} else {
		die("no font specified.");
	}

	font = ecalloc(1, sizeof(Fnt));
	font->xfont = xfont;
	font->pattern = pattern;
	font->h = xfont->ascent + xfont->descent;
	font->dpy = drw->dpy;

	return font;
}

static void
xfont_free(Fnt *font)
{
	if (!font)
		return;
	if (font->pattern)
		FcPatternDestroy(font->pattern);
	XftFontClose(font->dpy, font->xfont);
	free(font);
}

Fnt*
drw_fontset_create(Drw* drw, const char *fonts[], size_t fontcount)
{
	Fnt *cur, *ret = NULL;
	size_t i;

	if (!drw || !fonts)
		return NULL;

	for (i = 1; i <= fontcount; i++) {
		if ((cur = xfont_create(drw, fonts[fontcount - i], NULL))) {
			cur->next = ret;
			ret = cur;
		}
	}
	return (drw->fonts = ret);
}

void
drw_fontset_free(Fnt *font)
{
	if (font) {
		drw_fontset_free(font->next);
		xfont_free(font);
	}
}

void
drw_clr_create(Drw *drw, Clr *dest, const char *clrname)
{
	if (!drw || !dest || !clrname)
		return;

	if (!XftColorAllocName(drw->dpy, DefaultVisual(drw->dpy, drw->screen),
	                       DefaultColormap(drw->dpy, drw->screen),
	                       clrname, dest))
		die("error, cannot allocate color '%s'", clrname);

	dest->pixel |= 0xff << 24;
}

/* Wrapper to create color schemes. The caller has to call free(3) on the
 * returned color scheme when done using it. */
Clr *
drw_scm_create(Drw *drw, char *clrnames[], size_t clrcount)
{
	size_t i;
	Clr *ret;

	/* need at least two colors for a scheme */
	if (!drw || !clrnames || clrcount < 2 || !(ret = ecalloc(clrcount, sizeof(XftColor))))
		return NULL;

	for (i = 0; i < clrcount; i++)
		drw_clr_create(drw, &ret[i], clrnames[i]);
	return ret;
}

void
drw_setfontset(Drw *drw, Fnt *set)
{
	if (drw)
		drw->fonts = set;
}

void
drw_setscheme(Drw *drw, Clr *scm)
{
	if (drw)
		drw->scheme = scm;
}

void
drw_rect(Drw *drw, int x, int y, unsigned int w, unsigned int h, int filled, int invert)
{
	if (!drw || !drw->scheme)
		return;
	XSetForeground(drw->dpy, drw->gc, invert ? drw->scheme[ColBg].pixel : drw->scheme[ColFg].pixel);
	if (filled)
		XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, y, w, h);
	else
		XDrawRectangle(drw->dpy, drw->drawable, drw->gc, x, y, w - 1, h - 1);
}

int
drw_text(Drw *drw, int x, int y, unsigned int w, unsigned int h, unsigned int lpad, const char *text, int invert)
{
	char buf[1024];
	int ty;
	unsigned int ew;
	XftDraw *d = NULL;
	Fnt *usedfont, *curfont, *nextfont;
	size_t i, len;
	int utf8strlen, utf8charlen, render = x || y || w || h;
	long utf8codepoint = 0;
	const char *utf8str;
	FcCharSet *fccharset;
	FcPattern *fcpattern;
	FcPattern *match;
	XftResult result;
	int charexists = 0;

	if (!drw || (render && !drw->scheme) || !text || !drw->fonts)
		return 0;

	if (!render) {
		w = ~w;
	} else {
		XSetForeground(drw->dpy, drw->gc, drw->scheme[invert ? ColFg : ColBg].pixel);
		XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, y, w, h);
		d = XftDrawCreate(drw->dpy, drw->drawable,
		                  DefaultVisual(drw->dpy, drw->screen),
		                  DefaultColormap(drw->dpy, drw->screen));
		x += lpad;
		w -= lpad;
	}

	usedfont = drw->fonts;
	while (1) {
		utf8strlen = 0;
		utf8str = text;
		nextfont = NULL;
		while (*text) {
			utf8charlen = utf8decode(text, &utf8codepoint, UTF_SIZ);
			for (curfont = drw->fonts; curfont; curfont = curfont->next) {
				charexists = charexists || XftCharExists(drw->dpy, curfont->xfont, utf8codepoint);
				if (charexists) {
					if (curfont == usedfont) {
						utf8strlen += utf8charlen;
						text += utf8charlen;
					} else {
						nextfont = curfont;
					}
					break;
				}
			}

			if (!charexists || nextfont)
				break;
			else
				charexists = 0;
		}

		if (utf8strlen) {
			drw_font_getexts(usedfont, utf8str, utf8strlen, &ew, NULL);
			/* shorten text if necessary */
			for (len = MIN(utf8strlen, sizeof(buf) - 1); len && ew > w; len--)
				drw_font_getexts(usedfont, utf8str, len, &ew, NULL);

			if (len) {
				memcpy(buf, utf8str, len);
				buf[len] = '\0';
				if (len < utf8strlen)
					for (i = len; i && i > len - 3; buf[--i] = '.')
						; /* NOP */

				if (render) {
					ty = y + (h - usedfont->h) / 2 + usedfont->xfont->ascent;
					XftDrawStringUtf8(d, &drw->scheme[invert ? ColBg : ColFg],
					                  usedfont->xfont, x, ty, (XftChar8 *)buf, len);
				}
				x += ew;
				w -= ew;
			}
		}

		if (!*text) {
			break;
		} else if (nextfont) {
			charexists = 0;
			usedfont = nextfont;
		} else {
			/* Regardless of whether or not a fallback font is found, the
			 * character must be drawn. */
			charexists = 1;

			fccharset = FcCharSetCreate();
			FcCharSetAddChar(fccharset, utf8codepoint);

			if (!drw->fonts->pattern) {
				/* Refer to the comment in xfont_create for more information. */
				die("the first font in the cache must be loaded from a font string.");
			}

			fcpattern = FcPatternDuplicate(drw->fonts->pattern);
			FcPatternAddCharSet(fcpattern, FC_CHARSET, fccharset);
			FcPatternAddBool(fcpattern, FC_SCALABLE, FcTrue);
			FcPatternAddBool(fcpattern, FC_COLOR, FcFalse);

			FcConfigSubstitute(NULL, fcpattern, FcMatchPattern);
			FcDefaultSubstitute(fcpattern);
			match = XftFontMatch(drw->dpy, drw->screen, fcpattern, &result);

			FcCharSetDestroy(fccharset);
			FcPatternDestroy(fcpattern);

			if (match) {
				usedfont = xfont_create(drw, NULL, match);
				if (usedfont && XftCharExists(drw->dpy, usedfont->xfont, utf8codepoint)) {
					for (curfont = drw->fonts; curfont->next; curfont = curfont->next)
						; /* NOP */
					curfont->next = usedfont;
				} else {
					xfont_free(usedfont);
					usedfont = drw->fonts;
				}
			}
		}
	}
	if (d)
		XftDrawDestroy(d);

	return x + (render ? w : 0);
}

void
drw_map(Drw *drw, Window win, int x, int y, unsigned int w, unsigned int h)
{
	if (!drw)
		return;

	XCopyArea(drw->dpy, drw->drawable, win, drw->gc, x, y, w, h, x, y);
	XSync(drw->dpy, False);
}

unsigned int
drw_fontset_getwidth(Drw *drw, const char *text)
{
	if (!drw || !drw->fonts || !text)
		return 0;
	return drw_text(drw, 0, 0, 0, 0, 0, text, 0);
}

void
drw_font_getexts(Fnt *font, const char *text, unsigned int len, unsigned int *w, unsigned int *h)
{
	XGlyphInfo ext;

	if (!font || !text)
		return;

	XftTextExtentsUtf8(font->dpy, font->xfont, (XftChar8 *)text, len, &ext);
	if (w)
		*w = ext.xOff;
	if (h)
		*h = font->h;
}

Cur *
drw_cur_create(Drw *drw, int shape)
{
	Cur *cur;

	if (!drw || !(cur = ecalloc(1, sizeof(Cur))))
		return NULL;

	cur->cursor = XCreateFontCursor(drw->dpy, shape);

	return cur;
}

void
drw_cur_free(Drw *drw, Cur *cursor)
{
	if (!cursor)
		return;

	XFreeCursor(drw->dpy, cursor->cursor);
	free(cursor);
}

void
drw_logo(Drw* drw, int logo, int x, int y, unsigned int w, unsigned int h)
{
	XPutImage(drw->dpy, drw->drawable, drw->gc, icon_ximg[logo], 0, 0, x , y, w, h);
}

// Png draw support
int
get_png_files(const char *path, char ***png_list, char*** png_names)
{
	const unsigned char PNG_MAGIC[] = { 0x89, 0x50, 0x4e, 0x47,
					    0x0d, 0x0a, 0x1a, 0x0a };
	unsigned char header[8];
	int nbytes, npngs = 0;
	FILE *fp;
	DIR *dp;
	struct dirent *entry;
	char **png_array;
	char **png_n;

	dp = opendir(path);
	png_array = (char **) malloc(sizeof(char *) * 10);
	png_n = (char **) malloc(sizeof(char *) * 10);
	size_t plen = strlen(path);

	while ((entry = readdir(dp)) != NULL) {
		char* fpath = (char*) malloc(plen + strlen(entry->d_name) + 2);

		if (fpath == NULL) {
			fprintf(stderr, "memory alloc error");
		}

		if (strlen(entry->d_name) > 3) {
			sprintf(fpath, "%s%s", path, entry->d_name);

			if ((fp = fopen(fpath, "rb")) == NULL)
				fprintf(stderr, "open %s fail\n", entry->d_name);

			memset(header, 0, sizeof(header));
			nbytes = fread(header, sizeof(unsigned char), sizeof(header), fp);

			if (nbytes == sizeof(header) && !memcmp(header, PNG_MAGIC, sizeof(header))) {
				png_array[npngs] = (char *) malloc(strlen(fpath) + 1);
				png_n[npngs] = (char *) malloc(strlen(entry->d_name) + 1);
				strcpy(png_array[npngs], fpath);
				strcpy(png_n[npngs], entry->d_name);
				npngs++;
			}
			fclose(fp);
		}
		if (npngs > 0 && npngs % 10 == 0) {
			png_array = (char **) realloc(png_array,
					sizeof(char *) * 10 * (npngs / 10 + 1));
			png_n = (char **) realloc(png_n,
					sizeof(char *) * 10 * (npngs / 10 + 1));

		}
		free(fpath);
	}

	closedir(dp);
	png_array = (char **) realloc(png_array, sizeof(char *) * npngs);
	png_n = (char **) realloc(png_n, sizeof(char *) * npngs);
	*png_list = png_array;
	*png_names = png_n;

	return npngs;
}

static
void tearpng (png_structp png, png_infop info)
{
	if (png) {
		png_infop *realInfo = (info? &info: NULL);
		png_destroy_read_struct (&png, realInfo, NULL);
	}
}

void
loadpng (FILE* file, unsigned char** data, int* width, int* height, int *rowbytes, unsigned char b, unsigned char g, unsigned char r)
{

	size_t size = 0;
	int x, y;

	png_structp png = NULL;
	png_infop info = NULL;
	unsigned char **rowPointers = NULL;

	int depth = 0,
	colortype = 0,
	interlace = 0,
	compression = 0,
	filter = 0;

	png = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info = png_create_info_struct (png);
	png_init_io (png, file);
	png_read_info (png, info);
	png_get_IHDR (png, info, (png_uint_32*)width, (png_uint_32*)height, &depth, &colortype, &interlace, &compression, &filter);

	*rowbytes = png_get_rowbytes (png, info);

	png_set_bgr (png);
	*width = png_get_image_width (png, info);
	*height = png_get_image_height (png, info);
	size = *height * *rowbytes;

	// This gets freed by XDestroyImage
	*data = (unsigned char*) malloc (sizeof (png_byte) * size);

	rowPointers = (unsigned char**) malloc (*height * sizeof (unsigned char*));

	png_bytep cursor = *data;

	for (int i=0; i<*height; ++i, cursor += *rowbytes)
		rowPointers[i] = cursor;

	png_read_image (png, rowPointers);

	for (y=0; y<*height; ++y, cursor+=*rowbytes) {
		png_bytep row = rowPointers[y];

		for (x=0; x<*width; x++) {
			png_bytep px = &(row[x * 4]);

			if (!px[3]){
				px[0] = b;
				px[1] = g;
				px[2] = r;
			}
		}
	}
	tearpng (png, info);
	free (rowPointers);
}

void
load_png_icons(Drw* drw, char* sb, char* ib)
{
	if (drw && icon_ximg[0] == NULL) {
		char **png_files;
		char **png_names;
		int png_bytes, n, w, h;
		char buf[250];
		unsigned int r, g, b;
		unsigned char* data;
		FILE* fp;
		XImage* ximage;
		sb++;
		ib++;

		n = get_png_files(iconpath, &png_files, &png_names);

		for (int i=0; i<n && png_files[i] != NULL; i++)
		{
			fp = fopen(png_files[i], "rb");
			int idx = png_names[i][0] - '0';

			if (idx < 2) {
				sscanf(sb, "%02x%02x%02x", &r, &g, &b);
				loadpng(fp, &data, &w, &h, &png_bytes, b, g, r);
			}
			else {
				sscanf(ib, "%02x%02x%02x", &r, &g, &b);
				loadpng(fp, &data, &w, &h, &png_bytes, b, g, r);
			}

			ximage = XCreateImage(drw->dpy, DefaultVisual(drw->dpy, drw->screen), DefaultDepth(drw->dpy, drw->screen), ZPixmap, 0, (char*)data, w, h, 32, png_bytes);
			icon_ximg[idx] = ximage;
			fclose(fp);
		}
	}
}

void
clear_png_icons()
{
	for (int i=0; i<10; i++) {
		if (icon_ximg[i] != NULL)
			XDestroyImage(icon_ximg[i]);
	}
}
