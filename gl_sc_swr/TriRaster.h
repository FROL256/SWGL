#ifndef TRI_RASTER_GUARDIAN
#define TRI_RASTER_GUARDIAN

#include "LiteMath.h"
#include <cstdint>

enum RasterOp { ROP_FillColor = 1,

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

                ROP_FillStencil,
};

enum BlendOp { BlendOp_None               = 0, // no blending, replace color
               BlendOp_AlphaOneMinusAlpha = 1, // glBlendFunc(GL_ALPHA, GL_ONE_MINUS_ALPHS)
               BlendOp_Add,                    // glBlendFunc(GL_ONE, GL_ONE)
               BlendOp_Mul,                    // glBlendFunc(GL_DST_COLOR, GL_ZERO).
             };

#define HALF_SPACE_EPSILON -1e-4f

static inline float edgeFunction(float2 a, float2 b, float2 c) // actuattly just a mixed product ... :)
{
  return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}


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

struct ALIGNED(16) TexSampler
{
  int w;
  int h;
  int pitch;
  const int* data;
};


#endif
