#include "swgl.h"
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

  m_texTop = 0;
  m_textures.resize(1024); // max 1024 tex

  m_useTiledFB  = false;
  m_useTriQueue = false;

  if(m_useTiledFB)
    swglClearDrawListAndTiles(&m_drawList, &m_tiledFrameBuffer, MAX_NUM_TRIANGLES_TOTAL);
}

void SWGL_Context::ResizeCommon(int width, int height)
{
  const int tileSize = 4; // when we have 8x8 tiles, we just alloc a bit more memory then needed, but it should work fine
  const int size     = (width/tileSize)*(height/tileSize);
  m_locks = new std::atomic_flag[size];
  for(int i=0;i<size;i++)
    m_locks[i].clear(std::memory_order_release);
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

  if (a_pContext->logMode <= LOG_FOR_DEBUG_ERROR)
  {
    *(a_pContext->m_pLog) << "swglAppendVertices: (first,count) = (" << first << "," << count << ")" << std::endl;
    *(a_pContext->m_pLog) << "vertexPosPointer      = " << input.vertexPosPointer << std::endl;
    *(a_pContext->m_pLog) << "vertexColorPointer    = " << input.vertexColorPointer << std::endl;
    *(a_pContext->m_pLog) << "vertexNormalPointer   = " << input.vertexNormalPointer << std::endl;
    *(a_pContext->m_pLog) << "vertexTexCoordPointer = " << input.vertexTexCoordPointer << std::endl;
    *(a_pContext->m_pLog) << "vertPosComponents     = " << input.vertPosComponents << std::endl;
  }

  const int oldSize = int(currBatch->vertPos.size());

  currBatch->vertPos.resize(currBatch->vertPos.size() + count);
  currBatch->vertColor.resize(currBatch->vertColor.size() + count);
  //currBatch->vertNorm.resize(currBatch->vertNorm.size() + count);
  currBatch->vertTexCoord.resize(currBatch->vertTexCoord.size() + count);

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

    float4 vColor = a_pContext->input.currInputColor;

    if (input.vertexColorPointer != nullptr && input.vertexColorPtrEnabled)
    {
      vColor.x = input.vertexColorPointer[i*input.vertColorComponents + 0];
      vColor.y = input.vertexColorPointer[i*input.vertColorComponents + 1];
      vColor.z = input.vertexColorPointer[i*input.vertColorComponents + 2];

      if (input.vertColorComponents == 4)
        vColor.w = input.vertexColorPointer[i*input.vertColorComponents + 3];
    }

//    float4 vNorm = a_pContext->input.currInputNormal;
//
//    if (input.vertexNormalPointer != nullptr && input.vertexNormalPtrEnabled)
//    {
//      vNorm.x = input.vertexNormalPointer[i*input.vertNormalComponents + 0];
//      vNorm.y = input.vertexNormalPointer[i*input.vertNormalComponents + 1];
//      vNorm.z = input.vertexNormalPointer[i*input.vertNormalComponents + 2];
//    }

    float2 vTexCoord = a_pContext->input.currInputTexCoord[0];

    if (input.vertexTexCoordPointer != nullptr && input.vertexTexCoordPtrEnabled)
    {
      vTexCoord.x = input.vertexTexCoordPointer[i*input.vertTexCoordComponents + 0];
      vTexCoord.y = input.vertexTexCoordPointer[i*input.vertTexCoordComponents + 1];
    }

    const int index = oldSize + i - first;

    currBatch->vertPos  [index] = vPos;
    currBatch->vertColor[index] = vColor;
    //currBatch->vertNorm[index] = vNorm;
    currBatch->vertTexCoord[index] = vTexCoord;
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


int swglAppendTriIndices2(SWGL_Context* a_pContext, Batch* pCurr, GLenum currPrimType, size_t lastSizeVert, const int* inIndices, int count) // pre (a_pContext != nullptr) && (pCurr != nullptr) && (inIndices != nullptr)
{
  int maxId = 0;

  switch (currPrimType)
  {
  case GL_TRIANGLES:
  {
    pCurr->indices.resize(pCurr->indices.size() + count);

    for (int i = 0; i < count; i++)
    {
      const int index = lastSizeVert + inIndices[i];
      if (index > maxId)
        maxId = index;

      pCurr->indices[i] = index;
    }
  }
  break;

#ifdef FULL_GL

  case GL_QUADS:
  {
    //int qNum = int(pCurr->vertPos.size() - lastSizeVert) / 4;
    int qNum = count / 4; // ???

    for (int j = 0; j < qNum; j++)
    {
      const int index0 = lastSizeVert + inIndices[j * 4 + 0];
      const int index1 = lastSizeVert + inIndices[j * 4 + 1];
      const int index2 = lastSizeVert + inIndices[j * 4 + 2];
      const int index3 = lastSizeVert + inIndices[j * 4 + 3];

      if (index0 > maxId)
        maxId = index0;
      if (index1 > maxId)
        maxId = index1;
      if (index2 > maxId)
        maxId = index2;
      if (index3 > maxId)
        maxId = index3;

      pCurr->indices.push_back(index0);
      pCurr->indices.push_back(index1);
      pCurr->indices.push_back(index2);

      pCurr->indices.push_back(index0);
      pCurr->indices.push_back(index2);
      pCurr->indices.push_back(index3);
    }
  }
  break;

#endif

  case GL_TRIANGLE_FAN:
  {
    const int verts = pCurr->vertPos.size() - lastSizeVert;

    const int index0 = lastSizeVert + inIndices[0];
    if (index0 > maxId)
      maxId = index0;

    for (int j = 0; j < verts - 2; j++)
    {
      const int index1 = lastSizeVert + inIndices[j + 0 + 1];
      const int index2 = lastSizeVert + inIndices[j + 1 + 1];

      if (index1 > maxId)
        maxId = index1;
      if (index2 > maxId)
        maxId = index2;

      pCurr->indices.push_back(index0);
      pCurr->indices.push_back(index1);
      pCurr->indices.push_back(index2);
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
      const int index1 = lastSizeVert + inIndices[A];
      const int index2 = lastSizeVert + inIndices[B];
      const int index3 = lastSizeVert + inIndices[C];

      if (index1 > maxId)
        maxId = index1;
      if (index2 > maxId)
        maxId = index2;
      if (index3 > maxId)
        maxId = index3;

      pCurr->indices.push_back(index1);
      pCurr->indices.push_back(index2);
      pCurr->indices.push_back(index3);

      A++; B++; C++;
    }

  }
  break;

  //#TODO: implement maxId for lines and points !!!

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

  return maxId;
}

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

  pBatch->state.worldViewProjMatrix = mul(pBatch->state.projMatrix, pBatch->state.worldViewMatrix);

  HWImpl::VertexShader((const float*)pBatch->vertPos.data(), (float*)pBatch->vertPos.data(), int(pBatch->vertPos.size()),
                        viewportf, pBatch->state.worldViewProjMatrix.L());

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


void swglClearDrawListAndTiles(SWGL_DrawList* a_pDrawList, SWGL_FrameBuffer* a_pTiledFB, const int triNum)
{
  a_pDrawList->m_triTop = 0;
  a_pDrawList->m_triDrn = 0;

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

  for (int triId = 0; triId < triNum; triId++)
  {
    int i1 = indices[triId * 3 + 0];
    int i2 = indices[triId * 3 + 1];
    int i3 = indices[triId * 3 + 2];

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const float4 v1 = pBatch->vertPos[i1];
    const float4 v2 = pBatch->vertPos[i2];
    const float4 v3 = pBatch->vertPos[i3];

    const float4 u  = v2 - v1;
    const float4 v  = v3 - v1;
    const float nz  = u.x*v.y - u.y*v.x;

    if (pBatch->state.cullFaceEnabled && pBatch->state.cullFaceMode != 0)
    {
      const bool cullFace = ((pBatch->state.cullFaceMode == GL_FRONT) && (nz > 0.0f)) ||
                            ((pBatch->state.cullFaceMode == GL_BACK)  && (nz < 0.0f));

      if (cullFace)
        continue;
      else if (nz < 0.0f)
        std::swap(i2, i3);
    }
    else if (nz < 0.0f)
      std::swap(i2, i3);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Triangle* pTri   = &(a_pDrawList->m_triMemory[top + triId]);
    pTri->psoId      = psoId;
    //pTri->curr_sval  = pBatch->state.stencilValue;
    //pTri->curr_smask = pBatch->state.stencilMask;

    HWImpl::TriangleSetUp(a_pContext, pBatch, i1, i2, i3, pTri);
    clampTriBBox(pTri, frameBuff);                        // need this to prevent out of border

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

        auto& tile = a_pTiledFB->tiles[ty*a_pTiledFB->sizeX + tx];  // auto& tile = a_pDrawList->tiles[tx][ty];

        if (HWImpl::AABBTriangleOverlap(*pTri, tileMinX, tileMinY, tileMaxX, tileMaxY))
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

void swglDrawBatchTriangles(SWGL_Context* a_pContext, Batch* pBatch, FrameBuffer& frameBuff) // pre (a_pContext != nullptr && pBatch != nullptr)
{
  const std::vector<int>& indices = pBatch->indices;

  if (indices.size() == 0)
    return;

#ifdef MEASURE_STATS
  Timer timer(true);
#endif

  float timeAccumTriSetUp  = 0.0f;
  float timeAccumTriRaster = 0.0f;

  const int triNum = int(indices.size() / 3);

  #pragma omp parallel for if(triNum > 10) // #TODO: make parallel for and push triangles in queue with 1-2 threads (not many!).
  for (int triId = 0; triId < triNum; triId++)
  {
    int   i1 = indices[triId * 3 + 0];
    int   i2 = indices[triId * 3 + 1];
    int   i3 = indices[triId * 3 + 2];

    const float4 v1 = pBatch->vertPos[i1];
    const float4 v2 = pBatch->vertPos[i2];
    const float4 v3 = pBatch->vertPos[i3];

    const float4 u = v2 - v1;
    const float4 v = v3 - v1;
    const float nz = u.x*v.y - u.y*v.x;

    if (pBatch->state.cullFaceEnabled && pBatch->state.cullFaceMode != 0)
    {

      const bool cullFace = ((pBatch->state.cullFaceMode == GL_FRONT) && (nz > 0.0f)) ||
                            ((pBatch->state.cullFaceMode == GL_BACK) && (nz < 0.0f));

      if (cullFace)
        continue;
      else if (nz < 0.0f)
        std::swap(i2, i3);
    }
    else if (nz < 0.0f)
      std::swap(i2, i3);

    ////////////////////////////////////////////////////////////////////
    ////
    Triangle localTri;
    HWImpl::TriangleSetUp(a_pContext, pBatch, i1, i2, i3,
                          &localTri);
    
    const auto stateId = swglStateIdFromPSO(&pBatch->state, a_pContext, HWImpl::TriVertsAreOfSameColor(localTri));
    localTri.ropId     = stateId;
    ////
    ////////////////////////////////////////////////////////////////////
    
    clampTriBBox(&localTri, frameBuff);  // need this to prevent out of border, can be done in separate thread
    
    HWImpl::RasterizeTriangle(stateId, BlendOp_None, localTri, 0, 0,
                              &frameBuff);
  }

#ifdef MEASURE_STATS
  a_pContext->m_timeStats.msRasterAndPixelShader += timer.getElapsed()*1000.0f;
#endif

}

void swglEnqueueBatchTriangles(SWGL_Context* a_pContext, Batch* pBatch, FrameBuffer& frameBuff) // pre (a_pContext != nullptr && pBatch != nullptr)
{
  const std::vector<int>& indices = pBatch->indices;

  if (indices.size() == 0)
    return;

#ifdef MEASURE_STATS
  Timer timer(true);
#endif

  float timeAccumTriSetUp  = 0.0f;
  float timeAccumTriRaster = 0.0f;

  const int triNum = int(indices.size() / 3);

  #pragma omp parallel for if(triNum > 1000) num_threads(2)
  for (int triId = 0; triId < triNum; triId++)
  {
    int   i1 = indices[triId * 3 + 0];
    int   i2 = indices[triId * 3 + 1];
    int   i3 = indices[triId * 3 + 2];

    const float4 v1 = pBatch->vertPos[i1];
    const float4 v2 = pBatch->vertPos[i2];
    const float4 v3 = pBatch->vertPos[i3];

    const float4 u = v2 - v1;
    const float4 v = v3 - v1;
    const float nz = u.x*v.y - u.y*v.x;

    if (pBatch->state.cullFaceEnabled && pBatch->state.cullFaceMode != 0)
    {
      const bool cullFace = ((pBatch->state.cullFaceMode == GL_FRONT) && (nz > 0.0f)) ||
                            ((pBatch->state.cullFaceMode == GL_BACK) && (nz < 0.0f));

      if (cullFace)
        continue;
      else if (nz < 0.0f)
        std::swap(i2, i3);
    }
    else if (nz < 0.0f)
      std::swap(i2, i3);

    ////////////////////////////////////////////////////////////////////
    ////
    Triangle localTri;
    HWImpl::TriangleSetUp(a_pContext, pBatch, i1, i2, i3,
                          &localTri);

    const auto stateId = swglStateIdFromPSO(&pBatch->state, a_pContext, HWImpl::TriVertsAreOfSameColor(localTri));
    localTri.ropId     = stateId;
    ////
    ////////////////////////////////////////////////////////////////////

    a_pContext->m_tqueue.enqueue(localTri);
  }

#ifdef MEASURE_STATS
  a_pContext->m_timeStats.msRasterAndPixelShader += timer.getElapsed()*1000.0f;
#endif

}


RasterOp swglStateIdFromPSO(const Pipeline_State_Object* a_pso, const SWGL_Context* a_pContext, bool a_vertsAreOfSameColor)
{
  const bool trianglesAreTextured = a_pso->texure2DEnabled && (a_pso->slot_GL_TEXTURE_2D < (GLuint)a_pContext->m_texTop);

  if (a_pso->alphaBlendEnabled)
  {
    if (trianglesAreTextured)
    {
      if (a_pso->depthTestEnabled)
        return ROP_TexLinear3D_Blend;
      else
        return ROP_TexLinear2D_Blend;
    }
    else
    {
      if (a_pso->depthTestEnabled)
        return ROP_Colored3D_Blend;
      else
        return ROP_Colored2D_Blend;
    }
  }
  else
  {
    if (trianglesAreTextured)
    {
      if (a_pso->depthTestEnabled)
        return ROP_TexLinear3D;
      else
        return ROP_TexLinear2D;
    }
    else
    {
      if (a_pso->depthTestEnabled)
        return ROP_Colored3D;
      else
        return ROP_Colored2D;
    }
  }
  return ROP_FillColor;
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
