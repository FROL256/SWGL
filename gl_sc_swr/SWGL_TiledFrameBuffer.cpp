#include "SWGL_TiledFrameBuffer.h"
#include "HWAbstractionLayer.h"

#include <memory.h>

void SWGL_ScreenTile::ClearColor(int32_t a_color)
{
  HWImpl::memset32(m_color, a_color, BIN_SIZE*BIN_SIZE);
}

void SWGL_ScreenTile::ClearDepth(float a_val)
{
  int32_t idata = *((int32_t*)(&a_val));
  HWImpl::memset32((int32_t*)m_depth, idata, BIN_SIZE*BIN_SIZE);
}

void SWGL_FrameBuffer::Resize(int a_w, int a_h) 
{
  const int tilesX = a_w / BIN_SIZE;
  const int tilesY = a_h / BIN_SIZE;

  sizeX = tilesX;
  sizeY = tilesY;

  tiles.resize(sizeX*sizeY); 

  for (int ty=0, j=0; ty < a_h; ty += BIN_SIZE, j++)
  {
    for (int tx=0, i=0; tx < a_w; tx += BIN_SIZE, i++)
    {
      auto& tile = tiles[j*tilesX + i];
      tile.minX = tx;
      tile.minY = ty;
      tile.maxX = tile.minX + BIN_SIZE;
      tile.maxY = tile.minY + BIN_SIZE;
    }
  }

}

void SWGL_FrameBuffer::ClearColor(int32_t a_color)
{
  #pragma omp parallel for
  for (int tileId = 0; tileId < tiles.size(); tileId++)
    tiles[tileId].ClearColor(a_color);
}

void SWGL_FrameBuffer::ClearDepth(float a_val)
{
  #pragma omp parallel for
  for (int tileId = 0; tileId < tiles.size(); tileId++)
    tiles[tileId].ClearDepth(a_val);
}

void SWGL_FrameBuffer::CopyToRowPitch(int32_t* a_data)
{
  const int pitchX = sizeX*BIN_SIZE;

  #pragma omp parallel for num_threads(NUM_THREADS)
  for (int tileId = 0; tileId < tiles.size(); tileId++)
  {
    int32_t* output = a_data + tiles[tileId].minY*pitchX + tiles[tileId].minX;

    for (int y = 0; y < BIN_SIZE; y++)
    {
      int32_t* inLine  = tiles[tileId].m_color + y*BIN_SIZE;
      int32_t* outLine = output + y*pitchX;

      memcpy(outLine, inLine, BIN_SIZE*sizeof(int32_t));
    }
  }
}

void SWGL_FrameBuffer::TestClearChessBoard() 
{
  constexpr int tableSize = 3;
  int table[tableSize] = { 0x00FF0000,0x0000FF00, 0x000000FF }; // 0x00FFFFFF

  for (int tileId = 0; tileId < tiles.size(); tileId++)
    tiles[tileId].ClearColor(table[tileId%tableSize]);
}

void SWGL_FrameBuffer::TestFillNonEmptyTiles()
{
  for (int tileId = 0; tileId < tiles.size(); tileId++)
  {
    if(tiles[tileId].begOffs != tiles[tileId].endOffs)
      tiles[tileId].ClearColor(0xFFFFFFFF);
  }
}
