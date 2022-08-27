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

double G;
int bodies, numSpheres;
sphere *spheres;

void sort(sphere *spheres, int n, vector e) {
  sphere key, copyKey;
  for (int i = 1; i < n; i++) {
    key = copySphere(spheres[i]);
    copyKey = copySphere(spheres[i + bodies]);
    int j = i - 1;

    while (j >= 0 && qdist(spheres[j].pos, e) > qdist(key.pos, e)) {
      spheres[j + 1] = copySphere(spheres[j]);
      spheres[j + bodies + 1] = copySphere(spheres[j + bodies]);
      j = j - 1;
    }
    spheres[j + 1] = key;
    spheres[j + bodies + 1] = copyKey;
  }
}

void updateAccelSphere(int i) {
  double rx = 0;
  double ry = 0;
  double rz = 0;

  spheres[i + bodies].accel = newVector(0, 0, 0);
  for (int j = 0; j < bodies; j++) {
    if (i != j) {
      vector i_minus_j = qsubtract(spheres[i].pos, spheres[j].pos);
      vector j_minus_i = scale(-1, i_minus_j);
      vector force =
          scale(G * spheres[j].mass / pow(qsize(i_minus_j), 3), j_minus_i);
      rx += (double)force.x;
      ry += (double)force.y;
      rz += (double)force.z;
    }
  }
  spheres[i + bodies].accel = newVector((float)rx, (float)ry, (float)rz);
}

void updateAccelerations() {
  for (int i = 0; i < bodies; i++) {
    updateAccelSphere(i);
  }
}

void updateVelocities(float t) {
  for (int i = 0; i < bodies; i++) {
    spheres[i + bodies].vel = qadd(spheres[i].vel, scale(t, spheres[i].accel));
  }
}

void updatePositions(float t) {
  for (int i = 0; i < bodies; i++) {
    spheres[i + bodies].pos = qadd(spheres[i].pos, scale(t, spheres[i].vel));
  }
}

// runs simulation for minCollisionTime timesteps
// perform collision between spheres at indices i and j
void doMiniStepWithCollisions(float minCollisionTime, int i, int j) {
  updateAccelerations();
  updateVelocities(minCollisionTime);
  updatePositions(minCollisionTime);

  for (int k = 0; k < bodies; k++) {
    spheres[k] = copySphere(spheres[k + bodies]);
  }

  if (i == -1 || j == -1) {
    return;
  }

  vector distVec = qsubtract(spheres[i].pos, spheres[j].pos);
  float scale1 = 2 * spheres[j].mass /
                 (float)((double)spheres[i].mass + (double)spheres[j].mass);
  float scale2 = 2 * spheres[i].mass /
                 (float)((double)spheres[i].mass + (double)spheres[j].mass);
  float distNorm = qdot(distVec, distVec);
  vector velDiff = qsubtract(spheres[i].vel, spheres[j].vel);
  vector scaledDist = scale(qdot(velDiff, distVec) / distNorm, distVec);
  spheres[i].vel = qsubtract(spheres[i].vel, scale(scale1, scaledDist));
  spheres[j].vel = qsubtract(spheres[j].vel, scale(-1 * scale2, scaledDist));
}

// check if the spheres at indices i and j collide in the next
// timeLeft timesteps
// modifies mag to contain the frame-of-reference-adjusted velocity
// of sphere j in sphere i's frame of reference
int checkForCollision(int i, int j, float timeLeft, float *mag) {
  vector distVec = qsubtract(spheres[i].pos, spheres[j].pos);
  float dist = qsize(distVec);
  float sumRadii = (float)((double)spheres[i].r + (double)spheres[j].r);

  // Shift frame of reference to act like sphere i is stationary
  vector movevec = scale(
      timeLeft,
      qsubtract(qadd(spheres[j].vel, scale(0.5 * timeLeft, spheres[j].accel)),
                qadd(spheres[i].vel, scale(0.5 * timeLeft, spheres[j].accel))));

  // Break if the length the sphere moves in timeLeft time is less than
  // distance between the centers of these spheres minus their radii
  if (qsize(movevec) < (double)dist - sumRadii ||
      (movevec.x == 0 && movevec.y == 0 && movevec.z == 0)) {
    return 0;
  }

  vector unitMovevec = scale(1 / qsize(movevec), movevec);

  // distAlongMovevec = ||distVec|| * cos(angle between unitMovevec and distVec)
  float distAlongMovevec = qdot(unitMovevec, distVec);

  // Check that sphere j is moving towards sphere i
  if (distAlongMovevec <= 0) {
    return 0;
  }

  float jToMovevecDistSq =
      (float)((double)(dist * dist) -
              (double)(distAlongMovevec * distAlongMovevec));

  // Break if the closest that sphere j will get to sphere i is more than
  // the sum of their radii
  float sumRadiiSquared = sumRadii * sumRadii;
  if (jToMovevecDistSq >= sumRadiiSquared) {
    return 0;
  }

  // We now have jToMovevecDistSq and sumRadii, two sides of a right triangle.
  // Use these to find the third side, sqrt(T)
  float extraDist = (float)((double)sumRadiiSquared - (double)jToMovevecDistSq);

  if (extraDist < 0) {
    return 0;
  }

  // Draw out the spheres to check why this is the distance sphere j moves
  // before hitting sphere i;)
  float distance = (float)((double)distAlongMovevec - (double)sqrt(extraDist));

  // Break if the distance sphere j has to move to touch sphere i is too big
  *mag = qsize(movevec);
  if (*mag < distance) {
    return 0;
  }

  return 1;
}

void newDoTimeStep(float timeStep) {
  // TODO: delete this call to doTimeStep and write your own, optimized code!
  doTimeStep(timeStep);
}

void doTimeStep(float timeStep) {
  float timeLeft = timeStep;

  // If collisions are getting too frequent, we cut time step early
  // This allows for smoother rendering without losing accuracy
  while (timeLeft > 0.000001) {
    float minCollisionTime = timeLeft;
    int indexCollider1 = -1;
    int indexCollider2 = -1;

    for (int i = 0; i < bodies; i++) {
      for (int j = i + 1; j < bodies; j++) {
        float refFrameAdjustedVelMag;
        if (checkForCollision(i, j, timeLeft, &refFrameAdjustedVelMag)) {
          // Set the time step so that the spheres will just touch
          vector movevec =
              qadd(spheres[j].vel, scale(0.5 * timeLeft, spheres[j].accel));
          float touchTimePct =
              timeLeft * qsize(movevec) / refFrameAdjustedVelMag;

          if (touchTimePct > 1) {
            touchTimePct = 1 / touchTimePct;
          }

          if ((touchTimePct * timeLeft) < minCollisionTime) {
            minCollisionTime = touchTimePct * timeLeft;
            indexCollider1 = i;
            indexCollider2 = j;
          }
        }
      }
    }

    doMiniStepWithCollisions(minCollisionTime, indexCollider1, indexCollider2);

    timeLeft = timeLeft - minCollisionTime;
  }
}

void simulateOrig() { doTimeStep(1 / log(bodies)); }

void simulate() {
  // TODO: delete this call to simulateOrig and write your own, optimized code!
  simulateOrig();
}
