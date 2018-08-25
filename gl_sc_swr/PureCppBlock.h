//
// Created by frol on 24.08.18.
//

#ifndef TEST_GL_TOP_PURECPPBLOCK_H
#define TEST_GL_TOP_PURECPPBLOCK_H

#include "TriRaster.h"
#include "RasterOperations.h"


template<typename TriangleType, const int blockSize, typename ROP_VECTOR>
void RasterizeTriHalfSpace2D_Block(const TriangleType& tri, int tileMinX, int tileMinY,
                                   FrameBuffer* frameBuf)
{
  const float tileMinX_f = float(tileMinX);
  const float tileMinY_f = float(tileMinY);

  const float y1 = tri.v3.y - tileMinY_f;
  const float y2 = tri.v2.y - tileMinY_f;
  const float y3 = tri.v1.y - tileMinY_f;

  const float x1 = tri.v3.x - tileMinX_f;
  const float x2 = tri.v2.x - tileMinX_f;
  const float x3 = tri.v1.x - tileMinX_f;

  // Deltas
  const float Dx12 = x1 - x2;
  const float Dx23 = x2 - x3;
  const float Dx31 = x3 - x1;

  const float Dy12 = y1 - y2;
  const float Dy23 = y2 - y3;
  const float Dy31 = y3 - y1;

  const simdpp::float32<blockSize*blockSize> Dx12v = simdpp::splat(Dx12);
  const simdpp::float32<blockSize*blockSize> Dx23v = simdpp::splat(Dx23);
  const simdpp::float32<blockSize*blockSize> Dx31v = simdpp::splat(Dx31);

  const simdpp::float32<blockSize*blockSize> Dy12v = simdpp::splat(Dy12);
  const simdpp::float32<blockSize*blockSize> Dy23v = simdpp::splat(Dy23);
  const simdpp::float32<blockSize*blockSize> Dy31v = simdpp::splat(Dy31);

  // Bounding rectangle
  const int minx = std::max(tri.bb_iminX - tileMinX, 0);
  const int miny = std::max(tri.bb_iminY - tileMinY, 0);
  const int maxx = std::min(tri.bb_imaxX - tileMinX, frameBuf->w - 1);
  const int maxy = std::min(tri.bb_imaxY - tileMinY, frameBuf->h - 1);

  int* cbuff = frameBuf->cbuffer;

  // Constant part of half-edge functions
  const float C1 = Dy12 * x1 - Dx12 * y1;
  const float C2 = Dy23 * x2 - Dx23 * y2;
  const float C3 = Dy31 * x3 - Dx31 * y3;

  const float areaInv = 1.0f / fabs(Dy31*Dx12 - Dx31*Dy12); // edgeFunction(v0, v1, v2);

  const simdpp::float32<blockSize*blockSize> areaInvV = simdpp::splat(areaInv);

  float Cy1_b = C1 + Dx12 * miny - Dy12 * minx;
  float Cy2_b = C2 + Dx23 * miny - Dy23 * minx;
  float Cy3_b = C3 + Dx31 * miny - Dy31 * minx;

  int offset = lineOffset(miny, frameBuf->w, frameBuf->h);

  constexpr float blockSizeF = float(blockSize);

  typename VROP<blockSize*blockSize>::vec4 tri_c1, tri_c2, tri_c3;
  {
    tri_c1.x = simdpp::splat(tri.c1.x);
    tri_c1.y = simdpp::splat(tri.c1.y);
    tri_c1.z = simdpp::splat(tri.c1.z);
    tri_c1.w = simdpp::splat(tri.c1.w);

    tri_c2.x = simdpp::splat(tri.c2.x);
    tri_c2.y = simdpp::splat(tri.c2.y);
    tri_c2.z = simdpp::splat(tri.c2.z);
    tri_c2.w = simdpp::splat(tri.c2.w);

    tri_c3.x = simdpp::splat(tri.c3.x);
    tri_c3.y = simdpp::splat(tri.c3.y);
    tri_c3.z = simdpp::splat(tri.c3.z);
    tri_c3.w = simdpp::splat(tri.c3.w);
  }

  const auto pixOffsX = PixOffsetX<blockSize * blockSize>();
  const auto pixOffsY = PixOffsetY<blockSize * blockSize>();

  // Scan through bounding rectangle
  for (int by = miny; by <= maxy; by += blockSize)
  {
    // Start value for horizontal scan
    float Cx1_b = Cy1_b;
    float Cx2_b = Cy2_b;
    float Cx3_b = Cy3_b;

    for (int bx = minx; bx <= maxx; bx+= blockSize)
    {
      const float Cx1_00 = Cx1_b;
      const float Cx2_00 = Cx2_b;
      const float Cx3_00 = Cx3_b;

      const float Cx1_01 = Cx1_b - Dy12*blockSizeF;
      const float Cx2_01 = Cx2_b - Dy23*blockSizeF;
      const float Cx3_01 = Cx3_b - Dy31*blockSizeF;

      const float Cx1_10 = Cx1_b + Dx12*blockSizeF;
      const float Cx2_10 = Cx2_b + Dx23*blockSizeF;
      const float Cx3_10 = Cx3_b + Dx31*blockSizeF;

      const float Cx1_11 = Cx1_b + Dx12*blockSizeF - Dy12*blockSizeF;
      const float Cx2_11 = Cx2_b + Dx23*blockSizeF - Dy23*blockSizeF;
      const float Cx3_11 = Cx3_b + Dx31*blockSizeF - Dy31*blockSizeF;

      const bool v0Inside = (Cx1_00 > HALF_SPACE_EPSILON && Cx2_00 > HALF_SPACE_EPSILON && Cx3_00 > HALF_SPACE_EPSILON);
      const bool v1Inside = (Cx1_01 > HALF_SPACE_EPSILON && Cx2_01 > HALF_SPACE_EPSILON && Cx3_01 > HALF_SPACE_EPSILON);
      const bool v2Inside = (Cx1_10 > HALF_SPACE_EPSILON && Cx2_10 > HALF_SPACE_EPSILON && Cx3_10 > HALF_SPACE_EPSILON);
      const bool v3Inside = (Cx1_11 > HALF_SPACE_EPSILON && Cx2_11 > HALF_SPACE_EPSILON && Cx3_11 > HALF_SPACE_EPSILON);

      const simdpp::float32<blockSize*blockSize> Cx1_bv = simdpp::splat(Cx1_b);
      const simdpp::float32<blockSize*blockSize> Cx2_bv = simdpp::splat(Cx2_b);
      const simdpp::float32<blockSize*blockSize> Cx3_bv = simdpp::splat(Cx3_b);

      SIMDPP_ALIGN(64) int pixels[blockSize*blockSize];

      if (v0Inside && v1Inside && v2Inside && v3Inside)
      {

        const simdpp::float32<blockSize*blockSize> w1 = areaInvV*( Cx1_bv + Dx12v*pixOffsX - Dy12v*pixOffsY );
        const simdpp::float32<blockSize*blockSize> w2 = areaInvV*( Cx2_bv + Dx23v*pixOffsX - Dy23v*pixOffsY );
        const simdpp::float32<blockSize*blockSize> w3 = areaInvV*( Cx3_bv + Dx31v*pixOffsX - Dy31v*pixOffsY );

        const auto color   = VROP<blockSize*blockSize>::Colored2D::DrawPixel(tri_c1, tri_c2, tri_c3, w1, w2, w3);
        const auto pixData = VROP<blockSize*blockSize>::RealColorToUint32_BGRA(color);
        simdpp::store(pixels, pixData);

        // store pixels
        //

        #pragma unroll (blockSize)
        for (int iy = 0; iy < blockSize; iy++)
        {
          #pragma unroll (blockSize)
          for (int ix = 0; ix < blockSize; ix++)
          {
            const int x1 = bx + ix;
            const int y1 = by + iy;
            if(x1 <= maxx && y1 <= maxy)     // replace (maxx, maxy) to (maxx-1, maxy01) to see tile borders !!
              cbuff[frameBuf->w*y1 + x1] = pixels[iy*blockSize + ix];
          }
        }
      }
      else if (v0Inside || v1Inside || v2Inside || v3Inside)
      {
        // RenderPartiallyCoveredBlock
        float Cy1 = 0.0f;
        float Cy2 = 0.0f;
        float Cy3 = 0.0f;

        for (int y1 = by; y1 < by + blockSize; y1++)
        {
          float Cx1 = Cx1_b + Cy1;
          float Cx2 = Cx2_b + Cy2;
          float Cx3 = Cx3_b + Cy3;

          for (int x1 = bx; x1 < bx + blockSize; x1++)
          {
            const bool hsTest = (Cx1 > HALF_SPACE_EPSILON && Cx2 > HALF_SPACE_EPSILON && Cx3 > HALF_SPACE_EPSILON);
            if (x1 <= maxx && y1 <= maxy && hsTest)
            {
              //const float3 w = areaInv*float3(Cx1, Cx2, Cx3);
              cbuff[frameBuf->w * y1 + x1] = 0x0000FF00;
            }

            Cx1 -= Dy12;
            Cx2 -= Dy23;
            Cx3 -= Dy31;
          }

          Cy1 += Dx12;
          Cy2 += Dx23;
          Cy3 += Dx31;
        }
      }

      Cx1_b -= Dy12*blockSizeF;
      Cx2_b -= Dy23*blockSizeF;
      Cx3_b -= Dy31*blockSizeF;
    }

    Cy1_b += Dx12*blockSizeF;
    Cy2_b += Dx23*blockSizeF;
    Cy3_b += Dx31*blockSizeF;

    offset = nextLine(offset, frameBuf->w, frameBuf->h);
  }
}

#endif //TEST_GL_TOP_PURECPPBLOCK_H
