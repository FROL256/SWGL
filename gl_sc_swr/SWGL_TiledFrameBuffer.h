#pragma once

#include "config.h"
#include <cstdint>
#include <cassert>
#include <vector>

#include "alloc16.h"
#include "LiteMath.h"

 /**
  \brief Frame Buffer
  \param PackedColor    -- uint32_t, uint16_t, uint8_t
  \param FB_BIN_SIZE    -- bin size; if 0, bins are not used and the framebuffer become 1-lvl!
  \param FB_TILE_SIZE_X -- small tile size at x axis
  \param FB_TILE_SIZE_Y -- small tile size at y axis
*/

template<typename PackedColor, int FB_BIN_SIZE, int FB_TILE_SIZE_X, int FB_TILE_SIZE_Y>
struct FrameBufferTwoLvl
{
  using ColorType = PackedColor;

  void Resize(int a_x, int a_y);
  void CopyToPitchLinear(uint32_t* a_data, int a_pitch);  

  void Clear(uint32_t a_color, float a_depth);

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
 
  inline int TileOffset(int x, int y);

};

using FrameBufferType = FrameBufferTwoLvl<uint32_t,64,4,4>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename PackedColor, int FB_BIN_SIZE, int FB_TILE_SIZE_X, int FB_TILE_SIZE_Y>
inline int FrameBufferTwoLvl<PackedColor, FB_BIN_SIZE, FB_TILE_SIZE_X, FB_TILE_SIZE_Y>::TileOffset(int x, int y)
{ 
  if(FB_BIN_SIZE != 0) // #static_if: assume compiler will opt this 
  {
    assert(x % FB_TILE_SIZE_X == 0);
    assert(y % FB_TILE_SIZE_Y == 0);
  
    const int by = y/FB_BIN_SIZE;     
    const int bx = x/FB_BIN_SIZE;     
  
    const int y0 = y%FB_BIN_SIZE;     
    const int x0 = x%FB_BIN_SIZE;     
  
    const int tx = x0/FB_TILE_SIZE_X; 
    const int ty = y0/FB_TILE_SIZE_Y; 
  
    const int offToBin  = (by*m_binsX + bx)*(FB_BIN_SIZE*FB_BIN_SIZE);
    const int offToTile = (ty*TILES_IN_BIN_X + tx)*PIXS_IN_TILE;
      
    assert( (offToBin + offToTile) % 16 == 0);
    return offToBin + offToTile;
  }
  else
  {
    const int tx = x/FB_TILE_SIZE_X; 
    const int ty = y/FB_TILE_SIZE_Y; 
    return  (ty*m_tilesTotalX + tx)*PIXS_IN_TILE; 
  }
}


template<typename PackedColor, int FB_BIN_SIZE, int FB_TILE_SIZE_X, int FB_TILE_SIZE_Y>
void FrameBufferTwoLvl<PackedColor,FB_BIN_SIZE, FB_TILE_SIZE_X, FB_TILE_SIZE_Y>::Resize(int a_x, int a_y)
{
  m_width  = a_x;
  m_height = a_y;
  
  if(FB_BIN_SIZE != 0)
  {
    assert(a_x % FB_BIN_SIZE == 0);
    assert(a_y % FB_BIN_SIZE == 0);
  
    m_binsX = a_x/FB_BIN_SIZE;
    m_binsY = a_y/FB_BIN_SIZE;
  }
  else 
  {
    m_binsX = 0;
    m_binsY = 0;
  }

  m_tilesTotalX = a_x/FB_TILE_SIZE_X;
  m_tilesTotalY = a_y/FB_TILE_SIZE_Y;

  m_depth.resize(a_x*a_y);
  m_color.resize(a_x*a_y);
}

inline static bool IsAligned(const void * ptr, std::uintptr_t alignment) noexcept 
{
  auto iptr = reinterpret_cast<std::uintptr_t>(ptr);
  return !(iptr % alignment);
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