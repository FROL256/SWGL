//
// Created by frol on 28.10.18.
//

#include "HWPureCpp.h"
#include "TriRasterHalfSpaceBlockLineFixp.h"
#include "HWBlockLine4x4.h"
#include "HW_VROP.h"

#ifdef WIN32
#undef min
#undef max
#endif

#include <algorithm>

using TriangleLocal = HWImplementationPureCpp::TriangleType;

#include "vfloat4_x64.h"
//#include "vfloat4_gcc.h"

using cvex::load;
using cvex::splat;
using cvex::to_float32;
using cvex::to_int32;
using cvex::make_vint;

using cvex::load_u;
using cvex::store_u;
using cvex::rcp_e;
using cvex::blend;
using cvex::test_bits_any;

using ROP_CVEX_2D = VROP<TriangleLocal, cvex::vfloat4, cvex::vint4, 4>::Colored2D;
using ROP_CVEX_3D = VROP<TriangleLocal, cvex::vfloat4, cvex::vint4, 4>::Colored3D;


void HWImplBlockLine4x4_CVEX::RasterizeTriangle(RasterOp a_ropT, BlendOp a_bopT, const TriangleType& tri, int tileMinX, int tileMinY,
                                                FrameBuffer* frameBuf)
{
  cvex::set_ftz();

  switch (a_ropT)
  {
    case ROP_Colored2D:
      RasterizeTriHalfSpaceBlockLineFixp2D<ROP_CVEX_2D>(tri, tileMinX, tileMinY,
                                                        frameBuf);
      break;

    case ROP_Colored3D:
      RasterizeTriHalfSpaceBlockLineFixp3D<ROP_CVEX_3D>(tri, tileMinX, tileMinY,
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




