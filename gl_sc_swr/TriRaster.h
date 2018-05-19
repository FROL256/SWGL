#ifndef TRI_RASTER_GUARDIAN
#define TRI_RASTER_GUARDIAN

#include "LiteMath.h"
#include <cstdint>


enum ROP_TYPE { ROP_FillColor = 1,

                ROP_Colored2D,
                ROP_Colored3D,
                ROP_TexNearest2D,
                ROP_TexNearest3D,
                ROP_TexLinear2D,
                ROP_TexLinear3D,

                ROP_Colored2D_Blend,
                ROP_Colored3D_Blend,
                ROP_TexNearest2D_Blend,
                ROP_TexNearest3D_Blend,
                ROP_TexLinear2D_Blend,
                ROP_TexLinear3D_Blend,
};

#define HALF_SPACE_EPSILON -1e-3f

static inline float edgeFunction(float2 a, float2 b, float2 c) // actuattly just a mixed product ... :)
{
  return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

#ifdef ENABLE_SSE

static inline __m128 edgeFunction2(__m128 a, __m128 b, __m128 c) // actuattly just a mixed product ... :)
{
  __m128 ay = _mm_shuffle_ps(a, a, _MM_SHUFFLE(1, 1, 1, 1));
  __m128 by = _mm_shuffle_ps(b, b, _MM_SHUFFLE(1, 1, 1, 1));
  __m128 cy = _mm_shuffle_ps(c, c, _MM_SHUFFLE(1, 1, 1, 1));

   return _mm_sub_ss(_mm_mul_ss(_mm_sub_ss(c,a),   _mm_sub_ss(by,ay)),
                     _mm_mul_ss(_mm_sub_ss(cy,ay), _mm_sub_ss(b,a)));
}


#endif


#ifdef LINUX_PPC_HS_INVERT_Y

static inline inline int lineOffset(int y, int w, int h) { return (h - y - 1)*w; }
static inline inline int nextLine(int y, int w, int h)   { return y - w; }

#else

static inline int lineOffset(int y, int w, int h) { return y*w; }
static inline int nextLine(int y, int w, int h)   { return y + w; }

#endif

struct FrameBuffer
{
  FrameBuffer() : w(0), h(0), vx(0), vy(0), vw(0), vh(0),
                  cbuffer(nullptr), zbuffer(nullptr), sbuffer(nullptr) {}

  int w;
  int h;

  int vx; // viewport min x
  int vy; // viewport min y
  int vw; // viewport width
  int vh; // viewport height

  int*     cbuffer;
  float*   zbuffer;
  uint8_t* sbuffer;

  //uint8_t curr_sval;
  //uint8_t curr_smask;
};

struct ALIGNED16 TexSampler
{
#ifdef ENABLE_SSE
  __m128 txwh;
#endif

  int w;
  int h;
  int pitch;
  const int* data;
};


#endif
