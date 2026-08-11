/* See LICENSE for license details. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <Imlib2.h>

static struct {
	Display *dpy;
	Visual *vis;
	Drawable parent;
	int depth;
	Imlib_Image src;     /* original, decoded once */
	Imlib_Image scaled;  /* cover-scaled to the window size */
	Pixmap pm;           /* blended result, 32bit ARGB */
	int w, h;            /* size of scaled/pm */
} bg;

/*
 * Find the largest centered rectangle in an iw*ih image whose aspect ratio
 * matches a ww*wh window. Stretching that rectangle to the window size fills
 * the window with no distortion and no empty space (cover).
 */
void
bgimg_cover(int iw, int ih, int ww, int wh, int *sx, int *sy, int *sw, int *sh)
{
	if (iw <= 0 || ih <= 0 || ww <= 0 || wh <= 0) {
		/* No ratio is defined here. Use the whole image. */
		*sx = *sy = 0;
		*sw = iw > 0 ? iw : 0;
		*sh = ih > 0 ? ih : 0;
		return;
	}

	if ((int64_t)iw * wh > (int64_t)ww * ih) {
		/* Image is wider than the window: keep the height, crop the sides */
		*sh = ih;
		*sw = (int)(((int64_t)ih * ww) / wh);
	} else {
		/* Image is taller than the window: keep the width, crop top/bottom */
		*sw = iw;
		*sh = (int)(((int64_t)iw * wh) / ww);
	}

	*sx = (iw - *sw) / 2;
	*sy = (ih - *sh) / 2;
}

/*
 * Blend one image pixel with the background colour and apply window opacity.
 *   c = imga * src + (1 - imga) * bg
 *   A = 0xff * a,  RGB = premul ? c * a : c
 * The alpha channel of src is ignored (treated as opaque).
 */
uint32_t
bgimg_blendpx(uint32_t src, uint32_t bg, float imga, float a, int premul)
{
	uint32_t out = (uint32_t)(0xff * a + 0.5f) << 24;
	int shift;

	for (shift = 16; shift >= 0; shift -= 8) {
		float s = (float)((src >> shift) & 0xff);
		float b = (float)((bg >> shift) & 0xff);
		float c = imga * s + (1.0f - imga) * b;

		if (premul)
			c *= a;
		out |= (uint32_t)(c + 0.5f) << shift;
	}

	return out;
}

void
bgimg_xinit(Display *dpy, Visual *vis, Drawable parent, int depth)
{
	bg.dpy = dpy;
	bg.vis = vis;
	bg.parent = parent;
	bg.depth = depth;
}

void
bgimg_free(void)
{
	if (bg.pm != None) {
		XFreePixmap(bg.dpy, bg.pm);
		bg.pm = None;
	}
	if (bg.scaled) {
		imlib_context_set_image(bg.scaled);
		imlib_free_image();
		bg.scaled = NULL;
	}
	if (bg.src) {
		imlib_context_set_image(bg.src);
		imlib_free_image();
		bg.src = NULL;
	}
	bg.w = bg.h = 0;
}

int
bgimg_load(const char *file)
{
	bgimg_free();

	if (!file || !*file)
		return 0;

	if (!(bg.src = imlib_load_image(file))) {
		fprintf(stderr, "st: could not load background image: %s\n", file);
		return 0;
	}

	return 1;
}

void
bgimg_resize(int w, int h)
{
	int sx, sy, sw, sh, iw, ih;

	if (!bg.src || w <= 0 || h <= 0)
		return;

	/* Already scaled to this size. Rescaling is the expensive step, and
	 * callers such as chgalpha() reach us through cresize() without the
	 * window having actually changed size. */
	if (bg.scaled && bg.w == w && bg.h == h)
		return;

	if (bg.scaled) {
		imlib_context_set_image(bg.scaled);
		imlib_free_image();
		bg.scaled = NULL;
	}

	imlib_context_set_image(bg.src);
	iw = imlib_image_get_width();
	ih = imlib_image_get_height();

	bgimg_cover(iw, ih, w, h, &sx, &sy, &sw, &sh);
	bg.scaled = imlib_create_cropped_scaled_image(sx, sy, sw, sh, w, h);
	/* Leave bg.w/h alone on failure: while bg.scaled is NULL they must keep
	 * their previous (or zero) value, so that the invariant holds and
	 * bgimg_reblend() clears pm to None instead of handing out a pixmap
	 * sized for the old window. */
	if (!bg.scaled)
		return;
	bg.w = w;
	bg.h = h;
}

void
bgimg_reblend(float a, float imga, unsigned long bgpixel)
{
	uint32_t *sdata, *ddata, rgb;
	XImage *xi;
	GC gc;
	XGCValues gcv;
	int i, n, premul;

	if (bg.pm != None) {
		XFreePixmap(bg.dpy, bg.pm);
		bg.pm = None;
	}

	if (!bg.scaled || bg.w <= 0 || bg.h <= 0)
		return;

	imlib_context_set_image(bg.scaled);
	sdata = imlib_image_get_data_for_reading_only();
	if (!sdata)
		return;

	n = bg.w * bg.h;
	if (!(ddata = malloc(n * sizeof(*ddata))))
		return;

	/* Without depth 32 there is no alpha channel (-w embed). Skip premultiply. */
	premul = (bg.depth == 32);
	rgb = (uint32_t)(bgpixel & 0x00FFFFFF);

	for (i = 0; i < n; i++)
		ddata[i] = bgimg_blendpx(sdata[i], rgb, imga, a, premul);

	xi = XCreateImage(bg.dpy, bg.vis, bg.depth, ZPixmap, 0,
	                  (char *)ddata, bg.w, bg.h, 32, bg.w * 4);
	if (!xi) {
		free(ddata);
		return;
	}

	bg.pm = XCreatePixmap(bg.dpy, bg.parent, bg.w, bg.h, bg.depth);

	memset(&gcv, 0, sizeof(gcv));
	gcv.graphics_exposures = False;
	gc = XCreateGC(bg.dpy, bg.pm, GCGraphicsExposures, &gcv);
	XPutImage(bg.dpy, bg.pm, gc, xi, 0, 0, 0, 0, bg.w, bg.h);
	XFreeGC(bg.dpy, gc);

	/* XDestroyImage frees ddata as well */
	XDestroyImage(xi);
}

Pixmap
bgimg_pixmap(void)
{
	return bg.pm;
}
