#include "HWAbstractionLayer.h"
#include "swgl.h"

#ifdef WIN32
#undef min
#undef max
#endif

#include <algorithm>

void HWImplementationPureCpp::memset32(int32_t* a_data, int32_t a_val, int32_t numElements)
{
  if (numElements % 16 == 0)
  {
    const uint64_t val = (((uint64_t)a_val) << 32) | (uint64_t)a_val;
    uint64_t* color64  = (uint64_t*)(a_data);
    const int size     = numElements/2;

    for (int i = 0; i < size; i += 8)
    {
      color64[i + 0] = val;
      color64[i + 1] = val;
      color64[i + 2] = val;
      color64[i + 3] = val;
      color64[i + 4] = val;
      color64[i + 5] = val;
      color64[i + 6] = val;
      color64[i + 7] = val;
    }
  }
  else
  {
    for (int i = 0; i < numElements; i++)
      a_data[i] = a_val;
  }
}

bool HWImplementationPureCpp::AABBTriangleOverlap(const TriangleType& a_tri, const int tileMinX, const int tileMinY, const int tileMaxX, const int tileMaxY)
{
   const bool overlapBoxBox = IntersectBoxBox(int2(a_tri.bb_iminX, a_tri.bb_iminY), int2(a_tri.bb_imaxX, a_tri.bb_imaxY),
                                              int2(tileMinX, tileMinY),             int2(tileMaxX, tileMaxY));

  if (!overlapBoxBox)
    return false;

  const float xx0 = float(tileMinX);
  const float xx1 = float(tileMaxX);
  const float yy0 = float(tileMinY);
  const float yy1 = float(tileMaxY);

  const float y1 = a_tri.v3.y;
  const float y2 = a_tri.v2.y;
  const float y3 = a_tri.v1.y;
                   
  const float x1 = a_tri.v3.x;
  const float x2 = a_tri.v2.x;
  const float x3 = a_tri.v1.x;

  // Deltas
  const float Dx12 = x1 - x2;
  const float Dx23 = x2 - x3;
  const float Dx31 = x3 - x1;

  const float Dy12 = y1 - y2;
  const float Dy23 = y2 - y3;
  const float Dy31 = y3 - y1;

  const bool s10 = ( Dx12 * (yy0 - y1) - Dy12 * (xx0 - x1) ) < 0.0f; 
  const bool s11 = ( Dx12 * (yy0 - y1) - Dy12 * (xx1 - x1) ) < 0.0f;
  const bool s12 = ( Dx12 * (yy1 - y1) - Dy12 * (xx0 - x1) ) < 0.0f;
  const bool s13 = ( Dx12 * (yy1 - y1) - Dy12 * (xx1 - x1) ) < 0.0f;
             
  const bool s20 = ( Dx12 * (yy0 - y2) - Dy12 * (xx0 - x2) ) < 0.0f;
  const bool s21 = ( Dx12 * (yy0 - y2) - Dy12 * (xx1 - x2) ) < 0.0f;
  const bool s22 = ( Dx12 * (yy1 - y2) - Dy12 * (xx0 - x2) ) < 0.0f;
  const bool s23 = ( Dx12 * (yy1 - y2) - Dy12 * (xx1 - x2) ) < 0.0f;
             
  const bool s30 = ( Dx12 * (yy0 - y3) - Dy12 * (xx0 - x3) ) < 0.0f;
  const bool s31 = ( Dx12 * (yy0 - y3) - Dy12 * (xx1 - x3) ) < 0.0f;
  const bool s32 = ( Dx12 * (yy1 - y3) - Dy12 * (xx0 - x3) ) < 0.0f;
  const bool s33 = ( Dx12 * (yy1 - y3) - Dy12 * (xx1 - x3) ) < 0.0f;

  return (s10 || s11 || s12 || s13 ||
          s20 || s21 || s22 || s23 ||
          s30 || s31 || s32 || s33 );
}


void HWImplementationPureCpp::VertexShader(const float* v_in4f, float* v_out4f, int a_numVert,
                                           const float viewportData[4], const float a_worldViewMatrix[16], const float a_projMatrix[16])
{
  const float4   viewportf(viewportData[0], viewportData[1], viewportData[2], viewportData[3]);
  const float4x4 worldViewMatrix(a_worldViewMatrix);
  const float4x4 projMatrix(a_projMatrix);
  const float4x4 worldViewProjMatrix = mul(projMatrix, worldViewMatrix);
  
  const float4* inVert  = (const float4*)v_in4f;
  float4* outVert       = (float4*)v_out4f;

  if (isIdentityMatrix(projMatrix))
  {
    for (int i = 0; i < a_numVert; i++)
    {
      const float4 vClipSpace   = mul(worldViewMatrix, inVert[i]);
      const float4 vScreenSpace = swglClipSpaceToScreenSpaceTransform(vClipSpace, viewportf);
      outVert[i]                = vScreenSpace;
    }
  }
  else
  {
    for (int i = 0; i < a_numVert; i++)
    {
      const float4 clipSpace    = mul(worldViewProjMatrix, inVert[i]);
      const float invW          = (1.0f / fmax(clipSpace.w, 1e-20f));
      const float4 vClipSpace   = float4(clipSpace.x*invW, clipSpace.y*invW, invW, 1.0f);
      const float4 vScreenSpace = swglClipSpaceToScreenSpaceTransform(vClipSpace, viewportf);
      outVert[i]                = vScreenSpace;
    }
  }

}

void HWImplementationPureCpp::TriangleSetUp(const SWGL_Context* a_pContext, const Batch* pBatch, const FrameBuffer& frameBuff,
                                            const int i1, const int i2, const int i3, TriangleType* t1)
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

  const float triArea = edgeFunction(to_float2(v1), to_float2(v2), to_float2(v3));
  t1->triArea    = triArea;
  t1->triAreaInv = 1.0f / triArea;
  
  t1->bb_iminX = (int)(fmin(v1.x, fmin(v2.x, v3.x)) - 1.0f);
  t1->bb_imaxX = (int)(fmax(v1.x, fmax(v2.x, v3.x)) + 1.0f);

  t1->bb_iminY = (int)(fmin(v1.y, fmin(v2.y, v3.y)) - 1.0f);
  t1->bb_imaxY = (int)(fmax(v1.y, fmax(v2.y, v3.y)) + 1.0f);

  clampTriBBox(t1, frameBuff);  // need this to prevent out of border
                                //

  const bool triangleIsTextured = pBatch->state.texure2DEnabled && (pBatch->state.slot_GL_TEXTURE_2D < (GLuint)a_pContext->m_texTop);

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



static void rasterizeTriHalf_WithRespectToTile(const Triangle& tri, int tileMinX, int tileMinY, FrameBuffer* frameBuf)
{
  const float tileMinX_f = float(tileMinX);
  const float tileMinY_f = float(tileMinY);

  const float y1 = tri.v3.y - tileMinY_f;
  const float y2 = tri.v2.y - tileMinY_f;
  const float y3 = tri.v1.y - tileMinY_f;

  const float x1 = tri.v3.x - tileMinX_f;
  const float x2 = tri.v2.x - tileMinX_f;
  const float x3 = tri.v1.x - tileMinX_f;

  // Deltas
  const float Dx12 = x1 - x2;
  const float Dx23 = x2 - x3;
  const float Dx31 = x3 - x1;

  const float Dy12 = y1 - y2;
  const float Dy23 = y2 - y3;
  const float Dy31 = y3 - y1;

  // Bounding rectangle
  const int minx = std::max(tri.bb_iminX - tileMinX, 0);
  const int miny = std::max(tri.bb_iminY - tileMinY, 0);
  const int maxx = std::min(tri.bb_imaxX - tileMinX, frameBuf->w - 1);
  const int maxy = std::min(tri.bb_imaxY - tileMinY, frameBuf->h - 1);

  int* colorBuffer = frameBuf->cbuffer;
  //uint8_t* sbuff   = frameBuf->getSBuffer();
  //float* zbuff     = frameBuf->getZBuffer();

  // Constant part of half-edge functions
  const float C1 = Dy12 * x1 - Dx12 * y1;
  const float C2 = Dy23 * x2 - Dx23 * y2;
  const float C3 = Dy31 * x3 - Dx31 * y3;

  float Cy1 = C1 + Dx12 * miny - Dy12 * minx;
  float Cy2 = C2 + Dx23 * miny - Dy23 * minx;
  float Cy3 = C3 + Dx31 * miny - Dy31 * minx;

  int offset = lineOffset(miny, frameBuf->w, frameBuf->h);

  // Scan through bounding rectangle
  for (int y = miny; y <= maxy; y++)
  {
    // Start value for horizontal scan
    float Cx1 = Cy1;
    float Cx2 = Cy2;
    float Cx3 = Cy3;

    for (int x = minx; x <= maxx; x++)
    {
      if (Cx1 > HALF_SPACE_EPSILON && Cx2 > HALF_SPACE_EPSILON && Cx3 > HALF_SPACE_EPSILON)
      {
        colorBuffer[offset + x] = 0xFFFFFFFF;
      }

      Cx1 -= Dy12;
      Cx2 -= Dy23;
      Cx3 -= Dy31;
    }

    Cy1 += Dx12;
    Cy2 += Dx23;
    Cy3 += Dx31;

    offset = nextLine(offset, frameBuf->w, frameBuf->h);
  }

}



void HWImplementationPureCpp::RasterizeTriangle(ROP_TYPE a_ropT, const TriangleType& tri, int tileMinX, int tileMinY,
                                                FrameBuffer* frameBuf)
{
  rasterizeTriHalf_WithRespectToTile(tri, tileMinX, tileMinY, frameBuf);
}