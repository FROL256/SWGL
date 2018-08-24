//
// Created by frol on 24.08.18.
//
#include "PureCppBlock2x2.h"

#include "PureCppBlock.h"
#include "RasterOperations.h"

using TriangleLocal = HWImplementationPureCpp::TriangleType;

using FillColor_1x1 = VROP< float >::FillColor;
using Colored2D_1x1 = VROP< float >::Colored2D;
using Colored3D_1x1 = VROP< float >::Colored3D;

void HWImplBlock2x2::RasterizeTriangle(RasterOp a_ropT, BlendOp a_bopT, const TriangleType& tri, int tileMinX, int tileMinY,
                                       FrameBuffer* frameBuf)
{
  RasterizeTriHalfSpace2D_Block<TriangleLocal, 2, FillColor_1x1, FillColor_1x1>
      (tri, tileMinX, tileMinY,
       frameBuf);

}