/**
 * Author: Isabel Rosa, isrosa@mit.edu
 **/

#ifndef MAIN_H
#define MAIN_H

#include "utils/helper.h"

#define SET_UNUSED(v) (void)v;
#define SET_UNUSED_INT(v) v = -2;

#define MAX_TIER 80

#define COLOR_RED "\033[0;31m"
#define COLOR_GREEN "\033[0;32m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_DEFAULT "\033[0m"

#define PASS_STR COLOR_GREEN "PASS" COLOR_DEFAULT
#define FAIL_STR COLOR_RED "FAIL" COLOR_DEFAULT

// viewpoint and direction
extern vector e, viewDirection;

// basis vectors
extern vector w, u, v;

// lights info
extern int numLights;
extern light lights[MAX_NUM_LIGHTS];

// image array
extern float *img;

// counter for number of frames
extern int currFrames;

// optional paramter passed in for number of frames
extern int numFrames;

// graphics flag
extern int graphics;

void init(char *fileName, int height, int width);

uint32_t run_tester_tiers(const uint32_t tier_timeout, const uint32_t timeout,
                          const int start_n, const double increasing_ratio_of_n,
                          const int start_tier, const int highest_tier,
                          const int linear_tiers, unsigned blowthroughs);

#endif
