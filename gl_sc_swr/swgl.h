#pragma once

#include "gl_sc.h"

#ifdef WIN32
  #include <windows.h>
#else
  #include "glx_sc.h"
#endif

#include "LiteMath.h"
#include "config.h"

using namespace LiteMath;

#include <cstdint>
#include <vector>
#include <memory>
#include <fstream>
#include <time.h>
#include <atomic>

#include "alloc16.h"

#include <memory.h>
#include <atomic>

#include "concurrentqueue.h"

#define PI  ((float)3.1415926535)
#define DEG_TO_RAD (PI/(float)180.0)
#define RAD_TO_DEG ((float)180.0/PI)

#define MAX_NUM_VERTICES_IN_BATCH 65536*4                   // just a reccomendation, not a strict reqirenment // 262144
#define MAX_NUM_TRIANGLES_TOTAL (MAX_NUM_VERTICES_IN_BATCH) // strict rule ???

const int MMPSTACKSIZE = 4;  ///< Projection Matrix stack size
const int MMWSTACKSIZE = 32; ///< WolrdView  Matrix stack size

struct SWGL_Timings
{
  SWGL_Timings() { clear(); }
  
  void clear() { msClear = 0.0f; msVertexShader = 0.0f; msTriSetUp = 0.0f; msRasterAndPixelShader = 0.0f; msSwapBuffers = 0.0f; msTotal = 0.0f; msBinRaster = 0.0f; }
  
  float msClear;
  float msVertexShader;
  float msTriSetUp;
  float msRasterAndPixelShader;
  float msSwapBuffers;
  float msTotal;
  float msBinRaster;
};


/**
\brief This is Pipeline State Object (PSO) structure.

*/

struct Pipeline_State_Object
{
  Pipeline_State_Object()
  {
    worldViewMatrix.identity();
    projMatrix.identity();

    depthTestEnabled    = false; // ???
    depthWriteEnabled   = true;  // ???

    stencilTestEnabled  = false;
    stencilWriteEnabled = false;
    stencilMask         = 0;
    stencilValue        = 0;

    colorWriteEnabled   = true;
    alphaBlendEnabled   = false;
    cullFaceEnabled     = false;

    slot_GL_TEXTURE_2D  = -1;
    cullFaceMode        = 0;

    for (int i = 0; i < 4;i++)
      viewport[i] = 0;
  }

  float4x4 worldViewMatrix;     ///< current WolrdView  Matrix
  float4x4 projMatrix;          ///< current WolrdView  Matrix
  float4x4 worldViewProjMatrix; ///< INTERNAL

  bool depthTestEnabled;        ///< current depth test state
  bool depthWriteEnabled;       ///< current depth write state
  bool colorWriteEnabled;       ///< current color write state
  bool cullFaceEnabled;         ///< current cullFace write state

  bool   stencilTestEnabled;    ///< current stencil test state
  bool   stencilWriteEnabled;   ///< current write test state
  GLuint stencilMask;           ///< current stencil mask (see https://www.khronos.org/opengles/sdk/docs/man/xhtml/glStencilMask.xml )
  GLuint stencilValue;          ///< current stencil fill value (see second parameter 'ref' in glStencilFunc(GLenum func, GLint ref, GLuint mask))

  bool alphaBlendEnabled;       ///< current blend state enebale/disable
  bool texure2DEnabled;         ///< current texture mapping state. Equals true if glEnable(GL_TEXTURE_2D)

  GLuint slot_GL_TEXTURE_2D;    ///< this is texture id bind to GL_TEXTURE_2D slot by call glBindTexture(GL_TEXTURE_2D, textureId);
  GLenum cullFaceMode;          ///< cull face mode. Clockwise or counter clockwise.

  int viewport[4];              ///< current active output rectangle (see glViewport and/or glScissor)

  float lineWidth;              ///< current line width
  float lineSmoothWidthRange;   ///< GL_SMOOTH_LINE_WIDTH_RANGE
};

/**
\brief Single geometry batch with a fixed PSO.

*/
struct Batch
{
  void reserve(const size_t a_vertNum)
  {
    vertPos.reserve(a_vertNum);
    vertColor.reserve(vertPos.capacity());
    vertTexCoord.reserve(vertPos.capacity());
    indices.reserve(vertPos.capacity() * 3);
    indicesLines.reserve(vertPos.capacity() * 2);
    indicesPoints.reserve(vertPos.capacity());
    pointSize.reserve(indicesPoints.capacity() / 2);
  }

  void clear()
  {
    pointSize.clear();
    indicesPoints.clear();
    indicesLines.clear();
    indices.clear();
    vertPos.clear();
    vertColor.clear();
    vertTexCoord.clear();
  }

  std::vector<float>  pointSize;
  std::vector<int>    indicesPoints;
  std::vector<int>    indicesLines;
  std::vector<int>    indices;

  std::vector<float4, aligned<float4, 16> > vertPos;
  std::vector<float4, aligned<float4, 16> > vertPosT; // transformed

  std::vector<float4, aligned<float4, 16> > vertColor;
  std::vector<float2, aligned<float4, 16> > vertTexCoord;

  Pipeline_State_Object state; // curr PSO. if changed, switch to next batch ...
};

void SaveBatchToXML(const Batch* a_batch, char* a_fileName,
                    int* a_texData, int a_texW, int a_texH, int a_pitch);

void LoadBatchFromXML(const Batch* a_batch, char* a_fileName);



/**
\brief This is Input State Object (ISO) structure.

*/
struct SWGL_Input
{
  SWGL_Input() : currPrimType(0), lastSizeVert(0), lastSizeInd(0),
                 vertexPosPointer(nullptr), vertexColorPointer(nullptr), vertexNormalPointer(nullptr), vertexTexCoordPointer(nullptr),
                 vertexPosPtrEnabled(false), vertexColorPtrEnabled(false), vertexNormalPtrEnabled(false), vertexTexCoordPtrEnabled(false),
                 vertPosComponents(0), vertColorComponents(0), vertNormalComponents(0), vertTexCoordComponents(0)
  {
    memset(this, 0, sizeof(SWGL_Input));
    inputMatrixMode = GL_MODELVIEW;
    clearDepth      = 1.0f;
    clearStencil    = 0;
    currPointSize   = 1.0f;

    top_mw = 0;
    top_mp = 0;

    m_currBatch.reserve(MAX_NUM_VERTICES_IN_BATCH);
  }

  Batch* getCurrBatch() { return &m_currBatch; } // post (ret != nullptr)
  Batch  m_currBatch;

  ///////////////////////////////////////////////////////////////////////////////////////////////////////

  float4   clearColor4f;            ///< current clear color value
  uint32_t clearColor1u;            ///< INTERNAL
  float    clearDepth;              ///< current clear depth value
  GLint    clearStencil;            ///< current clear stencil value
                                    
  float4   currInputColor;          ///< current input color (see glColor3f)
  float4   currInputNormal;         ///< current input normal (see glNormal3f)
  float2   currInputTexCoord[32];   ///< GL_TEXTURE31 - GL_TEXTURE0 (probably we won't use them all)
                                    
  float    currPointSize;           ///< current point size for drawing points

  Pipeline_State_Object batchState; ///< This state SHOULD NOT BE HERE! PSO MUST BE SEPARATE FROM ISO IN FUTURE !!!
  uint32_t inputMatrixMode;         ///< current matrix mode set with glMatrixMode. GL_MODELVIEW or GL_PROJECTION

  float4x4 mmwstack[MMWSTACKSIZE]; ///< world view matrix stack
  float4x4 mmpstack[MMPSTACKSIZE]; ///< projection matrix stack
  int top_mw;                      ///< world view matrix stack top
  int top_mp;                      ///< projection matrix stack top

  ///////////////////////////////////////////////////////////////////////////////////////////////////////

  GLenum           currPrimType;
  size_t           lastSizeVert;
  size_t           lastSizeInd;

  // ClientStateArrays
  //

  float* vertexPosPointer;
  float* vertexColorPointer;
  float* vertexNormalPointer;
  float* vertexTexCoordPointer;

  bool vertexPosPtrEnabled;
  bool vertexColorPtrEnabled;
  bool vertexNormalPtrEnabled;
  bool vertexTexCoordPtrEnabled;

  int  vertPosComponents;
  int  vertColorComponents;
  int  vertNormalComponents;
  int  vertTexCoordComponents;

  ///////////////////////////////////////////////////////////////////////////////////////////////////////

};

struct CVEX_ALIGNED(16) SWGL_TextureStorage
{
  SWGL_TextureStorage()
  {
     modulateMode = GL_MODULATE;
     format       = GL_RGBA;
     texdata      = nullptr;
     pitch        = 0;
     w            = 0;
     h            = 0;
     data         = std::vector<int>();
  }

  void MakeBoundaryBillet();

  std::vector<int> data;
  int w;
  int h;
  int pitch;
  int* texdata;

  GLint modulateMode;
  GLint format;
};

struct SWGL_DrawList
{
  int                  m_triTop;
  mutable std::vector<Pipeline_State_Object> m_psoArray;
};


enum LOG_MODES { LOG_FOR_DEBUG_ERROR = 0, LOG_ALL = 1, LOG_NORMAL = 2, LOG_MINIMUM = 3, LOG_PERF = 4, LOG_NOTHING = 10 };


struct SWGL_Context
{
  static const int logMode = LOG_NOTHING; // select one from LOG_MODES

#ifdef WIN32

  SWGL_Context() : hbmp(NULL), hdcMem(NULL), hbmOld(NULL), m_width(0), m_height(0),
                   m_tqueue(MAX_NUM_TRIANGLES_TOTAL), m_useTriQueue(false), m_useTiledFB(false)
  {
    InitCommon();
  }

  void Create(HDC a_hdc, int width, int height);

  HBITMAP hbmOld;
  HBITMAP hbmp;
  HDC     hdcMem;
  HDC     m_hdc;

#else
  
  SWGL_Context() : m_pixels(0), m_width(0), m_height(0), m_fwidth(0.0f), m_fheight(0.0f),
                   m_tqueue(MAX_NUM_TRIANGLES_TOTAL), m_useTriQueue(false), m_useTiledFB(false)
  {
    InitCommon();
  }

  void Create(Display *dpy, XVisualInfo *vis, int width, int height);
  __GLXcontextRec glxrec;

#endif

  ~SWGL_Context();

  void Destroy();
  void CopyToScreeen();
  void InitCommon();
  void ResizeCommon(int width, int height);
  
  int*     m_pixels;
  int      m_width, m_height;
  float    m_fwidth, m_fheight;

  SWGL_Input input;
  std::vector<SWGL_TextureStorage> m_textures;

  int m_texTop;

  static std::ofstream* m_pLog;

  SWGL_DrawList   m_drawList;
  SWGL_Timings    m_timeStats;
  FrameBufferType m_tiledFb2;
  
  bool m_useTiledFB;
  bool m_useTriQueue;
  int  m_currTileId;

  moodycamel::ConcurrentQueue<HWImpl::TriangleType> m_tqueue;
  std::vector<FrameBuffer>                          batchFrameBuffers;
};


inline float4x4* swglGetCurrMatrix(SWGL_Context* a_pContext) // pre (a_pContext != nullptr) ; post (returnValue != nullptr)
{
  return (a_pContext->input.inputMatrixMode == GL_MODELVIEW) ? &a_pContext->input.batchState.worldViewMatrix : &a_pContext->input.batchState.projMatrix;
}

void swglDrawBatchTriangles(SWGL_Context* a_pContext, Batch* pBatch, FrameBuffer& frameBuff);
void swglRunBatchVertexShader(SWGL_Context* a_pContext, Batch* pBatch);

void swglAppendTriIndices(SWGL_Context* a_pContext, Batch* pCurr, GLenum currPrimType, size_t lastSizeVert);
int  swglAppendTriIndices2(SWGL_Context* a_pContext, Batch* pCurr, GLenum currPrimType, size_t lastSizeVert, const int* inIndices, int count);
void swglAppendVertices(SWGL_Context* a_pContext, GLenum currPrimType, size_t lastSizeVert, int first, int count);

void swglEnqueueTrianglesFromInput(SWGL_Context* a_pContext, const int* a_inIndices, int a_indicesNum, const SWGL_Input& a_input);

int atomic_add(int* pAddress, int pVal);
void clampTriBBox(Triangle* t1, const FrameBuffer& frameBuff);

inline static bool isIdentityMatrix(const float4x4& a_mat)
{
  float4x4 m;
  m.identity();
  return (memcmp(&a_mat, &m, sizeof(float) * 16) == 0);
}

inline static FrameBuffer swglBatchFb(SWGL_Context* a_pContext, const Pipeline_State_Object& a_state)
{
  FrameBuffer frameBuff;

  frameBuff.w  = a_pContext->m_width;
  frameBuff.h  = a_pContext->m_height;

  frameBuff.vx = a_state.viewport[0];
  frameBuff.vy = a_state.viewport[1];
  frameBuff.vw = a_state.viewport[2];
  frameBuff.vh = a_state.viewport[3];

  frameBuff.m_pImpl = &a_pContext->m_tiledFb2;

  return frameBuff;
}

static inline float4 swglVertexShaderTransform(Batch* pBatch, float4 a_pos) // pre (pBatch != nullptr)
{
  const float4 clipSpace = pBatch->state.worldViewProjMatrix*a_pos;
  const float invW       = (1.0f / fmax(clipSpace.w, 1e-20f));
  return float4(clipSpace.x*invW, clipSpace.y*invW, invW, 1.0f);
}

static inline float4 swglClipSpaceToScreenSpaceTransform(float4 a_pos, const float4 viewportf) // pre (g_pContext != nullptr)
{
  const float fw = viewportf.z;
  const float fh = viewportf.w;

  const float x  = a_pos.x*0.5f + 0.5f;
  const float y  = a_pos.y*0.5f + 0.5f;

  return float4(x*fw - 0.5f + viewportf.x, y*fh - 0.5f + viewportf.y, a_pos.z, a_pos.w);
}

RasterOp swglStateIdFromPSO(const Pipeline_State_Object* a_pso, const SWGL_Context* a_pContext, const bool a_sameColor);

void swglEnqueueBatchTriangles(SWGL_Context* a_pContext, Batch* pBatch, FrameBuffer& frameBuff);


template<typename SetupTriangleType>
inline void calcTriBBox(SetupTriangleType *pTri)
{
  const auto& v1 = pTri->v1;
  const auto& v2 = pTri->v2;
  const auto& v3 = pTri->v3;

  const float minX = (fmin(v1.x, fmin(v2.x, v3.x)) - 1.0f); // 1.0 is correct, don't try 0.5f
  const float maxX = (fmax(v1.x, fmax(v2.x, v3.x)) + 1.0f); // 1.0 is correct, don't try 0.5f
  const float minY = (fmin(v1.y, fmin(v2.y, v3.y)) - 1.0f);
  const float maxY = (fmax(v1.y, fmax(v2.y, v3.y)) + 1.0f);

  pTri->bb_iminX = (int)minX;
  pTri->bb_imaxX = (int)maxX;
  pTri->bb_iminY = (int)minY;
  pTri->bb_imaxY = (int)maxY;
  pTri->triSize  = int(fmax(maxX - minX, maxY - minY));
  pTri->triArea  = int(maxX - minX)*int(maxY - minY);
}

template<typename SetupTriangleType>
inline void swglTriangleSetUp(const SWGL_Context *a_pContext, const Batch *pBatch, int i1, int i2, int i3, int frameBufferId, const bool a_perspCorrect,
                              SetupTriangleType *pTri)
{
  pTri->v1 = pBatch->vertPosT[i1];
  pTri->v2 = pBatch->vertPosT[i2];
  pTri->v3 = pBatch->vertPosT[i3];

  pTri->c1 = pBatch->vertColor[i1];
  pTri->c2 = pBatch->vertColor[i2];
  pTri->c3 = pBatch->vertColor[i3];

  pTri->t1 = pBatch->vertTexCoord[i1];
  pTri->t2 = pBatch->vertTexCoord[i2];
  pTri->t3 = pBatch->vertTexCoord[i3];

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  calcTriBBox(pTri);

  const bool triangleIsTextured = pBatch->state.texure2DEnabled && (pBatch->state.slot_GL_TEXTURE_2D < (GLuint)a_pContext->m_texTop);

  if (triangleIsTextured)
  {
    const SWGL_TextureStorage& tex = a_pContext->m_textures[pBatch->state.slot_GL_TEXTURE_2D];

    pTri->texS.pitch = tex.pitch;   // tex.w; // !!! this is for textures with billet
    pTri->texS.w     = tex.w;       // tex.w; // !!! this is for textures with billet
    pTri->texS.h     = tex.h;
    pTri->texS.data  = tex.texdata; // &tex.data[0]; // !!! this is for textures with billet

    ///////////////////////////////////////////////////////////////////////////////////////////////////// FUCKING FUCK! FUCK LEGACY STATES! FUCK OPENGL!
    if (tex.modulateMode == GL_REPLACE) // don't apply vertex color, just take color from texture
    {
      if (tex.format == GL_RGBA)
      {
        pTri->c1 = float4(1, 1, 1, 1);
        pTri->c2 = float4(1, 1, 1, 1);
        pTri->c3 = float4(1, 1, 1, 1);
      }
      else if (tex.format == GL_ALPHA)
      {
        pTri->c1.w = 1.0f;
        pTri->c2.w = 1.0f;
        pTri->c3.w = 1.0f;
      }
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////// FUCKING FUCK! FUCK LEGACY STATES! FUCK OPENGL!

  }

  // TODO: move this block to a function and opt it, make without dot product
  {
    const LiteMath::float4 one(1.0f,1.0f,1.0f,1.0f);
    const LiteMath::float4 diff1 = one - pTri->c1;
    const LiteMath::float4 diff2 = one - pTri->c2;
    const LiteMath::float4 diff3 = one - pTri->c3;
    
    const float diff1Dp = LiteMath::dot3f(diff1, diff1);
    const float diff2Dp = LiteMath::dot3f(diff2, diff2);
    const float diff3Dp = LiteMath::dot3f(diff3, diff3);

    const LiteMath::float4 diff4 = pTri->c1 - pTri->c2;
    const LiteMath::float4 diff5 = pTri->c1 - pTri->c3;

    const float diff4Dp = LiteMath::dot3f(diff4, diff4);
    const float diff5Dp = LiteMath::dot3f(diff5, diff5);

    if(diff1Dp < 1e-4f && diff2Dp < 1e-4f && diff3Dp < 1e-4f)
      pTri->flags |= (Triangle::TRI_VERT_COLOR_WHITE | Triangle::TRI_VERT_SAME_COLOR);
    else if (diff4Dp < 1e-4f && diff5Dp < 1e-4f)
      pTri->flags |= Triangle::TRI_VERT_SAME_COLOR;

    pTri->fillCol = FB::ColorPack<float,FBColorType>(pTri->c1[0], pTri->c1[1], pTri->c1[2], pTri->c1[3]);  
  }

  pTri->ropId = swglStateIdFromPSO(&pBatch->state, a_pContext, pTri->HasSameVertColor());
  pTri->bopId = BlendOp_None;
  pTri->fbId  = frameBufferId;

#ifdef PERSP_CORRECT

  if (a_perspCorrect)
  {
    pTri->c1 *= pTri->v1.z; // div by z, not mult!
    pTri->c2 *= pTri->v2.z; // div by z, not mult!
    pTri->c3 *= pTri->v3.z; // div by z, not mult!

    pTri->t1 *= pTri->v1.z; // div by z, not mult!
    pTri->t2 *= pTri->v2.z; // div by z, not mult!
    pTri->t3 *= pTri->v3.z; // div by z, not mult!
  }

#endif

}

constexpr float NEAR_CLIP_PLANE  = 0.9995f;                 // OpenGL Z axis looks to (0,0,-1); 0.9995f is essential for some unknown reason ...
constexpr float NEAR_CLIP_PLANE2 = NEAR_CLIP_PLANE*0.9995f; // the trick wich has unknown nature ...

template<typename ClipTriangleType>
static inline int clipTriangle(const ClipTriangleType &a_inTri, ClipTriangleType *outTris)
{
  float3 verts[3]  = {to_float3(a_inTri.v1), to_float3(a_inTri.v2), to_float3(a_inTri.v3)};
  float4 colors[3] = {a_inTri.c1, a_inTri.c2, a_inTri.c3};
  float2 texcrs[3] = {a_inTri.t1, a_inTri.t2, a_inTri.t3};

  // note that we have preserved order
  //
  float3 edgesP[3][2] = { {verts[0], verts[1]},
                          {verts[2], verts[0]},
                          {verts[1], verts[2]} };

  float4 edgesC[3][2] = {  {colors[0], colors[1]},
                           {colors[2], colors[0]},
                           {colors[1], colors[2]} };

  float2 edgesT[3][2] = {  {texcrs[0], texcrs[1]},
                           {texcrs[2], texcrs[0]},
                           {texcrs[1], texcrs[2]} };


  struct LocalVertex
  {
    float3 pos;
    float4 clr;
    float2 tex;
  };

  LocalVertex split[3];

  constexpr float splitPos = -NEAR_CLIP_PLANE;

  int top=0;
  for (int i=0;i<3;i++)
  {
    float v0p = edgesP[i][0].z;
    float v1p = edgesP[i][1].z;

    // Edge intersects the plane => insert intersection to both boxes.
    //
    if ((v0p < splitPos && v1p > splitPos) || (v0p > splitPos && v1p < splitPos))
    {
      const float t   = ::clamp((splitPos - v0p) / (v1p - v0p), 0.0f, 1.0f);
      float3 splitPos = lerp(edgesP[i][0], edgesP[i][1], t);
      float4 splitCol = lerp(edgesC[i][0], edgesC[i][1], t);
      float2 splitTex = lerp(edgesT[i][0], edgesT[i][1], t);

      split[top].pos = splitPos;
      split[top].clr = splitCol;
      split[top].tex = splitTex;
      top++;
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  const float3& A = verts[0];
  const float3& B = verts[1];
  const float3& C = verts[2];

  // we should preserve input order of vertices. So, sorting them is not allowed.
  //
  if(A.z >= -NEAR_CLIP_PLANE && B.z >= -NEAR_CLIP_PLANE) // OpenGL Z axis looks to (0,0,-1)
  {
    outTris[0].v1 = to_float4(split[0].pos, 1.0f);
    outTris[0].c1 = split[0].clr;
    outTris[0].t1 = split[0].tex;

    outTris[0].v2 = to_float4(split[1].pos, 1.0f);
    outTris[0].c2 = split[1].clr;
    outTris[0].t2 = split[1].tex;

    outTris[0].v3 = to_float4(verts[2], 1.0f);
    outTris[0].c3 = colors[2];
    outTris[0].t3 = texcrs[2];

    return 1;
  }
  else if (B.z >= -NEAR_CLIP_PLANE && C.z >= -NEAR_CLIP_PLANE)
  {
    outTris[0].v1 = to_float4(verts[0], 1.0f);
    outTris[0].c1 = colors[0];
    outTris[0].t1 = texcrs[0];

    outTris[0].v2 = to_float4(split[0].pos, 1.0f);
    outTris[0].c2 = split[0].clr;
    outTris[0].t2 = split[0].tex;

    outTris[0].v3 = to_float4(split[1].pos, 1.0f);
    outTris[0].c3 = split[1].clr;
    outTris[0].t3 = split[1].tex;

    return 1;
  }
  else if (C.z >= -NEAR_CLIP_PLANE && A.z >= -NEAR_CLIP_PLANE)
  {
    outTris[0].v1 = to_float4(split[0].pos, 1.0f);
    outTris[0].c1 = split[0].clr;
    outTris[0].t1 = split[0].tex;

    outTris[0].v2 = to_float4(verts[1], 1.0f);
    outTris[0].c2 = colors[1];
    outTris[0].t2 = texcrs[1];

    outTris[0].v3 = to_float4(split[1].pos, 1.0f);
    outTris[0].c3 = split[1].clr;
    outTris[0].t3 = split[1].tex;

    return 1;
  }
  else if (A.z >= -NEAR_CLIP_PLANE) // swap ?
  {
    outTris[0].v1 = to_float4(split[0].pos, 1.0f);
    outTris[0].c1 = split[0].clr;
    outTris[0].t1 = split[0].tex;

    outTris[0].v2 = to_float4(verts[1], 1.0f);
    outTris[0].c2 = colors[1];
    outTris[0].t2 = texcrs[1];

    outTris[0].v3 = to_float4(verts[2], 1.0f);
    outTris[0].c3 = colors[2];
    outTris[0].t3 = texcrs[2];

    outTris[1].v1 = to_float4(split[0].pos, 1.0f);
    outTris[1].c1 = split[0].clr;
    outTris[1].t1 = split[0].tex;

    outTris[1].v2 = to_float4(verts[2], 1.0f);
    outTris[1].c2 = colors[2];
    outTris[1].t2 = texcrs[2];

    outTris[1].v3 = to_float4(split[1].pos, 1.0f);
    outTris[1].c3 = split[1].clr;
    outTris[1].t3 = split[1].tex;

    return 2;
  }
  else if (B.z >= -NEAR_CLIP_PLANE) // swap ?
  {
    outTris[0].v1 = to_float4(verts[0], 1.0f);
    outTris[0].c1 = colors[0];
    outTris[0].t1 = texcrs[0];

    outTris[0].v2 = to_float4(split[1].pos, 1.0f);
    outTris[0].c2 = split[1].clr;
    outTris[0].t2 = split[1].tex;

    outTris[0].v3 = to_float4(verts[2], 1.0f);
    outTris[0].c3 = colors[2];
    outTris[0].t3 = texcrs[2];

    outTris[1].v1 = to_float4(verts[0], 1.0f);
    outTris[1].c1 = colors[0];
    outTris[1].t1 = texcrs[0];

    outTris[1].v2 = to_float4(split[0].pos, 1.0f);
    outTris[1].c2 = split[0].clr;
    outTris[1].t2 = split[0].tex;

    outTris[1].v3 = to_float4(split[1].pos, 1.0f);
    outTris[1].c3 = split[1].clr;
    outTris[1].t3 = split[1].tex;

    return 2;
  }
  else if (C.z >= -NEAR_CLIP_PLANE) // swap ?
  {
    outTris[0].v1 = to_float4(verts[0], 1.0f);
    outTris[0].c1 = colors[0];
    outTris[0].t1 = texcrs[0];

    outTris[0].v2 = to_float4(verts[1], 1.0f);
    outTris[0].c2 = colors[1];
    outTris[0].t2 = texcrs[1];

    outTris[0].v3 = to_float4(split[0].pos, 1.0f);
    outTris[0].c3 = split[0].clr;
    outTris[0].t3 = split[0].tex;

    outTris[1].v1 = to_float4(verts[1], 1.0f);
    outTris[1].c1 = colors[1];
    outTris[1].t1 = texcrs[1];

    outTris[1].v2 = to_float4(split[1].pos, 1.0f);
    outTris[1].c2 = split[1].clr;
    outTris[1].t2 = split[1].tex;

    outTris[1].v3 = to_float4(split[0].pos, 1.0f);
    outTris[1].c3 = split[0].clr;
    outTris[1].t3 = split[0].tex;

    return 2;
  }

  return 0;
}


static inline void swglProcessBatch(SWGL_Context* a_pContext) // pre (pContext != nullptr)
{
  if (a_pContext->input.getCurrBatch()->vertPos.size() == 0) // if curr batch is empty, no need to change it.
    return;

  Batch* pBatch            = a_pContext->input.getCurrBatch();
  SWGL_DrawList* pDrawList = &a_pContext->m_drawList;
  FrameBuffer fb           = swglBatchFb(a_pContext, pBatch->state);
  
  //const int  triNum        = int(pBatch->indices.size() / 3);
  //const int  freeSpace     = int(swglGetDrawListFreeSpace(pDrawList));
  //if (triNum >= freeSpace)
  //  glFlush();
  
  #ifdef MEASURE_NOLOAD_PERF
    return;
  #endif

  if (pBatch->indices.size() == 0)
    return;

  swglRunBatchVertexShader(a_pContext, pBatch);
  
  if(a_pContext->m_useTriQueue)
    swglEnqueueBatchTriangles(a_pContext, pBatch, fb);
  else
    swglDrawBatchTriangles(a_pContext, pBatch, fb);

  pBatch->clear(); // don't clear batch if you need it further!

}
