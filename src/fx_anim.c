#include "fx_anim.h"
#include "scene1.h"

#include "agon.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <mos_api.h>


typedef struct Point2D
{
	uint8_t x, y;
}Point2D;

typedef struct Triangle
{
	Point2D p0, p1, p2;
	uint8_t c;
}Triangle;

typedef struct vec2i
{
	uint8_t x, y;
}vec2i;


static uint8_t block64index = 0;

static uint8_t *data = scene1_bin;

static Point2D pt[16];

static Triangle triangles[1024];
static uint8_t nextTriangle = 0;

static bool mustClearScreen = false;


static void renderPolygons()
{
	for (uint8_t i=0; i<nextTriangle; ++i) {
		Triangle *tri = &triangles[i];
		agon_draw_triangle(tri->p0.x, tri->p0.y, tri->p1.x, tri->p1.y, tri->p2.x, tri->p2.y, tri->c);
	}
}

static void addPolygon(Point2D *point, uint8_t numVertices, uint8_t colorIndex)
{
	const uint8_t numTriangles = numVertices - 2;

	Point2D *ptBase = point++;

	for (uint8_t i=0; i<numTriangles; ++i) {
		Triangle *trianglePtr = &triangles[nextTriangle++];
		trianglePtr->p0.x = ptBase->x;		trianglePtr->p0.y = ptBase->y;
		trianglePtr->p1.x = point->x;      	trianglePtr->p1.y = point->y;	point++;
		trianglePtr->p2.x = point->x;    	trianglePtr->p2.y = point->y;
		trianglePtr->c = colorIndex;
	}
}

static void interpretPaletteData()
{
	uint8_t bitmaskH = *data++;
	uint8_t bitmaskL = *data++;

	uint16_t bitmask = (bitmaskH << 8) | bitmaskL;

	for (uint8_t i = 0; i < 16; ++i) {
		uint8_t palNum = i;
		if (bitmask & 0x8000) {
			uint8_t colorH = *data++;
			uint8_t colorL = *data++;

			uint16_t color = (colorH << 8) | colorL;

			uint8_t r = (color >> 8) & 7;
			uint8_t g = (color >> 4) & 7;
			uint8_t b = color & 7;
			
			//setSingleColorPal(palNum, r << 3, g << 3, b << 3);
		}
		bitmask <<= 1;
	}
}

static void interpretDescriptorSpecial(uint8_t descriptor)
{
	switch (descriptor)
	{
	case 0xff:
	{
		// End of frame
	}
	break;

	case 0xfe:
	{
		// End of frame and skip at next 64k block

		++block64index;
		data = &scene1_bin[block64index << 16];
	}
	break;

	case 0xfd:
	{
		// That's all folks!

		data = &scene1_bin[0];
		block64index = 0;
	}
	break;
	}
}

static void interpretDescriptorNormal(uint8_t descriptor, uint8_t *polyNumVertices, uint8_t *colorIndex)
{
	*colorIndex = (uint8_t)((descriptor >> 4) & 15);
	*polyNumVertices = (uint8_t)(descriptor & 15);
}

static void interpretIndexedMode()
{
	static Point2D vi[256];

	uint8_t descriptor = 0;
	uint8_t polyPaletteIndex, polyNumVertices;

	uint8_t vertexNum = *data++;

	for (uint8_t i = 0; i < vertexNum; ++i) {
		vi[i].x = *data++;
		vi[i].y = *data++;
	}

	while(true) {
		descriptor = *data++;
		if (descriptor >= 0xfd) break;

		interpretDescriptorNormal(descriptor, &polyNumVertices, &polyPaletteIndex);

		for (uint8_t n = 0; n < polyNumVertices; ++n) {
			uint8_t vertexId = *data++;

			pt[n].x = vi[vertexId].x;
			pt[n].y = vi[vertexId].y;
		}
		addPolygon(pt, polyNumVertices, polyPaletteIndex);
	}
	interpretDescriptorSpecial(descriptor);
}

static void interpretNonIndexedMode()
{
	uint8_t descriptor = 0;
	uint8_t polyPaletteIndex, polyNumVertices;

	while (true) {
		descriptor = *data++;
		if (descriptor >= 0xfd) break;

		interpretDescriptorNormal(descriptor, &polyNumVertices, &polyPaletteIndex);

		for (uint8_t n = 0; n < polyNumVertices; ++n) {
			pt[n].x = *data++;
			pt[n].y = *data++;
		}
		addPolygon(pt, polyNumVertices, polyPaletteIndex);
	}
	interpretDescriptorSpecial(descriptor);
}


static void decodeFrame()
{
	uint8_t flags = *data++;

	mustClearScreen = false;
	nextTriangle = 0;

	if (flags & 1) {
		mustClearScreen = true;
	} 
	if (flags & 2) {
		interpretPaletteData();
	}
	if (flags & 4) {
		interpretIndexedMode();
	}
	else {
		interpretNonIndexedMode();
	}
}

void fxAnimRun()
{
	decodeFrame();
	
	if (mustClearScreen) {
		vdp_clear_graphics();
	}
	renderPolygons();
}
