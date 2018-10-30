//
// Created by frol on 24.08.18.
//

#ifndef TEST_GL_TOP_RASTEROPERATIONS_H
#define TEST_GL_TOP_RASTEROPERATIONS_H


#ifndef WIN32
#pragma GCC optimize ("unroll-loops")
#endif

template<typename vint, int n>
struct LineOffs
{
  static inline vint w(const int CX1, const int FDY12)
  {
    ALIGNED(n*4) int w1i[n];

    #pragma GCC ivdep
    for(int i=0;i<n;i++)
      w1i[i] = CX1 - i*FDY12;

    return (vint)load(w1i);
  }
};

// template<typename vint>
// struct LineOffs<vint, 4>
// {
//   static inline vint w(const int CX1, const int FDY12)
//   {
//     return vint{CX1, CX1 - FDY12, CX1 - FDY12*2, CX1 - FDY12*3};
//   }
// };
//
// template<typename vint>
// struct LineOffs<vint, 8>
// {
//   static inline vint w(const int CX1, const int FDY12)
//   {
//     return vint{CX1, CX1 - FDY12, CX1 - FDY12*2, CX1 - FDY12*3, CX1 - FDY12*4, CX1 - FDY12*5, CX1 - FDY12*6, CX1 - FDY12*7};
//   }
// };




template<typename TriangleT, typename vfloat, typename vint, int width>
struct VROP
{

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  struct Colored2D
  {
    enum {n = width};
    using Triangle = TriangleT;

    static inline void Line(const TriangleT& tri, const int CX1, const int CX2, const int FDY12, const int FDY23, const float areaInv,
                            int* pLineColor)
    {
      const vfloat c_one = splat(1.0f);
      const vfloat c_255 = splat(255.0f);

      const vfloat w1 = areaInv*to_float32( LineOffs<vint,n>::w(CX1, FDY12) );
      const vfloat w2 = areaInv*to_float32( LineOffs<vint,n>::w(CX2, FDY23) );
      const vfloat w3 = (c_one - w1 - w2);

      const vfloat r = tri.c1.x*w1 + tri.c2.x*w2 + tri.c3.x*w3;
      const vfloat g = tri.c1.y*w1 + tri.c2.y*w2 + tri.c3.y*w3;
      const vfloat b = tri.c1.z*w1 + tri.c2.z*w2 + tri.c3.z*w3;
      const vfloat a = tri.c1.w*w1 + tri.c2.w*w2 + tri.c3.w*w3;

      const vint res = (to_int32(r * c_255) << 16) | // BGRA
                       (to_int32(g * c_255) << 8)  |
                       (to_int32(b * c_255) << 0)  |
                       (to_int32(a * c_255) << 24);

      store_u(pLineColor, res);
    }

    static inline int Pixel(const TriangleT& tri, const int CX1, const int CX2, const float areaInv)
    {
      const float w1 = areaInv*float(CX1);
      const float w2 = areaInv*float(CX2);
      const float4 c = tri.c1*w1 + tri.c2*w2 + tri.c3*(1.0f - w1 - w2);
      return RealColorToUint32_BGRA(c);
    }

  };

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  struct Colored3D
  {
    enum {n = 4};
    using Triangle = TriangleT;

    static inline void Line(const TriangleT& tri, const int CX1, const int CX2, const int FDY12, const int FDY23, const float areaInv,
                            int* pLineColor, float* pLineDepth)
    {
      const vfloat c_one = splat(1.0f);
      const vfloat c_255 = splat(255.0f);

      const vfloat w1 = areaInv* to_float32( LineOffs<vint,n>::w(CX1, FDY12) );
      const vfloat w2 = areaInv* to_float32( LineOffs<vint,n>::w(CX2, FDY23) );
      const vfloat w3 = (c_one - w1 - w2);

      const vfloat zInv  = tri.v1.z*w1 + tri.v2.z*w2 + tri.v3.z*w3;
      const vfloat zOld  = load_u(pLineDepth);
      const vint   zTest = (zInv > zOld);

      if(test_bits_any(zTest))
      {
        const auto z = rcp_e(zInv);

        const auto r = (tri.c1.x * w1 + tri.c2.x * w2 + tri.c3.x * w3)*z;
        const auto g = (tri.c1.y * w1 + tri.c2.y * w2 + tri.c3.y * w3)*z;
        const auto b = (tri.c1.z * w1 + tri.c2.z * w2 + tri.c3.z * w3)*z;
        const auto a = (tri.c1.w * w1 + tri.c2.w * w2 + tri.c3.w * w3)*z;

        const vint colorOld = load_u(pLineColor);

        //const vint colori = splat(int(0xFFFFFFFF));

        //const vint ri = splat(255); // to_int32(r * c_255);
        //const vint gi = splat(0); // to_int32(g * c_255);
        //const vint bi = splat(255); // to_int32(b * c_255);
        //const vint ai = splat(0); // to_int32(a * c_255);
        //const vint colori = (ri << 16) | (gi << 8) | bi | (ai << 24);

        const vint colori = (to_int32(r * c_255) << 16) | // BGRA
                            (to_int32(g * c_255) << 8)  |
                            (to_int32(b * c_255) << 0)  |
                            (to_int32(a * c_255) << 24);

        store_u(pLineColor, blend(colori, colorOld, zTest));
        store_u(pLineDepth, blend(zInv,   zOld,     zTest));
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
        const float  z = 1.0f/zInv;
        const float4 c = (tri.c1*w1 + tri.c2*w2 + tri.c3*(1.0f - w1 - w2))*z;
        (*pPixelColor) = RealColorToUint32_BGRA(c);
        (*pPixelDepth) = zInv;
      }
    }

  };


};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*

template<typename TriangleT>
struct VROP<TriangleT, cvex::vfloat4, cvex::vint4,4>
{
  static constexpr cvex::vfloat4 c_one = {1.0f,   1.0f,   1.0f,   1.0f};
  static constexpr cvex::vfloat4 c_255 = {255.0f, 255.0f, 255.0f, 255.0f};

  struct Colored2D
  {
    enum {n = 4};
    using Triangle = TriangleT;

    static inline void Line(const TriangleT& tri, const int CX1, const int CX2, const int FDY12, const int FDY23, const float areaInv,
                            int* pLineColor)
    {
      const auto w1 = areaInv* cvex::to_float32(cvex::make_vint4(CX1, CX1 - FDY12, CX1 - 2 * FDY12, CX1 - 3 * FDY12));
      const auto w2 = areaInv* cvex::to_float32(cvex::make_vint4(CX2, CX2 - FDY23, CX2 - 2 * FDY23, CX2 - 3 * FDY23));
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

      const cvex::vint4 res = (cvex::to_int32(r * c_255) << 16) | // BGRA
                              (cvex::to_int32(g * c_255) << 8)  |
                              (cvex::to_int32(b * c_255) << 0)  |
                              (cvex::to_int32(a * c_255) << 24);

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
      const auto w1 = areaInv* cvex::to_float32(cvex::make_vint4(CX1, CX1 - FDY12, CX1 - 2 * FDY12, CX1 - 3 * FDY12));
      const auto w2 = areaInv* cvex::to_float32(cvex::make_vint4(CX2, CX2 - FDY23, CX2 - 2 * FDY23, CX2 - 3 * FDY23));
      const auto w3 = (c_one - w1 - w2);

      const auto zInv  = tri.v1.z*w1 + tri.v2.z*w2 + tri.v3.z*w3;
      const auto zOld  = cvex::load_u(pLineDepth);
      const cvex::vint4 zTest = (zInv > zOld);

      if(cvex::test_any(zTest))
      {
        const auto z = cvex::rcp_e(zInv);

        const auto r = (tri.c1.x * w1 + tri.c2.x * w2 + tri.c3.x * w3)*z;
        const auto g = (tri.c1.y * w1 + tri.c2.y * w2 + tri.c3.y * w3)*z;
        const auto b = (tri.c1.z * w1 + tri.c2.z * w2 + tri.c3.z * w3)*z;
        const auto a = (tri.c1.w * w1 + tri.c2.w * w2 + tri.c3.w * w3)*z;

        const cvex::vint4 colorOld = cvex::load_u(pLineColor);

        const auto colori = (cvex::to_int32(r * c_255) << 16) | // BGRA
                            (cvex::to_int32(g * c_255) << 8) |
                            (cvex::to_int32(b * c_255) << 0) |
                            (cvex::to_int32(a * c_255) << 24);

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

*/

#endif //TEST_GL_TOP_RASTEROPERATIONS_H
