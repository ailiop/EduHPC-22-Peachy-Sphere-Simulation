/**
 * Author: Isabel Rosa, isrosa@mit.edu
 **/

#ifndef RAY_TRACER_H
#define RAY_TRACER_H

#include "simulate.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

// Width and height of out image
#define WIDTH 512
#define HEIGHT 256

// Max number of objects and lights in scene
#define MAX_NUM_SPHERES 3
#define MAX_NUM_LIGHTS 3

ray eyeToPixel(int height, int width, float i, float j, vector origin, vector u,
               vector v);

int rayToSphereIntersection(ray *r, sphere *s, float *t);

void render(float *img, int height, int width, vector e, vector u, vector v,
            int numLights, light *lights);

void renderOrig(float *img, int height, int width, vector e, vector u, vector v,
                int numLights, light *lights);

#endif
