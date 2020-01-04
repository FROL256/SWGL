#pragma once

#include "config.h"
#include <cstdint>
#include <vector>

#include "alloc16.h"
#include "LiteMath.h"


#define FB_TILE_SIZE_X 4
#define FB_TILE_SIZE_Y 4
#define FB_TILE_SIZE   16

struct SWGL_FrameBufferTwoLvl
{
  void Resize(int a_x, int a_y);
  void CopyToPitchLinear(uint32_t* a_data, int a_pitch);  

  void TestClearChessBoard();
  void TestFillNonEmptyTiles();

private:

  void FillBinColor(int bx, int by, uint32_t color);

  std::vector<cvex::vint4, aligned<cvex::vint4, 16> > binsMinMax;

  std::vector<float,    aligned<float, 64> >    m_depth;
  std::vector<uint32_t, aligned<uint32_t, 64> > m_color;

  int m_binsX;
  int m_binsY;
};
