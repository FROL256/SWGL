#include "gl_sc.h"
#include "swgl.h"


#ifndef WIN32
  #include "glx_sc.h"
  extern struct __GLXcontextRec g_xcontext;
  #include "vfloat4_gcc.h"
#else
  #include "vfloat4_x64.h"
#endif // WIN32

#ifdef MEASURE_STATS
  #include "Timer.h"
#endif

#include <algorithm>
#include <future>
#include <thread>

SWGL_Context* g_pContext = nullptr;
SWGL_Timings _swglGetStats() { return g_pContext->m_timeStats; }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLAPI void APIENTRY glBegin(GLenum mode)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glBegin(" << mode << ")" << std::endl;

  if (g_pContext->input.getCurrBatch() == nullptr)
    return;

  g_pContext->input.currPrimType = mode;
  g_pContext->input.lastSizeVert = int(g_pContext->input.getCurrBatch()->vertPos.size());
  g_pContext->input.lastSizeInd  = int(g_pContext->input.getCurrBatch()->indices.size());
}


GLAPI void APIENTRY glEnd(void)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glEnd()" << std::endl;

  if (g_pContext->input.getCurrBatch() == nullptr)
    return;

  auto& input = g_pContext->input;

  swglAppendTriIndices(g_pContext, input.getCurrBatch(), input.currPrimType, input.lastSizeVert);

  input.getCurrBatch()->state = g_pContext->input.batchState; // ok
  swglProcessBatch(g_pContext);                               // run vertex shader and triangle setup immediately
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLAPI void APIENTRY glBlendFunc(GLenum sfactor, GLenum dfactor) //// ## CHECK_ONE_VALUE ## 100%; Support only one blend mode now (glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA), but probably add more modes in future.
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glBlendFunc(" << sfactor << ", " << dfactor << ")" << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLAPI void APIENTRY glBindTexture(GLenum target, GLuint texture)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glBindTexture(" << target << ", " << texture << ")" << std::endl;

  if (target == GL_TEXTURE_2D)
  {
    //GLuint oldTexture = g_pContext->input.batchState.slot_GL_TEXTURE_2D;
    g_pContext->input.batchState.slot_GL_TEXTURE_2D = texture;
  }

}

GLAPI void APIENTRY glClear(GLbitfield mask) // #TODO: clear tilef fb if used tiled
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glClear(" << mask << ")" << std::endl;

#ifdef MEASURE_STATS
  Timer timer(true);
#endif

   const auto& state = g_pContext->input;

  if((mask & GL_COLOR_BUFFER_BIT) != 0 && (mask & GL_DEPTH_BUFFER_BIT) != 0 )
    g_pContext->m_tiledFb2.ClearColorAndDepth(state.clearColor1u, 1.0f - state.clearDepth);
  else if (mask & GL_COLOR_BUFFER_BIT)
    g_pContext->m_tiledFb2.ClearColor(state.clearColor1u);
 
#ifdef MEASURE_STATS
  g_pContext->m_timeStats.msClear += timer.getElapsed()*1000.0f;
#endif
}

GLAPI void APIENTRY glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glClearColor(" << red << ", " << green << ", " << blue << ", " << alpha << ")" << std::endl;

  auto& state = g_pContext->input;

  state.clearColor4f = float4(red, green, blue, alpha);
  state.clearColor1u = color_pack_bgra(state.clearColor4f);
}

GLAPI void APIENTRY glClearDepthf(GLclampf a_depth)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glClearDepthf(" << a_depth << ")" << std::endl;

  g_pContext->input.clearDepth = a_depth;
}

GLAPI void APIENTRY glClearStencil(GLint s)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glClearStencil(" << s << ")" << std::endl;

  auto& state = g_pContext->input;
  state.clearStencil = s;
}

GLAPI void APIENTRY glClientActiveTexture(GLenum texture)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glClientActiveTexture(" << texture << ")" << std::endl;

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLAPI void APIENTRY glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glColor4f(" << red << ", " << green << ", " << blue << ", " << alpha << ")" << std::endl;

  g_pContext->input.currInputColor = float4(red, green, blue, alpha);
}

GLAPI void APIENTRY glColor4fv(const GLfloat *v)
{
  if (g_pContext == nullptr || v == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glColor4fv(" << v << ")" << std::endl;

  g_pContext->input.currInputColor = float4(v[0], v[1], v[2], v[3]);
}

GLAPI void APIENTRY glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glColor4ub(" << red << ", " << green << ", " << blue << ", " << alpha << ")" << std::endl;

  g_pContext->input.currInputColor = float4(red, green, blue, alpha) * (1.0f / 255.0f);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLAPI void APIENTRY glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glColorMask(" << red << ", " << green << ", " << blue << ", " << alpha << ")" << std::endl;

  // bool oldVal = g_pContext->input.batchState.colorWriteEnabled;

  if (red == GL_TRUE && green == GL_TRUE && blue == GL_TRUE && alpha == GL_TRUE)
    g_pContext->input.batchState.colorWriteEnabled = true;
  else if (red == GL_FALSE && green == GL_FALSE && blue == GL_FALSE && alpha == GL_FALSE)
    g_pContext->input.batchState.colorWriteEnabled = false;

  // bool newVal = g_pContext->input.batchState.colorWriteEnabled;
}

GLAPI void APIENTRY glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) // #TODO: add (type == GL_UNSIGNED_BYTE) support in future
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glColorPointer(" << size << ", " << type << ", " << stride << ", " << pointer << ")" << std::endl;

#ifdef STRICT_CLIENT_STATE_ARRAYS
  if (!g_pContext->input.vertexColorPtrEnabled)
  {
    if (g_pContext->logMode <= LOG_ALL)
      *(g_pContext->m_pLog) << "glColorPointer, WARNING: input.vertexColorPtrEnabled is disabled" << std::endl;
    return;
  }
#endif

  if (((type != GL_FLOAT) && (type != GL_UNSIGNED_BYTE)) || stride != 0) // not supported in SC profile
  {
    if (g_pContext->logMode <= LOG_ALL)
      *(g_pContext->m_pLog) << "glColorPointer, WARNING: type != [GL_FLOAT or GL_UNSIGNED_BYTE] or stride != 0" << std::endl;
    g_pContext->input.vertexColorPointer = nullptr;
    return;
  }

  g_pContext->input.vertexColorPointer = (float*)pointer;
  g_pContext->input.vertColorComponents = size;
}

GLAPI void APIENTRY glCullFace(GLenum mode)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glCullFace(" << mode << ")" << std::endl;

  // GLenum oldMode = g_pContext->input.batchState.cullFaceMode;
  g_pContext->input.batchState.cullFaceMode = mode;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLAPI void APIENTRY glDepthFunc(GLenum func) ///< CHECK_ONE_VALUE
{

}

GLAPI void APIENTRY glDepthMask(GLboolean flag) ///< CHECK_ONE_VALUE
{

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLAPI void APIENTRY glDisableClientState(GLenum a_array)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glDisableClientState(" << a_array << ")" << std::endl;

  switch (a_array)
  {
    case GL_VERTEX_ARRAY:
    g_pContext->input.vertexPosPtrEnabled    = false;
    break;

    case GL_NORMAL_ARRAY:
    g_pContext->input.vertexNormalPtrEnabled = false;
    break;

    case GL_COLOR_ARRAY:
    g_pContext->input.vertexColorPtrEnabled  = false;
    break;

    case GL_TEXTURE_COORD_ARRAY:
    g_pContext->input.vertexTexCoordPtrEnabled  = false;
    break;

    default:
    break;
  };

  if (g_pContext->logMode <= LOG_FOR_DEBUG_ERROR)
  {
    *(g_pContext->m_pLog) << "glDisableClientState() {" << std::endl;
    *(g_pContext->m_pLog) << "  vertexPosPtrEnabled      = " << g_pContext->input.vertexPosPtrEnabled << std::endl;
    *(g_pContext->m_pLog) << "  vertexNormalPtrEnabled   = " << g_pContext->input.vertexNormalPtrEnabled << std::endl;
    *(g_pContext->m_pLog) << "  vertexColorPtrEnabled    = " << g_pContext->input.vertexColorPtrEnabled << std::endl;
    *(g_pContext->m_pLog) << "  vertexTexCoordPtrEnabled = " << g_pContext->input.vertexTexCoordPtrEnabled << std::endl;
    *(g_pContext->m_pLog) << "glDisableClientState() }" << std::endl;
  }

}

GLAPI void APIENTRY glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glDrawArrays(" << mode << ", " << first << ", " << count << ")" << std::endl;

  if (g_pContext->input.getCurrBatch() == nullptr)
    return;

  auto& input     = g_pContext->input;
  auto* currBatch = input.getCurrBatch();

  int lastVertSize = (int)currBatch->vertPos.size();

  swglAppendVertices(g_pContext, mode, lastVertSize, first, count);
  swglAppendTriIndices(g_pContext, currBatch, mode, lastVertSize);

  currBatch->state = g_pContext->input.batchState; // ok
  swglProcessBatch(g_pContext);                    // run vertex shader and triangle setup immediately
}

GLAPI void APIENTRY glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices) // (?)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glDrawElements(" << mode << ", " << count << ", " << type << ", " << indices << ")" << std::endl;

  if (g_pContext->input.getCurrBatch() == nullptr)
    return;

  const int* inIndices   = (const int*)indices;

  auto& input            = g_pContext->input;
  auto* currBatch        = input.getCurrBatch();
  const int lastVertSize = currBatch->vertPos.size();
  
  if (g_pContext->logMode <= LOG_FOR_DEBUG_ERROR)
  {
    *(g_pContext->m_pLog) << "glDrawElements, before Append" << std::endl;
    *(g_pContext->m_pLog) << "lastVertSize = " << lastVertSize << std::endl;
    *(g_pContext->m_pLog) << "count        = " << count << std::endl;
  }

  const bool optInput = (g_pContext->input.vertexPosPointer      != nullptr) && (g_pContext->input.vertPosComponents      == 3) &&
                        (g_pContext->input.vertexTexCoordPointer != nullptr) && (g_pContext->input.vertTexCoordComponents == 2) && // g_pContext->input.vertexTexCoordPtrEnabled
                        ((g_pContext->input.vertexColorPointer == nullptr) || (g_pContext->input.vertexColorPtrEnabled == false)) &&
                        (mode == GL_TRIANGLES);

  if(g_pContext->m_useTriQueue && optInput) // append triangles to queue directly from input pointers
  {
    swglEnqueueTrianglesFromInput(g_pContext, inIndices, count, g_pContext->input);
  }
  else                                   // old way
  {
    const int maxVertexId = swglAppendTriIndices2(g_pContext, currBatch, mode, lastVertSize, inIndices, count);

    swglAppendVertices(g_pContext, mode, lastVertSize, 0,
                       maxVertexId + 1);                //#TODO: remove this, append directly to the triagle list

    if (g_pContext->logMode <= LOG_FOR_DEBUG_ERROR)
      *(g_pContext->m_pLog) << "glDrawElements, after Append (1) " << std::endl;

    currBatch->state = g_pContext->input.batchState; // ok
    swglProcessBatch(g_pContext);                    // run vertex shader and triangle setup immediately
  }
}

GLAPI void APIENTRY glEnable(GLenum cap)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glEnable(" << cap << ")" << std::endl;

  //bool oldValue = false;

  if (cap == GL_DEPTH_TEST)
  {
    //oldValue = g_pContext->input.batchState.depthTestEnabled;
    g_pContext->input.batchState.depthTestEnabled = true;
  }

  else if (cap == GL_STENCIL_TEST)
  {
    //oldValue = g_pContext->input.batchState.stencilTestEnabled;
    g_pContext->input.batchState.stencilTestEnabled = true;
  }

  else if (cap == GL_TEXTURE_2D)
  {
    //oldValue = g_pContext->input.batchState.texure2DEnabled;
    g_pContext->input.batchState.texure2DEnabled = true;
  }

  else if (cap == GL_BLEND)
  {
    //oldValue = g_pContext->input.batchState.alphaBlendEnabled;
    g_pContext->input.batchState.alphaBlendEnabled = true;
  }

  else if (cap == GL_CULL_FACE)
  {
    //oldValue = g_pContext->input.batchState.cullFaceEnabled;
    g_pContext->input.batchState.cullFaceEnabled = true;
  }

}


GLAPI void APIENTRY glDisable(GLenum cap)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glDisable(" << cap << ")" << std::endl;

  //bool oldValue = true;

  if (cap == GL_DEPTH_TEST)
  {
    //oldValue = g_pContext->input.batchState.depthTestEnabled;
    g_pContext->input.batchState.depthTestEnabled = false;
  }

  else if (cap == GL_STENCIL_TEST)
  {
    //oldValue = g_pContext->input.batchState.stencilTestEnabled;
    g_pContext->input.batchState.stencilTestEnabled = false;
  }

  else if (cap == GL_TEXTURE_2D)
  {
    //oldValue = g_pContext->input.batchState.texure2DEnabled;
    g_pContext->input.batchState.texure2DEnabled = false;
  }

  else if (cap == GL_BLEND)
  {
    //oldValue = g_pContext->input.batchState.alphaBlendEnabled;
    g_pContext->input.batchState.alphaBlendEnabled = false;
  }

  else if (cap == GL_CULL_FACE)
  {
    //oldValue = g_pContext->input.batchState.cullFaceEnabled;
    g_pContext->input.batchState.cullFaceEnabled = false;
  }

}


GLAPI void APIENTRY glEnableClientState(GLenum a_array)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glEnableClientState(" << a_array << ")" << std::endl;

  switch (a_array)
  {
    case GL_VERTEX_ARRAY:
    g_pContext->input.vertexPosPtrEnabled    = true;
    break;

    case GL_NORMAL_ARRAY:
    g_pContext->input.vertexNormalPtrEnabled = true;
    break;

    case GL_COLOR_ARRAY:
    g_pContext->input.vertexColorPtrEnabled  = true;
    break;

    case GL_TEXTURE_COORD_ARRAY:
    g_pContext->input.vertexTexCoordPtrEnabled  = true;
    break;

    default:
    break;
  };

  if (g_pContext->logMode <= LOG_FOR_DEBUG_ERROR)
  {
    *(g_pContext->m_pLog) << "glEnableClientState() {" << std::endl;
    *(g_pContext->m_pLog) << "  vertexPosPtrEnabled      = " << g_pContext->input.vertexPosPtrEnabled << std::endl;
    *(g_pContext->m_pLog) << "  vertexNormalPtrEnabled   = " << g_pContext->input.vertexNormalPtrEnabled << std::endl;
    *(g_pContext->m_pLog) << "  vertexColorPtrEnabled    = " << g_pContext->input.vertexColorPtrEnabled << std::endl;
    *(g_pContext->m_pLog) << "  vertexTexCoordPtrEnabled = " << g_pContext->input.vertexTexCoordPtrEnabled << std::endl;
    *(g_pContext->m_pLog) << "glEnableClientState() }" << std::endl;
  }

}

GLAPI void APIENTRY glFinish(void)
{
  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glFinish()" << std::endl;

  glFlush();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::thread          g_threads[NUM_THREADS_AUX];
int                  g_active [NUM_THREADS_AUX]; //#TODO: use std::atomics

bool          g_initialized_rast = false;
bool          g_kill_all         = false;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline void DrawTriangle(Triangle& localTri)
{
  FrameBuffer& frameBuff = g_pContext->batchFrameBuffers[localTri.fbId];
  clampTriBBox(&localTri, frameBuff);  // need this to prevent out of border, can be done in separate thread
  HWImpl::RasterizeTriangle(localTri, &frameBuff);
}


int SWGL_TriangleRenderThread(int a_threadId)
{
  if (g_pContext == nullptr)
    return 0;

  constexpr int BULKSIZE = 4;
  Triangle localTris[BULKSIZE];

  while(!g_kill_all)
  {
    const int fetchSize = g_pContext->m_tqueue.size_approx() > 4*BULKSIZE ? BULKSIZE : 1; // fetch single triangle if we don't have enough triangles
    const auto fetched  = g_pContext->m_tqueue.try_dequeue_bulk(localTris, fetchSize);
    if(fetched > 0)
    {
      g_active[a_threadId] = 1;
      for(int i=0;i<fetched;i++)
        DrawTriangle(localTris[i]);
    }
    else
    {
      g_active[a_threadId] = 0;
      //std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLAPI void APIENTRY glFlush(void)
{

#ifdef MEASURE_NOLOAD_PERF
  return;
#endif


  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glFlush()" << std::endl;

  auto* pDrawList = &g_pContext->m_drawList;

  if(g_pContext->m_useTriQueue)
  {
   #ifdef MEASURE_STATS
     Timer timer(true);
   #endif

    if(!g_initialized_rast)
    {
      for(int i=0;i<NUM_THREADS_AUX;i++)
      {
        g_active [i] = 0;
        g_threads[i] = std::thread(&SWGL_TriangleRenderThread, i);
      }
      g_initialized_rast = true;
    }

    // flush triangle queue
    //
    Triangle localTri;
    while(g_pContext->m_tqueue.try_dequeue(localTri))
      DrawTriangle(localTri);

    while (true) // waiting for all threads to finish
    {
      bool allFinished = true;
      for (int i = 0; i < NUM_THREADS_AUX; i++)
        allFinished = allFinished && (g_active[i] == 0);

      if (!allFinished)
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
      else
        break;
    }

   #ifdef MEASURE_STATS
   g_pContext->m_timeStats.msRasterAndPixelShader += timer.getElapsed()*1000.0f;
   #endif
  }
  else
  {
    // memset(g_pContext->m_pixels2, 0xFFFFFFFF, frameBuff.w*frameBuff.h*sizeof(int));
  }

  g_pContext->batchFrameBuffers.clear();
}

GLAPI void APIENTRY glGenTextures(GLsizei n, GLuint *textures)
{
  if (g_pContext == nullptr || textures == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glGenTextures(" << n << ", " << textures << ")" << std::endl;

  if (g_pContext->m_texTop + (int)n > (int)g_pContext->m_textures.size())
  {
    for (int i = 0; i < n; i++)
      textures[i] = -1;
    return;
  }

  for (int i = 0; i < n; i++)
  {
    SWGL_TextureStorage storageTemp;
    storageTemp.w = 0;
    storageTemp.h = 0;

    g_pContext->m_textures[g_pContext->m_texTop] = storageTemp;
    g_pContext->m_texTop++;
    textures[i] = g_pContext->m_texTop - 1;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLAPI GLenum APIENTRY glGetError(void)
{
  if (g_pContext == nullptr)
    return 0;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glGetError()" << std::endl;

  return 0;
}

GLAPI void APIENTRY glGetFloatv(GLenum pname, GLfloat *params)
{
  if (g_pContext == nullptr || params == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glGetFloatv(" << pname << ", " << params << ")" << std::endl;


  if (pname == GL_MODELVIEW_MATRIX)
  {
    float4x4 m1 = g_pContext->input.batchState.worldViewMatrix;

    float4x4 m2;
    for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 4; j++)
        m2(i, j) = m1(j, i);
    }

    memcpy(params, &m2, 16 * sizeof(float));
  }
  else if (pname == GL_VIEWPORT)
  {
    params[0] = float(g_pContext->input.batchState.viewport[0]);
    params[1] = float(g_pContext->input.batchState.viewport[1]);
    params[2] = float(g_pContext->input.batchState.viewport[2]);
    params[3] = float(g_pContext->input.batchState.viewport[3]);
  }

}

GLAPI void APIENTRY glGetIntegerv(GLenum pname, GLint *params)
{
  if (g_pContext == nullptr || params == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glGetIntegerv(" << pname << ", " << params << ")" << std::endl;

  if (pname == GL_MATRIX_MODE)
    (*params) = g_pContext->input.inputMatrixMode;
}

GLAPI void APIENTRY glGetBooleanv(GLenum pname, GLboolean *params)
{

}

GLAPI void APIENTRY glGetTexEnviv(GLenum target, GLenum pname, GLint *params)
{

}

GLAPI void APIENTRY glGetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{

}

GLAPI const GLubyte * APIENTRY glGetString(GLenum name)
{
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLAPI void APIENTRY glHint(GLenum target, GLenum mode)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glHint(" << target << ", " << mode << ")" << std::endl;
}

GLAPI GLboolean APIENTRY glIsEnabled(GLenum cap)
{
  if (g_pContext == nullptr)
    return 0;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glIsEnabled()" << std::endl;

  return 0;
}


GLAPI void APIENTRY glLineWidth(GLfloat width)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glLineWidth(" << width << ")" << std::endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// matrix
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// matrix
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// matrix
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// matrix

GLAPI void APIENTRY glMatrixMode(GLenum mode)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glMatrixMode(" << mode << ")" << std::endl;

  if (mode == GL_MODELVIEW)
    g_pContext->input.inputMatrixMode = GL_MODELVIEW;
  else if (mode == GL_PROJECTION)
    g_pContext->input.inputMatrixMode = GL_PROJECTION;
}


GLAPI void APIENTRY glLoadIdentity(void) // pre (g_pContext->state.inputMatrixMode == GL_MODELVIEW) || (g_pContext->state.inputMatrixMode == GL_PROJECTION)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glLoadIdentity()" << std::endl;

  float4x4* pmatrix = swglGetCurrMatrix(g_pContext);

  pmatrix->identity();
}

GLAPI void APIENTRY glLoadMatrixf(const GLfloat *m) // pre (g_pContext->state.inputMatrixMode == GL_MODELVIEW) || (g_pContext->state.inputMatrixMode == GL_PROJECTION)
{                                                   // pre sizeof(m as array) >= 16*sizeof(float)
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glLoadMatrixf(" << m << ")" << std::endl;

  float4x4* pmatrix = swglGetCurrMatrix(g_pContext);
  memcpy(pmatrix, m, 16*sizeof(float)); // opengl use transpose matrix layout
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// matrix
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// matrix
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// matrix
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// matrix


GLAPI void APIENTRY glPointSize(GLfloat size)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glPointSize(" << size << ")" << std::endl;

  g_pContext->input.currPointSize = size;
}


GLAPI void APIENTRY glReadPixels(GLint a_x, GLint a_y, GLsizei a_width, GLsizei a_height, GLenum format, GLenum type, GLvoid *pixels)
{
  if (g_pContext == nullptr || pixels == nullptr) // don't support fucking GL_PIXEL_PACK_BUFFER/GL_PIXEL_UNPACK_BUFFER slots
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glReadPixels()" << std::endl;

  if (a_width <= 0 || a_height <= 0)
    return;

  if (a_x < 0 || a_x > g_pContext->m_width || a_y < 0 || a_y > g_pContext->m_height)
    return;


  if (format == GL_RGBA && type == GL_UNSIGNED_BYTE)
  {
    int* outPixels = (int*)pixels;

  
  }
  else
  {
    if (g_pContext->logMode <= LOG_ALL)
      *(g_pContext->m_pLog) << "glReadPixels(), bad 'format' or 'type'" << std::endl;
  }

}


GLAPI void APIENTRY glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glScissor(" << x << ", " << y << ", " << width << ", " << height << ")" << std::endl;
}


GLAPI void APIENTRY glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glStencilFunc(" << func << ", " << ref << ", " << mask  << ")" << std::endl;

  g_pContext->input.batchState.stencilMask  = mask;
  g_pContext->input.batchState.stencilValue = ref;

  // bool oldValue = g_pContext->input.batchState.stencilWriteEnabled;

  if (func == GL_ALWAYS)
    g_pContext->input.batchState.stencilWriteEnabled = true;
  else
    g_pContext->input.batchState.stencilWriteEnabled = false;

}

GLAPI void APIENTRY glStencilMask(GLuint mask)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glStencilFunc(" << mask << ")" << std::endl;

  //bool oldValue = g_pContext->input.batchState.stencilMask;
  g_pContext->input.batchState.stencilMask = mask;

}

GLAPI void APIENTRY glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glStencilFunc(" << fail << ", " << zfail << ", " << zpass << ")" << std::endl;


  //bool oldVal = g_pContext->input.batchState.stencilWriteEnabled;

  if (fail == GL_REPLACE && zfail == GL_REPLACE && zpass == GL_REPLACE)
    g_pContext->input.batchState.stencilWriteEnabled = true;
  else if (fail == GL_KEEP && zfail == GL_KEEP && zpass == GL_KEEP)
    g_pContext->input.batchState.stencilWriteEnabled = false;

  //bool newVal = g_pContext->input.batchState.stencilWriteEnabled;

}

GLAPI void APIENTRY glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glTexCoordPointer(" << size << ", " << type << ", " << stride << ", " << pointer << ")" << std::endl;

#ifdef STRICT_CLIENT_STATE_ARRAYS
  if (!g_pContext->input.vertexTexCoordPtrEnabled)
  {
    if (g_pContext->logMode <= LOG_ALL)
      *(g_pContext->m_pLog) << "glTexCoordPointer, WARNING: input.vertexTexCoordPtrEnabled is disabled" << std::endl;
    return;
  }
#endif

  if (type != GL_FLOAT || stride != 0) // not supported in SC profile
  {
    if (g_pContext->logMode <= LOG_ALL)
      *(g_pContext->m_pLog) << "glTexCoordPointer, WARNING: (type != GL_FLOAT or stride != 0)" << std::endl;
    g_pContext->input.vertexTexCoordPointer = nullptr;
    return;
  }

  g_pContext->input.vertexTexCoordPointer  = (float*)pointer;
  g_pContext->input.vertTexCoordComponents = size;

}


GLAPI void APIENTRY glTexEnvi(GLenum target, GLenum pname, GLint param)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glTexEnvi(" << target << ", " << pname << ", " << param << ")" << std::endl;

  GLuint slotId = g_pContext->input.batchState.slot_GL_TEXTURE_2D;

  if (slotId >= (GLuint)g_pContext->m_texTop)
    return;

  auto& tex = g_pContext->m_textures[slotId];

  if (param == GL_REPLACE)
    tex.modulateMode = GL_REPLACE;
  else
    tex.modulateMode = GL_MODULATE;

}

GLAPI void APIENTRY glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glTexImage2D(" << target << ", " << internalformat << ", " << width << ", " << height << ")" << std::endl;


  if (internalformat != GL_RGBA && format != GL_ALPHA) // other formats unsupported
    return;

  if (internalformat != (GLint)format) // conversion is unsupported
    return;

  if (target != GL_TEXTURE_2D) // other slots unsupported
    return;

  int slotId = g_pContext->input.batchState.slot_GL_TEXTURE_2D;

  if (slotId >= g_pContext->m_texTop) // invalid texid wasd bound to GL_TEXTURE_2D
    return;

  g_pContext->m_textures[slotId].w = width;
  g_pContext->m_textures[slotId].h = height;
  g_pContext->m_textures[slotId].data.resize(width*height);
  g_pContext->m_textures[slotId].format = format;

  if (internalformat == GL_RGBA)
  {
    memcpy(&g_pContext->m_textures[slotId].data[0], pixels, width*height*sizeof(int));
  }
  else
  {
    const uint8_t* inData = (const uint8_t*)pixels;

    for (int i = 0; i < (width*height); i++)
      g_pContext->m_textures[slotId].data[i] = ( 0x00FFFFFF | (inData[i] << 24) );
  }

  g_pContext->m_textures[slotId].MakeBoundaryBillet();

}

GLAPI void APIENTRY glTexParameteri(GLenum target, GLenum pname, GLint param)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glTexParameteri(" << target << ", " << pname << ", " << param << ")" << std::endl;
}

GLAPI void APIENTRY glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glVertexPointer(" << size << ", " << type << ", " << stride << ", " << pointer << ")" << std::endl;

  if (!g_pContext->input.vertexPosPtrEnabled)
    return;

  if (type != GL_FLOAT || stride != 0) // not supported in SC profile
  {
    g_pContext->input.vertexPosPointer = nullptr;
    return;
  }

  g_pContext->input.vertexPosPointer  = (float*)pointer;
  g_pContext->input.vertPosComponents = size;

}

extern int g_fbColorDepth;
#include <iostream>

GLAPI void APIENTRY glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
  if (g_pContext == nullptr)
    return;


  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glViewport(" << x << ", " << y << ", " << width << ", " << height << ")" << std::endl;

  glFlush();


  #ifdef WIN32

  if (g_pContext->m_width != width && g_pContext->m_height != height)
  {
    HDC currHDC = g_pContext->m_hdc;
    g_pContext->Destroy();
    g_pContext->Create(currHDC, width, height);
  }

  #else // linux path
  
  if (g_pContext->m_width != width && g_pContext->m_height != height)
  {
    g_pContext->Destroy();
    g_pContext->Create(g_pContext->glxrec.dpy, nullptr, width, height);
  }
  
  #endif

  // if (g_pContext->m_width == 0 || g_pContext->m_height == 0) // first time, allocate memory
  // {
  //   HDC currHDC = g_pContext->m_hdc;
  //   g_pContext->Destroy();
  //   g_pContext->Create(currHDC, width, height);
  // }

  // ajust viewport
  //
  g_pContext->input.batchState.viewport[0] = x;
  g_pContext->input.batchState.viewport[1] = y;
  g_pContext->input.batchState.viewport[2] = width;
  g_pContext->input.batchState.viewport[3] = height;

}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline void swglAppendVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->input.getCurrBatch() == nullptr)
    return;

  auto* pCurr = g_pContext->input.getCurrBatch();

  pCurr->vertPos.push_back(float4(x, y, z, w));
  pCurr->vertColor.push_back(g_pContext->input.currInputColor);
  //pCurr->vertNorm.push_back(g_pContext->input.currInputNormal);
  pCurr->vertTexCoord.push_back(g_pContext->input.currInputTexCoord[0]);
}

GLAPI void APIENTRY glVertex2f(GLfloat x, GLfloat y)
{
  if (g_pContext == nullptr)
    return;

  swglAppendVertex4f(x, y, 0.0f, 1.0f);
}

GLAPI void APIENTRY glVertex2fv(const GLfloat *v)
{
  if (g_pContext == nullptr || v == nullptr)
    return;

  swglAppendVertex4f(v[0], v[1], 0.0f, 1.0f);
}

GLAPI void APIENTRY glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
  swglAppendVertex4f(x, y, z, 1.0f);
}

GLAPI void APIENTRY glVertex3fv(const GLfloat *v)
{
  if (g_pContext == nullptr || v == nullptr)
    return;

  swglAppendVertex4f(v[0], v[1], v[2], 1.0f);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLAPI void APIENTRY glPopMatrix(void)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glPopMatrix()" << std::endl;

  if (g_pContext->input.inputMatrixMode == GL_MODELVIEW && (g_pContext->input.top_mw > 0))
  {
    g_pContext->input.top_mw--;
    g_pContext->input.batchState.worldViewMatrix = g_pContext->input.mmwstack[g_pContext->input.top_mw];
  }
  else if (g_pContext->input.inputMatrixMode == GL_PROJECTION && (g_pContext->input.top_mp > 0))
  {
    g_pContext->input.top_mp--;
    g_pContext->input.batchState.projMatrix = g_pContext->input.mmpstack[g_pContext->input.top_mp];
  }

}

GLAPI void APIENTRY glPushMatrix(void)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glPushMatrix()" << std::endl;

  if (g_pContext->input.inputMatrixMode == GL_MODELVIEW && (g_pContext->input.top_mw < MMWSTACKSIZE - 1))
  {
    g_pContext->input.mmwstack[g_pContext->input.top_mw] = g_pContext->input.batchState.worldViewMatrix;
    g_pContext->input.top_mw++;
  }
  else if (g_pContext->input.inputMatrixMode == GL_PROJECTION && (g_pContext->input.top_mp < MMPSTACKSIZE - 1))
  {
    g_pContext->input.mmpstack[g_pContext->input.top_mp] = g_pContext->input.batchState.projMatrix;
    g_pContext->input.top_mp++;
  }

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


GLAPI void APIENTRY glMultMatrixf(const GLfloat *m)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glMultMatrixf(" << m << ")" << std::endl;

  float4x4* pmatrix = swglGetCurrMatrix(g_pContext);

  float4x4 newMat;

  // opengl use transpose matrix layout
  //
  newMat.col(0) = float4(m[0], m[1], m[2], m[3]);
  newMat.col(1) = float4(m[4], m[5], m[6], m[7]);
  newMat.col(2) = float4(m[8], m[9], m[10],m[11]);
  newMat.col(3) = float4(m[12],m[13],m[14],m[15]);

  (*pmatrix) = (*pmatrix)*newMat;

}

GLAPI void APIENTRY glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glRotatef(" << angle << ", " << x << "," << y << ", " << z << ")" << std::endl;


  float4x4* pmatrix = swglGetCurrMatrix(g_pContext);

  const float3 v = normalize(float3(x, y, z));

  const float c = cos(DEG_TO_RAD*angle);
  const float s = sin(DEG_TO_RAD*angle);

  float4x4 newMat;

  newMat(0,0) = v.x*v.x*(1.0f - c) + c;     newMat(0,1) = v.x*v.y*(1.0f - c) - v.z*s;  newMat(0,2) = v.x*v.z*(1.0f - c) + v.y*s; newMat(0,3) = 0.0f;
  newMat(1,0) = v.y*v.x*(1.0f - c) + v.z*s; newMat(1,1) = v.y*v.y*(1.0f - c) + c;      newMat(1,2) = v.y*v.z*(1.0f - c) - v.x*s; newMat(1,3) = 0.0f;
  newMat(2,0) = v.x*v.z*(1.0f - c) - v.y*s; newMat(2,1) = v.y*v.z*(1.0f - c) + v.x*s;  newMat(2,2) = v.z*v.z*(1.0f - c) + c;     newMat(2,3) = 0.0f;
  newMat(3,0) = 0.0f;                       newMat(3,1) = 0.0f;                        newMat(3,2) = 0.0f;                       newMat(3,3) = 1.0f;

  (*pmatrix) = (*pmatrix)*newMat;
}

GLAPI void APIENTRY glScalef(GLfloat x, GLfloat y, GLfloat z)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glScalef(" << x << "," << y << ", " << z << ")" << std::endl;

  float4x4* pmatrix = swglGetCurrMatrix(g_pContext);
  float4x4  newMat  = scale4x4(float3(x,y,z));

  (*pmatrix) = (*pmatrix)*newMat;
}

GLAPI void APIENTRY glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glTranslatef(" << x << "," << y << ", " << z << ")" << std::endl;

  float4x4* pmatrix = swglGetCurrMatrix(g_pContext);
  float4x4  newMat  = translate4x4(float3(x,y,z));

  (*pmatrix) = (*pmatrix)*newMat;
}

