#ifndef FX_ANIM_H
#define FX_ANIM_H

#define HIGH_RES

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

#endif
