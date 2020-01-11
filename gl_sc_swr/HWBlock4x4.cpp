//
// Created by frol on 28.10.18.
//

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

using cvex::load_u;
using cvex::store_u;
using cvex::rcp_e;
using cvex::blend;
using cvex::test_bits_any;
using cvex::clamp;

using cvex::store;
using cvex::prefetch;
using cvex::any_of;
using cvex::gather;

struct VT
{
  typedef typename cvex::vint4   vint;
  typedef typename cvex::vuint4  vuint;
  typedef typename cvex::vfloat4 vfloat;
  enum {width = 4};
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

void HWImplBlock4x4_CVEX::RasterizeTriangle(const TriangleType& tri, int tileMinX, int tileMinY,
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
      RasterizeTriHalfSpaceBlockFixp3D<ROP_CVEX_3D_TEX_P_Blend,4,4>(tri, tileMinX, tileMinY,
                                                                    frameBuf);
      break; 
                                                                        
    case ROP_TexLinear3D_Blend:
      RasterizeTriHalfSpaceBlockFixp3D<ROP_CVEX_3D_TEX_B_Blend,4,4>(tri, tileMinX, tileMinY,
                                                                    frameBuf);
      break;
  };

}




