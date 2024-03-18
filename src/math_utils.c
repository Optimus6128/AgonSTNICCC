#include "math_utils.h"

uint8_t *sinBase;

uint8_t* getSinBase()
{
	return sinBase;
}

void initMathUtils()
{
	int i;

	sinBase = malloc(SINEBASE_SIZE);
	for (i=0; i<SINEBASE_SIZE; ++i) {
		sinBase[i] = (uint8_t)(sin((double)i / (256.0 / (2.0 * M_PI))) * ((SINEBASE_HEIGHT - 1) + 0.5) + 127.5);
	}
}
