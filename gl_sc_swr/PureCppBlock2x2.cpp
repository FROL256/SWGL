//
// Created by frol on 24.08.18.
//
#include "PureCppBlock2x2.h"

#include "PureCppBlock.h"
#include "RasterOperations.h"

using TriangleLocal = HWImplementationPureCpp::TriangleType;

using FillColor_2x2 = VROP<4, TriangleLocal>::FillColor;
using Colored2D_2x2 = VROP<4, TriangleLocal>::Colored2D;
using Colored3D_2x2 = VROP<4, TriangleLocal>::Colored3D;

void HWImplBlock2x2::RasterizeTriangle(RasterOp a_ropT, BlendOp a_bopT, const TriangleType& tri, int tileMinX, int tileMinY,
                                       FrameBuffer* frameBuf)
{

  switch (a_ropT)
  {
    case ROP_Colored2D:
      RasterizeTriHalfSpace2D_Block<TriangleLocal, 2, Colored2D_2x2>(tri, tileMinX, tileMinY,
                                                                     frameBuf);
      break;

    case ROP_Colored3D:
      RasterizeTriHalfSpace3D_Block<TriangleLocal, 2, Colored3D_2x2>(tri, tileMinX, tileMinY,
                                                                     frameBuf);
      break;

    case ROP_TexNearest2D:
    case ROP_TexLinear2D:
      break;

    case ROP_TexNearest3D:
    case ROP_TexLinear3D:

      break;

    case ROP_TexNearest3D_Blend:
    case ROP_TexLinear3D_Blend:

      break;

    default :
      RasterizeTriHalfSpace2D_Block<TriangleLocal, 2, FillColor_2x2>(tri, tileMinX, tileMinY,
                                                                     frameBuf);
      break;
  };

}