#pragma once
#define NUM_THREADS     2
#define NUM_THREADS_AUX (NUM_THREADS-1)

//#define TEX_NEAREST
#define FILL_COLOR_ONLY

#define PERSP_CORRECT

//#define MEASURE_NOLOAD_PERF
#define MEASURE_STATS

#define BIN_SIZE  64

#ifndef FULL_GL
  #define FULL_GL
#endif

#define FB_BILLET_SIZE 8

