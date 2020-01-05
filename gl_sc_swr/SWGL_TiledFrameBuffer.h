#pragma once

#include "config.h"
#include <cstdint>
#include <cassert>
#include <vector>

#include "alloc16.h"
#include "LiteMath.h"

template<typename PackedColor, int FB_BIN_SIZE, int FB_TILE_SIZE_X, int FB_TILE_SIZE_Y>
struct FrameBufferTwoLvl
{
  using ColorType = PackedColor;

  void Resize(int a_x, int a_y);
  void CopyToPitchLinear(uint32_t* a_data, int a_pitch);  

  void Clear(uint32_t a_color, float a_depth);

  void TestClearCheckerBoard();

  inline PackedColor* TileColor(int x, int y) { return m_color.data() + TileOffset(x,y); }
  inline float*       TileDepth(int x, int y) { return m_depth.data() + TileOffset(x,y); }

  inline PackedColor* PixelColor(int x, int y) { return m_color.data() + PixelOffset(x,y); }
  inline float*       PixelDepth(int x, int y) { return m_depth.data() + PixelOffset(x,y); }
  

private:
  
  constexpr static int TILES_IN_BIN_X = FB_BIN_SIZE/FB_TILE_SIZE_X; 
  constexpr static int TILES_IN_BIN_Y = FB_BIN_SIZE/FB_TILE_SIZE_Y;  
  constexpr static int PIXS_IN_TILE   = FB_TILE_SIZE_X*FB_TILE_SIZE_Y;
  constexpr static int TILES_IN_BIN   = TILES_IN_BIN_X*TILES_IN_BIN_Y;

  constexpr static int ALIGN_OF_TILE  = sizeof(PackedColor)*(FB_TILE_SIZE_X*FB_TILE_SIZE_Y);

  std::vector<cvex::vint4, aligned<cvex::vint4, 16> >         binsMinMax;

  std::vector<float,    aligned<float, 64> >                  m_depth;
  std::vector<uint32_t, aligned<PackedColor, ALIGN_OF_TILE> > m_color;

  int m_binsX;
  int m_binsY;

  void FillBinColor(int bx, int by, uint32_t color);
 
  inline int TileOffset(int x, int y)
  {
    const int by = y/FB_BIN_SIZE;     
    const int bx = x/FB_BIN_SIZE;     

    const int y0 = y%FB_BIN_SIZE;     
    const int x0 = x%FB_BIN_SIZE;     

    const int ty = y0/TILES_IN_BIN_Y; 
    const int tx = x0/TILES_IN_BIN_X; 

    const int offToBin  = (by*m_binsX + bx)*(FB_BIN_SIZE*FB_BIN_SIZE);
    const int offToTile = (ty*TILES_IN_BIN_X + ty)*PIXS_IN_TILE;
    
    assert( (offToBin + offToTile) % 16 == 0);

    return offToBin + offToTile;
  }

};

using FrameBufferType = FrameBufferTwoLvl<uint32_t,64,4,4>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline static bool IsAligned(const void * ptr, std::uintptr_t alignment) noexcept 
{
  auto iptr = reinterpret_cast<std::uintptr_t>(ptr);
  return !(iptr % alignment);
}

template<typename PackedColor, int FB_BIN_SIZE, int FB_TILE_SIZE_X, int FB_TILE_SIZE_Y>
void FrameBufferTwoLvl<PackedColor,FB_BIN_SIZE, FB_TILE_SIZE_X, FB_TILE_SIZE_Y>::Resize(int a_x, int a_y)
{
  assert(a_x % FB_BIN_SIZE == 0);
  assert(a_y % FB_BIN_SIZE == 0);

  m_binsX = a_x/FB_BIN_SIZE;
  m_binsY = a_y/FB_BIN_SIZE;

  m_depth.resize(a_x*a_y);
  m_color.resize(a_x*a_y);
  binsMinMax.resize(m_binsX*m_binsY);
  for(int binY = 0; binY < m_binsY; binY++)
  {
    const int binOffsetY = m_binsX*binY;
    for(int binX = 0; binX < m_binsX; binX++)
      binsMinMax[binOffsetY + binX] = cvex::vint4{binX*FB_BIN_SIZE,             binY*FB_BIN_SIZE, 
                                                  binX*FB_BIN_SIZE+FB_BIN_SIZE, binY*FB_BIN_SIZE+FB_BIN_SIZE};
  }

}

template<typename PackedColor, int FB_BIN_SIZE, int FB_TILE_SIZE_X, int FB_TILE_SIZE_Y>
void FrameBufferTwoLvl<PackedColor,FB_BIN_SIZE, FB_TILE_SIZE_X, FB_TILE_SIZE_Y>::FillBinColor(int bx, int by, uint32_t a_color)
{
  const int offsetToBin = (by*m_binsX + bx)*(FB_BIN_SIZE*FB_BIN_SIZE);
  uint32_t* color = m_color.data() + offsetToBin;
  
  assert(IsAligned(color,64));

  const cvex::vuint4 res = cvex::splat(a_color);
  
  const uint32_t palette[20] = { 0xff06190b, 0xff3c044b, 0xff0fe109, 0xff0082c0,
                                 0xff050201, 0xff011eb4, 0xff46f000, 0xfff002e6,
                                 0xff02f53c, 0xff0a0ebe, 0xff008000, 0xff06beff,
                                 0xff0a0e08, 0xff0f0ac8, 0xff800000, 0xffaa0fc3,
                                 0xff800000, 0xf0ffd0b1, 0xff000080, 0xff008000 };

  for(int i=0; i<TILES_IN_BIN; i++)
  {
    const cvex::vuint4 tileColor = res & cvex::splat(palette[i%20]);

    cvex::store(color + i*PIXS_IN_TILE + 0,  tileColor);
    cvex::store(color + i*PIXS_IN_TILE + 4,  tileColor);
    cvex::store(color + i*PIXS_IN_TILE + 8,  tileColor);
    cvex::store(color + i*PIXS_IN_TILE + 12, tileColor);
  }

}

template<typename PackedColor, int FB_BIN_SIZE, int FB_TILE_SIZE_X, int FB_TILE_SIZE_Y>
void FrameBufferTwoLvl<PackedColor,FB_BIN_SIZE, FB_TILE_SIZE_X, FB_TILE_SIZE_Y>::TestClearCheckerBoard()
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

template<typename PackedColor, int FB_BIN_SIZE, int FB_TILE_SIZE_X, int FB_TILE_SIZE_Y>
void FrameBufferTwoLvl<PackedColor,FB_BIN_SIZE, FB_TILE_SIZE_X, FB_TILE_SIZE_Y>::CopyToPitchLinear(uint32_t* a_data, int a_pitch)
{
  const int binsTotal = m_binsY*m_binsX;

  //#pragma omp parallel for
  for (int binId = 0; binId < binsTotal; binId++)
  {
    const int offsetToBin       = binId*(FB_BIN_SIZE*FB_BIN_SIZE);
    const uint32_t* tilecolor   = m_color.data() + offsetToBin;
    const cvex::vint4 bMinMax   = binsMinMax[binId];

    for(int tileY = 0; tileY < TILES_IN_BIN_Y; tileY++)
    {
      const int y  = cvex::extract_1(bMinMax) + tileY*FB_TILE_SIZE_Y;
      const int x0 = cvex::extract_0(bMinMax);
      const int tilePitchYOffset = tileY*TILES_IN_BIN_X;

      for(int tileX = 0; tileX < TILES_IN_BIN_X; tileX++)
      {
        const int tilePitchOffset   = (tilePitchYOffset + tileX)*PIXS_IN_TILE;

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

template<typename PackedColor, int FB_BIN_SIZE, int FB_TILE_SIZE_X, int FB_TILE_SIZE_Y>
void FrameBufferTwoLvl<PackedColor,FB_BIN_SIZE, FB_TILE_SIZE_X, FB_TILE_SIZE_Y>::Clear(uint32_t a_color, float a_depth)
{
  memset(m_color.data(), 0, m_color.size()*sizeof(PackedColor));
}