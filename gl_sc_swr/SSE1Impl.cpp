#include "SSE1Impl.h"

#include <cstdint>

// #NOTE: _mm_maskmoveu_si128

void HWImpl_SSE1::memset32(int32_t* a_data, int32_t a_val, int32_t numElements)
{
  uintptr_t ip = reinterpret_cast<uintptr_t>(a_data);

  if ((numElements % 32 == 0) && (ip%16 == 0))
  {
    //__m128i val = _mm_set_epi32(a_val, a_val, a_val, a_val);
    __m128i val = _mm_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);

    __m128i* color128  = (__m128i*)(a_data);
    const int size     = numElements/4;

    for (int i = 0; i < size; i += 8)
    {
      _mm_stream_si128(color128 + i + 0, val); // _mm_store_si128
      _mm_stream_si128(color128 + i + 1, val);
      _mm_stream_si128(color128 + i + 2, val);
      _mm_stream_si128(color128 + i + 3, val);
      _mm_stream_si128(color128 + i + 4, val);
      _mm_stream_si128(color128 + i + 5, val);
      _mm_stream_si128(color128 + i + 6, val);
      _mm_stream_si128(color128 + i + 7, val);
    }
  }
  else
  {
    for (int i = 0; i < numElements; i++)
      a_data[i] = a_val;
  }
}


bool HWImpl_SSE1::AABBTriangleOverlap(const TriangleType& a_tri, const int tileMinX, const int tileMinY, const int tileMaxX, const int tileMaxY)
{
  const bool overlapBoxBox = IntersectBoxBox(int2(a_tri.bb_iminX, a_tri.bb_iminY), int2(a_tri.bb_imaxX, a_tri.bb_imaxY),
                                             int2(tileMinX, tileMinY),             int2(tileMaxX, tileMaxY));

  return overlapBoxBox;
}


void HWImpl_SSE1::VertexShader(const float* v_in4f, float* v_out4f, int a_numVert,
                               const float viewportData[4], const float worldViewProjMatrix[16])
{

}

void HWImpl_SSE1::TriangleSetUp(const SWGL_Context* a_pContext, const Batch* pBatch, int i1, int i2, int i3,
                                TriangleType* t1)
{

}

void HWImpl_SSE1::RasterizeTriangle(ROP_TYPE a_ropT, const TriangleType& tri, int tileMinX, int tileMinY,
                                    FrameBuffer* frameBuf)
{

}