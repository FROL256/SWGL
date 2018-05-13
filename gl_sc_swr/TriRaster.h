#ifndef TRI_RASTER_GUARDIAN
#define TRI_RASTER_GUARDIAN

#include "LiteMath.h"
#include <cstdint>

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

  float*   getZBuffer() { return zbuffer; }
  uint8_t* getSBuffer() { return sbuffer; }

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


struct TriangleDataNoSSE
{
  TriangleDataNoSSE()
  {
    texS.data = nullptr;
    texS.w = 0;
    texS.h = 0;

    curr_sval  = 0;
    curr_smask = 0;
    psoId = -1;
  }

  TexSampler texS;

  float4 v1;
  float4 v2;
  float4 v3;

  float4 c1;
  float4 c2;
  float4 c3;

  float2 t1;
  float2 t2;
  float2 t3;

  float  triAreaInv;
  float  triArea;

  int bb_iminX;
  int bb_imaxX;
  int bb_iminY;
  int bb_imaxY;

  uint8_t curr_sval;
  uint8_t curr_smask;
  int psoId;
};


#ifdef ENABLE_SSE

struct ALIGNED16 TriangleDataYesSSE
{
  TriangleDataYesSSE()
  {
    texS.data = nullptr;
    texS.w = 0;
    texS.h = 0;

    curr_sval  = 0;
    curr_smask = 0;
    psoId      = -1;
  }

  __m128 edgeTest;
  __m128 v1v3v2X;
  __m128 v3v2v1Z;

  __m128 kv1;

  __m128 cv1;
  __m128 cv2;
  __m128 cv3;

  __m128 tv1;
  __m128 tv2;
  __m128 tv3;

  TexSampler texS;

  float4 v1;
  float4 v2;
  float4 v3;

  float3 k1;
  float  triAreaInv;

  int bb_iminX;
  int bb_imaxX;
  int bb_iminY;
  int bb_imaxY;

  uint8_t curr_sval;
  uint8_t curr_smask;

  int psoId;
};

#endif

#ifdef ENABLE_SSE
  typedef TriangleDataYesSSE Triangle;
#else
  typedef TriangleDataNoSSE Triangle;
#endif


void rasterizeTriHalfSpace(FrameBuffer* frameBuf, const Triangle& tri, int a_tileX = 0, int a_tileY = 0); // half space rasterizer
void rasterizeTriHalfSpaceTwoLevel(FrameBuffer* frameBuf, const Triangle& tri); // Two level vectorized half space rasterizer

void rasterizeLine(FrameBuffer* frameBuf, float2 p1, float2 p2, float4 c1, float4 c2);
void rasterizePoint(FrameBuffer* frameBuf, float2 p1, float4 c1, float size);


inline bool AABBTriangleOverlap(const Triangle& a_tri, const int tileMinX, const int tileMinY, const int tileMaxX, const int tileMaxY)
{
  const bool overlapBoxBox = IntersectBoxBox(int2(a_tri.bb_iminX, a_tri.bb_iminY), int2(a_tri.bb_imaxX, a_tri.bb_imaxY),
                                             int2(tileMinX, tileMinY),             int2(tileMaxX, tileMaxY));

  return overlapBoxBox;
}



#endif
