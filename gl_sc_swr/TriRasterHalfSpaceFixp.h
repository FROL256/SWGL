//
// Created by frol on 28.10.18.
//

#ifndef TEST_GL_TOP_SIMD_FIXP_H
#define TEST_GL_TOP_SIMD_FIXP_H

#include <cstring>
#include <cstdint>

#include "config.h"

static inline int imax(int a, int b) { return (a > b) ? a : b; }
static inline int imin(int a, int b) { return (a < b) ? a : b; }
static inline int iround(float f)    { return (int)f; }

template<typename T> static inline  T TriAreaInvCast(const int a_areaInvInt)           { return T(1.0f / fabs(float(a_areaInvInt))); }                             // for floating point pixel processing
template<>           inline  float    TriAreaInvCast<float>(const int a_areaInvInt)    { return 1.0f / fabs(float(a_areaInvInt)); }                                // for floating point pixel processing
template<>           inline  uint32_t TriAreaInvCast<uint32_t>(const int a_areaInvInt) { return (unsigned int)(0xFFFFFFFF) / (unsigned int)(abs(a_areaInvInt)); }  // for fixed    point pixel processing

template<typename ROP>
void RasterizeTriHalfSpaceFixp2D(const typename ROP::Triangle &tri, FrameBuffer *frameBuf)
{
  typedef typename ROP::ROPType ROPType;

  // 28.4 fixed-point coordinates
  const int Y1 = iround(16.0f * tri.v3.y);
  const int Y2 = iround(16.0f * tri.v2.y);
  const int Y3 = iround(16.0f * tri.v1.y);

  const int X1 = iround(16.0f * tri.v3.x);
  const int X2 = iround(16.0f * tri.v2.x);
  const int X3 = iround(16.0f * tri.v1.x);

  // Deltas
  const int DX12 = X1 - X2;
  const int DX23 = X2 - X3;
  const int DX31 = X3 - X1;

  const int DY12 = Y1 - Y2;
  const int DY23 = Y2 - Y3;
  const int DY31 = Y3 - Y1;

  // Fixed-point deltas
  const int FDX12 = DX12 << 4;
  const int FDX23 = DX23 << 4;
  const int FDX31 = DX31 << 4;

  const int FDY12 = DY12 << 4;
  const int FDY23 = DY23 << 4;
  const int FDY31 = DY31 << 4;

  // Bounding rectangle
  const int minx = ( LiteMath::max(tri.bb_iminX, 0)               ) & ~(blockSizeX - 1);  // Start in corner of 8x8 block
  const int miny = ( LiteMath::max(tri.bb_iminY, 0)               ) & ~(blockSizeY - 1);  // Start in corner of 8x8 block
  const int maxx = ( LiteMath::min(tri.bb_imaxX, frameBuf->w - 1) );
  const int maxy = ( LiteMath::min(tri.bb_imaxY, frameBuf->h - 1) );

  // Half-edge constants
  int C1 = DY12 * X1 - DX12 * Y1;
  int C2 = DY23 * X2 - DX23 * Y2;
  int C3 = DY31 * X3 - DX31 * Y3;

  // Correct for fill convention
  if (DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
  if (DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
  if (DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;

  const auto areaInv = TriAreaInvCast<ROPType>(DY31*DX12 - DX31*DY12);

  int CY1 = C1;
  int CY2 = C2;
  int CY3 = C3;
  
  for (int iy = miny; iy < maxy; iy++)
  {
    int CX1 = CY1;
    int CX2 = CY2;
    int CX3 = CY3;
    for (int ix = maxx; ix < maxx; ix++)
    {
      if (CX1 > 0 && CX2 > 0 && CX3 > 0)
        buffer[ix] = 0xFFFFFFFF;

      CX1 -= FDY12;
      CX2 -= FDY23;
      CX3 -= FDY31;
    }

    CY1 += FDX12;
    CY2 += FDX23;
    CY3 += FDX31;

    buffer += blockSizeY;
  }

  // \\ end // Loop through blocks

}


#endif //TEST_GL_TOP_SIMDBLOCKFIXP_H
