#pragma once

#include "swgl.h"

void swglRunBatchVertexShaderNoSSE(SWGL_Context* a_pContext, Batch* a_pBatch);

void swglDrawBatchTriangles(SWGL_Context* a_pContext, Batch* pBatch, FrameBuffer& frameBuff);
void swglDrawBatchLines(SWGL_Context* a_pContext, Batch* pBatch, FrameBuffer& frameBuff);
void swglDrawBatchPoints(SWGL_Context* a_pContext, Batch* pBatch, FrameBuffer& frameBuff);

#ifdef ENABLE_SSE
void swglTriangleSetUpSSE(const SWGL_Context* a_pContext, const Batch* pBatch, const FrameBuffer& frameBuff, const int i1, const int i2, const int i3, Triangle* t1, bool triangleIsTextured);
#else
void swglTriangleSetUp(const SWGL_Context* a_pContext, const Batch* pBatch, const FrameBuffer& frameBuff, const int i1, const int i2, const int i3, Triangle* t1, bool triangleIsTextured);
#endif

static inline void swglRasterizeTriangle(const FillFuncPtr pFill, FrameBuffer* frameBuff, const Triangle& localTri)
{
  rasterizeTriHalfSpace(pFill, frameBuff, localTri);
}