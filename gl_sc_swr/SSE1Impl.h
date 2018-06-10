#pragma once

#include <cstdint>
#include "LiteMath.h"

#include "TriRaster.h"        

#include <xmmintrin.h> // SSE
#include <emmintrin.h> // SSE2
#include <smmintrin.h> // SSE4.1

struct SWGL_Context;
struct Batch;

struct HWImpl_SSE1
{
  struct ALIGNED(16) TriangleDataSSE
  {
    TriangleDataSSE()
    {
      texS.data = nullptr;
      texS.w    = 0;
      texS.h    = 0;
    
      curr_sval  = 0;
      curr_smask = 0;
      psoId      = -1;
    }

    __m128 v1;
    __m128 v2;
    __m128 v3;
    
    __m128 c1;
    __m128 c2;
    __m128 c3;
    
    __m128 t1;
    __m128 t2;
    __m128 t3;

    __m128 tex_txwh;
    
    int bb_iminX;
    int bb_imaxX;
    int bb_iminY;
    int bb_imaxY;
    
    int     psoId;
    uint8_t curr_sval;
    uint8_t curr_smask;
    
    TexSampler texS;
  };

  typedef TriangleDataSSE TriangleType;

  static void memset32(int32_t* a_data, int32_t val, int32_t numElements);

  static bool AABBTriangleOverlap(const TriangleType& a_tri, const int tileMinX, const int tileMinY, const int tileMaxX, const int tileMaxY);

  static inline bool TriVertsAreOfSameColor(const TriangleType& a_tri) { return false; }

  static void VertexShader(const float* v_in4f, float* v_out4f, int a_numVert, 
                           const float viewportData[4], const float worldViewProjMatrix[16]);

  static void TriangleSetUp(const SWGL_Context* a_pContext, const Batch* pBatch, int i1, int i2, int i3,
                            TriangleType* t1);

  static void RasterizeTriangle(ROP_TYPE a_ropT, const TriangleType& tri, int tileMinX, int tileMinY, 
                                FrameBuffer* frameBuf);
};
