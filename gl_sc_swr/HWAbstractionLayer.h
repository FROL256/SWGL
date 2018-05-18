#pragma once

#include <cstdint>
#include "LiteMath.h"

#include "TriRaster.h"
#include "swgl.h"

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

struct HWImplementationPureCpp
{
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