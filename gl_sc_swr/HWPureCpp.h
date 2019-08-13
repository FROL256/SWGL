#pragma once

#include <cstdint>
#include "LiteMath.h"
#include "TriRaster.h"

struct SWGL_Context;
struct Batch;

ALIGNED(16) struct HWImplementationPureCpp
{

  struct TriangleType
  {
    TriangleType()
    {
      texS.data = nullptr;
      texS.w    = 0;
      texS.h    = 0;
      //curr_sval  = 0;
      //curr_smask = 0;
      psoId = -1;
      ropId = ROP_FillColor;
      bopId = BlendOp_None;
      fbId  = 0;
    }

    ALIGNED(16) float4 v1;
    ALIGNED(16) float4 v2;
    ALIGNED(16) float4 v3;

    ALIGNED(16) float4 c1;
    ALIGNED(16) float4 c2;
    ALIGNED(16) float4 c3;

    float2 t1;
    float2 t2;
    float2 t3;

    int bb_iminX;
    int bb_imaxX;
    int bb_iminY;
    int bb_imaxY;

    int        psoId;
    TexSampler texS;
    RasterOp   ropId;
    BlendOp    bopId;
    int        fbId;
    //uint8_t curr_sval;
    //uint8_t curr_smask;
  };


  static void memset32(int32_t* a_data, int32_t val, int32_t numElements);

  static bool AABBTriangleOverlap(const TriangleType& a_tri, const int tileMinX, const int tileMinY, const int tileMaxX, const int tileMaxY);

  static inline bool TriVertsAreOfSameColor(const TriangleType& a_tri) 
  {
    const float4 diff1 = a_tri.c1 - a_tri.c2;
    const float4 diff2 = a_tri.c1 - a_tri.c3;

    return (diff1.x < 1e-5f) && (diff1.y < 1e-5f) && (diff1.z < 1e-5f) && (diff1.w < 1e-5f) && 
           (diff2.x < 1e-5f) && (diff2.y < 1e-5f) && (diff2.z < 1e-5f) && (diff2.w < 1e-5f);
  }

  static void VertexShader(const float* v_in4f, float* v_out4f, int a_numVert, 
                           const float viewportData[4], const float worldViewProjMatrix[16]);

  static void RasterizeTriangle(const TriangleType& tri, int tileMinX, int tileMinY,
                                FrameBuffer* frameBuf);
};


