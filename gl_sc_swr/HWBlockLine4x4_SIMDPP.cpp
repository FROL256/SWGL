//
// Created by frol on 29.10.18.
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



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define  	SIMDPP_ARCH_X86_AVX
#include "../simdpp/simd.h"

// only 2D
//
using simdpp::load;
using simdpp::splat;
using simdpp::to_float32;
using simdpp::to_int32;
using simdpp::store_u;

// enable 3D
//
using simdpp::load_u;
using simdpp::rcp_e;
using simdpp::blend;
using simdpp::test_bits_any;

static inline simdpp::int32<4> cmp_gt_asint(const simdpp::float32<4> a, const simdpp::float32<4> b)
{
  return simdpp::bit_cast< simdpp::int32<4>, simdpp::float32<4> > (
      (a > b).eval().unmask()
  );
}

//inline static unsigned int RealColorToUint32_BGRA(const simdpp::float32<4>& real_color)
//{
//  static const simdpp::float32<4> const_255 = simdpp::splat(255.0f);
//  static const simdpp::uint32<4>  shiftmask = simdpp::make_int(16,8,0,24);
//  const  simdpp::uint32<4>        rgbai     = simdpp::to_int32(real_color*const_255);
//  return simdpp::reduce_or(simdpp::shift_l(rgbai, shiftmask)); // return blue | (green << 8) | (red << 16) | (alpha << 24);
//}


using ROP_SIMDPP_2D = VROP<TriangleLocal, simdpp::float32x4, simdpp::int32x4, 4>::Colored2D;
using ROP_SIMDPP_3D = VROP<TriangleLocal, simdpp::float32x4, simdpp::int32x4, 4>::Colored3D;

void HWImplBlockLine4x4_SIMDPP::RasterizeTriangle(RasterOp a_ropT, BlendOp a_bopT, const TriangleType& tri, int tileMinX, int tileMinY,
                                                  FrameBuffer* frameBuf)
{
  //cvex::set_ftz();

  switch (a_ropT)
  {
    case ROP_Colored2D:
      RasterizeTriHalfSpaceBlockLineFixp2D<ROP_SIMDPP_2D>(tri, tileMinX, tileMinY,
                                                          frameBuf);
      break;

    case ROP_Colored3D:
      RasterizeTriHalfSpaceBlockLineFixp3D<ROP_SIMDPP_3D>(tri, tileMinX, tileMinY,
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



