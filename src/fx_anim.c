#include "fx_anim.h"


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


static uint8_t *scene1_bin;
static uint8_t block64index;

static uint8_t *data;

static Point2D pt[16];

static Triangle *triangles;
static uint8_t nextTriangle;

static FILE *animFile;
static int animFileSize;

static char *trianglesBuffer;


#ifdef HIGH_RES
	#define SCALE_X 2
#else
	#define SCALE_X 1
#endif

#define OFFSET_X (SCALE_X * 32)
#define OFFSET_Y (SCALE_X-1) * 20


#define VDP270_OR_ABOVE

static char triBuffer[] = { 18,0,0, 25,69,0,0,0,0, 25,69,0,0,0,0, 25,85,0,0,0,0 };
static char polyBuffer[] = { 18,0,0, 25,69,0,0,0,0, 25,69,0,0,0,0, 25,221,0,0,0,0, 25,221,0,0,0,0, 25,221,0,0,0,0, 25,221,0,0,0,0, 25,221,0,0,0,0, 25,221,0,0,0,0, 25,221,0,0,0,0, 25,221,0,0,0,0, 25,221,0,0,0,0, 25,221,0,0,0,0, 25,221,0,0,0,0, 25,221,0,0,0,0, 25,221,0,0,0,0, 25,221,0,0,0,0 };	// 16 points max

static char vdpBuffer[8192];
static int vdpBufferIndex = 0;

static void copyToVdbBuffer(char *src, int size)
{
	memcpy(&vdpBuffer[vdpBufferIndex], src, size);
	vdpBufferIndex += size;
}

static void initTrianglesBuffer()
{
	trianglesBuffer = malloc(256 * sizeof(triBuffer));

	for (int i=0; i<256; ++i) {
		memcpy(&trianglesBuffer[i * sizeof(triBuffer)], triBuffer, sizeof(triBuffer));
	}
}

static void calcFileSize()
{
	fseek(animFile, 0, SEEK_END);
	animFileSize = ftell(animFile);
	fseek(animFile, 0, SEEK_SET);
}

static bool openAnimFileIfNeeded()
{
	if (block64index==0) {
		fclose(animFile);
		animFile = fopen("scene1a.bin", "rb");
		calcFileSize();
	}
	if (block64index==5) {
		fclose(animFile);
		animFile = fopen("scene1b.bin", "rb");
		calcFileSize();
	}

	if (!animFile) {
		printf("Error Loading File\n");
		return false;
	}

	return true;
}

static bool initAnimFileStream()
{
	return openAnimFileIfNeeded();
}

static void loadNextBlock()
{
	openAnimFileIfNeeded();

	const int blockStart = (int)(block64index % 5) * 65536;
	int blockSize = 65536;
	if (blockStart + blockSize > animFileSize) {
		blockSize = animFileSize - blockStart;
	}

	data = scene1_bin;
	//fseek(animFile, blockStart, SEEK_SET);
	fread(data,blockSize, 1, animFile);
}

static void renderPolygons()
{
	Triangle *tri = triangles;
	char *dst = trianglesBuffer;

	for (uint8_t i=0; i<nextTriangle; ++i) {
		const int16_t x0 = OFFSET_X + SCALE_X * (int16_t)tri->p0.x;
		const int16_t y0 = OFFSET_Y + tri->p0.y;
		const int16_t x1 = OFFSET_X + SCALE_X * (int16_t)tri->p1.x;
		const int16_t y1 = OFFSET_Y + tri->p1.y;
		const int16_t x2 = OFFSET_X + SCALE_X * (int16_t)tri->p2.x;
		const int16_t y2 = OFFSET_Y + tri->p2.y;

		dst[2] = tri->c;
		*((int16_t*)&dst[5]) = x0;
		*((int16_t*)&dst[7]) = y0;
		*((int16_t*)&dst[11]) = x1;
		*((int16_t*)&dst[13]) = y1;
		*((int16_t*)&dst[17]) = x2;
		*((int16_t*)&dst[19]) = y2;

		++tri;
		dst += sizeof(triBuffer);
	}

	copyToVdbBuffer(trianglesBuffer, nextTriangle * sizeof(triBuffer));
}

static void addPolygon(Point2D *point, uint8_t numVertices, uint8_t colorIndex)
{
	#ifndef VDP270_OR_ABOVE
		const uint8_t numTriangles = numVertices - 2;

		Point2D *ptBase = point++;

		for (uint8_t i=0; i<numTriangles; ++i) {
			Triangle *trianglePtr = &triangles[nextTriangle++];
			trianglePtr->p0.x = ptBase->x;		trianglePtr->p0.y = ptBase->y;
			trianglePtr->p1.x = point->x;      	trianglePtr->p1.y = point->y;	point++;
			trianglePtr->p2.x = point->x;    	trianglePtr->p2.y = point->y;
			trianglePtr->c = colorIndex;
		}
	#else
		int16_t *dst16 = (int16_t*)&polyBuffer[5];

		polyBuffer[2] = colorIndex;
		for (uint8_t i=0; i<numVertices; ++i) {
			*dst16++ = (int16_t)(OFFSET_X + SCALE_X * (int16_t)point->x);
			*dst16 = (int16_t)(OFFSET_Y + point->y);
			point++;
			dst16 += 2;
		}
		copyToVdbBuffer(polyBuffer, 3 + numVertices * 6);
	#endif
}

static void vdpBuffer_setPal(uint8_t index, uint8_t r, uint8_t g, uint8_t b)
{
	vdpBuffer[vdpBufferIndex++] = 19;
	vdpBuffer[vdpBufferIndex++] = index;
	vdpBuffer[vdpBufferIndex++] = -1;
	vdpBuffer[vdpBufferIndex++] = r;
	vdpBuffer[vdpBufferIndex++] = g;
	vdpBuffer[vdpBufferIndex++] = b;
}

static void interpretPaletteData()
{
	uint8_t bitmaskH = *data++;
	uint8_t bitmaskL = *data++;

	uint16_t bitmask = ((uint16_t)bitmaskH << 8) | (uint16_t)bitmaskL;

	for (uint8_t i = 0; i < 16; ++i) {
		uint8_t palNum = i;
		if (bitmask & 0x8000) {
			uint8_t colorH = *data++;
			uint8_t colorL = *data++;

			uint16_t color = (colorH << 8) | colorL;

			uint8_t r = (color >> 8) & 7;
			uint8_t g = (color >> 4) & 7;
			uint8_t b = color & 7;
			
			vdpBuffer_setPal(palNum, r<<5, g<<5, b<<5);
		}
		bitmask <<= 1;
	}
	//updatePal();
}

static void interpretDescriptorSpecial(uint8_t descriptor)
{
	switch (descriptor)
	{
		case 0xff:
			// End of frame
		break;

		case 0xfe:
			// End of frame and skip at next 64k block

			++block64index;
			loadNextBlock();
		break;

		case 0xfd:
			// That's all folks!

			block64index = 0;
			//fseek(animFile, 0, SEEK_SET);
			loadNextBlock();
		break;

		default:
			printf("Something's wrong..\n");
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

	nextTriangle = 0;
	vdpBufferIndex = 0;

	if (flags & 1) {
		//vdp_clear_graphics();
		//agon_fill_rectangle(0,0, 639, 239, 0);
		vdpBuffer[vdpBufferIndex++] = 16;
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


bool fxAnimInit()
{
	nextTriangle = 0;
	block64index = 0;
	scene1_bin = malloc(65536);
	triangles = malloc(256 * sizeof(Triangle));

	initTrianglesBuffer();
	if (!openAnimFileIfNeeded()) {
		return false;
	}

	loadNextBlock();

	return true;
}

void fxAnimRun()
{
	decodeFrame();

	#ifndef VDP270_OR_ABOVE
		// If not VDP270, addPolygon from decodeFrame will have also split the n-gons to triangles in a structure for regular per polygon rendering
		// Else n-gons will be rendered earlier through the Fill-Path VDP 270 command, so we can skip this
		renderPolygons();
	#endif

	VDP_WRITE(vdpBuffer, vdpBufferIndex);
}

void fxAnimFree()
{
	fclose(animFile);

	free(trianglesBuffer);
	free(scene1_bin);
	free(triangles);
}
