#pragma once

#include "gl_sc.h"

#ifdef WIN32
  #include <windows.h>
#else
  #include "glx_sc.h"
#endif

#include "SWGL_TiledFrameBuffer.h"

#include "LiteMath.h"
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

#include "TriRaster.h"
#include "HWAbstractionLayer.h"

#include "concurrentqueue.h"

#define PI  ((float)3.1415926535)
#define DEG_TO_RAD (PI/(float)180.0)
#define RAD_TO_DEG ((float)180.0/PI)

#define MAX_NUM_VERTICES_IN_BATCH 65536*4                   // just a reccomendation, not a strict reqirenment // 262144
#define MAX_NUM_TRIANGLES_TOTAL (MAX_NUM_VERTICES_IN_BATCH) // strict rule ???

const int MMPSTACKSIZE = 4;  ///< Projection Matrix stack size
const int MMWSTACKSIZE = 32; ///< WolrdView  Matrix stack size

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
  Batch()
  {
    vertPos.reserve(MAX_NUM_VERTICES_IN_BATCH);
    //vertNorm.reserve(vertPos.size());
    vertColor.reserve(vertPos.size());
    vertTexCoord.reserve(vertPos.size());
    indices.reserve(vertPos.size()*3);
    indicesLines.reserve(vertPos.size()*2);
    indicesPoints.reserve(vertPos.size());
    pointSize.reserve(indicesPoints.capacity()/2);
  }

  void clear()
  {
    pointSize.clear();
    indicesPoints.clear();
    indicesLines.clear();
    indices.clear();
    vertPos.clear();
    //vertNorm.clear();
    vertColor.clear();
    vertTexCoord.clear();
  }

  std::vector<float>  pointSize;
  std::vector<int>    indicesPoints;
  std::vector<int>    indicesLines;
  std::vector<int>    indices;

  std::vector<float4, aligned<float4, 16> > vertPos;
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

  SWGL_Context() : hbmp(NULL), hdcMem(NULL), hbmOld(NULL), m_width(0), m_height(0), m_zbuffer(0), m_sbuffer(0),
                   m_locks(nullptr), m_tqueue(MAX_NUM_TRIANGLES_TOTAL), m_useTriQueue(false), m_useTiledFB(false)
  {
    InitCommon();
  }

  void Create(HDC a_hdc, int width, int height);

  HBITMAP hbmOld;
  HBITMAP hbmp;
  HDC     hdcMem;
  HDC     m_hdc;

#else
  
  SWGL_Context() : m_sbuffer(0),m_zbuffer(0), m_pixels(0), m_pixels2(0), m_width(0), m_height(0), m_fwidth(0.0f), m_fheight(0.0f),
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
  
  uint8_t* m_sbuffer;
  float*   m_zbuffer;
  int*     m_pixels;
  int*     m_pixels2;
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

  frameBuff.cbuffer    = a_pContext->m_pixels2;
  frameBuff.zbuffer    = a_pContext->m_zbuffer;
  frameBuff.sbuffer    = a_pContext->m_sbuffer;
  frameBuff.w          = a_pContext->m_width;
  frameBuff.h          = a_pContext->m_height;
  frameBuff.pitch      = frameBuff.w + FB_BILLET_SIZE;

  frameBuff.vx = a_state.viewport[0];
  frameBuff.vy = a_state.viewport[1];
  frameBuff.vw = a_state.viewport[2];
  frameBuff.vh = a_state.viewport[3];

  if (!a_state.depthTestEnabled)
    frameBuff.zbuffer = nullptr;

  if (!a_state.stencilTestEnabled)
    frameBuff.sbuffer = nullptr;

  frameBuff.m_pImpl = &a_pContext->m_tiledFb2;

  return frameBuff;
}


static inline float4 swglVertexShaderTransform2D(Batch* pBatch, float4 a_pos) // pre (pBatch != nullptr)
{
  return pBatch->state.worldViewMatrix*a_pos;
}


static inline float4 swglVertexShaderTransform(Batch* pBatch, float4 a_pos) // pre (pBatch != nullptr)
{
  //const float4 viewPos   = mul(pBatch->state.worldViewMatrix, a_pos);
  //const float4 clipSpace = mul(pBatch->state.projMatrix, viewPos);

  const float4 clipSpace   = pBatch->state.worldViewProjMatrix*a_pos;

  const float invW = (1.0f / fmax(clipSpace.w, 1e-20f));

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
