#include "SWGL_TiledFrameBuffer.h"

void SWGL_FrameBuffer::Resize(int a_x, int a_y) 
{ 
  sizeX = a_x; 
  sizeY = a_y; 
  tiles.resize(sizeX*sizeY); 

  const int tilesX = a_x / BIN_SIZE;

  for (int ty = 0, j=0; ty < sizeY; ty += BIN_SIZE, j++)
  {
    for (int tx = 0, i=0; tx < sizeX; tx += BIN_SIZE, i++)
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
  const int64_t val = (a_color << 32) | a_color;

  for (int tileId = 0; tileId < tiles.size(); tileId++)
  {
    int64_t* color16 = (int64_t*)(tiles[tileId].m_color);
    const int size   = (BIN_SIZE*BIN_SIZE / 2);

    for (int i = 0; i < size; i += 8)
    {
      color16[i + 0] = val;
      color16[i + 1] = val;
      color16[i + 2] = val;
      color16[i + 3] = val;
      color16[i + 4] = val;
      color16[i + 5] = val;
      color16[i + 6] = val;
      color16[i + 7] = val;
    }
  }
}

void SWGL_FrameBuffer::CopyToRowPitch(int32_t* a_data)
{
  const int pitchX = sizeX;

  for (int tileId = 0; tileId < tiles.size(); tileId++)
  {
    int32_t* output = a_data + tiles[tileId].minY*pitchX + tiles[tileId].minX;

    for (int y = 0; y < BIN_SIZE; y++)
    {
      int32_t* inLine  = tiles[tileId].m_color + y*BIN_SIZE;
      int32_t* outLine = output + y*pitchX;

      for (int x = 0; x < BIN_SIZE; x+=8)
      {
        outLine[x + 0] = inLine[x + 0];
        outLine[x + 1] = inLine[x + 1];
        outLine[x + 2] = inLine[x + 2];
        outLine[x + 3] = inLine[x + 3];
        outLine[x + 4] = inLine[x + 4];
        outLine[x + 5] = inLine[x + 5];
        outLine[x + 6] = inLine[x + 6];
        outLine[x + 7] = inLine[x + 7];
      }
    }

  }

}
