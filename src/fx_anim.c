#include "fx_anim.h"
#include "scene1.h"

#include "agon.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <mos_api.h>


typedef struct Point2D
{
	int x, y;
}Point2D;

typedef struct QuadStore
{
	Point2D p0, p1, p2, p3;
	unsigned char c;
}QuadStore;

typedef struct vec2i
{
	int x, y;
}vec2i;


static int block64index = 0;

static unsigned char *data = scene1_bin;

static Point2D pt[16];

static QuadStore quads[1024];
static QuadStore *quadPtr;
static int numQuads = 0;

static bool mustClearScreen = false;


static void addPolygon(Point2D *pt, int numVertices, unsigned char color)
{
	int pBaseIndex = 0;
	int pStartIndex = 1;
	const int maxIndex = numVertices - 1;

	while(pStartIndex < maxIndex)
	{
		quadPtr->p0.x = pt[pBaseIndex].x;       quadPtr->p0.y = pt[pBaseIndex].y;
		quadPtr->p1.x = pt[pStartIndex].x;      quadPtr->p1.y = pt[pStartIndex].y;
		quadPtr->p2.x = pt[pStartIndex+1].x;    quadPtr->p2.y = pt[pStartIndex + 1].y;

		pStartIndex += 2;
		if (pStartIndex > maxIndex) pStartIndex = maxIndex;
		quadPtr->p3.x = pt[pStartIndex].x;      quadPtr->p3.y = pt[pStartIndex].y;

		quadPtr->c = color;

		++quadPtr;
		++numQuads;
	}
}

static void interpretPaletteData()
{
	unsigned char bitmaskH = *data++;
	unsigned char bitmaskL = *data++;

	int bitmask = (bitmaskH << 8) | bitmaskL;

	for (int i = 0; i < 16; ++i) {
		int palNum = i;
		if (bitmask & 0x8000) {
			unsigned char colorH = *data++;
			unsigned char colorL = *data++;

			int color = (colorH << 8) | colorL;

			int r = (color >> 8) & 7;
			int g = (color >> 4) & 7;
			int b = color & 7;
			
			//setSingleColorPal(palNum, r << 3, g << 3, b << 3);
		}
		bitmask <<= 1;
	}
}

static void interpretDescriptorSpecial(unsigned char descriptor)
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

static void interpretDescriptorNormal(unsigned char descriptor, int *polyNumVertices, int *colorIndex)
{
	*colorIndex = (int)((descriptor >> 4) & 15);
	*polyNumVertices = (int)(descriptor & 15);
}

static void interpretIndexedMode()
{
	static Point2D vi[256];

	unsigned char descriptor = 0;
	int polyPaletteIndex, polyNumVertices;

	int vertexNum = *data++;

	for (int i = 0; i < vertexNum; ++i) {
		vi[i].x = (int)*data++;
		vi[i].y = (int)*data++;
	}

	while(true) {
		descriptor = *data++;
		if (descriptor >= 0xfd) break;

		interpretDescriptorNormal(descriptor, &polyNumVertices, &polyPaletteIndex);

		for (int n = 0; n < polyNumVertices; ++n) {
			int vertexId = *data++;

			pt[n].x = vi[vertexId].x;
			pt[n].y = vi[vertexId].y;
		}
		addPolygon(pt, polyNumVertices, polyPaletteIndex);
	}
	interpretDescriptorSpecial(descriptor);
}

static void interpretNonIndexedMode()
{
	unsigned char descriptor = 0;
	int polyPaletteIndex, polyNumVertices;

	while (true) {
		descriptor = *data++;
		if (descriptor >= 0xfd) break;

		interpretDescriptorNormal(descriptor, &polyNumVertices, &polyPaletteIndex);

		for (int n = 0; n < polyNumVertices; ++n) {
			pt[n].x = *data++;
			pt[n].y = *data++;
		}
		addPolygon(pt, polyNumVertices, polyPaletteIndex);
	}
	interpretDescriptorSpecial(descriptor);
}


static void decodeFrame()
{
	unsigned char flags = *data++;

	mustClearScreen = false;
	numQuads = 0;
	quadPtr = &quads[0];

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

static void renderPolygons()
{
}

void fxAnimRun()
{
	decodeFrame();
	
	if (mustClearScreen) {
		//clearScreenSpecial();
	}
	renderPolygons();
}
