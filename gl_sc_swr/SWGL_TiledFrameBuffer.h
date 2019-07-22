#pragma once

#include "config.h"
#include <cstdint>
#include <vector>

#include "LiteMath.h"

struct ALIGNED(16) SWGL_ScreenTile
{
  void ClearColor(int32_t a_color);
  void ClearDepth(float a_val);

  // tile bounding box
  //
  int minX;
  int minY;
  int maxX;
  int maxY;

  int begOffs; // triangle indices
  int endOffs; // triangle indices

  // tile data
  //
  ALIGNED(64) int32_t m_color[BIN_SIZE*BIN_SIZE];
  ALIGNED(64) float   m_depth[BIN_SIZE*BIN_SIZE];
  //uint8_t m_sbuffer[BIN_SIZE*BIN_SIZE];
};


struct SWGL_FrameBuffer
{
  void Resize(int a_x, int a_y);
  void ClearColor(int32_t a_color);
  void ClearDepth(float a_val);
  void CopyToRowPitch(int32_t* a_data);  //#TODO: consider pipelined copy to video mem, tile by tile.

  void TestClearChessBoard();
  void TestFillNonEmptyTiles();

  int sizeX;
  int sizeY;
  std::vector<SWGL_ScreenTile> tiles;
};
