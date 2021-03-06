#ifndef SWGL_CONFIG_GUARDIAN
#define SWGL_CONFIG_GUARDIAN

#define NUM_THREADS     2
#define NUM_THREADS_AUX (NUM_THREADS-1)

#define NOWINDOW false
#define PERSP_CORRECT

//#define MEASURE_NOLOAD_PERF
#define MEASURE_STATS

#ifndef FULL_GL
  #define FULL_GL
#endif

#include <cstdint>
#include "LiteMath.h"
#include "TiledFrameBuffer.h"

using FrameBufferType = FrameBufferTwoLvl<uint32_t,64,4,4>;
using FBColorType     = FrameBufferType::ColorType;

struct FrameBuffer
{
  FrameBuffer() : w(0), h(0), pitch(0), dummy(0), vx(0), vy(0), vw(0), vh(0) {}

  int w;
  int h;
  int pitch;
  int dummy;

  int vx; // viewport min x
  int vy; // viewport min y
  int vw; // viewport width
  int vh; // viewport height

  using ColorType = FrameBufferType::ColorType;
  FrameBufferType* m_pImpl;  

  inline ColorType* TileColor(int x, int y) { return m_pImpl->TileColor(x,y); }
  inline float*     TileDepth(int x, int y) { return m_pImpl->TileDepth(x,y); }

  inline ColorType* PixelColor(int x, int y) { return m_pImpl->PixelColor(x,y); }
  inline float*     PixelDepth(int x, int y) { return m_pImpl->PixelDepth(x,y); }

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline int imax(int a, int b) { return (a > b) ? a : b; }
static inline int imin(int a, int b) { return (a < b) ? a : b; }
static inline int iround(float f)    { return (int)f; }

template<typename T> static inline  T TriAreaInvCast(const int a_areaInvInt)           { return T(1.0f / fabs(float(a_areaInvInt))); }                             // for floating point pixel processing
template<>           inline  float    TriAreaInvCast<float>(const int a_areaInvInt)    { return 1.0f / fabs(float(a_areaInvInt)); }                                // for floating point pixel processing
template<>           inline  uint32_t TriAreaInvCast<uint32_t>(const int a_areaInvInt) { return (unsigned int)(0xFFFFFFFF) / (unsigned int)(abs(a_areaInvInt)); }  // for fixed    point pixel processing

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum RasterOp { ROP_FillColor = 1,

                ROP_Colored2D,
                ROP_Colored3D,
                ROP_TexNearest2D,
                ROP_TexNearest3D,
                ROP_TexLinear2D,
                ROP_TexLinear3D,

                ROP_Colored2D_Blend,
                ROP_Colored3D_Blend,
                ROP_TexNearest2D_Blend,
                ROP_TexNearest3D_Blend,
                ROP_TexLinear2D_Blend,
                ROP_TexLinear3D_Blend,
                ROP_FillStencil,
};

enum BlendOp { BlendOp_None               = 0, // no blending, replace color
               BlendOp_AlphaOneMinusAlpha = 1, // glBlendFunc(GL_ALPHA, GL_ONE_MINUS_ALPHS)
               BlendOp_Add,                    // glBlendFunc(GL_ONE, GL_ONE)
               BlendOp_Mul,                    // glBlendFunc(GL_DST_COLOR, GL_ZERO).
             };


struct CVEX_ALIGNED(16) TexSampler
{
  int w;
  int h;
  int pitch;
  const int* data;
};

CVEX_ALIGNED(16) struct HWImplementationPureCpp
{
  struct TriangleType
  {
    TriangleType()
    {
      texS.data = nullptr;
      texS.w    = 0;
      texS.h    = 0;
      //curr_sval  = 0;
      //curr_smask = 0;
      psoId = -1;
      ropId = ROP_FillColor;
      bopId = BlendOp_None;
      fbId  = 0;
      flags = 0;
    }
    
    enum {TRI_VERT_COLOR_WHITE = 1, TRI_VERT_SAME_COLOR = 2};

    const bool IsWhite()          const { return (flags & TRI_VERT_COLOR_WHITE) != 0; }
    const bool HasSameVertColor() const { return (flags & TRI_VERT_SAME_COLOR) != 0; }
  
    LiteMath::float4 v1;
    LiteMath::float4 v2;
    LiteMath::float4 v3;

    LiteMath::float4 c1;
    LiteMath::float4 c2;
    LiteMath::float4 c3;

    LiteMath::float2 t1;
    LiteMath::float2 t2;
    LiteMath::float2 t3;

    int bb_iminX;
    int bb_imaxX;
    int bb_iminY;
    int bb_imaxY;

    int triSize; // it is important for fixpoint rasterizer to know triangle size. VL triangles have to be clipped again.
    int triArea;

    int         psoId;
    TexSampler  texS;
    RasterOp    ropId;
    BlendOp     bopId;
    int         fbId;
    int         flags;
    FBColorType fillCol;
    //uint8_t curr_sval;
    //uint8_t curr_smask;
  };


  static void memset32(int32_t* a_data, int32_t val, int32_t numElements);

  static bool AABBTriangleOverlap(const TriangleType& a_tri, const int tileMinX, const int tileMinY, const int tileMaxX, const int tileMaxY);

  static void VertexShader(const float* v_in4f, float* v_out4f, int a_numVert, 
                           const float viewportData[4], const LiteMath::float4x4 worldViewProjMatrix);

  static void RasterizeTriangle(const TriangleType& tri, FrameBuffer* frameBuf);
};


struct HWImplBlock4x4_CVEX : public HWImplementationPureCpp
{
  static void RasterizeTriangle(const TriangleType& tri, FrameBuffer* frameBuf);
};


struct HWImplBlock8x2_CVEX : public HWImplementationPureCpp
{
  static void RasterizeTriangle(const TriangleType& tri, FrameBuffer* frameBuf);
};

struct HWImplBlock16x1_CVEX : public HWImplementationPureCpp
{
  static void RasterizeTriangle(const TriangleType& tri, FrameBuffer* frameBuf);
};


struct HWImplBlock4x4Fixp_CVEX : public HWImplementationPureCpp
{
  static void RasterizeTriangle(const TriangleType& tri, FrameBuffer* frameBuf);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using HWImpl = HWImplBlock4x4_CVEX;
//using HWImpl = HWImplBlock8x2_CVEX; // does not works with binned FB currently due to we removed FB_BILLET
//using HWImpl = HWImplBlock16x1_CVEX;
//using HWImpl = HWImplBlockLine4x4Fixp_CVEX;

using Triangle = HWImpl::TriangleType;

#endif
