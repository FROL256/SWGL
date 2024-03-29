//
// Created by frol on 24.08.18.
//

#ifndef TEST_GL_TOP_RASTEROPERATIONS_H
#define TEST_GL_TOP_RASTEROPERATIONS_H

#include "TexSampler.h"

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

  static inline void gather4(const int* a_data, const int pitch, const vint offset,
                             vint a_result[4])
  {

  }

  static inline vint gather(const int* a_data, const int pitch, const vint offset)
  {
    return splat(0);
  }
};

template<typename vint>
struct TileOp<vint, 4>
{
  static inline vint w(const int CX1, const int FDY12, const int FDX12)
  {
    const vint vCX1   = splat(CX1);
    const vint vFDY12 = splat(FDY12);
    return vCX1 - vFDY12*vint{0,1,2,3}; 
  }

  static inline void gather4(const int* a_data, const int pitch, const vint offset,
                             vint a_result[4])
  {
    
    const vint vOne   = splat(1);
    const vint vPitch = splat(pitch);

    a_result[0] = gather(a_data, offset);
    a_result[1] = gather(a_data, offset + vOne);
    a_result[2] = gather(a_data, offset + vPitch);
    a_result[3] = gather(a_data, offset + vPitch + vOne);
  }

};


template<typename vint>
struct TileOp<vint, 8>
{
  static inline vint w(const int CX1, const int FDY12, const int FDX12)
  {
    const cvex::vint4 row0   = cvex::splat(CX1) - cvex::splat(FDY12)*cvex::vint4{0,1,2,3}; 
    const cvex::vint4 row1   = row0 + cvex::splat(FDX12);
    return cvex8::join(row0, row1); 
  }

  static inline void gather4(const int* a_data, const int pitch, const vint offset,
                             vint a_result[4])
  {
    const vint vOne   = splat(1);
    const vint vPitch = splat(pitch);

    a_result[0] = gather(a_data, offset);
    a_result[1] = gather(a_data, offset + vOne);
    a_result[2] = gather(a_data, offset + vPitch);
    a_result[3] = gather(a_data, offset + vPitch + vOne);
  }

};

#ifdef USE_AVX512

template<typename vint>
struct TileOp<vint, 16>
{
  static inline vint w(const int CX1, const int FDY12, const int FDX12)
  {
    const cvex::vint4 vFDX12 = cvex::splat(FDX12);
    const cvex::vint4 row0   = cvex::splat(CX1) - cvex::splat(FDY12)*cvex::vint4{0,1,2,3}; 
    const cvex::vint4 row1   = row0 + vFDX12;
    const cvex::vint4 row2   = row0 + vFDX12*2;
    const cvex::vint4 row3   = row0 + vFDX12*3;
    
    const cvex8::vint8 half1 = cvex8::join(row0, row1); 
    const cvex8::vint8 half2 = cvex8::join(row2, row3);

    return cvex16::join(half1, half2); 
  }

  static inline void gather4(const int* a_data, const int pitch, const vint offset,
                             vint a_result[4])
  {
    const vint vOne   = splat(1);
    const vint vPitch = splat(pitch);

    a_result[0] = gather(a_data, offset);
    a_result[1] = gather(a_data, offset + vOne);
    a_result[2] = gather(a_data, offset + vPitch);
    a_result[3] = gather(a_data, offset + vPitch + vOne);
  }
};

#endif

template<typename TriangleT, typename VTYPES, bool bilinearIsEnabled>
struct VROP
{ 
  typedef typename VTYPES::vint   vint;
  typedef typename VTYPES::vuint  vuint;
  typedef typename VTYPES::vfloat vfloat;
  
  enum {width = VTYPES::width};

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  struct FillColor
  {
    enum {n = width};
    using Triangle = TriangleT;
    using ROPType  = float;

    static inline void Block(const TriangleT& tri, const int CX1, const int CX2, const int FDY12, const int FDY23, const int FDX12, const int FDX23, const float areaInv,
                             unsigned int* pLineColor)
    {
      const vuint res = splat(tri.fillCol);
      store(pLineColor, res);
    }

    static inline FBColorType Pixel(const TriangleT& tri, const int CX1, const int CX2, const float areaInv)
    {
      return tri.fillCol;
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  struct Colored2D
  {
    enum {n = width};
    using Triangle = TriangleT;
    using ROPType  = float;

    static inline void Block(const TriangleT& tri, const int CX1, const int CX2, const int FDY12, const int FDY23, const int FDX12, const int FDX23, const float areaInv,
                             FBColorType* pLineColor)
    {
      const vfloat c_one = splat(1.0f);

      const vfloat w1 = areaInv*to_float32( TileOp<vint,n>::w(CX1, FDY12, FDX12) );
      const vfloat w2 = areaInv*to_float32( TileOp<vint,n>::w(CX2, FDY23, FDX23) );
      const vfloat w3 = (c_one - w1 - w2);

      const vfloat r = tri.c1.x*w1 + tri.c2.x*w2 + tri.c3.x*w3;
      const vfloat g = tri.c1.y*w1 + tri.c2.y*w2 + tri.c3.y*w3;
      const vfloat b = tri.c1.z*w1 + tri.c2.z*w2 + tri.c3.z*w3;

      const vuint res = FB::ColorPack<vfloat,vuint>(r,g,b);

      store(pLineColor, res);
    }

    static inline FBColorType Pixel(const TriangleT& tri, const int CX1, const int CX2, const float areaInv)
    {
      const float w1 = areaInv*float(CX1);
      const float w2 = areaInv*float(CX2);
      const float4 c = tri.c1*w1 + tri.c2*w2 + tri.c3*(1.0f - w1 - w2);
      return LiteMath::color_pack_bgra(c);
    }

  };

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  struct Colored3D
  {
    enum {n = width};
    using Triangle = TriangleT;
    using ROPType  = float;

    static inline void Block(const TriangleT& tri, const int CX1, const int CX2, const int FDY12, const int FDY23, const int FDX12, const int FDX23, const float areaInv,
                             FBColorType* pLineColor, float* pLineDepth)
    {
      const vfloat c_one = splat(1.0f);
      const vfloat c_255 = splat(255.0f);

      const vfloat w1 = areaInv*to_float32( TileOp<vint,n>::w(CX1, FDY12, FDX12) );
      const vfloat w2 = areaInv*to_float32( TileOp<vint,n>::w(CX2, FDY23, FDX23) );
      const vfloat w3 = (c_one - w1 - w2);

      const vfloat zInv  = tri.v1.z*w1 + tri.v2.z*w2 + tri.v3.z*w3;
      const vfloat zOld  = load(pLineDepth);
      const vint   zTest = (zInv > zOld);

      if(any_of(zTest))
      {
        const auto z = rcp_e(zInv);

        const auto r = (tri.c1.x * w1 + tri.c2.x * w2 + tri.c3.x * w3)*z;
        const auto g = (tri.c1.y * w1 + tri.c2.y * w2 + tri.c3.y * w3)*z;
        const auto b = (tri.c1.z * w1 + tri.c2.z * w2 + tri.c3.z * w3)*z;

        const vuint colorOld = load(pLineColor);
        const vuint colori   = FB::ColorPack<vfloat,vuint>(r,g,b);

        store(pLineColor, blend(colori, colorOld, zTest));
        store(pLineDepth, blend(zInv,   zOld,     zTest));
      }
    }


    static inline void Pixel(const TriangleT& tri, const int CX1, const int CX2, const float areaInv,
                             FBColorType* pPixelColor, float* pPixelDepth)
    {
      const float w1 = areaInv*float(CX1);
      const float w2 = areaInv*float(CX2);

      const float zInv = tri.v1.z*w1 + tri.v2.z*w2 + tri.v3.z*(1.0f - w1 - w2);
      const float zOld = (*pPixelDepth);

      if (zInv > zOld)
      {
        const float  z = 1.0f/zInv;
        const float4 c = (tri.c1*w1 + tri.c2*w2 + tri.c3*(1.0f - w1 - w2))*z;
        (*pPixelColor) = LiteMath::color_pack_bgra(c);
        (*pPixelDepth) = zInv;
      }
    }

  };

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  static inline void WrapTexCoord(vfloat& x, vfloat& y)
  {
    x = x - to_float32(to_int32(x));
    y = y - to_float32(to_int32(y));

    const vfloat zero = splat(0.0f);
    const vfloat one  = splat(1.0f);

    x = blend(x + one, x, (x < zero)); //x = x < 0.0f ? x + 1.0f : x;
    y = blend(y + one, y, (y < zero)); //y = y < 0.0f ? y + 1.0f : y;
  }

  static inline void ReadImage4f_Bilinear(const int* pData, const int w, const int h, int pitch, const vfloat a_texCoordX, const vfloat a_texCoordY,
                                          vfloat a_result[4])
  {
    const vfloat fw = to_float32( (vint)splat(w) );
    const vfloat fh = to_float32( (vint)splat(h) );

    //const float ffx = a_texCoord.x*fw - 0.5f;
    //const float ffy = a_texCoord.y*fh - 0.5f;

    const vfloat zero = splat(0.0f);
    const vfloat one  = splat(1.0f);
    const vfloat half = splat(0.5f);

    const vfloat ffx = clamp((a_texCoordX*fw - half), zero, (fw - one) );
    const vfloat ffy = clamp((a_texCoordY*fh - half), zero, (fh - one) );

    const vint px = to_int32(ffx);
    const vint py = to_int32(ffy);

    // Calculate the weights for each pixel
    //
    const vfloat fx  = ffx - to_float32(px);
    const vfloat fy  = ffy - to_float32(py);
    const vfloat fx1 = one - fx;
    const vfloat fy1 = one - fy;

    const vfloat w1 = fx1 * fy1;
    const vfloat w2 = fx  * fy1;
    const vfloat w3 = fx1 * fy;
    const vfloat w4 = fx  * fy;

    const vfloat mult = splat(0.003921568f); // (1.0f/255.0f);

    const vint offset = (py*pitch) + px;

    vint ipixels[4];
    TileOp<vint,width>::gather4(pData, pitch, offset,
                                ipixels);

    const auto mask_R = splat(0x000000FF);
    const auto mask_G = splat(0x0000FF00);
    const auto mask_B = splat(0x00FF0000);
    const auto mask_A = splat(0xFF000000);

    const vfloat f1_x = mult*to_float32((ipixels[0] & mask_R) >> 0);
    const vfloat f1_y = mult*to_float32((ipixels[0] & mask_G) >> 8);
    const vfloat f1_z = mult*to_float32((ipixels[0] & mask_B) >> 16);
    const vfloat f1_w = mult*to_float32((ipixels[0] & mask_A) >> 24);

    const vfloat f2_x = mult*to_float32((ipixels[1] & mask_R) >> 0);
    const vfloat f2_y = mult*to_float32((ipixels[1] & mask_G) >> 8);
    const vfloat f2_z = mult*to_float32((ipixels[1] & mask_B) >> 16);
    const vfloat f2_w = mult*to_float32((ipixels[1] & mask_A) >> 24);

    const vfloat f3_x = mult*to_float32((ipixels[2] & mask_R) >> 0);
    const vfloat f3_y = mult*to_float32((ipixels[2] & mask_G) >> 8);
    const vfloat f3_z = mult*to_float32((ipixels[2] & mask_B) >> 16);
    const vfloat f3_w = mult*to_float32((ipixels[2] & mask_A) >> 24);

    const vfloat f4_x = mult*to_float32((ipixels[3] & mask_R) >> 0);
    const vfloat f4_y = mult*to_float32((ipixels[3] & mask_G) >> 8);
    const vfloat f4_z = mult*to_float32((ipixels[3] & mask_B) >> 16);
    const vfloat f4_w = mult*to_float32((ipixels[3] & mask_A) >> 24);

    // Calculate the weighted sum of pixels (for each color channel)
    //
    a_result[0] = f1_x * w1 + f2_x * w2 + f3_x * w3 + f4_x * w4;
    a_result[1] = f1_y * w1 + f2_y * w2 + f3_y * w3 + f4_y * w4;
    a_result[2] = f1_z * w1 + f2_z * w2 + f3_z * w3 + f4_z * w4;
    a_result[3] = f1_w * w1 + f2_w * w2 + f3_w * w3 + f4_w * w4;
  }

  static inline void ReadImage4f_Point(const int* pData, const int w, const int h, int pitch, const vfloat a_texCoordX, const vfloat a_texCoordY,
                                       vfloat a_result[4])
  {
    const vfloat zero = splat(0.0f);
    const vfloat one  = splat(1.0f);
    const vfloat half = splat(0.5f);

    const vfloat fw = to_float32( (vint)splat(w) );
    const vfloat fh = to_float32( (vint)splat(h) );

    //const vfloat ffx = a_texCoordX*fw + half;
    //const vfloat ffy = a_texCoordY*fh + half;

    const vfloat ffx = clamp(a_texCoordX*fw + half, zero, (fw - one) );
    const vfloat ffy = clamp(a_texCoordY*fh + half, zero, (fh - one) );

    const vint px = to_int32(ffx);
    const vint py = to_int32(ffy);

    const vfloat mult = splat(0.003921568f); // (1.0f/255.0f);

    const vint offset = (py*pitch) + px;
    const vint ipixel = gather(pData, offset);

    a_result[0] = mult*to_float32((ipixel & 0x000000FF) >> 0);
    a_result[1] = mult*to_float32((ipixel & 0x0000FF00) >> 8);
    a_result[2] = mult*to_float32((ipixel & 0x00FF0000) >> 16);
    a_result[3] = mult*to_float32((ipixel & 0xFF000000) >> 24);
  }


  template<bool bilinear>
  static inline void Tex2DSample4f(const TriangleT& tri, vfloat x, vfloat y,
                                 vfloat a_res[4])
  {
    WrapTexCoord(x,y);
    if(bilinear)        // assume compiler could optimize this
    {
      ReadImage4f_Bilinear(tri.texS.data, tri.texS.w, tri.texS.h, tri.texS.pitch, x, y,
                           a_res);
    }
    else
    {
      ReadImage4f_Point(tri.texS.data, tri.texS.w, tri.texS.h, tri.texS.pitch, x, y,
                        a_res);
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  static inline void ReadImage3f_Bilinear(const int* pData, const int w, const int h, int pitch, const vfloat a_texCoordX, const vfloat a_texCoordY,
                                          vfloat a_result[3])
  {
    const vfloat fw = to_float32( (vint)splat(w) );
    const vfloat fh = to_float32( (vint)splat(h) );

    //const float ffx = a_texCoord.x*fw - 0.5f;
    //const float ffy = a_texCoord.y*fh - 0.5f;

    const vfloat zero = splat(0.0f);
    const vfloat one  = splat(1.0f);
    const vfloat half = splat(0.5f);

    const vfloat ffx = clamp((a_texCoordX*fw - half), zero, (fw - one) );
    const vfloat ffy = clamp((a_texCoordY*fh - half), zero, (fh - one) );

    const vint px = to_int32(ffx);
    const vint py = to_int32(ffy);

    // Calculate the weights for each pixel
    //
    const vfloat fx  = ffx - to_float32(px);
    const vfloat fy  = ffy - to_float32(py);
    const vfloat fx1 = one - fx;
    const vfloat fy1 = one - fy;

    const vfloat w1 = fx1 * fy1;
    const vfloat w2 = fx  * fy1;
    const vfloat w3 = fx1 * fy;
    const vfloat w4 = fx  * fy;

    const vfloat mult = splat(0.003921568f); // (1.0f/255.0f);

    const vint offset = (py*pitch) + px;

    vint ipixels[4];
    TileOp<vint,width>::gather4(pData, pitch, offset,
                                ipixels);

    const vint mask_R = splat(int(0x000000FF));
    const vint mask_G = splat(int(0x0000FF00));
    const vint mask_B = splat(int(0x00FF0000));

    const vfloat f1_x = mult*to_float32((ipixels[0] & mask_R) >> 0);
    const vfloat f1_y = mult*to_float32((ipixels[0] & mask_G) >> 8);
    const vfloat f1_z = mult*to_float32((ipixels[0] & mask_B) >> 16);

    const vfloat f2_x = mult*to_float32((ipixels[1] & mask_R) >> 0);
    const vfloat f2_y = mult*to_float32((ipixels[1] & mask_G) >> 8);
    const vfloat f2_z = mult*to_float32((ipixels[1] & mask_B) >> 16);

    const vfloat f3_x = mult*to_float32((ipixels[2] & mask_R) >> 0);
    const vfloat f3_y = mult*to_float32((ipixels[2] & mask_G) >> 8);
    const vfloat f3_z = mult*to_float32((ipixels[2] & mask_B) >> 16);

    const vfloat f4_x = mult*to_float32((ipixels[3] & mask_R) >> 0);
    const vfloat f4_y = mult*to_float32((ipixels[3] & mask_G) >> 8);
    const vfloat f4_z = mult*to_float32((ipixels[3] & mask_B) >> 16);

    // Calculate the weighted sum of pixels (for each color channel)
    //
    a_result[0] = f1_x * w1 + f2_x * w2 + f3_x * w3 + f4_x * w4;
    a_result[1] = f1_y * w1 + f2_y * w2 + f3_y * w3 + f4_y * w4;
    a_result[2] = f1_z * w1 + f2_z * w2 + f3_z * w3 + f4_z * w4;
  }

  static inline void ReadImage3f_Point(const int* pData, const int w, const int h, int pitch, const vfloat a_texCoordX, const vfloat a_texCoordY,
                                       vfloat a_result[4])
  {
    const vfloat zero = splat(0.0f);
    const vfloat one  = splat(1.0f);
    const vfloat half = splat(0.5f);

    const vfloat fw = to_float32( (vint)splat(w) );
    const vfloat fh = to_float32( (vint)splat(h) );

    //const vfloat ffx = a_texCoordX*fw + half;
    //const vfloat ffy = a_texCoordY*fh + half;

    const vfloat ffx = clamp(a_texCoordX*fw + half, zero, (fw - one) );
    const vfloat ffy = clamp(a_texCoordY*fh + half, zero, (fh - one) );

    const vint px = to_int32(ffx);
    const vint py = to_int32(ffy);

    const vfloat mult = splat(0.003921568f); // (1.0f/255.0f);

    const vint offset = (py*pitch) + px;
    const vint ipixel = gather(pData, offset);

    const vint mask_R = splat(int(0x000000FF));
    const vint mask_G = splat(int(0x0000FF00));
    const vint mask_B = splat(int(0x00FF0000));

    a_result[0] = mult*to_float32((ipixel & mask_R) >> 0);
    a_result[1] = mult*to_float32((ipixel & mask_G) >> 8);
    a_result[2] = mult*to_float32((ipixel & mask_B) >> 16);
  }


  template<bool bilinear>
  static inline void Tex2DSample3f(const TriangleT& tri, vfloat x, vfloat y,
                                   vfloat a_res[3])
  {
    WrapTexCoord(x,y);
    if(bilinear)        // assume compiler could optimize this
    {
      ReadImage3f_Bilinear(tri.texS.data, tri.texS.w, tri.texS.h, tri.texS.pitch, x, y,
                           a_res);
    }
    else
    {
      ReadImage3f_Point(tri.texS.data, tri.texS.w, tri.texS.h, tri.texS.pitch, x, y,
                        a_res);
    }
  }


  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  struct Textured2D
  {
    enum {n = width};
    using Triangle = TriangleT;
    using ROPType  = float;

    static inline void Block(const TriangleT& tri, const int CX1, const int CX2, const int FDY12, const int FDY23, const int FDX12, const int FDX23, const float areaInv,
                             FBColorType* pLineColor)
    {
      const vfloat c_one = splat(1.0f);
      const vfloat c_255 = splat(255.0f);

      const vfloat w1 = areaInv*to_float32( TileOp<vint,n>::w(CX1, FDY12, FDX12) );
      const vfloat w2 = areaInv*to_float32( TileOp<vint,n>::w(CX2, FDY23, FDX23) );
      const vfloat w3 = (c_one - w1 - w2);

      const vfloat r = (tri.c1.x*w1 + tri.c2.x*w2 + tri.c3.x*w3);
      const vfloat g = (tri.c1.y*w1 + tri.c2.y*w2 + tri.c3.y*w3);
      const vfloat b = (tri.c1.z*w1 + tri.c2.z*w2 + tri.c3.z*w3);

      const vfloat tx = tri.t1.x*w1 + tri.t2.x*w2 + tri.t3.x*w3;
      const vfloat ty = tri.t1.y*w1 + tri.t2.y*w2 + tri.t3.y*w3;

      vfloat texColor[3];
      Tex2DSample3f<bilinearIsEnabled>(tri, tx, ty, texColor);

      const vuint res = FB::ColorPack<vfloat,vuint>(r*texColor[0], g*texColor[1], b*texColor[2]);                      

      store(pLineColor, res);
    }

    static inline FBColorType Pixel(const TriangleT& tri, const int CX1, const int CX2, const float areaInv)
    {
      const float w1  = areaInv*float(CX1);
      const float w2  = areaInv*float(CX2);
      const float w3  = (1.0f - w1 - w2);
      const float2 t  = tri.t1*w1 + tri.t2*w2 + tri.t3*w3;
      const float4 c  = tri.c1*w1 + tri.c2*w2 + tri.c3*w3;
      const float4 tc = tex2D(tri.texS, t);
      return LiteMath::color_pack_bgra(c*tc);
    }

  };

  struct Textured2D_White
  {
    enum {n = width};
    using Triangle = TriangleT;
    using ROPType  = float;

    static inline void Block(const TriangleT& tri, const int CX1, const int CX2, const int FDY12, const int FDY23, const int FDX12, const int FDX23, const float areaInv,
                             FBColorType* pLineColor)
    {
      const vfloat c_one = splat(1.0f);
      const vfloat c_255 = splat(255.0f);

      const vfloat w1 = areaInv*to_float32( TileOp<vint,n>::w(CX1, FDY12, FDX12) );
      const vfloat w2 = areaInv*to_float32( TileOp<vint,n>::w(CX2, FDY23, FDX23) );
      const vfloat w3 = (c_one - w1 - w2);

      const vfloat tx = tri.t1.x*w1 + tri.t2.x*w2 + tri.t3.x*w3;
      const vfloat ty = tri.t1.y*w1 + tri.t2.y*w2 + tri.t3.y*w3;

      vfloat texColor[3];
      Tex2DSample3f<bilinearIsEnabled>(tri, tx, ty, texColor);

      const vuint res = FB::ColorPack<vfloat,vuint>(texColor[0], texColor[1], texColor[2]);                      

      store(pLineColor, res);
    }

    static inline FBColorType Pixel(const TriangleT& tri, const int CX1, const int CX2, const float areaInv)
    {
      const float w1  = areaInv*float(CX1);
      const float w2  = areaInv*float(CX2);
      const float w3  = (1.0f - w1 - w2);
      const float2 t  = tri.t1*w1 + tri.t2*w2 + tri.t3*w3;
      const float4 tc = tex2D(tri.texS, t);
      return LiteMath::color_pack_bgra(tc);
    }

  };

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  struct Textured3D
  {
    enum {n = width};
    using Triangle = TriangleT;
    using ROPType  = float;

    static inline void Block(const TriangleT& tri, const int CX1, const int CX2, const int FDY12, const int FDY23, const int FDX12, const int FDX23, const float areaInv,
                             FBColorType* pLineColor, float* pLineDepth)
    {
      const vfloat c_one  = splat(1.0f);
      const vfloat c_zero = splat(0.0f);
      const vfloat c_255  = splat(255.0f);

      const vfloat w1 = areaInv*to_float32( TileOp<vint,n>::w(CX1, FDY12, FDX12) );
      const vfloat w2 = areaInv*to_float32( TileOp<vint,n>::w(CX2, FDY23, FDX23) );
      const vfloat w3 = (c_one - w1 - w2);

      const vfloat zInv  = tri.v1.z*w1 + tri.v2.z*w2 + tri.v3.z*w3;
      const vfloat zOld  = load(pLineDepth);
      const vint   zTest = (zInv > zOld);

      if(any_of(zTest))
      {
        store(pLineDepth, blend(zInv, zOld, zTest));

        const vfloat  z = rcp_e(zInv);

        const vfloat  r = (tri.c1.x * w1 + tri.c2.x * w2 + tri.c3.x * w3)*z;
        const vfloat  g = (tri.c1.y * w1 + tri.c2.y * w2 + tri.c3.y * w3)*z;
        const vfloat  b = (tri.c1.z * w1 + tri.c2.z * w2 + tri.c3.z * w3)*z;

        const vfloat tx = (tri.t1.x*w1 + tri.t2.x*w2 + tri.t3.x*w3)*z;
        const vfloat ty = (tri.t1.y*w1 + tri.t2.y*w2 + tri.t3.y*w3)*z;

        vfloat texColor[3];
        Tex2DSample3f<bilinearIsEnabled>(tri, tx, ty, texColor);

        const vuint colorOld = load(pLineColor);
        const vuint colori   = FB::ColorPack<vfloat,vuint>(r*texColor[0], g*texColor[1], b*texColor[2]);                            

        store(pLineColor, blend(colori, colorOld, zTest));
      }
    }


    static inline void Pixel(const TriangleT& tri, const int CX1, const int CX2, const float areaInv,
                             FBColorType* pPixelColor, float* pPixelDepth)
    {
      const float w1 = areaInv*float(CX1);
      const float w2 = areaInv*float(CX2);

      const float zInv = tri.v1.z*w1 + tri.v2.z*w2 + tri.v3.z*(1.0f - w1 - w2);
      const float zOld = (*pPixelDepth);

      if (zInv > zOld)
      {
        const float  z  = 1.0f/zInv;
        const float w3  = (1.0f - w1 - w2);
        const float4 c  = (tri.c1*w1 + tri.c2*w2 + tri.c3*w3)*z;
        const float2 t  = (tri.t1*w1 + tri.t2*w2 + tri.t3*w3)*z;
        const float4 tc = tex2D(tri.texS, t);

        (*pPixelColor) = LiteMath::color_pack_bgra(c*tc);
        (*pPixelDepth) = zInv;
      }
    }

  };

  struct Textured3D_White
  {
    enum {n = width};
    using Triangle = TriangleT;
    using ROPType  = float;

    static inline void Block(const TriangleT& tri, const int CX1, const int CX2, const int FDY12, const int FDY23, const int FDX12, const int FDX23, const float areaInv,
                             FBColorType* pLineColor, float* pLineDepth)
    {
      const vfloat c_one  = splat(1.0f);
      const vfloat c_zero = splat(0.0f);
      const vfloat c_255  = splat(255.0f);

      const vfloat w1 = areaInv*to_float32( TileOp<vint,n>::w(CX1, FDY12, FDX12) );
      const vfloat w2 = areaInv*to_float32( TileOp<vint,n>::w(CX2, FDY23, FDX23) );
      const vfloat w3 = (c_one - w1 - w2);

      const vfloat zInv  = tri.v1.z*w1 + tri.v2.z*w2 + tri.v3.z*w3;
      const vfloat zOld  = load(pLineDepth);
      const vint   zTest = (zInv > zOld);

      if(any_of(zTest))
      {
        store(pLineDepth, blend(zInv, zOld, zTest));

        const vfloat  z = rcp_e(zInv);
        const vfloat tx = (tri.t1.x*w1 + tri.t2.x*w2 + tri.t3.x*w3)*z;
        const vfloat ty = (tri.t1.y*w1 + tri.t2.y*w2 + tri.t3.y*w3)*z;

        vfloat texColor[3];
        Tex2DSample3f<bilinearIsEnabled>(tri, tx, ty, texColor);

        const vuint colorOld = load(pLineColor);
        const vuint colori   = FB::ColorPack<vfloat,vuint>(texColor[0], texColor[1], texColor[2]);

        store(pLineColor, blend(colori, colorOld, zTest));
      }
    }


    static inline void Pixel(const TriangleT& tri, const int CX1, const int CX2, const float areaInv,
                             FBColorType* pPixelColor, float* pPixelDepth)
    {
      const float w1 = areaInv*float(CX1);
      const float w2 = areaInv*float(CX2);

      const float zInv = tri.v1.z*w1 + tri.v2.z*w2 + tri.v3.z*(1.0f - w1 - w2);
      const float zOld = (*pPixelDepth);

      if (zInv > zOld)
      {
        const float  z  = 1.0f/zInv;
        const float w3  = (1.0f - w1 - w2);
        const float2 t  = (tri.t1*w1 + tri.t2*w2 + tri.t3*w3)*z;
        const float4 tc = tex2D(tri.texS, t);

        (*pPixelColor) = LiteMath::color_pack_bgra(tc);
        (*pPixelDepth) = zInv;
      }
    }

  };


  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  struct Textured3D_Blend
  {
    enum {n = width};
    using Triangle = TriangleT;
    using ROPType  = float;

    static inline void Block(const TriangleT& tri, const int CX1, const int CX2, const int FDY12, const int FDY23, const int FDX12, const int FDX23, const float areaInv,
                             FBColorType* pLineColor, float* pLineDepth)
    {
      const vfloat c_one    = splat(1.0f);
      const vfloat c_255    = splat(255.0f);
      const vfloat c_255Inv = splat(1.0f/255.0f);

      const vfloat w1 = areaInv*to_float32( TileOp<vint,n>::w(CX1, FDY12, FDX12) );
      const vfloat w2 = areaInv*to_float32( TileOp<vint,n>::w(CX2, FDY23, FDX23) );
      const vfloat w3 = (c_one - w1 - w2);

      const vfloat zInv  = tri.v1.z*w1 + tri.v2.z*w2 + tri.v3.z*w3;
      const vfloat zOld  = load(pLineDepth);
      const vint   zTest = (zInv > zOld);

      if(any_of(zTest))
      {
        store(pLineDepth, blend(zInv, zOld, zTest));

        const vfloat  z = rcp_e(zInv);

        const vfloat  r = (tri.c1.x * w1 + tri.c2.x * w2 + tri.c3.x * w3)*z;
        const vfloat  g = (tri.c1.y * w1 + tri.c2.y * w2 + tri.c3.y * w3)*z;
        const vfloat  b = (tri.c1.z * w1 + tri.c2.z * w2 + tri.c3.z * w3)*z;
        const vfloat  a = (tri.c1.w * w1 + tri.c2.w * w2 + tri.c3.w * w3)*z;

        const vfloat tx = (tri.t1.x*w1 + tri.t2.x*w2 + tri.t3.x*w3)*z;
        const vfloat ty = (tri.t1.y*w1 + tri.t2.y*w2 + tri.t3.y*w3)*z;

        vfloat texColor[4];
        Tex2DSample4f<bilinearIsEnabled>(tri, tx, ty, texColor);

        const vuint colorOld = load(pLineColor);
        
        vfloat redOld, greenOld, blueOld, alphaOld;
        FB::ColorUNPack<vfloat,vuint>(colorOld, 
                                      redOld, greenOld, blueOld, alphaOld);
       
        const vfloat alpha    = (a * texColor[3]);

        const vfloat redNew   = (r * texColor[0]) * alpha + (c_one - alpha)*redOld;
        const vfloat greenNew = (g * texColor[1]) * alpha + (c_one - alpha)*greenOld;
        const vfloat blueNew  = (b * texColor[2]) * alpha + (c_one - alpha)*blueOld;
        const vfloat alphaNew = alpha             * alpha + (c_one - alpha)*alphaOld;

        const vuint colori = FB::ColorPack<vfloat,vuint>(redNew, greenNew, blueNew, alphaNew);

        store(pLineColor, blend(colori, colorOld, zTest));
      }
    }


    static inline void Pixel(const TriangleT& tri, const int CX1, const int CX2, const float areaInv,
                             FBColorType* pPixelColor, float* pPixelDepth)
    {
      const float w1 = areaInv*float(CX1);
      const float w2 = areaInv*float(CX2);

      const float zInv = tri.v1.z*w1 + tri.v2.z*w2 + tri.v3.z*(1.0f - w1 - w2);
      const float zOld = (*pPixelDepth);

      if (zInv > zOld)
      {
        const float  z  = 1.0f/zInv;
        const float w3  = (1.0f - w1 - w2);
        const float4 c  = (tri.c1*w1 + tri.c2*w2 + tri.c3*w3)*z;
        const float2 t  = (tri.t1*w1 + tri.t2*w2 + tri.t3*w3)*z;
        const float4 tc = tex2D(tri.texS, t);

        const float4 oldColor = LiteMath::color_unpack_bgra((*pPixelColor));
        const float4 newColor = c*tc;
        const float alpha     = newColor.w;

        (*pPixelColor) = color_pack_bgra(oldColor*(1.0f - alpha) + alpha*newColor);
        (*pPixelDepth) = zInv;
      }
    }

  };


};



#endif //TEST_GL_TOP_RASTEROPERATIONS_H
