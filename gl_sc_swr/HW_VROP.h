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

  static inline void load4(const int* a_data, const int pitch, const vint offset,
                           vint a_result[4])
  {

  }

  static inline vint load1(const int* a_data, const int pitch, const vint offset)
  {
    vint temp = splat(0);
    return temp;
  }
};

template<typename vint>
struct LineOffs<vint, 4>
{
  static inline vint w(const int CX1, const int FDY12)
  {
    return make_vint(CX1, CX1 - FDY12, CX1 - FDY12*2, CX1 - FDY12*3);
  }

  static inline void load4(const int* a_data, const int pitch, const vint offset,
                           vint a_result[4])
  {
    ALIGNED(16) int myOffsets[4];
    store(myOffsets, offset);

    const int* p0 = a_data + myOffsets[0];
    const int* p1 = a_data + myOffsets[1];
    const int* p2 = a_data + myOffsets[2];
    const int* p3 = a_data + myOffsets[3];

    const int d01 = p0[0 + 0 * pitch];
    const int d02 = p0[1 + 0 * pitch];
    const int d03 = p0[0 + 1 * pitch];
    const int d04 = p0[1 + 1 * pitch];

    const int d11 = p1[0 + 0 * pitch];
    const int d12 = p1[1 + 0 * pitch];
    const int d13 = p1[0 + 1 * pitch];
    const int d14 = p1[1 + 1 * pitch];

    const int d21 = p2[0 + 0 * pitch];
    const int d22 = p2[1 + 0 * pitch];
    const int d23 = p2[0 + 1 * pitch];
    const int d24 = p2[1 + 1 * pitch];

    const int d31 = p3[0 + 0 * pitch];
    const int d32 = p3[1 + 0 * pitch];
    const int d33 = p3[0 + 1 * pitch];
    const int d34 = p3[1 + 1 * pitch];

    a_result[0] = make_vint(d01, d11, d21, d31);
    a_result[1] = make_vint(d02, d12, d22, d32);
    a_result[2] = make_vint(d03, d13, d23, d33);
    a_result[3] = make_vint(d04, d14, d24, d34);
  }

  static inline vint load1(const int* a_data, const int pitch, const vint offset)
  {
    ALIGNED(16) int myOffsets[4];
    store(myOffsets, offset);

    const int d01 = a_data[myOffsets[0]];
    const int d11 = a_data[myOffsets[1]];
    const int d21 = a_data[myOffsets[2]];
    const int d31 = a_data[myOffsets[3]];

    return make_vint(d01, d11, d21, d31);
  }

};

template<typename vint>
struct LineOffs<vint, 8>
{
  static inline vint w(const int CX1, const int FDY12)
  {
    return make_vint(CX1, CX1 - FDY12, CX1 - FDY12*2, CX1 - FDY12*3, CX1 - FDY12*4, CX1 - FDY12*5, CX1 - FDY12*6, CX1 - FDY12*7);
  }


  static inline void load4(const int* a_data, const int pitch, const vint offset,
                           vint a_result[4])
  {
    ALIGNED(32) int myOffsets[8];
    store(myOffsets, offset);

    const int* p0 = a_data + myOffsets[0];
    const int* p1 = a_data + myOffsets[1];
    const int* p2 = a_data + myOffsets[2];
    const int* p3 = a_data + myOffsets[3];

    const int* p4 = a_data + myOffsets[4];
    const int* p5 = a_data + myOffsets[5];
    const int* p6 = a_data + myOffsets[6];
    const int* p7 = a_data + myOffsets[7];

    const int d01 = p0[0 + 0 * pitch];
    const int d02 = p0[1 + 0 * pitch];
    const int d03 = p0[0 + 1 * pitch];
    const int d04 = p0[1 + 1 * pitch];

    const int d11 = p1[0 + 0 * pitch];
    const int d12 = p1[1 + 0 * pitch];
    const int d13 = p1[0 + 1 * pitch];
    const int d14 = p1[1 + 1 * pitch];

    const int d21 = p2[0 + 0 * pitch];
    const int d22 = p2[1 + 0 * pitch];
    const int d23 = p2[0 + 1 * pitch];
    const int d24 = p2[1 + 1 * pitch];

    const int d31 = p3[0 + 0 * pitch];
    const int d32 = p3[1 + 0 * pitch];
    const int d33 = p3[0 + 1 * pitch];
    const int d34 = p3[1 + 1 * pitch];

    const int d41 = p4[0 + 0 * pitch];
    const int d42 = p4[1 + 0 * pitch];
    const int d43 = p4[0 + 1 * pitch];
    const int d44 = p4[1 + 1 * pitch];

    const int d51 = p5[0 + 0 * pitch];
    const int d52 = p5[1 + 0 * pitch];
    const int d53 = p5[0 + 1 * pitch];
    const int d54 = p5[1 + 1 * pitch];

    const int d61 = p6[0 + 0 * pitch];
    const int d62 = p6[1 + 0 * pitch];
    const int d63 = p6[0 + 1 * pitch];
    const int d64 = p6[1 + 1 * pitch];

    const int d71 = p7[0 + 0 * pitch];
    const int d72 = p7[1 + 0 * pitch];
    const int d73 = p7[0 + 1 * pitch];
    const int d74 = p7[1 + 1 * pitch];

    a_result[0] = make_vint(d01, d11, d21, d31, d41, d51, d61, d71);
    a_result[1] = make_vint(d02, d12, d22, d32, d42, d52, d62, d72);
    a_result[2] = make_vint(d03, d13, d23, d33, d43, d53, d63, d73);
    a_result[3] = make_vint(d04, d14, d24, d34, d44, d54, d64, d74);
  }


  static inline vint load1(const int* a_data, const int pitch, const vint offset)
  {
    ALIGNED(32) int myOffsets[8];
    store(myOffsets, offset);

    const int d01 = a_data[myOffsets[0]];
    const int d11 = a_data[myOffsets[1]];
    const int d21 = a_data[myOffsets[2]];
    const int d31 = a_data[myOffsets[3]];
    const int d41 = a_data[myOffsets[4]];
    const int d51 = a_data[myOffsets[5]];
    const int d61 = a_data[myOffsets[6]];
    const int d71 = a_data[myOffsets[7]];

    return make_vint(d01, d11, d21, d31, d41, d51, d61, d71);
  }
};




template<typename TriangleT, typename vfloat, typename vint, int width, bool bilinearIsEnabled>
struct VROP
{
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  struct FillColor
  {
    enum {n = width};
    using Triangle = TriangleT;

    static inline vint Line(const TriangleT& tri)
    {
      return splat((int)RealColorToUint32_BGRA(tri.c1));
    }

    static inline void store_line(int* line, vint data)
    {
      store(line, data);
    }

  };

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

      store(pLineColor, res);
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
    enum {n = width};
    using Triangle = TriangleT;

    static inline void Line(const TriangleT& tri, const int CX1, const int CX2, const int FDY12, const int FDY23, const float areaInv,
                            int* pLineColor, float* pLineDepth)
    {
      prefetch(pLineDepth);
      const vfloat c_one = splat(1.0f);
      const vfloat c_255 = splat(255.0f);

      const vfloat w1 = areaInv* to_float32( LineOffs<vint,n>::w(CX1, FDY12) );
      const vfloat w2 = areaInv* to_float32( LineOffs<vint,n>::w(CX2, FDY23) );
      const vfloat w3 = (c_one - w1 - w2);

      const vfloat zInv  = tri.v1.z*w1 + tri.v2.z*w2 + tri.v3.z*w3;
      const vfloat zOld  = load(pLineDepth);
      const vint   zTest = (zInv > zOld);

      if(test_bits_any(zTest))
      {
        prefetch(pLineColor);
        const auto z = rcp_e(zInv);

        const auto r = (tri.c1.x * w1 + tri.c2.x * w2 + tri.c3.x * w3)*z;
        const auto g = (tri.c1.y * w1 + tri.c2.y * w2 + tri.c3.y * w3)*z;
        const auto b = (tri.c1.z * w1 + tri.c2.z * w2 + tri.c3.z * w3)*z;
        const auto a = (tri.c1.w * w1 + tri.c2.w * w2 + tri.c3.w * w3)*z;

        const vint colorOld = load(pLineColor);

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

        store(pLineColor, blend(colori, colorOld, zTest));
        store(pLineDepth, blend(zInv,   zOld,     zTest));
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

    const vfloat ffx = vclamp((a_texCoordX*fw - half), zero, (fw - one) );
    const vfloat ffy = vclamp((a_texCoordY*fh - half), zero, (fh - one) );

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
    LineOffs<vint,width>::load4(pData, pitch, offset,
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

    const vfloat ffx = vclamp(a_texCoordX*fw + half, zero, (fw - one) );
    const vfloat ffy = vclamp(a_texCoordY*fh + half, zero, (fh - one) );

    const vint px = to_int32(ffx);
    const vint py = to_int32(ffy);

    const vfloat mult = splat(0.003921568f); // (1.0f/255.0f);

    const vint offset = (py*pitch) + px;
    const vint ipixel = LineOffs<vint,width>::load1(pData, pitch, offset);

    const auto mask_R = splat(0x000000FF);
    const auto mask_G = splat(0x0000FF00);
    const auto mask_B = splat(0x00FF0000);
    const auto mask_A = splat(0xFF000000);

    a_result[0] = mult*to_float32((ipixel & mask_R) >> 0);
    a_result[1] = mult*to_float32((ipixel & mask_G) >> 8);
    a_result[2] = mult*to_float32((ipixel & mask_B) >> 16);
    a_result[3] = mult*to_float32((ipixel & mask_A) >> 24);
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

    const vfloat ffx = vclamp((a_texCoordX*fw - half), zero, (fw - one) );
    const vfloat ffy = vclamp((a_texCoordY*fh - half), zero, (fh - one) );

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
    LineOffs<vint,width>::load4(pData, pitch, offset,
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

    const vfloat ffx = vclamp(a_texCoordX*fw + half, zero, (fw - one) );
    const vfloat ffy = vclamp(a_texCoordY*fh + half, zero, (fh - one) );

    const vint px = to_int32(ffx);
    const vint py = to_int32(ffy);

    const vfloat mult = splat(0.003921568f); // (1.0f/255.0f);

    const vint offset = (py*pitch) + px;
    const vint ipixel = LineOffs<vint,width>::load1(pData, pitch, offset);

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

    static inline void Line(const TriangleT& tri, const int CX1, const int CX2, const int FDY12, const int FDY23, const float areaInv,
                            int* pLineColor)
    {
      const vfloat c_one = splat(1.0f);
      const vfloat c_255 = splat(255.0f);

      const vfloat w1 = areaInv*to_float32( LineOffs<vint,n>::w(CX1, FDY12) );
      const vfloat w2 = areaInv*to_float32( LineOffs<vint,n>::w(CX2, FDY23) );
      const vfloat w3 = (c_one - w1 - w2);

      const vfloat r = (tri.c1.x*w1 + tri.c2.x*w2 + tri.c3.x*w3);
      const vfloat g = (tri.c1.y*w1 + tri.c2.y*w2 + tri.c3.y*w3);
      const vfloat b = (tri.c1.z*w1 + tri.c2.z*w2 + tri.c3.z*w3);

      const vfloat tx = tri.t1.x*w1 + tri.t2.x*w2 + tri.t3.x*w3;
      const vfloat ty = tri.t1.y*w1 + tri.t2.y*w2 + tri.t3.y*w3;

      vfloat texColor[3];
      Tex2DSample3f<bilinearIsEnabled>(tri, tx, ty, texColor);

      const vint res = (to_int32(r * texColor[0] * c_255) << 16) | // BGRA
                       (to_int32(g * texColor[1] * c_255) << 8)  |
                       (to_int32(b * texColor[2] * c_255) << 0);

      store(pLineColor, res);
    }

    static inline int Pixel(const TriangleT& tri, const int CX1, const int CX2, const float areaInv)
    {
      const float w1  = areaInv*float(CX1);
      const float w2  = areaInv*float(CX2);
      const float w3  = (1.0f - w1 - w2);
      const float2 t  = tri.t1*w1 + tri.t2*w2 + tri.t3*w3;
      const float4 c  = tri.c1*w1 + tri.c2*w2 + tri.c3*w3;
      const float4 tc = tex2D(tri.texS, t);
      return RealColorToUint32_BGRA(c*tc);
    }

  };

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  struct Textured3D
  {
    enum {n = width};
    using Triangle = TriangleT;

    static inline void Line(const TriangleT& tri, const int CX1, const int CX2, const int FDY12, const int FDY23, const float areaInv,
                            int* pLineColor, float* pLineDepth)
    {
      prefetch(pLineDepth);

      const vfloat c_one = splat(1.0f);
      const vfloat c_255 = splat(255.0f);

      const vfloat w1 = areaInv*to_float32( LineOffs<vint,n>::w(CX1, FDY12) );
      const vfloat w2 = areaInv*to_float32( LineOffs<vint,n>::w(CX2, FDY23) );
      const vfloat w3 = (c_one - w1 - w2);

      const vfloat zInv  = tri.v1.z*w1 + tri.v2.z*w2 + tri.v3.z*w3;
      const vfloat zOld  = load(pLineDepth);
      const vint   zTest = (zInv > zOld);

      if(test_bits_any(zTest))
      {
        store(pLineDepth, blend(zInv, zOld, zTest));
        prefetch(pLineColor);

        const vfloat  z = rcp_e(zInv);

        const vfloat  r = (tri.c1.x * w1 + tri.c2.x * w2 + tri.c3.x * w3)*z;
        const vfloat  g = (tri.c1.y * w1 + tri.c2.y * w2 + tri.c3.y * w3)*z;
        const vfloat  b = (tri.c1.z * w1 + tri.c2.z * w2 + tri.c3.z * w3)*z;

        const vfloat tx = (tri.t1.x*w1 + tri.t2.x*w2 + tri.t3.x*w3)*z;
        const vfloat ty = (tri.t1.y*w1 + tri.t2.y*w2 + tri.t3.y*w3)*z;

        vfloat texColor[3];
        Tex2DSample3f<bilinearIsEnabled>(tri, tx, ty, texColor);

        const vint colorOld = load(pLineColor);

        const vint colori = (to_int32(r * texColor[0] * c_255) << 16) | // BGRA
                            (to_int32(g * texColor[1] * c_255) << 8)  |
                            (to_int32(b * texColor[2] * c_255) << 0);

        store(pLineColor, blend(colori, colorOld, zTest));
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
        const float  z  = 1.0f/zInv;
        const float w3  = (1.0f - w1 - w2);
        const float4 c  = (tri.c1*w1 + tri.c2*w2 + tri.c3*w3)*z;
        const float2 t  = (tri.t1*w1 + tri.t2*w2 + tri.t3*w3)*z;
        const float4 tc = tex2D(tri.texS, t);

        (*pPixelColor) = RealColorToUint32_BGRA(c*tc);
        (*pPixelDepth) = zInv;
      }
    }

  };


  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  #ifndef WIN32

  struct Textured3D_Blend
  {
    enum {n = width};
    using Triangle = TriangleT;

    static inline void Line(const TriangleT& tri, const int CX1, const int CX2, const int FDY12, const int FDY23, const float areaInv,
                            int* pLineColor, float* pLineDepth)
    {
      prefetch(pLineDepth);

      const vfloat c_one    = splat(1.0f);
      const vfloat c_255    = splat(255.0f);
      const vfloat c_255Inv = splat(1.0f/255.0f);

      const vfloat w1 = areaInv*to_float32( LineOffs<vint,n>::w(CX1, FDY12) );
      const vfloat w2 = areaInv*to_float32( LineOffs<vint,n>::w(CX2, FDY23) );
      const vfloat w3 = (c_one - w1 - w2);

      const vfloat zInv  = tri.v1.z*w1 + tri.v2.z*w2 + tri.v3.z*w3;
      const vfloat zOld  = load(pLineDepth);
      const vint   zTest = (zInv > zOld);

      if(test_bits_any(zTest))
      {
        store(pLineDepth, blend(zInv, zOld, zTest));
        prefetch(pLineColor);

        const vfloat  z = rcp_e(zInv);

        const vfloat  r = (tri.c1.x * w1 + tri.c2.x * w2 + tri.c3.x * w3)*z;
        const vfloat  g = (tri.c1.y * w1 + tri.c2.y * w2 + tri.c3.y * w3)*z;
        const vfloat  b = (tri.c1.z * w1 + tri.c2.z * w2 + tri.c3.z * w3)*z;
        const vfloat  a = (tri.c1.w * w1 + tri.c2.w * w2 + tri.c3.w * w3)*z;

        const vfloat tx = (tri.t1.x*w1 + tri.t2.x*w2 + tri.t3.x*w3)*z;
        const vfloat ty = (tri.t1.y*w1 + tri.t2.y*w2 + tri.t3.y*w3)*z;

        vfloat texColor[4];
        Tex2DSample4f<bilinearIsEnabled>(tri, tx, ty, texColor);

        const vint colorOld = load(pLineColor);

        const vfloat redOld   = to_float32( (to_uint32(colorOld) & 0x00FF0000) >> 16)*c_255Inv;
        const vfloat greenOld = to_float32( (to_uint32(colorOld) & 0x0000FF00) >> 8 )*c_255Inv;
        const vfloat blueOld  = to_float32( (to_uint32(colorOld) & 0x000000FF) >> 0 )*c_255Inv;
        const vfloat alphaOld = to_float32( (to_uint32(colorOld) & 0xFF000000) >> 24)*c_255Inv;

        const vfloat alpha    = (a * texColor[3]);

        const vfloat redNew   = (r * texColor[0]) * alpha + (c_one - alpha)*redOld;
        const vfloat greenNew = (g * texColor[1]) * alpha + (c_one - alpha)*greenOld;
        const vfloat blueNew  = (b * texColor[2]) * alpha + (c_one - alpha)*blueOld;
        const vfloat alphaNew = alpha             * alpha + (c_one - alpha)*alphaOld;

        const auto colori = (to_uint32(redNew   * c_255) << 16) | // BGRA
                            (to_uint32(greenNew * c_255) << 8)  |
                            (to_uint32(blueNew  * c_255) << 0)  |
                            (to_uint32(alphaNew * c_255) << 24);

        store(pLineColor, blend(colori, colorOld, zTest));
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
        const float  z  = 1.0f/zInv;
        const float w3  = (1.0f - w1 - w2);
        const float4 c  = (tri.c1*w1 + tri.c2*w2 + tri.c3*w3)*z;
        const float2 t  = (tri.t1*w1 + tri.t2*w2 + tri.t3*w3)*z;
        const float4 tc = tex2D(tri.texS, t);

        const float4 oldColor = Uint32_BGRAToRealColor((*pPixelColor));
        const float4 newColor = c*tc;
        const float alpha     = newColor.w;

        (*pPixelColor) = RealColorToUint32_BGRA(oldColor*(1.0f - alpha) + alpha*newColor);
        (*pPixelDepth) = zInv;
      }
    }

  };

  #endif

};



#endif //TEST_GL_TOP_RASTEROPERATIONS_H
