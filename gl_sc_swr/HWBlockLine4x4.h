#pragma once

#include "HWPureCpp.h"


struct HWImplBlockLine4x4_CVEX : public HWImplementationPureCpp
{
  static void RasterizeTriangle(const TriangleType& tri, int tileMinX, int tileMinY,
                                FrameBuffer* frameBuf);
};


struct HWImplBlockLine8x8_CVEX : public HWImplementationPureCpp
{
  static void RasterizeTriangle(const TriangleType& tri, int tileMinX, int tileMinY,
                                FrameBuffer* frameBuf);
};


struct HWImplBlockLine4x4Fixp_CVEX : public HWImplementationPureCpp
{
  static void RasterizeTriangle(const TriangleType& tri, int tileMinX, int tileMinY,
                                FrameBuffer* frameBuf);
};
