#include "DrawSpan.h"
#include "TexSampler.h"


struct PsRet
{
  float w0;
  float w1;
  float w2;
  float zInv;
  bool  overlaps;
};

inline static PsRet CheckPixelCoverage(float x, const TriangleDataNoSSE& tri, const float3 k1, const float3 k2)
{
  const float2 v1 = to_float2(tri.v1);
  const float2 v2 = to_float2(tri.v2);
  const float2 v3 = to_float2(tri.v3);

  //const float w0 = edgeFunction(v2, v3, p)*areaInv;
  //const float w1 = edgeFunction(v3, v1, p)*areaInv;
  //const float w2 = edgeFunction(v1, v2, p)*areaInv;

  const float w0 = ((x - v2.x)*k1.x - k2.x);
  const float w1 = ((x - v3.x)*k1.y - k2.y);
  const float w2 = ((x - v1.x)*k1.z - k2.z);

  const bool overlaps1 = ((w0 > -5e-6f && tri.edgeTest0) || (w0 > 0.0f));
  const bool overlaps2 = ((w1 > -5e-6f && tri.edgeTest1) || (w1 > 0.0f));
  const bool overlaps3 = ((w2 > -5e-6f && tri.edgeTest2) || (w2 > 0.0f));

  PsRet result;
  result.zInv = w0*tri.v1.z + w1*tri.v2.z + w2*tri.v3.z;
  result.overlaps = (overlaps1 && overlaps2 && overlaps3);
  result.w0 = w0;
  result.w1 = w1;
  result.w2 = w2;

  return result;
}


#ifdef ENABLE_SSE

struct ALIGNED16 PsRetSSE
{
  __m128 w;
  __m128 zInv;
  bool overlaps;
};

const __m128 negEps = _mm_set_ps(-5e-6f, -5e-6f, -5e-6f, -5e-6f);

inline static PsRetSSE CheckPixelCoverageSSE(float x, const TriangleDataYesSSE& tri, const __m128 k1, const __m128 k2) // give boost only for x64 build
{
  const float fx        = (float)x;

  const __m128 vx       = _mm_set_ps(0.0f, fx, fx, fx);
  const __m128 w        = _mm_sub_ps(_mm_mul_ps(_mm_sub_ps(vx, tri.v1v3v2X), k1), k2);
  const __m128 overlaps = _mm_or_ps(_mm_and_ps(_mm_cmpgt_ps(w, negEps), tri.edgeTest), _mm_cmpgt_ps(w, _mm_setzero_ps()));
  const __m128 zInvV    = _mm_dp_ps(w, tri.v3v2v1Z, 0x7f);

  PsRetSSE result;
  result.w        = w;
  result.zInv     = zInvV;
  result.overlaps = ((_mm_movemask_ps(overlaps) & 7) == 7);

  return result;
}

#endif



#ifdef ENABLE_SSE

inline __m128 CalcWeights(const float x, const float y)
{
  const __m128 ssx = _mm_set_ss(x);
  const __m128 ssy = _mm_set_ss(y);
  const __m128 psXY = _mm_unpacklo_ps(ssx, ssy);      // 0 0 y x

  const __m128 psXYfloor = _mm_floor_ps(psXY); // use this line for if you have SSE4
  //__m128 psXYfloor = _mm_cvtepi32_ps(_mm_cvtps_epi32(psXY));
  const __m128 psXYfrac = _mm_sub_ps(psXY, psXYfloor);            // = frac(psXY)

  const __m128 psXYfrac1 = _mm_sub_ps(const_1111, psXYfrac);       // ? ? (1-y) (1-x)
  const __m128 w_x = _mm_unpacklo_ps(psXYfrac1, psXYfrac);   // ? ?     x (1-x)
  const __m128 w_x2 = _mm_movelh_ps(w_x, w_x);                // x (1-x) x (1-x)
  const __m128 w_y = _mm_shuffle_ps(psXYfrac1, psXYfrac, _MM_SHUFFLE(1, 1, 1, 1)); // y y (1-y) (1-y)

  return _mm_mul_ps(w_x2, w_y);  // complete weight vector
}

#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawSpan_FillColor(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const TriangleDataNoSSE& t, const float3 k2)
{
  for (int x = x1c; x <= x2c; x++)
  {
    const PsRet psData = CheckPixelCoverage((float)x, t, t.k1, k2);

    if (psData.overlaps)
      frameBuff->data[offset + x] = 0xFFFFFFFF;
  }
}

#ifdef ENABLE_SSE

void DrawSpan_FillColor_SSE(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const TriangleDataYesSSE& t, const __m128 k2v)
{
  for (int x = x1c; x <= x2c; x++)
  {
    const PsRetSSE psData  = CheckPixelCoverageSSE((float)x, t, t.kv1, k2v);
    if (psData.overlaps)
      frameBuff->data[offset + x] = 0xFFFFFFFF;
  }
}

#endif



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawSpan_Colored2D(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const TriangleDataNoSSE& t, const float3 k2)
{
  uint8_t* sbuff         = frameBuff->getSBuffer();
  const uint8_t currVal  = t.curr_sval;
  const uint8_t currMask = t.curr_smask;

  for (int x = x1c; x <= x2c; x++)
  {
    const PsRet psData = CheckPixelCoverage((float)x, t, t.k1, k2);
    const bool spass = (sbuff == nullptr) ? true : (sbuff[offset + x] & currMask) != (currVal & currMask); // curr stencil func here ...

    if (psData.overlaps && spass)
    {
      const float4 color = psData.w0*t.c1 + psData.w1*t.c2 + psData.w2*t.c3;
      frameBuff->data[offset + x] = RealColorToUint32_BGRA(color);
    }
  }
}

#ifdef ENABLE_SSE

void DrawSpan_Colored2D_SSE(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const TriangleDataYesSSE& t, const __m128 k2v)
{
  uint8_t* sbuff         = frameBuff->getSBuffer();
  const uint8_t currVal  = t.curr_sval;
  const uint8_t currMask = t.curr_smask;

  for (int x = x1c; x <= x2c; x++)
  {
    const bool spass = (sbuff == nullptr) ? true : (sbuff[offset + x] & currMask) != (currVal & currMask); // curr stencil func here ...
    const PsRetSSE psData = CheckPixelCoverageSSE((float)x, t, t.kv1, k2v);
    if (psData.overlaps && spass)
    {
      const __m128 cc1 = _mm_mul_ps(t.cv1, _mm_shuffle_ps(psData.w, psData.w, _MM_SHUFFLE(0, 0, 0, 0)));
      const __m128 cc2 = _mm_mul_ps(t.cv2, _mm_shuffle_ps(psData.w, psData.w, _MM_SHUFFLE(1, 1, 1, 1)));
      const __m128 cc3 = _mm_mul_ps(t.cv3, _mm_shuffle_ps(psData.w, psData.w, _MM_SHUFFLE(2, 2, 2, 2)));

      frameBuff->data[offset + x] = RealColorToUint32_BGRA(_mm_add_ps(cc1, _mm_add_ps(cc2, cc3)));
    }
  }
}

#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawSpan_Colored3D(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const TriangleDataNoSSE& t, const float3 k2)
{
  uint8_t* sbuff         = frameBuff->getSBuffer();
  const uint8_t currVal  = t.curr_sval;
  const uint8_t currMask = t.curr_smask;

  float* zbuff = frameBuff->getZBuffer();

  for (int x = x1c; x <= x2c; x++)
  {
    const bool spass   = (sbuff == nullptr) ? true : (sbuff[offset + x] & currMask) != (currVal & currMask); // curr stencil func here ...
    const PsRet psData = CheckPixelCoverage((float)x, t, t.k1, k2);

    if (psData.overlaps && psData.zInv > zbuff[offset + x] && spass)
    {
      float4 color = psData.w0*t.c1 + psData.w1*t.c2 + psData.w2*t.c3;

    #ifdef PERSP_CORRECT
      float z = 1.0f / psData.zInv;
      color *= z;
    #endif

      frameBuff->data[offset + x] = RealColorToUint32_BGRA(color);
      zbuff[offset + x]           = psData.zInv;
    }
  }
}

#ifdef ENABLE_SSE

void DrawSpan_Colored3D_SSE(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const TriangleDataYesSSE& t, const __m128 k2v)
{
  uint8_t* sbuff = frameBuff->getSBuffer();
  float* zbuff   = frameBuff->getZBuffer();

  const uint8_t currVal  = t.curr_sval;
  const uint8_t currMask = t.curr_smask;

  for (int x = x1c; x <= x2c; x++)
  {
    const bool spass = (sbuff == nullptr) ? true : (sbuff[offset + x] & currMask) != (currVal & currMask); // curr stencil func here ...
    PsRetSSE psData  = CheckPixelCoverageSSE((float)x, t, t.kv1, k2v);

    const __m128 zBuffVal = _mm_load_ss(zbuff + offset + x);
    const __m128 cmpRes = _mm_cmpgt_ss(psData.zInv, zBuffVal);

    if (psData.overlaps && (_mm_movemask_ps(cmpRes) & 1) && spass) // psData.overlaps &&
    {
      const __m128 cc1 = _mm_mul_ps(t.cv1, _mm_shuffle_ps(psData.w, psData.w, _MM_SHUFFLE(0, 0, 0, 0)));
      const __m128 cc2 = _mm_mul_ps(t.cv2, _mm_shuffle_ps(psData.w, psData.w, _MM_SHUFFLE(1, 1, 1, 1)));
      const __m128 cc3 = _mm_mul_ps(t.cv3, _mm_shuffle_ps(psData.w, psData.w, _MM_SHUFFLE(2, 2, 2, 2)));

      __m128 clr = _mm_add_ps(cc1, _mm_add_ps(cc2, cc3));

    #ifdef PERSP_CORRECT

      const __m128 z1 = _mm_rcp_ss(psData.zInv);
      const __m128 z  = _mm_shuffle_ps(z1, z1, _MM_SHUFFLE(0, 0, 0, 0));

      clr = _mm_mul_ps(clr, z);
    #endif

      frameBuff->data[offset + x] = RealColorToUint32_BGRA(clr);
      _mm_store_ss(zbuff + offset + x, psData.zInv);
    }

  }

}

#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawSpan_TexLinear2D(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const TriangleDataNoSSE& t, const float3 k2)
{
  uint8_t* sbuff         = frameBuff->getSBuffer();
  const uint8_t currVal  = t.curr_sval;
  const uint8_t currMask = t.curr_smask;

  for (int x = x1c; x <= x2c; x++)
  {
    const bool spass = (sbuff == nullptr) ? true : (sbuff[offset + x] & currMask) != (currVal & currMask); // curr stencil func here ...
    PsRet psData     = CheckPixelCoverage((float)x, t, t.k1, k2);

    if (psData.overlaps && spass)
    {
      float4 color    = psData.w0*t.c1 + psData.w1*t.c2 + psData.w2*t.c3;
      float2 texCoord = psData.w0*t.t1 + psData.w1*t.t2 + psData.w2*t.t3;
      float4 texColor = tex2D(t.texS, texCoord);

      frameBuff->data[offset + x] = RealColorToUint32_BGRA(color*texColor);
    }
  }

}

#ifdef ENABLE_SSE

void DrawSpan_TexLinear2D_SSE(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const TriangleDataYesSSE& t, const __m128 k2v)
{
  uint8_t* sbuff         = frameBuff->getSBuffer();
  const uint8_t currVal  = t.curr_sval;
  const uint8_t currMask = t.curr_smask;

  float* zbuff = frameBuff->getZBuffer();

  for (int x = x1c; x <= x2c; x++)
  {
    const bool spass = (sbuff == nullptr) ? true : (sbuff[offset + x] & currMask) != (currVal & currMask); // curr stencil func here ...
    PsRetSSE psData  = CheckPixelCoverageSSE((float)x, t, t.kv1, k2v);

    if (psData.overlaps && spass)
    {
      const __m128 w0  = _mm_shuffle_ps(psData.w, psData.w, _MM_SHUFFLE(0, 0, 0, 0));
      const __m128 w1  = _mm_shuffle_ps(psData.w, psData.w, _MM_SHUFFLE(1, 1, 1, 1));
      const __m128 w2  = _mm_shuffle_ps(psData.w, psData.w, _MM_SHUFFLE(2, 2, 2, 2));

      const __m128 cc1 = _mm_mul_ps(t.cv1, w0);
      const __m128 cc2 = _mm_mul_ps(t.cv2, w1);
      const __m128 cc3 = _mm_mul_ps(t.cv3, w2);

      const __m128 t1  = _mm_mul_ps(t.tv1, w0);
      const __m128 t2  = _mm_mul_ps(t.tv2, w1);
      const __m128 t3  = _mm_mul_ps(t.tv3, w2);

      __m128 clr = _mm_add_ps(cc1, _mm_add_ps(cc2, cc3));
      __m128 tc  = _mm_add_ps(t1,  _mm_add_ps(t2, t3));

      const __m128 texColor = tex2D_sse(t.texS, tc);

      frameBuff->data[offset + x] = RealColorToUint32_BGRA(_mm_mul_ps(clr, texColor));
    }

  }
}

#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawSpan_TexLinear3D(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const TriangleDataNoSSE& t, const float3 k2)
{
  uint8_t* sbuff = frameBuff->getSBuffer();
  float* zbuff   = frameBuff->getZBuffer();

  const uint8_t currVal  = t.curr_sval;
  const uint8_t currMask = t.curr_smask;

  for (int x = x1c; x <= x2c; x++)
  {
    const bool spass = (sbuff == nullptr) ? true : (sbuff[offset + x] & currMask) != (currVal & currMask); // curr stencil func here ...
    PsRet psData     = CheckPixelCoverage((float)x, t, t.k1, k2);

    if (psData.overlaps && psData.zInv > zbuff[offset + x] && spass)
    {
      float4 color    = psData.w0*t.c1 + psData.w1*t.c2 + psData.w2*t.c3;
      float2 texCoord = psData.w0*t.t1 + psData.w1*t.t2 + psData.w2*t.t3;

    #ifdef PERSP_CORRECT
      const float z = 1.0f / psData.zInv;
      color    *= z;
      texCoord *= z;
    #endif

      float4 texColor = tex2D(t.texS, texCoord);

      frameBuff->data[offset + x] = RealColorToUint32_BGRA(color*texColor);
      zbuff[offset + x]           = psData.zInv;
    }
  }

}

#ifdef ENABLE_SSE

void DrawSpan_TexLinear3D_SSE(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const TriangleDataYesSSE& t, const __m128 k2v)
{
  uint8_t* sbuff = frameBuff->getSBuffer();
  float*   zbuff = frameBuff->getZBuffer();

  const uint8_t currVal  = t.curr_sval;
  const uint8_t currMask = t.curr_smask;

  for (int x = x1c; x <= x2c; x++)
  {
    const bool spass = (sbuff == nullptr) ? true : (sbuff[offset + x] & currMask) != (currVal & currMask); // curr stencil func here ...
    PsRetSSE psData  = CheckPixelCoverageSSE((float)x, t, t.kv1, k2v);

    const __m128 zBuffVal = _mm_load_ss(zbuff + offset + x);
    const __m128 cmpRes   = _mm_cmpgt_ss(psData.zInv, zBuffVal);

    if (psData.overlaps && (_mm_movemask_ps(cmpRes) & 1) && spass)
    {
      const __m128 w0  = _mm_shuffle_ps(psData.w, psData.w, _MM_SHUFFLE(0, 0, 0, 0));
      const __m128 w1  = _mm_shuffle_ps(psData.w, psData.w, _MM_SHUFFLE(1, 1, 1, 1));
      const __m128 w2  = _mm_shuffle_ps(psData.w, psData.w, _MM_SHUFFLE(2, 2, 2, 2));

      const __m128 cc1 = _mm_mul_ps(t.cv1, w0);
      const __m128 cc2 = _mm_mul_ps(t.cv2, w1);
      const __m128 cc3 = _mm_mul_ps(t.cv3, w2);

      const __m128 t1  = _mm_mul_ps(t.tv1, w0);
      const __m128 t2  = _mm_mul_ps(t.tv2, w1);
      const __m128 t3  = _mm_mul_ps(t.tv3, w2);

      __m128 clr = _mm_add_ps(cc1, _mm_add_ps(cc2, cc3));
      __m128 tc  = _mm_add_ps(t1,  _mm_add_ps(t2, t3));

    #ifdef PERSP_CORRECT

      const __m128 z1 = _mm_rcp_ss(psData.zInv);
      const __m128 z  = _mm_shuffle_ps(z1, z1, _MM_SHUFFLE(0, 0, 0, 0));

      clr = _mm_mul_ps(clr, z);
      tc  = _mm_mul_ps(tc, z);

    #endif

      const __m128 texColor = tex2D_sse(t.texS, tc);

      frameBuff->data[offset + x] = RealColorToUint32_BGRA(_mm_mul_ps(clr, texColor));
      _mm_store_ss(zbuff + offset + x, psData.zInv);
    }

  }
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawSpan_Colored2D_Blend(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const TriangleDataNoSSE& t, const float3 k2)
{
  uint8_t* sbuff         = frameBuff->getSBuffer();
  const uint8_t currVal  = t.curr_sval;
  const uint8_t currMask = t.curr_smask;

  for (int x = x1c; x <= x2c; x++)
  {
    const bool spass = (sbuff == nullptr) ? true : (sbuff[offset + x] & currMask) != (currVal & currMask); // curr stencil func here ...
    PsRet psData     = CheckPixelCoverage((float)x, t, t.k1, k2);

    if (psData.overlaps && spass)
    {
      const float4 oldColor = Uint32_BGRAToRealColor(frameBuff->data[offset + x]);
      const float4 color    = psData.w0*t.c1 + psData.w1*t.c2 + psData.w2*t.c3;
      const float  alpha    = color.w;

      frameBuff->data[offset + x] = RealColorToUint32_BGRA(oldColor*(1.0f - alpha) + alpha*color);
    }
  }
}

#ifdef ENABLE_SSE

void DrawSpan_Colored2D_Blend_SSE(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const TriangleDataYesSSE& t, const __m128 k2v)
{
  uint8_t* sbuff         = frameBuff->getSBuffer();
  const uint8_t currVal  = t.curr_sval;
  const uint8_t currMask = t.curr_smask;

  for (int x = x1c; x <= x2c; x++)
  {
    const bool spass = (sbuff == nullptr) ? true : (sbuff[offset + x] & currMask) != (currVal & currMask); // curr stencil func here ...
    PsRetSSE psData  = CheckPixelCoverageSSE((float)x, t, t.kv1, k2v);

    if (psData.overlaps && spass)
    {
      const __m128 oldColor = Uint32_BGRAToRealColor_SSE(frameBuff->data, offset + x);

      const __m128 w0  = _mm_shuffle_ps(psData.w, psData.w, _MM_SHUFFLE(0, 0, 0, 0));
      const __m128 w1  = _mm_shuffle_ps(psData.w, psData.w, _MM_SHUFFLE(1, 1, 1, 1));
      const __m128 w2  = _mm_shuffle_ps(psData.w, psData.w, _MM_SHUFFLE(2, 2, 2, 2));

      const __m128 cc1 = _mm_mul_ps(t.cv1, w0);
      const __m128 cc2 = _mm_mul_ps(t.cv2, w1);
      const __m128 cc3 = _mm_mul_ps(t.cv3, w2);

      const __m128 resColor = _mm_add_ps(cc1, _mm_add_ps(cc2, cc3));
      const __m128 alpha    = _mm_shuffle_ps(resColor, resColor, _MM_SHUFFLE(3, 3, 3, 3));
      const __m128 finColor = _mm_add_ps(_mm_mul_ps(resColor, alpha), _mm_mul_ps(oldColor, _mm_sub_ps(const_1111, alpha)));

      frameBuff->data[offset + x] = RealColorToUint32_BGRA(finColor);
    }
  }
}

#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawSpan_TexLinear2D_Blend(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const TriangleDataNoSSE& t, const float3 k2)
{
  uint8_t* sbuff         = frameBuff->getSBuffer();
  const uint8_t currVal  = t.curr_sval;
  const uint8_t currMask = t.curr_smask;

  for (int x = x1c; x <= x2c; x++)
  {
    const bool spass = (sbuff == nullptr) ? true : (sbuff[offset + x] & currMask) != (currVal & currMask); // curr stencil func here ...
    PsRet psData     = CheckPixelCoverage((float)x, t, t.k1, k2);

    if (psData.overlaps && spass)
    {
      const float4 oldColor = Uint32_BGRAToRealColor(frameBuff->data[offset + x]);

      const float4 color    = psData.w0*t.c1 + psData.w1*t.c2 + psData.w2*t.c3;
      const float2 texCoord = psData.w0*t.t1 + psData.w1*t.t2 + psData.w2*t.t3;
      const float4 texColor = tex2D(t.texS, texCoord);

      const float alpha = color.w*texColor.w;

      frameBuff->data[offset + x] = RealColorToUint32_BGRA(oldColor*(1.0f - alpha) + alpha*color*texColor);
    }
  }
}

#ifdef ENABLE_SSE

void DrawSpan_TexLinear2D_Blend_SSE(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const TriangleDataYesSSE& t, const __m128 k2v)
{
  uint8_t* sbuff         = frameBuff->getSBuffer();
  const uint8_t currVal  = t.curr_sval;
  const uint8_t currMask = t.curr_smask;

  for (int x = x1c; x <= x2c; x++)
  {
    const bool spass = (sbuff == nullptr) ? true : (sbuff[offset + x] & currMask) != (currVal & currMask); // curr stencil func here ...
    PsRetSSE psData  = CheckPixelCoverageSSE((float)x, t, t.kv1, k2v);

    if (psData.overlaps && spass)
    {
      const __m128 oldColor = Uint32_BGRAToRealColor_SSE(frameBuff->data, offset + x);

      const __m128 w0  = _mm_shuffle_ps(psData.w, psData.w, _MM_SHUFFLE(0, 0, 0, 0));
      const __m128 w1  = _mm_shuffle_ps(psData.w, psData.w, _MM_SHUFFLE(1, 1, 1, 1));
      const __m128 w2  = _mm_shuffle_ps(psData.w, psData.w, _MM_SHUFFLE(2, 2, 2, 2));

      const __m128 cc1 = _mm_mul_ps(t.cv1, w0);
      const __m128 cc2 = _mm_mul_ps(t.cv2, w1);
      const __m128 cc3 = _mm_mul_ps(t.cv3, w2);
      const __m128 clr = _mm_add_ps(cc1, _mm_add_ps(cc2, cc3));

      const __m128 t1  = _mm_mul_ps(t.tv1, w0);
      const __m128 t2  = _mm_mul_ps(t.tv2, w1);
      const __m128 t3  = _mm_mul_ps(t.tv3, w2);
      const __m128 tc  = _mm_add_ps(t1, _mm_add_ps(t2, t3));

      const __m128 texColor = tex2D_sse(t.texS, tc);
      const __m128 resColor = _mm_mul_ps(clr, texColor);

      const __m128 alpha    = _mm_shuffle_ps(resColor, resColor, _MM_SHUFFLE(3, 3, 3, 3));
      const __m128 finColor = _mm_add_ps(_mm_mul_ps(resColor, alpha), _mm_mul_ps(oldColor, _mm_sub_ps(const_1111, alpha)));

      frameBuff->data[offset + x] = RealColorToUint32_BGRA(finColor);
    }
  }

}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawSpan_TexLinear3D_Blend(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const TriangleDataNoSSE& t, const float3 k2)
{
  uint8_t* sbuff         = frameBuff->getSBuffer();
  const uint8_t currVal  = t.curr_sval;
  const uint8_t currMask = t.curr_smask;

  float* zbuff = frameBuff->getZBuffer();

  for (int x = x1c; x <= x2c; x++)
  {
    const bool spass = (sbuff == nullptr) ? true : (sbuff[offset + x] & currMask) != (currVal & currMask); // curr stencil func here ...
    PsRet psData     = CheckPixelCoverage((float)x, t, t.k1, k2);

    if (psData.overlaps && psData.zInv > zbuff[offset + x] && spass)
    {
      float4 oldColor = Uint32_BGRAToRealColor(frameBuff->data[offset + x]);
      float4 color    = psData.w0*t.c1 + psData.w1*t.c2 + psData.w2*t.c3;
      float2 texCoord = psData.w0*t.t1 + psData.w1*t.t2 + psData.w2*t.t3;

    #ifdef PERSP_CORRECT

      const float z = 1.0f / psData.zInv;
      color    *= z;
      texCoord *= z;

    #endif

      const float4 texColor = tex2D(t.texS, texCoord);
      const float  alpha    = color.w*texColor.w;

      frameBuff->data[offset + x] = RealColorToUint32_BGRA(oldColor*(1.0f - alpha) + alpha*color*texColor);
      zbuff[offset + x]           = psData.zInv;
    }
  }
}


#ifdef ENABLE_SSE

void DrawSpan_TexLinear3D_Blend_SSE(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const TriangleDataYesSSE& t, const __m128 k2v)
{
  uint8_t* sbuff         = frameBuff->getSBuffer();
  const uint8_t currVal  = t.curr_sval;
  const uint8_t currMask = t.curr_smask;

  float* zbuff = frameBuff->getZBuffer();

  for (int x = x1c; x <= x2c; x++)
  {
    const bool spass = (sbuff == nullptr) ? true : (sbuff[offset + x] & currMask) != (currVal & currMask); // curr stencil func here ...
    PsRetSSE psData  = CheckPixelCoverageSSE((float)x, t, t.kv1, k2v);

    const __m128 zBuffVal = _mm_load_ss(zbuff + offset + x);
    const __m128 cmpRes   = _mm_cmpgt_ss(psData.zInv, zBuffVal);

    if (psData.overlaps && (_mm_movemask_ps(cmpRes) & 1) && spass)
    {
      const __m128 oldColor = Uint32_BGRAToRealColor_SSE(frameBuff->data, offset + x);

      const __m128 w0  = _mm_shuffle_ps(psData.w, psData.w, _MM_SHUFFLE(0, 0, 0, 0));
      const __m128 w1  = _mm_shuffle_ps(psData.w, psData.w, _MM_SHUFFLE(1, 1, 1, 1));
      const __m128 w2  = _mm_shuffle_ps(psData.w, psData.w, _MM_SHUFFLE(2, 2, 2, 2));

      const __m128 cc1 = _mm_mul_ps(t.cv1, w0);
      const __m128 cc2 = _mm_mul_ps(t.cv2, w1);
      const __m128 cc3 = _mm_mul_ps(t.cv3, w2);
      __m128 clr = _mm_add_ps(cc1, _mm_add_ps(cc2, cc3));

      const __m128 t1  = _mm_mul_ps(t.tv1, w0);
      const __m128 t2  = _mm_mul_ps(t.tv2, w1);
      const __m128 t3  = _mm_mul_ps(t.tv3, w2);
      __m128 tc = _mm_add_ps(t1, _mm_add_ps(t2, t3));

    #ifdef PERSP_CORRECT

      const __m128 z1 = _mm_rcp_ss(psData.zInv);
      const __m128 z  = _mm_shuffle_ps(z1, z1, _MM_SHUFFLE(0, 0, 0, 0));

      clr = _mm_mul_ps(clr, z);
      tc  = _mm_mul_ps(tc, z);

    #endif

      const __m128 texColor = tex2D_sse(t.texS, tc);
      const __m128 resColor = _mm_mul_ps(clr, texColor);

      const __m128 alpha    = _mm_shuffle_ps(resColor, resColor, _MM_SHUFFLE(3, 3, 3, 3));
      const __m128 finColor = _mm_add_ps(_mm_mul_ps(resColor, alpha), _mm_mul_ps(oldColor, _mm_sub_ps(const_1111, alpha)));

      frameBuff->data[offset + x] = RealColorToUint32_BGRA(finColor);
      _mm_store_ss(zbuff + offset + x, psData.zInv);
    }
  }
}

#endif



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawSpan_NoColorStencilAlwaysReplace(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const TriangleDataNoSSE& t, const float3 k2) // pre (frameBuff != nullptr)
{
  uint8_t* sbuff        = frameBuff->getSBuffer();
  const uint8_t currVal = t.curr_sval; // (frameBuff->curr_sval & frameBuff->curr_smask);

  for (int x = x1c; x <= x2c; x++)
  {
    PsRet psData = CheckPixelCoverage((float)x, t, t.k1, k2);
    if (psData.overlaps)
      sbuff[offset + x] = currVal;
  }
}


#ifdef ENABLE_SSE

void DrawSpan_NoColorStencilAlwaysReplace_SSE(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const TriangleDataYesSSE& t, const __m128 k2v)
{
  uint8_t* sbuff        = frameBuff->getSBuffer();
  const uint8_t currVal = t.curr_sval; // (frameBuff->curr_sval & frameBuff->curr_smask);

  for (int x = x1c; x <= x2c; x++)
  {
    PsRetSSE psData = CheckPixelCoverageSSE((float)x, t, t.kv1, k2v);
    if (psData.overlaps)
      sbuff[offset + x] = currVal;
  }
}


#endif
