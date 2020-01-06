//
// Created by frol on 13.11.18.
//

#ifndef TEST_GL_TOP_VFLOAT8_GCC_H
#define TEST_GL_TOP_VFLOAT8_GCC_H

#ifdef __x86_64
  #include <immintrin.h>
#endif

#include "vfloat4_gcc.h"

namespace cvex8
{
  typedef unsigned           int _uint32_t;
  typedef unsigned long long int _uint64_t;
  typedef          long long int _sint64_t;

  typedef          int   vint8   __attribute__((vector_size(32)));
  typedef unsigned int   vuint8  __attribute__((vector_size(32)));
  typedef          float vfloat8 __attribute__((vector_size(32)));

  typedef          int   _vint8u   __attribute__((vector_size(32), aligned(1)));
  typedef unsigned int   _vuint8u  __attribute__((vector_size(32), aligned(1)));
  typedef          float _vfloat8u __attribute__((vector_size(32), aligned(1)));

  static inline vint8   load_u(const int* p)       { return *((_vint8u*)p);   }
  static inline vuint8  load_u(const _uint32_t* p) { return *((_vuint8u*)p);  }
  static inline vfloat8 load_u(const float* p)     { return *((_vfloat8u*)p); }

  static inline vint8   load  (const int* p)       { return *((vint8*)p);   }
  static inline vuint8  load  (const _uint32_t* p) { return *((vuint8*)p);  }
  static inline vfloat8 load  (const float* p)     { return *((vfloat8*)p); }

  static inline void store_u(int* p,       vint8   a_val)   { *((_vint8u*)(p))   = a_val; }
  static inline void store_u(_uint32_t* p, vuint8  a_val)   { *((_vuint8u*)(p))  = a_val; }
  static inline void store_u(float* p,     vfloat8 a_val)   { *((_vfloat8u*)(p)) = a_val; }

  static inline void store(int* p,       vint8   a_val)   { *((vint8*)(p))   = a_val; }
  static inline void store(_uint32_t* p, vuint8  a_val)   { *((vuint8*)(p))  = a_val; }
  static inline void store(float* p,     vfloat8 a_val)   { *((vfloat8*)(p)) = a_val; }

  static inline vuint8  splat(unsigned int x) { return vuint8  {x,x,x,x,x,x,x,x}; }
  static inline vint8   splat(int x)          { return vint8   {x,x,x,x,x,x,x,x}; }
  static inline vfloat8 splat(float x)        { return vfloat8 {x,x,x,x,x,x,x,x}; }

  static inline vfloat8 as_float32(const vint8 a_val)   { return reinterpret_cast<vfloat8>(a_val); }
  static inline vfloat8 as_float32(const vuint8 a_val)  { return reinterpret_cast<vfloat8>(a_val); }
  static inline vint8   as_int32  (const vfloat8 a_val) { return reinterpret_cast<vint8>(a_val);   }
  static inline vuint8  as_uint32 (const vfloat8 a_val) { return reinterpret_cast<vuint8>(a_val);  }

  static inline vfloat8 to_float32(vint8 a)    { return vfloat8{(float)a[0], (float)a[1], (float)a[2], (float)a[3], (float)a[4], (float)a[5], (float)a[6], (float)a[7]}; }
  static inline vfloat8 to_float32(vuint8 a)   { return vfloat8{(float)a[0], (float)a[1], (float)a[2], (float)a[3], (float)a[4], (float)a[5], (float)a[6], (float)a[7]}; }

  static inline vint8   to_int32  (vfloat8 a)  { return vint8  {  (int)a[0],   (int)a[1],   (int)a[2],   (int)a[3],   (int)a[4],   (int)a[5],   (int)a[6],   (int)a[7]}; }
  static inline vuint8  to_uint32 (vfloat8 a)  { return vuint8 {  (unsigned int)a[0], (unsigned int)a[1], (unsigned int)a[2], (unsigned int)a[3], (unsigned int)a[4], (unsigned int)a[5], (unsigned int)a[6], (unsigned int)a[7]}; }

  static inline vint8   to_int32  (vuint8 a)   { return (vint8)a; }
  static inline vuint8  to_uint32 (vint8 a)    { return (vuint8)a; }

  static inline vfloat8 blend(const vfloat8 a, const vfloat8 b, const vint8 mask)
  {
    const vint8 ia = reinterpret_cast<vint8>(a);
    const vint8 ib = reinterpret_cast<vint8>(b);
    return reinterpret_cast<vfloat8>((mask & ia) | (~mask & ib));
  }

  static inline vint8  blend(const vint8 a, const vint8 b, const vint8 mask)    { return ((mask & a) | (~mask & b)); }
  static inline vuint8 blend(const vuint8 a, const vuint8 b, const vuint8 mask) { return ((mask & a) | (~mask & b)); }
  static inline vint8  blend(const vuint8 a, const vint8 b, const vint8 mask)   { return ((mask & (vint8)a) | (~mask & b)); }

#ifdef __x86_64
  
  static inline void set_ftz() { _MM_SET_ROUNDING_MODE(_MM_ROUND_TOWARD_ZERO); }
  static inline vfloat8 rcp_e(vfloat8 a) { return _mm256_rcp_ps(a); }

  //static inline bool test_bits_any(const vfloat8 a) { return (_mm256_movemask_ps(a) & 15) != 0; }

  static inline vfloat8 floor(const vfloat8 a_val) { return _mm256_floor_ps(a_val); }
  static inline vfloat8 ceil (const vfloat8 a_val) { return _mm256_ceil_ps(a_val);  }
  static inline vfloat8 fabs (const vfloat8 a_val)
  {
    const __m256 absmask = _mm256_castsi256_ps(_mm256_set1_epi32((1<<31)));
    return _mm256_andnot_ps(absmask, a_val);
  }

  static inline bool cmp_gt(const vfloat8 a, const vfloat8 b) { return (_mm256_movemask_ps(_mm256_cmp_ps(a, b, _MM_CMPINT_GT))) == 255; }
  static inline bool cmp_lt(const vfloat8 a, const vfloat8 b) { return (_mm256_movemask_ps(_mm256_cmp_ps(a, b, _MM_CMPINT_LT))) == 255; }
  static inline bool cmp_ge(const vfloat8 a, const vfloat8 b) { return (_mm256_movemask_ps(_mm256_cmp_ps(a, b, _MM_CMPINT_GE))) == 255; }
  static inline bool cmp_le(const vfloat8 a, const vfloat8 b) { return (_mm256_movemask_ps(_mm256_cmp_ps(a, b, _MM_CMPINT_LE))) == 255; }
  static inline bool cmp_eq(const vfloat8 a, const vfloat8 b) { return (_mm256_movemask_ps(_mm256_cmp_ps(a, b, _MM_CMPINT_EQ))) == 255; }
  static inline bool cmp_ne(const vfloat8 a, const vfloat8 b) { return (_mm256_movemask_ps(_mm256_cmp_ps(a, b, _MM_CMPINT_NE))) == 255; }
  
  static inline vfloat8 join(cvex::vfloat4 a, cvex::vfloat4 b) { return         _mm256_insertf128_ps   (_mm256_castps128_ps256(a),b,1); }
  static inline vint8   join(cvex::vint4 a,   cvex::vint4 b)   { return (vint8) _mm256_insertf128_si256(_mm256_castsi128_si256((__m128i)a), (__m128i)b,1); }
  static inline vuint8  join(cvex::vuint4 a,  cvex::vuint4 b)  { return (vuint8)_mm256_insertf128_si256(_mm256_castsi128_si256((__m128i)a), (__m128i)b,1); }

#else
  
  static inline void set_ftz() {}
  static inline vfloat8 rcp_e(vfloat8 a)  { return 1.0f/a; }

  static inline vfloat8 floor(const vfloat8 a_val) { return vfloat8{::floorf(a[0]), ::floorf(a[1]), ::floorf(a[2]), ::floorf(a[3]), ::floorf(a[4]), ::floorf(a[5]), ::floorf(a[6]), ::floorf(a[7])}; }
  static inline vfloat8 ceil (const vfloat8 a_val) { return vfloat8{::ceilf(a[0]), ::ceilf(a[1]), ::ceilf(a[2]), ::ceilf(a[3]), ::ceilf(a[4]), ::ceilf(a[5]), ::ceilf(a[6]), ::ceilf(a[7])}; }
  static inline vfloat8 fabs (const vfloat8 a_val) { return vfloat8{::fabs(a[0]), ::fabs(a[1]), ::fabs(a[2]), ::fabs(a[3]), ::fabs(a[4]), ::fabs(a[5]), ::fabs(a[6]), ::fabs(a[7])}; }

#endif

  static inline bool test_bits_any(const vint8 a)
  {
    typedef int myvint4 __attribute__((vector_size(16)));

    const myvint4 a4 = ( *(myvint4*)&a  ) | ( *(myvint4*)&a[4] );
    const int64_t a2 = ( *(int64_t*)&a4 ) | ( *(int64_t*)&a4[2] );
    return (a2 != 0);
  }

  static inline bool test_bits_any(const vuint8 a)  { return test_bits_any( reinterpret_cast<vint8>(a) );  } 
  static inline bool test_bits_any(const vfloat8 a) { return test_bits_any( reinterpret_cast<vint8>(a) );  } 

  static inline bool test_bits_all(const vint8  v)  { return !test_bits_any(~v); }
  static inline bool test_bits_all(const vuint8 v)  { return !test_bits_any(~v); }
  static inline bool test_bits_all(const vfloat8 v) { return !test_bits_any(~as_int32(v)); }

  static inline vfloat8 min  (const vfloat8 a, const vfloat8 b)                            { return a < b ? a : b; }
  static inline vfloat8 max  (const vfloat8 a, const vfloat8 b)                            { return a > b ? a : b; }
  static inline vfloat8 clamp(const vfloat8 x, const vfloat8 minVal, const vfloat8 maxVal) { return max(min(x, maxVal), minVal); }

  static inline vint8 min  (const vint8 a, const vint8 b)                            { return a < b ? a : b; }
  static inline vint8 max  (const vint8 a, const vint8 b)                            { return a > b ? a : b; }
  static inline vint8 clamp(const vint8 x, const vint8 minVal, const vint8 maxVal)   { return max(min(x, maxVal), minVal); }

  static inline vuint8 min  (const vuint8 a, const vuint8 b)                            { return a < b ? a : b; }
  static inline vuint8 max  (const vuint8 a, const vuint8 b)                            { return a > b ? a : b; }
  static inline vuint8 clamp(const vuint8 x, const vuint8 minVal, const vuint8 maxVal)  { return max(min(x, maxVal), minVal); }

  static inline vfloat8 splat_0(const vfloat8 v) { return __builtin_shuffle(v, vint8{0,0,0,0,0,0,0,0}); }
  static inline vfloat8 splat_1(const vfloat8 v) { return __builtin_shuffle(v, vint8{1,1,1,1,1,1,1,1}); }
  static inline vfloat8 splat_2(const vfloat8 v) { return __builtin_shuffle(v, vint8{2,2,2,2,2,2,2,2}); }
  static inline vfloat8 splat_3(const vfloat8 v) { return __builtin_shuffle(v, vint8{3,3,3,3,3,3,3,3}); }
  static inline vfloat8 splat_4(const vfloat8 v) { return __builtin_shuffle(v, vint8{4,4,4,4,4,4,4,4}); }
  static inline vfloat8 splat_5(const vfloat8 v) { return __builtin_shuffle(v, vint8{5,5,5,5,5,5,5,5}); }
  static inline vfloat8 splat_6(const vfloat8 v) { return __builtin_shuffle(v, vint8{6,6,6,6,6,6,6,6}); }
  static inline vfloat8 splat_7(const vfloat8 v) { return __builtin_shuffle(v, vint8{7,7,7,7,7,7,7,7}); }

  static inline vint8   splat_0(const vint8 v) { return __builtin_shuffle(v, vint8{0,0,0,0,0,0,0,0}); }
  static inline vint8   splat_1(const vint8 v) { return __builtin_shuffle(v, vint8{1,1,1,1,1,1,1,1}); }
  static inline vint8   splat_2(const vint8 v) { return __builtin_shuffle(v, vint8{2,2,2,2,2,2,2,2}); }
  static inline vint8   splat_3(const vint8 v) { return __builtin_shuffle(v, vint8{3,3,3,3,3,3,3,3}); }
  static inline vint8   splat_4(const vint8 v) { return __builtin_shuffle(v, vint8{4,4,4,4,4,4,4,4}); }
  static inline vint8   splat_5(const vint8 v) { return __builtin_shuffle(v, vint8{5,5,5,5,5,5,5,5}); }
  static inline vint8   splat_6(const vint8 v) { return __builtin_shuffle(v, vint8{6,6,6,6,6,6,6,6}); }
  static inline vint8   splat_7(const vint8 v) { return __builtin_shuffle(v, vint8{7,7,7,7,7,7,7,7}); }

  static inline vuint8  splat_0(const vuint8 v){ return __builtin_shuffle(v, vuint8{0,0,0,0,0,0,0,0}); }
  static inline vuint8  splat_1(const vuint8 v){ return __builtin_shuffle(v, vuint8{1,1,1,1,1,1,1,1}); }
  static inline vuint8  splat_2(const vuint8 v){ return __builtin_shuffle(v, vuint8{2,2,2,2,2,2,2,2}); }
  static inline vuint8  splat_3(const vuint8 v){ return __builtin_shuffle(v, vuint8{3,3,3,3,3,3,3,3}); }
  static inline vuint8  splat_4(const vuint8 v){ return __builtin_shuffle(v, vuint8{4,4,4,4,4,4,4,4}); }
  static inline vuint8  splat_5(const vuint8 v){ return __builtin_shuffle(v, vuint8{5,5,5,5,5,5,5,5}); }
  static inline vuint8  splat_6(const vuint8 v){ return __builtin_shuffle(v, vuint8{6,6,6,6,6,6,6,6}); }
  static inline vuint8  splat_7(const vuint8 v){ return __builtin_shuffle(v, vuint8{7,7,7,7,7,7,7,7}); }

  static inline float extract_0(const vfloat8 a_val) { return a_val[0]; }
  static inline float extract_1(const vfloat8 a_val) { return a_val[1]; }
  static inline float extract_2(const vfloat8 a_val) { return a_val[2]; }
  static inline float extract_3(const vfloat8 a_val) { return a_val[3]; }
  static inline float extract_4(const vfloat8 a_val) { return a_val[4]; }
  static inline float extract_5(const vfloat8 a_val) { return a_val[5]; }
  static inline float extract_6(const vfloat8 a_val) { return a_val[6]; }
  static inline float extract_7(const vfloat8 a_val) { return a_val[7]; }

  static inline int   extract_0(const vint8 a_val)   { return a_val[0]; }
  static inline int   extract_1(const vint8 a_val)   { return a_val[1]; }
  static inline int   extract_2(const vint8 a_val)   { return a_val[2]; }
  static inline int   extract_3(const vint8 a_val)   { return a_val[3]; }
  static inline int   extract_4(const vint8 a_val)   { return a_val[4]; }
  static inline int   extract_5(const vint8 a_val)   { return a_val[5]; }
  static inline int   extract_6(const vint8 a_val)   { return a_val[6]; }
  static inline int   extract_7(const vint8 a_val)   { return a_val[7]; }

  static inline unsigned int extract_0(const vuint8 a_val) { return a_val[0]; }
  static inline unsigned int extract_1(const vuint8 a_val) { return a_val[1]; }
  static inline unsigned int extract_2(const vuint8 a_val) { return a_val[2]; }
  static inline unsigned int extract_3(const vuint8 a_val) { return a_val[3]; }
  static inline unsigned int extract_4(const vuint8 a_val) { return a_val[4]; }
  static inline unsigned int extract_5(const vuint8 a_val) { return a_val[5]; }
  static inline unsigned int extract_6(const vuint8 a_val) { return a_val[6]; }
  static inline unsigned int extract_7(const vuint8 a_val) { return a_val[7]; }

  static inline void prefetch(const float* ptr) {  __builtin_prefetch(ptr); }
  static inline void prefetch(const int* ptr)   {  __builtin_prefetch(ptr); }

};

#endif //TEST_GL_TOP_VFLOAT8_GCC_H