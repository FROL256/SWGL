#ifndef TEST_SIMDCPPBLOCKLINE_H
#define TEST_SIMDCPPBLOCKLINE_H

#include "TriRaster.h"
#include "RasterOperations.h"


SIMDPP_ALIGN(64) static const float g_XPixOffets_8[8] = {0.0f, 1.0f, 2.0f, 3.0f,
                                                         4.0f, 5.0f, 6.0f, 7.0f};


template<int DIM>
static inline simdpp::float32<DIM> PixOffsetX()
{
  simdpp::float32<4> r = simdpp::make_float(0.0f, 1.0f, 2.0f, 3.0f);
  return r;
}

template<>
inline simdpp::float32<2> PixOffsetX<2>()
{
  simdpp::float32<2> r = simdpp::make_float(0.0f, 1.0f);
  return r;
}

template<>
inline simdpp::float32<8> PixOffsetX<8>()
{
  simdpp::float32<8> r = simdpp::load(g_XPixOffets_8);
  return r;
}


inline static unsigned int RealColorToUint32_BGRA_SIMD(const simdpp::float32<4>& real_color)
{
  static const simdpp::float32<4> const_255 = simdpp::make_float(255.0f);
  static const simdpp::uint32<4>  shiftmask = simdpp::make_int(16,8,0,24);
  const simdpp::uint32<4>         rgbai     = simdpp::to_int32(real_color*const_255);
  return simdpp::reduce_or(simdpp::shift_l(rgbai, shiftmask)); // return blue | (green << 8) | (red << 16) | (alpha << 24);
}


template<typename TriangleType, const int lineSize, typename ROP, typename SROP>
void RasterizeTriHalfSpace2D_BlockLine(const TriangleType& tri, int tileMinX, int tileMinY,
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
  const simdpp::float32<4> tx231 = ((simdpp::float32<4>)simdpp::load(x2314)) - tileMinX_f4; // simdpp::permute4<1,2,0,0>(tx123); seems permute4 does not works !!!
  const simdpp::float32<4> ty231 = ((simdpp::float32<4>)simdpp::load(y2314)) - tileMinY_f4; // simdpp::permute4<1,2,0,0>(ty123);

  const simdpp::float32<4> Dx_12_23_31 = tx123 - tx231;
  const simdpp::float32<4> Dy_12_23_31 = ty123 - ty231;
  const simdpp::float32<4> C_123 = Dy_12_23_31*tx123 - Dx_12_23_31*ty123;

  simdpp::float32<4> Cy_abc = C_123 + Dx_12_23_31*simdpp::to_float32(((simdpp::int32<4>)simdpp::splat(miny))) - \
                                      Dy_12_23_31*simdpp::to_float32(((simdpp::int32<4>)simdpp::splat(minx)));

  //#TODO: if (lineSize == 4) => special case !!!
  //
  SIMDPP_ALIGN(16) float Dx12_Dx23_Dx31[4];
  SIMDPP_ALIGN(16) float Dy12_Dy23_Dy31[4];

  simdpp::store(Dx12_Dx23_Dx31, Dx_12_23_31);
  simdpp::store(Dy12_Dy23_Dy31, Dy_12_23_31);

  const float areaInv = 1.0f / fabs(Dy12_Dy23_Dy31[2]*Dx12_Dx23_Dx31[0] - Dx12_Dx23_Dx31[2]*Dy12_Dy23_Dy31[0]);

  const simdpp::float32<lineSize> areaInvV = simdpp::splat(areaInv);
  const simdpp::float32<lineSize> Dx12v    = simdpp::splat(Dx12_Dx23_Dx31[0]);
  const simdpp::float32<lineSize> Dx23v    = simdpp::splat(Dx12_Dx23_Dx31[1]);
  const simdpp::float32<lineSize> Dx31v    = simdpp::splat(Dx12_Dx23_Dx31[2]);

  const simdpp::float32<lineSize> Dy12v    = simdpp::splat(Dy12_Dy23_Dy31[0]);
  const simdpp::float32<lineSize> Dy23v    = simdpp::splat(Dy12_Dy23_Dy31[1]);
  const simdpp::float32<lineSize> Dy31v    = simdpp::splat(Dy12_Dy23_Dy31[2]);

  const simdpp::float32<4> blockSizeF_4v = simdpp::to_float32(((simdpp::int32<4>)simdpp::splat(lineSize)));
  const simdpp::float32<4> hs_eps_v = simdpp::make_float(HALF_SPACE_EPSILON, HALF_SPACE_EPSILON, HALF_SPACE_EPSILON, HALF_SPACE_EPSILON);

  simdpp::float32<4> col0 = simdpp::make_float(0.0f, 0.0f, 0.0f, 0.0f);  // {0.f, 0.f, 0.f, 0.f};
  simdpp::float32<4> col1 = simdpp::neg(Dy_12_23_31*blockSizeF_4v);      // {0.f, - Dy12*blockSizeF, Dx12*blockSizeF, Dx12*blockSizeF - Dy12*blockSizeF};
  simdpp::float32<4> col2 = Dx_12_23_31*blockSizeF_4v;                   // {0.f, - Dy23*blockSizeF, Dx23*blockSizeF, Dx23*blockSizeF - Dy23*blockSizeF};
  simdpp::float32<4> col3 = col1 + col2;                                 // {0.f, - Dy31*blockSizeF, Dx31*blockSizeF, Dx31*blockSizeF - Dy31*blockSizeF};
  simdpp::transpose4(col0, col1, col2, col3);

  //
  ///////////////////////////////////////////////////////////////////////////////// vectorized per triangle variables

  int* cbuff = frameBuf->cbuffer;

  // Scan through bounding rectangle
  for (int by = miny; by <= maxy; by += lineSize)
  {
    simdpp::float32<4> Cx_abc = Cy_abc;

    for (int bx = minx; bx <= maxx; bx+= lineSize)
    {
      const simdpp::float32<4> Cx1_v = simdpp::splat<0>(Cx_abc) + col0; // ((const simdpp::float32<4>)simdpp::load(Cx1_va));
      const simdpp::float32<4> Cx2_v = simdpp::splat<1>(Cx_abc) + col1; // ((const simdpp::float32<4>)simdpp::load(Cx2_va));
      const simdpp::float32<4> Cx3_v = simdpp::splat<2>(Cx_abc) + col2; // ((const simdpp::float32<4>)simdpp::load(Cx3_va));

      const auto vInside_v4u = simdpp::bit_cast< simdpp::uint32<4>, simdpp::float32<4> >(
          ( (Cx1_v > hs_eps_v) & (Cx2_v > hs_eps_v) & (Cx3_v > hs_eps_v) ).eval().unmask()
      );

      if(simdpp::reduce_and(vInside_v4u) != 0) // render fully covered block
      {
        // store all pixels
        //
        #pragma unroll (lineSize)
        for (int iy = 0; iy < lineSize; iy++)
        {
          const int y1 = by + iy;

          if(y1 <= maxy)
          {
            const simdpp::float32<lineSize> pixOffsY = simdpp::to_float32(((simdpp::int32<lineSize>)simdpp::splat(iy)));
            const simdpp::float32<lineSize> pixOffsX = PixOffsetX<lineSize>();

            const simdpp::float32<lineSize> w1 = areaInvV*( simdpp::splat<0>(Cx_abc) + Dx12v*pixOffsY - Dy12v*pixOffsX );
            const simdpp::float32<lineSize> w2 = areaInvV*( simdpp::splat<1>(Cx_abc) + Dx23v*pixOffsY - Dy23v*pixOffsX );
            const simdpp::float32<lineSize> w3 = areaInvV*( simdpp::splat<2>(Cx_abc) + Dx31v*pixOffsY - Dy31v*pixOffsX );

            const auto color   = ROP::DrawPixel(tri, w1, w3, w2);
            const auto pixData = VROP<lineSize, TriangleType>::RealColorToUint32_BGRA(color);

            simdpp::store_u(cbuff + frameBuf->pitch * y1 + bx, pixData);
          } //  end if(y1 <= maxy)

        } // end for (int iy = 0; iy < lineSize; iy++)
      }
      else if (simdpp::reduce_or(vInside_v4u) != 0) // render partially covered block
      {
        simdpp::float32<4> Cy_123 = simdpp::make_float(0.0f, 0.0f, 0.0f, 10.0f); // set last component to 10 to be always gt than hs_eps_v.w

        for (int iy = 0; iy < lineSize; iy++)
        {
          const int          y1     = by + iy;
          simdpp::float32<4> Cx_123 = Cy_123 + Cx_abc;

          #pragma unroll (lineSize)
          for (int ix = 0; ix < lineSize; ix++)
          {
            const auto vInside_123 = simdpp::bit_cast< simdpp::uint32<4>, simdpp::float32<4> >(
                ((Cx_123 > hs_eps_v)).eval().unmask()
            );

            const int x1 = bx + ix;
            if (x1 <= maxx && y1 <= maxy && (simdpp::reduce_and(vInside_123) != 0))
            {
              const simdpp::float32<4> color2  = SROP::DrawPixel(tri, areaInvV*Cx_123);
              cbuff[frameBuf->pitch * y1 + x1] = RealColorToUint32_BGRA_SIMD(color2);
            }

            Cx_123 = Cx_123 - Dy_12_23_31;
          }

          Cy_123 = Cy_123 + Dx_12_23_31;
        }
      }

      Cx_abc = Cx_abc - Dy_12_23_31*blockSizeF_4v;
    }

    Cy_abc = Cy_abc + Dx_12_23_31*blockSizeF_4v;
  }
}




#endif