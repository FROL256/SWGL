//
// Created by frol on 30.10.18.
//

#ifndef TEST_GL_TOP_VFLOAT4_ARM_H
#define TEST_GL_TOP_VFLOAT4_ARM_H

#include <arm_neon.h>

#define ALIGNED(x) __attribute__ ((aligned (x)))

namespace cvex
{

  struct vfloat4
  {
    inline vfloat4(float32x4_t a) {data = a;}
    inline operator float32x4_t() const {return data;}
    float32x4_t data;   
  };

  struct vint4 
  {
    inline vint4(int32x4_t a) {data = a;}
    inline operator int32x4_t() const {return data;}
    int32x4_t data;
  };

  struct vuint4 
  {
    inline vuint4(uint32x4_t a) {data = a;}
    inline operator uint32x4_t() const {return data;}
    uint32x4_t data;
  };  

  //typedef float32x4_t vfloat4;
  //typedef int32x4_t   vint4;
  //typedef uint32x4_t  vuint4;

  static inline vfloat4 load  (const float* p) { return vld1q_f32(p); }
  static inline vfloat4 load_u(const float* p) { return vld1q_f32(p); }

  static inline void store  (float* p, vfloat4 a_val) { vst1q_f32(p, a_val); }
  static inline void store_u(float* p, vfloat4 a_val) { vst1q_f32(p, a_val); }

 
  static inline vuint4  load  (const unsigned int* p) { return vld1q_u32(p); }
  static inline vuint4  load_u(const unsigned int* p) { return vld1q_u32(p); }

  static inline vint4   load  (const int* p)          { return vld1q_s32(p); }
  static inline vint4   load_u(const int* p)          { return vld1q_s32(p); }

  static inline void store  (unsigned int* p, vuint4 a_val) { vst1q_u32(p, a_val); }
  static inline void store_u(unsigned int* p, vuint4 a_val) { vst1q_u32(p, a_val); }

  static inline void store  (int* p, vint4 a_val)  { vst1q_s32(p, a_val); }
  static inline void store_u(int* p, vint4 a_val)  { vst1q_s32(p, a_val); }

  static inline vuint4  splat(unsigned int x)  { return vld1q_dup_u32(&x); }
  static inline vint4   splat(int x)           { return vld1q_dup_s32(&x); }
  static inline vfloat4 splat(float x)         { return vld1q_dup_f32(&x); }

  static inline vfloat4 to_float32(vint4 a)    { return vcvtq_f32_s32(a); }
  static inline vfloat4 to_float32(vuint4 a)   { return vcvtq_f32_u32(a); }
  
  static inline vint4   to_int32  (vfloat4 a)  { return vcvtq_s32_f32(a); }
  static inline vuint4  to_uint32 (vfloat4 a)  { return vcvtq_u32_f32(a); }
  static inline vuint4  to_uint32 (vint4 a)    { return (uint32x4_t)a.data; }

  static inline vfloat4 rcp_e(vfloat4 a)       { return vrecpeq_f32(a); }

  static inline vfloat4 blend(const vfloat4 a, const vfloat4 b, const vint4 mask) { return vbslq_f32((uint32x4_t)mask.data, a.data, b.data); }
  static inline vint4 blend(const vint4 a, const vint4 b, const vint4 mask) { return vbslq_s32((uint32x4_t)mask.data, a.data, b.data); }
  static inline vint4 blend(const vuint4 a, const vint4 b, const vint4 mask) { return vbslq_s32((uint32x4_t)mask.data, (int32x4_t)a.data, b.data); }

  /*

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

  */

  static inline void set_ftz() { }

  static inline void prefetch(const float* ptr) {  __builtin_prefetch(ptr); }
  static inline void prefetch(const int* ptr)   {  __builtin_prefetch(ptr); }
};

static inline cvex::vfloat4 operator+(cvex::vfloat4 a, cvex::vfloat4 b) { return vaddq_f32(a.data, b.data); }
static inline cvex::vfloat4 operator-(cvex::vfloat4 a, cvex::vfloat4 b) { return vsubq_f32(a.data, b.data); }
static inline cvex::vfloat4 operator*(cvex::vfloat4 a, cvex::vfloat4 b) { return vmulq_f32(a.data, b.data); }
static inline cvex::vfloat4 operator/(cvex::vfloat4 a, cvex::vfloat4 b) { float32x4_t rcp = vrecpeq_f32(b); return vmulq_f32(a.data, rcp); }

#endif //TEST_GL_TOP_VFLOAT4_GCC_H
