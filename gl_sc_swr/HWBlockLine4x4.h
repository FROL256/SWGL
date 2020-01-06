#pragma once

#include "HWPureCpp.h"


struct HWImplBlockLine4x4_CVEX : public HWImplementationPureCpp
{
  static void RasterizeTriangle(const TriangleType& tri, int tileMinX, int tileMinY,
                                FrameBuffer* frameBuf);
};


struct HWImplBlockLine8x2_CVEX : public HWImplementationPureCpp
{
  static void RasterizeTriangle(const TriangleType& tri, int tileMinX, int tileMinY,
                                FrameBuffer* frameBuf);
};

struct HWImplBlockLine16x1_CVEX : public HWImplementationPureCpp
{
  static void RasterizeTriangle(const TriangleType& tri, int tileMinX, int tileMinY,
                                FrameBuffer* frameBuf);
};



struct HWImplBlockLine4x4Fixp_CVEX : public HWImplementationPureCpp
{
  static void RasterizeTriangle(const TriangleType& tri, int tileMinX, int tileMinY,
                                FrameBuffer* frameBuf);
};
