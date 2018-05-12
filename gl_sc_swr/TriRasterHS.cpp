#include "TriRaster.h"
#include "TexSampler.h"

#include <cstdint>
#include <memory.h>

#ifdef ENABLE_SSE

inline __m128 barycentricCoordFromHalfSpaceDist(const __m128& Cx123)
{
  const __m128 temp     = _mm_hadd_ps(Cx123, Cx123);
  const __m128 scaleInv = _mm_hadd_ps(temp, temp);
  //const __m128 scale    = _mm_rcp_ps(scaleInv);
  const __m128 scaleX   = _mm_rcp_ss(scaleInv);
  const __m128 scale    = _mm_shuffle_ps(scaleX, scaleX, _MM_SHUFFLE(0, 0, 0, 0));

  return _mm_mul_ps(scale, Cx123);
}

#ifdef FILL_COLOR_ONLY

inline int HSPixelColor(const Triangle& tri, const __m128& Cx123, const __m128& zInv)
{
  return 0xFFFFFFFF;
}


inline int HSPixelColorTex(const Triangle& tri, const __m128& Cx123, const __m128& zInv)
{
  return 0xFFFFFFFF;
}


#else

inline int HSPixelColor(const Triangle& tri, const __m128& w, const __m128& zInv)
{
  const __m128 cc1 = _mm_mul_ps(tri.cv1, _mm_shuffle_ps(w, w, _MM_SHUFFLE(0, 0, 0, 0)));
  const __m128 cc2 = _mm_mul_ps(tri.cv3, _mm_shuffle_ps(w, w, _MM_SHUFFLE(1, 1, 1, 1)));
  const __m128 cc3 = _mm_mul_ps(tri.cv2, _mm_shuffle_ps(w, w, _MM_SHUFFLE(2, 2, 2, 2)));

  __m128 clr = _mm_add_ps(cc1, _mm_add_ps(cc2, cc3));

#ifdef PERSP_CORRECT
  const __m128 z1 = _mm_rcp_ss(zInv);
  const __m128 z  = _mm_shuffle_ps(z1, z1, _MM_SHUFFLE(0, 0, 0, 0));
  clr = _mm_mul_ps(clr, z);
#endif

  return RealColorToUint32_BGRA(clr);
}


inline int HSPixelColorTex(const Triangle& tri, const __m128& w, const __m128& zInv)
{
  const __m128 w0  = _mm_shuffle_ps(w, w, _MM_SHUFFLE(0, 0, 0, 0));
  const __m128 w1  = _mm_shuffle_ps(w, w, _MM_SHUFFLE(1, 1, 1, 1));
  const __m128 w2  = _mm_shuffle_ps(w, w, _MM_SHUFFLE(2, 2, 2, 2));

  const __m128 cc1 = _mm_mul_ps(tri.cv1, w0);
  const __m128 cc2 = _mm_mul_ps(tri.cv3, w1);
  const __m128 cc3 = _mm_mul_ps(tri.cv2, w2);

  const __m128 t1  = _mm_mul_ps(tri.tv1, w0);
  const __m128 t2  = _mm_mul_ps(tri.tv3, w1);
  const __m128 t3  = _mm_mul_ps(tri.tv2, w2);

  __m128 clr = _mm_add_ps(cc1, _mm_add_ps(cc2, cc3));
  __m128 tc  = _mm_add_ps(t1, _mm_add_ps(t2, t3));

#ifdef PERSP_CORRECT

  const __m128 z1 = _mm_rcp_ss(zInv);
  const __m128 z = _mm_shuffle_ps(z1, z1, _MM_SHUFFLE(0, 0, 0, 0));

  clr = _mm_mul_ps(clr, z);
  tc  = _mm_mul_ps(tc, z);

#endif

  const __m128 texColor = tex2D_sse(tri.texS, tc);

  return RealColorToUint32_BGRA(_mm_mul_ps(clr, texColor));
}

#endif


struct Colored3DSSE
{
  inline static int DrawPixel(const Triangle& tri, const __m128& w, const __m128& zInv) { return HSPixelColor(tri, w, zInv); }
};

struct ColoredTexured3DSSE
{
  inline static int DrawPixel(const Triangle& tri, const __m128& w, const __m128& zInv) { return HSPixelColorTex(tri, w, zInv); }
};


const __m128 g_epsE3 = _mm_set_ps(-10000000.0f, HALF_SPACE_EPSILON, HALF_SPACE_EPSILON, HALF_SPACE_EPSILON);
const __m128 g_one   = _mm_set_ps(1.0f, 1.0f, 1.0f, 1.0f);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename FillFunc>
void rasterizeTriHalfSpaceSimple2D(FrameBuffer* frameBuf, const Triangle& tri)
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

  __m128 Cy = vCy;

  // Scan through bounding rectangle
  for (int y = miny; y <= maxy; y++)
  {
    __m128 Cx = Cy;

    for (int x = minx; x <= maxx; x++)
    {
      if ((_mm_movemask_ps(_mm_cmpgt_ps(Cx, g_epsE3)) & 7) == 7)
        colorBuffer[x] = FillFunc::DrawPixel(tri, barycentricCoordFromHalfSpaceDist(Cx), g_one);

      Cx = _mm_sub_ps(Cx, Dy);
    }

    Cy = _mm_add_ps(Cy, Dx);

    colorBuffer += frameBuf->w;
  }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename FillFunc>
void rasterizeTriHalfSpaceSimple3D(FrameBuffer* frameBuf, const Triangle& tri)
{
  // Bounding rectangle
  const int minx = tri.bb_iminX;
  const int maxx = tri.bb_imaxX;
  const int miny = tri.bb_iminY;
  const int maxy = tri.bb_imaxY;

  int offset       = miny * frameBuf->w;
  int* colorBuffer = frameBuf->data;
  //uint8_t* sbuff   = frameBuf->getSBuffer();
  float* zbuff     = frameBuf->getZBuffer();

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

  __m128 Cy = vCy;

  // Scan through bounding rectangle
  for (int y = miny; y <= maxy; y++)
  {
    __m128 Cx = Cy;

    for (int x = minx; x <= maxx; x++)
    {
      if ((_mm_movemask_ps(_mm_cmpgt_ps(Cx, g_epsE3)) & 7) == 7)
      {
        const __m128 w        = barycentricCoordFromHalfSpaceDist(Cx);

        const __m128 vertZ    = _mm_shuffle_ps(tri.v3v2v1Z, tri.v3v2v1Z, _MM_SHUFFLE(3, 1, 2, 0));
        const __m128 zInvV    = _mm_dp_ps(w, vertZ, 0x7f);
        const __m128 zBuffVal = _mm_load_ss(zbuff + offset + x);
        const __m128 cmpRes   = _mm_cmpgt_ss(zInvV, zBuffVal);

        if (_mm_movemask_ps(cmpRes) & 1)
        {
          colorBuffer[offset + x] = FillFunc::DrawPixel(tri, w, zInvV);
          _mm_store_ss(zbuff + offset + x, zInvV);
        }
      }

      Cx = _mm_sub_ps(Cx, Dy);
    }

    Cy = _mm_add_ps(Cy, Dx);

    offset += frameBuf->w;
  }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#else // no SSE

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef FILL_COLOR_ONLY

inline int HSPixelColor2D(const Triangle& tri, const float3& w, const float zInv)
{
  return 0xFFFFFFFF;
}

inline int HSPixelColor3D(const Triangle& tri, const float3& w, const float zInv)
{
  return 0xFFFFFFFF;
}

inline int HSPixelColorTex3D(const Triangle& tri, const float3& w, const float zInv)
{
  return 0xFFFFFFFF;
}

#else

inline int HSPixelColor2D(const Triangle& tri, const float3& w, const float zInv)
{
  const float4 color = w.x*tri.c1 + w.y*tri.c3 + w.z*tri.c2;
  return RealColorToUint32_BGRA(color);
}

inline int HSPixelColor3D(const Triangle& tri, const float3& w, const float zInv)
{
  float4 color = w.x*tri.c1 + w.y*tri.c3 + w.z*tri.c2;

#ifdef PERSP_CORRECT

  const float z = 1.0f/zInv;
  color *= z;

#endif

  return RealColorToUint32_BGRA(color);
}


inline int HSPixelColorTex3D(const Triangle& tri, const float3& w, const float zInv)
{
  float4 color    = w.x*tri.c1 + w.y*tri.c3 + w.z*tri.c2;
  float2 texCoord = w.x*tri.t1 + w.y*tri.t3 + w.z*tri.t2;

#ifdef PERSP_CORRECT

  const float z = 1.0f / zInv;
  color    *= z;
  texCoord *= z;

#endif

  const float4 texColor = tex2D(tri.texS, texCoord);

  return RealColorToUint32_BGRA(color*texColor);
}

#endif

struct Colored2D
{
  inline static int DrawPixel(const Triangle& tri, const float3& w, const float zInv) { return HSPixelColor2D(tri, w, zInv); }
};

struct Colored3D
{
  inline static int DrawPixel(const Triangle& tri, const float3& w, const float zInv) { return HSPixelColor3D(tri, w, zInv); }
};

struct ColoredTextured3D
{
  inline static int DrawPixel(const Triangle& tri, const float3& w, const float zInv) { return HSPixelColorTex3D(tri, w, zInv); }
};

inline float3 barycentricCoordFromHalfSpaceDist(const float Cx1, const float Cx2, const float Cx3)
{
  const float scale = 1.0f / (Cx1 + Cx2 + Cx3);
  return float3(Cx1, Cx2, Cx3)*scale;
}


template<typename FillFunc>
void rasterizeTriHalfSpaceSimple2D(FrameBuffer* frameBuf, const Triangle& tri)
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

  int* colorBuffer = frameBuf->data;

  int offset = lineOffset(miny, frameBuf->w, frameBuf->h);

  // Constant part of half-edge functions
  const float C1 = Dy12 * x1 - Dx12 * y1;
  const float C2 = Dy23 * x2 - Dx23 * y2;
  const float C3 = Dy31 * x3 - Dx31 * y3;

  float Cy1 = C1 + Dx12 * miny - Dy12 * minx;
  float Cy2 = C2 + Dx23 * miny - Dy23 * minx;
  float Cy3 = C3 + Dx31 * miny - Dy31 * minx;

  // Scan through bounding rectangle
  for (int y = miny; y <= maxy; y++)
  {
    // Start value for horizontal scan
    float Cx1 = Cy1;
    float Cx2 = Cy2;
    float Cx3 = Cy3;

    for (int x = minx; x <= maxx; x++)
    {
      if (Cx1 > HALF_SPACE_EPSILON && Cx2 > HALF_SPACE_EPSILON && Cx3 > HALF_SPACE_EPSILON)
        colorBuffer[offset + x] = FillFunc::DrawPixel(tri, barycentricCoordFromHalfSpaceDist(Cx1, Cx2, Cx3), 1.0f);

      Cx1 -= Dy12;
      Cx2 -= Dy23;
      Cx3 -= Dy31;
    }

    Cy1 += Dx12;
    Cy2 += Dx23;
    Cy3 += Dx31;

    offset = nextLine(offset, frameBuf->w, frameBuf->h);
  }

}

template<typename FillFunc>
void rasterizeTriHalfSpaceSimple3D(FrameBuffer* frameBuf, const Triangle& tri)
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

  int* colorBuffer = frameBuf->data;
  //uint8_t* sbuff   = frameBuf->getSBuffer();
  float* zbuff     = frameBuf->getZBuffer();

  // Constant part of half-edge functions
  const float C1 = Dy12 * x1 - Dx12 * y1;
  const float C2 = Dy23 * x2 - Dx23 * y2;
  const float C3 = Dy31 * x3 - Dx31 * y3;

  float Cy1 = C1 + Dx12 * miny - Dy12 * minx;
  float Cy2 = C2 + Dx23 * miny - Dy23 * minx;
  float Cy3 = C3 + Dx31 * miny - Dy31 * minx;

  int offset = lineOffset(miny, frameBuf->w, frameBuf->h);

  // Scan through bounding rectangle
  for (int y = miny; y <= maxy; y++)
  {
    // Start value for horizontal scan
    float Cx1 = Cy1;
    float Cx2 = Cy2;
    float Cx3 = Cy3;

    for (int x = minx; x <= maxx; x++)
    {
      if (Cx1 > HALF_SPACE_EPSILON && Cx2 > HALF_SPACE_EPSILON && Cx3 > HALF_SPACE_EPSILON)
      {
        const float3 w       = barycentricCoordFromHalfSpaceDist(Cx1, Cx2, Cx3);
        const float zInv     = dot(w, float3(tri.v1.z, tri.v3.z, tri.v2.z));
        const float zBuffVal = zbuff[offset + x];

        if (zInv > zBuffVal)
        {
          colorBuffer[offset + x] = FillFunc::DrawPixel(tri, w, zInv);
          zbuff      [offset + x] = zInv;
        }

      }

      Cx1 -= Dy12;
      Cx2 -= Dy23;
      Cx3 -= Dy31;
    }

    Cy1 += Dx12;
    Cy2 += Dx23;
    Cy3 += Dx31;

    offset = nextLine(offset, frameBuf->w, frameBuf->h);
  }

}


#endif


void rasterizeTriHalfSpaceNaive(FrameBuffer* frameBuf, const Triangle& tri)
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

  int* colorBuffer = frameBuf->data;

  int offset = lineOffset(miny, frameBuf->w, frameBuf->h);

  for (int y = miny; y < maxy; y++)
  {
    for (int x = minx; x < maxx; x++)
    {
      // When all half-space functions positive, pixel is in triangle
      if (Dx12 * (y - y1) - Dy12 * (x - x1) > HALF_SPACE_EPSILON &&
          Dx23 * (y - y2) - Dy23 * (x - x2) > HALF_SPACE_EPSILON &&
          Dx31 * (y - y3) - Dy31 * (x - x3) > HALF_SPACE_EPSILON)
      {
        colorBuffer[offset + x] = 0x00FFFFFF; // White
      }
    }

    offset = nextLine(offset, frameBuf->w, frameBuf->h);
  }



}


void rasterizeTriHalfSpace(const FillFuncPtr pFill, FrameBuffer* frameBuf, const Triangle& tri)
{
#ifdef ENABLE_SSE

  if(pFill == DrawSpan_Colored3D_SSE)
  {
    rasterizeTriHalfSpaceSimple3D<Colored3DSSE>(frameBuf, tri);
  }
  else if (pFill == DrawSpan_TexLinear3D_SSE)
  {
    rasterizeTriHalfSpaceSimple3D<ColoredTexured3DSSE>(frameBuf, tri);
  }
  else
  {
    rasterizeTriHalfSpaceSimple2D<Colored3DSSE>(frameBuf, tri);
  }

#else // don't implement different versions without SSE for a while ...

  //rasterizeTriHalfSpaceNaive(frameBuf, tri);
  //return;

  //if (pFill == DrawSpan_Colored3D)
  //{
    rasterizeTriHalfSpaceSimple3D<Colored3D>(frameBuf, tri);
  //}
  //else if (pFill == DrawSpan_TexLinear3D)
  //{
  //  rasterizeTriHalfSpaceSimple3D<ColoredTextured3D>(frameBuf, tri);
  //}
  //else
  //{
  //  rasterizeTriHalfSpaceSimple2D<Colored2D>(frameBuf, tri);
  //}

#endif

}



void rasterizeTriHalfSpaceSimple(FrameBuffer* frameBuf, const Triangle& tri)
{
#ifndef ENABLE_SSE
  rasterizeTriHalfSpaceSimple2D<Colored2D>(frameBuf, tri);
#endif
}
