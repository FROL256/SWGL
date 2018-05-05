#ifndef FILL_ROW_GUARDIAN
#define FILL_ROW_GUARDIAN

#include "LiteMath.h"
#include "TriRaster.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawSpan_FillColor(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const Triangle& t, const float3 k2);
void DrawSpan_Colored2D(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const Triangle& t, const float3 k2);
void DrawSpan_Colored3D(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const Triangle& t, const float3 k2);

void DrawSpan_TexLinear2D(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const Triangle& t, const float3 k2);
void DrawSpan_TexLinear3D(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const Triangle& t, const float3 k2);

void DrawSpan_TexLinear2D_Blend(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const Triangle& t, const float3 k2);
void DrawSpan_TexLinear3D_Blend(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const Triangle& t, const float3 k2);

void DrawSpan_Colored2D_Blend(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const Triangle& t, const float3 k2);

void DrawSpan_NoColorStencilAlwaysReplace(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const Triangle& t, const float3 k2);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef ENABLE_SSE

void DrawSpan_FillColor_SSE(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const Triangle& t, const __m128 k2v);
void DrawSpan_Colored2D_SSE(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const Triangle& t, const __m128 k2v);
void DrawSpan_Colored3D_SSE(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const Triangle& t, const __m128 k2v);

void DrawSpan_TexLinear2D_SSE(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const Triangle& t, const __m128 k2v);
void DrawSpan_TexLinear3D_SSE(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const Triangle& t, const __m128 k2v);

void DrawSpan_TexLinear2D_Blend_SSE(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const Triangle& t, const __m128 k2v);
void DrawSpan_TexLinear3D_Blend_SSE(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const Triangle& t, const __m128 k2v);

void DrawSpan_Colored2D_Blend_SSE(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const Triangle& t, const __m128 k2v);

void DrawSpan_NoColorStencilAlwaysReplace_SSE(FrameBuffer* frameBuff, const int x1c, const int x2c, const int offset, const Triangle& t, const __m128 k2v);

#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#endif
