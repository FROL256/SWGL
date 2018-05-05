#include "TriRaster.h"
#include "TexSampler.h"

#include <cstdint>
#include <memory.h>
#include <vector>

struct ushort2
{
  inline ushort2() : x(0), y(0) {}
  inline ushort2(int a_x, int a_y) { x = (unsigned short)a_x; y = (unsigned short)a_y; }

  unsigned short x;
  unsigned short y;
};

#ifdef ENABLE_SSE 

const __m128 g_eps2E3 = _mm_set_ps(-10000000.0f, HALF_SPACE_EPSILON, HALF_SPACE_EPSILON, HALF_SPACE_EPSILON);
const __m128 g_one2   = _mm_set_ps(1.0f, 1.0f, 1.0f, 1.0f);
const __m128 g_white  = _mm_castsi128_ps(_mm_set1_epi32(0xFFFFFFFF));

inline bool CoverageTestSSE(const __m128& Cx)
{
  return ((_mm_movemask_ps(_mm_cmpgt_ps(Cx, g_eps2E3)) & 7) == 7);
}

void rasterizeTriHalfSpaceTiledSimpleWhiteSSE(FrameBuffer* frameBuf, const Triangle& tri)
{
  // Bounding rectangle
  const int minx = tri.bb_iminX;
  const int maxx = tri.bb_imaxX;
  const int miny = tri.bb_iminY;
  const int maxy = tri.bb_imaxY;

  int* colorBuffer = frameBuf->data + miny * frameBuf->w;

  //
  //
  const __m128 vy    = _mm_set_ps(0.0f, tri.v1.y, tri.v2.y, tri.v3.y);
  const __m128 vx    = _mm_set_ps(0.0f, tri.v1.x, tri.v2.x, tri.v3.x);
  const __m128 vMinX = _mm_set_ps(0.0f, (float)minx, (float)minx, (float)minx);
  const __m128 vMinY = _mm_set_ps(0.0f, (float)miny, (float)miny, (float)miny);

  const __m128 vDx   = _mm_sub_ps(vx, _mm_shuffle_ps(vx, vx, _MM_SHUFFLE(0, 0, 2, 1)));
  const __m128 vDy   = _mm_sub_ps(vy, _mm_shuffle_ps(vy, vy, _MM_SHUFFLE(0, 0, 2, 1)));
  const __m128 vC    = _mm_sub_ps(_mm_mul_ps(vDy, vx), _mm_mul_ps(vDx, vy));
  const __m128 vCy   = _mm_add_ps(vC, _mm_sub_ps(_mm_mul_ps(vDx, vMinY), _mm_mul_ps(vDy, vMinX)));
  const __m128 Dx    = _mm_sub_ps(vx, _mm_shuffle_ps(vx, vx, _MM_SHUFFLE(3, 0, 2, 1)));
  const __m128 Dy    = _mm_sub_ps(vy, _mm_shuffle_ps(vy, vy, _MM_SHUFFLE(3, 0, 2, 1))); // _mm_set_ps(0.0f, Dy31, Dy23, Dy12);

  const int TILE_SIZE = 4;
  const int w = frameBuf->w;

  const __m128 c3 = _mm_set_ps1(3.0f);
  const __m128 c4 = _mm_set_ps1(4.0f);

  __m128 Cy = vCy;

  // Scan through bounding rectangle
  for (int y = miny; y <= maxy; y += TILE_SIZE)
  {
    __m128 Cx = Cy;

    for (int x = minx; x <= maxx; x += TILE_SIZE)
    {
      const __m128 Cx_00 = Cx;
      const __m128 Cx_03 = _mm_add_ps(Cx, _mm_mul_ps(c3, Dy));
      const __m128 Cx_30 = _mm_add_ps(Cx, _mm_mul_ps(c3, Dx));
      const __m128 Cx_33 = _mm_add_ps(Cx, _mm_mul_ps(c3, _mm_add_ps(Dx,Dy)));

      const bool corner0 = CoverageTestSSE(Cx_00);
      const bool corner1 = CoverageTestSSE(Cx_03);
      const bool corner2 = CoverageTestSSE(Cx_30);
      const bool corner3 = CoverageTestSSE(Cx_33);

      if (corner0 && corner1 && corner2 && corner3) // this is not strictly correct
      {
        _mm_storeu_ps((float*)(colorBuffer) + x,       g_white);
        _mm_storeu_ps((float*)(colorBuffer) + x + w,   g_white);
        _mm_storeu_ps((float*)(colorBuffer) + x + w*2, g_white);
        _mm_storeu_ps((float*)(colorBuffer) + x + w*3, g_white);
      }

      Cx = _mm_sub_ps(Cx, _mm_mul_ps(Dy,c4));
    }

    Cy = _mm_add_ps(Cy, _mm_mul_ps(Dx,c4));

    colorBuffer += w*TILE_SIZE;
  }

}


inline __m128 barycentricScale(const __m128& Cx1, const __m128& Cx2, const __m128& Cx3)
{
  return _mm_rcp_ps(_mm_add_ps(Cx1, _mm_add_ps(Cx2, Cx3)));
}

const __m128 g_c255 = _mm_set_ps(255.0f, 255.0f, 255.0f, 255.0f);

inline int packRGBA(const __m128& color)
{
  const __m128i rgba = _mm_cvtps_epi32(_mm_mul_ps(g_c255, color));
  const __m128i out  = _mm_packus_epi32(rgba, _mm_setzero_si128());
  const __m128i out2 = _mm_packus_epi16(out, _mm_setzero_si128());

  return _mm_cvtsi128_si32(out2);
}

struct ALIGNED16 TriColorVEX
{
  __m128 color1_r;
  __m128 color1_g;
  __m128 color1_b;
  
  __m128 color2_r;
  __m128 color2_g;
  __m128 color2_b;
  
  __m128 color3_r;
  __m128 color3_g;
  __m128 color3_b;
};

inline __m128 evalLine4Px(const __m128& Cx1_0v, const __m128& Cx2_0v, const __m128& Cx3_0v, const TriColorVEX& t)
{
  const __m128 scale0 = barycentricScale(Cx1_0v, Cx2_0v, Cx3_0v);

  const __m128 w1_0v = _mm_mul_ps(scale0, Cx1_0v);
  const __m128 w2_0v = _mm_mul_ps(scale0, Cx2_0v);
  const __m128 w3_0v = _mm_mul_ps(scale0, Cx3_0v);

  const __m128 c0_r = _mm_add_ps(_mm_add_ps(_mm_mul_ps(w1_0v, t.color1_r), _mm_mul_ps(w2_0v, t.color2_r)), _mm_mul_ps(w3_0v, t.color3_r));
  const __m128 c0_g = _mm_add_ps(_mm_add_ps(_mm_mul_ps(w1_0v, t.color1_g), _mm_mul_ps(w2_0v, t.color2_g)), _mm_mul_ps(w3_0v, t.color3_g));
  const __m128 c0_b = _mm_add_ps(_mm_add_ps(_mm_mul_ps(w1_0v, t.color1_b), _mm_mul_ps(w2_0v, t.color2_b)), _mm_mul_ps(w3_0v, t.color3_b));

  const __m128 rg0     = _mm_shuffle_ps(c0_r, c0_g, _MM_SHUFFLE(0, 0, 0, 0));
  const __m128 rg1     = _mm_shuffle_ps(c0_r, c0_g, _MM_SHUFFLE(1, 1, 1, 1));
  const __m128 rg2     = _mm_shuffle_ps(c0_r, c0_g, _MM_SHUFFLE(2, 2, 2, 2));
  const __m128 rg3     = _mm_shuffle_ps(c0_r, c0_g, _MM_SHUFFLE(3, 3, 3, 3));

  const __m128 p0_rgba = _mm_shuffle_ps(rg0, c0_b, _MM_SHUFFLE(0, 0, 2, 0));
  const __m128 p1_rgba = _mm_shuffle_ps(rg1, c0_b, _MM_SHUFFLE(1, 1, 2, 0));
  const __m128 p2_rgba = _mm_shuffle_ps(rg2, c0_b, _MM_SHUFFLE(2, 2, 2, 0));
  const __m128 p3_rgba = _mm_shuffle_ps(rg3, c0_b, _MM_SHUFFLE(3, 3, 2, 0));

  const int p0 = packRGBA(p0_rgba);
  const int p1 = packRGBA(p1_rgba);
  const int p2 = packRGBA(p2_rgba);
  const int p3 = packRGBA(p3_rgba);

  return _mm_castsi128_ps(_mm_set_epi32(p3, p2, p1, p0));
}

void rasterizeTriHalfSpaceTiledSimpleSSE(FrameBuffer* frameBuf, const Triangle& tri)
{
  // Bounding rectangle
  const int minx = tri.bb_iminX;
  const int maxx = tri.bb_imaxX;
  const int miny = tri.bb_iminY;
  const int maxy = tri.bb_imaxY;

  int* colorBuffer = frameBuf->data + miny * frameBuf->w;

  //
  //
  const __m128 vy    = _mm_set_ps(0.0f, tri.v1.y, tri.v2.y, tri.v3.y);
  const __m128 vx    = _mm_set_ps(0.0f, tri.v1.x, tri.v2.x, tri.v3.x);
  const __m128 vMinX = _mm_set_ps(0.0f, (float)minx, (float)minx, (float)minx);
  const __m128 vMinY = _mm_set_ps(0.0f, (float)miny, (float)miny, (float)miny);

  const __m128 vDx   = _mm_sub_ps(vx, _mm_shuffle_ps(vx, vx, _MM_SHUFFLE(0, 0, 2, 1)));
  const __m128 vDy   = _mm_sub_ps(vy, _mm_shuffle_ps(vy, vy, _MM_SHUFFLE(0, 0, 2, 1)));
  const __m128 vC    = _mm_sub_ps(_mm_mul_ps(vDy, vx), _mm_mul_ps(vDx, vy));
  const __m128 vCy   = _mm_add_ps(vC, _mm_sub_ps(_mm_mul_ps(vDx, vMinY), _mm_mul_ps(vDy, vMinX)));
  const __m128 Dx    = _mm_sub_ps(vx, _mm_shuffle_ps(vx, vx, _MM_SHUFFLE(3, 0, 2, 1)));
  const __m128 Dy    = _mm_sub_ps(vy, _mm_shuffle_ps(vy, vy, _MM_SHUFFLE(3, 0, 2, 1))); // _mm_set_ps(0.0f, Dy31, Dy23, Dy12);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  const int TILE_SIZE = 4;
  const int w = frameBuf->w;

  const __m128 c2    = _mm_set_ps1(2.0f);
  const __m128 c3    = _mm_set_ps1(3.0f);
  const __m128 c4    = _mm_set_ps1(4.0f);
  const __m128 c0123 = _mm_set_ps(3.0f, 2.0f, 1.0f, 0.0f);

  TriColorVEX triColor;

  triColor.color1_r = _mm_shuffle_ps(tri.cv1, tri.cv1, _MM_SHUFFLE(0, 0, 0, 0));
  triColor.color1_g = _mm_shuffle_ps(tri.cv1, tri.cv1, _MM_SHUFFLE(1, 1, 1, 1));
  triColor.color1_b = _mm_shuffle_ps(tri.cv1, tri.cv1, _MM_SHUFFLE(2, 2, 2, 2));
  triColor.color2_r = _mm_shuffle_ps(tri.cv2, tri.cv2, _MM_SHUFFLE(0, 0, 0, 0));
  triColor.color2_g = _mm_shuffle_ps(tri.cv2, tri.cv2, _MM_SHUFFLE(1, 1, 1, 1));
  triColor.color2_b = _mm_shuffle_ps(tri.cv2, tri.cv2, _MM_SHUFFLE(2, 2, 2, 2));
  triColor.color3_r = _mm_shuffle_ps(tri.cv3, tri.cv3, _MM_SHUFFLE(0, 0, 0, 0));
  triColor.color3_g = _mm_shuffle_ps(tri.cv3, tri.cv3, _MM_SHUFFLE(1, 1, 1, 1));
  triColor.color3_b = _mm_shuffle_ps(tri.cv3, tri.cv3, _MM_SHUFFLE(2, 2, 2, 2));

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  __m128 Cy = vCy;

  // Scan through bounding rectangle
  for (int y = miny; y <= maxy; y += TILE_SIZE)
  {
    __m128 Cx = Cy;

    for (int x = minx; x <= maxx; x += TILE_SIZE)
    {
      const __m128 Cx_00 = Cx;
      const __m128 Cx_03 = _mm_add_ps(Cx, _mm_mul_ps(c3, Dy));
      const __m128 Cx_30 = _mm_add_ps(Cx, _mm_mul_ps(c3, Dx));
      const __m128 Cx_33 = _mm_add_ps(Cx, _mm_mul_ps(c3, _mm_add_ps(Dx,Dy)));

      const bool corner0 = CoverageTestSSE(Cx_00);
      const bool corner1 = CoverageTestSSE(Cx_03);
      const bool corner2 = CoverageTestSSE(Cx_30);
      const bool corner3 = CoverageTestSSE(Cx_33);

      if (corner0 && corner1 && corner2 && corner3) // this is not strictly correct
      {
        const __m128 Cx1_00  = _mm_shuffle_ps(Cx, Cx, _MM_SHUFFLE(0, 0, 0, 0));
        const __m128 Cx2_00  = _mm_shuffle_ps(Cx, Cx, _MM_SHUFFLE(1, 1, 1, 1));
        const __m128 Cx3_00  = _mm_shuffle_ps(Cx, Cx, _MM_SHUFFLE(2, 2, 2, 2));
        
        const __m128 Dy_xxxx = _mm_shuffle_ps(Dy, Dy, _MM_SHUFFLE(0, 0, 0, 0));
        const __m128 Dy_yyyy = _mm_shuffle_ps(Dy, Dy, _MM_SHUFFLE(1, 1, 1, 1));
        const __m128 Dy_zzzz = _mm_shuffle_ps(Dy, Dy, _MM_SHUFFLE(2, 2, 2, 2));

        const __m128 Dx_xxxx = _mm_shuffle_ps(Dx, Dx, _MM_SHUFFLE(0, 0, 0, 0));
        const __m128 Dx_yyyy = _mm_shuffle_ps(Dx, Dx, _MM_SHUFFLE(1, 1, 1, 1));
        const __m128 Dx_zzzz = _mm_shuffle_ps(Dx, Dx, _MM_SHUFFLE(2, 2, 2, 2));

        // line zero
        //
        {
          const __m128 Cx1_0v = _mm_sub_ps(Cx1_00, _mm_mul_ps(c0123, Dy_xxxx));
          const __m128 Cx2_0v = _mm_sub_ps(Cx2_00, _mm_mul_ps(c0123, Dy_yyyy));
          const __m128 Cx3_0v = _mm_sub_ps(Cx3_00, _mm_mul_ps(c0123, Dy_zzzz));

          const __m128 line0 = evalLine4Px(Cx1_0v, Cx2_0v, Cx3_0v, triColor);

          _mm_storeu_ps((float*)(colorBuffer)+x, line0);
        }

        // line one
        //
        {
          const __m128 Cx1_1v = _mm_add_ps(Dx_xxxx, _mm_sub_ps(Cx1_00, _mm_mul_ps(c0123, Dy_xxxx)));
          const __m128 Cx2_1v = _mm_add_ps(Dx_yyyy, _mm_sub_ps(Cx2_00, _mm_mul_ps(c0123, Dy_yyyy)));
          const __m128 Cx3_1v = _mm_add_ps(Dx_zzzz, _mm_sub_ps(Cx3_00, _mm_mul_ps(c0123, Dy_zzzz)));

          const __m128 line1 = evalLine4Px(Cx1_1v, Cx2_1v, Cx3_1v, triColor);

          _mm_storeu_ps((float*)(colorBuffer)+x + w, line1);
        }

        // line two
        //
        {
          const __m128 Cx1_2v = _mm_add_ps(_mm_mul_ps(c2,Dx_xxxx), _mm_sub_ps(Cx1_00, _mm_mul_ps(c0123, Dy_xxxx)));
          const __m128 Cx2_2v = _mm_add_ps(_mm_mul_ps(c2,Dx_yyyy), _mm_sub_ps(Cx2_00, _mm_mul_ps(c0123, Dy_yyyy)));
          const __m128 Cx3_2v = _mm_add_ps(_mm_mul_ps(c2,Dx_zzzz), _mm_sub_ps(Cx3_00, _mm_mul_ps(c0123, Dy_zzzz)));

          const __m128 line2 = evalLine4Px(Cx1_2v, Cx2_2v, Cx3_2v, triColor);

          _mm_storeu_ps((float*)(colorBuffer)+x + w * 2, line2);
        }

        // line three
        //
        {
          const __m128 Cx1_3v = _mm_add_ps(_mm_mul_ps(c3, Dx_xxxx), _mm_sub_ps(Cx1_00, _mm_mul_ps(c0123, Dy_xxxx)));
          const __m128 Cx2_3v = _mm_add_ps(_mm_mul_ps(c3, Dx_yyyy), _mm_sub_ps(Cx2_00, _mm_mul_ps(c0123, Dy_yyyy)));
          const __m128 Cx3_3v = _mm_add_ps(_mm_mul_ps(c3, Dx_zzzz), _mm_sub_ps(Cx3_00, _mm_mul_ps(c0123, Dy_zzzz)));

          const __m128 line3 = evalLine4Px(Cx1_3v, Cx2_3v, Cx3_3v, triColor);

          _mm_storeu_ps((float*)(colorBuffer)+x + w * 3, line3);
        }

      }

      Cx = _mm_sub_ps(Cx, _mm_mul_ps(Dy,c4));
    }

    Cy = _mm_add_ps(Cy, _mm_mul_ps(Dx,c4));

    colorBuffer += w*TILE_SIZE;
  }

}




#else


inline bool CoverageTest(const float Cx1, const float Cx2, const float Cx3)
{
  return (Cx1 > HALF_SPACE_EPSILON && Cx2 > HALF_SPACE_EPSILON && Cx3 > HALF_SPACE_EPSILON);
}

void rasterizeTriHalfSpaceTiledSimple(FrameBuffer* frameBuf, const Triangle& tri)
{
  const float y1 = tri.v3.y;
  const float y2 = tri.v2.y;
  const float y3 = tri.v1.y;

  const float x1 = tri.v3.x;
  const float x2 = tri.v2.x;
  const float x3 = tri.v1.x;

  // Deltas
  const float Dx12 = x1 - x2;
  const float Dx23 = x2 - x3;
  const float Dx31 = x3 - x1;

  const float Dy12 = y1 - y2;
  const float Dy23 = y2 - y3;
  const float Dy31 = y3 - y1;

  // Bounding rectangle
  const int minx = tri.bb_iminX;
  const int maxx = tri.bb_imaxX;
  const int miny = tri.bb_iminY;
  const int maxy = tri.bb_imaxY;

  int* colorBuffer = frameBuf->data + miny * frameBuf->w;

  // Constant part of half-edge functions
  const float C1 = Dy12 * x1 - Dx12 * y1;
  const float C2 = Dy23 * x2 - Dx23 * y2;
  const float C3 = Dy31 * x3 - Dx31 * y3;

  float Cy1 = C1 + Dx12 * miny - Dy12 * minx;
  float Cy2 = C2 + Dx23 * miny - Dy23 * minx;
  float Cy3 = C3 + Dx31 * miny - Dy31 * minx;

  const int   TILE_SIZE  = 4;
  const float TILE_SIZEF = 4.0f;
  const int w = frameBuf->w;

  // Scan through bounding rectangle
  for (int y = miny; y <= maxy; y += TILE_SIZE)
  {
    // Start value for horizontal scan
    float Cx1 = Cy1;
    float Cx2 = Cy2;
    float Cx3 = Cy3;

    for (int x = minx; x <= maxx; x += TILE_SIZE)
    {
      const float Cx1_00 = Cx1;
      const float Cx2_00 = Cx2;
      const float Cx3_00 = Cx3;

      const float Cx1_03 = Cx1 + 3.0f*Dy12;
      const float Cx2_03 = Cx2 + 3.0f*Dy23;
      const float Cx3_03 = Cx3 + 3.0f*Dy31;
                              
      const float Cx1_30 = Cx1 + 3.0f*Dx12;
      const float Cx2_30 = Cx2 + 3.0f*Dx23;
      const float Cx3_30 = Cx3 + 3.0f*Dx31;
                            
      const float Cx1_33 = Cx1 + 3.0f*(Dy12 + Dx12);
      const float Cx2_33 = Cx2 + 3.0f*(Dy23 + Dx23);
      const float Cx3_33 = Cx3 + 3.0f*(Dy31 + Dx31);

      const bool corner0 = CoverageTest(Cx1_00, Cx2_00, Cx3_00);
      const bool corner1 = CoverageTest(Cx1_03, Cx2_03, Cx3_03);
      const bool corner2 = CoverageTest(Cx1_30, Cx2_30, Cx3_30);
      const bool corner3 = CoverageTest(Cx1_33, Cx2_33, Cx3_33);

      if (corner0 && corner1 && corner2 && corner3) // this is not strictly correct
      {
        colorBuffer[x+0]         = 0xFFFFFFFF;
        colorBuffer[x+1]         = 0xFFFFFFFF;
        colorBuffer[x+2]         = 0xFFFFFFFF;
        colorBuffer[x+3]         = 0xFFFFFFFF;

        colorBuffer[x + w*1 + 0] = 0xFFFFFFFF;
        colorBuffer[x + w*1 + 1] = 0xFFFFFFFF;
        colorBuffer[x + w*1 + 2] = 0xFFFFFFFF;
        colorBuffer[x + w*1 + 3] = 0xFFFFFFFF;

        colorBuffer[x + w*2 + 0] = 0xFFFFFFFF;
        colorBuffer[x + w*2 + 1] = 0xFFFFFFFF;
        colorBuffer[x + w*2 + 2] = 0xFFFFFFFF;
        colorBuffer[x + w*2 + 3] = 0xFFFFFFFF;

        colorBuffer[x + w*3 + 0] = 0xFFFFFFFF;
        colorBuffer[x + w*3 + 1] = 0xFFFFFFFF;
        colorBuffer[x + w*3 + 2] = 0xFFFFFFFF;
        colorBuffer[x + w*3 + 3] = 0xFFFFFFFF;
      }
      else if (corner0 || corner1 || corner2 || corner3) // comment this to get maximum perf, anyway this is not strictly correct
      {
        // int* colorBuffer2 = colorBuffer;
        // 
        // float Cy1_i = Cx1;
        // float Cy2_i = Cx2;
        // float Cy3_i = Cx3;
        // 
        // for (int y2 = 0; y2 < 4; y2++)
        // {
        //   float Cx1_i = Cy1_i;
        //   float Cx2_i = Cy2_i;
        //   float Cx3_i = Cy3_i;
        // 
        //   for (int x2 = 0; x2 < 4; x2++)
        //   {
        //     if (CoverageTest(Cx1_i, Cx2_i, Cx3_i))
        //       colorBuffer2[x+x2] = 0xFFFFFFFF;
        // 
        //     Cx1_i -= Dy12;
        //     Cx2_i -= Dy23;
        //     Cx3_i -= Dy31;
        //   }
        // 
        //   Cy1_i += Dx12;
        //   Cy2_i += Dx23;
        //   Cy3_i += Dx31;
        // 
        //   colorBuffer2 += w;
        // }
      }

      Cx1 -= Dy12*TILE_SIZEF;
      Cx2 -= Dy23*TILE_SIZEF;
      Cx3 -= Dy31*TILE_SIZEF;
    }

    Cy1 += Dx12*TILE_SIZEF;
    Cy2 += Dx23*TILE_SIZEF;
    Cy3 += Dx31*TILE_SIZEF;

    colorBuffer += w*TILE_SIZE;
  }

}

#endif


inline int imax(int a, int b) { return (a > b) ? a : b; }
inline int imin(int a, int b) { return (a < b) ? a : b; }
inline int iround(float f) { return (int)f; }

inline int imax(int a, int b, int c)
{
  return imax(a, imax(b, c));
}

inline int imin(int a, int b, int c)
{
  return imin(a, imin(b, c));
}

void rasterizeTriHalfSpaceBlockOrientedFixp(FrameBuffer* frameBuf, const Triangle& tri)
{

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
  int minx = (imin(X1, X2, X3) + 0xF) >> 4;
  int maxx = (imax(X1, X2, X3) + 0xF) >> 4;
  int miny = (imin(Y1, Y2, Y3) + 0xF) >> 4;
  int maxy = (imax(Y1, Y2, Y3) + 0xF) >> 4;

  // Block size, standard 8x8 (must be power of two)
  const int q = 8;

  // Start in corner of 8x8 block
  minx &= ~(q - 1);
  miny &= ~(q - 1);

  const int w = frameBuf->w;

  int* colorBuffer = frameBuf->data + miny * w;

  // Half-edge constants
  int C1 = DY12 * X1 - DX12 * Y1;
  int C2 = DY23 * X2 - DX23 * Y2;
  int C3 = DY31 * X3 - DX31 * Y3;

  // Correct for fill convention
  if (DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
  if (DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
  if (DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;

  // Loop through blocks
  for (int y = miny; y < maxy; y += q)
  {
    for (int x = minx; x < maxx; x += q)
    {
      // Corners of block
      int x0 = x << 4;
      int x1 = (x + q - 1) << 4;
      int y0 = y << 4;
      int y1 = (y + q - 1) << 4;

      // Evaluate half-space functions
      bool a00 = C1 + DX12 * y0 - DY12 * x0 > 0;
      bool a10 = C1 + DX12 * y0 - DY12 * x1 > 0;
      bool a01 = C1 + DX12 * y1 - DY12 * x0 > 0;
      bool a11 = C1 + DX12 * y1 - DY12 * x1 > 0;
      int a = (a00 << 0) | (a10 << 1) | (a01 << 2) | (a11 << 3);

      bool b00 = C2 + DX23 * y0 - DY23 * x0 > 0;
      bool b10 = C2 + DX23 * y0 - DY23 * x1 > 0;
      bool b01 = C2 + DX23 * y1 - DY23 * x0 > 0;
      bool b11 = C2 + DX23 * y1 - DY23 * x1 > 0;
      int b = (b00 << 0) | (b10 << 1) | (b01 << 2) | (b11 << 3);

      bool c00 = C3 + DX31 * y0 - DY31 * x0 > 0;
      bool c10 = C3 + DX31 * y0 - DY31 * x1 > 0;
      bool c01 = C3 + DX31 * y1 - DY31 * x0 > 0;
      bool c11 = C3 + DX31 * y1 - DY31 * x1 > 0;
      int c = (c00 << 0) | (c10 << 1) | (c01 << 2) | (c11 << 3);

      // Skip block when outside an edge
      if (a == 0x0 || b == 0x0 || c == 0x0) continue;

      int *buffer = colorBuffer;

      // Accept whole block when totally covered
      if (a == 0xF && b == 0xF && c == 0xF)
      {
        for (int iy = 0; iy < q; iy++)
        {
          for (int ix = x; ix < x + q; ix++)
          {
            buffer[ix] = 0x0000FF00; // Green
          }

          buffer += w;
        }
      }
      else // Partially covered block
      {
        int CY1 = C1 + DX12 * y0 - DY12 * x0;
        int CY2 = C2 + DX23 * y0 - DY23 * x0;
        int CY3 = C3 + DX31 * y0 - DY31 * x0;

        for (int iy = y; iy < y + q; iy++)
        {
          int CX1 = CY1;
          int CX2 = CY2;
          int CX3 = CY3;

          for (int ix = x; ix < x + q; ix++)
          {
            if (CX1 > 0 && CX2 > 0 && CX3 > 0)
            {
              buffer[ix] = 0x000000FF; // Blue
            }

            CX1 -= FDY12;
            CX2 -= FDY23;
            CX3 -= FDY31;
          }

          CY1 += FDX12;
          CY2 += FDX23;
          CY3 += FDX31;

          buffer += w;
        }
      }
    }

    colorBuffer += q * w;
  }


}




void rasterizeTriHalfSpaceTwoLevel(const FillFuncPtr pf, FrameBuffer* frameBuf, const Triangle& tri)
{

#ifdef ENABLE_SSE 

  #ifdef FILL_COLOR_ONLY
  rasterizeTriHalfSpaceTiledSimpleWhiteSSE(frameBuf, tri);
  #else
  rasterizeTriHalfSpaceTiledSimpleSSE(frameBuf, tri);
  #endif
#else

  rasterizeTriHalfSpaceBlockOrientedFixp(frameBuf, tri);
  //rasterizeTriHalfSpaceTiledSimple(frameBuf, tri);

#endif

}

