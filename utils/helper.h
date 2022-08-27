/**
 * Authors: Charlotte Park, cispark@mit.edu and Isabel Rosa, isrosa@mit.edu
 **/

#ifndef HELPER_H
#define HELPER_H

#include "../render.h"
#include "../simulate.h"

void exportFramesRender(sphere *spheres, int numSpheres, vector e, vector u,
                        vector v, int numLights, light *lights, int nFrames);

void exportFramesSimulate(sphere *spheres, int numSpheres, vector e, vector u,
                          vector v, int numLights, light *lights, int nFrames);

#endif
