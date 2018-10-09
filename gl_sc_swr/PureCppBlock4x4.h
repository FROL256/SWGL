#pragma once

#include "PureCpp.h"

struct HWImplBlock4x4 : public HWImplementationPureCpp
{
  static void RasterizeTriangle(RasterOp a_ropT, BlendOp a_bopT, const TriangleType& tri, int tileMinX, int tileMinY,
                                FrameBuffer* frameBuf);
};


struct HWImplBlockLine4x4 : public HWImplementationPureCpp
{
  static void RasterizeTriangle(RasterOp a_ropT, BlendOp a_bopT, const TriangleType& tri, int tileMinX, int tileMinY,
                                FrameBuffer* frameBuf);
};
