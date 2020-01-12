//
// Created by frol on 28.10.18.
//

#ifndef TEST_GL_TOP_SIMDBLOCKFIXP_H
#define TEST_GL_TOP_SIMDBLOCKFIXP_H

#include <cstring>
#include <cstdint>

#include "config.h"

static inline int imax(int a, int b) { return (a > b) ? a : b; }
static inline int imin(int a, int b) { return (a < b) ? a : b; }
static inline int iround(float f)    { return (int)f; }

template<typename T> inline  T        TriAreaInvCast(const int a_areaInvInt)           { return T(1.0f / fabs(float(a_areaInvInt))); }                             // for floating point pixel processing
template<>           inline  float    TriAreaInvCast<float>(const int a_areaInvInt)    { return 1.0f / fabs(float(a_areaInvInt)); }                                // for floating point pixel processing
template<>           inline  uint32_t TriAreaInvCast<uint32_t>(const int a_areaInvInt) { return (unsigned int)(0xFFFFFFFF) / (unsigned int)(abs(a_areaInvInt)); }  // for fixed    point pixel processing

template<typename ROP, int blockSizeX, int blockSizeY>
void RasterizeTriHalfSpaceBlockFixp2D(const typename ROP::Triangle &tri, int tileMinX, int tileMinY,
                                      FrameBuffer *frameBuf)
{
  constexpr int BLOCK_ITER = (blockSizeX*blockSizeY)/ROP::n;       
  constexpr int BMULT      = ROP::n/blockSizeX;   

  typedef typename ROP::ROPType ROPType;

  // 28.4 fixed-point coordinates
  const int Y1 = iround(16.0f * tri.v3.y) - 16*tileMinY;
  const int Y2 = iround(16.0f * tri.v2.y) - 16*tileMinY;
  const int Y3 = iround(16.0f * tri.v1.y) - 16*tileMinY;

  const int X1 = iround(16.0f * tri.v3.x) - 16*tileMinX;
  const int X2 = iround(16.0f * tri.v2.x) - 16*tileMinX;
  const int X3 = iround(16.0f * tri.v1.x) - 16*tileMinX;

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
  const int minx = ( LiteMath::max(tri.bb_iminX - tileMinX, 0)               ) & ~(blockSizeX - 1);  // Start in corner of 8x8 block
  const int miny = ( LiteMath::max(tri.bb_iminY - tileMinY, 0)               ) & ~(blockSizeY - 1);  // Start in corner of 8x8 block
  const int maxx = ( LiteMath::min(tri.bb_imaxX - tileMinX, frameBuf->w - 1) );
  const int maxy = ( LiteMath::min(tri.bb_imaxY - tileMinY, frameBuf->h - 1) );

  // Half-edge constants
  int C1 = DY12 * X1 - DX12 * Y1;
  int C2 = DY23 * X2 - DX23 * Y2;
  int C3 = DY31 * X3 - DX31 * Y3;

  // Correct for fill convention
  if (DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
  if (DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
  if (DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;

  const auto areaInv = TriAreaInvCast<ROPType>(DY31*DX12 - DX31*DY12);
  
  // Loop through blocks
  for (int y = miny; y < maxy; y += blockSizeY)
  {
    for (int x = minx; x < maxx; x += blockSizeX)
    {
      // Corners of block
      const int x0 = x << 4;
      const int x1 = (x + blockSizeX - 1) << 4;
      const int y0 = y << 4;
      const int y1 = (y + blockSizeY - 1) << 4;

      // Evaluate half-space functions
      const bool a00 = C1 + DX12 * y0 - DY12 * x0 > 0;
      const bool a10 = C1 + DX12 * y0 - DY12 * x1 > 0;
      const bool a01 = C1 + DX12 * y1 - DY12 * x0 > 0;
      const bool a11 = C1 + DX12 * y1 - DY12 * x1 > 0;
      const int  a   = (a00 << 0) | (a10 << 1) | (a01 << 2) | (a11 << 3);

      const bool b00 = C2 + DX23 * y0 - DY23 * x0 > 0;
      const bool b10 = C2 + DX23 * y0 - DY23 * x1 > 0;
      const bool b01 = C2 + DX23 * y1 - DY23 * x0 > 0;
      const bool b11 = C2 + DX23 * y1 - DY23 * x1 > 0;
      const int  b   = (b00 << 0) | (b10 << 1) | (b01 << 2) | (b11 << 3);

      const bool c00 = C3 + DX31 * y0 - DY31 * x0 > 0;
      const bool c10 = C3 + DX31 * y0 - DY31 * x1 > 0;
      const bool c01 = C3 + DX31 * y1 - DY31 * x0 > 0;
      const bool c11 = C3 + DX31 * y1 - DY31 * x1 > 0;
      const int  c   = (c00 << 0) | (c10 << 1) | (c01 << 2) | (c11 << 3);

      // Skip block when outside an edge
      if (a == 0x0 || b == 0x0 || c == 0x0)
        continue;

      auto* buffer = frameBuf->TileColor(x,y);
      
      // Accept whole block when totally covered
      if (a == 0xF && b == 0xF && c == 0xF)
      {
        int CY1 = C1 + DX12 * y0 - DY12 * x0;
        int CY2 = C2 + DX23 * y0 - DY23 * x0;
        int CY3 = C3 + DX31 * y0 - DY31 * x0;

        for (int iy = 0; iy < BLOCK_ITER; iy++)
        {
          ROP::Block(tri, CY1, CY3, FDY12, FDY31, FDX12, FDX31, areaInv,
                     buffer);

          CY1 += FDX12*BMULT;
          CY2 += FDX23*BMULT;
          CY3 += FDX31*BMULT;

          buffer += blockSizeX*BMULT;
        }
      }
      else
      {
        int CY1 = C1 + DX12 * y0 - DY12 * x0;
        int CY2 = C2 + DX23 * y0 - DY23 * x0;
        int CY3 = C3 + DX31 * y0 - DY31 * x0;

        for (int iy = y; iy < y + blockSizeY; iy++)
        {
          int CX1 = CY1;
          int CX2 = CY2;
          int CX3 = CY3;

          for (int ix = 0; ix < blockSizeX; ix++)
          {
            if (CX1 > 0 && CX2 > 0 && CX3 > 0)
              buffer[ix] = ROP::Pixel(tri, CX1, CX3, areaInv);

            CX1 -= FDY12;
            CX2 -= FDY23;
            CX3 -= FDY31;
          }

          CY1 += FDX12;
          CY2 += FDX23;
          CY3 += FDX31;

          buffer += blockSizeY;
        }
      }
      
    
    }
  }
  // \\ end // Loop through blocks

}

template<typename T>
int EvalSubpixelBits(const T& tri) // #TODO: implement this carefully
{
  if (tri.triSize < 1024)
    return 4;
  if (tri.triSize < 32768)
    return 3;
  else if (tri.triSize < 65536)
    return 2;
  else
    return 0;
}

template<typename ROP, int blockSizeX, int blockSizeY>
void RasterizeTriHalfSpaceBlockFixp3D(const typename ROP::Triangle &tri, int tileMinX, int tileMinY,
                                      FrameBuffer *frameBuf)
{
  constexpr int BLOCK_ITER = (blockSizeX*blockSizeY)/ROP::n;       
  constexpr int BMULT      = ROP::n/blockSizeX;   

  // 28.4 or 30.2 or else fixed-point coordinates
  //
  const int   SUBPIXELBITS  = EvalSubpixelBits(tri); // clipped (by a nearest clipping plane) triangles could be very big, so we have to estimate this or clip triangle also in 2D.
  const int   SUBPIXELMULTI = (1 << SUBPIXELBITS);
  const float SUBPIXELMULTF = float(SUBPIXELMULTI);

  const int Y1 = iround(tri.v3.y*SUBPIXELMULTF) - tileMinY*SUBPIXELMULTI;
  const int Y2 = iround(tri.v2.y*SUBPIXELMULTF) - tileMinY*SUBPIXELMULTI;
  const int Y3 = iround(tri.v1.y*SUBPIXELMULTF) - tileMinY*SUBPIXELMULTI;

  const int X1 = iround(tri.v3.x*SUBPIXELMULTF) - tileMinX*SUBPIXELMULTI;
  const int X2 = iround(tri.v2.x*SUBPIXELMULTF) - tileMinX*SUBPIXELMULTI;
  const int X3 = iround(tri.v1.x*SUBPIXELMULTF) - tileMinX*SUBPIXELMULTI;

  // Deltas
  const int DX12 = X1 - X2;
  const int DX23 = X2 - X3;
  const int DX31 = X3 - X1;

  const int DY12 = Y1 - Y2;
  const int DY23 = Y2 - Y3;
  const int DY31 = Y3 - Y1;

  // Fixed-point deltas
  const int FDX12 = DX12 << SUBPIXELBITS;
  const int FDX23 = DX23 << SUBPIXELBITS;
  const int FDX31 = DX31 << SUBPIXELBITS;

  const int FDY12 = DY12 << SUBPIXELBITS;
  const int FDY23 = DY23 << SUBPIXELBITS;
  const int FDY31 = DY31 << SUBPIXELBITS;

  // Bounding rectangle
  const int minx = ( LiteMath::max(tri.bb_iminX - tileMinX, 0)               ) & ~(blockSizeX - 1);  // Start in corner of 8x8 block
  const int miny = ( LiteMath::max(tri.bb_iminY - tileMinY, 0)               ) & ~(blockSizeY - 1);  // Start in corner of 8x8 block
  const int maxx = ( LiteMath::min(tri.bb_imaxX - tileMinX, frameBuf->w - 1) );
  const int maxy = ( LiteMath::min(tri.bb_imaxY - tileMinY, frameBuf->h - 1) );

  // Half-edge constants
  int C1 = DY12 * X1 - DX12 * Y1;
  int C2 = DY23 * X2 - DX23 * Y2;
  int C3 = DY31 * X3 - DX31 * Y3;

  // Correct for fill convention
  if (DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
  if (DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
  if (DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;

  const float areaInv = 1.0f / fabs(float(DY31*DX12 - DX31*DY12));
  
  // Loop through blocks
  for (int y = miny; y < maxy; y += blockSizeY)
  {
    for (int x = minx; x < maxx; x += blockSizeX)
    {
      // Corners of block
      const int x0 = x                    << SUBPIXELBITS;
      const int y0 = y                    << SUBPIXELBITS;
      const int x1 = (x + blockSizeX - 1) << SUBPIXELBITS;
      const int y1 = (y + blockSizeY - 1) << SUBPIXELBITS;

      // Evaluate half-space functions
      const bool a00 = C1 + DX12 * y0 - DY12 * x0 > 0;
      const bool a10 = C1 + DX12 * y0 - DY12 * x1 > 0;
      const bool a01 = C1 + DX12 * y1 - DY12 * x0 > 0;
      const bool a11 = C1 + DX12 * y1 - DY12 * x1 > 0;
      const int  a   = (a00 << 0) | (a10 << 1) | (a01 << 2) | (a11 << 3);

      const bool b00 = C2 + DX23 * y0 - DY23 * x0 > 0;
      const bool b10 = C2 + DX23 * y0 - DY23 * x1 > 0;
      const bool b01 = C2 + DX23 * y1 - DY23 * x0 > 0;
      const bool b11 = C2 + DX23 * y1 - DY23 * x1 > 0;
      const int  b   = (b00 << 0) | (b10 << 1) | (b01 << 2) | (b11 << 3);

      const bool c00 = C3 + DX31 * y0 - DY31 * x0 > 0;
      const bool c10 = C3 + DX31 * y0 - DY31 * x1 > 0;
      const bool c01 = C3 + DX31 * y1 - DY31 * x0 > 0;
      const bool c11 = C3 + DX31 * y1 - DY31 * x1 > 0;
      const int  c   = (c00 << 0) | (c10 << 1) | (c01 << 2) | (c11 << 3);

      // Skip block when outside an edge
      if (a == 0x0 || b == 0x0 || c == 0x0)
        continue;

      auto* buffer = frameBuf->TileColor(x,y);
      auto* depth  = frameBuf->TileDepth(x,y);
      
      // Accept whole block when totally covered
      if (a == 0xF && b == 0xF && c == 0xF)
      {
        int CY1 = C1 + DX12 * y0 - DY12 * x0;
        int CY2 = C2 + DX23 * y0 - DY23 * x0;
        int CY3 = C3 + DX31 * y0 - DY31 * x0;

        for (int iy = 0; iy < BLOCK_ITER; iy++)
        {
          ROP::Block(tri, CY1, CY3, FDY12, FDY31, FDX12, FDX31, areaInv,
                     buffer, depth);

          CY1 += FDX12*BMULT;
          CY2 += FDX23*BMULT;
          CY3 += FDX31*BMULT;

          buffer += blockSizeX*BMULT;
          depth  += blockSizeX*BMULT;
        }
      }
      else
      {
        int CY1 = C1 + DX12 * y0 - DY12 * x0;
        int CY2 = C2 + DX23 * y0 - DY23 * x0;
        int CY3 = C3 + DX31 * y0 - DY31 * x0;

        for (int iy = y; iy < y + blockSizeY; iy++)
        {
          int CX1 = CY1;
          int CX2 = CY2;
          int CX3 = CY3;

          for (int ix = 0; ix < blockSizeX; ix++)
          {
            if (CX1 > 0 && CX2 > 0 && CX3 > 0)
            {
              ROP::Pixel(tri, CX1, CX3, areaInv,
                         buffer + ix, depth + ix);
            }

            CX1 -= FDY12;
            CX2 -= FDY23;
            CX3 -= FDY31;
          }

          CY1 += FDX12;
          CY2 += FDX23;
          CY3 += FDX31;

          buffer += blockSizeY;
          depth  += blockSizeY;
        }
      }
    
    }
  }
  // \\ end // Loop through blocks

}


#endif //TEST_GL_TOP_SIMDBLOCKFIXP_H
