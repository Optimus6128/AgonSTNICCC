#ifndef FX_ANIM_H
#define FX_ANIM_H

#define EMULATOR

// New Fab Agon 0.9.34 has added support for VDP270 but totally failed with Polygon Paths, so I still have to turn it off for emulators
#ifndef EMULATOR
	#define VDP270_OR_ABOVE
#endif

#include "agon.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <mos_api.h>

bool fxAnimInit();
void fxAnimRun();
void fxAnimFree();

extern bool animationLoaded;
extern bool animationLoopedOnce;

extern char resolutionSelection;

#endif
