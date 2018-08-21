#include "PureCppBlock4x4.h"

#ifdef WIN32
#undef min
#undef max
#endif

#include <algorithm>

using TriangleLocal = HWImplementationPureCpp::TriangleType;


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

  float Cy1 = C1 + Dx12 * miny - Dy12 * minx;
  float Cy2 = C2 + Dx23 * miny - Dy23 * minx;
  float Cy3 = C3 + Dx31 * miny - Dy31 * minx;

  int offset = lineOffset(miny, frameBuf->w, frameBuf->h);
  
  constexpr int   blockSize  = 4;
  constexpr float blockSizeF = float(blockSize);

  // Scan through bounding rectangle
  for (int by = miny; by <= maxy; by += blockSize)
  {
    // Start value for horizontal scan
    float Cx1 = Cy1;
    float Cx2 = Cy2;
    float Cx3 = Cy3;

    for (int bx = minx; bx <= maxx; bx+= blockSize)
    {
      const float Cx1_00 = Cx1;
      const float Cx2_00 = Cx2;
      const float Cx3_00 = Cx3;

      const float Cx1_01 = Cx1 - Dy12*blockSizeF;
      const float Cx2_01 = Cx2 - Dy23*blockSizeF;
      const float Cx3_01 = Cx3 - Dy31*blockSizeF;

      const float Cx1_10 = Cx1 + Dx12*blockSizeF;
      const float Cx2_10 = Cx2 + Dx23*blockSizeF;
      const float Cx3_10 = Cx3 + Dx31*blockSizeF;

      const float Cx1_11 = Cx1 + Dx12*blockSizeF - Dy12*blockSizeF;
      const float Cx2_11 = Cx2 + Dx23*blockSizeF - Dy23*blockSizeF;
      const float Cx3_11 = Cx3 + Dx31*blockSizeF - Dy31*blockSizeF;

      const bool v0Inside = (Cx1_00 > HALF_SPACE_EPSILON && Cx2_00 > HALF_SPACE_EPSILON && Cx3_00 > HALF_SPACE_EPSILON);
      const bool v1Inside = (Cx1_01 > HALF_SPACE_EPSILON && Cx2_01 > HALF_SPACE_EPSILON && Cx3_01 > HALF_SPACE_EPSILON);
      const bool v2Inside = (Cx1_10 > HALF_SPACE_EPSILON && Cx2_10 > HALF_SPACE_EPSILON && Cx3_10 > HALF_SPACE_EPSILON);
      const bool v3Inside = (Cx1_11 > HALF_SPACE_EPSILON && Cx2_11 > HALF_SPACE_EPSILON && Cx3_11 > HALF_SPACE_EPSILON);

      if (v0Inside && v1Inside && v2Inside && v3Inside)
      {
        // RenderBlock(x,j,BlockSize);
        for (int y1 = by; y1 < by + blockSize; y1++)
          for (int x1 = bx; x1 < bx + blockSize; x1++)
            if(x1 < frameBuf->w && y1 < frameBuf->h)
              cbuff[frameBuf->w*y1 + x1] = 0x0000FF00;
      }
      else if (v0Inside || v1Inside || v2Inside || v3Inside)
      {
        // RenderPartiallyCoveredBlock(j,x, BlockSize);
        for (int y1 = by; y1 < by + blockSize; y1++)
          for (int x1 = bx; x1 < bx + blockSize; x1++)
            if(x1 < frameBuf->w && y1 < frameBuf->h)
              cbuff[frameBuf->w*y1 + x1] = 0x00FF0000;
      }


      Cx1 -= Dy12*blockSizeF;
      Cx2 -= Dy23*blockSizeF;
      Cx3 -= Dy31*blockSizeF;
    }

    Cy1 += Dx12*blockSizeF;
    Cy2 += Dx23*blockSizeF;
    Cy3 += Dx31*blockSizeF;

    offset = nextLine(offset, frameBuf->w, frameBuf->h);
  }
}



void HWImplBlock4x4::RasterizeTriangle(RasterOp a_ropT, BlendOp a_bopT, const TriangleType& tri, int tileMinX, int tileMinY,
                                       FrameBuffer* frameBuf)
{
  RasterizeTriHalfSpace2D_Block(tri, tileMinX, tileMinY,
                                frameBuf);
}