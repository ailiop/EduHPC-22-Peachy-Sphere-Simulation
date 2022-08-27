/**
 * Author: Isabel Rosa, isrosa@mit.edu
 **/

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

#include "../main.h"
#include "./fasttime.h"

void exitfunc(int sig) {
  printf("End execution due to 58s timeout\n");
  exit(0);
}

static uint32_t timed_eval(char *fileName, int N) {
  init(fileName, N, N);
  fasttime_t start = gettime();
  int currFrames = 0;
  while (currFrames++ < 3) {
    simulate();
    sort(spheres, numSpheres, e);
    render(img, N, N, e, u, v, numLights, lights);
  }
  fasttime_t stop = gettime();
  free(img);
  free(spheres);
  return tdiff_msec(start, stop);
}

static void print_pass_message(int tier, int N, int bodies,
                               uint32_t user_msec) {
  // For some fun!
  // Celebrations must be under 5 chars
  const char *celebrations[] = {"yay", "woot", "boyah", "skrrt",
                                "ayy", "yeee", "eoo"};
  const uint32_t ncelebrations = sizeof(celebrations) / sizeof(celebrations[0]);
  const char *const random_celebration = celebrations[rand() % ncelebrations];

  printf(PASS_STR
         " (%s!):\tTier %d :\tRan %dx%d\timage with %d bodies in %d ms\n",
         random_celebration, tier, N, N, bodies, user_msec);
}

static void print_tier_pass_message(int tier, int N, int bodies,
                                    uint32_t user_msec) {
  return print_pass_message(tier, N, bodies, user_msec);
}

static void print_tier_fail_message(int tier, int N, int bodies,
                                    uint32_t user_msec, uint32_t tier_timeout) {
  printf(FAIL_STR
         " (timeout):\tTier %d :\tRan %dx%d\timage with %d bodies in %d "
         "ms but the cutoff is %d ms\n",
         tier, N, N, bodies, user_msec, tier_timeout);
}

uint32_t run_tester_tiers(const uint32_t tier_timeout, const uint32_t timeout,
                          const int start_n, const double increasing_ratio_of_n,
                          const int start_tier, const int highest_tier,
                          const int linear_tiers, unsigned blowthroughs) {
  // Sanity check the input
  assert(highest_tier <= MAX_TIER);

  // set timer
  uint32_t MS_TO_SEC = 1000;
  signal(SIGALRM, exitfunc);
  alarm((uint32_t)(timeout / MS_TO_SEC));

  printf("Setting up test up to tier %u: ", highest_tier);

  // Generate tier sizes starting from start_n and increase by
  // increasing_ratio_of_n until we have enough tiers
  int N = start_n;
  int tier_img_sizes[2 * MAX_TIER];

  uint32_t i = 0;
  for (N = start_n; i <= MAX_TIER;
       N = (uint64_t)ceil(N * increasing_ratio_of_n / 64) * 64) {
    tier_img_sizes[i++] = N;
    tier_img_sizes[i++] = N;
  }

  uint32_t tier = start_tier;
  uint32_t linear_tier_cutoff = tier + linear_tiers;
  if (linear_tiers == -1) {
    linear_tier_cutoff = highest_tier;
  }
  if (linear_tier_cutoff > highest_tier) {
    linear_tier_cutoff = highest_tier;
  }
  int lowest_fail = -1;
  int highest_pass = -1;
  bool blowthrough_used = false;

  // Linearly Test up to linear_tier_cutoff
  printf(COLOR_YELLOW "Linear search from tier %d to %d..." COLOR_DEFAULT "\n",
         tier, linear_tier_cutoff);

  for (; tier <= linear_tier_cutoff; tier++) {
    N = tier_img_sizes[tier];
    char fileName[100];
    sprintf(fileName, "tiers/tier%d.txt", tier);

    FILE *fp = fopen((char *)&fileName, "r");
    if (fp == NULL) {
      sprintf(fileName, "/var/6172/tiers/tier%d.txt", tier);
      fp = fopen((char *)&fileName, "r");
    }
    assert(fp != NULL);
    int bodies;
    double G;
    fscanf(fp, "%lf%d", &G, &bodies);
    fclose(fp);

    const uint32_t user_msec = timed_eval((char *)&fileName, N);

    // Exit if the user time is too much, but was still correct!
    if (user_msec >= tier_timeout) {
      print_tier_fail_message(tier, N, bodies, user_msec, tier_timeout);
      if (blowthroughs > 0 && tier != linear_tier_cutoff) {
        blowthroughs--;
        blowthrough_used = true;
        printf("Blowing through this failure. Remaining blowthroughs: %u\n",
               blowthroughs);
        continue;
      } else {
        goto finish;
      }
    } else { // Success
      highest_pass = tier;
      print_tier_pass_message(tier, N, bodies, user_msec);
    }
  }

  if (highest_pass != linear_tier_cutoff) {
    printf(FAIL_STR ": Linear search had failures. Done searching.\n");
    goto finish;
  }

  // Reset lowest_fail for binary search
  lowest_fail = highest_tier + 1;

  if (lowest_fail - highest_pass > 1) {
    printf(COLOR_YELLOW "Binary search from tier %d to %d...\n" COLOR_DEFAULT,
           highest_pass, lowest_fail - 1);
    printf(COLOR_YELLOW
           "This search might be affected by outliers.\n" COLOR_DEFAULT);
    if (blowthroughs > 0 && blowthrough_used) {
      // If it's 0, nothing left
      // If it's 2, none used, user doesn't know that it exists
      printf("Remaining blowthroughs will not be used for binary search.\n");
    }

    while (lowest_fail - highest_pass > 1) {
      tier = (lowest_fail + highest_pass) / 2;
      N = tier_img_sizes[tier];
      char fileName[100];
      sprintf(fileName, "tiers/tier%d.txt", tier);

      FILE *fp = fopen((char *)&fileName, "r");
      if (fp == NULL) {
        sprintf(fileName, "/var/6172/tiers/tier%d.txt", tier);
        fp = fopen((char *)&fileName, "r");
      }
      assert(fp != NULL);
      int bodies;
      double G;
      fscanf(fp, "%lf%d", &G, &bodies);
      fclose(fp);

      const uint32_t user_msec = timed_eval((char *)&fileName, N);

      // Exit if the user time is too much, but was still correct!
      if (user_msec >= tier_timeout) {
        lowest_fail = tier;
        print_tier_fail_message(tier, N, bodies, user_msec, tier_timeout);
      } else { // Success
        highest_pass = tier;
        print_tier_pass_message(tier, N, bodies, user_msec);
      }
    }
  }

finish:
  // Print update!
  if (highest_pass >= MAX_TIER + 1) {
    printf(COLOR_GREEN "Congrats! You reached the highest tier we will test "
                       "for!!!\n" COLOR_DEFAULT);
  } else if (highest_pass == highest_tier) {
    printf(COLOR_GREEN
           "You reached the highest tier you specified!\n" COLOR_DEFAULT);
    printf(COLOR_YELLOW
           "Please run this test with a higher tier to find your maximum "
           "tier.\n" COLOR_DEFAULT);
  }

  return highest_pass;
}
