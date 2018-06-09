#pragma once

#include <math.h>
#include <stdlib.h>

#include "config.h"

#ifdef ENABLE_SSE
  #include <xmmintrin.h> // SSE
  #include <emmintrin.h> // SSE2
  #include <smmintrin.h> // SSE4.1
#endif

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef WIN32
  #define ALIGNED(x) __declspec(align(x))
#else
  #define ALIGNED(x) __attribute__ ((aligned (x)))
#endif

struct int2
{
  int2() : x(0), y(0){}
  int2(int a, int b) : x(a), y(b) {}

  int x, y;
};


struct float2
{
  float2() :x(0), y(0){}
  float2(float a, float b) : x(a), y(b) {}

  float x, y;
};

struct float3
{
  float3() :x(0), y(0), z(0) {}
  float3(float a, float b, float c) : x(a), y(b), z(c) {}
  float3(const float* ptr) : x(ptr[0]), y(ptr[1]), z(ptr[0]) {}

  float x, y, z;
};

struct float4
{
  float4() :x(0), y(0), z(0), w(0) {}
  float4(float a, float b, float c, float d) : x(a), y(b), z(c), w(d) {}

  float x, y, z, w;
};


struct float4x4
{
  float4x4(){ identity(); }

  float4x4(const float arr[16])
  {
    row[0] = float4(arr[0], arr[1], arr[2], arr[3]);
    row[1] = float4(arr[4], arr[5], arr[6], arr[7]);
    row[2] = float4(arr[8], arr[9], arr[10], arr[11]);
    row[3] = float4(arr[12], arr[13], arr[14], arr[15]);
  }

  void identity()
  {
    row[0] = float4(1, 0, 0, 0);
    row[1] = float4(0, 1, 0, 0);
    row[2] = float4(0, 0, 1, 0);
    row[3] = float4(0, 0, 0, 1);
  }

  float& M(int x, int y) { return ((float*)row)[y * 4 + x]; }
  float  M(int x, int y) const { return ((float*)row)[y * 4 + x]; }

  float* L() { return (float*)row; }
  const float* L() const { return (float*)row; }

  float4 row[4];
};

inline float rnd(float s, float e)
{

  float t = (float)(rand())/RAND_MAX;
  return s + t*(e - s);
}

static inline float max(float a, float b) { return a > b ? a : b; }
static inline float min(float a, float b) { return a < b ? a : b; }

static inline float clamp(float u, float a, float b) { float r = max(a, u); return min(r, b); }
static inline int   clamp(int u, int a, int b) { int r = (a > u) ? a : u; return (r < b) ? r : b; }

static inline int max(int a, int b) { return a > b ? a : b; }
static inline int min(int a, int b) { return a < b ? a : b; }


#define SQR(x) (x)*(x)

static inline float4 make_float4(float a, float b, float c, float d) { return float4(a, b, c, d); }
static inline float3 make_float3(float a, float b, float c) { return float3(a, b, c); }
static inline float2 make_float2(float a, float b) { return float2(a, b); }

static inline float2 to_float2(float4 v) { return make_float2(v.x, v.y); }
static inline float2 to_float2(float3 v) { return make_float2(v.x, v.y); }
static inline float3 to_float3(float4 v) { return make_float3(v.x, v.y, v.z); }
static inline float4 to_float4(float3 v, float w) { return make_float4(v.x, v.y, v.z, w); }

//**********************************************************************************
// float4 operators and functions
//**********************************************************************************

static inline float4 operator + (const float4 & u, float v) { return make_float4(u.x + v, u.y + v, u.z + v, u.w + v); }
static inline float4 operator - (const float4 & u, float v) { return make_float4(u.x - v, u.y - v, u.z - v, u.w - v); }
static inline float4 operator * (const float4 & u, float v) { return make_float4(u.x * v, u.y * v, u.z * v, u.w * v); }
static inline float4 operator / (const float4 & u, float v) { return make_float4(u.x / v, u.y / v, u.z / v, u.w / v); }

static inline float4 operator + (float v, const float4 & u) { return make_float4(v + u.x, v + u.y, v + u.z, v + u.w); }
static inline float4 operator - (float v, const float4 & u) { return make_float4(v - u.x, v - u.y, v - u.z, v - u.w); }
static inline float4 operator * (float v, const float4 & u) { return make_float4(v * u.x, v * u.y, v * u.z, v * u.w); }
static inline float4 operator / (float v, const float4 & u) { return make_float4(v / u.x, v / u.y, v / u.z, v / u.w); }

static inline float4 operator + (const float4 & u, const float4 & v) { return make_float4(u.x + v.x, u.y + v.y, u.z + v.z, u.w + v.w); }
static inline float4 operator - (const float4 & u, const float4 & v) { return make_float4(u.x - v.x, u.y - v.y, u.z - v.z, u.w - v.w); }
static inline float4 operator * (const float4 & u, const float4 & v) { return make_float4(u.x * v.x, u.y * v.y, u.z * v.z, u.w * v.w); }
static inline float4 operator / (const float4 & u, const float4 & v) { return make_float4(u.x / v.x, u.y / v.y, u.z / v.z, u.w / v.w); }

static inline float4 & operator += (float4 & u, const float4 & v) { u.x += v.x; u.y += v.y; u.z += v.z; u.w += v.w; return u; }
static inline float4 & operator -= (float4 & u, const float4 & v) { u.x -= v.x; u.y -= v.y; u.z -= v.z; u.w -= v.w; return u; }
static inline float4 & operator *= (float4 & u, const float4 & v) { u.x *= v.x; u.y *= v.y; u.z *= v.z; u.w *= v.w; return u; }
static inline float4 & operator /= (float4 & u, const float4 & v) { u.x /= v.x; u.y /= v.y; u.z /= v.z; u.w /= v.w; return u; }

static inline float4 & operator += (float4 & u, float v) { u.x += v; u.y += v; u.z += v; u.w += v; return u; }
static inline float4 & operator -= (float4 & u, float v) { u.x -= v; u.y -= v; u.z -= v; u.w -= v; return u; }
static inline float4 & operator *= (float4 & u, float v) { u.x *= v; u.y *= v; u.z *= v; u.w *= v; return u; }
static inline float4 & operator /= (float4 & u, float v) { u.x /= v; u.y /= v; u.z /= v; u.w /= v; return u; }

static inline float4   operator - (const float4 & v) { return make_float4(-v.x, -v.y, -v.z, -v.w); }

static inline float4 catmullrom(const float4 & P0, const float4 & P1, const float4 & P2, const float4 & P3, float t)
{
  const float ts = t * t;
  const float tc = t * ts;

  return (P0 * (-tc + 2.0f * ts - t) + P1 * (3.0f * tc - 5.0f * ts + 2.0f) + P2 * (-3.0f * tc + 4.0f * ts + t) + P3 * (tc - ts)) * 0.5f;
}

static inline float4 lerp(const float4 & u, const float4 & v, float t) { return u + t * (v - u); }
static inline float  dot(const float4 & u, const float4 & v) { return (u.x*v.x + u.y*v.y + u.z*v.z + u.w*v.w); }
static inline float  dot3(const float4 & u, const float4 & v) { return (u.x*v.x + u.y*v.y + u.z*v.z); }
static inline float  dot3(const float4 & u, const float3 & v) { return (u.x*v.x + u.y*v.y + u.z*v.z); }

static inline float4 clamp(const float4 & u, float a, float b) { return make_float4(clamp(u.x, a, b), clamp(u.y, a, b), clamp(u.z, a, b), clamp(u.w, a, b)); }

static inline float  length3(const float4 & u) { return sqrtf(SQR(u.x) + SQR(u.y) + SQR(u.z)); }
static inline float  length(const float4 & u)  { return sqrtf(SQR(u.x) + SQR(u.y) + SQR(u.z) + SQR(u.w)); }

//inline float4 sqrt   (const float4 & u) { make_float4( sqrt(u.x), sqrt(u.y), sqrt(u.z), sqrt(u.w) ); }

//**********************************************************************************
// float3 operators and functions
//**********************************************************************************

static inline float3 operator + (const float3 & u, float v) { return make_float3(u.x + v, u.y + v, u.z + v); }
static inline float3 operator - (const float3 & u, float v) { return make_float3(u.x - v, u.y - v, u.z - v); }
static inline float3 operator * (const float3 & u, float v) { return make_float3(u.x * v, u.y * v, u.z * v); }
static inline float3 operator / (const float3 & u, float v) { return make_float3(u.x / v, u.y / v, u.z / v); }

static inline float3 operator + (float v, const float3 & u) { return make_float3(v + u.x, v + u.y, v + u.z); }
static inline float3 operator - (float v, const float3 & u) { return make_float3(v - u.x, v - u.y, v - u.z); }
static inline float3 operator * (float v, const float3 & u) { return make_float3(v * u.x, v * u.y, v * u.z); }
static inline float3 operator / (float v, const float3 & u) { return make_float3(v / u.x, v / u.y, v / u.z); }

static inline float3 operator + (const float3 & u, const float3 & v) { return make_float3(u.x + v.x, u.y + v.y, u.z + v.z); }
static inline float3 operator - (const float3 & u, const float3 & v) { return make_float3(u.x - v.x, u.y - v.y, u.z - v.z); }
static inline float3 operator * (const float3 & u, const float3 & v) { return make_float3(u.x * v.x, u.y * v.y, u.z * v.z); }
static inline float3 operator / (const float3 & u, const float3 & v) { return make_float3(u.x / v.x, u.y / v.y, u.z / v.z); }

static inline float3 operator - (const float3 & u) { return make_float3(-u.x, -u.y, -u.z); }

static inline float3 & operator += (float3 & u, const float3 & v) { u.x += v.x; u.y += v.y; u.z += v.z; return u; }
static inline float3 & operator -= (float3 & u, const float3 & v) { u.x -= v.x; u.y -= v.y; u.z -= v.z; return u; }
static inline float3 & operator *= (float3 & u, const float3 & v) { u.x *= v.x; u.y *= v.y; u.z *= v.z; return u; }
static inline float3 & operator /= (float3 & u, const float3 & v) { u.x /= v.x; u.y /= v.y; u.z /= v.z; return u; }

static inline float3 & operator += (float3 & u, float v) { u.x += v; u.y += v; u.z += v; return u; }
static inline float3 & operator -= (float3 & u, float v) { u.x -= v; u.y -= v; u.z -= v; return u; }
static inline float3 & operator *= (float3 & u, float v) { u.x *= v; u.y *= v; u.z *= v; return u; }
static inline float3 & operator /= (float3 & u, float v) { u.x /= v; u.y /= v; u.z /= v; return u; }


static inline float3 catmullrom(const float3 & P0, const float3 & P1, const float3 & P2, const float3 & P3, float t)
{
  const float ts = t * t;
  const float tc = t * ts;

  return (P0 * (-tc + 2.0f * ts - t) + P1 * (3.0f * tc - 5.0f * ts + 2.0f) + P2 * (-3.0f * tc + 4.0f * ts + t) + P3 * (tc - ts)) * 0.5f;
}

static inline float3 lerp(const float3 & u, const float3 & v, float t) { return u + t * (v - u); }
static inline float  dot(const float3 & u, const float3 & v) { return (u.x*v.x + u.y*v.y + u.z*v.z); }
static inline float3 cross(const float3 & u, const float3 & v) { return make_float3(u.y*v.z - u.z*v.y, u.z*v.x - u.x*v.z, u.x*v.y - u.y*v.x); }
//inline float3 mul       (const float3 & u, const float3 & v) { return make_float3( u.x*v.x, u.y*v.y, u.z*v.z} ; return r; }
static inline float3 clamp(const float3 & u, float a, float b) { return make_float3(clamp(u.x, a, b), clamp(u.y, a, b), clamp(u.z, a, b)); }

static inline float  triple(const float3 & a, const float3 & b, const float3 & c) { return dot(a, cross(b, c)); }
static inline float  length(const float3 & u) { return sqrtf(SQR(u.x) + SQR(u.y) + SQR(u.z)); }
static inline float  lengthSquare(const float3 u) { return u.x*u.x + u.y*u.y + u.z*u.z; }
static inline float3 normalize(const float3 & u) { return u / length(u); }
static inline float  coordSumm(const float3 u) { return u.x* +u.y + u.z; }
//static inline float  coordAbsMax (const float3 u) { return max(max(abs(u.x), abs(u.y)), abs(u.z)); }

static inline float  maxcomp(const float3 & u) { return max(u.x, max(u.y, u.z)); }
static inline float  mincomp(const float3 & u) { return min(u.x, min(u.y, u.z)); }


//**********************************************************************************
// float2 operators and functions
//**********************************************************************************

static inline float2 operator + (const float2 & u, float v) { return make_float2(u.x + v, u.y + v); }
static inline float2 operator - (const float2 & u, float v) { return make_float2(u.x - v, u.y - v); }
static inline float2 operator * (const float2 & u, float v) { return make_float2(u.x * v, u.y * v); }
static inline float2 operator / (const float2 & u, float v) { return make_float2(u.x / v, u.y / v); }

static inline float2 operator + (float v, const float2 & u) { return make_float2(v + u.x, v + u.y); }
static inline float2 operator - (float v, const float2 & u) { return make_float2(v - u.x, v - u.y); }
static inline float2 operator * (float v, const float2 & u) { return make_float2(v * u.x, v * u.y); }
static inline float2 operator / (float v, const float2 & u) { return make_float2(v / u.x, v / u.y); }

static inline float2 operator + (const float2 & u, const float2 & v) { return make_float2(u.x + v.x, u.y + v.y); }
static inline float2 operator - (const float2 & u, const float2 & v) { return make_float2(u.x - v.x, u.y - v.y); }
static inline float2 operator * (const float2 & u, const float2 & v) { return make_float2(u.x * v.x, u.y * v.y); }
static inline float2 operator / (const float2 & u, const float2 & v) { return make_float2(u.x / v.x, u.y / v.y); }

static inline float2   operator - (const float2 & v) { return make_float2(-v.x, -v.y); }

static inline float2 & operator += (float2 & u, const float2 & v) { u.x += v.x; u.y += v.y; return u; }
static inline float2 & operator -= (float2 & u, const float2 & v) { u.x -= v.x; u.y -= v.y; return u; }
static inline float2 & operator *= (float2 & u, const float2 & v) { u.x *= v.x; u.y *= v.y; return u; }
static inline float2 & operator /= (float2 & u, const float2 & v) { u.x /= v.x; u.y /= v.y; return u; }

static inline float2 & operator += (float2 & u, float v) { u.x += v; u.y += v; return u; }
static inline float2 & operator -= (float2 & u, float v) { u.x -= v; u.y -= v; return u; }
static inline float2 & operator *= (float2 & u, float v) { u.x *= v; u.y *= v; return u; }
static inline float2 & operator /= (float2 & u, float v) { u.x /= v; u.y /= v; return u; }

static inline float2 catmullrom(const float2 & P0, const float2 & P1, const float2 & P2, const float2 & P3, float t)
{
  const float ts = t * t;
  const float tc = t * ts;

  return (P0 * (-tc + 2.0f * ts - t) + P1 * (3.0f * tc - 5.0f * ts + 2.0f) + P2 * (-3.0f * tc + 4.0f * ts + t) + P3 * (tc - ts)) * 0.5f;
}

static inline float2 lerp(const float2 & u, const float2 & v, float t) { return u + t * (v - u); }
static inline float  dot(const float2 & u, const float2 & v) { return (u.x*v.x + u.y*v.y); }
static inline float2 clamp(const float2 & u, float a, float b) { return make_float2(clamp(u.x, a, b), clamp(u.y, a, b)); }


static inline float  length(const float2 & u) { return sqrtf(SQR(u.x) + SQR(u.y)); }
static inline float2 normalize(const float2 & u) { return u / length(u); }


static inline float lerp(float u, float v, float t) { return u + t * (v - u); }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline bool IntersectBoxBox(float2 box1Min, float2 box1Max, float2 box2Min, float2 box2Max)
{
  return box1Min.x <= box2Max.x && box2Min.x <= box1Max.x &&
         box1Min.y <= box2Max.y && box2Min.y <= box1Max.y;
}

static inline bool IntersectBoxBox(int2 box1Min, int2 box1Max, int2 box2Min, int2 box2Max)
{
  return box1Min.x <= box2Max.x && box2Min.x <= box1Max.x &&
         box1Min.y <= box2Max.y && box2Min.y <= box1Max.y;
}

inline float4 mul(float4x4 m, float4 v)
{
  float4 res;
  res.x = m.row[0].x*v.x + m.row[0].y*v.y + m.row[0].z*v.z + m.row[0].w*v.w;
  res.y = m.row[1].x*v.x + m.row[1].y*v.y + m.row[1].z*v.z + m.row[1].w*v.w;
  res.z = m.row[2].x*v.x + m.row[2].y*v.y + m.row[2].z*v.z + m.row[2].w*v.w;
  res.w = m.row[3].x*v.x + m.row[3].y*v.y + m.row[3].z*v.z + m.row[3].w*v.w;
  return res;
}

inline float4x4 make_float4x4_by_columns(float4 a, float4 b, float4 c, float4 d)
{
  float4x4 m;

  m.row[0].x = a.x;
  m.row[1].x = a.y;
  m.row[2].x = a.z;
  m.row[3].x = a.w;

  m.row[0].y = b.x;
  m.row[1].y = b.y;
  m.row[2].y = b.z;
  m.row[3].y = b.w;

  m.row[0].z = c.x;
  m.row[1].z = c.y;
  m.row[2].z = c.z;
  m.row[3].z = c.w;

  m.row[0].w = d.x;
  m.row[1].w = d.y;
  m.row[2].w = d.z;
  m.row[3].w = d.w;

  return m;
}

inline float4x4 mul(float4x4 m1, float4x4 m2)
{
  const float4 column1 = mul(m1, float4(m2.row[0].x, m2.row[1].x, m2.row[2].x, m2.row[3].x));
  const float4 column2 = mul(m1, float4(m2.row[0].y, m2.row[1].y, m2.row[2].y, m2.row[3].y));
  const float4 column3 = mul(m1, float4(m2.row[0].z, m2.row[1].z, m2.row[2].z, m2.row[3].z));
  const float4 column4 = mul(m1, float4(m2.row[0].w, m2.row[1].w, m2.row[2].w, m2.row[3].w));

  return make_float4x4_by_columns(column1, column2, column3, column4);
}

inline float4x4 inverse4x4(float4x4 m1)
{
  float tmp[12]; // temp array for pairs
  float4x4 m;

  // calculate pairs for first 8 elements (cofactors)
  //
  tmp[0] = m1.row[2].z * m1.row[3].w;
  tmp[1] = m1.row[2].w * m1.row[3].z;
  tmp[2] = m1.row[2].y * m1.row[3].w;
  tmp[3] = m1.row[2].w * m1.row[3].y;
  tmp[4] = m1.row[2].y * m1.row[3].z;
  tmp[5] = m1.row[2].z * m1.row[3].y;
  tmp[6] = m1.row[2].x * m1.row[3].w;
  tmp[7] = m1.row[2].w * m1.row[3].x;
  tmp[8] = m1.row[2].x * m1.row[3].z;
  tmp[9] = m1.row[2].z * m1.row[3].x;
  tmp[10] = m1.row[2].x * m1.row[3].y;
  tmp[11] = m1.row[2].y * m1.row[3].x;

  // calculate first 8 m1.rowents (cofactors)
  //
  m.row[0].x = tmp[0] * m1.row[1].y + tmp[3] * m1.row[1].z + tmp[4] * m1.row[1].w;
  m.row[0].x -= tmp[1] * m1.row[1].y + tmp[2] * m1.row[1].z + tmp[5] * m1.row[1].w;
  m.row[1].x = tmp[1] * m1.row[1].x + tmp[6] * m1.row[1].z + tmp[9] * m1.row[1].w;
  m.row[1].x -= tmp[0] * m1.row[1].x + tmp[7] * m1.row[1].z + tmp[8] * m1.row[1].w;
  m.row[2].x = tmp[2] * m1.row[1].x + tmp[7] * m1.row[1].y + tmp[10] * m1.row[1].w;
  m.row[2].x -= tmp[3] * m1.row[1].x + tmp[6] * m1.row[1].y + tmp[11] * m1.row[1].w;
  m.row[3].x = tmp[5] * m1.row[1].x + tmp[8] * m1.row[1].y + tmp[11] * m1.row[1].z;
  m.row[3].x -= tmp[4] * m1.row[1].x + tmp[9] * m1.row[1].y + tmp[10] * m1.row[1].z;
  m.row[0].y = tmp[1] * m1.row[0].y + tmp[2] * m1.row[0].z + tmp[5] * m1.row[0].w;
  m.row[0].y -= tmp[0] * m1.row[0].y + tmp[3] * m1.row[0].z + tmp[4] * m1.row[0].w;
  m.row[1].y = tmp[0] * m1.row[0].x + tmp[7] * m1.row[0].z + tmp[8] * m1.row[0].w;
  m.row[1].y -= tmp[1] * m1.row[0].x + tmp[6] * m1.row[0].z + tmp[9] * m1.row[0].w;
  m.row[2].y = tmp[3] * m1.row[0].x + tmp[6] * m1.row[0].y + tmp[11] * m1.row[0].w;
  m.row[2].y -= tmp[2] * m1.row[0].x + tmp[7] * m1.row[0].y + tmp[10] * m1.row[0].w;
  m.row[3].y = tmp[4] * m1.row[0].x + tmp[9] * m1.row[0].y + tmp[10] * m1.row[0].z;
  m.row[3].y -= tmp[5] * m1.row[0].x + tmp[8] * m1.row[0].y + tmp[11] * m1.row[0].z;

  // calculate pairs for second 8 m1.rowents (cofactors)
  //
  tmp[0] = m1.row[0].z * m1.row[1].w;
  tmp[1] = m1.row[0].w * m1.row[1].z;
  tmp[2] = m1.row[0].y * m1.row[1].w;
  tmp[3] = m1.row[0].w * m1.row[1].y;
  tmp[4] = m1.row[0].y * m1.row[1].z;
  tmp[5] = m1.row[0].z * m1.row[1].y;
  tmp[6] = m1.row[0].x * m1.row[1].w;
  tmp[7] = m1.row[0].w * m1.row[1].x;
  tmp[8] = m1.row[0].x * m1.row[1].z;
  tmp[9] = m1.row[0].z * m1.row[1].x;
  tmp[10] = m1.row[0].x * m1.row[1].y;
  tmp[11] = m1.row[0].y * m1.row[1].x;

  // calculate second 8 m1 (cofactors)
  //
  m.row[0].z = tmp[0] * m1.row[3].y + tmp[3] * m1.row[3].z + tmp[4] * m1.row[3].w;
  m.row[0].z -= tmp[1] * m1.row[3].y + tmp[2] * m1.row[3].z + tmp[5] * m1.row[3].w;
  m.row[1].z = tmp[1] * m1.row[3].x + tmp[6] * m1.row[3].z + tmp[9] * m1.row[3].w;
  m.row[1].z -= tmp[0] * m1.row[3].x + tmp[7] * m1.row[3].z + tmp[8] * m1.row[3].w;
  m.row[2].z = tmp[2] * m1.row[3].x + tmp[7] * m1.row[3].y + tmp[10] * m1.row[3].w;
  m.row[2].z -= tmp[3] * m1.row[3].x + tmp[6] * m1.row[3].y + tmp[11] * m1.row[3].w;
  m.row[3].z = tmp[5] * m1.row[3].x + tmp[8] * m1.row[3].y + tmp[11] * m1.row[3].z;
  m.row[3].z -= tmp[4] * m1.row[3].x + tmp[9] * m1.row[3].y + tmp[10] * m1.row[3].z;
  m.row[0].w = tmp[2] * m1.row[2].z + tmp[5] * m1.row[2].w + tmp[1] * m1.row[2].y;
  m.row[0].w -= tmp[4] * m1.row[2].w + tmp[0] * m1.row[2].y + tmp[3] * m1.row[2].z;
  m.row[1].w = tmp[8] * m1.row[2].w + tmp[0] * m1.row[2].x + tmp[7] * m1.row[2].z;
  m.row[1].w -= tmp[6] * m1.row[2].z + tmp[9] * m1.row[2].w + tmp[1] * m1.row[2].x;
  m.row[2].w = tmp[6] * m1.row[2].y + tmp[11] * m1.row[2].w + tmp[3] * m1.row[2].x;
  m.row[2].w -= tmp[10] * m1.row[2].w + tmp[2] * m1.row[2].x + tmp[7] * m1.row[2].y;
  m.row[3].w = tmp[10] * m1.row[2].z + tmp[4] * m1.row[2].x + tmp[9] * m1.row[2].y;
  m.row[3].w -= tmp[8] * m1.row[2].y + tmp[11] * m1.row[2].z + tmp[5] * m1.row[2].x;

  // calculate matrix inverse
  //
  float k = 1.0f / (m1.row[0].x * m.row[0].x + m1.row[0].y * m.row[1].x + m1.row[0].z * m.row[2].x + m1.row[0].w * m.row[3].x);

  for (int i = 0; i<4; i++)
  {
    m.row[i].x *= k;
    m.row[i].y *= k;
    m.row[i].z *= k;
    m.row[i].w *= k;
  }

  return m;
}

// Look At matrix creation
// return the inverse view matrix
//
inline float4x4 lookAt(float3 eye, float3 center, float3 up)
{
  float3 x, y, z; // basis; will make a rotation matrix

  z.x = eye.x - center.x;
  z.y = eye.y - center.y;
  z.z = eye.z - center.z;
  z = normalize(z);

  y.x = up.x;
  y.y = up.y;
  y.z = up.z;

  x = cross(y, z); // X vector = Y cross Z
  y = cross(z, x); // Recompute Y = Z cross X

  // cross product gives area of parallelogram, which is < 1.0 for
  // non-perpendicular unit-length vectors; so normalize x, y here
  x = normalize(x);
  y = normalize(y);

  float4x4 M;
  M.row[0].x = x.x; M.row[1].x = x.y; M.row[2].x = x.z; M.row[3].x = -x.x * eye.x - x.y * eye.y - x.z*eye.z;
  M.row[0].y = y.x; M.row[1].y = y.y; M.row[2].y = y.z; M.row[3].y = -y.x * eye.x - y.y * eye.y - y.z*eye.z;
  M.row[0].z = z.x; M.row[1].z = z.y; M.row[2].z = z.z; M.row[3].z = -z.x * eye.x - z.y * eye.y - z.z*eye.z;
  M.row[0].w = 0.0; M.row[1].w = 0.0; M.row[2].w = 0.0; M.row[3].w = 1.0;
  return M;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef ENABLE_SSE

static const __m128 const_255_inv  = _mm_set_ps(1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f);
static const __m128 const_256      = _mm_set_ps(256.0f, 256.0f, 256.0f, 256.0f);
static const __m128 const_255      = _mm_set_ps(255.0f, 255.0f, 255.0f, 255.0f);
static const __m128 const_2222     = _mm_set_ps(2.0f, 2.0f, 2.0f, 2.0f);
static const __m128 const_1111     = _mm_set_ps(1.0f, 1.0f, 1.0f, 1.0f);
static const __m128 const_0000     = _mm_set_ps(0.0f, 0.0f, 0.0f, 0.0f);
static const __m128 const_half_one = _mm_set_ps(0.5f, 0.5f, 0.5f, 0.5f);

static const __m128i const_maskBGRA = _mm_set_epi32(0xFF000000, 0x000000FF, 0x0000FF00, 0x00FF0000);
static const __m128i const_shftBGRA = _mm_set_epi32(24, 0, 8, 16);
static const __m128i const_maskW    = _mm_set_epi32(0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000);
static const __m128i const_maskXYZ  = _mm_set_epi32(0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);

#endif

inline static int RealColorToUint32_BGRA(float4 real_color)
{

#ifndef ENABLE_SSE

  const float r = real_color.x*255.0f;
  const float g = real_color.y*255.0f;
  const float b = real_color.z*255.0f;
  const float a = real_color.w*255.0f;

  const unsigned char red   = (unsigned char)r;
  const unsigned char green = (unsigned char)g;
  const unsigned char blue  = (unsigned char)b;
  const unsigned char alpha = (unsigned char)a;

  return blue | (green << 8) | (red << 16) | (alpha << 24);

#else

  const __m128  const_255 = _mm_set_ps(255.0f, 255.0f, 255.0f, 255.0f);
  const __m128  rel_col   = _mm_set_ps(real_color.w, real_color.x, real_color.y, real_color.z);
  const __m128i rgba      = _mm_cvtps_epi32(_mm_mul_ps(rel_col, const_255));

  const __m128i out       = _mm_packus_epi32(rgba, _mm_setzero_si128());
  const __m128i out2      = _mm_packus_epi16(out,  _mm_setzero_si128());

  return _mm_cvtsi128_si32(out2);

#endif

}


inline static float4 Uint32_BGRAToRealColor(int packedColor)
{
  const int red   = (packedColor & 0x00FF0000) >> 16;
  const int green = (packedColor & 0x0000FF00) >> 8;
  const int blue  = (packedColor & 0x000000FF) >> 0;
  const int alpha = (packedColor & 0xFF000000) >> 24;

  return float4((float)red, (float)green, (float)blue, (float)alpha)*(1.0f / 255.0f);
}


#ifdef ENABLE_SSE

inline static __m128 Uint32_BGRAToRealColor_SSE(int* ptr, int offset)
{
  //const __m128  data     = _mm_load_ss((const float*)(ptr + offset));
  //const __m128i pixeliSS = _mm_castps_si128(_mm_shuffle_ps(data, data, _MM_SHUFFLE(0, 0, 0, 0)));

  const int packedColor = ptr[offset];
  const int red   = (packedColor & 0x00FF0000) >> 16;
  const int green = (packedColor & 0x0000FF00) >> 8;
  const int blue  = (packedColor & 0x000000FF) >> 0;
  const int alpha = (packedColor & 0xFF000000) >> 24;

  return _mm_mul_ps(_mm_cvtepi32_ps(_mm_set_epi32(alpha, red, green, blue)), const_255_inv);
}

inline static int RealColorToUint32_BGRA(const __m128 rel_col)
{
  const __m128i rgba = _mm_cvtps_epi32(_mm_mul_ps(rel_col, const_255));
  const __m128i out  = _mm_packus_epi32(rgba, _mm_setzero_si128());
  const __m128i out2 = _mm_packus_epi16(out,  _mm_setzero_si128());

  return _mm_cvtsi128_si32(out2);
}

#endif


#include <string>
#include <sstream>

static std::string ToString(int i)
{
  std::stringstream out;
  out << i;
  return out.str();
}

static std::string ToString(float i)
{
  std::stringstream out;
  out << i;
  return out.str();
}

static std::string ToString(unsigned int i)
{
  std::stringstream out;
  out << i;
  return out.str();
}

static void RunTimeError(const char* file, int line, std::string msg)
{
  // throw std::runtime_error(std::string("Run time error at ") + file + std::string(", line ") + ToString(line) + ": " + msg);
}

#undef  RUN_TIME_ERROR
#define RUN_TIME_ERROR(e) (RunTimeError(__FILE__,__LINE__,(e)))

#undef  RUN_TIME_ERROR_AT
#define RUN_TIME_ERROR_AT(e, file, line) (RunTimeError((file),(line),(e)))



#undef  ASSERT

#ifdef  NDEBUG
  #define ASSERT(_expression) ((void)0)
#else
  #define ASSERT(_expression) if(!(_expression)) RUN_TIME_ERROR_AT("Assertion Failed", __FILE__, __LINE__)
#endif

