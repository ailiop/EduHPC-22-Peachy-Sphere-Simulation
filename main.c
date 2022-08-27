/**
 * Author: Isabel Rosa, isrosa@mit.edu
 **/

#include <assert.h>
#include <cilk/cilkscale.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h> // For `getopt`

#include "main.h"
#include "render.h"
#include "simulate.h"
#include "utils/fasttime.h"
#include "utils/helper.h"

#if defined(__APPLE_CC__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

const uint32_t TIER_TIMEOUT = 2000;
const uint32_t TIMEOUT = 58000;
const int START_SIZE = 512;
const double GROWTH_RATE = 1.08;

const int MAX_TIER_ALLOW = MAX_TIER;
const int DEFAULT_MIN_TIER = 0;
const int DEFAULT_MAX_TIER = MAX_TIER;
const int DEFAULT_LINEAR_TIERS = MAX_TIER;
const unsigned DEFAULT_BLOWTHROUGHS = 2;

const int DEFAULT_NUM_FRAMES = 10;

// viewpoint and direction
vector e, viewDirection;

// basis vectors
vector w, u, v;

// lights info
int numLights = MAX_NUM_LIGHTS;
light lights[MAX_NUM_LIGHTS];

// image array
float *img;

// counter for number of frames
int currFrames = 0;

// optional paramter passed in for number of frames
int numFrames = -1;

// graphics flag
int graphics = -1;

void init(char *fileName, int height, int width) {
  FILE *fp = fopen(fileName, "r");
  if (fp == NULL) {
    printf("The file name you entered (%s) is invalid.\n", fileName);
    return;
  }
  assert(fp != NULL);

  img = (float *)malloc(3 * height * width * sizeof(float));

  fscanf(fp, "%lf%d", &G, &bodies);
  numSpheres = bodies;
  spheres = (sphere *)malloc(2 * bodies * sizeof(sphere));

  for (int i = 0; i < bodies; i++) {
    fscanf(fp, "%f%f%f%f%f%f%f%f%f%f%f%f", (float *)&spheres[i].r,
           (float *)&spheres[i].mass, (float *)&spheres[i].pos.x,
           (float *)&spheres[i].pos.y, (float *)&spheres[i].pos.z,
           (float *)&spheres[i].vel.x, (float *)&spheres[i].vel.y,
           (float *)&spheres[i].vel.z, &spheres[i].mat.diffuse.red,
           &spheres[i].mat.diffuse.green, &spheres[i].mat.diffuse.blue,
           &spheres[i].mat.reflection);
  }

  for (int i = 0; i < bodies; i++) {
    spheres[i + bodies] = copySphere(spheres[i]);
  }

  fclose(fp);

  // set the viewpoint and direction
  e = newVector(800, 100, 0);
  viewDirection = newVector(-1, 0, 0);

  // calculate basis vectors
  vector up = newVector(0, 0, 1);
  w = scale(1 / qsize(viewDirection), viewDirection);
  u = scale(1 / qsize(qcross(up, w)), qcross(up, w));
  v = qcross(w, u);

  lights[0] = newLight(newVector(0, 240, -100), newColor(1, 1, 1));
  lights[1] = newLight(newVector(3200, 3000, -1000), newColor(0.6, 0.7, 1));
  lights[2] = newLight(newVector(600, 0, -100), newColor(0.3, 0.5, 1));
}

void display(void) {
  // reset drawing window
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Draw the pixel array
  glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_FLOAT, img);

  // Reset buffer for next frame
  glutSwapBuffers();
}

void idle(void) {
  if (currFrames++ > numFrames) {
    exit(0);
  }

  glutPostRedisplay();

  simulate();
  sort(spheres, numSpheres, e);
  render(img, HEIGHT, WIDTH, e, u, v, numLights, lights);
}

// Allows use of arrow keys to control movement
void special(int key, int x, int y) {
  switch (key) {
  case GLUT_KEY_UP:
    e = newVector(e.x, e.y, e.z + 1);
    break;
  case GLUT_KEY_DOWN:
    e = newVector(e.x, e.y, e.z - 1);
    break;
  case GLUT_KEY_LEFT:
    e = newVector(e.x, e.y + 1, e.z);
    break;
  case GLUT_KEY_RIGHT:
    e = newVector(e.x, e.y - 1, e.z);
    break;
  }

  glutPostRedisplay();
}

// Allows keyboard commands to control parameters and movement
void keyboard(unsigned char key, int x, int y) {
  switch (key) {
  case 'h':
    printf("HELP\n");
    printf("----\n");
    printf("l - increase number of lights (max: 3)\n");
    printf("o - decrease number of lights (min: 1)\n");
    printf("s - increase number of spheres (max: number from textfile)\n");
    printf("d - decrease number of spheres (min: 1)\n");
    printf("f - move forward\n");
    printf("b - move backward\n");
    printf("c - rotate clockwise\n");
    printf("right arrow - move right\n");
    printf("left arrow - move left\n");
    printf("up arrow - move up\n");
    printf("down arrow - move down\n");
    break;
  case 'l':
    numLights += (numLights < 3) ? 1 : 0;
    break;
  case 'o':
    numLights -= (numLights > 1) ? 1 : 0;
    break;
  case 's':
    numSpheres += (numSpheres < bodies) ? 1 : 0;
    break;
  case 'd':
    numSpheres -= (numSpheres > 1) ? 1 : 0;
    break;
  case 'f':
    e = newVector(e.x - 1, e.y, e.z);
    break;
  case 'b':
    e = newVector(e.x + 1, e.y, e.z);
    break;
  case 'c':;
    float angle;
    if (viewDirection.x != 0) {
      angle = atan(viewDirection.y / viewDirection.x);
    } else if (viewDirection.y < 0) {
      angle = -3.14 / 2;
    } else if (viewDirection.y > 0) {
      angle = 3.14 / 2;
    } else {
      break;
    }
    if (viewDirection.x < 0) {
      angle = 3.14 + angle;
    }
    viewDirection =
        newVector(cos(angle + 0.1), sin(angle + 0.1), viewDirection.z);
    // calculate basis vectors
    vector up = newVector(0, 0, 1);
    w = scale(1 / qsize(viewDirection), viewDirection);
    u = scale(1 / qsize(qcross(up, w)), qcross(up, w));
    v = qcross(w, u);
    break;
  }

  glutPostRedisplay();
}

int main(int argc, char *argv[]) {
  int opt;
  int test_tiers = -1;
  int correctnessTool = -1;
  char *input_file = NULL;

  // Parse the CLI input!
  while ((opt = getopt(argc, argv, "hmgtf:n:")) != -1) {

    switch (opt) {
    case 'h': // Help
      goto help;
      break; // For completeness

    case 'f': // Input file name
      // Make sure the input is fresh
      if (input_file != NULL) { // Also triggered by `UNUSED`
        goto help;
      }

      input_file = optarg;

      // The fields that should be unused
      SET_UNUSED_INT(test_tiers);
      break;

    case 'n':                // Number of frames
      if (numFrames != -1) { // Also triggered by `UNUSED`
        goto help;
      }

      numFrames = atoi(optarg);

      SET_UNUSED_INT(test_tiers);
      break;

    case 'g':               // Flag that we want to use graphics
      if (graphics != -1) { // Also triggered by `UNUSED`
        goto help;
      }

      graphics = 1;

      SET_UNUSED_INT(correctnessTool);
      SET_UNUSED_INT(test_tiers);
      break;

    case 't':                 // Flag that we want to test
      if (test_tiers != -1) { // Also triggered by `UNUSED`
        goto help;
      }

      test_tiers = 1;

      SET_UNUSED(input_file);
      SET_UNUSED_INT(numFrames);
      SET_UNUSED_INT(graphics);
      SET_UNUSED_INT(correctnessTool);
      break;

    case 'm':                      // Flag that we want to use correctness tool
      if (correctnessTool != -1) { // Also triggered by `UNUSED`
        goto help;
      }

      correctnessTool = 1;

      SET_UNUSED_INT(graphics);
      SET_UNUSED_INT(test_tiers);
      break;
    }
  }

  // There should not be any extra arguments to be parsed,
  // otherwise this likely is a malformed input
  if (optind < argc) {
    goto help;
  }

  if (input_file == NULL) {
    input_file = "simulations/250.txt";
  }

  if (numFrames <= 0) {
    numFrames = DEFAULT_NUM_FRAMES;
  }

  if (test_tiers <= 0) {
    init(input_file, HEIGHT, WIDTH);
  }

  fasttime_t start = gettime();

  if (graphics > 0) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(WIDTH, HEIGHT);

    glutCreateWindow("Project 2 Graphical Display");
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutIdleFunc(idle);

    glutMainLoop();
  } else if (correctnessTool > 0) {
    exportFramesRender(spheres, numSpheres, e, u, v, numLights, lights,
                       numFrames);
    exportFramesSimulate(spheres, numSpheres, e, u, v, numLights, lights,
                         numFrames);
  } else if (test_tiers > 0) {
    uint32_t tier = run_tester_tiers(
        TIER_TIMEOUT, TIMEOUT, START_SIZE, GROWTH_RATE, DEFAULT_MIN_TIER,
        DEFAULT_MAX_TIER, DEFAULT_LINEAR_TIERS, DEFAULT_BLOWTHROUGHS);

    if (tier == -1) {
      printf(FAIL_STR ": too slow for any tiers\n");
    } else {
      printf("Result: reached tier %d\n", tier);
    }
  } else {
    while (currFrames++ < numFrames) {
      simulate();
      sort(spheres, numSpheres, e);
      render(img, HEIGHT, WIDTH, e, u, v, numLights, lights);
    }
  }

  fasttime_t stop = gettime();
  uint32_t time = tdiff_msec(start, stop);
  if (test_tiers <= 0) {
    printf("Num spheres: %d\nImg size: %dx%d\nNum frames: %d\n---- RESULTS "
           "----\nTime elapsed: %u ms\n---- END RESULTS ----\n",
           bodies, HEIGHT, WIDTH, numFrames, time);
  }

  // Success!
  return 0;

help:
  printf(
      "Usage: ./main [-f FILE_NAME] [-n NUM_FRAMES] [-m] [-g] [-t] [-h]\n"
      "\t"
      "-f file-name              \t Input file name                       \t "
      "Optional, may not be used with performance test flag\n"
      "\t"
      "-n num-frames             \t Number of frames to execute           \t "
      "Optional, may not be used with performance test flag\n"
      "\t"
      "-m                        \t Writes files for ref-tests            \t "
      "Optional, may not be used with performance test or graphics flag\n"
      "\t"
      "-g                        \t Runs code with graphics enabled       \t "
      "Optional, may not be used with performance or ref-tests flag\n"
      "\t"
      "-t                        \t Runs performance tests                \t "
      "Optional, no other flags may be used\n"
      "\t"
      "-h                        \t This help message\n");

  return 1;
}
