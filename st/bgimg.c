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
	Imlib_Image src;     /* 원본, 1회 디코드 */
	Imlib_Image scaled;  /* 창 크기로 cover 스케일 */
	Pixmap pm;           /* 블렌드 결과, 32bit ARGB */
	int w, h;            /* scaled/pm 의 크기 */
} bg;

/*
 * 원본 iw*ih 에서 창 ww*wh 와 같은 비율을 갖는 최대 크기의 중앙 사각형을
 * 구한다. 이 사각형을 창 크기로 늘리면 왜곡 없이 창을 꽉 채운다 (cover).
 */
void
bgimg_cover(int iw, int ih, int ww, int wh, int *sx, int *sy, int *sw, int *sh)
{
	if (iw <= 0 || ih <= 0 || ww <= 0 || wh <= 0) {
		/* 비율을 정의할 수 없다. 원본 전체를 쓴다. */
		*sx = *sy = 0;
		*sw = iw > 0 ? iw : 0;
		*sh = ih > 0 ? ih : 0;
		return;
	}

	if ((int64_t)iw * wh > (int64_t)ww * ih) {
		/* 이미지가 창보다 가로로 넓다 -> 높이를 다 쓰고 양옆을 자른다 */
		*sh = ih;
		*sw = (int)(((int64_t)ih * ww) / wh);
	} else {
		/* 이미지가 창보다 세로로 길다 -> 너비를 다 쓰고 위아래를 자른다 */
		*sw = iw;
		*sh = (int)(((int64_t)iw * wh) / ww);
	}

	*sx = (iw - *sw) / 2;
	*sy = (ih - *sh) / 2;
}

/*
 * 이미지 픽셀 하나를 배경색과 섞고 창 투명도를 입힌다.
 *   c = imga * src + (1 - imga) * bg
 *   A = 0xff * a,  RGB = premul ? c * a : c
 * src 의 알파 채널은 무시한다 (불투명 취급).
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
	/* 실패 시 bg.w/h 를 갱신하지 않는다: bg.scaled == NULL 이면 bg.w/h 도
	 * 이전(또는 0) 값을 유지해야 bgimg_reblend() 가 pm 을 None 으로 비우는
	 * 불변식이 깨지지 않는다. */
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

	/* 깊이가 32 가 아니면 알파 채널이 없다 (-w 임베드). 프리멀티플 생략. */
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

	/* XDestroyImage 가 ddata 도 free 한다 */
	XDestroyImage(xi);
}

Pixmap
bgimg_pixmap(void)
{
	return bg.pm;
}
