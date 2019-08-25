#pragma once

#define NUM_THREADS     1
#define NUM_THREADS_AUX (NUM_THREADS-1)

#define NUM_THREADS_CLS 2

#define NOWINDOW false

#define FILL_COLOR_ONLY

#define PERSP_CORRECT

//#define MEASURE_NOLOAD_PERF
#define MEASURE_STATS

#define BIN_SIZE  64

#ifndef FULL_GL
  #define FULL_GL
#endif

#define FB_BILLET_SIZE 8


static constexpr int   SUBPIXELBITS  = 2;
static constexpr int   SUBPIXELMULTI = (1 << SUBPIXELBITS);
static constexpr float SUBPIXELMULTF = float(SUBPIXELMULTI);


struct SWGL_Timings
{
  SWGL_Timings() { clear(); }
  
  void clear() { msClear = 0.0f; msVertexShader = 0.0f; msTriSetUp = 0.0f; msRasterAndPixelShader = 0.0f; msSwapBuffers = 0.0f; msTotal = 0.0f; msBinRaster = 0.0f; }
  
  float msClear;
  float msVertexShader;
  float msTriSetUp;
  float msRasterAndPixelShader;
  float msSwapBuffers;
  float msTotal;
  float msBinRaster;
};
