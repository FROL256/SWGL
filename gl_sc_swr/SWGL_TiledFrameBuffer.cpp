#include "SWGL_TiledFrameBuffer.h"
#include "HWAbstractionLayer.h"

#include <memory.h>
#include <cassert>


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline bool IsAligned(const void * ptr, std::uintptr_t alignment) noexcept 
{
  auto iptr = reinterpret_cast<std::uintptr_t>(ptr);
  return !(iptr % alignment);
}

constexpr int TILES_IN_BIN_X = BIN_SIZE/FB_TILE_SIZE_X; 
constexpr int TILES_IN_BIN_Y = BIN_SIZE/FB_TILE_SIZE_Y; 
constexpr int TILES_IN_BIN   = TILES_IN_BIN_X*TILES_IN_BIN_Y;

void SWGL_FrameBufferTwoLvl::Resize(int a_x, int a_y)
{
  assert(a_x % BIN_SIZE == 0);
  assert(a_y % BIN_SIZE == 0);

  m_binsX = a_x/BIN_SIZE;
  m_binsY = a_y/BIN_SIZE;

  m_depth.resize(a_x*a_y);
  m_color.resize(a_x*a_y);
  binsMinMax.resize(m_binsX*m_binsY);
  for(int binY = 0; binY < m_binsY; binY++)
  {
    const int binOffsetY = m_binsX*binY;
    for(int binX = 0; binX < m_binsX; binX++)
      binsMinMax[binOffsetY + binX] = cvex::vint4{binX*BIN_SIZE,          binY*BIN_SIZE, 
                                                  binX*BIN_SIZE+BIN_SIZE, binY*BIN_SIZE+BIN_SIZE};
  }

}

void SWGL_FrameBufferTwoLvl::FillBinColor(int bx, int by, uint32_t a_color)
{
  const int offsetToBin = (by*m_binsX + bx)*(BIN_SIZE*BIN_SIZE);
  uint32_t* color = m_color.data() + offsetToBin;
  
  assert(IsAligned(color,64));

  //memset(color, 255, sizeof(uint32_t)*BIN_SIZE*BIN_SIZE);

  const cvex::vuint4 res = cvex::splat(a_color);
  
  for(int i=0; i<TILES_IN_BIN; i++)
  {
    cvex::store(color + i*FB_TILE_SIZE + 0,  res);
    cvex::store(color + i*FB_TILE_SIZE + 4,  res);
    cvex::store(color + i*FB_TILE_SIZE + 8,  res);
    cvex::store(color + i*FB_TILE_SIZE + 12, res);
  }

}

void SWGL_FrameBufferTwoLvl::TestClearChessBoard()
{
  const uint32_t defaultpalette[20] = { 0xffe6194b, 0xff3cb44b, 0xffffe119, 0xff0082c8,
                                        0xfff58231, 0xff911eb4, 0xff46f0f0, 0xfff032e6,
                                        0xffd2f53c, 0xfffabebe, 0xff008080, 0xffe6beff,
                                        0xffaa6e28, 0xfffffac8, 0xff800000, 0xffaaffc3,
                                        0xff808000, 0xffffd8b1, 0xff000080, 0xff808080 };
  
  int counter = 0;
  for(int binY = 0; binY < m_binsY; binY++)
  {
    for(int binX = 0; binX < m_binsX; binX++)
    {
      FillBinColor(binX, binY, defaultpalette[counter]);
      counter++;
      if(counter >= 20)
        counter = 0;
    }
  }
}

void SWGL_FrameBufferTwoLvl::CopyToPitchLinear(uint32_t* a_data, int a_pitch)
{
  const int binsTotal = m_binsY*m_binsX;

  //#pragma omp parallel for
  for (int binId = 0; binId < binsTotal; binId++)
  {
    const int offsetToBin       = binId*(BIN_SIZE*BIN_SIZE);
    const uint32_t* tilecolor   = m_color.data() + offsetToBin;
    const cvex::vint4 bMinMax   = binsMinMax[binId];

    for(int tileY = 0; tileY < TILES_IN_BIN_Y; tileY++)
    {
      const int y  = cvex::extract_1(bMinMax) + tileY*FB_TILE_SIZE_Y;
      const int x0 = cvex::extract_0(bMinMax);
      const int tilePitchYOffset = tileY*TILES_IN_BIN_X;

      for(int tileX = 0; tileX < TILES_IN_BIN_X; tileX++)
      {
        const int tilePitchOffset   = (tilePitchYOffset + tileX)*FB_TILE_SIZE;

        const cvex::vuint4 tileRow0 = cvex::load(tilecolor + tilePitchOffset + 0);
        const cvex::vuint4 tileRow1 = cvex::load(tilecolor + tilePitchOffset + 4);
        const cvex::vuint4 tileRow2 = cvex::load(tilecolor + tilePitchOffset + 8);
        const cvex::vuint4 tileRow3 = cvex::load(tilecolor + tilePitchOffset + 12);
        
        const int x = x0 + tileX*FB_TILE_SIZE_X;

        cvex::store(a_data + (y + 0)*a_pitch + x, tileRow0); 
        cvex::store(a_data + (y + 1)*a_pitch + x, tileRow1); 
        cvex::store(a_data + (y + 2)*a_pitch + x, tileRow2); 
        cvex::store(a_data + (y + 3)*a_pitch + x, tileRow3); 

        //cvex::stream(a_data + (y + 0)*a_pitch + x, tileRow0); 
        //cvex::stream(a_data + (y + 1)*a_pitch + x, tileRow1); 
        //cvex::stream(a_data + (y + 2)*a_pitch + x, tileRow2); 
        //cvex::stream(a_data + (y + 3)*a_pitch + x, tileRow3); 
      }
    }

  }
}
