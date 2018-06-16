#include "PureCppBlock4x4.h"

#ifdef WIN32
#undef min
#undef max
#endif

#include <algorithm>

using TriangleLocal = HWImplementationPureCpp::TriangleType;

#define QSIZE 4

void RasterizeTriHalfSpace2D(const TriangleLocal& tri, int tileMinX, int tileMinY,
                             FrameBuffer* frameBuf)
{
  // already done (triangle set up): Calculate triangle bounding box(minX, minY, maxX, maxY);
  // already done (triangle set up): Clip box against render target bounds(minX, minY, maxX, maxY);
  //
  
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

  //int offset = lineOffset(miny, frameBuf->w, frameBuf->h);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  const int halfY     = tri.bb_iminY + ((tri.bb_imaxY - tri.bb_iminY) >> 1) & ~(QSIZE - 1);
  const int BlockSize = QSIZE;

  // Find Topmost vertex, calculate leftPoint and rightPoint ... (#NOT_SURE !!!)
  int leftPoint  = tri.bb_iminX;
  int rightPoint = tri.bb_imaxX;

  //Loop j=minY to (halfY + BlockSize) step=BlockSize
  for (int j = tri.bb_iminY; j < halfY + BlockSize; j += BlockSize)
  {
    const int midPoint = leftPoint + ((rightPoint - leftPoint) >> 1) & ~(BlockSize - 1);

    //Loop k=0 to 2 step=1
    //Loop x to (q > 0 ? x < maxX : x > minX - BlockSize) step=q
    for (int x = midPoint; x < tri.bb_imaxX; x += BlockSize) // k = 0
    {
      const float fx1 = float(x);
      const float fy1 = float(j);
      const float fx2 = float(x + BlockSize - 1);
      const float fy2 = float(j + BlockSize - 1);

      float Cx1_00 = C1 + Dx12 * fy1 - Dy12 * fx1;
      float Cx2_00 = C2 + Dx23 * fy1 - Dy23 * fx1;
      float Cx3_00 = C3 + Dx31 * fy1 - Dy31 * fx1;
       
      float Cx1_01 = C1 + Dx12 * fy1 - Dy12 * fx2;
      float Cx2_01 = C2 + Dx23 * fy1 - Dy23 * fx2;
      float Cx3_01 = C3 + Dx31 * fy1 - Dy31 * fx2;

      float Cx1_10 = C1 + Dx12 * fy2 - Dy12 * fx1;
      float Cx2_10 = C2 + Dx23 * fy2 - Dy23 * fx1;
      float Cx3_10 = C3 + Dx31 * fy2 - Dy31 * fx1;

      float Cx1_11 = C1 + Dx12 * fy2 - Dy12 * fx2;
      float Cx2_11 = C2 + Dx23 * fy2 - Dy23 * fx2;
      float Cx3_11 = C3 + Dx31 * fy2 - Dy31 * fx2;

      const bool v0Inside = (Cx1_00 > HALF_SPACE_EPSILON && Cx2_00 > HALF_SPACE_EPSILON && Cx3_00 > HALF_SPACE_EPSILON);
      const bool v1Inside = (Cx1_01 > HALF_SPACE_EPSILON && Cx2_01 > HALF_SPACE_EPSILON && Cx3_01 > HALF_SPACE_EPSILON);
      const bool v2Inside = (Cx1_10 > HALF_SPACE_EPSILON && Cx2_10 > HALF_SPACE_EPSILON && Cx3_10 > HALF_SPACE_EPSILON);
      const bool v3Inside = (Cx1_11 > HALF_SPACE_EPSILON && Cx2_11 > HALF_SPACE_EPSILON && Cx3_11 > HALF_SPACE_EPSILON);

      if (v0Inside && v1Inside && v2Inside && v3Inside)
      {
        // RenderBlock(x,j,BlockSize);
        for (int y1 = j; y1 < j + BlockSize; y1++)
          for (int x1 = x; x1 < x + BlockSize; x1++)
            cbuff[frameBuf->w*y1 + x1] = 0x0000FF00;
      }
      else if (v0Inside || v1Inside || v2Inside || v3Inside)
      {
        // RenderPartiallyCoveredBlock(j,x, BlockSize);
        for (int y1 = j; y1 < j + BlockSize; y1++)
          for (int x1 = x; x1 < x + BlockSize; x1++)
            cbuff[frameBuf->w*y1 + x1] = 0x00FF0000;
        //break;
      }

    }

    
    for (int x = midPoint; x > tri.bb_iminX - BlockSize; x -= BlockSize) // k = 1
    {
      const float fx1 = float(x);
      const float fy1 = float(j);
      const float fx2 = float(x + BlockSize - 1);
      const float fy2 = float(j + BlockSize - 1);

      float Cx1_00 = C1 + Dx12 * fy1 - Dy12 * fx1;
      float Cx2_00 = C2 + Dx23 * fy1 - Dy23 * fx1;
      float Cx3_00 = C3 + Dx31 * fy1 - Dy31 * fx1;

      float Cx1_01 = C1 + Dx12 * fy1 - Dy12 * fx2;
      float Cx2_01 = C2 + Dx23 * fy1 - Dy23 * fx2;
      float Cx3_01 = C3 + Dx31 * fy1 - Dy31 * fx2;

      float Cx1_10 = C1 + Dx12 * fy2 - Dy12 * fx1;
      float Cx2_10 = C2 + Dx23 * fy2 - Dy23 * fx1;
      float Cx3_10 = C3 + Dx31 * fy2 - Dy31 * fx1;

      float Cx1_11 = C1 + Dx12 * fy2 - Dy12 * fx2;
      float Cx2_11 = C2 + Dx23 * fy2 - Dy23 * fx2;
      float Cx3_11 = C3 + Dx31 * fy2 - Dy31 * fx2;

      const bool v0Inside = (Cx1_00 > HALF_SPACE_EPSILON && Cx2_00 > HALF_SPACE_EPSILON && Cx3_00 > HALF_SPACE_EPSILON);
      const bool v1Inside = (Cx1_01 > HALF_SPACE_EPSILON && Cx2_01 > HALF_SPACE_EPSILON && Cx3_01 > HALF_SPACE_EPSILON);
      const bool v2Inside = (Cx1_10 > HALF_SPACE_EPSILON && Cx2_10 > HALF_SPACE_EPSILON && Cx3_10 > HALF_SPACE_EPSILON);
      const bool v3Inside = (Cx1_11 > HALF_SPACE_EPSILON && Cx2_11 > HALF_SPACE_EPSILON && Cx3_11 > HALF_SPACE_EPSILON);

      if (v0Inside && v1Inside && v2Inside && v3Inside)
      {
        // RenderBlock(x,j,BlockSize);
        for (int y1 = j; y1 < j + BlockSize; y1++)
          for (int x1 = x; x1 < x + BlockSize; x1++)
            cbuff[frameBuf->w*y1 + x1] = 0x0000FF00;
      }
      else if (v0Inside || v1Inside || v2Inside || v3Inside)
      {
        // RenderPartiallyCoveredBlock(j,x, BlockSize);
        for (int y1 = j; y1 < j + BlockSize; y1++)
          for (int x1 = x; x1 < x + BlockSize; x1++)
            cbuff[frameBuf->w*y1 + x1] = 0x00FF0000;
        //break;
      }
     
     
    }


  }

  // Find Bottommost vertex, calculate leftPoint and rightPoint
  // Loop j = maxY &~(BlockSize - 1) to halfY step = -BlockSize
  //
  const int startY = tri.bb_imaxY & ~(BlockSize - 1);
  for (int j = startY; j >= halfY; j -= BlockSize)
  {
    const int midPoint = leftPoint + ((rightPoint - leftPoint) >> 1) & ~(BlockSize - 1);

    //Loop k=0 to 2 step=1
    //Loop x to (q > 0 ? x < maxX : x > minX - BlockSize) step=q
    for (int x = midPoint; x < tri.bb_imaxX; x += BlockSize) // k = 0
    {
      const float fx1 = float(x);
      const float fy1 = float(j);
      const float fx2 = float(x + BlockSize - 1);
      const float fy2 = float(j + BlockSize - 1);

      float Cx1_00 = C1 + Dx12 * fy1 - Dy12 * fx1;
      float Cx2_00 = C2 + Dx23 * fy1 - Dy23 * fx1;
      float Cx3_00 = C3 + Dx31 * fy1 - Dy31 * fx1;

      float Cx1_01 = C1 + Dx12 * fy1 - Dy12 * fx2;
      float Cx2_01 = C2 + Dx23 * fy1 - Dy23 * fx2;
      float Cx3_01 = C3 + Dx31 * fy1 - Dy31 * fx2;

      float Cx1_10 = C1 + Dx12 * fy2 - Dy12 * fx1;
      float Cx2_10 = C2 + Dx23 * fy2 - Dy23 * fx1;
      float Cx3_10 = C3 + Dx31 * fy2 - Dy31 * fx1;

      float Cx1_11 = C1 + Dx12 * fy2 - Dy12 * fx2;
      float Cx2_11 = C2 + Dx23 * fy2 - Dy23 * fx2;
      float Cx3_11 = C3 + Dx31 * fy2 - Dy31 * fx2;

      const bool v0Inside = (Cx1_00 > HALF_SPACE_EPSILON && Cx2_00 > HALF_SPACE_EPSILON && Cx3_00 > HALF_SPACE_EPSILON);
      const bool v1Inside = (Cx1_01 > HALF_SPACE_EPSILON && Cx2_01 > HALF_SPACE_EPSILON && Cx3_01 > HALF_SPACE_EPSILON);
      const bool v2Inside = (Cx1_10 > HALF_SPACE_EPSILON && Cx2_10 > HALF_SPACE_EPSILON && Cx3_10 > HALF_SPACE_EPSILON);
      const bool v3Inside = (Cx1_11 > HALF_SPACE_EPSILON && Cx2_11 > HALF_SPACE_EPSILON && Cx3_11 > HALF_SPACE_EPSILON);

      if (v0Inside && v1Inside && v2Inside && v3Inside)
      {
        // RenderBlock(x,j,BlockSize);
        for (int y1 = j; y1 < j + BlockSize; y1++)
          for (int x1 = x; x1 < x + BlockSize; x1++)
            cbuff[frameBuf->w*y1 + x1] = 0x0000FF00;
      }
      else if (v0Inside || v1Inside || v2Inside || v3Inside)
      {
        // RenderPartiallyCoveredBlock(j,x, BlockSize);
        for (int y1 = j; y1 < j + BlockSize; y1++)
          for (int x1 = x; x1 < x + BlockSize; x1++)
            cbuff[frameBuf->w*y1 + x1] = 0x00FF0000;
        //break;
      }

    }


    for (int x = midPoint; x > tri.bb_iminX - BlockSize; x -= BlockSize) // k = 1
    {
      const float fx1 = float(x);
      const float fy1 = float(j);
      const float fx2 = float(x + BlockSize - 1);
      const float fy2 = float(j + BlockSize - 1);

      float Cx1_00 = C1 + Dx12 * fy1 - Dy12 * fx1;
      float Cx2_00 = C2 + Dx23 * fy1 - Dy23 * fx1;
      float Cx3_00 = C3 + Dx31 * fy1 - Dy31 * fx1;

      float Cx1_01 = C1 + Dx12 * fy1 - Dy12 * fx2;
      float Cx2_01 = C2 + Dx23 * fy1 - Dy23 * fx2;
      float Cx3_01 = C3 + Dx31 * fy1 - Dy31 * fx2;

      float Cx1_10 = C1 + Dx12 * fy2 - Dy12 * fx1;
      float Cx2_10 = C2 + Dx23 * fy2 - Dy23 * fx1;
      float Cx3_10 = C3 + Dx31 * fy2 - Dy31 * fx1;

      float Cx1_11 = C1 + Dx12 * fy2 - Dy12 * fx2;
      float Cx2_11 = C2 + Dx23 * fy2 - Dy23 * fx2;
      float Cx3_11 = C3 + Dx31 * fy2 - Dy31 * fx2;

      const bool v0Inside = (Cx1_00 > HALF_SPACE_EPSILON && Cx2_00 > HALF_SPACE_EPSILON && Cx3_00 > HALF_SPACE_EPSILON);
      const bool v1Inside = (Cx1_01 > HALF_SPACE_EPSILON && Cx2_01 > HALF_SPACE_EPSILON && Cx3_01 > HALF_SPACE_EPSILON);
      const bool v2Inside = (Cx1_10 > HALF_SPACE_EPSILON && Cx2_10 > HALF_SPACE_EPSILON && Cx3_10 > HALF_SPACE_EPSILON);
      const bool v3Inside = (Cx1_11 > HALF_SPACE_EPSILON && Cx2_11 > HALF_SPACE_EPSILON && Cx3_11 > HALF_SPACE_EPSILON);

      if (v0Inside && v1Inside && v2Inside && v3Inside)
      {
        // RenderBlock(x,j,BlockSize);
        for (int y1 = j; y1 < j + BlockSize; y1++)
          for (int x1 = x; x1 < x + BlockSize; x1++)
            cbuff[frameBuf->w*y1 + x1] = 0x0000FF00;
      }
      else if (v0Inside || v1Inside || v2Inside || v3Inside)
      {
        // RenderPartiallyCoveredBlock(j,x, BlockSize);
        for (int y1 = j; y1 < j + BlockSize; y1++)
          for (int x1 = x; x1 < x + BlockSize; x1++)
            cbuff[frameBuf->w*y1 + x1] = 0x00FF0000;
        //break;
      }
    }

  }

}


void HWImplBlock4x4::RasterizeTriangle(ROP_TYPE a_ropT, const TriangleType& tri, int tileMinX, int tileMinY,
                                       FrameBuffer* frameBuf)
{
  RasterizeTriHalfSpace2D(tri, tileMinX, tileMinY,
                          frameBuf);
}