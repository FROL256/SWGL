//
// Created by frol on 28.10.18.
//

#include "PureCpp.h"
#include "SimdBlockFixp.h"
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

    static inline vint BlockLine(const TriangleLocal& tri, const int CX1, const int CX2, const int FDY12, const int FDY23, const float a_areaInv)
    {
      const vfloat areaInv = cvex::splat_1to4(a_areaInv);
      const vfloat w1      = areaInv*cvex::to_vfloat( cvex::make_vint4(CX1, CX1-FDY12, CX1 - 2*FDY12,CX1 - 3*FDY12) );
      const vfloat w2      = areaInv*cvex::to_vfloat( cvex::make_vint4(CX2, CX2-FDY23, CX2 - 2*FDY23,CX2 - 3*FDY23) );
      const vfloat w3      = (c_one - w1 - w2);

      const vfloat r = cvex::splat_1to4(tri.c1.x)*w1 + cvex::splat_1to4(tri.c2.x)*w2 + cvex::splat_1to4(tri.c3.x)*w3;
      const vfloat g = cvex::splat_1to4(tri.c1.y)*w1 + cvex::splat_1to4(tri.c2.y)*w2 + cvex::splat_1to4(tri.c3.y)*w3;
      const vfloat b = cvex::splat_1to4(tri.c1.z)*w1 + cvex::splat_1to4(tri.c2.z)*w2 + cvex::splat_1to4(tri.c3.z)*w3;
      const vfloat a = cvex::splat_1to4(tri.c1.w)*w1 + cvex::splat_1to4(tri.c2.w)*w2 + cvex::splat_1to4(tri.c3.w)*w3;

      return (cvex::to_vint(r*c_255) << 16) | // BGRA
             (cvex::to_vint(g*c_255) << 8)  |
             (cvex::to_vint(b*c_255) << 0)  |
             (cvex::to_vint(a*c_255) << 24);
    }
  };

};


void HWImplBlock4x4_Fixp::RasterizeTriangle(RasterOp a_ropT, BlendOp a_bopT, const TriangleType& tri, int tileMinX, int tileMinY,
                                            FrameBuffer* frameBuf)
{
  RasterizeTriHalfSpaceBlockFixp2D<TriangleType, VROP_2D_CVEX<4>::Colored2D >(tri, tileMinX, tileMinY,
                                                                              frameBuf);
}