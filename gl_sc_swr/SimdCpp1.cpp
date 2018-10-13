//
// Created by frol on 13.10.18.
//

#include "SimdCpp1.h"
#include "RasterOperations.h"


template<typename TriangleType, typename ROP>
void RasterizeTriHalfSpace3D_SIMD1(const TriangleType& tri, int tileMinX, int tileMinY,
                                   FrameBuffer* frameBuf)
{
  // Bounding rectangle
  const int minx = std::max(tri.bb_iminX - tileMinX, 0);
  const int miny = std::max(tri.bb_iminY - tileMinY, 0);
  const int maxx = std::min(tri.bb_imaxX - tileMinX, frameBuf->w - 1);
  const int maxy = std::min(tri.bb_imaxY - tileMinY, frameBuf->h - 1);

  SIMDPP_ALIGN(16) const float x1234[4] = {tri.v3.x, tri.v2.x, tri.v1.x, 0.0f};
  SIMDPP_ALIGN(16) const float y1234[4] = {tri.v3.y, tri.v2.y, tri.v1.y, 0.0f};
  SIMDPP_ALIGN(16) const float x2314[4] = {tri.v2.x, tri.v1.x, tri.v3.x, 0.0f};
  SIMDPP_ALIGN(16) const float y2314[4] = {tri.v2.y, tri.v1.y, tri.v3.y, 0.0f};

  const simdpp::float32<4> tileMinX_f4 = simdpp::to_float32(((simdpp::int32<4>)simdpp::splat(tileMinX)));
  const simdpp::float32<4> tileMinY_f4 = simdpp::to_float32(((simdpp::int32<4>)simdpp::splat(tileMinY)));

  const simdpp::float32<4> tx123 = ((simdpp::float32<4>)simdpp::load(x1234)) - tileMinX_f4;
  const simdpp::float32<4> ty123 = ((simdpp::float32<4>)simdpp::load(y1234)) - tileMinY_f4;
  const simdpp::float32<4> tx231 = ((simdpp::float32<4>)simdpp::load(x2314)) - tileMinX_f4; // seems permute4 does not works !!!
  const simdpp::float32<4> ty231 = ((simdpp::float32<4>)simdpp::load(y2314)) - tileMinY_f4; // seems permute4 does not works !!!

  const simdpp::float32<4> Dx_12_23_31 = tx123 - tx231;
  const simdpp::float32<4> Dy_12_23_31 = ty123 - ty231;
  const simdpp::float32<4> C_123 = Dy_12_23_31*tx123 - Dx_12_23_31*ty123;

  simdpp::float32<4> Cy_123 = C_123 + Dx_12_23_31*simdpp::to_float32(((simdpp::int32<4>)simdpp::splat(miny))) - \
                                      Dy_12_23_31*simdpp::to_float32(((simdpp::int32<4>)simdpp::splat(minx)));

  //#TODO: if (lineSize == 4) => special case !!!
  //
  SIMDPP_ALIGN(16) float Dx12_Dx23_Dx31[4];
  SIMDPP_ALIGN(16) float Dy12_Dy23_Dy31[4];

  simdpp::store(Dx12_Dx23_Dx31, Dx_12_23_31);
  simdpp::store(Dy12_Dy23_Dy31, Dy_12_23_31);

  const float areaInv = 1.0f / fabs(Dy12_Dy23_Dy31[2]*Dx12_Dx23_Dx31[0] - Dx12_Dx23_Dx31[2]*Dy12_Dy23_Dy31[0]);

  int*   cbuff = frameBuf->cbuffer;
  float* zbuff = frameBuf->zbuffer;

  SIMDPP_ALIGN(16) float tri_V1V3V2Z[4] = {tri.v1.z, tri.v3.z, tri.v2.z, 0.0f};

  const simdpp::float32<4> hs_eps_v  = simdpp::make_float(HALF_SPACE_EPSILON, HALF_SPACE_EPSILON, HALF_SPACE_EPSILON, HALF_SPACE_EPSILON);
  const simdpp::float32<4> areaInvV4 = simdpp::splat(areaInv);

  for (int iy = miny; iy <= maxy; iy++)
  {
    simdpp::float32<4> Cx_123 = Cy_123;
    const int offsetY         = frameBuf->pitch * iy;

    for (int ix = minx; ix <= maxx; ix++)
    {
      const auto vInside_123 = simdpp::bit_cast< simdpp::uint32<4>, simdpp::float32<4> >(
          ((Cx_123 > hs_eps_v)).eval().unmask()
      );

      if (simdpp::reduce_and(vInside_123) != 0)
      {
        const simdpp::float32<4> w1234 = areaInvV4*Cx_123;
        const simdpp::float32<4> triZ  = simdpp::load(tri_V1V3V2Z);

        const float zInv = simdpp::reduce_add(w1234*triZ);
        const float zOld = zbuff[offsetY + ix];

        if(zInv > zOld)
        {
          const simdpp::float32<4> color2  = ROP::DrawPixel(tri, w1234, simdpp::splat(zInv));
          cbuff[offsetY + ix] = RealColorToUint32_BGRA_SIMD(color2);
          zbuff[offsetY + ix] = zInv;
        }
      }

      Cx_123 = Cx_123 - Dy_12_23_31;
    }

    Cy_123 = Cy_123 + Dx_12_23_31;
  }

}

using TriangleLocal = HWImplementationPureCpp::TriangleType;

using FillColor_S = SROP<TriangleLocal>::FillColor;
using Colored2D_S = SROP<TriangleLocal>::Colored2D;
using Colored3D_S = SROP<TriangleLocal>::Colored3D;

void HWImplSimdCpp1::RasterizeTriangle(RasterOp a_ropT, BlendOp a_bopT, const TriangleType& tri, int tileMinX, int tileMinY,
                                       FrameBuffer* frameBuf)
{
  RasterizeTriHalfSpace3D_SIMD1<TriangleLocal,Colored3D_S>(tri, tileMinX, tileMinY,
                                                           frameBuf);
}