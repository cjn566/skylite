// Map.h

#ifndef _MAP_h
#define _MAP_h

#include "settings.h"

enum Shape
{
	HEXAGON,
	SM_TRIANGLE,
	LG_TRIANGLE,
	J_SHAPE,
	SM_CHEVRON,
	LG_CHEVRON,
	SM_TRUNC_TRI,
	LG_TRUNC_TRI
};

int shape_start_addr[6][4];

int shape_led_counts[4] = {12, 27, 27, 27};

// This function populates all the data for the rest of the pixel address array.
void mapInit() {
	int blurp = 0;
	int shape = 0;
	for (int i = 1; i < 6; i++) {
		for (int j = 0; j < 4; j++)
		{
			shape_start_addr[i][j] = blurp;
			blurp += shape_led_counts[shape];
			shape = (shape + 1) % 4;
		}
	}
}


#endif

