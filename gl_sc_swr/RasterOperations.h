//
// Created by frol on 24.08.18.
//

#ifndef TEST_GL_TOP_RASTEROPERATIONS_H
#define TEST_GL_TOP_RASTEROPERATIONS_H

namespace simdpp
{
  //float splat(const float x) { return x; }
  static inline float rcp_e(const float x) {return 1.0f/x; }
}

template<typename Scalar>
struct VROP                  // Vectorizeable Raster OPerations
{
  struct vec4
  {
    Scalar x,y,z,w;
  };

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
