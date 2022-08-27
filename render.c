/**
 * Author: Isabel Rosa, isrosa@mit.edu
 **/

#include <assert.h>
#include <cilk/cilk.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "render.h"
#include "simulate.h"

// (i, j) is point in image coordinates
// origin is location of eye
// vectors u and v span the projection plane
ray eyeToPixel(int height, int width, float i, float j, vector origin, vector u,
               vector v) {
  ray viewingRay;

  // center image frame
  float us = -width / 2 + i;
  float vs = -height / 2 + j;

  viewingRay.origin = origin;
  viewingRay.dir =
      qsubtract(qadd(scale(us, u), scale(vs, v)), viewingRay.origin);
  viewingRay.dir = scale(1 / qsize(viewingRay.dir), viewingRay.dir);

  return viewingRay;
}

// returns 1 if ray and sphere intersect, else 0
int rayToSphereIntersection(ray *r, sphere *s, float *t) {
  vector dist = qsubtract(r->origin, s->pos);
  float a = qdot(r->dir, r->dir);
  float b = 2 * qdot(r->dir, dist);
  float c = (float)((double)qdot(dist, dist) - (double)(s->r * s->r));
  float discr = (float)((double)(b * b) - (double)(4 * a * c));

  if (discr >= 0) {
    // ray hits sphere
    float sqrtdiscr = sqrtf(discr);
    float sol1 = (float)((double)-b + (double)sqrtdiscr) / 2;
    float sol2 = (float)((double)-b - (double)sqrtdiscr) / 2;
    float new_t = min(sol1, sol2);

    // if new_t > 0 and smaller than original t, we
    // found a new, closer ray-sphere intersection
    if (new_t > 0 && new_t < *t) {
      *t = new_t;
      return 1;
    }
  }

  return 0;
}

void render(float *img, int height, int width, vector e, vector u, vector v,
            int numLights, light *lights) {
  // TODO: delete this call to renderOrig and write your own, optimized code!
  renderOrig(img, height, width, e, u, v, numLights, lights);
}

void renderOrig(float *img, int height, int width, vector e, vector u, vector v,
                int numLights, light *lights) {
  ray r;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      double red = 0;
      double green = 0;
      double blue = 0;

      r = eyeToPixel(height, width, x, y, e, u, v);

      // find closest ray-sphere intersection
      float t = 20000.0Q; // approx. infinity
      int currentSphere = -1;

      for (int i = 0; i < numSpheres; i++) {
        if (rayToSphereIntersection(&r, &spheres[i], &t)) {
          currentSphere = i;
          break;
        }
      }

      if (currentSphere == -1)
        goto setpixel;

      material currentMat = spheres[currentSphere].mat;

      vector newOrigin = qadd(r.origin, scale(t, r.dir));

      // normal for new vector at intersection point
      vector n = qsubtract(newOrigin, spheres[currentSphere].pos);
      float n_size = qsize(n);
      if (n_size == 0)
        goto setpixel;
      n = scale(1 / n_size, n);

      for (int j = 0; j < numLights; j++) {
        light currentLight = lights[j];
        vector dist = qsubtract(currentLight.pos, newOrigin);
        if (qdot(n, dist) <= 0)
          continue;

        ray lightRay;
        lightRay.origin = newOrigin;
        lightRay.dir = scale(1 / qsize(dist), dist);

        // calculate Lambert diffusion
        float lambert = qdot(lightRay.dir, n);
        red += (double)(currentLight.intensity.red * currentMat.diffuse.red *
                        lambert);
        green += (double)(currentLight.intensity.green *
                          currentMat.diffuse.green * lambert);
        blue += (double)(currentLight.intensity.blue * currentMat.diffuse.blue *
                         lambert);
      }
    setpixel:
      img[(x + y * width) * 3 + 0] = min((float)red, 1.0);
      img[(x + y * width) * 3 + 1] = min((float)green, 1.0);
      img[(x + y * width) * 3 + 2] = min((float)blue, 1.0);
    }
  }
}
