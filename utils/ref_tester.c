/**
 * Authors: Charlotte Park, cispark@mit.edu and Isabel Rosa, isrosa@mit.edu
 **/

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h> // For `getopt`

#include "../render.h"

#if defined(__APPLE_CC__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#define COLOR_RED "\033[0;31m"
#define COLOR_GREEN "\033[0;32m"
#define COLOR_DEFAULT "\033[0m"

#define PASS_STR COLOR_GREEN "PASS" COLOR_DEFAULT
#define FAIL_STR COLOR_RED "FAIL" COLOR_DEFAULT

// image array
float img[WIDTH * HEIGHT];

// input file
FILE *fpOld;
FILE *fpNew;

// statistics to return with heatmap
float avg;
float stdDev;
float max;
float min;

void cleanup(float *correct, float *actual, char *lineOld, char *lineNew) {
  free(correct);
  free(actual);
  free(lineOld);
  free(lineNew);
  fclose(fpOld);
  fclose(fpNew);
  printf("Average difference: %f\n", avg);
  printf("Maximum difference: %f\n", max);
  printf("Minimum difference: %f\n", min);
  printf("Standard deviation: %f\n\n", stdDev);
  if (max == 0) {
    printf(PASS_STR "\tYay! Correctness test passed.\n");
  } else {
    printf(FAIL_STR "\tCorrectness failed:(\n");
  }
}

void populate(float *correct, float *actual, char *lineOld, char *lineNew) {
  char *tokenNew = strtok(lineNew, " ");
  int i = 0;
  while (tokenNew != NULL) {
    actual[i] = atof(tokenNew);
    tokenNew = strtok(NULL, " ");
    i++;
  }

  char *tokenOld = strtok(lineOld, " ");
  int j = 0;
  while (tokenOld != NULL) {
    correct[j] = atof(tokenOld);
    tokenOld = strtok(NULL, " ");
    j++;
  }
}

void calcStatsAndRenderHeatmap(float *actual, float *correct) {
  float sum = 0;
  float sumSq = 0;
  max = 0;
  min = 1;

  for (int x = 0; x < WIDTH; x++) {
    for (int y = 0; y < HEIGHT; y++) {
      float redDiff =
          fabs(correct[(x + y * WIDTH) * 3] - actual[(x + y * WIDTH) * 3]);
      float greenDiff = fabs(correct[(x + y * WIDTH) * 3 + 1] -
                             actual[(x + y * WIDTH) * 3 + 1]);
      float blueDiff = fabs(correct[(x + y * WIDTH) * 3 + 2] -
                            actual[(x + y * WIDTH) * 3 + 2]);
      img[(x + y * WIDTH)] = (redDiff + greenDiff + blueDiff) / 3;

      sum += img[(x + y * WIDTH)];
      sumSq += img[(x + y * WIDTH)] * img[(x + y * WIDTH)];

      if (img[(x + y * WIDTH)] > max)
        max = img[(x + y * WIDTH)];
      else if (img[(x + y * WIDTH)] < min)
        min = img[(x + y * WIDTH)];
    }
  }

  avg = sum / (WIDTH * HEIGHT);
  stdDev = sqrt((sumSq - (sum * sum) / (WIDTH * HEIGHT)) / (WIDTH * HEIGHT));
}

void display(void) {
  // reset drawing window
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Draw the pixel array
  glDrawPixels(WIDTH, HEIGHT, GL_LUMINANCE, GL_FLOAT, img);

  // Reset buffer for next frame
  glutSwapBuffers();
}

void idle(void) {
  size_t lenOld = 3 * WIDTH * HEIGHT;
  size_t lenNew = 3 * WIDTH * HEIGHT;
  char *lineOld = (char *)malloc(lenOld * sizeof(char));
  char *lineNew = (char *)malloc(lenNew * sizeof(char));
  float *actual = (float *)malloc(lenNew * sizeof(float));
  float *correct = (float *)malloc(lenOld * sizeof(float));

  glutPostRedisplay();

  if (getline(&lineNew, &lenNew, fpNew) != -1 &&
      getline(&lineOld, &lenOld, fpOld) != -1) {
    populate(correct, actual, lineOld, lineNew);
  } else {
    cleanup(correct, actual, lineOld, lineNew);
    exit(0);
  }
  calcStatsAndRenderHeatmap(actual, correct);
  sleep(1);
  free(correct);
  free(actual);
}

int main(int argc, char *argv[]) {

  // Parse the CLI input!
  int opt;
  int graphics = 0;

  while ((opt = getopt(argc, argv, "hsrg")) != -1) {
    switch (opt) {
    case 'h': // Help
      goto help;
      break; // For completeness

    case 'r': // Render
      if (fpOld != NULL || fpNew != NULL) {
        goto help;
      }

      fpOld = fopen("framesRenderOld.txt", "r");
      fpNew = fopen("framesRenderNew.txt", "r");
      break;

    case 's': // Simulate
      if (fpOld != NULL || fpNew != NULL) {
        goto help;
      }

      fpOld = fopen("framesSimOld.txt", "r");
      fpNew = fopen("framesSimNew.txt", "r");
      break;

    case 'g': // Graphics
      graphics = 1;
      break;

    default:
      goto help;
      break;
    }
  }

  // There should not be any extra arguments to be parsed,
  // otherwise this likely is a malformed input
  if (optind < argc) {
    goto help;
  }

  if (fpOld == NULL || fpNew == NULL) {
    goto help;
  }

  if (graphics) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(WIDTH, HEIGHT);

    glutCreateWindow("Project 2 Error Heatmap");
    glutDisplayFunc(display);
    glutIdleFunc(idle);

    glutMainLoop();
  } else {
    size_t lenOld = 3 * WIDTH * HEIGHT;
    size_t lenNew = 3 * WIDTH * HEIGHT;
    char *lineOld = (char *)malloc(lenOld * sizeof(char));
    char *lineNew = (char *)malloc(lenNew * sizeof(char));
    float *actual = (float *)malloc(lenNew * sizeof(float));
    float *correct = (float *)malloc(lenOld * sizeof(float));

    while (getline(&lineNew, &lenNew, fpNew) != -1 &&
           getline(&lineOld, &lenOld, fpOld) != -1) {
      populate(correct, actual, lineOld, lineNew);
      calcStatsAndRenderHeatmap(actual, correct);
    }

    cleanup(correct, actual, lineOld, lineNew);
  }

  return EXIT_SUCCESS;

help:
  printf("Usage: ./ref_test [-r] [-s] [-g] [-h]\n"
         "\t"
         "-r                        \t Run tool for render function\n"
         "\t"
         "-s                        \t Run tool for simulate function\n"
         "\t"
         "-g                        \t Display graphical heatmap while "
         "reference testing\n"
         "\t"
         "-h                        \t This help message\n");

  return 1;
}
