#pragma once

#include <cstdint>

static const uint64_t RM_FillColor         = 1;
static const uint64_t RM_Colored2D         = 2;
static const uint64_t RM_Colored3D         = 4;
static const uint64_t RM_TexLinear2D       = 8;
static const uint64_t RM_TexLinear3D       = 16;
static const uint64_t RM_TexLinear2D_Blend = 32;
static const uint64_t RM_TexLinear3D_Blend = 64;
static const uint64_t RM_StencilFill2D     = 128;
static const uint64_t RM_StencilFill3D     = 256;

struct SWGL_Context;
struct Batch;
struct SWGL_DrawList;
struct FrameBuffer;

struct ITriangleRasterizer
{
  ITriangleRasterizer(){}
  virtual ~ITriangleRasterizer(){}

  virtual uint64_t SupportedModesFlags()   const = 0;                      ///< Return flags what Raster Modes we are actually support .. i.e. Tex2D, Tex2D and e.t.c.
  virtual void     DrawBatch(SWGL_Context* a_pContext, Batch* pBatch) = 0; ///< Only Full pipeline shall implement this function

};

ITriangleRasterizer* CreateRasterizer(char* a_name);
