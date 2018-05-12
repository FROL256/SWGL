#pragma once

#include "config.h"

#include <cstdint>
#include <vector>

struct SWGL_ScreenTile
{

  void ClearColor(int32_t a_color);

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
  int32_t m_color  [BIN_SIZE*BIN_SIZE]; 
  //float   m_zbuffer[BIN_SIZE*BIN_SIZE];
  //uint8_t m_sbuffer[BIN_SIZE*BIN_SIZE];
};


struct SWGL_FrameBuffer
{
  void Resize(int a_x, int a_y);
  void ClearColor(int32_t a_color);
  void CopyToRowPitch(int32_t* a_data);  //#TODO: consider pipelined copy to video mem, tile by tile.

  void TestClearChessBoard();
  void TestFillNonEmptyTiles();

  int sizeX;
  int sizeY;
  std::vector<SWGL_ScreenTile> tiles;
};
