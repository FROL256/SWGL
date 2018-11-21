//
// Created by frol on 13.11.18.
//

#ifndef TEST_GL_TOP_VFLOAT8_GCC_H
#define TEST_GL_TOP_VFLOAT8_GCC_H

namespace cvex
{
  typedef          int   vint8   __attribute__((vector_size(32)));
  typedef unsigned int   vuint8  __attribute__((vector_size(32)));
  typedef          float vfloat8 __attribute__((vector_size(32)));

  typedef          int   _vint8u   __attribute__((vector_size(32), aligned(1)));
  typedef unsigned int   _vuint8u  __attribute__((vector_size(32), aligned(1)));
  typedef          float _vfloat8u __attribute__((vector_size(32), aligned(1)));

  static inline vint8   load_u(const int* p)   { return *((_vint8u*)p);   }
  static inline vfloat8 load_u(const float* p) { return *((_vfloat8u*)p); }

  static inline vint8   load  (const int* p)           { return *((vint8*)p);  }
  static inline vfloat8 load  (const float* p)         { return *((vfloat8*)p);}

  static inline void store_u(unsigned int* p, vuint8 a_val)  { p[0] = a_val[0]; p[1] = a_val[1]; p[2] = a_val[2]; p[3] = a_val[3]; p[4] = a_val[4]; p[5] = a_val[5]; p[6] = a_val[6]; p[7] = a_val[7]; }
  static inline void store_u(int* p,          vuint8 a_val)  { store_u((unsigned int*)p, a_val);}

  static inline void store_u(int* p,   vint8   a_val) { p[0] = a_val[0]; p[1] = a_val[1]; p[2] = a_val[2]; p[3] = a_val[3]; p[4] = a_val[4]; p[5] = a_val[5]; p[6] = a_val[6]; p[7] = a_val[7]; }
  static inline void store_u(float* p, vfloat8 a_val) { p[0] = a_val[0]; p[1] = a_val[1]; p[2] = a_val[2]; p[3] = a_val[3]; p[4] = a_val[4]; p[5] = a_val[5]; p[6] = a_val[6]; p[7] = a_val[7]; }
  static inline void store  (int* p,   vint8   a_val) { p[0] = a_val[0]; p[1] = a_val[1]; p[2] = a_val[2]; p[3] = a_val[3]; p[4] = a_val[4]; p[5] = a_val[5]; p[6] = a_val[6]; p[7] = a_val[7]; }
  static inline void store  (float* p, vfloat8 a_val) { p[0] = a_val[0]; p[1] = a_val[1]; p[2] = a_val[2]; p[3] = a_val[3]; p[4] = a_val[4]; p[5] = a_val[5]; p[6] = a_val[6]; p[7] = a_val[7]; }

  static inline vuint8  splat(unsigned int x) { return vuint8  {x,x,x,x,x,x,x,x}; }
  static inline vint8   splat(int x)          { return vint8   {x,x,x,x,x,x,x,x}; }
  static inline vfloat8 splat(float x)        { return vfloat8 {x,x,x,x,x,x,x,x}; }

  static inline vfloat8 to_float32(vint8 a)    { return vfloat8{(float)a[0], (float)a[1], (float)a[2], (float)a[3], (float)a[4], (float)a[5], (float)a[6], (float)a[7]}; }
  static inline vfloat8 to_float32(vuint8 a)   { return vfloat8{(float)a[0], (float)a[1], (float)a[2], (float)a[3], (float)a[4], (float)a[5], (float)a[6], (float)a[7]}; }

  static inline vint8   to_int32  (vfloat8 a)  { return vint8  {  (int)a[0],   (int)a[1],   (int)a[2],   (int)a[3],   (int)a[4],   (int)a[5],   (int)a[6],   (int)a[7]}; }
  static inline vuint8  to_uint32 (vfloat8 a)  { return vuint8  {  (unsigned int)a[0], (unsigned int)a[1], (unsigned int)a[2], (unsigned int)a[3], (unsigned int)a[4], (unsigned int)a[5], (unsigned int)a[6], (unsigned int)a[7]}; }

  static inline vint8   to_int32  (vuint8 a)   { return (vint8)a; }
  static inline vuint8  to_uint32 (vint8 a)    { return (vuint8)a; }

  static inline vfloat8 rcp_e(vfloat8 a)       { return 1.0f/a; }


  static inline vfloat8 blend(const vfloat8 a, const vfloat8 b, const vint8 mask)
  {
    const vint8 ia = reinterpret_cast<vint8>(a);
    const vint8 ib = reinterpret_cast<vint8>(b);
    return reinterpret_cast<vfloat8>((mask & ia) | (~mask & ib));
  }

  static inline vint8 blend(const vint8 a, const vint8 b, const vint8 mask)
  {
    return ((mask & a) | (~mask & b));
  }

  static inline vint8 blend(const vuint8 a, const vint8 b, const vint8 mask)
  {
    return ((mask & (vint8)a) | (~mask & b));
  }

  static inline bool test_bits_any(const vint8 a)
  {
    typedef int myvint4 __attribute__((vector_size(16)));

    //return (a[0] != 0 && a[1] != 0 && a[2] != 0 && a[3] != 0) && (a[4] != 0 && a[5] != 0 && a[6] != 0 && a[7] != 0);
    const myvint4 a4 = ( *(myvint4*)&a )  | ( *(myvint4*)&a[4] );
    const int64_t a2 = ( *(int64_t*)&a4 ) | ( *(int64_t*)&a4[2] );
    return (a2 != 0);
  }

  static inline vint8 make_vint(const int a, const int b, const int c, const int d,
                                const int e, const int f, const int g, const int h) { return vint8{a,b,c,d, e,f,g,h}; }


  static inline vfloat8 min(const vfloat8 a, const vfloat8 b)
  {
    return a < b ? a : b;
  }

  static inline vfloat8 max(const vfloat8 a, const vfloat8 b)
  {
    return a > b ? a : b;
  }

  static inline vfloat8 vclamp(const vfloat8 x, const vfloat8 minVal, const vfloat8 maxVal)
  {
    return max(min(x, maxVal), minVal);
  }


#ifdef __x86_64
#include <xmmintrin.h> // SSE
  void set_ftz() { _MM_SET_ROUNDING_MODE(_MM_ROUND_TOWARD_ZERO); }
#else
  void set_ftz() {}
#endif


  static inline void prefetch(const float* ptr) {  __builtin_prefetch(ptr); }
  static inline void prefetch(const int* ptr)   {  __builtin_prefetch(ptr); }

};

#endif //TEST_GL_TOP_VFLOAT8_GCC_H
