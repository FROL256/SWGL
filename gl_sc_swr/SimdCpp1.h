//
// Created by frol on 13.10.18.
//

#ifndef TEST_GL_TOP_SIMDCPP1_H
#define TEST_GL_TOP_SIMDCPP1_H

#include "PureCpp.h"

struct HWImplSimdCpp1 : public HWImplementationPureCpp
{
  static void RasterizeTriangle(RasterOp a_ropT, BlendOp a_bopT, const TriangleType& tri, int tileMinX, int tileMinY,
                                FrameBuffer* frameBuf);
};

#endif //TEST_GL_TOP_SIMDCPP1_H
