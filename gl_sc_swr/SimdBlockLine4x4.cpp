//
// Created by frol on 28.10.18.
//

#include "PureCpp.h"
#include "TriRasterHalfSpaceBlockLineFixp.h"
#include "PureCppBlock4x4.h"

#ifdef WIN32
#undef min
#undef max
#endif

#include <algorithm>

using TriangleLocal = HWImplementationPureCpp::TriangleType;


template<int width>
struct VROP_2D_CVEX
{
};


template<>
struct VROP_2D_CVEX<4>
{
  static constexpr cvex::vfloat4 c_one = {1.0f,1.0f,1.0f,1.0f};
  static constexpr cvex::vfloat4 c_255 = {255.0f, 255.0f, 255.0f, 255.0f};

  struct Colored2D
  {
    enum {n = 4};
    using vfloat = cvex::vfloat4;
    using vint   = cvex::vint4;

    static inline vint Line(const TriangleLocal& tri, const int CX1, const int CX2, const int FDY12, const int FDY23, const float a_areaInv)
    {
      const vfloat areaInv = cvex::splat_1to4(a_areaInv);
      const vfloat w1      = areaInv*cvex::to_vfloat( cvex::make_vint4(CX1, CX1-FDY12, CX1 - 2*FDY12,CX1 - 3*FDY12) );
      const vfloat w2      = areaInv*cvex::to_vfloat( cvex::make_vint4(CX2, CX2-FDY23, CX2 - 2*FDY23,CX2 - 3*FDY23) );
      const vfloat w3      = (c_one - w1 - w2);

      //const vfloat c1 = cvex::load((const float*)&tri.c1);
      //const vfloat c2 = cvex::load((const float*)&tri.c2);
      //const vfloat c3 = cvex::load((const float*)&tri.c3);
      //
      //const vfloat r = cvex::splat_0(c1)*w1 + cvex::splat_0(c2)*w2 + cvex::splat_0(c3)*w3;
      //const vfloat g = cvex::splat_1(c1)*w1 + cvex::splat_1(c2)*w2 + cvex::splat_1(c3)*w3;
      //const vfloat b = cvex::splat_2(c1)*w1 + cvex::splat_2(c2)*w2 + cvex::splat_2(c3)*w3;
      //const vfloat a = cvex::splat_3(c1)*w1 + cvex::splat_3(c2)*w2 + cvex::splat_3(c3)*w3;

      const vfloat r = tri.c1.x*w1 + tri.c2.x*w2 + tri.c3.x*w3;
      const vfloat g = tri.c1.y*w1 + tri.c2.y*w2 + tri.c3.y*w3;
      const vfloat b = tri.c1.z*w1 + tri.c2.z*w2 + tri.c3.z*w3;
      const vfloat a = tri.c1.w*w1 + tri.c2.w*w2 + tri.c3.w*w3;

      return (cvex::to_vint(r*c_255) << 16) | // BGRA
             (cvex::to_vint(g*c_255) << 8)  |
             (cvex::to_vint(b*c_255) << 0)  |
             (cvex::to_vint(a*c_255) << 24);
    }

    static inline store_u(int* buffer, vint a_color)
    {
      cvex::store_u(buffer, a_color);
    }

    static inline Pixel(const TriangleLocal& tri, const int CX1, const int CX2, const float areaInv)
    {
      const float w1 = areaInv*float(CX1);
      const float w2 = areaInv*float(CX2);

      // const float4 c = tri.c1*w1 + tri.c2*w2 + tri.c3*(1.0f - w1 - w2);
      // return RealColorToUint32_BGRA(c);

      const vfloat c1 = cvex::load((const float*)&tri.c1);
      const vfloat c2 = cvex::load((const float*)&tri.c2);
      const vfloat c3 = cvex::load((const float*)&tri.c3);
      const vfloat c  = c1*w1 + c2*w2 + c3*(1.0f - w1 - w2);
      return cvex::color_compress_bgra(c);
    }

  };

};


void HWImplBlock4x4_Fixp::RasterizeTriangle(RasterOp a_ropT, BlendOp a_bopT, const TriangleType& tri, int tileMinX, int tileMinY,
                                            FrameBuffer* frameBuf)
{
  RasterizeTriHalfSpaceBlockLineFixp2D<TriangleType, VROP_2D_CVEX<4>::Colored2D>(tri, tileMinX, tileMinY,
                                                                                 frameBuf);
}