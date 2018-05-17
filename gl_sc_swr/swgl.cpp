#include "swgl.h"
#include "RasterAlgorithms.h"
#include "HWAbstractionLayer.h"

#ifdef FULL_GL
  #include "gl_std.h"
#endif

#ifdef MEASURE_STATS
  #include "Timer.h"
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// x x x x x
// x d d d x
// x d d d x
// x d d d x
// x x x x x

void SWGL_TextureStorage::MakeBoundaryBillet()
{
  const int boundarySize = 4;
  const int newW         = w + 2 * boundarySize;
  const int newH         = h + 2 * boundarySize;

  std::vector<int> dataBillet(newW*newH);
  for (size_t i = 0; i < dataBillet.size(); i++)
    dataBillet[i] = 0;

  pitch   = newW;
  texdata = &dataBillet[0] + boundarySize*pitch + boundarySize;

  for (int y = 0; y < h; y++)
  {
    const int offset1 = y*pitch;
    const int offset2 = y*w;

    for (int x = 0; x < w; x++)
      texdata[offset1 + x] = data[offset2 + x];
  }

  // fill boundaries, not fast but simple
  //

  for (int j = 0; j < newH; j++)
  {
    for (int i = 0; i < newW; i++)
    {
      int x = i - boundarySize;
      int y = j - boundarySize;

      if (x >= 0 && y >= 0 && x < w && y < h)
        continue;

      int x2 = x;
      int y2 = y;

      if (x2 < 0) x2 = 0;
      if (y2 < 0) y2 = 0;

      if (x2 >= w) x2 = w - 1;
      if (y2 >= h) y2 = h - 1;

      dataBillet[j*pitch + i] = texdata[y2*pitch+x2];
    }
  }

  pitch   = newW;
  data    = dataBillet;
  texdata = &data[0] + boundarySize*pitch + boundarySize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::ofstream* SWGL_Context::m_pLog = nullptr;

void SWGL_Context::InitCommon()
{
  if (m_pLog == nullptr)
    m_pLog = new std::ofstream("zgl_log.txt");

  freopen("stdout.txt", "wt", stdout);
  freopen("stderr.txt", "wt", stderr);

  m_currFrame    = 0;
  m_lastNFramesT = clock();
  m_texTop       = 0;
  m_textures.resize(1024); // max 1024 tex

  m_useTiledFB = true;
  if(m_useTiledFB)
    swglClearDrawListAndTiles(&m_drawList, &m_tiledFrameBuffer, MAX_NUM_TRIANGLES_TOTAL);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void swglCheckInputPointers(SWGL_Context* a_pContext)  // pre (a_pContext != nullptr)
{
  if (!a_pContext->input.vertexColorPtrEnabled)
    a_pContext->input.vertexColorPointer = nullptr;

  if (!a_pContext->input.vertexNormalPointer)
    a_pContext->input.vertexNormalPointer = nullptr;

  if (!a_pContext->input.vertTexCoordComponents)
    a_pContext->input.vertexTexCoordPointer = nullptr;
}


void swglAppendVertices(SWGL_Context* a_pContext, GLenum currPrimType, size_t lastSizeVert, int first, int count) // pre (a_pContext != nullptr)
{
  auto& input     = a_pContext->input;
  auto* currBatch = input.getCurrBatch();

  if (!input.vertexPosPtrEnabled || input.vertexPosPointer == nullptr)
    return;

#ifndef STRICT_CLIENT_STATE_ARRAYS
  swglCheckInputPointers(a_pContext);
#endif

  // int lastVertSize = (int)currBatch->vertPos.size();

  if (a_pContext->logMode <= LOG_FOR_DEBUG_ERROR)
  {
    *(a_pContext->m_pLog) << "swglAppendVertices: (first,count) = (" << first << "," << count << ")" << std::endl;
    *(a_pContext->m_pLog) << "vertexPosPointer      = " << input.vertexPosPointer << std::endl;
    *(a_pContext->m_pLog) << "vertexColorPointer    = " << input.vertexColorPointer << std::endl;
    *(a_pContext->m_pLog) << "vertexNormalPointer   = " << input.vertexNormalPointer << std::endl;
    *(a_pContext->m_pLog) << "vertexTexCoordPointer = " << input.vertexTexCoordPointer << std::endl;
    *(a_pContext->m_pLog) << "vertPosComponents     = " << input.vertPosComponents << std::endl;
  }

  for (int i = first; i < count; i++)
  {
    if (a_pContext->logMode <= LOG_FOR_DEBUG_ERROR)
      *(a_pContext->m_pLog) << "appending vertex " << i << std::endl;

    float4 vPos(0.0f, 0.0f, 0.0f, 1.0f);

    vPos.x = input.vertexPosPointer[i*input.vertPosComponents + 0];
    vPos.y = input.vertexPosPointer[i*input.vertPosComponents + 1];

    if (input.vertPosComponents >= 3)
      vPos.z = input.vertexPosPointer[i*input.vertPosComponents + 2];
    if (input.vertPosComponents >= 4)
      vPos.w = input.vertexPosPointer[i*input.vertPosComponents + 3];


    //if (a_pContext->logMode <= LOG_FOR_DEBUG_ERROR) a_pContext->m_log << "appending vertex  (1)" << i << std::endl;

    float4 vColor = a_pContext->input.currInputColor;

    if (input.vertexColorPointer != nullptr && input.vertexColorPtrEnabled)
    {
      vColor.x = input.vertexColorPointer[i*input.vertColorComponents + 0];
      vColor.y = input.vertexColorPointer[i*input.vertColorComponents + 1];
      vColor.z = input.vertexColorPointer[i*input.vertColorComponents + 2];

      if (input.vertColorComponents == 4)
        vColor.w = input.vertexColorPointer[i*input.vertColorComponents + 3];
    }

    //if (a_pContext->logMode <= LOG_FOR_DEBUG_ERROR) a_pContext->m_log << "appending vertex  (2)" << i << std::endl;

    float4 vNorm = a_pContext->input.currInputNormal;

    if (input.vertexNormalPointer != nullptr && input.vertexNormalPtrEnabled)
    {
      vNorm.x = input.vertexNormalPointer[i*input.vertNormalComponents + 0];
      vNorm.y = input.vertexNormalPointer[i*input.vertNormalComponents + 1];
      vNorm.z = input.vertexNormalPointer[i*input.vertNormalComponents + 2];
    }

    //if (a_pContext->logMode <= LOG_FOR_DEBUG_ERROR) a_pContext->m_log << "appending vertex  (3)" << i << std::endl;

    float2 vTexCoord = a_pContext->input.currInputTexCoord[0];

    if (input.vertexTexCoordPointer != nullptr && input.vertexTexCoordPtrEnabled)
    {
      vTexCoord.x = input.vertexTexCoordPointer[i*input.vertTexCoordComponents + 0];
      vTexCoord.y = input.vertexTexCoordPointer[i*input.vertTexCoordComponents + 1];
    }

    //if (a_pContext->logMode <= LOG_FOR_DEBUG_ERROR) a_pContext->m_log << "appending vertex  (4)" << i << std::endl;

    currBatch->vertPos.push_back(vPos);
    currBatch->vertColor.push_back(vColor);
    currBatch->vertNorm.push_back(vNorm);
    currBatch->vertTexCoord.push_back(vTexCoord);
  }

}

void swglAppendTriIndices(SWGL_Context* a_pContext, Batch* pCurr, GLenum currPrimType, size_t lastSizeVert) // pre (a_pContext != nullptr) && (pCurr != nullptr)
{

  switch (currPrimType)
  {
    case GL_TRIANGLES:
    {
      int triNum = int(pCurr->vertPos.size() - lastSizeVert)/3;
      int indNum = triNum * 3;

      for (int i = 0; i < indNum; i++)
        pCurr->indices.push_back(lastSizeVert + i);
    }
    break;

#ifdef FULL_GL

    case GL_QUADS:
    {
      int qNum = int(pCurr->vertPos.size() - lastSizeVert) / 4;

      for (int j = 0; j < qNum; j++)
      {
        pCurr->indices.push_back(lastSizeVert + j*4 + 0);
        pCurr->indices.push_back(lastSizeVert + j*4 + 1);
        pCurr->indices.push_back(lastSizeVert + j*4 + 2);

        pCurr->indices.push_back(lastSizeVert + j*4 + 0);
        pCurr->indices.push_back(lastSizeVert + j*4 + 2);
        pCurr->indices.push_back(lastSizeVert + j*4 + 3);
      }
    }
    break;

#endif

    case GL_TRIANGLE_FAN:
    {
      const int verts = pCurr->vertPos.size() - lastSizeVert;

      for (int j = 0; j < verts - 2; j++)
      {
        pCurr->indices.push_back(lastSizeVert + 0);
        pCurr->indices.push_back(lastSizeVert + j + 0 + 1);
        pCurr->indices.push_back(lastSizeVert + j + 1 + 1);
      }

    }
    break;


    case GL_TRIANGLE_STRIP:
    {
      const int verts = pCurr->vertPos.size() - lastSizeVert;

      int A = 0;
      int B = 1;
      int C = 2;

      for (int j = 0; j < verts - 2; j++)
      {
        pCurr->indices.push_back(lastSizeVert + A);
        pCurr->indices.push_back(lastSizeVert + B);
        pCurr->indices.push_back(lastSizeVert + C);

        A++; B++; C++;
      }

    }
    break;

    case GL_LINES:
    {
      int triNum = int(pCurr->vertPos.size() - lastSizeVert) / 2;
      int indNum = triNum * 2;

      for (int i = 0; i < indNum; i++)
        pCurr->indicesLines.push_back(lastSizeVert + i);
    }
    break;


    case GL_LINE_STRIP:
    {
      const int verts = pCurr->vertPos.size() - lastSizeVert;

      int A = 0;
      int B = 1;

      for (int j = 0; j < verts - 1; j++)
      {
        pCurr->indicesLines.push_back(lastSizeVert + A);
        pCurr->indicesLines.push_back(lastSizeVert + B);
        A++; B++;
      }

    }
    break;


    case GL_POINTS:
    {
      const int verts = pCurr->vertPos.size() - lastSizeVert;

      pCurr->indicesPoints.push_back(lastSizeVert + 0);
      pCurr->indicesPoints.push_back(lastSizeVert + verts);
      pCurr->pointSize.push_back(a_pContext->input.currPointSize);
    }
    break;


    default:
      break;

  };

}


void swglAppendTriIndices2(SWGL_Context* a_pContext, Batch* pCurr, GLenum currPrimType, size_t lastSizeVert, const int* inIndices, int count) // pre (a_pContext != nullptr) && (pCurr != nullptr) && (inIndices != nullptr)
{

  switch (currPrimType)
  {
  case GL_TRIANGLES:
  {
    for (int i = 0; i < count; i++)
      pCurr->indices.push_back(lastSizeVert + inIndices[i]);
  }
  break;

#ifdef FULL_GL

  case GL_QUADS:
  {
    //int qNum = int(pCurr->vertPos.size() - lastSizeVert) / 4;
    int qNum = count / 4; // ???

    for (int j = 0; j < qNum; j++)
    {
      pCurr->indices.push_back(lastSizeVert + inIndices[j * 4 + 0]);
      pCurr->indices.push_back(lastSizeVert + inIndices[j * 4 + 1]);
      pCurr->indices.push_back(lastSizeVert + inIndices[j * 4 + 2]);

      pCurr->indices.push_back(lastSizeVert + inIndices[j * 4 + 0]);
      pCurr->indices.push_back(lastSizeVert + inIndices[j * 4 + 2]);
      pCurr->indices.push_back(lastSizeVert + inIndices[j * 4 + 3]);
    }
  }
  break;

#endif

  case GL_TRIANGLE_FAN:
  {
    const int verts = pCurr->vertPos.size() - lastSizeVert;

    for (int j = 0; j < verts - 2; j++)
    {
      pCurr->indices.push_back(lastSizeVert + inIndices[0]);
      pCurr->indices.push_back(lastSizeVert + inIndices[j + 0 + 1]);
      pCurr->indices.push_back(lastSizeVert + inIndices[j + 1 + 1]);
    }

  }
  break;


  case GL_TRIANGLE_STRIP:
  {
    //const int verts = pCurr->vertPos.size() - lastSizeVert;
   const int verts = count; // ????????

    int A = 0;
    int B = 1;
    int C = 2;

    for (int j = 0; j < verts - 2; j++)
    {
      pCurr->indices.push_back(lastSizeVert + inIndices[A]);
      pCurr->indices.push_back(lastSizeVert + inIndices[B]);
      pCurr->indices.push_back(lastSizeVert + inIndices[C]);

      A++; B++; C++;
    }

  }
  break;

  case GL_LINES:
  {
    int triNum = int(pCurr->vertPos.size() - lastSizeVert) / 2;
    int indNum = triNum * 2;

    for (int i = 0; i < indNum; i++)
      pCurr->indicesLines.push_back(lastSizeVert + inIndices[i]);
  }
  break;


  case GL_LINE_STRIP:
  {
    const int verts = pCurr->vertPos.size() - lastSizeVert;

    int A = 0;
    int B = 1;

    for (int j = 0; j < verts - 1; j++)
    {
      pCurr->indicesLines.push_back(lastSizeVert + inIndices[A]);
      pCurr->indicesLines.push_back(lastSizeVert + inIndices[B]);
      A++; B++;
    }

  }
  break;


  case GL_POINTS:
  {
    const int verts = pCurr->vertPos.size() - lastSizeVert;

    pCurr->indicesPoints.push_back(lastSizeVert + inIndices[0]);
    pCurr->indicesPoints.push_back(lastSizeVert + inIndices[verts]);
    pCurr->pointSize.push_back(a_pContext->input.currPointSize);
  }
  break;


  default:
    break;

  };

}


#ifdef ENABLE_SSE_VS

inline __m128 mul_sse(const __m128 cols[4], const __m128 v)
{
  const __m128 u1    = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0));
  const __m128 u2    = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
  const __m128 u3    = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
  const __m128 u4    = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3));

  const __m128 prod1 = _mm_mul_ps(u1, cols[0]);
  const __m128 prod2 = _mm_mul_ps(u2, cols[1]);
  const __m128 prod3 = _mm_mul_ps(u3, cols[2]);
  const __m128 prod4 = _mm_mul_ps(u4, cols[3]);

  return _mm_add_ps(_mm_add_ps(prod1, prod2), _mm_add_ps(prod3, prod4));
}


inline __m128  swglVertexShaderTransformSSE(Batch* pBatch, __m128 a_pos) // pre (pBatch != nullptr)
{
  const __m128 cs   = mul_sse(pBatch->state.worlViewProjCols, a_pos);

  const __m128 w    = _mm_shuffle_ps(cs, cs, _MM_SHUFFLE(3, 3, 3, 3));
  const __m128 invW = _mm_rcp_ps(w);

  const __m128 w2   = _mm_mul_ss(w, invW);                              // [1.0f, w, w, w]
  const __m128 cs1  = _mm_shuffle_ps(cs, w2, _MM_SHUFFLE(1, 0, 1, 0));  // [cs.x, cs.y, 1.0f, w]

  return _mm_mul_ps(cs1, invW);  // float4(clipSpace.x*invW, clipSpace.y*invW, invW, 1.0f); <-- (clipSpace.x, clipSpace.y, 1.0f, w)
}


inline __m128 swglClipSpaceToScreenSpaceTransformSSE(__m128 a_pos, const __m128 viewportf) // pre (g_pContext != nullptr)
{
  const __m128 xyuu = _mm_add_ps(_mm_mul_ps(a_pos, const_half_one), const_half_one);
  const __m128 vpzw = _mm_shuffle_ps(viewportf, viewportf, _MM_SHUFFLE(3, 2, 3, 2));
  const __m128 ss   = _mm_add_ps(_mm_sub_ps(viewportf, const_half_one), _mm_mul_ps(xyuu, vpzw));

  return _mm_shuffle_ps(ss, a_pos, _MM_SHUFFLE(3, 2, 1, 0));
}

#endif


#ifndef WIN32

 int atomic_add(int* pAddress, int pVal)
 {
   return __sync_fetch_and_add(pAddress, pVal);
   //return __sync_add_and_fetch(pAddress, pVal) - 1;
 }

#else

 int atomic_add(int* pAddress, int pVal)
 {
   return InterlockedExchangeAdd((volatile long*)pAddress, pVal);
 }

#endif // WIN32

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void swglRunBatchVertexShader(SWGL_Context* a_pContext, Batch* pBatch) // pre (a_pContext != nullptr && pBatch != nullptr)
{
#ifdef MEASURE_STATS
  Timer timer(true);
#endif

  const float viewportf[4] = { (float)pBatch->state.viewport[0], 
                               (float)pBatch->state.viewport[1], 
                               (float)pBatch->state.viewport[2], 
                               (float)pBatch->state.viewport[3] };

  HWImpl::VertexShader((const float*)pBatch->vertPos.data(), (float*)pBatch->vertPos.data(), int(pBatch->vertPos.size()),
                        viewportf, pBatch->state.worldViewMatrix.L(), pBatch->state.projMatrix.L());

#ifdef MEASURE_STATS
  a_pContext->m_timeStats.msVertexShader += timer.getElapsed()*1000.0f;
#endif
}

void clampTriBBox(Triangle* t1, const FrameBuffer& frameBuff)
{
  // clamp viewport
  //
  if (t1->bb_iminX <  frameBuff.vx)                t1->bb_iminX = frameBuff.vx;
  if (t1->bb_imaxX >= frameBuff.vx + frameBuff.vw) t1->bb_imaxX = frameBuff.vx + frameBuff.vw - 1;

  if (t1->bb_iminY <  frameBuff.vy)                t1->bb_iminY = frameBuff.vy;
  if (t1->bb_imaxY >= frameBuff.vy + frameBuff.vh) t1->bb_imaxY = frameBuff.vy + frameBuff.vh - 1;

  // clamp screen
  //
  if (t1->bb_imaxX < 0) t1->bb_imaxX = 0;
  if (t1->bb_imaxY < 0) t1->bb_imaxY = 0;

  if (t1->bb_iminX >= frameBuff.w) t1->bb_iminX = frameBuff.w - 1;
  if (t1->bb_iminY >= frameBuff.h) t1->bb_iminY = frameBuff.h - 1;

}

#ifdef ENABLE_SSE

inline __m128 colorSwap(const __m128 a_col)
{
  return _mm_shuffle_ps(a_col, a_col, _MM_SHUFFLE(3, 0, 1, 2));
}

void swglTriangleSetUpSSE(const SWGL_Context* a_pContext, const Batch* pBatch, const FrameBuffer& frameBuff, const int i1, const int i2, const int i3, Triangle* t1, bool triangleIsTextured)
{
  const float* vpos = (const float*)&pBatch->vertPos[0];
  const float* vcol = (const float*)&pBatch->vertColor[0];
  const float* vtex = (const float*)&pBatch->vertTexCoord[0];

  const __m128 v1 = _mm_load_ps(vpos + i1 * 4);
  const __m128 v2 = _mm_load_ps(vpos + i2 * 4);
  const __m128 v3 = _mm_load_ps(vpos + i3 * 4);

  const __m128 c1 = colorSwap(_mm_load_ps(vcol + i1 * 4));
  const __m128 c2 = colorSwap(_mm_load_ps(vcol + i2 * 4));
  const __m128 c3 = colorSwap(_mm_load_ps(vcol + i3 * 4));

  _mm_store_ps((float*)&t1->v1, v1);
  _mm_store_ps((float*)&t1->v2, v2);
  _mm_store_ps((float*)&t1->v3, v3);

  const __m128 tx1 = _mm_loadu_ps(vtex + i1 * 2);
  const __m128 tx2 = _mm_loadu_ps(vtex + i2 * 2);
  const __m128 tx3 = _mm_loadu_ps(vtex + i3 * 2);

  // const __m128 v1v3v2X = _mm_set_ps(0.0f, v1.x, v3.x, v2.x);
  // const __m128 v1v3v2Y = _mm_set_ps(0.0f, v1.y, v3.y, v2.y);
  // const __m128 v2v1v3X = _mm_set_ps(0.0f, v2.x, v1.x, v3.x);
  // const __m128 v2v1v3Y = _mm_set_ps(0.0f, v2.y, v1.y, v3.y);
  // const __m128 v3v2v1Z = _mm_set_ps(0.0f, v3.z, v2.z, v1.z);

  // i like to mov it mov it ...
  //
  {
    const __m128 v2xv3xX = _mm_shuffle_ps(v2, v3, _MM_SHUFFLE(0, 0, 0, 0));
    const __m128 v2v3xxX = _mm_shuffle_ps(v2xv3xX, v2xv3xX, _MM_SHUFFLE(0, 0, 2, 0));

    const __m128 v1v3v2X = _mm_shuffle_ps(v2v3xxX, v1, _MM_SHUFFLE(0, 0, 1, 0));      // got _mm_set_ps(0.0f, v1.x, v3.x, v2.x);
    const __m128 v2v1v3X = _mm_shuffle_ps(v1v3v2X, v1v3v2X, _MM_SHUFFLE(0, 0, 2, 1)); // got _mm_set_ps(0.0f, v1.y, v3.y, v2.y) from _mm_set_ps(0.0f, v1.x, v3.x, v2.x);

    // i like to mov it mov it ...
    //
    const __m128 v2xv3xY = _mm_shuffle_ps(v2, v3, _MM_SHUFFLE(1, 1, 1, 1));
    const __m128 v2v3xxY = _mm_shuffle_ps(v2xv3xY, v2xv3xY, _MM_SHUFFLE(0, 0, 2, 0));

    const __m128 v1v3v2Y = _mm_shuffle_ps(v2v3xxY, v1, _MM_SHUFFLE(1, 1, 1, 1));      // got _mm_set_ps(0.0f, v1.y, v3.y, v2.y);
    const __m128 v2v1v3Y = _mm_shuffle_ps(v1v3v2Y, v1v3v2Y, _MM_SHUFFLE(0, 0, 2, 1)); // got _mm_set_ps(0.0f, v2.y, v1.y, v3.y) from _mm_set_ps(0.0f, v2.x, v1.x, v3.x);

    // i like to mov it mov it ...
    //
    const __m128 v1xv2xZ = _mm_shuffle_ps(v1, v2, _MM_SHUFFLE(2, 2, 2, 2));
    const __m128 v1v2xxZ = _mm_shuffle_ps(v1xv2xZ, v1xv2xZ, _MM_SHUFFLE(0, 0, 2, 0));
    const __m128 v1v3v2Z = _mm_shuffle_ps(v1v2xxZ, v3, _MM_SHUFFLE(2, 2, 1, 0));      // got _mm_set_ps(0.0f, v3.z, v2.z, v1.z);


    // finally stop this madeness ....
    //
    const __m128 edgesX = _mm_sub_ps(v2v1v3X, v1v3v2X);
    const __m128 edgesY = _mm_sub_ps(v2v1v3Y, v1v3v2Y);

    const __m128 edgeTest = _mm_or_ps(_mm_and_ps(_mm_cmpeq_ps(edgesY, _mm_setzero_ps()), _mm_cmpgt_ps(edgesX, _mm_setzero_ps())), _mm_cmpgt_ps(edgesY, _mm_setzero_ps()));

    t1->edgeTest = _mm_or_ps(_mm_and_ps(_mm_cmpeq_ps(edgesY, _mm_setzero_ps()), _mm_cmpgt_ps(edgesX, _mm_setzero_ps())), _mm_cmpgt_ps(edgesY, _mm_setzero_ps()));
    t1->v3v2v1Z = v1v3v2Z;
    t1->v1v3v2X = v1v3v2X;
  }

  __m128 triAreaInv = _mm_rcp_ss(edgeFunction2(v1, v2, v3));
  _mm_store_ss(&t1->triAreaInv, triAreaInv);

  {
    const __m128 v3v1yy1 = _mm_shuffle_ps(v3, v1, _MM_SHUFFLE(1, 1, 1, 1));
    const __m128 v3v1yy2 = _mm_shuffle_ps(v3v1yy1, v3v1yy1, _MM_SHUFFLE(0, 0, 2, 0));

    const __m128 y1 = _mm_shuffle_ps(v3v1yy2, v2, _MM_SHUFFLE(1, 1, 1, 0)); //  _mm_set_ps(0.0f, t1->v2.y, t1->v1.y, t1->v3.y);
    const __m128 y2 = _mm_shuffle_ps(y1, y1, _MM_SHUFFLE(0, 1, 0, 2)); //  _mm_set_ps(0.0f, t1->v1.y, t1->v3.y, t1->v2.y);

    t1->kv1 = _mm_mul_ps(_mm_shuffle_ps(triAreaInv, triAreaInv, _MM_SHUFFLE(0, 0, 0, 0)), _mm_sub_ps(y1, y2));
  }

  t1->cv1 = c1;
  t1->cv2 = c2;
  t1->cv3 = c3;

  t1->tv1 = tx1;
  t1->tv2 = tx2;
  t1->tv3 = tx3;

  const __m128 bbMin = _mm_min_ps(_mm_min_ps(v1,v2), v3);
  const __m128 bbMax = _mm_max_ps(_mm_max_ps(v1,v2), v3);

  const __m128i bbMinI = _mm_cvtps_epi32(bbMin);
  const __m128i bbMaxI = _mm_cvtps_epi32(bbMax);

  t1->bb_iminX = _mm_cvtsi128_si32(bbMinI);
  t1->bb_imaxX = _mm_cvtsi128_si32(bbMaxI);

  t1->bb_iminY = _mm_cvtsi128_si32(_mm_shuffle_epi32(bbMinI, _MM_SHUFFLE(0,0,1,1)));
  t1->bb_imaxY = _mm_cvtsi128_si32(_mm_shuffle_epi32(bbMaxI, _MM_SHUFFLE(0,0,1,1)));

  clampTriBBox(t1, frameBuff);  // need this for scan line to prevent out of border

  //
  //
  if (triangleIsTextured)
  {
    const SWGL_TextureStorage& tex = a_pContext->m_textures[pBatch->state.slot_GL_TEXTURE_2D];

    t1->texS.pitch = tex.pitch;   // tex.w; // !!! this is for textures with billet
    t1->texS.w     = tex.w;       // tex.w; // !!! this is for textures with billet
    t1->texS.h     = tex.h;
    t1->texS.data  = tex.texdata; // &tex.data[0]; // !!! this is for textures with billet
    t1->texS.txwh  = tex.xxwh;

    ///////////////////////////////////////////////////////////////////////////////////////////////////// FUCKING FUCK! FUCK LEGACY STATES! FUCK OPENGL!
    if (tex.modulateMode == GL_REPLACE) // don't apply vertex color, just take color from texture
    {
      if (tex.format == GL_RGBA)
      {
        t1->cv1 = const_1111;
        t1->cv2 = const_1111;
        t1->cv3 = const_1111;
      }
      else if (tex.format == GL_ALPHA)
      {
        #ifdef WIN32
        t1->cv1.m128_f32[3] = 1.0f;
        t1->cv2.m128_f32[3] = 1.0f;
        t1->cv3.m128_f32[3] = 1.0f;
        #else

        // put fucking color here

        #endif
      }
    }
  }
  ///////////////////////////////////////////////////////////////////////////////////////////////////// FUCKING FUCK! FUCK LEGACY STATES! FUCK OPENGL!

#ifdef PERSP_CORRECT

  if (frameBuff.zbuffer != nullptr)
  {
    __m128 invZ1 = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(2, 2, 2, 2)); // _mm_set_ps(t1->v1.z, t1->v1.z, t1->v1.z, t1->v1.z);
    __m128 invZ2 = _mm_shuffle_ps(v2, v2, _MM_SHUFFLE(2, 2, 2, 2)); // _mm_set_ps(t1->v2.z, t1->v2.z, t1->v2.z, t1->v2.z);
    __m128 invZ3 = _mm_shuffle_ps(v3, v3, _MM_SHUFFLE(2, 2, 2, 2)); // _mm_set_ps(t1->v3.z, t1->v3.z, t1->v3.z, t1->v3.z);

    t1->cv1 = _mm_mul_ps(t1->cv1, invZ1);
    t1->cv2 = _mm_mul_ps(t1->cv2, invZ2);
    t1->cv3 = _mm_mul_ps(t1->cv3, invZ3);

    t1->tv1 = _mm_mul_ps(t1->tv1, invZ1);
    t1->tv2 = _mm_mul_ps(t1->tv2, invZ2);
    t1->tv3 = _mm_mul_ps(t1->tv3, invZ3);
  }

#endif

}

#else




#endif


// void swglTileRaster_ForSeg(void* customData, int begin, int end)
// {
//   TileRasterData* pData = (TileRasterData*)customData;
// 
//   SWGL_DrawList* a_pDrawList    = pData->pDrawList;
//   const FrameBuffer* pFrameBuff = pData->pFrameBuff;
// 
// 
// #ifdef ENABLE_SSE
//   _MM_SET_ROUNDING_MODE(_MM_ROUND_TOWARD_ZERO);
// #endif
// 
//   if (a_pDrawList == nullptr || pFrameBuff == nullptr)
//     return;
// 
//   if (a_pDrawList->tilesIds.size() < size_t(end))
//     return;
// 
//   for (int i = begin; i < end; i++)
//   {
//     const int2 tileCoord = a_pDrawList->tilesIds[i];
//     const auto& tile     = a_pDrawList->tiles[tileCoord.x][tileCoord.y];
// 
//     FrameBuffer fb = (*pFrameBuff);
// 
//     fb.vx = tile.minX;
//     fb.vy = tile.minY;
//     fb.vw = tile.maxX - tile.minX;
//     fb.vh = tile.maxY - tile.minY;
// 
//     // clamp to screen size
//     if (fb.vx + fb.vw >= fb.w) fb.vw = fb.w - fb.vx;
//     if (fb.vy + fb.vh >= fb.h) fb.vh = fb.h - fb.vy;
// 
//     auto zbuff = fb.zbuffer;
//     auto sbuff = fb.sbuffer;
// 
//     for (int tri = tile.beginOffs; tri < tile.endOffs; tri++)
//     {
//       const int triId2 = a_pDrawList->m_tilesTriIndicesMemory[tri];
//       Triangle triCopy = a_pDrawList->m_triMemory[triId2];
// 
//       const auto* pso = &(a_pDrawList->m_psoArray[triCopy.psoId]);
// 
//       if (pso->depthTestEnabled)
//         fb.zbuffer = zbuff;
//       else
//         fb.zbuffer = nullptr;
// 
//       if (pso->stencilTestEnabled)
//         fb.sbuffer = sbuff;
//       else
//         fb.sbuffer = nullptr;
// 
//       clampTriBBox(&triCopy, fb); // frameBuff
// 
//       swglRasterizeTriangle(a_pDrawList->m_stateFuncs[triId2], &fb, triCopy);
//     }
//   }
// 
// }


void swglClearDrawListAndTiles(SWGL_DrawList* a_pDrawList, SWGL_FrameBuffer* a_pTiledFB, const int triNum)
{
  a_pDrawList->m_triTop = 0;
  a_pDrawList->m_linTop = 0;
  a_pDrawList->m_ptsTop = 0;

  const size_t tilesNum = a_pTiledFB->tiles.size();

  if (triNum == 0 || tilesNum == 0)
    return;

  if (a_pDrawList->m_triMemory.size() < (size_t)triNum)
    a_pDrawList->m_triMemory.resize(triNum);

  if (a_pDrawList->m_tilesTriIndicesMemory.size() < triNum*tilesNum)
    a_pDrawList->m_tilesTriIndicesMemory.resize(triNum*tilesNum);

  for (size_t i = 0; i < tilesNum; i++)
  {
    auto& tile2   = a_pTiledFB->tiles[i];
    tile2.begOffs = i*triNum;
    tile2.endOffs = tile2.begOffs;
  }

  if (a_pDrawList->m_psoArray.capacity() < 100)
    a_pDrawList->m_psoArray.reserve(100);

  a_pDrawList->m_psoArray.resize(0);

}


void swglAppendTrianglesToDrawList(SWGL_DrawList* a_pDrawList, SWGL_Context* a_pContext, const Batch* pBatch, 
                                   const FrameBuffer& frameBuff, SWGL_FrameBuffer* a_pTiledFB)
{
  if (a_pDrawList->m_tilesTriIndicesMemory.size() == 0)
    return;

#ifdef MEASURE_STATS
  Timer timer(true);
#endif

  const int  triNum = int(pBatch->indices.size() / 3);
  const int top = atomic_add(&a_pDrawList->m_triTop, triNum);

  a_pDrawList->m_psoArray.push_back(pBatch->state);
  int psoId = a_pDrawList->m_psoArray.size() - 1;

  int* triIndicesMem              = &(a_pDrawList->m_tilesTriIndicesMemory[0]);
  const std::vector<int>& indices = pBatch->indices;

  const bool trianglesAreTextured = pBatch->state.texure2DEnabled && (pBatch->state.slot_GL_TEXTURE_2D < (GLuint)a_pContext->m_texTop);

  //#pragma omp parallel for if (triNum >= 1000)
  for (int triId = 0; triId < triNum; triId++)
  {
    const int i1 = indices[triId * 3 + 0];
    const int i2 = indices[triId * 3 + 1];
    const int i3 = indices[triId * 3 + 2];

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const float4 v1 = pBatch->vertPos[i1];
    const float4 v2 = pBatch->vertPos[i2];
    const float4 v3 = pBatch->vertPos[i3];

    if (pBatch->state.cullFaceEnabled && pBatch->state.cullFaceMode != 0)
    {
      const float4 u = v2 - v1;
      const float4 v = v3 - v1;
      const float nz = u.x*v.y - u.y*v.x;

      const bool cullFace = ((pBatch->state.cullFaceMode == GL_FRONT) && (nz > 0.0f)) ||
                            ((pBatch->state.cullFaceMode == GL_BACK)  && (nz < 0.0f));

      if (cullFace)
        continue;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Triangle* pTri   = &(a_pDrawList->m_triMemory[top + triId]);
    pTri->psoId      = psoId;
    pTri->curr_sval  = pBatch->state.stencilValue;
    pTri->curr_smask = pBatch->state.stencilMask;

    HWImpl::TriangleSetUp(a_pContext, pBatch, frameBuff, i1, i2, i3, trianglesAreTextured, pTri);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const unsigned int tMinX = divTileSize(pTri->bb_iminX);
    const unsigned int tMinY = divTileSize(pTri->bb_iminY);
 
    const unsigned int tMaxX = divTileSize(pTri->bb_imaxX);
    const unsigned int tMaxY = divTileSize(pTri->bb_imaxY);

    for (unsigned int ty = tMinY; ty <= tMaxY; ty++)
    {
      for (unsigned int tx = tMinX; tx <= tMaxX; tx++)
      {
        const int tileMinX = tx*BIN_SIZE;
        const int tileMinY = ty*BIN_SIZE;
        
        const int tileMaxX = tx*BIN_SIZE + BIN_SIZE;
        const int tileMaxY = ty*BIN_SIZE + BIN_SIZE;

        //auto& tile = a_pDrawList->tiles[tx][ty];
        auto& tile = a_pTiledFB->tiles[ty*a_pTiledFB->sizeX + tx];

        if (AABBTriangleOverlap(*pTri, tileMinX, tileMinY, tileMaxX, tileMaxY))
        {
          int oldOffset = atomic_add((int*)&tile.endOffs, 1); // tile.endOffs; tile.endOffs++;
          triIndicesMem[oldOffset] = top + triId;
        }
      }
    }
  }

#ifdef MEASURE_STATS
  a_pContext->m_timeStats.msTriSetUp += timer.getElapsed()*1000.0f;
#endif

}

void swglDrawListInParallel(SWGL_Context* a_pContext, SWGL_DrawList* a_pDrawList, const FrameBuffer& frameBuff)
{

#ifdef MEASURE_STATS
  Timer timer(true);
#endif

  const int tilesNum = a_pContext->m_tiledFrameBuffer.tiles.size();

  // #pragma omp parallel for
  // for (int i = 0; i < tilesNum; i++)
  //   swglTileRaster_ForSeg((void*)&tdata, i, i + 1);

#ifdef MEASURE_STATS
  a_pContext->m_timeStats.msRasterAndPixelShader += timer.getElapsed()*1000.0f;
#endif

}


void swglDrawBatch(SWGL_Context* a_pContext, Batch* pBatch) // pre (a_pContext != nullptr && pBatch != nullptr)
{
#ifdef MEASURE_NOLOAD_PERF
  return;
#endif

#ifdef ENABLE_SSE
  _MM_SET_ROUNDING_MODE(_MM_ROUND_TOWARD_ZERO);
#endif

  swglRunBatchVertexShader(a_pContext, pBatch);

  FrameBuffer frameBuff = swglBatchFb(a_pContext, pBatch->state);

  swglDrawBatchTriangles(a_pContext, pBatch, frameBuff);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// int swglGetDrawListFreeSpace(SWGL_DrawList* a_pDrawList) // pre (a_pDrawList != nullptr)
// {
//   const size_t tilesNum = a_pDrawList->tilesIds.size();
//   if(tilesNum == 0)
//     return 0;
// 
//   int maxSize = 0;
// 
//   for (size_t i = 0; i < tilesNum; i++)
//   {
//     int2 tileCoord   = a_pDrawList->tilesIds[i];
//     const auto& tile = a_pDrawList->tiles[tileCoord.x][tileCoord.y];
//     const int size   = tile.endOffs - tile.beginOffs;
// 
//     if (size > maxSize)
//       maxSize = size;
//   }
// 
//   int2 res(0, 0);
// 
//   res.x = a_pDrawList->m_triMemory.size() - a_pDrawList->m_triTop;
//   res.y = a_pDrawList->m_tilesTriIndicesMemory.size() / tilesNum - maxSize;
// 
//   if (res.x < res.y)
//     return res.x;
//   else
//     return res.y;
// }
