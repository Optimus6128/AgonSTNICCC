#ifndef AGON_H
#define AGON_H

#include <agon/vdp_vdu.h>
#include <agon/vdp_key.h>

#include <mos_api.h>

#include <stdio.h>
#include <stdlib.h>


#define FAST_VDP

#ifdef FAST_VDP
	extern void fast_vdu(char *data, int len);
	#define VDP_WRITE(source,length) fast_vdu(source, length);
#else
	#define VDP_WRITE(source,length) mos_puts(source, length, ' ');
#endif


enum {
	BUFFER_BITMAP_RGBA8888 = 0,
	BUFFER_BITMAP_RGBA2222 = 1,
	BUFFER_BITMAP_MONOCHROME = 2
};


#define CLAMP(x,y,z) if (x<y) x = y; if (x>z) x = z;
#define SET_PAL_BITS(c,r,g,b) setPal(c, r << 6, g << 6, b << 6);

#define HB(x) (char)(x >> 8)
#define LB(x) (char)(x & 0xFF)
#define LH_ARGS(x) LB(x),HB(x)


void agon_clearScreen();
void agon_clearGScreen();
void agon_plot_pixel(int16_t x, int16_t y, uint8_t color);
void agon_draw_triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t color);
void agon_set_video_mode(uint8_t mode);
void agon_swapBuffers();

void setPal(uint8_t color, uint8_t r, uint8_t g, uint8_t b);
void setPal8(uint8_t index, uint8_t c);
void updatePal();

void agon_clearBuffer(uint16_t bufferId);
void agon_writeBlockToBuffer(uint16_t bufferId, uint16_t length);

void agon_createBuffer(uint16_t bufferId, uint16_t length);

void agon_selectBitmapFromBufferId(uint16_t bufferId, uint16_t width, uint16_t height, uint8_t type);

void agon_setCursorPosition(uint8_t x, uint8_t y);

void initAgonFunctions();
char *getBigBuffer();

#endif
