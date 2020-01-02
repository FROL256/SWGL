//
// Created by Vladimir Frolov on 14.10.18.
//
#ifndef TEST_GL_TOP_VFLOAT8_H
#define TEST_GL_TOP_VFLOAT8_H

//#include "vfloat4_x64.h"

#include  <immintrin.h>

namespace cvex8
{
  typedef __m256  vfloat8;
  typedef __m256i vint8;

  static inline void set_ftz() { _MM_SET_ROUNDING_MODE(_MM_ROUND_TOWARD_ZERO);}

  static inline void store(float* data, vfloat8 a_val)   { _mm256_store_ps(data, a_val);  }
  static inline void store(int*   data, vint8 a_val)     { _mm256_store_ps((float*)data, _mm256_castsi256_ps(a_val));  }

  static inline void store_u(float* data, vfloat8 a_val) { _mm256_storeu_ps(data, a_val); }
  static inline void store_u(int*   data,  vint8 a_val)  { _mm256_storeu_ps((float*)data, _mm256_castsi256_ps(a_val)); }

  static inline auto load  (const float *data) -> vfloat8 { return _mm256_load_ps(data);  }
  static inline auto load  (const int *data)   -> vint8   { return _mm256_castps_si256(_mm256_load_ps((float*)data));  }

  static inline auto load_u(const float *data) -> vfloat8 { return _mm256_loadu_ps(data); }
  static inline auto load_u(const int *data)   -> vint8   { return _mm256_castps_si256(_mm256_loadu_ps((float*)data)); }

  static inline auto splat(const int i)          -> vint8   { return _mm256_castps_si256(_mm256_broadcast_ss((float*)&i)); }
  static inline auto splat(const unsigned int i) -> vint8   { return _mm256_castps_si256(_mm256_broadcast_ss((float*)&i)); }
  static inline auto splat(const float i)        -> vfloat8 { return _mm256_broadcast_ss(&i); }

  static inline vint8   to_int32(const vfloat8 a_val) { return _mm256_cvtps_epi32(a_val);}
  static inline vfloat8 to_float32(const vint8 a_val) { return _mm256_cvtepi32_ps(a_val);}

  static inline vint8 make_vint(const int a, const int b, const int c, const int d,
                                const int e, const int f, const int g, const int h) { return _mm256_set_epi32(h, g, f, e, d, c, b, a); }

  static inline vfloat8 as_vfloat(const vint8 a_val) { return _mm256_castsi256_ps(a_val); }
  static inline vint8   as_vint(const vfloat8 a_val) { return _mm256_castps_si256(a_val); }

  static inline vfloat8 rcp_e(const vfloat8 a) { return _mm256_rcp_ps(a); }

  static inline vfloat8 blend(const vfloat8 a, const vfloat8 b, const vint8 mask)
  {
    return _mm256_or_ps(_mm256_and_ps(   _mm256_castsi256_ps(mask), a),
                        _mm256_andnot_ps(_mm256_castsi256_ps(mask), b));
  }

  static inline vint8 blend(const vint8 a, const vint8 b, const vint8 mask)
  {
    return _mm256_castps_si256(_mm256_or_ps(_mm256_and_ps(   _mm256_castsi256_ps(mask), _mm256_castsi256_ps(a) ),
                               _mm256_andnot_ps(_mm256_castsi256_ps(mask), _mm256_castsi256_ps(b) )));
  }

  static inline bool test_bits_any(const vint8 a) { return (_mm256_movemask_ps(_mm256_castsi256_ps(a)) & 15) != 0; }

  static inline vfloat8 clamp(const vfloat8 x, const vfloat8 minVal, const vfloat8 maxVal) { return _mm256_max_ps(_mm256_min_ps(x, maxVal), minVal); }

  static inline void prefetch(const float* ptr) {  _mm_prefetch((const char*)ptr, _MM_HINT_T0); }
  static inline void prefetch(const int* ptr)   {  _mm_prefetch((const char*)ptr, _MM_HINT_T0); }
};


#ifdef WIN32 // MVSC does not define operators !!!

static inline cvex8::vfloat8 operator+(const cvex8::vfloat8 a, const cvex8::vfloat8 b) { return _mm256_add_ps(a, b); }
static inline cvex8::vfloat8 operator-(const cvex8::vfloat8 a, const cvex8::vfloat8 b) { return _mm256_sub_ps(a, b); }
static inline cvex8::vfloat8 operator*(const cvex8::vfloat8 a, const cvex8::vfloat8 b) { return _mm256_mul_ps(a, b); }
static inline cvex8::vfloat8 operator/(const cvex8::vfloat8 a, const cvex8::vfloat8 b) { return _mm256_div_ps(a, b); }

static inline cvex8::vfloat8 operator+(const cvex8::vfloat8 a, const float b) { return _mm256_add_ps(a, _mm256_broadcast_ss(&b)); }
static inline cvex8::vfloat8 operator-(const cvex8::vfloat8 a, const float b) { return _mm256_sub_ps(a, _mm256_broadcast_ss(&b)); }
static inline cvex8::vfloat8 operator*(const cvex8::vfloat8 a, const float b) { return _mm256_mul_ps(a, _mm256_broadcast_ss(&b)); }
static inline cvex8::vfloat8 operator/(const cvex8::vfloat8 a, const float b) { return _mm256_div_ps(a, _mm256_broadcast_ss(&b)); }

static inline cvex8::vfloat8 operator+(const float a, const cvex8::vfloat8 b) { return _mm256_add_ps(_mm256_broadcast_ss(&a), b); }
static inline cvex8::vfloat8 operator-(const float a, const cvex8::vfloat8 b) { return _mm256_sub_ps(_mm256_broadcast_ss(&a), b); }
static inline cvex8::vfloat8 operator*(const float a, const cvex8::vfloat8 b) { return _mm256_mul_ps(_mm256_broadcast_ss(&a), b); }
static inline cvex8::vfloat8 operator/(const float a, const cvex8::vfloat8 b) { return _mm256_div_ps(_mm256_broadcast_ss(&a), b); }

static inline cvex8::vint8 operator+(const cvex8::vint8 a, const cvex8::vint8 b) { return _mm256_add_epi32(a, b); }
static inline cvex8::vint8 operator-(const cvex8::vint8 a, const cvex8::vint8 b) { return _mm256_sub_epi32(a, b); }
static inline cvex8::vint8 operator*(const cvex8::vint8 a, const cvex8::vint8 b) { return _mm256_mullo_epi32(a, b); }

static inline cvex8::vint8 operator+(const cvex8::vint8 a, const int b) { return _mm256_add_epi32  (a, cvex8::splat(b)); }
static inline cvex8::vint8 operator-(const cvex8::vint8 a, const int b) { return _mm256_sub_epi32  (a, cvex8::splat(b)); }
static inline cvex8::vint8 operator*(const cvex8::vint8 a, const int b) { return _mm256_mullo_epi32(a, cvex8::splat(b)); }

static inline cvex8::vint8 operator+(const int a, const cvex8::vint8 b) { return _mm256_add_epi32  (cvex8::splat(a), b); }
static inline cvex8::vint8 operator-(const int a, const cvex8::vint8 b) { return _mm256_sub_epi32  (cvex8::splat(a), b); }
static inline cvex8::vint8 operator*(const int a, const cvex8::vint8 b) { return _mm256_mullo_epi32(cvex8::splat(a), b); }

static inline cvex8::vint8 operator<<(const cvex8::vint8 a, const int val) { return _mm256_slli_epi32(a, val); }
static inline cvex8::vint8 operator>>(const cvex8::vint8 a, const int val) { return _mm256_srli_epi32(a, val); }

static inline cvex8::vint8 operator|(const cvex8::vint8 a, const cvex::vint8 b) { return _mm256_or_si256(a, b); }
static inline cvex8::vint8 operator&(const cvex8::vint8 a, const cvex::vint8 b) { return _mm256_and_si256(a, b); }

static inline cvex8::vint8 operator> (const cvex8::vfloat8 a, const cvex8::vfloat8 b) { return cvex8::as_vint(_mm256_cmp_ps(a, b, _CMP_GT_OQ)); }
static inline cvex8::vint8 operator< (const cvex8::vfloat8 a, const cvex8::vfloat8 b) { return cvex8::as_vint(_mm256_cmp_ps(a, b, _CMP_LT_OQ)); }
static inline cvex8::vint8 operator>=(const cvex8::vfloat8 a, const cvex8::vfloat8 b) { return cvex8::as_vint(_mm256_cmp_ps(a, b, _CMP_GE_OQ)); }
static inline cvex8::vint8 operator<=(const cvex8::vfloat8 a, const cvex8::vfloat8 b) { return cvex8::as_vint(_mm256_cmp_ps(a, b, _CMP_GE_OQ)); }

#endif


#endif //TEST_GL_TOP_VFLOAT8_H
