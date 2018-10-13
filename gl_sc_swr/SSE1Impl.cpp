#include <cstdint>
#include <algorithm>

#include "SSE1Impl.h"
#include "swgl.h"

#ifdef WIN32
#undef min
#undef max
#endif

// #NOTE: _mm_maskmoveu_si128

using TriangleLocal = HWImpl_SSE1::TriangleType;

void HWImpl_SSE1::memset32(int32_t* a_data, int32_t a_val, int32_t numElements)
{
  uintptr_t ip = reinterpret_cast<uintptr_t>(a_data);

  if ((numElements % 32 == 0) && (ip%16 == 0))
  {
    const vint4 val = splat_1to4(a_val);

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

static const vint4 const_maskXYZ     = make_vint4(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000);

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


static inline __m128  swglVertexShaderTransformSSE(const __m128 worlViewProjCols[4], __m128 a_pos) // pre (pBatch != nullptr)
{
  const __m128 cs   = mul_matrix_vector(worlViewProjCols, a_pos);

  const __m128 w    = _mm_shuffle_ps(cs, cs, _MM_SHUFFLE(3, 3, 3, 3));
  const __m128 invW = _mm_rcp_ps(w);

  const __m128 w2   = _mm_mul_ss(w, invW);                              // [1.0f, w, w, w]
  const __m128 cs1  = _mm_shuffle_ps(cs, w2, _MM_SHUFFLE(1, 0, 1, 0));  // [cs.x, cs.y, 1.0f, w]

  return _mm_mul_ps(cs1, invW);  // float4(clipSpace.x*invW, clipSpace.y*invW, invW, 1.0f); <-- (clipSpace.x, clipSpace.y, 1.0f, w)
}


static inline __m128 swglClipSpaceToScreenSpaceTransformSSE(__m128 a_pos, const __m128 viewportf) // pre (g_pContext != nullptr)
{
  const __m128 xyuu = _mm_add_ps(_mm_mul_ps(a_pos, const_half_one), const_half_one);
  const __m128 vpzw = _mm_shuffle_ps(viewportf, viewportf, _MM_SHUFFLE(3, 2, 3, 2));
  const __m128 ss   = _mm_add_ps(_mm_sub_ps(viewportf, const_half_one), _mm_mul_ps(xyuu, vpzw));

  return _mm_shuffle_ps(ss, a_pos, _MM_SHUFFLE(3, 2, 1, 0));
}


void HWImpl_SSE1::VertexShader(const float* v_in4f, float* v_out4f, int a_numVert,
                               const float viewportData[4], const float a_worldViewProjMatrix[16])
{
  _MM_SET_ROUNDING_MODE(_MM_ROUND_TOWARD_ZERO);

  const float4   viewportf(viewportData[0], viewportData[1], viewportData[2], viewportData[3]);
  const float4x4 worldViewProjMatrix(a_worldViewProjMatrix);

  const auto& m = worldViewProjMatrix;

  __m128  worlViewProjCols[4];
  for (int col = 0; col < 4; col++)
    worlViewProjCols[col] = _mm_set_ps(m.M(col, 3), m.M(col, 2), m.M(col, 1), m.M(col, 0));

  const __m128 viewportv = _mm_set_ps(viewportf.w, viewportf.z, viewportf.y, viewportf.x);

  for (int i = 0; i < a_numVert; i++)
  {
    const __m128 oldVal = _mm_load_ps(v_in4f + i * 4);

    const __m128 vClipSpace   = swglVertexShaderTransformSSE(worlViewProjCols, oldVal);
    const __m128 vScreenSpace = swglClipSpaceToScreenSpaceTransformSSE(vClipSpace, viewportv);

    _mm_store_ps(v_out4f + i * 4, vScreenSpace);
  }

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline __m128 colorSwap(const __m128 a_col)
{
  return _mm_shuffle_ps(a_col, a_col, _MM_SHUFFLE(3, 0, 1, 2));
}

static inline __m128 edgeFunction2(__m128 a, __m128 b, __m128 c) // actuattly just a mixed product ... :)
{
  __m128 ay = _mm_shuffle_ps(a, a, _MM_SHUFFLE(1, 1, 1, 1));
  __m128 by = _mm_shuffle_ps(b, b, _MM_SHUFFLE(1, 1, 1, 1));
  __m128 cy = _mm_shuffle_ps(c, c, _MM_SHUFFLE(1, 1, 1, 1));

   return _mm_sub_ss(_mm_mul_ss(_mm_sub_ss(c,a),   _mm_sub_ss(by,ay)),
                     _mm_mul_ss(_mm_sub_ss(cy,ay), _mm_sub_ss(b,a)));
}

// static inline __m128 abs_ps(__m128 x)
// {
//   static const __m128 sign_mask = _mm_set1_ps(-0.f); // -0.f = 1 << 31
//   return _mm_andnot_ps(sign_mask, x);
// }


void HWImpl_SSE1::TriangleSetUp(const SWGL_Context* a_pContext, const Batch* pBatch, int i1, int i2, int i3,
                                TriangleLocal* t1)
{
  const float* vpos = (const float*)pBatch->vertPos.data();
  const float* vcol = (const float*)pBatch->vertColor.data();
  const float* vtex = (const float*)pBatch->vertTexCoord.data();

  const __m128 v1  = _mm_load_ps(vpos + i1 * 4);
  const __m128 v2  = _mm_load_ps(vpos + i2 * 4);
  const __m128 v3  = _mm_load_ps(vpos + i3 * 4);
                   
  const __m128 c1  = colorSwap(_mm_load_ps(vcol + i1 * 4));
  const __m128 c2  = colorSwap(_mm_load_ps(vcol + i2 * 4));
  const __m128 c3  = colorSwap(_mm_load_ps(vcol + i3 * 4));

  const __m128 tx1 = _mm_loadu_ps(vtex + i1 * 2);
  const __m128 tx2 = _mm_loadu_ps(vtex + i2 * 2);
  const __m128 tx3 = _mm_loadu_ps(vtex + i3 * 2);

  t1->v1 = v1;
  t1->v2 = v2;
  t1->v3 = v3;

  t1->c1 = c1;
  t1->c2 = c2;
  t1->c3 = c3;

  t1->t1 = tx1;
  t1->t2 = tx2;
  t1->t3 = tx3;

  const __m128 bbMin   = _mm_min_ps(_mm_min_ps(v1,v2), v3);
  const __m128 bbMax   = _mm_max_ps(_mm_max_ps(v1,v2), v3);

  const __m128i bbMinI = _mm_cvtps_epi32(bbMin);
  const __m128i bbMaxI = _mm_cvtps_epi32(bbMax);

  t1->bb_iminX = _mm_cvtsi128_si32(bbMinI);
  t1->bb_imaxX = _mm_cvtsi128_si32(bbMaxI);

  t1->bb_iminY = _mm_cvtsi128_si32(_mm_shuffle_epi32(bbMinI, _MM_SHUFFLE(0,0,1,1)));
  t1->bb_imaxY = _mm_cvtsi128_si32(_mm_shuffle_epi32(bbMaxI, _MM_SHUFFLE(0,0,1,1)));

  const bool triangleIsTextured = pBatch->state.texure2DEnabled && (pBatch->state.slot_GL_TEXTURE_2D < (GLuint)a_pContext->m_texTop);

  if (triangleIsTextured)
  {
    const SWGL_TextureStorage& tex = a_pContext->m_textures[pBatch->state.slot_GL_TEXTURE_2D];

    t1->texS.pitch = tex.pitch;   // tex.w; // !!! this is for textures with billet
    t1->texS.w     = tex.w;       // tex.w; // !!! this is for textures with billet
    t1->texS.h     = tex.h;
    t1->texS.data  = tex.texdata; // &tex.data[0]; // !!! this is for textures with billet
    t1->tex_txwh   = _mm_set_ps(0, 0, (float)tex.w, (float)tex.h);                           // #TODO: opt float conversion

    ///////////////////////////////////////////////////////////////////////////////////////////////////// FUCKING FUCK! FUCK LEGACY STATES! FUCK OPENGL!
    if (tex.modulateMode == GL_REPLACE) // don't apply vertex color, just take color from texture
    {
      if (tex.format == GL_RGBA)
      {
        t1->c1 = const_1111;
        t1->c2 = const_1111;
        t1->c3 = const_1111;
      }
      else if (tex.format == GL_ALPHA)
      {
        t1->c1 = setWtoOne(t1->c1);
        t1->c2 = setWtoOne(t1->c2);
        t1->c3 = setWtoOne(t1->c3);
      }
    }
  }
  ///////////////////////////////////////////////////////////////////////////////////////////////////// FUCKING FUCK! FUCK LEGACY STATES! FUCK OPENGL!

#ifdef PERSP_CORRECT

  if (pBatch->state.depthTestEnabled)
  {
    __m128 invZ1 = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(2, 2, 2, 2)); // _mm_set_ps(t1->v1.z, t1->v1.z, t1->v1.z, t1->v1.z);
    __m128 invZ2 = _mm_shuffle_ps(v2, v2, _MM_SHUFFLE(2, 2, 2, 2)); // _mm_set_ps(t1->v2.z, t1->v2.z, t1->v2.z, t1->v2.z);
    __m128 invZ3 = _mm_shuffle_ps(v3, v3, _MM_SHUFFLE(2, 2, 2, 2)); // _mm_set_ps(t1->v3.z, t1->v3.z, t1->v3.z, t1->v3.z);

    t1->c1 = _mm_mul_ps(t1->c1, invZ1);
    t1->c2 = _mm_mul_ps(t1->c2, invZ2);
    t1->c3 = _mm_mul_ps(t1->c3, invZ3);

    t1->t1 = _mm_mul_ps(t1->t1, invZ1);
    t1->t2 = _mm_mul_ps(t1->t2, invZ2);
    t1->t3 = _mm_mul_ps(t1->t3, invZ3);
  }

#endif

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline __m128 CalcWeights(const __m128 psXY)
{
  const __m128 psXYfloor = _mm_floor_ps(psXY); // use this line for if you have SSE4
  //__m128 psXYfloor = _mm_cvtepi32_ps(_mm_cvtps_epi32(psXY));
  const __m128 psXYfrac = _mm_sub_ps(psXY, psXYfloor);            // = frac(psXY)

  const __m128 psXYfrac1 = _mm_sub_ps(const_1111, psXYfrac);       // ? ? (1-y) (1-x)
  const __m128 w_x       = _mm_unpacklo_ps(psXYfrac1, psXYfrac);   // ? ?     x (1-x)
  const __m128 w_x2      = _mm_movelh_ps(w_x, w_x);                // x (1-x) x (1-x)
  const __m128 w_y       = _mm_shuffle_ps(psXYfrac1, psXYfrac, _MM_SHUFFLE(1, 1, 1, 1)); // y y (1-y) (1-y)

  return _mm_mul_ps(w_x2, w_y);  // complete weight vector
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
  static inline __m128 DrawPixel(const TriangleLocal& tri, const __m128& w)
  {
    const __m128 cc1 = _mm_mul_ps(tri.c1, _mm_shuffle_ps(w, w, _MM_SHUFFLE(0, 0, 0, 0)));
    const __m128 cc2 = _mm_mul_ps(tri.c3, _mm_shuffle_ps(w, w, _MM_SHUFFLE(1, 1, 1, 1)));
    const __m128 cc3 = _mm_mul_ps(tri.c2, _mm_shuffle_ps(w, w, _MM_SHUFFLE(2, 2, 2, 2)));

    return _mm_add_ps(cc1, _mm_add_ps(cc2, cc3));  // RealColorToUint32_BGRA_SIMD(clr);
  }

};

struct Colored3D
{
  static inline __m128 DrawPixel(const TriangleLocal& tri, const __m128& w, const __m128& zInv)
  {
    const __m128 cc1 = _mm_mul_ps(tri.c1, _mm_shuffle_ps(w, w, _MM_SHUFFLE(0, 0, 0, 0)));
    const __m128 cc2 = _mm_mul_ps(tri.c3, _mm_shuffle_ps(w, w, _MM_SHUFFLE(1, 1, 1, 1)));
    const __m128 cc3 = _mm_mul_ps(tri.c2, _mm_shuffle_ps(w, w, _MM_SHUFFLE(2, 2, 2, 2)));

    __m128 clr = _mm_add_ps(cc1, _mm_add_ps(cc2, cc3));

  #ifdef PERSP_CORRECT
    const __m128 z1 = _mm_rcp_ss(zInv);
    const __m128 z = _mm_shuffle_ps(z1, z1, _MM_SHUFFLE(0, 0, 0, 0));
    clr = _mm_mul_ps(clr, z);
  #endif

    return clr; // RealColorToUint32_BGRA_SIMD(clr);
  }

};

struct Textured3D
{
  static inline __m128 DrawPixel(const TriangleLocal& tri, const __m128& w, const __m128& zInv)
  {
    const __m128 w0 = _mm_shuffle_ps(w, w, _MM_SHUFFLE(0, 0, 0, 0));
    const __m128 w1 = _mm_shuffle_ps(w, w, _MM_SHUFFLE(1, 1, 1, 1));
    const __m128 w2 = _mm_shuffle_ps(w, w, _MM_SHUFFLE(2, 2, 2, 2));

    const __m128 cc1 = _mm_mul_ps(tri.c1, w0);
    const __m128 cc2 = _mm_mul_ps(tri.c3, w1);
    const __m128 cc3 = _mm_mul_ps(tri.c2, w2);

    __m128 clr = _mm_add_ps(cc1, _mm_add_ps(cc2, cc3));

    const __m128 t1 = _mm_mul_ps(tri.t1, w0);
    const __m128 t2 = _mm_mul_ps(tri.t3, w1);
    const __m128 t3 = _mm_mul_ps(tri.t2, w2);

    __m128 tc = _mm_add_ps(t1, _mm_add_ps(t2, t3));

  #ifdef PERSP_CORRECT
    const __m128 z1 = _mm_rcp_ss(zInv);
    const __m128 z = _mm_shuffle_ps(z1, z1, _MM_SHUFFLE(0, 0, 0, 0));
    clr = _mm_mul_ps(clr, z);
    tc  = _mm_mul_ps(tc, z);
  #endif

    const __m128 texColor = tex2D_sse(tri.texS, tc, tri.tex_txwh);

    return _mm_mul_ps(clr, texColor);
  }

};

/*
   const __m128 v2xv3xX = _mm_shuffle_ps(v2, v3, _MM_SHUFFLE(0, 0, 0, 0));
   const __m128 v2v3xxX = _mm_shuffle_ps(v2xv3xX, v2xv3xX, _MM_SHUFFLE(0, 0, 2, 0));
   const __m128 v1v3v2X = _mm_shuffle_ps(v2v3xxX, v1, _MM_SHUFFLE(0, 0, 1, 0));      // got _mm_set_ps(0.0f, v1.x, v3.x, v2.x);
   const __m128 v2v1v3X = _mm_shuffle_ps(v1v3v2X, v1v3v2X, _MM_SHUFFLE(0, 0, 2, 1)); // got _mm_set_ps(0.0f, v1.y, v3.y, v2.y) from _mm_set_ps(0.0f, v1.x, v3.x, v2.x);
   
   const __m128 v2xv3xY = _mm_shuffle_ps(v2, v3, _MM_SHUFFLE(1, 1, 1, 1));
   const __m128 v2v3xxY = _mm_shuffle_ps(v2xv3xY, v2xv3xY, _MM_SHUFFLE(0, 0, 2, 0));
   const __m128 v1v3v2Y = _mm_shuffle_ps(v2v3xxY, v1, _MM_SHUFFLE(1, 1, 1, 1));      // got _mm_set_ps(0.0f, v1.y, v3.y, v2.y);
   const __m128 v2v1v3Y = _mm_shuffle_ps(v1v3v2Y, v1v3v2Y, _MM_SHUFFLE(0, 0, 2, 1)); // got _mm_set_ps(0.0f, v2.y, v1.y, v3.y) from _mm_set_ps(0.0f, v2.y, v1.y, v3.y);
   
   const __m128 v1xv2xZ = _mm_shuffle_ps(v1, v2, _MM_SHUFFLE(2, 2, 2, 2));
   const __m128 v1v2xxZ = _mm_shuffle_ps(v1xv2xZ, v1xv2xZ, _MM_SHUFFLE(0, 0, 2, 0));
   const __m128 v1v3v2Z = _mm_shuffle_ps(v1v2xxZ, v3, _MM_SHUFFLE(2, 2, 1, 0));      // got _mm_set_ps(0.0f, v3.z, v2.z, v1.z);
 
 */

inline static __m128 GetColX(const __m128 v1, const __m128 v2, const __m128 v3)
{
  const __m128 v1xv2xZ = _mm_shuffle_ps(v1, v2,           _MM_SHUFFLE(0, 0, 0, 0));
  const __m128 v1v2xxZ = _mm_shuffle_ps(v1xv2xZ, v1xv2xZ, _MM_SHUFFLE(0, 0, 2, 0));
  const __m128 v1v3v2Z = _mm_shuffle_ps(v1v2xxZ, v3,      _MM_SHUFFLE(0, 0, 1, 0));      // got _mm_set_ps(0.0f, v3.z, v2.z, v1.z);
  return v1v3v2Z;
}

inline static __m128 GetColY(const __m128 v1, const __m128 v2, const __m128 v3)
{
  const __m128 v1xv2xZ = _mm_shuffle_ps(v1, v2,           _MM_SHUFFLE(1, 1, 1, 1));
  const __m128 v1v2xxZ = _mm_shuffle_ps(v1xv2xZ, v1xv2xZ, _MM_SHUFFLE(0, 0, 2, 0));
  const __m128 v1v3v2Z = _mm_shuffle_ps(v1v2xxZ, v3,      _MM_SHUFFLE(1, 1, 1, 0));      // got _mm_set_ps(0.0f, v3.z, v2.z, v1.z);
  return v1v3v2Z;
}


template<typename ROP>
void RasterizeTriHalfSpaceSimple2D(const TriangleLocal& tri, int tileMinX, int tileMinY, FrameBuffer* frameBuf)
{
  // Bounding rectangle
  const int minx = std::max(tri.bb_iminX - tileMinX, 0);
  const int miny = std::max(tri.bb_iminY - tileMinY, 0);
  const int maxx = std::min(tri.bb_imaxX - tileMinX, frameBuf->w - 1);
  const int maxy = std::min(tri.bb_imaxY - tileMinY, frameBuf->h - 1);

  int* colorBuffer = frameBuf->cbuffer + miny * frameBuf->pitch;

  const __m128 vTileMinX = _mm_cvtepi32_ps(_mm_set_epi32(0, tileMinX, tileMinX, tileMinX));  
  const __m128 vTileMinY = _mm_cvtepi32_ps(_mm_set_epi32(0, tileMinY, tileMinY, tileMinY)); 

  const __m128 vx = _mm_sub_ps( GetColX(tri.v3, tri.v2, tri.v1), vTileMinX); // same as _mm_sub_ps(_mm_set_ps(0.0f, tri.v1.m128_f32[0], tri.v2.m128_f32[0], tri.v3.m128_f32[0]), vTileMinX);
  const __m128 vy = _mm_sub_ps( GetColY(tri.v3, tri.v2, tri.v1), vTileMinY); // smae as _mm_sub_ps(_mm_set_ps(0.0f, tri.v1.m128_f32[1], tri.v2.m128_f32[1], tri.v3.m128_f32[1]), vTileMinY);

  const __m128 vMinX = _mm_cvtepi32_ps(_mm_set_epi32(0, minx, minx, minx));                                          
  const __m128 vMinY = _mm_cvtepi32_ps(_mm_set_epi32(0, miny, miny, miny));                                              

  const __m128 vDx   = _mm_sub_ps(vx, _mm_shuffle_ps(vx, vx, _MM_SHUFFLE(0, 0, 2, 1)));
  const __m128 vDy   = _mm_sub_ps(vy, _mm_shuffle_ps(vy, vy, _MM_SHUFFLE(0, 0, 2, 1)));

  const __m128 vC    = _mm_sub_ps(_mm_mul_ps(vDy, vx), _mm_mul_ps(vDx, vy));
  const __m128 vCy   = _mm_add_ps(vC, _mm_sub_ps(_mm_mul_ps(vDx, vMinY), _mm_mul_ps(vDy, vMinX)));

  const __m128 triAreaInv  = _mm_rcp_ss(edgeFunction2(tri.v1, tri.v3, tri.v2)); // const float areaInv = 1.0f / fabs(Dy31*Dx12 - Dx31*Dy12);
  const __m128 triAreaInvV = _mm_shuffle_ps(triAreaInv, triAreaInv, _MM_SHUFFLE(0, 0, 0, 0));

  __m128 Cy = vCy;

  // Scan through bounding rectangle
  for (int y = miny; y <= maxy; y++)
  {
    __m128 Cx = Cy;

    for (int x = minx; x <= maxx; x++)
    {
      if ((_mm_movemask_ps(_mm_cmpgt_ps(Cx, g_epsE3)) & 7) == 7)
      {
        const __m128 w = _mm_mul_ps(triAreaInvV, Cx);
        colorBuffer[x] = RealColorToUint32_BGRA_SIMD(ROP::DrawPixel(tri, w));
      }

      Cx = _mm_sub_ps(Cx, vDy);
    }

    Cy = _mm_add_ps(Cy, vDx);

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


  const __m128 vTileMinX = _mm_cvtepi32_ps(_mm_set_epi32(0, tileMinX, tileMinX, tileMinX));  
  const __m128 vTileMinY = _mm_cvtepi32_ps(_mm_set_epi32(0, tileMinY, tileMinY, tileMinY)); 

  const __m128 vx    = _mm_sub_ps( GetColX(tri.v3, tri.v2, tri.v1), vTileMinX);
  const __m128 vy    = _mm_sub_ps( GetColY(tri.v3, tri.v2, tri.v1), vTileMinY);

  const __m128 vMinX = _mm_cvtepi32_ps(_mm_set_epi32(0, minx, minx, minx));                                          
  const __m128 vMinY = _mm_cvtepi32_ps(_mm_set_epi32(0, miny, miny, miny));                                              

  const __m128 vDx   = _mm_sub_ps(vx, _mm_shuffle_ps(vx, vx, _MM_SHUFFLE(0, 0, 2, 1)));
  const __m128 vDy   = _mm_sub_ps(vy, _mm_shuffle_ps(vy, vy, _MM_SHUFFLE(0, 0, 2, 1)));

  const __m128 vC    = _mm_sub_ps(_mm_mul_ps(vDy, vx), _mm_mul_ps(vDx, vy));
  const __m128 vCy   = _mm_add_ps(vC, _mm_sub_ps(_mm_mul_ps(vDx, vMinY), _mm_mul_ps(vDy, vMinX)));

  const __m128 triAreaInv  = _mm_rcp_ss(edgeFunction2(tri.v1, tri.v3, tri.v2)); // const float areaInv = 1.0f / fabs(Dy31*Dx12 - Dx31*Dy12);
  const __m128 triAreaInvV = _mm_shuffle_ps(triAreaInv, triAreaInv, _MM_SHUFFLE(0, 0, 0, 0));

  const __m128 v1xv2xZ = _mm_shuffle_ps(tri.v1, tri.v2,   _MM_SHUFFLE(2, 2, 2, 2));
  const __m128 v1v2xxZ = _mm_shuffle_ps(v1xv2xZ, v1xv2xZ, _MM_SHUFFLE(0, 0, 2, 0));
  const __m128 v3v2v1Z = _mm_shuffle_ps(v1v2xxZ, tri.v3,  _MM_SHUFFLE(2, 2, 1, 0));      // got _mm_set_ps(0.0f, v3.z, v2.z, v1.z);
  const __m128 vertZ   = _mm_shuffle_ps(v3v2v1Z, v3v2v1Z, _MM_SHUFFLE(3, 1, 2, 0));

  __m128 Cy = vCy;

  int offset = lineOffset(miny, frameBuf->pitch, frameBuf->h);

  // Scan through bounding rectangle
  for (int y = miny; y <= maxy; y++)
  {
    __m128 Cx = Cy;

    //if(y != maxy)
    //  _mm_prefetch (zbuff + offset + frameBuf->pitch, _MM_HINT_T0);

    for (int x = minx; x <= maxx; x++)
    {
      if ((_mm_movemask_ps(_mm_cmpgt_ps(Cx, g_epsE3)) & 7) == 7)
      {
        const __m128 w        = _mm_mul_ps(triAreaInvV, Cx);
        const __m128 zInvV    = _mm_dp_ps(w, vertZ, 0x7f);
        const __m128 zBuffVal = _mm_load_ss(zbuff + offset + x);

        if (_mm_movemask_ps(_mm_cmpgt_ss(zInvV, zBuffVal)) & 1)
        {
          cbuff[offset + x] = RealColorToUint32_BGRA_SIMD(ROP::DrawPixel(tri, w, zInvV));
          _mm_store_ss(zbuff + offset + x, zInvV);
        }

      }

      Cx = _mm_sub_ps(Cx, vDy);
    }

    Cy = _mm_add_ps(Cy, vDx);

    offset += frameBuf->pitch;
  }

}

void HWImpl_SSE1::RasterizeTriangle(RasterOp a_ropT, BlendOp a_bopT, const TriangleLocal& tri, int tileMinX, int tileMinY,
                                    FrameBuffer* frameBuf)
{
  _MM_SET_ROUNDING_MODE(_MM_ROUND_TOWARD_ZERO);

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