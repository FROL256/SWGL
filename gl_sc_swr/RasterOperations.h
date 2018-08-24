//
// Created by frol on 24.08.18.
//

#ifndef TEST_GL_TOP_RASTEROPERATIONS_H
#define TEST_GL_TOP_RASTEROPERATIONS_H

#define  	SIMDPP_ARCH_X86_SSE4_1
#include "../simdpp/simd.h"


template<int DIM>
struct VROP                  // Vectorizeable Raster OPerations
{
  using Scalar   = simdpp::float32<DIM>;
  using Scalarui = simdpp::uint32<DIM>;

  SIMDPP_ALIGN(64) struct vec4
  {
    Scalar x,y,z,w;
  };

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
    inline static void DrawPixel(const vec4& tri_c1, const vec4& tri_c2, const vec4& tri_c3,
                                 const Scalar& w0, const Scalar& w1, const Scalar& w2,
                                 vec4& res)
    {
      res.x = tri_c1.x;
      res.y = tri_c1.y;
      res.z = tri_c1.z;
      res.w = tri_c1.w;
    }
  };

  struct Colored2D
  {
    inline static vec4 DrawPixel(const vec4& tri_c1, const vec4& tri_c2, const vec4& tri_c3,
                                 const Scalar& w0, const Scalar& w1, const Scalar& w2,
                                 vec4& res)
    {
      res.x = tri_c1.x*w0 + tri_c2.x*w1 + tri_c3.x*w2;
      res.y = tri_c1.y*w0 + tri_c2.y*w1 + tri_c3.y*w2;
      res.z = tri_c1.z*w0 + tri_c2.z*w1 + tri_c3.z*w2;
      res.w = tri_c1.w*w0 + tri_c2.w*w1 + tri_c3.w*w2;
    }
  };

  struct Colored3D
  {
    inline static void DrawPixel(const vec4& tri_c1, const vec4& tri_c2, const vec4& tri_c3,
                                 const Scalar& w0, const Scalar& w1, const Scalar& w2, const Scalar zInv,
                                 vec4& res)
    {
      const Scalar z = simdpp::rcp_e(zInv);
      res.x = z*( tri_c1.x*w0 + tri_c2.x*w1 + tri_c3.x*w2 );
      res.y = z*( tri_c1.y*w0 + tri_c2.y*w1 + tri_c3.y*w2 );
      res.z = z*( tri_c1.z*w0 + tri_c2.z*w1 + tri_c3.z*w2 );
      res.w = z*( tri_c1.w*w0 + tri_c2.w*w1 + tri_c3.w*w2 );
    }
  };

};


#endif //TEST_GL_TOP_RASTEROPERATIONS_H
