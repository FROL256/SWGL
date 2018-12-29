//
// Created by frol on 29.12.18.
//

//
// Created by frol on 28.10.18.
//

#include "HWPureCpp.h"
#include "TriRasterHalfSpaceBlockLineFixp.h"
#include "HWBlockLine4x4.h"


#include "HW_VROP_FIXP.h"

#ifdef WIN32
#undef min
#undef max
#endif

#include <algorithm>

using TriangleLocal = HWImplementationPureCpp::TriangleType;

#ifdef WIN32
#include "vfloat4_x64.h"
#else
#include "vfloat4_gcc.h"
#endif

using cvex::load;
using cvex::splat;
using cvex::to_float32;
using cvex::to_int32;
#ifndef WIN32
using cvex::to_uint32;
#endif
using cvex::make_vint;
using cvex::make_vuint;


using cvex::load_u;
using cvex::store_u;
using cvex::rcp_e;
using cvex::blend;
using cvex::test_bits_any;

using cvex::vclamp;
using cvex::store;
using cvex::prefetch;


using ROP_CVEX_2D = VROP_FIXP<TriangleLocal, cvex::vfloat4, cvex::vuint4, 4, false>::Colored2D;


void HWImplBlockLine4x4Fixp_CVEX::RasterizeTriangle(RasterOp a_ropT, BlendOp a_bopT, const TriangleType& tri, int tileMinX, int tileMinY,
                                                    FrameBuffer* frameBuf)
{
  RasterizeTriHalfSpaceBlockLineFixp2D_FixpRast<ROP_CVEX_2D>(tri, tileMinX, tileMinY,
                                                             frameBuf);

}




