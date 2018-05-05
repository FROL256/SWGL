#ifndef TEXSAMPLER_GUARDIAN
#define TEXSAMPLER_GUARDIAN


#include "LiteMath.h"
#include "TriRaster.h"

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

inline float4 read_imagef(const int* pData, const int w, const int h, int pitch, const float2 a_texCoord) // look ak LaMote's integer interpolation
{
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

  const float4 f1 = mult*make_float4((float)p1.x, (float)p1.y, (float)p1.z, (float)p1.w);
  const float4 f2 = mult*make_float4((float)p2.x, (float)p2.y, (float)p2.z, (float)p2.w);
  const float4 f3 = mult*make_float4((float)p3.x, (float)p3.y, (float)p3.z, (float)p3.w);
  const float4 f4 = mult*make_float4((float)p4.x, (float)p4.y, (float)p4.z, (float)p4.w);

  // Calculate the weighted sum of pixels (for each color channel)
  //
  float outr = f1.x * w1 + f2.x * w2 + f3.x * w3 + f4.x * w4;
  float outg = f1.y * w1 + f2.y * w2 + f3.y * w3 + f4.y * w4;
  float outb = f1.z * w1 + f2.z * w2 + f3.z * w3 + f4.z * w4;
  float outa = f1.w * w1 + f2.w * w2 + f3.w * w3 + f4.w * w4;

  return make_float4(outr, outg, outb, outa);
}

#endif

inline static float2 wrapTexCoord(float2 a_texCoord)
{
  a_texCoord = a_texCoord - make_float2((float)((int)(a_texCoord.x)), (float)((int)(a_texCoord.y)));

  float x = a_texCoord.x < 0.0f ? a_texCoord.x + 1.0f : a_texCoord.x;
  float y = a_texCoord.y < 0.0f ? a_texCoord.y + 1.0f : a_texCoord.y;

  return make_float2(x, y);
}


inline float4 tex2D(const TexSampler& sampler, float2 texCoord)
{
  return read_imagef(sampler.data, sampler.w, sampler.h, sampler.pitch, wrapTexCoord(texCoord));
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef ENABLE_SSE

inline __m128 CalcWeights(const __m128 psXY)
{
  const __m128 psXYfloor = _mm_floor_ps(psXY); // use this line for if you have SSE4
  //__m128 psXYfloor = _mm_cvtepi32_ps(_mm_cvtps_epi32(psXY));
  const __m128 psXYfrac = _mm_sub_ps(psXY, psXYfloor);            // = frac(psXY)

  const __m128 psXYfrac1 = _mm_sub_ps(const_1111, psXYfrac);       // ? ? (1-y) (1-x)
  const __m128 w_x       = _mm_unpacklo_ps(psXYfrac1, psXYfrac);   // ? ?     x (1-x)
  const __m128 w_x2      = _mm_movelh_ps(w_x, w_x);                // x (1-x) x (1-x)
  const __m128 w_y       = _mm_shuffle_ps(psXYfrac1, psXYfrac, _MM_SHUFFLE(1, 1, 1, 1)); // y y (1-y) (1-y)

  return _mm_mul_ps(w_x2, w_y);  // complete weight vector
}

inline __m128 read_imagef_sse(const int* data, const int w, const int h, const int pitch, const __m128 wh, const __m128 tc)
{
  const __m128  ffxy = _mm_min_ps(_mm_max_ps(_mm_sub_ps(_mm_mul_ps(wh, tc), const_half_one), const_0000), _mm_sub_ps(wh, const_1111));
  //const __m128  ffxy = _mm_sub_ps(_mm_mul_ps(wh, tc), const_half_one);
  const __m128i iixy = _mm_cvtps_epi32(ffxy);

  int px = _mm_cvtsi128_si32(iixy);
  int py = _mm_cvtsi128_si32(_mm_shuffle_epi32(iixy, _MM_SHUFFLE(1, 1, 1, 1)));

  const int* p0 = data + px + py * pitch; // pointer to first pixel

  // Load the data (2 pixels in one load)
  const __m128i p12 = _mm_loadl_epi64((const __m128i*)&p0[0 * pitch]);
  const __m128i p34 = _mm_loadl_epi64((const __m128i*)&p0[1 * pitch]);

  __m128 weight = CalcWeights(ffxy);

  // convert RGBA RGBA RGBA RGAB to RRRR GGGG BBBB AAAA (AoS to SoA)
  const __m128i p1234 = _mm_unpacklo_epi8(p12, p34);
  const __m128i p34xx = _mm_unpackhi_epi64(p1234, _mm_setzero_si128());
  const __m128i p1234_8bit = _mm_unpacklo_epi8(p1234, p34xx);

  // extend to 16bit 
  const __m128i pRG = _mm_unpacklo_epi8(p1234_8bit, _mm_setzero_si128());
  const __m128i pBA = _mm_unpackhi_epi8(p1234_8bit, _mm_setzero_si128());

  // convert weights to integer
  weight = _mm_mul_ps(weight, const_256);
  __m128i weighti = _mm_cvtps_epi32(weight); // w4 w3 w2 w1
  weighti = _mm_packs_epi32(weighti, weighti); // 32->2x16bit

  //outRG = [w1*R1 + w2*R2 | w3*R3 + w4*R4 | w1*G1 + w2*G2 | w3*G3 + w4*G4]
  const __m128i outRG = _mm_madd_epi16(pRG, weighti);
  //outBA = [w1*B1 + w2*B2 | w3*B3 + w4*B4 | w1*A1 + w2*A2 | w3*A3 + w4*A4]
  const __m128i outBA = _mm_madd_epi16(pBA, weighti);

  const __m128i color255  = _mm_srli_epi32(_mm_hadd_epi32(outRG, outBA), 8);
  const __m128  finColor  = _mm_mul_ps(const_255_inv, _mm_cvtepi32_ps(color255));

  return _mm_shuffle_ps(finColor, finColor, _MM_SHUFFLE(3, 0, 1, 2)); // swap red and blue
}


inline static __m128 wrapTexCoord(const __m128 a_texCoord)
{
  const __m128 texCoord2 = _mm_sub_ps(a_texCoord, _mm_floor_ps(a_texCoord));
  const __m128 a_texCoord3 = _mm_add_ps(texCoord2, const_1111);
  const __m128 lessMask = _mm_cmplt_ps(texCoord2, _mm_setzero_ps());

  return _mm_or_ps(_mm_and_ps(lessMask, a_texCoord3), _mm_andnot_ps(lessMask, texCoord2));
}

inline __m128 tex2D_sse(const TexSampler& sampler, const __m128 texCoord)
{
  return read_imagef_sse(sampler.data, sampler.w, sampler.h, sampler.pitch, sampler.txwh, wrapTexCoord(texCoord));
}


#endif


#endif

