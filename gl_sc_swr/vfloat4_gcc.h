//
// Created by frol on 30.10.18.
//

#ifndef TEST_GL_TOP_VFLOAT4_GCC_H
#define TEST_GL_TOP_VFLOAT4_GCC_H

namespace cvex
{
  typedef unsigned int vuint4  __attribute__((vector_size(16)));
  typedef int          vint4   __attribute__((vector_size(16)));
  typedef float        vfloat4 __attribute__((vector_size(16)));

  typedef unsigned int _vuint4u  __attribute__((vector_size(16), aligned(1)));
  typedef int          _vint4u   __attribute__((vector_size(16), aligned(1)));
  typedef float        _vfloat4u __attribute__((vector_size(16), aligned(1)));

  static inline vuint4  load_u(const unsigned int* p)   { return *((_vuint4u*)p); }
  static inline vint4   load_u(const int* p)   { return *((_vint4u*)p); }
  static inline vfloat4 load_u(const float* p) { return *((_vfloat4u*)p); }

  static inline vuint4  load  (const unsigned int* p) { return *((vuint4*)p);   }
  static inline vint4   load  (const int* p)          { return *((vint4*)p);   }
  static inline vfloat4 load  (const float* p)        { return *((vfloat4*)p); }

  static inline void store_u(unsigned int* p, vuint4 a_val) { p[0] = a_val[0]; p[1] = a_val[1]; p[2] = a_val[2]; p[3] = a_val[3];  }
  static inline void store_u(int* p,          vuint4 a_val) { store_u((unsigned int*)p, a_val);  }
  static inline void store_u(int* p,   vint4   a_val) { p[0] = a_val[0]; p[1] = a_val[1]; p[2] = a_val[2]; p[3] = a_val[3];  }
  static inline void store_u(float* p, vfloat4 a_val) { p[0] = a_val[0]; p[1] = a_val[1]; p[2] = a_val[2]; p[3] = a_val[3];  }

  static inline void store(int* p,   vint4   a_val)   { p[0] = a_val[0]; p[1] = a_val[1]; p[2] = a_val[2]; p[3] = a_val[3];  }
  static inline void store(float* p, vfloat4 a_val)   { p[0] = a_val[0]; p[1] = a_val[1]; p[2] = a_val[2]; p[3] = a_val[3];  }

  static inline vuint4  splat(unsigned int x)  { return vuint4 {x,x,x,x}; }
  static inline vint4   splat(int x)           { return vint4  {x,x,x,x}; }
  static inline vfloat4 splat(float x)         { return vfloat4{x,x,x,x}; }

  static inline vfloat4 to_float32(vint4 a)    { return vfloat4{(float)a[0], (float)a[1], (float)a[2], (float)a[3]}; }
  static inline vfloat4 to_float32(vuint4 a)   { return vfloat4{(float)a[0], (float)a[1], (float)a[2], (float)a[3]}; }

  static inline vint4   to_int32  (vfloat4 a)  { return vint4  {  (int)a[0],   (int)a[1],   (int)a[2],   (int)a[3]}; }
  static inline vuint4  to_uint32 (vfloat4 a)  { return vuint4 {  (unsigned int)a[0], (unsigned int)a[1], (unsigned int)a[2], (unsigned int)a[3]}; }
  static inline vuint4  to_uint32 (vint4 a)    { return (vuint4)a; }

  static inline vfloat4 rcp_e(vfloat4 a)       { return 1.0f/a; }

  static inline vfloat4 blend(const vfloat4 a, const vfloat4 b, const vint4 mask)
  {
    const vint4 ia = reinterpret_cast<vint4>(a);
    const vint4 ib = reinterpret_cast<vint4>(b);
    return reinterpret_cast<vfloat4>((mask & ia) | (~mask & ib));
  }

  static inline vint4 blend(const vint4 a, const vint4 b, const vint4 mask)
  {
    return ((mask & a) | (~mask & b));
  }

  static inline vint4 blend(const vuint4 a, const vint4 b, const vint4 mask)
  {
    return ((mask & (vint4)a) | (~mask & b));
  }

  static inline bool test_bits_any(const vint4 a)
  {
    //return (a[0] != 0 && a[1] != 0 && a[2] != 0 && a[3] != 0);
    const int64_t a2 = ( *(int64_t*)&a ) | ( *(int64_t*)&a[2] );
    return (a2 != 0);
  }

  static inline vint4  make_vint(const int a, const int b, const int c, const int d) { return vint4{a,b,c,d}; }
  static inline vuint4 make_vuint(const unsigned int a, const unsigned int b, const unsigned int c, const unsigned int d) { return vuint4{a,b,c,d}; }

  static inline vfloat4 min(const vfloat4 a, const vfloat4 b)
  {
    return a < b ? a : b;
  }

  static inline vfloat4 max(const vfloat4 a, const vfloat4 b)
  {
    return a > b ? a : b;
  }

  static inline vfloat4 vclamp(const vfloat4 x, const vfloat4 minVal, const vfloat4 maxVal)
  {
    return max(min(x, maxVal), minVal);
  }

  #ifdef __x86_64
  #include <xmmintrin.h> // SSE
  static inline void set_ftz() { _MM_SET_ROUNDING_MODE(_MM_ROUND_TOWARD_ZERO); }
  #else
  static inline void set_ftz() { }
  #endif


  static inline void prefetch(const float* ptr) {  __builtin_prefetch(ptr); }
  static inline void prefetch(const int* ptr)   {  __builtin_prefetch(ptr); }
};


#endif //TEST_GL_TOP_VFLOAT4_GCC_H
