/**
 * Authors: Charlotte Park, cispark@mit.edu and Isabel Rosa, isrosa@mit.edu
 **/

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../render.h"
#include "../simulate.h"

// image array
float testImg[3 * WIDTH * HEIGHT];

// image array for correctly rendered image
float refImg[3 * WIDTH * HEIGHT];

// counter for number of frames
int frameCounter = 0;

void exportFramesRender(sphere *spheres, int numSpheres, vector e, vector u,
                        vector v, int numLights, light *lights, int nFrames) {
  FILE *fpNew = fopen("framesRenderNew.txt", "w");
  FILE *fpOld = fopen("framesRenderOld.txt", "w");
  frameCounter = 0;
  while (frameCounter++ < nFrames) {
    simulateOrig();
    sort(spheres, numSpheres, e);
    render((float *)&testImg, HEIGHT, WIDTH, e, u, v, numLights, lights);
    renderOrig((float *)&refImg, HEIGHT, WIDTH, e, u, v, numLights, lights);
    for (int i = 0; i < 3 * WIDTH * HEIGHT; i++) {
      fprintf(fpNew, "%f ", testImg[i]);
    }
    fprintf(fpNew, "%f\n", testImg[3 * WIDTH * HEIGHT - 1]);
    for (int i = 0; i < 3 * WIDTH * HEIGHT; i++) {
      fprintf(fpOld, "%f ", refImg[i]);
    }
    fprintf(fpOld, "%f\n", refImg[3 * WIDTH * HEIGHT - 1]);
  }
  fclose(fpNew);
  fclose(fpOld);
}

void exportFramesSimulate(sphere *spheres, int numSpheres, vector e, vector u,
                          vector v, int numLights, light *lights, int nFrames) {
  FILE *fpNew = fopen("framesSimNew.txt", "w");
  FILE *fpOld = fopen("framesSimOld.txt", "w");
  frameCounter = 0;
  while (frameCounter++ < nFrames) {
    sphere spheresOG[2 * bodies];
    sphere spheresCopy[2 * bodies];
    for (int i = 0; i < 2 * bodies; i++) {
      spheresOG[i] = copySphere(spheres[i]);
    }
    simulate();
    sort(spheres, numSpheres, e);
    renderOrig((float *)&testImg, HEIGHT, WIDTH, e, u, v, numLights, lights);
    for (int i = 0; i < 2 * bodies; i++) {
      spheresCopy[i] = copySphere(spheres[i]);
      spheres[i] = copySphere(spheresOG[i]);
    }
    simulateOrig();
    sort(spheres, numSpheres, e);
    renderOrig((float *)&refImg, HEIGHT, WIDTH, e, u, v, numLights, lights);
    for (int i = 0; i < 3 * WIDTH * HEIGHT; i++) {
      fprintf(fpNew, "%f ", testImg[i]);
    }
    fprintf(fpNew, "%f\n", testImg[3 * WIDTH * HEIGHT - 1]);
    for (int i = 0; i < 3 * WIDTH * HEIGHT; i++) {
      fprintf(fpOld, "%f ", refImg[i]);
    }
    fprintf(fpOld, "%f\n", refImg[3 * WIDTH * HEIGHT - 1]);
  }
  fclose(fpNew);
  fclose(fpOld);
}
