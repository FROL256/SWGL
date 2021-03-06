#include "config.h"
#include "swgl.h"

#ifdef WIN32
#undef min
#undef max
#endif

#include <algorithm>

#include "LiteMath.h"
using namespace LiteMath;

#ifdef LINUX_PPC_HS_INVERT_Y

static inline inline int lineOffset(int y, int w, int h) { return (h - y - 1)*w; }
static inline inline int nextLine(int y, int w, int h)   { return y - w; }

#else

static inline int lineOffset(int y, int w, int h) { return y*w; }
static inline int nextLine(int y, int w, int h)   { return y + w; }

#endif

static inline float edgeFunction(LiteMath::float2 a, LiteMath::float2 b, LiteMath::float2 c) // actuattly just a mixed product ... :)
{
  return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

#define HALF_SPACE_EPSILON (-1e-4f)

using TriangleLocal = HWImplementationPureCpp::TriangleType;

void HWImplementationPureCpp::memset32(int32_t* a_data, int32_t a_val, int32_t numElements)
{
  if (numElements % 16 == 0)
  {
    const uint64_t val = (((uint64_t)a_val) << 32) | (uint64_t)a_val;
    uint64_t* color64  = (uint64_t*)(a_data);
    const int size     = numElements/2;

    for (int i = 0; i < size; i += 8)
    {
      color64[i + 0] = val;
      color64[i + 1] = val;
      color64[i + 2] = val;
      color64[i + 3] = val;
      color64[i + 4] = val;
      color64[i + 5] = val;
      color64[i + 6] = val;
      color64[i + 7] = val;
    }
  }
  else
  {
    for (int i = 0; i < numElements; i++)
      a_data[i] = a_val;
  }
}


bool HWImplementationPureCpp::AABBTriangleOverlap(const TriangleType& a_tri, const int tileMinX, const int tileMinY, const int tileMaxX, const int tileMaxY)
{
  const bool overlapBoxBox = LiteMath::IntersectBox2Box2(int2(a_tri.bb_iminX, a_tri.bb_iminY), int2(a_tri.bb_imaxX, a_tri.bb_imaxY),
                                                         int2(tileMinX, tileMinY),             int2(tileMaxX, tileMaxY));

  if (!overlapBoxBox)
    return false;

  const float xx0 = float(tileMinX);
  const float xx1 = float(tileMaxX);
  const float yy0 = float(tileMinY);
  const float yy1 = float(tileMaxY);

  const float y1 = a_tri.v3.y;
  const float y2 = a_tri.v2.y;
  const float y3 = a_tri.v1.y;
                   
  const float x1 = a_tri.v3.x;
  const float x2 = a_tri.v2.x;
  const float x3 = a_tri.v1.x;

  // Deltas
  const float Dx12 = x1 - x2;
  const float Dy12 = y1 - y2;

  const bool s10 = ( Dx12 * (yy0 - y1) - Dy12 * (xx0 - x1) ) < 0.0f; 
  const bool s11 = ( Dx12 * (yy0 - y1) - Dy12 * (xx1 - x1) ) < 0.0f;
  const bool s12 = ( Dx12 * (yy1 - y1) - Dy12 * (xx0 - x1) ) < 0.0f;
  const bool s13 = ( Dx12 * (yy1 - y1) - Dy12 * (xx1 - x1) ) < 0.0f;
             
  const bool s20 = ( Dx12 * (yy0 - y2) - Dy12 * (xx0 - x2) ) < 0.0f;
  const bool s21 = ( Dx12 * (yy0 - y2) - Dy12 * (xx1 - x2) ) < 0.0f;
  const bool s22 = ( Dx12 * (yy1 - y2) - Dy12 * (xx0 - x2) ) < 0.0f;
  const bool s23 = ( Dx12 * (yy1 - y2) - Dy12 * (xx1 - x2) ) < 0.0f;
             
  const bool s30 = ( Dx12 * (yy0 - y3) - Dy12 * (xx0 - x3) ) < 0.0f;
  const bool s31 = ( Dx12 * (yy0 - y3) - Dy12 * (xx1 - x3) ) < 0.0f;
  const bool s32 = ( Dx12 * (yy1 - y3) - Dy12 * (xx0 - x3) ) < 0.0f;
  const bool s33 = ( Dx12 * (yy1 - y3) - Dy12 * (xx1 - x3) ) < 0.0f;

  return (s10 || s11 || s12 || s13 ||
          s20 || s21 || s22 || s23 ||
          s30 || s31 || s32 || s33 );
}


void HWImplementationPureCpp::VertexShader(const float* v_in4f, float* v_out4f, int a_numVert,
                                           const float viewportData[4], const float4x4 worldViewProjMatrix)
{
  const float4   viewportf(viewportData[0], viewportData[1], viewportData[2], viewportData[3]);
  
  const float4* inVert  = (const float4*)v_in4f;
  float4* outVert       = (float4*)v_out4f;

  for (int i = 0; i < a_numVert; i++) // this implementations became broken if posCamSpace.z > -1.0f ...
  {
    const float4 clipSpace = worldViewProjMatrix*inVert[i];
    if(clipSpace.w > 0.0f)
    {
      const float invW          = 1.0f/clipSpace.w;                                           // its ok, no zero division, see (clipSpace.w >= NEAR_CLIP_PLANE2) condition
      const float4 vClipSpace   = float4(clipSpace.x * invW, clipSpace.y * invW, invW, 1.0f);
      const float4 vScreenSpace = swglClipSpaceToScreenSpaceTransform(vClipSpace, viewportf);
      outVert[i]                = vScreenSpace;
    }
    else
    {
      outVert[i] = clipSpace;
    }
  }

}

inline float4 read_imagef(const int* pData, const int w, const int h, int pitch, const float2 a_texCoord) // look ak LaMote's integer interpolation
{
  struct uchar4
  {
    unsigned char x, y, z, w;
  };

  const float fw = (float)(w);
  const float fh = (float)(h);

  //const float ffx = a_texCoord.x*fw - 0.5f; 
  //const float ffy = a_texCoord.y*fh - 0.5f; 

  const float ffx = clamp(a_texCoord.x*fw - 0.5f, 0.0f, fw - 1.0f);
  const float ffy = clamp(a_texCoord.y*fh - 0.5f, 0.0f, fh - 1.0f);

  const int px = (int)(ffx);
  const int py = (int)(ffy);

  // Calculate the weights for each pixel
  //
  const float fx  = ffx - (float)px;
  const float fy  = ffy - (float)py;
  const float fx1 = 1.0f - fx;
  const float fy1 = 1.0f - fy;

  const float w1 = fx1 * fy1;
  const float w2 = fx * fy1;
  const float w3 = fx1 * fy;
  const float w4 = fx * fy;

  const uchar4* a_data = (const uchar4*)pData;
  const uchar4* p0 = a_data + (py*pitch) + px;

  const uchar4 p1 = p0[0 + 0 * pitch];
  const uchar4 p2 = p0[1 + 0 * pitch];
  const uchar4 p3 = p0[0 + 1 * pitch];
  const uchar4 p4 = p0[1 + 1 * pitch];

  const float mult = 0.003921568f; // (1.0f/255.0f);

  const float4 f1 = mult * float4((float)p1.x, (float)p1.y, (float)p1.z, (float)p1.w);
  const float4 f2 = mult * float4((float)p2.x, (float)p2.y, (float)p2.z, (float)p2.w);
  const float4 f3 = mult * float4((float)p3.x, (float)p3.y, (float)p3.z, (float)p3.w);
  const float4 f4 = mult * float4((float)p4.x, (float)p4.y, (float)p4.z, (float)p4.w);

  // Calculate the weighted sum of pixels (for each color channel)
  //
  float outr = f1.x * w1 + f2.x * w2 + f3.x * w3 + f4.x * w4;
  float outg = f1.y * w1 + f2.y * w2 + f3.y * w3 + f4.y * w4;
  float outb = f1.z * w1 + f2.z * w2 + f3.z * w3 + f4.z * w4;
  float outa = f1.w * w1 + f2.w * w2 + f3.w * w3 + f4.w * w4;

  return float4(outr, outg, outb, outa);
}


inline static float2 wrapTexCoord(float2 a_texCoord)
{
  a_texCoord = a_texCoord - float2((float)((int)(a_texCoord.x)), (float)((int)(a_texCoord.y)));

  float x = a_texCoord.x < 0.0f ? a_texCoord.x + 1.0f : a_texCoord.x;
  float y = a_texCoord.y < 0.0f ? a_texCoord.y + 1.0f : a_texCoord.y;

  return float2(x, y);
}


inline float4 tex2D(const TexSampler& sampler, float2 texCoord)
{
  return read_imagef(sampler.data, sampler.w, sampler.h, sampler.pitch, wrapTexCoord(texCoord));
}


struct FillColor
{
  inline static float4 DrawPixel(const TriangleLocal& tri, const float3& w) { return tri.c1; }
};

struct Colored2D
{
  inline static float4 DrawPixel(const TriangleLocal& tri, const float3& w)
  { 
    return tri.c1*w.x + tri.c2*w.y + tri.c3*w.z;
  }
};

struct Colored3D
{
  inline static float4 DrawPixel(const TriangleLocal& tri, const float3& w, const float zInv)
  {
    float4 color = tri.c1*w.x + tri.c2*w.y + tri.c3*w.z;

#ifdef PERSP_CORRECT
    const float z = 1.0f / zInv;
    color *= z;
#endif

    return color;
  }
};


struct Textured2D
{
  inline static float4 DrawPixel(const TriangleLocal& tri, const float3& w)
  {
    const float4 color    = tri.c1*w.x + tri.c2*w.y + tri.c3*w.z;
    const float2 texCoord = tri.t1*w.x + tri.t2*w.y + tri.t3*w.z;
    const float4 texColor = tex2D(tri.texS, texCoord);
    return texColor*color;
  }
};

struct Textured3D
{
  inline static float4 DrawPixel(const TriangleLocal& tri, const float3& w, const float zInv)
  {
    float4 color    = tri.c1*w.x + tri.c2*w.y + tri.c3*w.z;
    float2 texCoord = tri.t1*w.x + tri.t2*w.y + tri.t3*w.z;

#ifdef PERSP_CORRECT
    const float z = 1.0f / zInv;
    color    *= z;
    texCoord *= z;
#endif

    const float4 texColor = tex2D(tri.texS, texCoord);
    return texColor*color;
  }
};

struct Blend_Alpha_OneMinusAlpha
{
  inline static int BlendPixel(const int a_colorBefore, const float4 newColor)
  {
    const float4 oldColor = color_unpack_bgra(a_colorBefore);
    const float alpha     = newColor.w;
    return color_pack_bgra(oldColor*(1.0f - alpha) + alpha*newColor);
  }
};


static void RasterizeTriHalfSpace2D_Fill(const TriangleLocal& tri, int tileMinX, int tileMinY, FrameBuffer* frameBuf)
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

  int* cbuff     = nullptr; // frameBuf->cbuffer;

  // Constant part of half-edge functions
  const float C1 = Dy12 * x1 - Dx12 * y1;
  const float C2 = Dy23 * x2 - Dx23 * y2;
  const float C3 = Dy31 * x3 - Dx31 * y3;

  const float areaInv = 1.0f / fabs(Dy31*Dx12 - Dx31*Dy12); // edgeFunction(v0, v1, v2);

  float Cy1 = C1 + Dx12 * miny - Dy12 * minx;
  float Cy2 = C2 + Dx23 * miny - Dy23 * minx;
  float Cy3 = C3 + Dx31 * miny - Dy31 * minx;

  int offset = lineOffset(miny, frameBuf->pitch, frameBuf->h);

  const int fillColorI = color_pack_bgra(tri.c1*(1.0f/tri.v1.z)); // #TODO: need opt for color fill mode

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
        cbuff[offset + x]  = fillColorI;

      Cx1 -= Dy12;
      Cx2 -= Dy23;
      Cx3 -= Dy31;
    }

    Cy1 += Dx12;
    Cy2 += Dx23;
    Cy3 += Dx31;

    offset = nextLine(offset, frameBuf->pitch, frameBuf->h);
  }

}

template<typename ROP>
static void RasterizeTriHalfSpace2D(const TriangleLocal& tri, int tileMinX, int tileMinY, FrameBuffer* frameBuf)
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

  int* cbuff     = nullptr; //frameBuf->cbuffer;

  // Constant part of half-edge functions
  const float C1 = Dy12 * x1 - Dx12 * y1;
  const float C2 = Dy23 * x2 - Dx23 * y2;
  const float C3 = Dy31 * x3 - Dx31 * y3;

  const float areaInv = 1.0f / fabs(Dy31*Dx12 - Dx31*Dy12); // edgeFunction(v0, v1, v2);

  const float fMinY = float(miny);
  const float fMinX = float(minx);

  float Cy1 = C1 + Dx12 * fMinY - Dy12 * fMinX;
  float Cy2 = C2 + Dx23 * fMinY - Dy23 * fMinX;
  float Cy3 = C3 + Dx31 * fMinY - Dy31 * fMinX;

  int offset = lineOffset(miny, frameBuf->pitch, frameBuf->h);

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
        const float4 color = ROP::DrawPixel(tri, areaInv*float3(Cx1, Cx3, Cx2));
        cbuff[offset + x]  = LiteMath::color_pack_bgra(color);
      }

      Cx1 -= Dy12;
      Cx2 -= Dy23;
      Cx3 -= Dy31;
    }

    Cy1 += Dx12;
    Cy2 += Dx23;
    Cy3 += Dx31;

    offset = nextLine(offset, frameBuf->pitch, frameBuf->h);
  }

}

template<typename ROP>
static void RasterizeTriHalfSpace3D(const TriangleLocal& tri, int tileMinX, int tileMinY, FrameBuffer* frameBuf)
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

  int*     cbuff = nullptr; // frameBuf->cbuffer;
  float*   zbuff = nullptr; // frameBuf->zbuffer;
  //uint8_t* sbuff = frameBuf->sbuffer;

  // Constant part of half-edge functions
  const float C1 = Dy12 * x1 - Dx12 * y1;
  const float C2 = Dy23 * x2 - Dx23 * y2;
  const float C3 = Dy31 * x3 - Dx31 * y3;

  const float areaInv = 1.0f / fabs(Dy31*Dx12 - Dx31 * Dy12); // edgeFunction(v0, v1, v2);

  const float fMinY = float(miny);
  const float fMinX = float(minx);

  float Cy1 = C1 + Dx12 * fMinY - Dy12 * fMinX;
  float Cy2 = C2 + Dx23 * fMinY - Dy23 * fMinX;
  float Cy3 = C3 + Dx31 * fMinY - Dy31 * fMinX;

  int offset = lineOffset(miny, frameBuf->pitch, frameBuf->h);

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
        const float3 w       = areaInv * float3(Cx1, Cx3, Cx2);
        const float zInv     = tri.v1.z*w.x + tri.v2.z*w.y + tri.v3.z*w.z;
        const float zBuffVal = zbuff[offset + x];

        if (zInv > zBuffVal)
        {
          const float4 col  = ROP::DrawPixel(tri, areaInv*float3(Cx1, Cx3, Cx2), zInv);
          cbuff[offset + x] = LiteMath::color_pack_bgra(col);
          zbuff[offset + x] = zInv;
        }
      }

      Cx1 -= Dy12;
      Cx2 -= Dy23;
      Cx3 -= Dy31;
    }

    Cy1 += Dx12;
    Cy2 += Dx23;
    Cy3 += Dx31;

    offset = nextLine(offset, frameBuf->pitch, frameBuf->h);
  }

}

template<typename ROP, typename BOP>
static void RasterizeTriHalfSpace3DBlend(const TriangleLocal& tri, int tileMinX, int tileMinY, FrameBuffer* frameBuf)
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

  int*   cbuff = nullptr; //frameBuf->cbuffer;
  float* zbuff = nullptr; //frameBuf->zbuffer;
  //uint8_t* sbuff = frameBuf->getSBuffer();

  // Constant part of half-edge functions
  const float C1 = Dy12 * x1 - Dx12 * y1;
  const float C2 = Dy23 * x2 - Dx23 * y2;
  const float C3 = Dy31 * x3 - Dx31 * y3;

  const float areaInv = 1.0f / fabs(Dy31*Dx12 - Dx31 * Dy12); // edgeFunction(v0, v1, v2);

  const float fMinY = float(miny);
  const float fMinX = float(minx);

  float Cy1 = C1 + Dx12 * fMinY - Dy12 * fMinX;
  float Cy2 = C2 + Dx23 * fMinY - Dy23 * fMinX;
  float Cy3 = C3 + Dx31 * fMinY - Dy31 * fMinX;

  int offset = lineOffset(miny, frameBuf->pitch, frameBuf->h);

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
        const float3 w       = areaInv * float3(Cx1, Cx3, Cx2);
        const float zInv     = tri.v1.z*w.x + tri.v2.z*w.y + tri.v3.z*w.z;
        const float zBuffVal = zbuff[offset + x];

        if (zInv > zBuffVal)
        {
          const float4 color = ROP::DrawPixel(tri, areaInv*float3(Cx1, Cx3, Cx2), zInv);
          cbuff[offset + x]  = BOP::BlendPixel(cbuff[offset + x], color);
          zbuff[offset + x]  = zInv;
        }
      }

      Cx1 -= Dy12;
      Cx2 -= Dy23;
      Cx3 -= Dy31;
    }

    Cy1 += Dx12;
    Cy2 += Dx23;
    Cy3 += Dx31;

    offset = nextLine(offset, frameBuf->pitch, frameBuf->h);
  }

}


void HWImplementationPureCpp::RasterizeTriangle(const TriangleType& tri, FrameBuffer* frameBuf)
{
  const auto a_ropT = tri.ropId;

  constexpr int tileMinX = 0;
  constexpr int tileMinY = 0;

  switch (a_ropT)
  {
  case ROP_Colored2D:
    RasterizeTriHalfSpace2D<Colored2D>(tri, tileMinX, tileMinY, 
                                       frameBuf);
    break;

  case ROP_Colored3D:
    RasterizeTriHalfSpace3D<Colored3D>(tri, tileMinX, tileMinY, 
                                       frameBuf);
    break;

  case ROP_TexNearest2D:
  case ROP_TexLinear2D:
    RasterizeTriHalfSpace2D<Textured2D>(tri, tileMinX, tileMinY, 
                                        frameBuf);
    break;

  case ROP_TexNearest3D:
  case ROP_TexLinear3D:
    RasterizeTriHalfSpace3D<Textured3D>(tri, tileMinX, tileMinY, 
                                        frameBuf);
    break;

  case ROP_TexNearest3D_Blend:
  case ROP_TexLinear3D_Blend:
    RasterizeTriHalfSpace3DBlend<Textured3D, Blend_Alpha_OneMinusAlpha>(tri, tileMinX, tileMinY,
                                                                         frameBuf);
    break;

  default :
    RasterizeTriHalfSpace2D_Fill(tri, tileMinX, tileMinY, 
                                 frameBuf);
    break;
  };

}