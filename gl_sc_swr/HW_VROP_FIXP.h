//
// Created by frol on 24.08.18.
//

#ifndef TEST_GL_TOP_RASTEROPERATIONS_FIXP_H
#define TEST_GL_TOP_RASTEROPERATIONS_FIXP_H

#include <cstdint>

#ifndef WIN32
#pragma GCC optimize ("unroll-loops")
#endif

template<typename vint, int n>
struct TileOp
{
  static inline vint w(const int CX1, const int FDY12, const int FDX12)
  {
    CVEX_ALIGNED(n*4) int w1i[n];

    #pragma GCC ivdep
    for(int i=0;i<n;i++)
      w1i[i] = CX1 - i*FDY12;

    return (vint)load(w1i);
  }
};

template<typename vint>
struct TileOp<vint, 4>
{
  static inline vint w(const int CX1, const int FDY12, const int FDX12)
  {
    return vint{ (int)CX1, 
                 (int)(CX1 - FDY12), 
                 (int)(CX1 - FDY12*2), 
                 (int)(CX1 - FDY12*3) >> 16}; // #TODO: conver this to vuint16
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
    using ROPType  = uint32_t;

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
    using ROPType  = uint32_t;

    static inline void Block(const TriangleT &tri, const int CX1, const int CX2, const int FDY12, const int FDY23, const int FDX12, const int FDX23, const unsigned int areaInv,
                             int *pLineColor)
    {
      const vint w1 = (areaInv * TileOp<vint, n>::w(CX1, FDY12, FDX12)) >> 8;
      const vint w2 = (areaInv * TileOp<vint, n>::w(CX2, FDY23, FDX23)) >> 8;
      const vint w3 = (splat((unsigned int)255) - w1 - w2);

      const vint r = ((255) * w1 + (0  ) * w2 + (0  ) * w3) >> 8;
      const vint g = ((0  ) * w1 + (255) * w2 + (0  ) * w3) >> 8;
      const vint b = ((0  ) * w1 + (0  ) * w2 + (255) * w3) >> 8;

      const vint res = (r << 16) | // BGRA
                       (g << 8)  |
                       (b << 0);

      store_u(pLineColor, res);
    }

    static inline unsigned int Pixel(const TriangleT &tri, const int CX1, const int CX2, const unsigned int areaInv)
    {
      const unsigned int w1 = (areaInv * (CX1 >> 16)) >> 8;
      const unsigned int w2 = (areaInv * (CX2 >> 16)) >> 8;
      const unsigned int w3 = ((unsigned int)255 - w1 - w2);

      const unsigned int r = ((255) * w1 + (0  ) * w2 + (0  ) * w3) >> 8;
      const unsigned int g = ((0  ) * w1 + (255) * w2 + (0  ) * w3) >> 8;
      const unsigned int b = ((0  ) * w1 + (0  ) * w2 + (255) * w3) >> 8;

      return (r << 16) | (g << 8) | (b << 0);
    }

  };

};


#endif //TEST_GL_TOP_RASTEROPERATIONS_FIXP_H
