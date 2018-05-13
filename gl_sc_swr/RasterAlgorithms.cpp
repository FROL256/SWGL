#include "RasterAlgorithms.h"
#include "TriRaster.h"

void swglRunBatchVertexShaderNoSSE(SWGL_Context* a_pContext, Batch* a_pBatch) // pre (a_pContext != nullptr && pBatch != nullptr)
{
#ifdef MEASURE_STATS
  Timer timer(true);
#endif

  float4 viewportf((float)a_pBatch->state.viewport[0], (float)a_pBatch->state.viewport[1],
                   (float)a_pBatch->state.viewport[2], (float)a_pBatch->state.viewport[3]);

  a_pBatch->state.worldViewProjMatrix = mul(a_pBatch->state.projMatrix, a_pBatch->state.worldViewMatrix);

  if (isIdentityMatrix(a_pBatch->state.projMatrix))
  {
    for (int i = 0; i < int(a_pBatch->vertPos.size()); i++)
    {
      const float4 vClipSpace   = swglVertexShaderTransform2D(a_pBatch, a_pBatch->vertPos[i]);
      const float4 vScreenSpace = swglClipSpaceToScreenSpaceTransform(vClipSpace, viewportf);

      a_pBatch->vertPos[i] = vScreenSpace;
    }
  }
  else
  {
    for (int i = 0; i < int(a_pBatch->vertPos.size()); i++)
    {
      const float4 vClipSpace   = swglVertexShaderTransform(a_pBatch, a_pBatch->vertPos[i]);
      const float4 vScreenSpace = swglClipSpaceToScreenSpaceTransform(vClipSpace, viewportf);

      a_pBatch->vertPos[i] = vScreenSpace;
    }

  }

#ifdef MEASURE_STATS
  a_pContext->m_timeStats.msVertexShader += timer.getElapsed()*1000.0f;
#endif

}


#ifndef ENABLE_SSE

void swglTriangleSetUp(const SWGL_Context* a_pContext, const Batch* pBatch, const FrameBuffer& frameBuff, const int i1, const int i2, const int i3, Triangle* t1, bool triangleIsTextured)
{
  const float4 v1 = pBatch->vertPos[i1];
  const float4 v2 = pBatch->vertPos[i2];
  const float4 v3 = pBatch->vertPos[i3];

  const float4 c1 = pBatch->vertColor[i1];
  const float4 c2 = pBatch->vertColor[i2];
  const float4 c3 = pBatch->vertColor[i3];

  const float2 tx1 = pBatch->vertTexCoord[i1];
  const float2 tx2 = pBatch->vertTexCoord[i2];
  const float2 tx3 = pBatch->vertTexCoord[i3];

  //
  t1->v1 = v1;
  t1->v2 = v2;
  t1->v3 = v3;

  t1->c1 = c1;
  t1->c2 = c2;
  t1->c3 = c3;

  t1->t1 = tx1;
  t1->t2 = tx2;
  t1->t3 = tx3;

  t1->triAreaInv = 1.0f / edgeFunction(to_float2(v1), to_float2(v2), to_float2(v3));


  const float2 edge0 = to_float2(v3 - v2);
  const float2 edge1 = to_float2(v1 - v3);
  const float2 edge2 = to_float2(v2 - v1);

  t1->edgeTest0 = ((edge0.y == 0.0f && edge0.x > 0.0f) || edge0.y > 0.0f);
  t1->edgeTest1 = ((edge1.y == 0.0f && edge1.x > 0.0f) || edge1.y > 0.0f);
  t1->edgeTest2 = ((edge2.y == 0.0f && edge2.x > 0.0f) || edge2.y > 0.0f);


  t1->bb_iminX = (int)(fmin(v1.x, fmin(v2.x, v3.x)) - 1.0f);
  t1->bb_imaxX = (int)(fmax(v1.x, fmax(v2.x, v3.x)) + 1.0f);

  t1->bb_iminY = (int)(fmin(v1.y, fmin(v2.y, v3.y)) - 1.0f);
  t1->bb_imaxY = (int)(fmax(v1.y, fmax(v2.y, v3.y)) + 1.0f);

  clampTriBBox(t1, frameBuff);  // need this for scan line to prevent out of border
                                //

  if (triangleIsTextured)
  {
    const SWGL_TextureStorage& tex = a_pContext->m_textures[pBatch->state.slot_GL_TEXTURE_2D];

    t1->texS.pitch = tex.pitch;   // tex.w; // !!! this is for textures with billet
    t1->texS.w     = tex.w;       // tex.w; // !!! this is for textures with billet
    t1->texS.h     = tex.h;
    t1->texS.data  = tex.texdata; // &tex.data[0]; // !!! this is for textures with billet

    ///////////////////////////////////////////////////////////////////////////////////////////////////// FUCKING FUCK! FUCK LEGACY STATES! FUCK OPENGL!
    if (tex.modulateMode == GL_REPLACE) // don't apply vertex color, just take color from texture
    {
      if (tex.format == GL_RGBA)
      {
        t1->c1 = float4(1, 1, 1, 1);
        t1->c2 = float4(1, 1, 1, 1);
        t1->c3 = float4(1, 1, 1, 1);
      }
      else if (tex.format == GL_ALPHA)
      {
        t1->c1.w = 1.0f;
        t1->c2.w = 1.0f;
        t1->c3.w = 1.0f;
      }
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////// FUCKING FUCK! FUCK LEGACY STATES! FUCK OPENGL!

  }

#ifdef PERSP_CORRECT

  if (frameBuff.zbuffer != nullptr)
  {
    t1->c1 *= v1.z; // div by z, not mult!
    t1->c2 *= v2.z; // div by z, not mult!
    t1->c3 *= v3.z; // div by z, not mult!

    t1->t1 *= v1.z; // div by z, not mult!
    t1->t2 *= v2.z; // div by z, not mult!
    t1->t3 *= v3.z; // div by z, not mult!
  }

#endif

}

#endif

void swglDrawBatchTriangles(SWGL_Context* a_pContext, Batch* pBatch, FrameBuffer& frameBuff) // pre (a_pContext != nullptr && pBatch != nullptr)
{
  const std::vector<int>& indices = pBatch->indices;

  if (indices.size() == 0)
    return;

#ifdef MEASURE_STATS
  Timer timer(true);
  //Timer timerLocal(false);
#endif

  float timeAccumTriSetUp = 0.0f;
  float timeAccumTriRaster = 0.0f;

  const bool trianglesAreTextured = pBatch->state.texure2DEnabled && (pBatch->state.slot_GL_TEXTURE_2D < (GLuint)a_pContext->m_texTop);

  // const int vertNum = int(pBatch->vertPos.size());
  const int triNum = int(indices.size() / 3);

  for (int triId = 0; triId < triNum; triId++)
  {
    const int    i1 = indices[triId * 3 + 0];
    const int    i2 = indices[triId * 3 + 1];
    const int    i3 = indices[triId * 3 + 2];

    const float4 v1 = pBatch->vertPos[i1];
    const float4 v2 = pBatch->vertPos[i2];
    const float4 v3 = pBatch->vertPos[i3];

    if (pBatch->state.cullFaceEnabled && pBatch->state.cullFaceMode != 0)
    {
      const float4 u = v2 - v1;
      const float4 v = v3 - v1;
      const float nz = u.x*v.y - u.y*v.x;

      const bool cullFace = ((pBatch->state.cullFaceMode == GL_FRONT) && (nz > 0.0f)) ||
                            ((pBatch->state.cullFaceMode == GL_BACK) && (nz < 0.0f));

      if (cullFace)
        continue;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Triangle localTri;
    localTri.curr_sval  = pBatch->state.stencilValue;
    localTri.curr_smask = pBatch->state.stencilMask;

    #ifdef ENABLE_SSE
    swglTriangleSetUpSSE(a_pContext, pBatch, frameBuff, i1, i2, i3, &localTri, trianglesAreTextured);
    #else
    swglTriangleSetUp(a_pContext, pBatch, frameBuff, i1, i2, i3, &localTri, trianglesAreTextured);
    #endif // ENABLE_SSE

    swglRasterizeTriangle(&frameBuff, localTri);
  }

#ifdef MEASURE_STATS
  a_pContext->m_timeStats.msRasterAndPixelShader += timer.getElapsed()*1000.0f;
#endif

}

