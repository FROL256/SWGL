//
// Created by frol on 14.10.18.
//

#ifndef TEST_GL_TOP_VFLOAT4_H
#define TEST_GL_TOP_VFLOAT4_H

#include <xmmintrin.h> // SSE
#include <emmintrin.h> // SSE2
#include <smmintrin.h> // SSE4.1 //#TODO: optionally

typedef __m128   vfloat4;
typedef __m128i  vint4;

#if __GNUC__
#define ALIGN(X) __attribute__((__aligned__(X)))
#elif _MSC_VER
#define ALIGN(X) __declspec(align(X))
#else
#error "Unsupported compiler"
#endif

static inline void store(float* data, vfloat4 a_val)   { _mm_store_ps(data,a_val); }
static inline void store_u(float* data, vfloat4 a_val) { _mm_storeu_ps(data,a_val); }

static inline vfloat4 load(float* data)   { return _mm_load_ps(data); }
static inline vfloat4 load_u(float* data) { return _mm_loadu_ps(data); }


static inline void stream(void* data, vint4 a_val) { _mm_stream_si128((vint4*)data, a_val); }

static inline vint4 splat_1to4(const int i) { return _mm_set_epi32(i,i,i,i); }

static inline vfloat4 splat_0(const vfloat4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0)); }
static inline vfloat4 splat_1(const vfloat4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1)); }
static inline vfloat4 splat_2(const vfloat4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2)); }
static inline vfloat4 splat_3(const vfloat4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3)); }

static inline vint4 make_vint4(const int a, const int b, const int c, const int d) { return _mm_set_epi32(d,c,b,a); }

static inline vfloat4 as_vfloat(const vint4 a_val) { return _mm_castsi128_ps(a_val); }


static inline vfloat4 blend(const vfloat4 a, const vfloat4 b, const vint4 mask)
{
  return _mm_or_ps(_mm_and_ps   (as_vfloat(mask), a),
                   _mm_andnot_ps(as_vfloat(mask), b));
}


inline static int RealColorToUint32_BGRA_SIMD(const vfloat4 rel_col)
{
  static const vfloat4 const_255 = {255.0f, 255.0f, 255.0f, 255.0f};

  const __m128i rgba = _mm_cvtps_epi32(_mm_mul_ps(rel_col, const_255));
  const __m128i out  = _mm_packus_epi32(rgba, _mm_setzero_si128());
  const __m128i out2 = _mm_packus_epi16(out,  _mm_setzero_si128());

  return _mm_cvtsi128_si32(out2);
}

#endif //TEST_GL_TOP_VFLOAT4_H
