//
// Created by frol on 30.10.18.
//

#ifndef TEST_GL_TOP_VFLOAT4_GCC_H
#define TEST_GL_TOP_VFLOAT4_GCC_H

namespace cvex
{
  typedef int   vint4   __attribute__((vector_size(16)));
  typedef float vfloat4 __attribute__((vector_size(16)));

  static inline auto load_u(const int* p) -> vint4 { return vint4{p[0], p[1], p[2], p[3]}; } //__builtin_assume_aligned(p, 16)
  static inline auto load  (const int* p) -> vint4
  {
    const int* pa = __builtin_assume_aligned(p, 16);
    return vint4{pa[0], pa[1], pa[2], pa[3]};
  }

  static inline auto load_u(const float* p) -> vfloat4 { return vfloat4{p[0], p[1], p[2], p[3]}; } //__builtin_assume_aligned(p, 16)
  static inline auto load  (const float* p) -> vfloat4
  {
    const float* pa = __builtin_assume_aligned(p, 16);
    return vfloat4{pa[0], pa[1], pa[2], pa[3]};
  }

  static inline void store_u(int* p,   vint4   a_val) { p[0] = a_val[0]; p[1] = a_val[1]; p[2] = a_val[2]; p[3] = a_val[3];  }
  static inline void store_u(float* p, vfloat4 a_val) { p[0] = a_val[0]; p[1] = a_val[1]; p[2] = a_val[2]; p[3] = a_val[3];  }

  static inline void store(int* p,   vint4   a_val)
  {
    int*   p2 = __builtin_assume_aligned(p, 16);
    p2[0] = a_val[0]; p2[1] = a_val[1]; p2[2] = a_val[2]; p2[3] = a_val[3];
  }

  static inline void store(float* p, vfloat4 a_val)
  {
    float* p2 = __builtin_assume_aligned(p, 16);
    p2[0] = a_val[0]; p2[1] = a_val[1]; p2[2] = a_val[2]; p2[3] = a_val[3];
  }

  static inline auto splat(int x)   -> vint4   { return vint4  {x,x,x,x}; }
  static inline auto splat(float x) -> vfloat4 { return vfloat4{x,x,x,x}; }

  static inline vfloat4 to_float32(vint4 a)    { return vfloat4{(float)a[0], (float)a[1], (float)a[2], (float)a[3]}; }
  static inline vint4   to_int32  (vfloat4 a)  { return vint4  {  (int)a[0],   (int)a[1],   (int)a[2],   (int)a[3]}; }

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

  static inline vint4 cmp_gt_asint(vfloat4 a, vfloat4 b) { return (a > b); }

  static inline bool test_bits_any(const vint4 a) { return (a[0] != 0 && a[1] != 0 && a[2] != 0 && a[3] != 0); }

  void set_ftz() {}
};


#endif //TEST_GL_TOP_VFLOAT4_GCC_H