//
// Created by frol on 28.10.18.
//

#include "HWPureCpp.h"
#include "TriRasterHalfSpaceBlockLineFixp.h"
#include "HWBlockLine4x4.h"

#ifdef WIN32
#undef min
#undef max
#endif

#include <algorithm>

using TriangleLocal = HWImplementationPureCpp::TriangleType;


using ROP_C2D = VROP_CVEX<TriangleLocal, 4>::Colored2D;
using ROP_C3D = VROP_CVEX<TriangleLocal, 4>::Colored3D;


void HWImplBlockLine4x4_CVEX::RasterizeTriangle(RasterOp a_ropT, BlendOp a_bopT, const TriangleType& tri, int tileMinX, int tileMinY,
                                                FrameBuffer* frameBuf)
{

  switch (a_ropT)
  {
    case ROP_Colored2D:
      RasterizeTriHalfSpaceBlockLineFixp2D<ROP_C2D>(tri, tileMinX, tileMinY,
                                                    frameBuf);
      break;

    case ROP_Colored3D:
      RasterizeTriHalfSpaceBlockLineFixp3D<ROP_C3D>(tri, tileMinX, tileMinY,
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
      RasterizeTriHalfSpaceBlockFixp2D_Fill<TriangleType, 4>(tri, tileMinX, tileMinY,
                                                             frameBuf);
      break;
  };

}


void HWImplBlockLine4x4_SIMDPP::RasterizeTriangle(RasterOp a_ropT, BlendOp a_bopT, const TriangleType& tri, int tileMinX, int tileMinY,
                                                  FrameBuffer* frameBuf)
{

}