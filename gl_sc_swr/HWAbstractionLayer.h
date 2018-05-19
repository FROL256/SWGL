#pragma once

#include <cstdint>
#include "LiteMath.h"

#include "TriRaster.h"        

struct SWGL_Context;
struct Batch;

struct HWImplementationPureCpp
{

  struct TriangleDataNoSSE
  {
    TriangleDataNoSSE()
    {
      texS.data = nullptr;
      texS.w    = 0;
      texS.h    = 0;

      curr_sval  = 0;
      curr_smask = 0;
      psoId      = -1;
    }

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

    int     psoId;
    uint8_t curr_sval;
    uint8_t curr_smask;

    TexSampler texS;
  };

  //
  //
  typedef TriangleDataNoSSE TriangleType;

  static void memset32(int32_t* a_data, int32_t val, int32_t numElements);

  static bool AABBTriangleOverlap(const TriangleType& a_tri, const int tileMinX, const int tileMinY, const int tileMaxX, const int tileMaxY);

  static void VertexShader(const float* v_in4f, float* v_out4f, int a_numVert, 
                           const float viewportData[4], const float worldViewMatrix[16], const float projMatrix[16]);

  static void TriangleSetUp(const SWGL_Context* a_pContext, const Batch* pBatch, const FrameBuffer& frameBuff, 
                            const int i1, const int i2, const int i3,
                            TriangleType* t1);

  static void RasterizeTriangle(ROP_TYPE a_ropT, const TriangleType& tri, int tileMinX, int tileMinY, 
                                FrameBuffer* frameBuf);
};


//@select current implementation here via typedef assigment :)
typedef HWImplementationPureCpp HWImpl;



using Triangle = HWImplementationPureCpp::TriangleType;