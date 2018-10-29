//
// Created by frol on 24.08.18.
//

#ifndef TEST_GL_TOP_RASTEROPERATIONS_H
#define TEST_GL_TOP_RASTEROPERATIONS_H

#include "vfloat4.h"

template<typename TriangleT, int width>
struct VROP_CVEX
{
};


template<typename TriangleT>
struct VROP_CVEX<TriangleT,4>
{
  static constexpr cvex::vfloat4 c_one = {1.0f,1.0f,1.0f,1.0f};
  static constexpr cvex::vfloat4 c_255 = {255.0f, 255.0f, 255.0f, 255.0f};

  struct Colored2D
  {
    enum {n = 4};
    using Triangle = TriangleT;

    static inline cvex::vint4 Line(const TriangleT& tri, const int CX1, const int CX2, const int FDY12, const int FDY23, const float areaInv,
                                   int* pLineColor)
    {
      const auto w1 = areaInv*cvex::to_vfloat( cvex::make_vint4(CX1, CX1-FDY12, CX1 - 2*FDY12,CX1 - 3*FDY12) );
      const auto w2 = areaInv*cvex::to_vfloat( cvex::make_vint4(CX2, CX2-FDY23, CX2 - 2*FDY23,CX2 - 3*FDY23) );
      const auto w3 = (c_one - w1 - w2);

      //const auto c1 = cvex::load((const float*)&tri.c1);
      //const auto c2 = cvex::load((const float*)&tri.c2);
      //const auto c3 = cvex::load((const float*)&tri.c3);

      //const auto r = cvex::splat_0(c1)*w1 + cvex::splat_0(c2)*w2 + cvex::splat_0(c3)*w3;
      //const auto g = cvex::splat_1(c1)*w1 + cvex::splat_1(c2)*w2 + cvex::splat_1(c3)*w3;
      //const auto b = cvex::splat_2(c1)*w1 + cvex::splat_2(c2)*w2 + cvex::splat_2(c3)*w3;
      //const auto a = cvex::splat_3(c1)*w1 + cvex::splat_3(c2)*w2 + cvex::splat_3(c3)*w3;

      const auto r = tri.c1.x*w1 + tri.c2.x*w2 + tri.c3.x*w3;
      const auto g = tri.c1.y*w1 + tri.c2.y*w2 + tri.c3.y*w3;
      const auto b = tri.c1.z*w1 + tri.c2.z*w2 + tri.c3.z*w3;
      const auto a = tri.c1.w*w1 + tri.c2.w*w2 + tri.c3.w*w3;

      const cvex::vint4 res = (cvex::to_vint(r*c_255) << 16) | // BGRA
                              (cvex::to_vint(g*c_255) << 8)  |
                              (cvex::to_vint(b*c_255) << 0)  |
                              (cvex::to_vint(a*c_255) << 24);

      cvex::store_u(pLineColor, res);
    }

    static inline int Pixel(const TriangleT& tri, const int CX1, const int CX2, const float areaInv)
    {
      const float w1 = areaInv*float(CX1);
      const float w2 = areaInv*float(CX2);

      // const float4 c = tri.c1*w1 + tri.c2*w2 + tri.c3*(1.0f - w1 - w2);
      // return RealColorToUint32_BGRA(c);

      const cvex::vfloat4 c1 = cvex::load((const float*)&tri.c1);
      const cvex::vfloat4 c2 = cvex::load((const float*)&tri.c2);
      const cvex::vfloat4 c3 = cvex::load((const float*)&tri.c3);
      const cvex::vfloat4 c  = c1*w1 + c2*w2 + c3*(1.0f - w1 - w2);
      return cvex::color_compress_bgra(c);
    }

  };

  struct Colored3D
  {
    enum {n = 4};
    using Triangle = TriangleT;

    static inline void Line(const TriangleT& tri, const int CX1, const int CX2, const int FDY12, const int FDY23, const float areaInv,
                            int* pLineColor, float* pLineDepth)
    {
      const auto w1 = areaInv*cvex::to_vfloat( cvex::make_vint4(CX1, CX1-FDY12, CX1 - 2*FDY12,CX1 - 3*FDY12) );
      const auto w2 = areaInv*cvex::to_vfloat( cvex::make_vint4(CX2, CX2-FDY23, CX2 - 2*FDY23,CX2 - 3*FDY23) );
      const auto w3 = (c_one - w1 - w2);

      const auto zInv  = tri.v1.z*w1 + tri.v2.z*w2 + tri.v3.z*w3;
      const auto zOld  = cvex::load_u(pLineDepth);
      const cvex::vint4 zTest = (zInv > zOld);

      if(cvex::cmp_test_any(zTest))
      {
        const auto z = cvex::rcp_e(zInv);

        const auto r = (tri.c1.x * w1 + tri.c2.x * w2 + tri.c3.x * w3)*z;
        const auto g = (tri.c1.y * w1 + tri.c2.y * w2 + tri.c3.y * w3)*z;
        const auto b = (tri.c1.z * w1 + tri.c2.z * w2 + tri.c3.z * w3)*z;
        const auto a = (tri.c1.w * w1 + tri.c2.w * w2 + tri.c3.w * w3)*z;

        const cvex::vint4 colorOld = cvex::load_u(pLineColor);

        const auto colori = (cvex::to_vint(r * c_255) << 16) | // BGRA
                            (cvex::to_vint(g * c_255) << 8) |
                            (cvex::to_vint(b * c_255) << 0) |
                            (cvex::to_vint(a * c_255) << 24);

        cvex::store_u(pLineColor, cvex::blend(colori, colorOld, zTest));
        cvex::store_u(pLineDepth, cvex::blend(zInv,   zOld,     zTest));
      }
    }


    static inline void Pixel(const TriangleT& tri, const int CX1, const int CX2, const float areaInv,
                             int* pPixelColor, float* pPixelDepth)
    {
      const float w1 = areaInv*float(CX1);
      const float w2 = areaInv*float(CX2);

      const float zInv = tri.v1.z*w1 + tri.v2.z*w2 + tri.v3.z*(1.0f - w1 - w2);
      const float zOld = (*pPixelDepth);

      if (zInv > zOld)
      {
        const float z = 1.0f/zInv;

        // const float4 c = tri.c1*w1 + tri.c2*w2 + tri.c3*(1.0f - w1 - w2);
        // RealColorToUint32_BGRA(c);

        const cvex::vfloat4 c1  = cvex::load((const float *) &tri.c1);
        const cvex::vfloat4 c2  = cvex::load((const float *) &tri.c2);
        const cvex::vfloat4 c3  = cvex::load((const float *) &tri.c3);
        const cvex::vfloat4 col = (c1 * w1 + c2 * w2 + c3 * (1.0f - w1 - w2))*z;

        (*pPixelColor) = cvex::color_compress_bgra(col);
        (*pPixelDepth) = zInv;
      }
    }

  };


};



#endif //TEST_GL_TOP_RASTEROPERATIONS_H
