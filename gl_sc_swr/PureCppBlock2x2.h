//
// Created by frol on 24.08.18.
//

#ifndef TEST_GL_TOP_PURECPPBLOCKVZ_H
#define TEST_GL_TOP_PURECPPBLOCKVZ_H

#include "PureCpp.h"

struct HWImplBlock2x2 : public HWImplementationPureCpp
{
  static void RasterizeTriangle(RasterOp a_ropT, BlendOp a_bopT, const TriangleType& tri, int tileMinX, int tileMinY,
                                FrameBuffer* frameBuf);
};

#endif //TEST_GL_TOP_PURECPPBLOCKVZ_H
