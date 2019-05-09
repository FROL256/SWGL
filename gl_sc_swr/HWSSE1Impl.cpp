#include <cstdint>
#include <algorithm>

#include "HWSSE1Impl.h"
#include "swgl.h"

#ifdef WIN32
#undef min
#undef max
#endif

using TriangleLocal = HWImpl_SSE1::TriangleType;

using namespace cvex;

void HWImpl_SSE1::memset32(int32_t* a_data, int32_t a_val, int32_t numElements)
{
  uintptr_t ip = reinterpret_cast<uintptr_t>(a_data);

  if ((numElements % 32 == 0) && (ip%16 == 0))
  {
    const vint4 val = splat(a_val);

    vint4* color128  = (vint4*)(a_data);
    const int size   = numElements/4;

    for (int i = 0; i < size; i += 8)
    {
      stream(color128 + i + 0, val);
      stream(color128 + i + 1, val);
      stream(color128 + i + 2, val);
      stream(color128 + i + 3, val);
      stream(color128 + i + 4, val);
      stream(color128 + i + 5, val);
      stream(color128 + i + 6, val);
      stream(color128 + i + 7, val);
    }
  }
  else
  {
    for (int i = 0; i < numElements; i++)
      a_data[i] = a_val;
  }
}

//#TODO: implement precise AABBTriangleOverlap test (rely on half space dist for corners)

bool HWImpl_SSE1::AABBTriangleOverlap(const TriangleType& a_tri, const int tileMinX, const int tileMinY, const int tileMaxX, const int tileMaxY) 
{
  const bool overlapBoxBox = IntersectBoxBox(int2(a_tri.bb_iminX, a_tri.bb_iminY), int2(a_tri.bb_imaxX, a_tri.bb_imaxY),
                                             int2(tileMinX, tileMinY),             int2(tileMaxX, tileMaxY));

  return overlapBoxBox;
}

static const vfloat4 const_255_inv   = {1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f};
static const vfloat4 const_256       = {256.0f, 256.0f, 256.0f, 256.0f};
static const vfloat4 const_2222      = {2.0f, 2.0f, 2.0f, 2.0f};
static const vfloat4 const_1111      = {1.0f, 1.0f, 1.0f, 1.0f};
static const vfloat4 const_0000      = {0.0f, 0.0f, 0.0f, 0.0f};
static const vfloat4 const_half_one  = {0.5f, 0.5f, 0.5f, 0.5f};
static const vfloat4 g_epsE3         = {HALF_SPACE_EPSILON, HALF_SPACE_EPSILON, HALF_SPACE_EPSILON, -10000000.0f};

static const vint4 const_maskXYZ     = make_vint(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000);

inline static vfloat4 setWtoOne(vfloat4 a_rhs)
{
  return blend(a_rhs, const_1111, const_maskXYZ);
}

static inline vfloat4 mul_matrix_vector(const vfloat4 cols[4], const vfloat4 v)
{
  const vfloat4 prod1 = splat_0(v)*cols[0];
  const vfloat4 prod2 = splat_1(v)*cols[1];
  const vfloat4 prod3 = splat_2(v)*cols[2];
  const vfloat4 prod4 = splat_3(v)*cols[3];

  return ((prod1 + prod2) + (prod3 + prod4));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline vfloat4  swglVertexShaderTransformSSE(const vfloat4 worlViewProjCols[4], vfloat4 a_pos) // pre (pBatch != nullptr)
{
  const vfloat4 cs   = mul_matrix_vector(worlViewProjCols, a_pos);

  const vfloat4 w    = splat_3(cs);
  const vfloat4 invW = rcp_e(w);

  const vfloat4 w2   = w*invW; // [1.0f, w, w, w]
  const vfloat4 cs1  = shuffle2_xy_xy(cs, w2);

  return cs1*invW;  // float4(clipSpace.x*invW, clipSpace.y*invW, invW, 1.0f); <-- (clipSpace.x, clipSpace.y, 1.0f, w)
}


static inline vfloat4 swglClipSpaceToScreenSpaceTransformSSE(const vfloat4 a_pos, const vfloat4 viewportf) // pre (g_pContext != nullptr)
{
  const vfloat4 xyuu = a_pos*const_half_one + const_half_one;
  const vfloat4 vpzw = shuffle_zwzw(viewportf);
  const vfloat4 ss   = viewportf - const_half_one + xyuu*vpzw;

  return shuffle2_xy_zw(ss, a_pos);
}


void HWImpl_SSE1::VertexShader(const float* v_in4f, float* v_out4f, int a_numVert,
                               const float viewportData[4], const float a_worldViewProjMatrix[16])
{
  set_ftz();

  const float4x4 worldViewProjMatrix(a_worldViewProjMatrix);
  const auto& m = worldViewProjMatrix;

  vfloat4  worlViewProjCols[4] = { load_u(&a_worldViewProjMatrix[0]),
                                   load_u(&a_worldViewProjMatrix[4]),
                                   load_u(&a_worldViewProjMatrix[8]),
                                   load_u(&a_worldViewProjMatrix[12])};

  transpose4(worlViewProjCols[0],
             worlViewProjCols[1],
             worlViewProjCols[2],
             worlViewProjCols[3]);

  const vfloat4 viewportv = load_u(viewportData);

  for (int i = 0; i < a_numVert; i++)
  {
    const vfloat4 oldVal = load(v_in4f + i * 4);

    const vfloat4 vClipSpace   = swglVertexShaderTransformSSE(worlViewProjCols, oldVal);
    const vfloat4 vScreenSpace = swglClipSpaceToScreenSpaceTransformSSE(vClipSpace, viewportv);

    store(v_out4f + i * 4, vScreenSpace);
  }

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline vfloat4 colorSwap(const vfloat4 a_col)
{
  return shuffle_zyxw(a_col);
}

static inline vfloat4 edgeFunction2(vfloat4 a, vfloat4 b, vfloat4 c) // actuattly just a mixed product ... :)
{
  const vfloat4 ay = splat_1(a);
  const vfloat4 by = splat_1(b);
  const vfloat4 cy = splat_1(c);

  return sub_s(mul_s(c-a,   by-ay),
               mul_s(cy-ay, b-a));
}


void HWImpl_SSE1::TriangleSetUp(const SWGL_Context* a_pContext, const Batch* pBatch, int i1, int i2, int i3,
                                TriangleLocal* t1)
{
  const float* vpos = (const float*)pBatch->vertPos.data();
  const float* vcol = (const float*)pBatch->vertColor.data();
  const float* vtex = (const float*)pBatch->vertTexCoord.data();

  const vfloat4 v1  = load(vpos + i1 * 4);
  const vfloat4 v2  = load(vpos + i2 * 4);
  const vfloat4 v3  = load(vpos + i3 * 4);

  t1->v1 = v1;
  t1->v2 = v2;
  t1->v3 = v3;

  const vfloat4 bbMin = min( min(v1,v2), v3);
  const vfloat4 bbMax = max( max(v1,v2), v3);

  const vint4 bbMinI  = to_int32(bbMin);
  const vint4 bbMaxI  = to_int32(bbMax);

  t1->bb_iminX = extract_0(bbMinI);
  t1->bb_imaxX = extract_0(bbMaxI);
  t1->bb_iminY = extract_1(bbMinI);
  t1->bb_imaxY = extract_1(bbMaxI);

  vfloat4 c1  = colorSwap(load(vcol + i1 * 4));
  vfloat4 c2  = colorSwap(load(vcol + i2 * 4));
  vfloat4 c3  = colorSwap(load(vcol + i3 * 4));

  const vfloat4 tx1 = load_u(vtex + i1 * 2);
  const vfloat4 tx2 = load_u(vtex + i2 * 2);
  const vfloat4 tx3 = load_u(vtex + i3 * 2);

  const bool triangleIsTextured = pBatch->state.texure2DEnabled && (pBatch->state.slot_GL_TEXTURE_2D < (GLuint)a_pContext->m_texTop);

  if (triangleIsTextured)
  {
    const SWGL_TextureStorage& tex = a_pContext->m_textures[pBatch->state.slot_GL_TEXTURE_2D];

    const vfloat4 vtexwh00 = {(float)tex.w, (float)tex.h, 0.0f, 0.0f};

    t1->texS.pitch = tex.pitch;   // tex.w; // !!! this is for textures with billet
    t1->texS.w     = tex.w;       // tex.w; // !!! this is for textures with billet
    t1->texS.h     = tex.h;
    t1->texS.data  = tex.texdata; // &tex.data[0]; // !!! this is for textures with billet
    t1->tex_txwh   = vtexwh00;

    ///////////////////////////////////////////////////////////////////////////////////////////////////// FUCKING FUCK! FUCK LEGACY STATES! FUCK OPENGL!
    if (tex.modulateMode == GL_REPLACE) // don't apply vertex color, just take color from texture
    {
      if (tex.format == GL_RGBA)
      {
        c1 = const_1111;
        c2 = const_1111;
        c3 = const_1111;
      }
      else if (tex.format == GL_ALPHA)
      {
        c1 = setWtoOne(c1);
        c2 = setWtoOne(c2);
        c3 = setWtoOne(c3);
      }
    }
  }
  ///////////////////////////////////////////////////////////////////////////////////////////////////// FUCKING FUCK! FUCK LEGACY STATES! FUCK OPENGL!

#ifdef PERSP_CORRECT

  if (pBatch->state.depthTestEnabled)
  {
    const vfloat4 invZ1 = splat_2(v1);
    const vfloat4 invZ2 = splat_2(v2);
    const vfloat4 invZ3 = splat_2(v3);

    t1->c1 = c1*invZ1;
    t1->c2 = c2*invZ2;
    t1->c3 = c3*invZ3;

    t1->t1 = tx1*invZ1;
    t1->t2 = tx2*invZ2;
    t1->t3 = tx3*invZ3;
  }
  else
  {
    t1->c1 = c1;
    t1->c2 = c2;
    t1->c3 = c3;

    t1->t1 = tx1;
    t1->t2 = tx2;
    t1->t3 = tx3;
  }

#endif

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline vfloat4 CalcWeights(const vfloat4 psXY)
{
  const vfloat4 psXYfrac = psXY - cvex::floor(psXY);
  const vfloat4 psXYfrac1 = const_1111 - psXYfrac;                                        // ? ? (1-y) (1-x)
  const vfloat4 w_x       = _mm_unpacklo_ps(psXYfrac1, psXYfrac);                         // ? ?     x (1-x) //#TODO: change implementation!!!
  const vfloat4 w_x2      = _mm_movelh_ps(w_x, w_x);                                      // x (1-x) x (1-x) //#TODO: change implementation!!!
  const vfloat4 w_y       = _mm_shuffle_ps(psXYfrac1, psXYfrac, _MM_SHUFFLE(1, 1, 1, 1)); // y y (1-y) (1-y) //#TODO: change implementation!!!
  return w_x2*w_y;  // complete weight vector
}


 inline __m128 read_imagef_sse(const int* data, const int w, const int h, const int pitch, const __m128 wh, const __m128 tc)
 {
   const __m128  ffxy = _mm_min_ps(_mm_max_ps(_mm_sub_ps(_mm_mul_ps(wh, tc), const_half_one), const_0000), _mm_sub_ps(wh, const_1111));
   //const __m128  ffxy = _mm_sub_ps(_mm_mul_ps(wh, tc), const_half_one);
   const __m128i iixy = _mm_cvtps_epi32(ffxy);

   int px = _mm_cvtsi128_si32(iixy);
   int py = _mm_cvtsi128_si32(_mm_shuffle_epi32(iixy, _MM_SHUFFLE(1, 1, 1, 1)));

   const int* p0 = data + px + py * pitch; // pointer to first pixel

   // Load the data (2 pixels in one load)
   const __m128i p12 = _mm_loadl_epi64((const __m128i*)&p0[0 * pitch]);
   const __m128i p34 = _mm_loadl_epi64((const __m128i*)&p0[1 * pitch]);

   __m128 weight = CalcWeights(ffxy);

   // convert RGBA RGBA RGBA RGAB to RRRR GGGG BBBB AAAA (AoS to SoA)
   const __m128i p1234 = _mm_unpacklo_epi8(p12, p34);
   const __m128i p34xx = _mm_unpackhi_epi64(p1234, _mm_setzero_si128());
   const __m128i p1234_8bit = _mm_unpacklo_epi8(p1234, p34xx);

   // extend to 16bit
   const __m128i pRG = _mm_unpacklo_epi8(p1234_8bit, _mm_setzero_si128());
   const __m128i pBA = _mm_unpackhi_epi8(p1234_8bit, _mm_setzero_si128());

   // convert weights to integer
   weight = _mm_mul_ps(weight, const_256);
   __m128i weighti = _mm_cvtps_epi32(weight); // w4 w3 w2 w1
   weighti = _mm_packs_epi32(weighti, weighti); // 32->2x16bit

   //outRG = [w1*R1 + w2*R2 | w3*R3 + w4*R4 | w1*G1 + w2*G2 | w3*G3 + w4*G4]
   const __m128i outRG = _mm_madd_epi16(pRG, weighti);
   //outBA = [w1*B1 + w2*B2 | w3*B3 + w4*B4 | w1*A1 + w2*A2 | w3*A3 + w4*A4]
   const __m128i outBA = _mm_madd_epi16(pBA, weighti);

   const __m128i color255  = _mm_srli_epi32(_mm_hadd_epi32(outRG, outBA), 8);
   const __m128  finColor  = _mm_mul_ps(const_255_inv, _mm_cvtepi32_ps(color255));

   return _mm_shuffle_ps(finColor, finColor, _MM_SHUFFLE(3, 0, 1, 2)); // swap red and blue
 }

inline static __m128 wrapTexCoord(const __m128 a_texCoord)
{
  const __m128 texCoord2   = _mm_sub_ps(a_texCoord, _mm_floor_ps(a_texCoord));
  const __m128 a_texCoord3 = _mm_add_ps(texCoord2, const_1111);
  const __m128 lessMask    = _mm_cmplt_ps(texCoord2, _mm_setzero_ps());
  return _mm_or_ps(_mm_and_ps(lessMask, a_texCoord3), _mm_andnot_ps(lessMask, texCoord2));
}

inline __m128 tex2D_sse(const TexSampler& sampler, const __m128 texCoord, const __m128 txwh)
{
  return read_imagef_sse(sampler.data, sampler.w, sampler.h, sampler.pitch, txwh, wrapTexCoord(texCoord));
}

struct Colored2D
{
  static inline vfloat4 DrawPixel(const TriangleLocal& tri, const vfloat4& w)
  {
    return tri.c1*splat_0(w) + tri.c3*splat_1(w) + tri.c2*splat_2(w);
  }

};

struct Colored3D
{
  static inline vfloat4 DrawPixel(const TriangleLocal& tri, const vfloat4& w, const vfloat4& zInv)
  {
    vfloat4 clr = tri.c1*splat_0(w) + tri.c3*splat_1(w) + tri.c2*splat_2(w);

  #ifdef PERSP_CORRECT
    const vfloat4 z1 = rcp_s(zInv);
    const vfloat4 z  = splat_0(z1);
    clr = clr* z;
  #endif

    return clr;
  }

};

struct Textured3D
{
  static inline vfloat4 DrawPixel(const TriangleLocal& tri, const vfloat4& w, const vfloat4& zInv)
  {
    const vfloat4 w0 = splat_0(w);
    const vfloat4 w1 = splat_1(w);
    const vfloat4 w2 = splat_2(w);

    const vfloat4 cc1 = tri.c1*w0;
    const vfloat4 cc2 = tri.c3*w1;
    const vfloat4 cc3 = tri.c2*w2;

    vfloat4 clr = cc1 + cc2 + cc3;

    vfloat4 tc = tri.t1*w0 + tri.t3*w1 + tri.t2*w2;

  #ifdef PERSP_CORRECT
    const vfloat4 z1 = rcp_s(zInv);
    const vfloat4 z  = splat_0(z1);
    clr = clr*z;
    tc  = tc*z;
  #endif

    const __m128 texColor = tex2D_sse(tri.texS, tc, tri.tex_txwh);
    //const vfloat4 texColor = {1.0f, 1.0f, 1.0f, 1.0f};
    return clr*texColor;
  }

};


template<typename ROP>
void RasterizeTriHalfSpaceSimple2D(const TriangleLocal& tri, int tileMinX, int tileMinY, FrameBuffer* frameBuf)
{
  // Bounding rectangle
  const int minx = std::max(tri.bb_iminX - tileMinX, 0);
  const int miny = std::max(tri.bb_iminY - tileMinY, 0);
  const int maxx = std::min(tri.bb_imaxX - tileMinX, frameBuf->w - 1);
  const int maxy = std::min(tri.bb_imaxY - tileMinY, frameBuf->h - 1);

  int*     cbuff = frameBuf->cbuffer;
  float*   zbuff = frameBuf->zbuffer;

  const vfloat4 vTileMinX = to_float32(make_vint(tileMinX, tileMinX, tileMinX, 0));
  const vfloat4 vTileMinY = to_float32(make_vint(tileMinY, tileMinY, tileMinY, 0));

  vfloat4 vx = tri.v3;
  vfloat4 vy = tri.v2;
  vfloat4 vz = tri.v1;
  vfloat4 vw = {0.0f,0.0f,0.0f,0.0f};
  transpose4(vx,vy,vz,vw);
  vx = vx - vTileMinX;
  vy = vy - vTileMinX;

  const vfloat4 vMinX = to_float32(make_vint(minx, minx, minx, 0));
  const vfloat4 vMinY = to_float32(make_vint(miny, miny, miny, 0));

  const vfloat4 vDx   = vx - shuffle_yzxw(vx);
  const vfloat4 vDy   = vy - shuffle_yzxw(vy);

  const vfloat4 vC    = vDy*vx - vDx*vy;
  const vfloat4 vCy   = vC + vDx*vMinY - vDy*vMinX;

  const vfloat4 triAreaInv  = rcp_s(edgeFunction2(tri.v1, tri.v3, tri.v2));
  const vfloat4 triAreaInvV = splat_0(triAreaInv);

  int* colorBuffer = frameBuf->cbuffer + miny * frameBuf->pitch;

  vfloat4 Cy = vCy;

  // Scan through bounding rectangle
  for (int y = miny; y <= maxy; y++)
  {
    vfloat4 Cx = Cy;

    for (int x = minx; x <= maxx; x++)
    {
      if(cmpgt_all_xyz(Cx, g_epsE3))
      {
        const vfloat4 w = triAreaInvV*Cx;
        colorBuffer[x] = color_compress_bgra(ROP::DrawPixel(tri, w));
      }

      Cx = Cx - vDy;
    }

    Cy = Cy + vDx;

    colorBuffer += frameBuf->pitch;
  }

}

template<typename ROP>
void RasterizeTriHalfSpaceSimple3D(const TriangleLocal& tri, int tileMinX, int tileMinY, FrameBuffer* frameBuf)
{
  // Bounding rectangle
  const int minx = std::max(tri.bb_iminX - tileMinX, 0);
  const int miny = std::max(tri.bb_iminY - tileMinY, 0);
  const int maxx = std::min(tri.bb_imaxX - tileMinX, frameBuf->w - 1);
  const int maxy = std::min(tri.bb_imaxY - tileMinY, frameBuf->h - 1);

  int*     cbuff = frameBuf->cbuffer;
  float*   zbuff = frameBuf->zbuffer;

  const vfloat4 vTileMinX = to_float32(make_vint(tileMinX, tileMinX, tileMinX, 0));
  const vfloat4 vTileMinY = to_float32(make_vint(tileMinY, tileMinY, tileMinY, 0));

  vfloat4 vx = tri.v3;
  vfloat4 vy = tri.v2;
  vfloat4 vz = tri.v1;
  vfloat4 vw = {0.0f,0.0f,0.0f,0.0f};
  transpose4(vx,vy,vz,vw);
  vx = vx - vTileMinX;
  vy = vy - vTileMinX;

  const vfloat4 vMinX = to_float32(make_vint(minx, minx, minx, 0));
  const vfloat4 vMinY = to_float32(make_vint(miny, miny, miny, 0));

  const vfloat4 vDx   = vx - shuffle_yzxw(vx);
  const vfloat4 vDy   = vy - shuffle_yzxw(vy);

  const vfloat4 vC    = vDy*vx - vDx*vy;
  const vfloat4 vCy   = vC + vDx*vMinY - vDy*vMinX;

  const vfloat4 triAreaInv  = rcp_s(edgeFunction2(tri.v1, tri.v3, tri.v2)); // const float areaInv = 1.0f / fabs(Dy31*Dx12 - Dx31*Dy12);
  const vfloat4 triAreaInvV = splat_0(triAreaInv);
  const vfloat4 vertZ       = shuffle_zxyw(vz);

  vfloat4 Cy = vCy;

  int offset = lineOffset(miny, frameBuf->pitch, frameBuf->h);

  // Scan through bounding rectangle
  for (int y = miny; y <= maxy; y++)
  {
    vfloat4 Cx = Cy;

    //if(y != maxy)
    //  _mm_prefetch (zbuff + offset + frameBuf->pitch, _MM_HINT_T0);

    for (int x = minx; x <= maxx; x++)
    {
      if(cmpgt_all_xyz(Cx, g_epsE3))
      {
        const vfloat4 w        = triAreaInvV*Cx;
        const vfloat4 zInvV    = dot3v(w, vertZ);
        const vfloat4 zBuffVal = load_s(zbuff + offset + x);

        if (cmpgt_all_x(zInvV, zBuffVal))
        {
          cbuff[offset + x] = color_compress_bgra(ROP::DrawPixel(tri, w, zInvV));
          store_s(zbuff + offset + x, zInvV);
        }
      }

      Cx = Cx - vDy;
    }

    Cy = Cy + vDx;

    offset += frameBuf->pitch;
  }

}

void HWImpl_SSE1::RasterizeTriangle(const TriangleLocal& tri, int tileMinX, int tileMinY,
                                    FrameBuffer* frameBuf)
{
  set_ftz();

  const auto a_ropT = tri.ropId;

  switch (a_ropT)
  {
  case ROP_Colored2D:
    RasterizeTriHalfSpaceSimple2D<Colored2D>(tri, tileMinX, tileMinY, frameBuf);
    break;

  case ROP_Colored3D:
    RasterizeTriHalfSpaceSimple3D<Colored3D>(tri, tileMinX, tileMinY, frameBuf);
    break;

  // case ROP_TexNearest2D:
  // case ROP_TexLinear2D:
  //   RasterizeTriHalfSpace2D<Textured2D>(tri, tileMinX, tileMinY, 
  //                                       frameBuf);
  //   break;
  // 
  case ROP_TexNearest3D:
  case ROP_TexLinear3D:
    RasterizeTriHalfSpaceSimple3D<Textured3D>(tri, tileMinX, tileMinY,
                                              frameBuf);
    break;
  // 
  // case ROP_TexNearest3D_Blend:
  // case ROP_TexLinear3D_Blend:
  //   RasterizeTriHalfSpace3DBlend<Textured3D, Blend_Alpha_OneMinusAlpha>(tri, tileMinX, tileMinY,
  //                                                                        frameBuf);
  //   break;

  default :
    break;
  };
}