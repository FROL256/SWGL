#pragma once

#include "vfloat4_x64.h"

#include <cstdint>
#include "LiteMath.h"

#include "TriRaster.h"

struct SWGL_Context;
struct Batch;


using cvex::vfloat4;
using cvex::vint4;

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
      ropId      = ROP_FillColor;
      bopId      = BlendOp_None;
    }

    vfloat4 v1;
    vfloat4 v2;
    vfloat4 v3;

    vfloat4 c1;
    vfloat4 c2;
    vfloat4 c3;

    vfloat4 t1;
    vfloat4 t2;
    vfloat4 t3;

    vfloat4 tex_txwh;
    
    int bb_iminX;
    int bb_imaxX;
    int bb_iminY;
    int bb_imaxY;
    
    TexSampler texS;

    int        psoId;
    RasterOp   ropId;
    BlendOp    bopId;
    uint8_t curr_sval;
    uint8_t curr_smask;
  };

  typedef TriangleDataSSE TriangleType;

  static void memset32(int32_t* a_data, int32_t val, int32_t numElements);

  static bool AABBTriangleOverlap(const TriangleType& a_tri, const int tileMinX, const int tileMinY, const int tileMaxX, const int tileMaxY);

  static inline bool TriVertsAreOfSameColor(const TriangleType& a_tri) { return false; }

  static void VertexShader(const float* v_in4f, float* v_out4f, int a_numVert, 
                           const float viewportData[4], const float worldViewProjMatrix[16]);

  static void TriangleSetUp(const SWGL_Context* a_pContext, const Batch* pBatch, int i1, int i2, int i3,
                            TriangleType* t1);

  static void RasterizeTriangle(const TriangleType& tri, int tileMinX, int tileMinY,
                                FrameBuffer* frameBuf);
};
