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
using cvex16::any_of;
using cvex16::gather;

struct VT
{
  typedef typename cvex16::vint16   vint;
  typedef typename cvex16::vuint16  vuint;
  typedef typename cvex16::vfloat16 vfloat;
  enum {width=16};
};

using ROP_CVEX_FILL      = VROP<TriangleLocal, VT, false>::FillColor;
using ROP_CVEX_2D        = VROP<TriangleLocal, VT, false>::Colored2D;
using ROP_CVEX_3D        = VROP<TriangleLocal, VT, false>::Colored3D;
using ROP_CVEX_2D_TEX_P  = VROP<TriangleLocal, VT, false>::Textured2D;
using ROP_CVEX_2D_TEX_B  = VROP<TriangleLocal, VT, true >::Textured2D;
using ROP_CVEX_3D_TEX_P  = VROP<TriangleLocal, VT, false>::Textured3D;
using ROP_CVEX_3D_TEX_B  = VROP<TriangleLocal, VT, true >::Textured3D;
using ROP_CVEX_2D_TEX_PW = VROP<TriangleLocal, VT, false>::Textured2D_White;
using ROP_CVEX_2D_TEX_BW = VROP<TriangleLocal, VT, true >::Textured2D_White;
using ROP_CVEX_3D_TEX_PW = VROP<TriangleLocal, VT, false>::Textured3D_White;
using ROP_CVEX_3D_TEX_BW = VROP<TriangleLocal, VT, true >::Textured3D_White;

using ROP_CVEX_3D_TEX_P_Blend = VROP<TriangleLocal, VT, false>::Textured3D_Blend;
using ROP_CVEX_3D_TEX_B_Blend = VROP<TriangleLocal, VT, true >::Textured3D_Blend;

void HWImplBlock16x1_CVEX::RasterizeTriangle(const TriangleType& tri, int tileMinX, int tileMinY,
                                             FrameBuffer* frameBuf)
{
  cvex::set_ftz();

  switch (tri.ropId)
  {
    case ROP_Colored2D:
    {
      if(tri.HasSameVertColor())
        RasterizeTriHalfSpaceBlockFixp2D<ROP_CVEX_FILL,4,4>(tri, tileMinX, tileMinY,
                                                            frameBuf);
      else                                                  
        RasterizeTriHalfSpaceBlockFixp2D<ROP_CVEX_2D,4,4>(tri, tileMinX, tileMinY,
                                                          frameBuf);
      break;
    }
    
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
      RasterizeTriHalfSpaceBlockFixp3D<ROP_CVEX_3D_TEX_P_Blend,4,4>(tri, tileMinX, tileMinY,
                                                                    frameBuf);
      break; 
                                                                        
    case ROP_TexLinear3D_Blend:
      RasterizeTriHalfSpaceBlockFixp3D<ROP_CVEX_3D_TEX_B_Blend,4,4>(tri, tileMinX, tileMinY,
                                                                    frameBuf);
      break;

  };

}