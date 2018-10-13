//
// Created by frol on 24.08.18.
//

#ifndef TEST_GL_TOP_RASTEROPERATIONS_H
#define TEST_GL_TOP_RASTEROPERATIONS_H

#define  	SIMDPP_ARCH_X86_AVX
#include "../simdpp/simd.h"

inline static unsigned int RealColorToUint32_BGRA_SIMD(const simdpp::float32<4>& real_color)
{
  static const simdpp::float32<4> const_255 = simdpp::make_float(255.0f);
  static const simdpp::uint32<4>  shiftmask = simdpp::make_int(16,8,0,24);
  const simdpp::uint32<4>         rgbai     = simdpp::to_int32(real_color*const_255);
  return simdpp::reduce_or(simdpp::shift_l(rgbai, shiftmask)); // return blue | (green << 8) | (red << 16) | (alpha << 24);
}

template<int DIM, typename TriangleType>
struct VROP                  // Vectorizeable Raster OPerations
{
  using Scalar   = simdpp::float32<DIM>;
  using Scalarui = simdpp::uint32<DIM>;

  SIMDPP_ALIGN(64) struct vec4
  {
    Scalar x,y,z,w;
  };

  static inline vec4 splat_v4(const float4& v)
  {
    vec4 res;
    res.x = simdpp::splat(v.x);
    res.y = simdpp::splat(v.y);
    res.z = simdpp::splat(v.z);
    res.w = simdpp::splat(v.w);
    return res;
  }

  static inline Scalarui RealColorToUint32_BGRA(const vec4& rc)
  {
    const Scalar const_255 = simdpp::splat(255.0f);
    const Scalar r = rc.x*const_255;
    const Scalar g = rc.y*const_255;
    const Scalar b = rc.z*const_255;
    const Scalar a = rc.w*const_255;

    const auto red   = simdpp::to_uint32(r);
    const auto green = simdpp::to_uint32(g);
    const auto blue  = simdpp::to_uint32(b);
    const auto alpha = simdpp::to_uint32(a);

    return simdpp::bit_or(blue, simdpp::bit_or(simdpp::shift_l(green, 8), simdpp::bit_or(simdpp::shift_l(red,  16), simdpp::shift_l(alpha,24)) ));
  }

  struct FillColor
  {
    inline static vec4 DrawPixel(const TriangleType& tri,
                                 const Scalar& w0, const Scalar& w1, const Scalar& w2)
    {
      return splat_v4(tri.c1);
    }

    inline static vec4 DrawPixel(const vec4& tri_c1, const vec4& tri_c2, const vec4& tri_c3,
                                 const Scalar& w0, const Scalar& w1, const Scalar& w2)
    {
      vec4 res;
      res.x = tri_c1.x;
      res.y = tri_c1.y;
      res.z = tri_c1.z;
      res.w = tri_c1.w;
      return res;
    }
  };

  struct Colored2D
  {
    inline static vec4 DrawPixel(const TriangleType& tri,
                                 const Scalar& w0, const Scalar& w1, const Scalar& w2)
    {
      const auto tri_c1 = splat_v4(tri.c1);
      const auto tri_c2 = splat_v4(tri.c2);
      const auto tri_c3 = splat_v4(tri.c3);

      vec4 res;
      res.x = tri_c1.x*w0 + tri_c2.x*w1 + tri_c3.x*w2;
      res.y = tri_c1.y*w0 + tri_c2.y*w1 + tri_c3.y*w2;
      res.z = tri_c1.z*w0 + tri_c2.z*w1 + tri_c3.z*w2;
      res.w = tri_c1.w*w0 + tri_c2.w*w1 + tri_c3.w*w2;
      return res;
    }

    inline static vec4 DrawPixel(const vec4& tri_c1, const vec4& tri_c2, const vec4& tri_c3,
                                 const Scalar& w0, const Scalar& w1, const Scalar& w2)
    {
      vec4 res;
      res.x = tri_c1.x*w0 + tri_c2.x*w1 + tri_c3.x*w2;
      res.y = tri_c1.y*w0 + tri_c2.y*w1 + tri_c3.y*w2;
      res.z = tri_c1.z*w0 + tri_c2.z*w1 + tri_c3.z*w2;
      res.w = tri_c1.w*w0 + tri_c2.w*w1 + tri_c3.w*w2;
      return res;
    }
  };

  struct Colored3D
  {
    inline static vec4 DrawPixel(const TriangleType& tri,
                                 const Scalar& w0, const Scalar& w1, const Scalar& w2, const Scalar& zInv)
    {
      const Scalar z    = simdpp::rcp_e(zInv);

      const auto tri_c1 = splat_v4(tri.c1);
      const auto tri_c2 = splat_v4(tri.c2);
      const auto tri_c3 = splat_v4(tri.c3);

      vec4 res;
      res.x = ( tri_c1.x*w0 + tri_c2.x*w1 + tri_c3.x*w2 )*z;
      res.y = ( tri_c1.y*w0 + tri_c2.y*w1 + tri_c3.y*w2 )*z;
      res.z = ( tri_c1.z*w0 + tri_c2.z*w1 + tri_c3.z*w2 )*z;
      res.w = ( tri_c1.w*w0 + tri_c2.w*w1 + tri_c3.w*w2 )*z;
      return res;
    }

    inline static vec4 DrawPixel(const vec4& tri_c1, const vec4& tri_c2, const vec4& tri_c3,
                                 const Scalar& w0, const Scalar& w1, const Scalar& w2, const Scalar& zInv)
    {
      const Scalar z = simdpp::rcp_e(zInv);
      vec4 res;
      res.x = z*( tri_c1.x*w0 + tri_c2.x*w1 + tri_c3.x*w2 );
      res.y = z*( tri_c1.y*w0 + tri_c2.y*w1 + tri_c3.y*w2 );
      res.z = z*( tri_c1.z*w0 + tri_c2.z*w1 + tri_c3.z*w2 );
      res.w = z*( tri_c1.w*w0 + tri_c2.w*w1 + tri_c3.w*w2 );
      return res;
    }
  };

};


template<typename TriangleType>
struct SROP
{
  struct FillColor
  {
    inline static simdpp::float32<4> DrawPixel(const TriangleType& tri, const simdpp::float32<4>& w1234)
    {
      return simdpp::load(&tri.c1);
    }
  };

  struct Colored2D
  {
    inline static simdpp::float32<4> DrawPixel(const TriangleType& tri, const simdpp::float32<4>& w1234)
    {
      const simdpp::float32<4> tv1 = simdpp::load(&tri.c1);
      const simdpp::float32<4> tv2 = simdpp::load(&tri.c2);
      const simdpp::float32<4> tv3 = simdpp::load(&tri.c3);
      return tv1*simdpp::splat<0>(w1234) + tv2*simdpp::splat<2>(w1234) + tv3*simdpp::splat<1>(w1234);
    }
  };

  struct Colored3D
  {
    inline static simdpp::float32<4> DrawPixel(const TriangleType& tri, const simdpp::float32<4>& w1234, const simdpp::float32<4>& zInv)
    {
      const auto z = simdpp::rcp_e(zInv);
      const simdpp::float32<4> tv1 = simdpp::load(&tri.c1);
      const simdpp::float32<4> tv2 = simdpp::load(&tri.c2);
      const simdpp::float32<4> tv3 = simdpp::load(&tri.c3);
      return ( tv1*simdpp::splat<0>(w1234) + tv2*simdpp::splat<2>(w1234) + tv3*simdpp::splat<1>(w1234) )*z;
    }
  };

};




#endif //TEST_GL_TOP_RASTEROPERATIONS_H
