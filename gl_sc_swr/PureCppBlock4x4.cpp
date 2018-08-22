#include "PureCppBlock4x4.h"

#ifdef WIN32
#undef min
#undef max
#endif

#include <algorithm>

using TriangleLocal = HWImplementationPureCpp::TriangleType;


void RasterizeTriHalfSpace2DFill_Block(const TriangleLocal& tri, int tileMinX, int tileMinY,
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

  float Cy1_b = C1 + Dx12 * miny - Dy12 * minx;
  float Cy2_b = C2 + Dx23 * miny - Dy23 * minx;
  float Cy3_b = C3 + Dx31 * miny - Dy31 * minx;

  int offset = lineOffset(miny, frameBuf->w, frameBuf->h);
  
  constexpr int   blockSize  = 4;
  constexpr float blockSizeF = float(blockSize);

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

      const float Cx1_03 = Cx1_b - Dy12*blockSizeF;
      const float Cx2_03 = Cx2_b - Dy23*blockSizeF;
      const float Cx3_03 = Cx3_b - Dy31*blockSizeF;

      const float Cx1_30 = Cx1_b + Dx12*blockSizeF;
      const float Cx2_30 = Cx2_b + Dx23*blockSizeF;
      const float Cx3_30 = Cx3_b + Dx31*blockSizeF;

      const float Cx1_33 = Cx1_b + Dx12*blockSizeF - Dy12*blockSizeF;
      const float Cx2_33 = Cx2_b + Dx23*blockSizeF - Dy23*blockSizeF;
      const float Cx3_33 = Cx3_b + Dx31*blockSizeF - Dy31*blockSizeF;

      const bool v0Inside = (Cx1_00 > HALF_SPACE_EPSILON && Cx2_00 > HALF_SPACE_EPSILON && Cx3_00 > HALF_SPACE_EPSILON);
      const bool v1Inside = (Cx1_03 > HALF_SPACE_EPSILON && Cx2_03 > HALF_SPACE_EPSILON && Cx3_03 > HALF_SPACE_EPSILON);
      const bool v2Inside = (Cx1_30 > HALF_SPACE_EPSILON && Cx2_30 > HALF_SPACE_EPSILON && Cx3_30 > HALF_SPACE_EPSILON);
      const bool v3Inside = (Cx1_33 > HALF_SPACE_EPSILON && Cx2_33 > HALF_SPACE_EPSILON && Cx3_33 > HALF_SPACE_EPSILON);

      if (v0Inside && v1Inside && v2Inside && v3Inside)
      {
        // RenderBlock
        //
        for (int y1 = by; y1 < by + blockSize; y1++)
          for (int x1 = bx; x1 < bx + blockSize; x1++)
            if(x1 <= maxx && y1 <= maxy)     // replace (maxx, maxy) to (maxx-1, maxy01) to see tile borders !!
              cbuff[frameBuf->w*y1 + x1] = 0x0000FF00;
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
              cbuff[frameBuf->w * y1 + x1] = 0x0000FF00;

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


void RasterizeTriHalfSpace2D_Block(const TriangleLocal& tri, int tileMinX, int tileMinY,
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

  float Cy1_b = C1 + Dx12 * miny - Dy12 * minx;
  float Cy2_b = C2 + Dx23 * miny - Dy23 * minx;
  float Cy3_b = C3 + Dx31 * miny - Dy31 * minx;

  int offset = lineOffset(miny, frameBuf->w, frameBuf->h);

  constexpr int   blockSize  = 4;
  constexpr float blockSizeF = float(blockSize);

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

      if (v0Inside && v1Inside && v2Inside && v3Inside)
      {
        // w1
        //
        const float w1_00 = (Cx1_b + Dx12*0.0f - Dy12*0.0f);
        const float w1_01 = (Cx1_b + Dx12*0.0f - Dy12*1.0f);
        const float w1_02 = (Cx1_b + Dx12*0.0f - Dy12*2.0f);
        const float w1_03 = (Cx1_b + Dx12*0.0f - Dy12*3.0f);

        const float w1_10 = (Cx1_b + Dx12*1.0f - Dy12*0.0f);
        const float w1_11 = (Cx1_b + Dx12*1.0f - Dy12*1.0f);
        const float w1_12 = (Cx1_b + Dx12*1.0f - Dy12*2.0f);
        const float w1_13 = (Cx1_b + Dx12*1.0f - Dy12*3.0f);

        const float w1_20 = (Cx1_b + Dx12*2.0f - Dy12*0.0f);
        const float w1_21 = (Cx1_b + Dx12*2.0f - Dy12*1.0f);
        const float w1_22 = (Cx1_b + Dx12*2.0f - Dy12*2.0f);
        const float w1_23 = (Cx1_b + Dx12*2.0f - Dy12*3.0f);

        const float w1_30 = (Cx1_b + Dx12*3.0f - Dy12*0.0f);
        const float w1_31 = (Cx1_b + Dx12*3.0f - Dy12*1.0f);
        const float w1_32 = (Cx1_b + Dx12*3.0f - Dy12*2.0f);
        const float w1_33 = (Cx1_b + Dx12*3.0f - Dy12*3.0f);

        // w2
        //
        const float w2_00 = (Cx2_b + Dx23*0.0f - Dy23*0.0f);
        const float w2_01 = (Cx2_b + Dx23*0.0f - Dy23*1.0f);
        const float w2_02 = (Cx2_b + Dx23*0.0f - Dy23*2.0f);
        const float w2_03 = (Cx2_b + Dx23*0.0f - Dy23*3.0f);

        const float w2_10 = (Cx2_b + Dx23*1.0f - Dy23*0.0f);
        const float w2_11 = (Cx2_b + Dx23*1.0f - Dy23*1.0f);
        const float w2_12 = (Cx2_b + Dx23*1.0f - Dy23*2.0f);
        const float w2_13 = (Cx2_b + Dx23*1.0f - Dy23*3.0f);

        const float w2_20 = (Cx2_b + Dx23*2.0f - Dy23*0.0f);
        const float w2_21 = (Cx2_b + Dx23*2.0f - Dy23*1.0f);
        const float w2_22 = (Cx2_b + Dx23*2.0f - Dy23*2.0f);
        const float w2_23 = (Cx2_b + Dx23*2.0f - Dy23*3.0f);

        const float w2_30 = (Cx2_b + Dx23*3.0f - Dy23*0.0f);
        const float w2_31 = (Cx2_b + Dx23*3.0f - Dy23*1.0f);
        const float w2_32 = (Cx2_b + Dx23*3.0f - Dy23*2.0f);
        const float w2_33 = (Cx2_b + Dx23*3.0f - Dy23*3.0f);

        // w3
        //
        const float w3_00 = (Cx3_b + Dx31*0.0f - Dy31*0.0f);
        const float w3_01 = (Cx3_b + Dx31*0.0f - Dy31*1.0f);
        const float w3_02 = (Cx3_b + Dx31*0.0f - Dy31*2.0f);
        const float w3_03 = (Cx3_b + Dx31*0.0f - Dy31*3.0f);

        const float w3_10 = (Cx3_b + Dx31*1.0f - Dy31*0.0f);
        const float w3_11 = (Cx3_b + Dx31*1.0f - Dy31*1.0f);
        const float w3_12 = (Cx3_b + Dx31*1.0f - Dy31*2.0f);
        const float w3_13 = (Cx3_b + Dx31*1.0f - Dy31*3.0f);

        const float w3_20 = (Cx3_b + Dx31*2.0f - Dy31*0.0f);
        const float w3_21 = (Cx3_b + Dx31*2.0f - Dy31*1.0f);
        const float w3_22 = (Cx3_b + Dx31*2.0f - Dy31*2.0f);
        const float w3_23 = (Cx3_b + Dx31*2.0f - Dy31*3.0f);

        const float w3_30 = (Cx3_b + Dx31*3.0f - Dy31*0.0f);
        const float w3_31 = (Cx3_b + Dx31*3.0f - Dy31*1.0f);
        const float w3_32 = (Cx3_b + Dx31*3.0f - Dy31*2.0f);
        const float w3_33 = (Cx3_b + Dx31*3.0f - Dy31*3.0f);

        // colors
        //
        const float4 color00 = areaInv*( tri.c1*w1_00 + tri.c2*w2_00 + tri.c3*w3_00 );
        const float4 color01 = areaInv*( tri.c1*w1_01 + tri.c2*w2_01 + tri.c3*w3_01 );
        const float4 color02 = areaInv*( tri.c1*w1_02 + tri.c2*w2_02 + tri.c3*w3_02 );
        const float4 color03 = areaInv*( tri.c1*w1_03 + tri.c2*w2_03 + tri.c3*w3_03 );

        const float4 color10 = areaInv*( tri.c1*w1_10 + tri.c2*w2_10 + tri.c3*w3_10 );
        const float4 color11 = areaInv*( tri.c1*w1_11 + tri.c2*w2_11 + tri.c3*w3_11 );
        const float4 color12 = areaInv*( tri.c1*w1_12 + tri.c2*w2_12 + tri.c3*w3_12 );
        const float4 color13 = areaInv*( tri.c1*w1_13 + tri.c2*w2_13 + tri.c3*w3_13 );

        const float4 color20 = areaInv*( tri.c1*w1_20 + tri.c2*w2_20 + tri.c3*w3_20 );
        const float4 color21 = areaInv*( tri.c1*w1_21 + tri.c2*w2_21 + tri.c3*w3_21 );
        const float4 color22 = areaInv*( tri.c1*w1_22 + tri.c2*w2_22 + tri.c3*w3_22 );
        const float4 color23 = areaInv*( tri.c1*w1_23 + tri.c2*w2_23 + tri.c3*w3_23 );

        const float4 color30 = areaInv*( tri.c1*w1_30 + tri.c2*w2_30 + tri.c3*w3_30 );
        const float4 color31 = areaInv*( tri.c1*w1_31 + tri.c2*w2_31 + tri.c3*w3_31 );
        const float4 color32 = areaInv*( tri.c1*w1_32 + tri.c2*w2_32 + tri.c3*w3_32 );
        const float4 color33 = areaInv*( tri.c1*w1_33 + tri.c2*w2_33 + tri.c3*w3_33 );

        // (3) calc color32i
        //
        int colors_int32[16];

        colors_int32[0]  = RealColorToUint32_BGRA(color00); // RealColorToUint32_BGRA(color);
        colors_int32[1]  = RealColorToUint32_BGRA(color01);
        colors_int32[2]  = RealColorToUint32_BGRA(color02);
        colors_int32[3]  = RealColorToUint32_BGRA(color03);
        colors_int32[4]  = RealColorToUint32_BGRA(color10);
        colors_int32[5]  = RealColorToUint32_BGRA(color11);
        colors_int32[6]  = RealColorToUint32_BGRA(color12);
        colors_int32[7]  = RealColorToUint32_BGRA(color13);
        colors_int32[8]  = RealColorToUint32_BGRA(color20);
        colors_int32[9]  = RealColorToUint32_BGRA(color21);
        colors_int32[10] = RealColorToUint32_BGRA(color22);
        colors_int32[11] = RealColorToUint32_BGRA(color23);
        colors_int32[12] = RealColorToUint32_BGRA(color30);
        colors_int32[13] = RealColorToUint32_BGRA(color31);
        colors_int32[14] = RealColorToUint32_BGRA(color32);
        colors_int32[15] = RealColorToUint32_BGRA(color33);

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
              cbuff[frameBuf->w*y1 + x1] = colors_int32[iy*blockSize + ix];
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
              const float3 w = areaInv*float3(Cx1, Cx2, Cx3);
              cbuff[frameBuf->w * y1 + x1] = RealColorToUint32_BGRA( tri.c1*w.x + tri.c2*w.y + tri.c3*w.z );
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



void HWImplBlock4x4::RasterizeTriangle(RasterOp a_ropT, BlendOp a_bopT, const TriangleType& tri, int tileMinX, int tileMinY,
                                       FrameBuffer* frameBuf)
{
  RasterizeTriHalfSpace2D_Block(tri, tileMinX, tileMinY,
                                frameBuf);

  //RasterizeTriHalfSpace2DFill_Block(tri, tileMinX, tileMinY,
    //                                frameBuf);
}