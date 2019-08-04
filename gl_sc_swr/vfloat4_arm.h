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
    inline vfloat4() {}
    inline vfloat4(float32x4_t a) {data = a;}
    inline operator float32x4_t() const {return data;}
    float32x4_t data;   
  };

  struct vint4 
  {
    inline vint4() {}
    inline vint4(int32x4_t a) {data = a;}
    inline operator int32x4_t() const {return data;}
    int32x4_t data;
  };

  struct vuint4 
  {
    inline vuint4() {}
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
  static inline vint4   blend(const vint4 a, const vint4 b, const vint4 mask)     { return vbslq_s32((uint32x4_t)mask.data, a.data, b.data); }
  static inline vint4   blend(const vuint4 a, const vint4 b, const vint4 mask)    { return vbslq_s32((uint32x4_t)mask.data, (int32x4_t)a.data, b.data); }

  static inline bool test_bits_any(const vint4 a)
  {
    const uint32x2_t tmp = vorr_u32(vget_low_u32((uint32x4_t)a.data), 
                                    vget_high_u32((uint32x4_t)a.data));
    return (vget_lane_u32(vpmax_u32(tmp, tmp), 0) != 0);
  }

  static inline vint4  make_vint(const int a, const int b, const int c, const int d) { return int32x4_t{a,b,c,d}; }
  static inline vuint4 make_vuint(const unsigned int a, const unsigned int b, const unsigned int c, const unsigned int d) { return uint32x4_t{a,b,c,d}; }

  static inline vfloat4 min(const vfloat4 a, const vfloat4 b) { return vminq_f32(a.data,b.data); }
  static inline vfloat4 max(const vfloat4 a, const vfloat4 b) { return vmaxq_f32(a.data,b.data); }

  static inline vfloat4 vclamp(const vfloat4 x, const vfloat4 minVal, const vfloat4 maxVal) { return max(min(x, maxVal), minVal);  }

  static inline void set_ftz() { }

  static inline void prefetch(const float* ptr) {  __builtin_prefetch(ptr); }
  static inline void prefetch(const int* ptr)   {  __builtin_prefetch(ptr); }
};

static inline cvex::vfloat4 operator+(cvex::vfloat4 a, cvex::vfloat4 b) { return vaddq_f32(a.data, b.data); }
static inline cvex::vfloat4 operator-(cvex::vfloat4 a, cvex::vfloat4 b) { return vsubq_f32(a.data, b.data); }
static inline cvex::vfloat4 operator*(cvex::vfloat4 a, cvex::vfloat4 b) { return vmulq_f32(a.data, b.data); }
static inline cvex::vfloat4 operator/(cvex::vfloat4 a, cvex::vfloat4 b) { float32x4_t rcp = vrecpeq_f32(b); return vmulq_f32(a.data, rcp); }

static inline cvex::vfloat4 operator+(const cvex::vfloat4 a, const float b) { return vaddq_f32(a.data, vld1q_dup_f32(&b)); }
static inline cvex::vfloat4 operator-(const cvex::vfloat4 a, const float b) { return vsubq_f32(a.data, vld1q_dup_f32(&b)); }
static inline cvex::vfloat4 operator*(const cvex::vfloat4 a, const float b) { return vmulq_f32(a.data, vld1q_dup_f32(&b)); }
static inline cvex::vfloat4 operator/(const cvex::vfloat4 a, const float b) { const float bInv = 1.0f/b; return vmulq_f32(a.data, vld1q_dup_f32(&bInv)); }

static inline cvex::vfloat4 operator+(const float b, const cvex::vfloat4 a) { return vaddq_f32(a.data, vld1q_dup_f32(&b)); }
static inline cvex::vfloat4 operator-(const float b, const cvex::vfloat4 a) { return vsubq_f32(a.data, vld1q_dup_f32(&b)); }
static inline cvex::vfloat4 operator*(const float b, const cvex::vfloat4 a) { return vmulq_f32(a.data, vld1q_dup_f32(&b)); }



static inline cvex::vint4 operator+(const cvex::vint4 a, const cvex::vint4 b) { return vaddq_s32(a.data, b.data); }
static inline cvex::vint4 operator-(const cvex::vint4 a, const cvex::vint4 b) { return vsubq_s32(a.data, b.data);}
static inline cvex::vint4 operator*(const cvex::vint4 a, const cvex::vint4 b) { return vmulq_s32(a.data, b.data); }

static inline cvex::vint4 operator+(const cvex::vint4 a, const int b) { return vaddq_s32(a, vld1q_dup_s32(&b)); }
static inline cvex::vint4 operator-(const cvex::vint4 a, const int b) { return vsubq_s32(a, vld1q_dup_s32(&b)); }
static inline cvex::vint4 operator*(const cvex::vint4 a, const int b) { return vmulq_s32(a, vld1q_dup_s32(&b)); }

static inline cvex::vint4 operator+(const int a, const cvex::vint4 b) { return vaddq_s32(vld1q_dup_s32(&a), b); }
static inline cvex::vint4 operator-(const int a, const cvex::vint4 b) { return vsubq_s32(vld1q_dup_s32(&a), b); }
static inline cvex::vint4 operator*(const int a, const cvex::vint4 b) { return vmulq_s32(vld1q_dup_s32(&a), b); }



static inline cvex::vuint4 operator+(const cvex::vuint4 a, const cvex::vuint4 b) { return vaddq_u32(a.data, b.data); }
static inline cvex::vuint4 operator-(const cvex::vuint4 a, const cvex::vuint4 b) { return vsubq_u32(a.data, b.data);}
static inline cvex::vuint4 operator*(const cvex::vuint4 a, const cvex::vuint4 b) { return vmulq_u32(a.data, b.data); }

static inline cvex::vuint4 operator+(const cvex::vuint4 a, const int b) { return vaddq_u32(a, vld1q_dup_u32((unsigned int*)&b)); }
static inline cvex::vuint4 operator-(const cvex::vuint4 a, const int b) { return vsubq_u32(a, vld1q_dup_u32((unsigned int*)&b)); }
static inline cvex::vuint4 operator*(const cvex::vuint4 a, const int b) { return vmulq_u32(a, vld1q_dup_u32((unsigned int*)&b)); }

static inline cvex::vuint4 operator+(const int a, const cvex::vuint4 b) { return vaddq_u32(vld1q_dup_u32((unsigned int*)&a), b); }
static inline cvex::vuint4 operator-(const int a, const cvex::vuint4 b) { return vsubq_u32(vld1q_dup_u32((unsigned int*)&a), b); }
static inline cvex::vuint4 operator*(const int a, const cvex::vuint4 b) { return vmulq_u32(vld1q_dup_u32((unsigned int*)&a), b); }



static inline cvex::vint4 operator<<(const cvex::vint4 a, const int val) { return vshlq_n_s32(a, val); }
static inline cvex::vint4 operator>>(const cvex::vint4 a, const int val) { return vshrq_n_s32(a, val); }

static inline cvex::vuint4 operator<<(const cvex::vuint4 a, const int val) { return vshlq_n_u32(a, val); }
static inline cvex::vuint4 operator>>(const cvex::vuint4 a, const int val) { return vshrq_n_u32(a, val); }

static inline cvex::vint4 operator|(const cvex::vint4 a, const cvex::vint4 b) { return vorrq_s32(a,b); }
static inline cvex::vint4 operator&(const cvex::vint4 a, const cvex::vint4 b) { return vandq_s32(a,b); }

static inline cvex::vuint4 operator|(const cvex::vuint4 a, const cvex::vuint4 b) { return vorrq_u32(a,b); }
static inline cvex::vuint4 operator&(const cvex::vuint4 a, const cvex::vuint4 b) { return vandq_u32(a,b); }

static inline cvex::vuint4 operator|(const cvex::vint4 a, const cvex::vuint4 b) { return vorrq_u32((uint32x4_t)a.data, b); }
static inline cvex::vuint4 operator&(const cvex::vint4 a, const cvex::vuint4 b) { return vandq_u32((uint32x4_t)a.data, b); }

static inline cvex::vuint4 operator|(const cvex::vuint4 a, const int b) { return vorrq_u32(a, vld1q_dup_u32((unsigned int*)&b)); }
static inline cvex::vuint4 operator&(const cvex::vuint4 a, const int b) { return vandq_u32(a, vld1q_dup_u32((unsigned int*)&b)); }

static inline cvex::vint4 operator> (const cvex::vfloat4 a, const cvex::vfloat4 b) { return (int32x4_t)vcgtq_f32(a, b); }
static inline cvex::vint4 operator< (const cvex::vfloat4 a, const cvex::vfloat4 b) { return (int32x4_t)vcltq_f32(a, b); }
static inline cvex::vint4 operator>=(const cvex::vfloat4 a, const cvex::vfloat4 b) { return (int32x4_t)vcgeq_f32(a, b); }
static inline cvex::vint4 operator<=(const cvex::vfloat4 a, const cvex::vfloat4 b) { return (int32x4_t)vcleq_f32(a, b); }

#endif //TEST_GL_TOP_VFLOAT4_GCC_H
