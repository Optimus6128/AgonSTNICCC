#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <mos_api.h>


#define UNROLL_2x(f) f f
#define UNROLL_4x(f) UNROLL_2x(f) UNROLL_2x(f)
#define UNROLL_8x(f) UNROLL_4x(f) UNROLL_4x(f)
#define UNROLL_16x(f) UNROLL_8x(f) UNROLL_8x(f)
#define UNROLL_32x(f) UNROLL_16x(f) UNROLL_16x(f)
#define UNROLL_64x(f) UNROLL_32x(f) UNROLL_32x(f)
#define UNROLL_128x(f) UNROLL_64x(f) UNROLL_64x(f)
#define UNROLL_256x(f) UNROLL_128x(f) UNROLL_128x(f)

#define SINEBASE_SIZE 256
#define SINEBASE_HEIGHT 128

void initMathUtils();
uint8_t* getSinBase();

#endif
