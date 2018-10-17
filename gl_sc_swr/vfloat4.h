//
// Created by Vladimir Frolov on 14.10.18.
//
// This library is created as lightweight and more optimised replacement for famous libsimdpp
// In the case if no implementation for tarhet architecture presents, you should include
// file (vfloat4_simdpp.h) that defines all operations and types via libsimdpp ...
// so it can work ok different platforms anyway

#ifndef TEST_GL_TOP_VFLOAT4_H
#define TEST_GL_TOP_VFLOAT4_H

#include <xmmintrin.h> // SSE
#include <emmintrin.h> // SSE2
#include <smmintrin.h> // SSE4.1 //#TODO: optionally

#if __GNUC__
#define ALIGN(X) __attribute__((__aligned__(X)))
#elif _MSC_VER
#define ALIGN(X) __declspec(align(X))
#else
#error "Unsupported compiler"
#endif

namespace cvex
{
  typedef __m128  vfloat4;
  typedef __m128i vint4;

  static inline void set_ftz() { _MM_SET_ROUNDING_MODE(_MM_ROUND_TOWARD_ZERO);}

  static inline void store(float *data, vfloat4 a_val)   { _mm_store_ps(data, a_val);  }
  static inline void store_u(float *data, vfloat4 a_val) { _mm_storeu_ps(data, a_val); }
  static inline void store_s(float *data, vfloat4 a_val) { _mm_store_ss(data, a_val);  } // store single ...

  static inline vfloat4 load(float *data)   { return _mm_load_ps(data);  }
  static inline vfloat4 load_u(float *data) { return _mm_loadu_ps(data); }
  static inline vfloat4 load_s(float *data) { return _mm_load_ss(data);  }

  // due to _mm_***_ss is the x64 only feature, when using these functions you must guarantee that
  // only first vector component is used further. Other components are undefined!
  //
  static inline vfloat4 add_s(vfloat4 a, vfloat4 b) { return _mm_add_ss(a,b); } // #NOTE: assume you will never use .yzw coordinates!; only .x is valid!
  static inline vfloat4 sub_s(vfloat4 a, vfloat4 b) { return _mm_sub_ss(a,b); } // #NOTE: assume you will never use .yzw coordinates!; only .x is valid!
  static inline vfloat4 mul_s(vfloat4 a, vfloat4 b) { return _mm_mul_ss(a,b); } // #NOTE: assume you will never use .yzw coordinates!; only .x is valid!
  static inline vfloat4 div_s(vfloat4 a, vfloat4 b) { return _mm_div_ss(a,b); } // #NOTE: assume you will never use .yzw coordinates!; only .x is valid!
  static inline vfloat4 rcp_s(vfloat4 a)            { return _mm_rcp_ss(a);   } // #NOTE: assume you will never use .yzw coordinates!; only .x is valid!

  static inline int extract_0(const vint4 a_val)    { return _mm_cvtsi128_si32(a_val); }
  static inline int extract_1(const vint4 a_val)    { return _mm_cvtsi128_si32( _mm_shuffle_epi32(a_val, _MM_SHUFFLE(1,1,1,1)) ); }
  static inline int extract_2(const vint4 a_val)    { return _mm_cvtsi128_si32( _mm_shuffle_epi32(a_val, _MM_SHUFFLE(2,2,2,2)) ); }
  static inline int extract_3(const vint4 a_val)    { return _mm_cvtsi128_si32( _mm_shuffle_epi32(a_val, _MM_SHUFFLE(3,3,3,3)) ); }

  static inline void stream(void *data, vint4 a_val) { _mm_stream_si128((vint4 *) data, a_val); }

  static inline vint4 splat_1to4(const int i) { return _mm_set_epi32(i, i, i, i); }

  static inline vfloat4 splat_0(const vfloat4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0)); }
  static inline vfloat4 splat_1(const vfloat4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1)); }
  static inline vfloat4 splat_2(const vfloat4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2)); }
  static inline vfloat4 splat_3(const vfloat4 v) { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3)); }

  static inline vint4 make_vint4(const int a, const int b, const int c, const int d) { return _mm_set_epi32(d, c, b, a); }

  static inline vfloat4 as_vfloat(const vint4 a_val) { return _mm_castsi128_ps(a_val); }
  static inline vint4   as_vint(const vfloat4 a_val) { return _mm_castps_si128(a_val); }

  static inline vint4   to_vint(const vfloat4 a_val) { return _mm_cvtps_epi32(a_val);}
  static inline vfloat4 to_vfloat(const vint4 a_val) { return _mm_cvtepi32_ps(a_val);}

  static inline vfloat4 floor(const vfloat4 a_val) { return _mm_floor_ps(a_val); }

  static inline vfloat4 min(const vfloat4 a, const vfloat4 b) {return _mm_min_ps(a, b);}
  static inline vfloat4 max(const vfloat4 a, const vfloat4 b) {return _mm_max_ps(a, b);}

  static inline vfloat4 rcp_e(const vfloat4 a) { return _mm_rcp_ps(a); }

  static inline vfloat4 blend(const vfloat4 a, const vfloat4 b, const vint4 mask)
  {
    return _mm_or_ps(_mm_and_ps(as_vfloat(mask), a),
                     _mm_andnot_ps(as_vfloat(mask), b));
  }

  static inline void transpose4(vfloat4& a0, vfloat4& a1, vfloat4& a2, vfloat4& a3)
  {
    const vint4 b0 = _mm_unpacklo_epi32(as_vint(a0), as_vint(a1));
    const vint4 b1 = _mm_unpackhi_epi32(as_vint(a0), as_vint(a1));
    const vint4 b2 = _mm_unpacklo_epi32(as_vint(a2), as_vint(a3));
    const vint4 b3 = _mm_unpackhi_epi32(as_vint(a2), as_vint(a3));

    a0 = as_vfloat(_mm_unpacklo_epi64(b0, b2));
    a1 = as_vfloat(_mm_unpackhi_epi64(b0, b2));
    a2 = as_vfloat(_mm_unpacklo_epi64(b1, b3));
    a3 = as_vfloat(_mm_unpackhi_epi64(b1, b3));
  }

  inline static int RealColorToUint32_BGRA_SIMD(const vfloat4 rel_col)
  {
    static const vfloat4 const_255 = {255.0f, 255.0f, 255.0f, 255.0f};

    const __m128i rgba = _mm_cvtps_epi32(_mm_mul_ps(rel_col, const_255));
    const __m128i out  = _mm_packus_epi32(rgba, _mm_setzero_si128());
    const __m128i out2 = _mm_packus_epi16(out, _mm_setzero_si128());

    return _mm_cvtsi128_si32(out2);
  }

};

#endif //TEST_GL_TOP_VFLOAT4_H
