#pragma once

#include <cstdint>
#include <vector>

enum RENDER_MODE {
  RM_FillColor         = 1,
  RM_Colored2D         = 2,
  RM_Colored3D         = 3,
  RM_TexLinear2D       = 4,
  RM_TexLinear3D       = 5,
  RM_TexLinear2D_Blend = 6,
  RM_TexLinear3D_Blend = 7,
  RM_StencilFill2D     = 8,
  RM_StencilFill3D     = 9,
};

struct SWGL_Context;
struct SWGL_DrawList;

struct Batch;
struct FrameBuffer;

struct ITriangleRasterizer
{
  ITriangleRasterizer(){}
  virtual ~ITriangleRasterizer(){}

  virtual std::vector<bool> SupportedModesFlags() const                   = 0; ///< Return flags what Raster Modes we are actually support .. i.e. Tex2D, Tex2D and e.t.c.
  virtual void DrawBatch(const Batch* a_pBatch, SWGL_Context* a_pContext) = 0; ///< Only Full pipeline shall implement this function

};

//ITriangleRasterizer* CreateRasterizer(char* a_name);
