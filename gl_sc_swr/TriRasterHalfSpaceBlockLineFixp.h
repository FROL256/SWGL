//
// Created by frol on 28.10.18.
//

#ifndef TEST_GL_TOP_SIMDBLOCKFIXP_H
#define TEST_GL_TOP_SIMDBLOCKFIXP_H

static inline int imax(int a, int b) { return (a > b) ? a : b; }
static inline int imin(int a, int b) { return (a < b) ? a : b; }
static inline int iround(float f)    { return (int)f; }

template<int value> inline int div    (int a_arg) { return a_arg/value; }
template<>          inline int div<2> (int a_arg) { return a_arg >> 1; }
template<>          inline int div<4> (int a_arg) { return a_arg >> 2; }
template<>          inline int div<8> (int a_arg) { return a_arg >> 3; }
template<>          inline int div<16>(int a_arg) { return a_arg >> 4; }

template<typename ROP>
void RasterizeTriHalfSpaceBlockFixp2D_Fill(const typename ROP::Triangle& tri, int tileMinX, int tileMinY,
                                           FrameBuffer* frameBuf)
{
  constexpr int blockSize = ROP::n;
  typedef typename ROP::Triangle Triangle;

  constexpr int   SUBPIXELBITS  = 4;
  constexpr int   SUBPIXELMULTI = (1 << SUBPIXELBITS);
  constexpr float SUBPIXELMULTF = float(SUBPIXELMULTI);

  // 28.4 fixed-point coordinates
  const int Y1 = iround(SUBPIXELMULTF * tri.v3.y) - 16*SUBPIXELMULTI;
  const int Y2 = iround(SUBPIXELMULTF * tri.v2.y) - 16*SUBPIXELMULTI;
  const int Y3 = iround(SUBPIXELMULTF * tri.v1.y) - 16*SUBPIXELMULTI;

  const int X1 = iround(SUBPIXELMULTF * tri.v3.x) - 16*SUBPIXELMULTI;
  const int X2 = iround(SUBPIXELMULTF * tri.v2.x) - 16*SUBPIXELMULTI;
  const int X3 = iround(SUBPIXELMULTF * tri.v1.x) - 16*SUBPIXELMULTI;

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
  const int minx = ( imax(tri.bb_iminX - tileMinX, 0)               ) & ~(blockSize - 1);  // Start in corner of 8x8 block
  const int miny = ( imax(tri.bb_iminY - tileMinY, 0)               ) & ~(blockSize - 1);  // Start in corner of 8x8 block
  const int maxx = ( imin(tri.bb_imaxX - tileMinX, frameBuf->w - 1) );
  const int maxy = ( imin(tri.bb_imaxY - tileMinY, frameBuf->h - 1) );

  const int pitch  = frameBuf->pitch;
  int* colorBuffer = frameBuf->cbuffer + miny * pitch;

  // Half-edge constants
  int C1 = DY12 * X1 - DX12 * Y1;
  int C2 = DY23 * X2 - DX23 * Y2;
  int C3 = DY31 * X3 - DX31 * Y3;

  const auto triColor  = ROP::Line(tri);

  int dataTemp[blockSize];
  store_u(dataTemp, triColor);

  const int triColorS = dataTemp[0];

  // Correct for fill convention
  if (DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
  if (DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
  if (DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;
  
  // Loop through blocks
  for (int y = miny; y < maxy; y += blockSize)
  {
    for (int x = minx; x < maxx; x += blockSize)
    {
      // Corners of block
      int x0 = x << SUBPIXELBITS;
      int y0 = y << SUBPIXELBITS;
      int x1 = (x + blockSize - 1) << SUBPIXELBITS;
      int y1 = (y + blockSize - 1) << SUBPIXELBITS;

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

      int *buffer = colorBuffer;
  
      const int MTX = div<blockSize>(x);
      const int MTY = div<blockSize>(y);
      const int MTO = MTY*div<blockSize>(frameBuf->w) + MTX;
  
      while (frameBuf->lockbuffer[MTO].test_and_set(std::memory_order_acquire)); // lock micro tile
      
      // Accept whole block when totally covered
      if (a == 0xF && b == 0xF && c == 0xF)
      {

        for (int iy = 0; iy < blockSize; iy++)
        {
          ROP::store_line(buffer + x, triColor);
          buffer += pitch;
        }
      }
      else // Partially covered block
      {
        int CY1 = C1 + DX12 * y0 - DY12 * x0;
        int CY2 = C2 + DX23 * y0 - DY23 * x0;
        int CY3 = C3 + DX31 * y0 - DY31 * x0;

        for (int iy = y; iy < y + blockSize; iy++)
        {
          int CX1 = CY1;
          int CX2 = CY2;
          int CX3 = CY3;

          for (int ix = x; ix < x + blockSize; ix++)
          {
            if (CX1 > 0 && CX2 > 0 && CX3 > 0)
              buffer[ix] = triColorS;

            CX1 -= FDY12;
            CX2 -= FDY23;
            CX3 -= FDY31;
          }

          CY1 += FDX12;
          CY2 += FDX23;
          CY3 += FDX31;

          buffer += pitch;
        }
      }
  
      frameBuf->lockbuffer[MTO].clear(std::memory_order_release);
    }

    colorBuffer += blockSize * pitch;
  }


}


template<typename ROP>
void RasterizeTriHalfSpaceBlockLineFixp2D(const typename ROP::Triangle &tri, int tileMinX, int tileMinY,
                                          FrameBuffer *frameBuf)
{
  constexpr int blockSize = ROP::n;
  typedef typename ROP::Triangle Triangle;

  constexpr int   SUBPIXELBITS  = 4;
  constexpr int   SUBPIXELMULTI = (1 << SUBPIXELBITS);
  constexpr float SUBPIXELMULTF = float(SUBPIXELMULTI);

  // 28.4 fixed-point coordinates
  const int Y1 = iround(SUBPIXELMULTF * tri.v3.y) - SUBPIXELMULTI*tileMinY;
  const int Y2 = iround(SUBPIXELMULTF * tri.v2.y) - SUBPIXELMULTI*tileMinY;
  const int Y3 = iround(SUBPIXELMULTF * tri.v1.y) - SUBPIXELMULTI*tileMinY;

  const int X1 = iround(SUBPIXELMULTF * tri.v3.x) - SUBPIXELMULTI*tileMinX;
  const int X2 = iround(SUBPIXELMULTF * tri.v2.x) - SUBPIXELMULTI*tileMinX;
  const int X3 = iround(SUBPIXELMULTF * tri.v1.x) - SUBPIXELMULTI*tileMinX;

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
  const int minx = ( imax(tri.bb_iminX - tileMinX, 0)               ) & ~(blockSize - 1);  // Start in corner of 8x8 block
  const int miny = ( imax(tri.bb_iminY - tileMinY, 0)               ) & ~(blockSize - 1);  // Start in corner of 8x8 block
  const int maxx = ( imin(tri.bb_imaxX - tileMinX, frameBuf->w - 1) );
  const int maxy = ( imin(tri.bb_imaxY - tileMinY, frameBuf->h - 1) );

  const int pitch  = frameBuf->pitch;

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
  for (int y = miny; y < maxy; y += blockSize)
  {
    for (int x = minx; x < maxx; x += blockSize)
    {
      // Corners of block
      const int x0 = x << SUBPIXELBITS;
      const int y0 = y << SUBPIXELBITS;
      const int x1 = (x + blockSize - 1) << SUBPIXELBITS;
      const int y1 = (y + blockSize - 1) << SUBPIXELBITS;

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

      int* buffer = frameBuf->cbuffer + y * pitch;

      const int MTX = div<blockSize>(x);
      const int MTY = div<blockSize>(y);
      const int MTO = MTY*div<blockSize>(frameBuf->w) + MTX;
      
      while (frameBuf->lockbuffer[MTO].test_and_set(std::memory_order_acquire)); // lock micro tile
      
      // Accept whole block when totally covered
      if (a == 0xF && b == 0xF && c == 0xF)
      {
        int CY1 = C1 + DX12 * y0 - DY12 * x0;
        int CY2 = C2 + DX23 * y0 - DY23 * x0;
        int CY3 = C3 + DX31 * y0 - DY31 * x0;

        for (int iy = 0; iy < blockSize; iy++)
        {
          ROP::Line(tri, CY1, CY3, FDY12, FDY31, areaInv,
                    buffer + x);

          CY1 += FDX12;
          CY2 += FDX23;
          CY3 += FDX31;

          buffer += pitch;
        }
      }
      else
      {
        int CY1 = C1 + DX12 * y0 - DY12 * x0;
        int CY2 = C2 + DX23 * y0 - DY23 * x0;
        int CY3 = C3 + DX31 * y0 - DY31 * x0;

        for (int iy = y; iy < y + blockSize; iy++)
        {
          int CX1 = CY1;
          int CX2 = CY2;
          int CX3 = CY3;

          for (int ix = x; ix < x + blockSize; ix++)
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

          buffer += pitch;
        }
      }
  
      frameBuf->lockbuffer[MTO].clear(std::memory_order_release); // unlock micro tile
    }

  }

}

template<typename T>
int EvalSubpixelBits(const T& tri) // #TODO: implement this carefully
{
  if (tri.triSize < 32768.0f)
    return 3;
  else if (tri.triSize < 65536.0f)
    return 2;
  else
    return 0;
}

template<typename ROP>
void RasterizeTriHalfSpaceBlockLineFixp3D(const typename ROP::Triangle &tri, int tileMinX, int tileMinY,
                                          FrameBuffer *frameBuf)
{
  constexpr int blockSize = ROP::n;
  typedef typename ROP::Triangle Triangle;

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
  const int minx = ( imax(tri.bb_iminX - tileMinX, 0)               ) & ~(blockSize - 1);  // Start in corner of 8x8 block
  const int miny = ( imax(tri.bb_iminY - tileMinY, 0)               ) & ~(blockSize - 1);  // Start in corner of 8x8 block
  const int maxx = ( imin(tri.bb_imaxX - tileMinX, frameBuf->w - 1) );
  const int maxy = ( imin(tri.bb_imaxY - tileMinY, frameBuf->h - 1) );

  // Half-edge constants
  int C1 = DY12 * X1 - DX12 * Y1;
  int C2 = DY23 * X2 - DX23 * Y2;
  int C3 = DY31 * X3 - DX31 * Y3;

  // Correct for fill convention
  if (DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
  if (DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
  if (DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;

  const float areaInv = 1.0f / fabs(float(DY31*DX12 - DX31*DY12));
  const int pitch     = frameBuf->pitch;

  // Loop through blocks
  #ifdef WIN32
  #pragma omp parallel for num_threads(2)
  #else
  #pragma omp parallel for collapse(2) num_threads(2)
  #endif
  for (int y = miny; y < maxy; y += blockSize)
  {
    for (int x = minx; x < maxx; x += blockSize)
    {
      // Corners of block
      const int x0 = x << SUBPIXELBITS;
      const int y0 = y << SUBPIXELBITS;
      const int x1 = (x + blockSize - 1) << SUBPIXELBITS;
      const int y1 = (y + blockSize - 1) << SUBPIXELBITS;

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

      int*   bufferc = frameBuf->cbuffer + y * pitch;
      float* bufferz = frameBuf->zbuffer + y * pitch;
  
      const int MTX = div<blockSize>(x);
      const int MTY = div<blockSize>(y);
      const int MTO = MTY*div<blockSize>(frameBuf->w) + MTX;
  
      while (frameBuf->lockbuffer[MTO].test_and_set(std::memory_order_acquire)); // lock micro tile

      // Accept whole block when totally covered
      if (a == 0xF && b == 0xF && c == 0xF)
      {
        int CY1 = C1 + DX12 * y0 - DY12 * x0;
        int CY2 = C2 + DX23 * y0 - DY23 * x0;
        int CY3 = C3 + DX31 * y0 - DY31 * x0;

        for (int iy = 0; iy < blockSize; iy++)
        {
          ROP::Line(tri, CY1, CY3, FDY12, FDY31, areaInv,
                    bufferc + x, bufferz + x);

          CY1 += FDX12;
          CY2 += FDX23;
          CY3 += FDX31;

          bufferc += pitch;
          bufferz += pitch;
        }
      }
      else
      {
        int CY1 = C1 + DX12 * y0 - DY12 * x0;
        int CY2 = C2 + DX23 * y0 - DY23 * x0;
        int CY3 = C3 + DX31 * y0 - DY31 * x0;

        for (int iy = y; iy < y + blockSize; iy++)
        {
          int CX1 = CY1;
          int CX2 = CY2;
          int CX3 = CY3;

          for (int ix = x; ix < x + blockSize; ix++)
          {
            if (CX1 > 0 && CX2 > 0 && CX3 > 0)
            {
              ROP::Pixel(tri, CX1, CX3, areaInv,
                         bufferc + ix, bufferz + ix);
            }

            CX1 -= FDY12;
            CX2 -= FDY23;
            CX3 -= FDY31;
          }

          CY1 += FDX12;
          CY2 += FDX23;
          CY3 += FDX31;

          bufferc += pitch;
          bufferz += pitch;
        }
      }
  
      frameBuf->lockbuffer[MTO].clear(std::memory_order_release); // unlock micro tile

    } // for x

  } // for y

}

// pure fix point implementation further

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template<typename ROP>
void RasterizeTriHalfSpaceBlockLineFixp2D_FixpPixel(const typename ROP::Triangle &tri, int tileMinX, int tileMinY,
                                                    FrameBuffer *frameBuf)
{
  constexpr int blockSize = ROP::n;
  typedef typename ROP::Triangle Triangle;

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
  const int minx = ( imax(tri.bb_iminX - tileMinX, 0)               ) & ~(blockSize - 1);  // Start in corner of 8x8 block
  const int miny = ( imax(tri.bb_iminY - tileMinY, 0)               ) & ~(blockSize - 1);  // Start in corner of 8x8 block
  const int maxx = ( imin(tri.bb_imaxX - tileMinX, frameBuf->w - 1) );
  const int maxy = ( imin(tri.bb_imaxY - tileMinY, frameBuf->h - 1) );

  const int pitch  = frameBuf->pitch;

  // Half-edge constants
  int C1 = DY12 * X1 - DX12 * Y1;
  int C2 = DY23 * X2 - DX23 * Y2;
  int C3 = DY31 * X3 - DX31 * Y3;

  // Correct for fill convention
  if (DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
  if (DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
  if (DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;

  const unsigned int areaInv = (unsigned int)(0xFFFFFFFF) / (unsigned int)(abs(DY31*DX12 - DX31*DY12));
  
  // Loop through blocks
  for (int y = miny; y < maxy; y += blockSize)
  {
    for (int x = minx; x < maxx; x += blockSize)
    {
      // Corners of block
      const int x0 = x << 4;
      const int y0 = y << 4;
      const int x1 = (x + blockSize - 1) << 4;
      const int y1 = (y + blockSize - 1) << 4;

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

      int* buffer = frameBuf->cbuffer + y * frameBuf->pitch;
  
      const int MTX = div<blockSize>(x);
      const int MTY = div<blockSize>(y);
      const int MTO = MTY*div<blockSize>(frameBuf->w) + MTX;
  
      while (frameBuf->lockbuffer[MTO].test_and_set(std::memory_order_acquire)); // lock micro tile

      // Accept whole block when totally covered
      if (a == 0xF && b == 0xF && c == 0xF)
      {
        int CY1 = C1 + DX12 * y0 - DY12 * x0;
        int CY2 = C2 + DX23 * y0 - DY23 * x0;
        int CY3 = C3 + DX31 * y0 - DY31 * x0;

        for (int iy = 0; iy < blockSize; iy++)
        {
          ROP::Line(tri, CY1, CY3, FDY12, FDY31, areaInv,
                    buffer + x);

          CY1 += FDX12;
          CY2 += FDX23;
          CY3 += FDX31;

          buffer += pitch;
        }
      }
      else
      {
        int CY1 = C1 + DX12 * y0 - DY12 * x0;
        int CY2 = C2 + DX23 * y0 - DY23 * x0;
        int CY3 = C3 + DX31 * y0 - DY31 * x0;

        for (int iy = y; iy < y + blockSize; iy++)
        {
          int CX1 = CY1;
          int CX2 = CY2;
          int CX3 = CY3;

          for (int ix = x; ix < x + blockSize; ix++)
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

          buffer += pitch;
        }
      }
  
      frameBuf->lockbuffer[MTO].clear(std::memory_order_release); // unlock micro tile
    }

  }

}

#endif //TEST_GL_TOP_SIMDBLOCKFIXP_H
