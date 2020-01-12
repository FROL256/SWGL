//
// Created by frol on 13.11.18.
//

#ifndef TEST_GL_TOP_VFLOAT16_GCC_H
#define TEST_GL_TOP_VFLOAT16_GCC_H

#ifdef __x86_64
  #include <immintrin.h>
#endif

#include "vfloat8_gcc.h"

namespace cvex16
{
  typedef unsigned           int _uint32_t;
  typedef unsigned long long int _uint64_t;
  typedef          long long int _sint64_t;

  typedef          int   vint16   __attribute__((vector_size(64)));
  typedef unsigned int   vuint16  __attribute__((vector_size(64)));
  typedef          float vfloat16 __attribute__((vector_size(64)));

  typedef          int   _vint16u   __attribute__((vector_size(64), aligned(1)));
  typedef unsigned int   _vuint16u  __attribute__((vector_size(64), aligned(1)));
  typedef          float _vfloat16u __attribute__((vector_size(64), aligned(1)));

  static inline vint16   load_u(const int* p)       { return *((_vint16u*)p);   }
  static inline vuint16  load_u(const _uint32_t* p) { return *((_vuint16u*)p);  }
  static inline vfloat16 load_u(const float* p)     { return *((_vfloat16u*)p); }

  static inline vint16   load  (const int* p)       { return *((vint16*)p);   }
  static inline vuint16  load  (const _uint32_t* p) { return *((vuint16*)p);  }
  static inline vfloat16 load  (const float* p)     { return *((vfloat16*)p); }

  static inline void store_u(int* p,       vint16   a_val)   { *((_vint16u*)(p))   = a_val; }
  static inline void store_u(_uint32_t* p, vuint16  a_val)   { *((_vuint16u*)(p))  = a_val; }
  static inline void store_u(float* p,     vfloat16 a_val)   { *((_vfloat16u*)(p)) = a_val; }

  static inline void store(int* p,       vint16   a_val)   { *((vint16*)(p))   = a_val; }
  static inline void store(_uint32_t* p, vuint16  a_val)   { *((vuint16*)(p))  = a_val; }
  static inline void store(float* p,     vfloat16 a_val)   { *((vfloat16*)(p)) = a_val; }

  static inline vuint16  splat(unsigned int x) { return vuint16  {x,x,x,x,x,x,x,x, x,x,x,x,x,x,x,x}; }
  static inline vint16   splat(int x)          { return vint16   {x,x,x,x,x,x,x,x, x,x,x,x,x,x,x,x}; }
  static inline vfloat16 splat(float x)        { return vfloat16 {x,x,x,x,x,x,x,x, x,x,x,x,x,x,x,x}; }

  static inline vfloat16 as_float32(const vint16 a_val)   { return reinterpret_cast<vfloat16>(a_val); }
  static inline vfloat16 as_float32(const vuint16 a_val)  { return reinterpret_cast<vfloat16>(a_val); }
  static inline vint16   as_int32  (const vfloat16 a_val) { return reinterpret_cast<vint16>(a_val);   }
  static inline vuint16  as_uint32 (const vfloat16 a_val) { return reinterpret_cast<vuint16>(a_val);  }

  static inline vfloat16 to_float32(vint16 a)    { return vfloat16{(float)a[0], (float)a[1], (float)a[2],  (float)a[3],  (float)a[4],  (float)a[5],  (float)a[6],  (float)a[7],
                                                                   (float)a[8], (float)a[9], (float)a[10], (float)a[11], (float)a[12], (float)a[13], (float)a[14], (float)a[15]}; }

  static inline vfloat16 to_float32(vuint16 a)   { return vfloat16{(float)a[0], (float)a[1], (float)a[2],  (float)a[3],  (float)a[4],  (float)a[5],  (float)a[6],  (float)a[7],
                                                                   (float)a[8], (float)a[9], (float)a[10], (float)a[11], (float)a[12], (float)a[13], (float)a[14], (float)a[15]}; }

  static inline vint16   to_int32  (vfloat16 a)  { return vint16  {  (int)a[0],   (int)a[1],   (int)a[2],   (int)a[3],   (int)a[4],   (int)a[5],   (int)a[6],   (int)a[7],  
                                                                     (int)a[8],   (int)a[9],   (int)a[10],  (int)a[11],  (int)a[12],  (int)a[13],  (int)a[14],  (int)a[15] }; }

  static inline vuint16  to_uint32 (vfloat16 a)  { return vuint16 {  (unsigned int)a[0], (unsigned int)a[1], (unsigned int)a[2],  (unsigned int)a[3],  (unsigned int)a[4],  (unsigned int)a[5],  (unsigned int)a[6],  (unsigned int)a[7],
                                                                     (unsigned int)a[8], (unsigned int)a[9], (unsigned int)a[10], (unsigned int)a[11], (unsigned int)a[12], (unsigned int)a[13], (unsigned int)a[14], (unsigned int)a[15] }; }

  static inline vint16   to_int32  (vuint16 a)   { return (vint16)a; }
  static inline vuint16  to_uint32 (vint16 a)    { return (vuint16)a; }

  static inline vfloat16 blend(const vfloat16 a, const vfloat16 b, const vint16 mask)
  {
    const vint16 ia = reinterpret_cast<vint16>(a);
    const vint16 ib = reinterpret_cast<vint16>(b);
    return reinterpret_cast<vfloat16>((mask & ia) | (~mask & ib));
  }

  static inline vint16  blend(const vint16 a, const vint16 b, const vint16 mask)  { return ((mask & a)        | (~mask & b)); }
  static inline vint16  blend(const vuint16 a, const vint16 b, const vint16 mask) { return ((mask & (vint16)a) | (~mask & b)); }
  static inline vuint16 blend(const vuint16 a, const vuint16 b, const vint16 a_mask) 
  {
    const vuint16 mask = reinterpret_cast<vuint16>(a_mask);
    return (mask & a) | (~mask & b);
  }

  #ifdef __x86_64
  
  static inline void set_ftz() { _MM_SET_ROUNDING_MODE(_MM_ROUND_TOWARD_ZERO); }
  static inline vfloat16 rcp_e(vfloat16 a) { return _mm512_rcp14_ps(a); }

  //static inline bool test_bits_any(const vfloat16 a) { return (_mm512_movemask_ps(a) & 15) != 0; }

  static inline vfloat16 floor(const vfloat16 a_val) { return _mm512_floor_ps(a_val); }
  static inline vfloat16 ceil (const vfloat16 a_val) { return _mm512_ceil_ps(a_val);  }
  static inline vfloat16 fabs (const vfloat16 a_val)
  {
    const __m512 absmask = _mm512_castsi512_ps(_mm512_set1_epi32((1<<31)));
    return _mm512_andnot_ps(absmask, a_val);
  }

  static inline bool cmp_gt(const vfloat16 a, const vfloat16 b) { return (_mm512_cmp_ps_mask(a, b, _MM_CMPINT_GT) == __mmask16(65535) ); }
  static inline bool cmp_lt(const vfloat16 a, const vfloat16 b) { return (_mm512_cmp_ps_mask(a, b, _MM_CMPINT_LT) == __mmask16(65535) ); }
  static inline bool cmp_ge(const vfloat16 a, const vfloat16 b) { return (_mm512_cmp_ps_mask(a, b, _MM_CMPINT_GE) == __mmask16(65535) ); }
  static inline bool cmp_le(const vfloat16 a, const vfloat16 b) { return (_mm512_cmp_ps_mask(a, b, _MM_CMPINT_LE) == __mmask16(65535) ); }
  static inline bool cmp_eq(const vfloat16 a, const vfloat16 b) { return (_mm512_cmp_ps_mask(a, b, _MM_CMPINT_EQ) == __mmask16(65535) ); }
  static inline bool cmp_ne(const vfloat16 a, const vfloat16 b) { return (_mm512_cmp_ps_mask(a, b, _MM_CMPINT_NE) == __mmask16(65535) ); }

  static inline bool any_of(const vint16 a) { return _mm512_cmp_ps_mask(as_float32(a), _mm512_setzero_ps(), _MM_CMPINT_EQ) != __mmask16(65535); }

  static inline vfloat16 join(const cvex8::vfloat8 a, const cvex8::vfloat8 b) { return _mm512_insertf32x8(_mm512_castps256_ps512(a),b,1); }
  static inline vint16   join(const cvex8::vint8   a, const cvex8::vint8   b) { return (vint16)_mm512_inserti32x8(_mm512_castsi256_si512((__m256i)a), (__m256i)b,1); }
  static inline vuint16  join(const cvex8::vuint8  a, const cvex8::vuint8  b) { return (vuint16)_mm512_inserti32x8(_mm512_castsi256_si512((__m256i)a), (__m256i)b,1); }

  static inline vint16   gather(const int* a_data, const vint16 offset) { return (vint16)_mm512_i32gather_epi32((__m512i)offset, a_data, 4); }

  #endif

  static inline vfloat16 min  (const vfloat16 a, const vfloat16 b)                             { return a < b ? a : b; }
  static inline vfloat16 max  (const vfloat16 a, const vfloat16 b)                             { return a > b ? a : b; }
  static inline vfloat16 clamp(const vfloat16 x, const vfloat16 minVal, const vfloat16 maxVal) { return max(min(x, maxVal), minVal); }

  static inline vint16 min  (const vint16 a, const vint16 b)                            { return a < b ? a : b; }
  static inline vint16 max  (const vint16 a, const vint16 b)                            { return a > b ? a : b; }
  static inline vint16 clamp(const vint16 x, const vint16 minVal, const vint16 maxVal)  { return max(min(x, maxVal), minVal); }

  static inline vuint16 min  (const vuint16 a, const vuint16 b)                            { return a < b ? a : b; }
  static inline vuint16 max  (const vuint16 a, const vuint16 b)                            { return a > b ? a : b; }
  static inline vuint16 clamp(const vuint16 x, const vuint16 minVal, const vuint16 maxVal) { return max(min(x, maxVal), minVal); } 

  static inline void prefetch(const float* ptr) {  __builtin_prefetch(ptr); }
  static inline void prefetch(const int* ptr)   {  __builtin_prefetch(ptr); }

};

#endif //TEST_GL_TOP_vfloat16_GCC_H