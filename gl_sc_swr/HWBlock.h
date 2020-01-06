#pragma once

#include "HWPureCpp.h"


struct HWImplBlock4x4_CVEX : public HWImplementationPureCpp
{
  static void RasterizeTriangle(const TriangleType& tri, int tileMinX, int tileMinY,
                                FrameBuffer* frameBuf);
};


struct HWImplBlock8x2_CVEX : public HWImplementationPureCpp
{
  static void RasterizeTriangle(const TriangleType& tri, int tileMinX, int tileMinY,
                                FrameBuffer* frameBuf);
};

struct HWImplBlock16x1_CVEX : public HWImplementationPureCpp
{
  static void RasterizeTriangle(const TriangleType& tri, int tileMinX, int tileMinY,
                                FrameBuffer* frameBuf);
};


struct HWImplBlock4x4Fixp_CVEX : public HWImplementationPureCpp
{
  static void RasterizeTriangle(const TriangleType& tri, int tileMinX, int tileMinY,
                                FrameBuffer* frameBuf);
};
