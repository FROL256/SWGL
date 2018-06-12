#include "PureCppBlock4x4.h"

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

  const int halfY     = tri.bb_iminY + ((tri.bb_imaxY - tri.bb_iminY) >> 1) & ~(QSIZE - 1);
  const int BlockSize = QSIZE;


  // Find Topmost vertex, calculate leftPoint and rightPoint ... (#NOT_SURE !!!)
  const int leftPoint  = tri.bb_iminX;
  const int rightPoint = tri.bb_imaxX;

  //Loop j=minY to (halfY + BlockSize) step=BlockSize
  for (int j = tri.bb_iminY; j < halfY + BlockSize; j += BlockSize)
  {
    const int midPoint = leftPoint + ((rightPoint - leftPoint) >> 1) & ~(BlockSize - 1);

    //Loop k=0 to 2 step=1
    //Loop x to (q > 0 ? x < maxX : x > minX - BlockSize) step=q
    for (int x = midPoint; x < tri.bb_imaxX; x += BlockSize) // k = 0
    {

    }

    for (int x = midPoint; x > tri.bb_iminX - BlockSize; x -= BlockSize) // k = 1
    {

    }


  }

}


void HWImplBlock4x4::RasterizeTriangle(ROP_TYPE a_ropT, const TriangleType& tri, int tileMinX, int tileMinY,
                                       FrameBuffer* frameBuf)
{
  RasterizeTriHalfSpace2D(tri, tileMinX, tileMinY,
                          frameBuf);
}