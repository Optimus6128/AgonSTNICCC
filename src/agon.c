#include "agon.h"

#include <stdio.h>
#include <stdlib.h>

char fullPalBuffer[6*16] = {
	19, 0, -1, 0, 0, 0,
	19, 1, -1, 0, 0, 0,
	19, 2, -1, 0, 0, 0,
	19, 3, -1, 0, 0, 0,
	19, 4, -1, 0, 0, 0,
	19, 5, -1, 0, 0, 0,
	19, 6, -1, 0, 0, 0,
	19, 7, -1, 0, 0, 0,
	19, 8, -1, 0, 0, 0,
	19, 9, -1, 0, 0, 0,
	19, 10, -1, 0, 0, 0,
	19, 11, -1, 0, 0, 0,
	19, 12, -1, 0, 0, 0,
	19, 13, -1, 0, 0, 0,
	19, 14, -1, 0, 0, 0,
	19, 15, -1, 0, 0, 0
};


void agon_set_tcol(uint8_t color)
{
	char buffer_set_color[] = {17, color};

	VDP_WRITE(buffer_set_color, sizeof(buffer_set_color))
}

void agon_set_gcol(uint8_t mode, uint8_t color)
{
  char buffer_set_color[] = {18, mode, color};

  VDP_WRITE(buffer_set_color, sizeof(buffer_set_color))
}

void agon_plot_pixel(int16_t x, int16_t y, uint8_t color)
{
  char buffer_set_color[] = {18, 0, color};
  VDP_WRITE(buffer_set_color, sizeof(buffer_set_color))

  char buffer_plot_pixel[] = {25, 69, LH_ARGS(x), LH_ARGS(y)};
  VDP_WRITE(buffer_plot_pixel, sizeof(buffer_plot_pixel))
}

void agon_fill_rectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color)
{
	char buffer[] = { 18,0,color, 25,69,LH_ARGS(x0),LH_ARGS(y0), 25,101,LH_ARGS(x1),LH_ARGS(y1) };

	VDP_WRITE(buffer, sizeof(buffer));
}

void agon_draw_triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t color)
{
	char buffer[] = { 18,0,color, 25,69,LH_ARGS(x0),LH_ARGS(y0), 25,69,LH_ARGS(x1),LH_ARGS(y1), 25,85,LH_ARGS(x2),LH_ARGS(y2) };

	VDP_WRITE(buffer, sizeof(buffer));
}

void agon_set_video_mode(uint8_t mode)
{
  char buffer[] = {22, (char)mode};

  VDP_WRITE(buffer, sizeof(buffer))
}

void agon_setCursorPosition(uint8_t x, uint8_t y)
{
  char buffer[] = {31, x, y};

  VDP_WRITE(buffer, sizeof(buffer))
}

void agon_swapBuffers()
{
	char buffer[] = {23, 0, 0xC3};

	VDP_WRITE(buffer, sizeof(buffer))
}

void agon_clearBuffer(uint16_t bufferId)
{
	char buffer[] = {23, 0, 0xA0, LH_ARGS(bufferId), 2 };

	VDP_WRITE(buffer, sizeof(buffer))
}

void agon_createBuffer(uint16_t bufferId, uint16_t length)
{
	char buffer[] = {23, 0, 0xA0, LH_ARGS(bufferId), 3, LH_ARGS(length)};

	VDP_WRITE(buffer, sizeof(buffer))
}

void agon_selectBitmapFromBufferId(uint16_t bufferId, uint16_t width, uint16_t height, uint8_t type)
{
	char buffer[] = {	23, 27, 0x20, LH_ARGS(bufferId),
						23, 27, 0x21, LH_ARGS(width), LH_ARGS(height), type};

	VDP_WRITE(buffer, sizeof(buffer))
}

void setPal(uint8_t index, uint8_t r, uint8_t g, uint8_t b)
{
	char *buffer = &fullPalBuffer[6 * index];

	buffer[3] = r;
	buffer[4] = g;
	buffer[5] = b;
}

void setPal8(uint8_t index, uint8_t c)
{
	char buffer[] = {19, index, c, 0, 0, 0 };

	VDP_WRITE(buffer, sizeof(buffer))
}

void updatePal()
{
	VDP_WRITE(fullPalBuffer, sizeof(fullPalBuffer))
}

void agon_call_buffer(uint16_t id)
{
	char buffer[] = { 23,0,0xA0, LH_ARGS(id), 1 };

	VDP_WRITE(buffer, sizeof(buffer));
}

