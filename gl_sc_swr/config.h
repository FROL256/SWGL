#pragma once

//#define ENABLE_SSE
//#define ENABLE_SSE_VS

#define ENABLE_MT false

//#define TEX_NEAREST
//#define FILL_COLOR_ONLY

#define PERSP_CORRECT
#define RASTER_HALF_SPACE
//#define RASTER_HALF_SPACE_TWO_LEVEL

//#define RASTER_FAST_SCANLINE
//#define RASTER_FAST_SCANLINE_W0W1W2
//#define ENABLE_SSE_FOR_RASTER

//#define MEASURE_NOLOAD_PERF
//#define MEASURE_STATS

#define BIN_SIZE  64

/////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef FULL_GL
  #define FULL_GL
#endif


//#define LINUX_PPC
//#define LINUX_PPC_HS_INVERT_Y
