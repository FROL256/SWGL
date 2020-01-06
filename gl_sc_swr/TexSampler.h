#ifndef TEXSAMPLER_GUARDIAN
#define TEXSAMPLER_GUARDIAN

#include "LiteMath.h"
#include "config.h"

struct uchar4
{
  unsigned char x, y, z, w;
};

#ifdef TEX_NEAREST

inline float4 read_imagef(const int* pData, const int w, const int h, const int pitch, const float2 a_texCoord) // look ak LaMote's integer interpolation
{
  const float fw = (float)(w);
  const float fh = (float)(h);

  const float ffx = a_texCoord.x*fw; // clamp(a_texCoord.x*fw - 0.5f, 0.0f, fw - 1.0f); // a_texCoord.x*fw; //clamp(a_texCoord.x*fw - 0.5f, 0.0f, fw - 1.0f);
  const float ffy = a_texCoord.y*fh; // clamp(a_texCoord.y*fh - 0.5f, 0.0f, fh - 1.0f); // a_texCoord.y*fh; //clamp(a_texCoord.y*fh - 0.5f, 0.0f, fh - 1.0f);

  const int px = (int)(ffx);
  const int py = (int)(ffy);

  const uchar4* a_data = (const uchar4*)pData;
  const uchar4 p0      = a_data[py*pitch + px];

  //return Uint32_BGRAToRealColor(pData[py*pitch + px]);

  const float mult = 0.003921568f; // (1.0f/255.0f);
  return mult*make_float4((float)p0.x, (float)p0.y, (float)p0.z, (float)p0.w);
}

#else

using LiteMath::float4;
using LiteMath::float2;

inline float4 read_imagef(const int* pData, const int w, const int h, int pitch, const float2 a_texCoord) // look ak LaMote's integer interpolation
{
  const float fw = (float)(w);
  const float fh = (float)(h);

  //const float ffx = a_texCoord.x*fw - 0.5f; 
  //const float ffy = a_texCoord.y*fh - 0.5f; 

  const float ffx = LiteMath::clamp(a_texCoord.x*fw - 0.5f, 0.0f, fw - 1.0f);
  const float ffy = LiteMath::clamp(a_texCoord.y*fh - 0.5f, 0.0f, fh - 1.0f);

  const int px = (int)(ffx);
  const int py = (int)(ffy);

  // Calculate the weights for each pixel
  //
  const float fx = ffx - (float)px;
  const float fy = ffy - (float)py;
  const float fx1 = 1.0f - fx;
  const float fy1 = 1.0f - fy;

  const float w1 = fx1 * fy1;
  const float w2 = fx  * fy1;
  const float w3 = fx1 * fy;
  const float w4 = fx  * fy;

  const uchar4* a_data = (const uchar4*)pData;
  const uchar4* p0 = a_data + (py*pitch) + px;

  const uchar4 p1 = p0[0 + 0 * pitch];
  const uchar4 p2 = p0[1 + 0 * pitch];
  const uchar4 p3 = p0[0 + 1 * pitch];
  const uchar4 p4 = p0[1 + 1 * pitch];

  const float mult = 0.003921568f; // (1.0f/255.0f);

  const float4 f1 = mult*float4((float)p1.x, (float)p1.y, (float)p1.z, (float)p1.w);
  const float4 f2 = mult*float4((float)p2.x, (float)p2.y, (float)p2.z, (float)p2.w);
  const float4 f3 = mult*float4((float)p3.x, (float)p3.y, (float)p3.z, (float)p3.w);
  const float4 f4 = mult*float4((float)p4.x, (float)p4.y, (float)p4.z, (float)p4.w);

  // Calculate the weighted sum of pixels (for each color channel)
  //
  float outr = f1.x * w1 + f2.x * w2 + f3.x * w3 + f4.x * w4;
  float outg = f1.y * w1 + f2.y * w2 + f3.y * w3 + f4.y * w4;
  float outb = f1.z * w1 + f2.z * w2 + f3.z * w3 + f4.z * w4;
  float outa = f1.w * w1 + f2.w * w2 + f3.w * w3 + f4.w * w4;

  return float4(outr, outg, outb, outa);
}

#endif

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


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#endif

