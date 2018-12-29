//
// Created by frol on 24.08.18.
//

#ifndef TEST_GL_TOP_RASTEROPERATIONS_FIXP_H
#define TEST_GL_TOP_RASTEROPERATIONS_FIXP_H


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

template<typename vint>
struct LineOffs<vint, 4>
{
  static inline vint w(const int CX1, const int FDY12)
  {
    return make_vuint((unsigned int)CX1, (unsigned int)(CX1 - FDY12), (unsigned int)(CX1 - FDY12*2), (unsigned int)(CX1 - FDY12*3)) >> 16;
  }

};


template<typename TriangleT, typename vfloat, typename vint, int width, bool bilinearIsEnabled>
struct VROP_FIXP
{
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  struct FillColor
  {
    enum { n = width };
    using Triangle = TriangleT;

    static inline vint Line(const TriangleT &tri)
    {
      return splat(0xFFFFFFFF);
    }

    static inline void store_line(int *line, vint data)
    {
      store_u(line, data);
    }

  };

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  struct Colored2D
  {
    enum { n = width };
    using Triangle = TriangleT;

    static inline void Line(const TriangleT &tri, const int CX1, const int CX2, const int FDY12, const int FDY23, const unsigned int areaInv,
                            int *pLineColor)
    {
      const vint w1 = (areaInv * LineOffs<vint, n>::w(CX1, FDY12)) >> 8;
      const vint w2 = (areaInv * LineOffs<vint, n>::w(CX2, FDY23)) >> 8;
      const vint w3 = (splat((unsigned int)255) - w1 - w2);

      //const vint r = ( (tri.c1.x)*w1 + (tri.c2.x)*w2 + (tri.c3.x)*w3 ) >> 8;
      //const vint g = ( (tri.c1.y)*w1 + (tri.c2.y)*w2 + (tri.c3.y)*w3 ) >> 8;
      //const vint b = ( (tri.c1.z)*w1 + (tri.c2.z)*w2 + (tri.c3.z)*w3 ) >> 8;

      const vint r = ((255) * w1 + (0  ) * w2 + (0  ) * w3) >> 8;
      const vint g = ((0  ) * w1 + (255) * w2 + (0  ) * w3) >> 8;
      const vint b = ((0  ) * w1 + (0  ) * w2 + (255) * w3) >> 8;

      const vint res = (r << 16) | // BGRA
                       (g << 8)  |
                       (b << 0);

      store_u(pLineColor, res);
    }

    static inline int Pixel(const TriangleT &tri, const int CX1, const int CX2, const float areaInv)
    {
      return 0xFFFFFFFF;
    }

  };

};


#endif //TEST_GL_TOP_RASTEROPERATIONS_FIXP_H
