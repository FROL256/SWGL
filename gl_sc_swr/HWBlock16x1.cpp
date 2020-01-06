#include "config.h"
#include "TriRasterHalfSpaceBlockFixp.h"
#include "HW_VROP.h"

#ifdef WIN32
#undef min
#undef max
#endif

#include <algorithm>

using TriangleLocal = HWImplementationPureCpp::TriangleType;

#ifdef WIN32
  #include "vfloat16_x64.h"
#else
  #include "vfloat16_gcc.h"
#endif

using cvex16::load;
using cvex16::splat;
using cvex16::to_float32;
using cvex16::to_int32;
using cvex16::to_uint32;

using cvex16::load_u;
using cvex16::store_u;
using cvex16::rcp_e;
using cvex16::blend;
//using cvex16::test_bits_any;
using cvex16::clamp;

using cvex16::store;
using cvex16::prefetch;

using ROP_CVEX_FILL     = VROP<TriangleLocal, cvex16::vfloat16, cvex16::vint16, 16, false>::FillColor;

using ROP_CVEX_2D       = VROP<TriangleLocal, cvex16::vfloat16, cvex16::vint16, 16, false>::Colored2D;
//using ROP_CVEX_3D       = VROP<TriangleLocal, cvex16::vfloat16, cvex16::vint16, 16, false>::Colored3D;

//using ROP_CVEX_2D_TEX_P = VROP<TriangleLocal, cvex16::vfloat16, cvex16::vint16, 16, false>::Textured2D;
//using ROP_CVEX_2D_TEX_B = VROP<TriangleLocal, cvex16::vfloat16, cvex16::vint16, 16, true >::Textured2D;
//
//using ROP_CVEX_3D_TEX_P = VROP<TriangleLocal, cvex16::vfloat16, cvex16::vint16, 8, false>::Textured3D;
//using ROP_CVEX_3D_TEX_B = VROP<TriangleLocal, cvex16::vfloat16, cvex16::vint16, 8, true >::Textured3D;
//
//#ifndef WIN32
//using ROP_CVEX_3D_TEX_P_Blend = VROP<TriangleLocal, cvex16::vfloat16, cvex16::vint16, 16, false>::Textured3D_Blend;
//using ROP_CVEX_3D_TEX_B_Blend = VROP<TriangleLocal, cvex16::vfloat16, cvex16::vint16, 16, true >::Textured3D_Blend;
//#endif

void HWImplBlock16x1_CVEX::RasterizeTriangle(const TriangleType& tri, int tileMinX, int tileMinY,
                                             FrameBuffer* frameBuf)
{
  cvex::set_ftz();

  const auto a_ropT = tri.ropId;

  switch (a_ropT)
  {
    case ROP_Colored2D:
      RasterizeTriHalfSpaceBlockFixp2D<ROP_CVEX_2D,4,4>(tri, tileMinX, tileMinY,
                                                        frameBuf);
      break;

    //case ROP_Colored3D:
    // RasterizeTriHalfSpaceBlockFixp3D<ROP_CVEX_3D,4,4>(tri, tileMinX, tileMinY,
    //                                                   frameBuf);
    // break;
  
  };

}