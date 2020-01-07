#include "config.h"
#include "TriRasterHalfSpaceBlockFixp.h"
#include "HW_VROP.h"

#ifdef WIN32
#undef min
#undef max
#endif

#include <algorithm>

using TriangleLocal = HWImpl::TriangleType;

#ifdef WIN32
  #include "vfloat8_x64.h"
#else
  #include "vfloat8_gcc.h"
#endif

using cvex8::load;
using cvex8::splat;
using cvex8::to_float32;
using cvex8::to_int32;
using cvex8::to_uint32;

using cvex8::load_u;
using cvex8::store_u;
using cvex8::rcp_e;
using cvex8::blend;
using cvex8::test_bits_any;
using cvex8::clamp;

using cvex8::store;
using cvex8::prefetch;
using cvex8::any_of;
using cvex8::gather;

using ROP_CVEX_FILL     = VROP<TriangleLocal, cvex8::vfloat8, cvex8::vint8, 8, false>::FillColor;

using ROP_CVEX_2D       = VROP<TriangleLocal, cvex8::vfloat8, cvex8::vint8, 8, false>::Colored2D;
using ROP_CVEX_3D       = VROP<TriangleLocal, cvex8::vfloat8, cvex8::vint8, 8, false>::Colored3D;

using ROP_CVEX_2D_TEX_P = VROP<TriangleLocal, cvex8::vfloat8, cvex8::vint8, 8, false>::Textured2D;
using ROP_CVEX_2D_TEX_B = VROP<TriangleLocal, cvex8::vfloat8, cvex8::vint8, 8, true >::Textured2D;

using ROP_CVEX_3D_TEX_P = VROP<TriangleLocal, cvex8::vfloat8, cvex8::vint8, 8, false>::Textured3D;
using ROP_CVEX_3D_TEX_B = VROP<TriangleLocal, cvex8::vfloat8, cvex8::vint8, 8, true >::Textured3D;


using ROP_CVEX_2D_TEX_PW = VROP<TriangleLocal, cvex8::vfloat8, cvex8::vint8, 8, false>::Textured2D_White;
using ROP_CVEX_2D_TEX_BW = VROP<TriangleLocal, cvex8::vfloat8, cvex8::vint8, 8, true >::Textured2D_White;

using ROP_CVEX_3D_TEX_PW = VROP<TriangleLocal, cvex8::vfloat8, cvex8::vint8, 8, false>::Textured3D_White;
using ROP_CVEX_3D_TEX_BW = VROP<TriangleLocal, cvex8::vfloat8, cvex8::vint8, 8, true >::Textured3D_White;

using ROP_CVEX_3D_TEX_P_Blend = VROP<TriangleLocal, cvex8::vfloat8, cvex8::vint8, 8, false>::Textured3D_Blend;
using ROP_CVEX_3D_TEX_B_Blend = VROP<TriangleLocal, cvex8::vfloat8, cvex8::vint8, 8, true >::Textured3D_Blend;

void HWImplBlock8x2_CVEX::RasterizeTriangle(const TriangleType& tri, int tileMinX, int tileMinY,
                                            FrameBuffer* frameBuf)
{
  cvex::set_ftz();

  switch (tri.ropId)
  {
    case ROP_Colored2D:
      RasterizeTriHalfSpaceBlockFixp2D<ROP_CVEX_2D,4,4>(tri, tileMinX, tileMinY,
                                                        frameBuf);
      break;
   
   
    case ROP_Colored3D:
      RasterizeTriHalfSpaceBlockFixp3D<ROP_CVEX_3D,4,4>(tri, tileMinX, tileMinY,
                                                        frameBuf);
      break;
  
    case ROP_TexNearest2D:

      if(tri.IsWhite())
         RasterizeTriHalfSpaceBlockFixp2D<ROP_CVEX_2D_TEX_PW,4,4>(tri, tileMinX, tileMinY, 
                                                                  frameBuf);
      else
         RasterizeTriHalfSpaceBlockFixp2D<ROP_CVEX_2D_TEX_P,4,4>(tri, tileMinX, tileMinY, 
                                                                 frameBuf);
      break;

    case ROP_TexLinear2D:
      if(tri.IsWhite())
         RasterizeTriHalfSpaceBlockFixp2D<ROP_CVEX_2D_TEX_BW,4,4>(tri, tileMinX, tileMinY, 
                                                                  frameBuf);
      else
         RasterizeTriHalfSpaceBlockFixp2D<ROP_CVEX_2D_TEX_B,4,4>(tri, tileMinX, tileMinY, 
                                                                 frameBuf);
      break;

    case ROP_TexNearest3D:

      if(tri.IsWhite())
        RasterizeTriHalfSpaceBlockFixp3D<ROP_CVEX_3D_TEX_PW,4,4>(tri, tileMinX, tileMinY,
                                                                frameBuf);
      else
        RasterizeTriHalfSpaceBlockFixp3D<ROP_CVEX_3D_TEX_P,4,4>(tri, tileMinX, tileMinY,
                                                                frameBuf);

    case ROP_TexLinear3D:
    
      if(tri.IsWhite())
        RasterizeTriHalfSpaceBlockFixp3D<ROP_CVEX_3D_TEX_BW,4,4>(tri, tileMinX, tileMinY,
                                                                frameBuf);
      else
        RasterizeTriHalfSpaceBlockFixp3D<ROP_CVEX_3D_TEX_B,4,4>(tri, tileMinX, tileMinY,
                                                                frameBuf);
                                                              
      break;

    case ROP_TexNearest3D_Blend:
    case ROP_TexLinear3D_Blend:
      RasterizeTriHalfSpaceBlockFixp3D<ROP_CVEX_3D_TEX_P_Blend,4,4>(tri, tileMinX, tileMinY,
                                                                    frameBuf);
      break;

  };
}