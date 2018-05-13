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

struct HWImplementationExample
{
  typedef int TriangleType;
  typedef float4 f4;

  void memset32(int32_t* a_data, int32_t val, int32_t numElements);

  static inline f4 VertexTransform(const f4& v_in, const f4& viewportData);

  static inline void TriangleSetUp(const SWGL_Context* a_pContext, const Batch* pBatch, const FrameBuffer& frameBuff, 
                                   const int i1, const int i2, const int i3, bool triangleIsTextured,
                                   TriangleType* t1);

  static inline void RasterizeTriangle(ROP_TYPE a_ropT, FrameBuffer* frameBuf, const TriangleType& tri, int tileMinX, int tileMinY);
};
