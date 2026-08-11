/* See LICENSE for license details. */
#include <stdint.h>

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
