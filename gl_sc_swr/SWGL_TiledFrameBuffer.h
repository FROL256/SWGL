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

private:
  
  constexpr static int TILES_IN_BIN_X = FB_BIN_SIZE/FB_TILE_SIZE_X; 
  constexpr static int TILES_IN_BIN_Y = FB_BIN_SIZE/FB_TILE_SIZE_Y;  
  constexpr static int PIXS_IN_TILE   = FB_TILE_SIZE_X*FB_TILE_SIZE_Y;
  constexpr static int TILES_IN_BIN   = TILES_IN_BIN_X*TILES_IN_BIN_Y;

  constexpr static int ALIGN_OF_TILE  = sizeof(PackedColor)*(FB_TILE_SIZE_X*FB_TILE_SIZE_Y);

  std::vector<float,    aligned<float, 64> >                  m_depth;
  std::vector<uint32_t, aligned<PackedColor, ALIGN_OF_TILE> > m_color;

  int m_width;
  int m_height;
  int m_binsX;
  int m_binsY;

  int m_tilesTotalX;
  int m_tilesTotalY;

  void FillBinColor(int bx, int by, uint32_t color);
 
  inline int TileOffset(int x, int y)
  { 
    /*
    assert(x % FB_TILE_SIZE_X == 0);
    assert(y % FB_TILE_SIZE_Y == 0);

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
    */

    //const int tx = x/FB_TILE_SIZE_X; 
    //const int ty = y/FB_TILE_SIZE_Y; 
    //return (ty*m_tilesTotalX + ty)*PIXS_IN_TILE;

    const int offsetReal = m_width*y + x;
    return (offsetReal/PIXS_IN_TILE)*PIXS_IN_TILE;
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
  m_width  = a_x;
  m_height = a_y;

  assert(a_x % FB_BIN_SIZE == 0);
  assert(a_y % FB_BIN_SIZE == 0);

  m_binsX = a_x/FB_BIN_SIZE;
  m_binsY = a_y/FB_BIN_SIZE;

  m_tilesTotalX = a_x/FB_TILE_SIZE_X;
  m_tilesTotalY = a_y/FB_TILE_SIZE_Y;

  m_depth.resize(a_x*a_y);
  m_color.resize(a_x*a_y);
}

template<typename PackedColor, int FB_BIN_SIZE, int FB_TILE_SIZE_X, int FB_TILE_SIZE_Y>
void FrameBufferTwoLvl<PackedColor,FB_BIN_SIZE, FB_TILE_SIZE_X, FB_TILE_SIZE_Y>::FillBinColor(int bx, int by, uint32_t a_color)
{
  const int offsetToBin = (by*m_binsX + bx)*(FB_BIN_SIZE*FB_BIN_SIZE);
  uint32_t* color = m_color.data() + offsetToBin;
  
  assert(IsAligned(color,64));

  const cvex::vuint4 res = cvex::splat(a_color);
  
  //std::vector<uint32_t> pallette(TILES_IN_BIN);
  //for(int i=0;i<pallette.size();i++)
  //  pallette[i] = LiteMath::rnd(0.0f, 1.0f) < 0.5f ? 0 : 0xFFFFFFFF;

  for(int i=0; i<TILES_IN_BIN; i++)
  {
    const cvex::vuint4 tileColor = cvex::splat(a_color);

    cvex::store(color + i*PIXS_IN_TILE + 0,  tileColor);
    cvex::store(color + i*PIXS_IN_TILE + 4,  tileColor);
    cvex::store(color + i*PIXS_IN_TILE + 8,  tileColor);
    cvex::store(color + i*PIXS_IN_TILE + 12, tileColor);
  }

}

template<typename PackedColor, int FB_BIN_SIZE, int FB_TILE_SIZE_X, int FB_TILE_SIZE_Y>
void FrameBufferTwoLvl<PackedColor,FB_BIN_SIZE, FB_TILE_SIZE_X, FB_TILE_SIZE_Y>::TestClearCheckerBoard()
{
  const uint32_t defaultpalette[19] = { 0xffe6194b, 0xff3cb44b, 0xffffe119, 0xff0082c8,
                                        0xfff58231, 0xff911eb4, 0xff46f0f0, 0xfff032e6,
                                        0xffd2f53c, 0xfffabebe, 0xff008080, 0xffe6beff,
                                        0xffaa6e28, 0xfffffac8, 0xff800000, 0xffaaffc3,
                                        0xff808000, 0xffffd8b1, 0xff808080 };
  
  /*
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
  }*/
  
  uint32_t* color = m_color.data();

  int counter = 0;
  for(int y=0; y<m_height; y+= FB_TILE_SIZE_Y)
  {
    for(int x=0; x<m_width; x += FB_TILE_SIZE_X)
    {
      const PackedColor* pColor    = TileColor(x,y);
      const cvex::vuint4 tileColor = cvex::splat( defaultpalette[counter] );
    
      cvex::store(pColor + 0,  tileColor);
      cvex::store(pColor + 4,  tileColor);
      cvex::store(pColor + 8,  tileColor);
      cvex::store(pColor + 12, tileColor);

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
  
  for(int y=0; y<m_height; y+= FB_TILE_SIZE_Y)
  {
    for(int x=0; x<m_width; x += FB_TILE_SIZE_X)
    {
      const PackedColor* tilecolor = TileColor(x,y);

      const cvex::vuint4 tileRow0 = cvex::load(tilecolor + 0);
      const cvex::vuint4 tileRow1 = cvex::load(tilecolor + 4);
      const cvex::vuint4 tileRow2 = cvex::load(tilecolor + 8);
      const cvex::vuint4 tileRow3 = cvex::load(tilecolor + 12);

      cvex::store(a_data + (y + 0)*a_pitch + x, tileRow0); 
      cvex::store(a_data + (y + 1)*a_pitch + x, tileRow1); 
      cvex::store(a_data + (y + 2)*a_pitch + x, tileRow2); 
      cvex::store(a_data + (y + 3)*a_pitch + x, tileRow3); 
    }
  }

}

template<typename PackedColor, int FB_BIN_SIZE, int FB_TILE_SIZE_X, int FB_TILE_SIZE_Y>
void FrameBufferTwoLvl<PackedColor,FB_BIN_SIZE, FB_TILE_SIZE_X, FB_TILE_SIZE_Y>::Clear(uint32_t a_color, float a_depth)
{
  memset(m_color.data(), 0, m_color.size()*sizeof(PackedColor));
}