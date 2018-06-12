#pragma once

#include "PureCpp.h"

struct HWImplBlock4x4 : public HWImplementationPureCpp
{
  static void RasterizeTriangle(ROP_TYPE a_ropT, const TriangleType& tri, int tileMinX, int tileMinY, 
                                FrameBuffer* frameBuf);
};

