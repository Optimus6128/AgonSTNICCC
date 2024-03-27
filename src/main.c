#include "agon.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <mos_api.h>

#include "fx_anim.h"


char resolutionSelection = 0;
static uint8_t screenModes[4] = {1, 137,132,0};

typedef void (*vblank_handler_t)();
vblank_handler_t prevHandler;

void keyEventHandler(KEY_EVENT keyEvent);

static bool quit = false;
static bool noVsync = true;

static uint16_t lastSecRefresh = 0;
static uint16_t prevRefresh = 0;
static uint16_t nextRefresh = 0;

static uint16_t frameStart;

// vblank.asm
extern void vblank_handler();


// called from vblank_handler
void on_vblank()
{
	++nextRefresh;
}

void keyEventHandler(KEY_EVENT keyEvent)
{
	static KEY_EVENT prevKeyEvent;

	if (keyEvent.key_data == prevKeyEvent.key_data) return;
	prevKeyEvent = keyEvent;

	if (keyEvent.code == 0x7d) {	// Escape
		quit = true;
	}

	if (keyEvent.down) {
		switch (keyEvent.code) {
			case 0x01:	// Space
				noVsync = !noVsync;
			break;
		}
	}
}


static void agonInit()
{
	vdp_clear_screen();

	vdp_key_init();
	vdp_set_key_event_handler(keyEventHandler);
}

static void agonDeInit()
{
	agon_clearBuffer(-1);

	agon_set_video_mode(1);		// default system mode
	vdp_cursor_enable(true);
	vdp_logical_scr_dims(true);

	mos_setintvector(0x32, prevHandler);
}

static bool initDemo()
{
	agon_setCursorPosition(0,0);
	
	printf("\nChoose resolution:\n\n");
	printf("\t1) 320x240 - 16 colors (double buffered)\n");
	printf("\t2) 640x240 - 16 colors (double buffered)\n");
	printf("\t3) 640x480 - 16 colors (single buffered)\n\n");

	do {
		resolutionSelection = getch() - '0';
	} while (!(resolutionSelection > 0 && resolutionSelection < 4));

	agon_set_video_mode(screenModes[(int)resolutionSelection]);
	
	vdp_cursor_enable(false);
	vdp_logical_scr_dims(false);

	prevHandler = mos_setintvector(0x32, &vblank_handler);

	if (!fxAnimInit()) {
		return false;
	}

	frameStart = nextRefresh;
	return true;
}

static void drawFps()
{
	static uint16_t fps = 0;
	static uint16_t framesPassed = 0;

	agon_set_tcol(3);

	agon_setCursorPosition(0,0);
	printf("%d  ", fps);

	if (nextRefresh - lastSecRefresh >= 60) {	// assumes 60hz mode
		lastSecRefresh = nextRefresh;
		fps = framesPassed;
		framesPassed = 0;
	}

	++framesPassed;
}

static void drawBenchResult()
{
	static int benchResult = 0;
	static bool benchResultCalculated = false;

	if (benchResultCalculated) {
		agon_setCursorPosition(0,29);
		printf("%d", benchResult);
	} else {
		benchResult = (1800 * 60) / (nextRefresh - frameStart);
		benchResultCalculated = true;
	}
}

static void runDemo()
{
	while(!quit) {	// not ESC, but is it from keyboard matrix?
		if (noVsync || prevRefresh != nextRefresh) {
			prevRefresh = nextRefresh;

			fxAnimRun();

			if (animationLoaded) {
				drawFps();
			}
			if (animationLoopedOnce) {
				drawBenchResult();
			}

			agon_swapBuffers();
		}

		// Janky solution for now
		if (noVsync) {
			uint8_t i = 32;
			do {
				vdp_update_key_state();
			} while(i-- !=0);
		} else {
			vdp_update_key_state();
		}
	};

	fxAnimFree();
}

int main(void)
{
	agonInit();

	if (initDemo()) {
		runDemo();
	}

	agonDeInit();

	return 0;
}
