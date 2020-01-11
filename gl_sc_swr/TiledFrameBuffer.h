#pragma once

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
  void CopyToPitchLinear(uint32_t* a_data, int a_pitch, bool invertY = true);  

  void ClearColor        (uint32_t a_color);
  void ClearColorAndDepth(uint32_t a_color, float a_depth);

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
void FrameBufferTwoLvl<PackedColor,FB_BIN_SIZE, FB_TILE_SIZE_X, FB_TILE_SIZE_Y>::CopyToPitchLinear(uint32_t* a_data, int a_pitch, bool invertY = true)
{
  const int binsTotal = m_binsY*m_binsX;
  
  if(invertY)
  {
    for(int y=0; y<m_height; y+= FB_TILE_SIZE_Y)
    {
      for(int x=0; x<m_width; x += FB_TILE_SIZE_X)
      {
        const PackedColor* tilecolor = TileColor(x,y);
  
        const cvex::vuint4 tileRow0 = cvex::load(tilecolor + 0);
        const cvex::vuint4 tileRow1 = cvex::load(tilecolor + 4);
        const cvex::vuint4 tileRow2 = cvex::load(tilecolor + 8);
        const cvex::vuint4 tileRow3 = cvex::load(tilecolor + 12);
  
        cvex::store(a_data + (m_height - (y + 0) - 1)*a_pitch + x, tileRow0); 
        cvex::store(a_data + (m_height - (y + 1) - 1)*a_pitch + x, tileRow1); 
        cvex::store(a_data + (m_height - (y + 2) - 1)*a_pitch + x, tileRow2); 
        cvex::store(a_data + (m_height - (y + 3) - 1)*a_pitch + x, tileRow3); 
      }
    }
  }
  else
  {
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
}

template<typename PackedColor, int FB_BIN_SIZE, int FB_TILE_SIZE_X, int FB_TILE_SIZE_Y>
void FrameBufferTwoLvl<PackedColor,FB_BIN_SIZE, FB_TILE_SIZE_X, FB_TILE_SIZE_Y>::ClearColor(uint32_t a_color)
{
  if(a_color == 0)
  {
    memset(m_color.data(), 0, m_color.size()*sizeof(PackedColor));
    return;
  }

  const cvex::vuint4 vcolor = cvex::splat(a_color);
  const int size = (m_width*m_height);

  for(int i=0; i<size; i+=8)
  {
    cvex::store(m_color.data() + i + 0, vcolor);
    cvex::store(m_color.data() + i + 4, vcolor);
  }
}

template<typename PackedColor, int FB_BIN_SIZE, int FB_TILE_SIZE_X, int FB_TILE_SIZE_Y>
void FrameBufferTwoLvl<PackedColor,FB_BIN_SIZE, FB_TILE_SIZE_X, FB_TILE_SIZE_Y>::ClearColorAndDepth(uint32_t a_color, float a_depth)
{
  if(m_color.size() == 0)
    return;

  const cvex::vuint4  vcolor = cvex::splat(a_color);
  const cvex::vfloat4 vdepth = cvex::splat(a_depth);

  const int size = (m_width*m_height);

  //#pragma omp parallel for
  for(int i=0; i<size; i+=8)
  {
    cvex::store(m_color.data() + i + 0, vcolor);
    cvex::store(m_color.data() + i + 4, vcolor);

    cvex::store(m_depth.data() + i + 0, vdepth);
    cvex::store(m_depth.data() + i + 4, vdepth);
  }
  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace FB
{

template<typename SrcType, typename DstType>
static inline DstType ColorPack(const SrcType r, const SrcType g, const SrcType b, const SrcType a)
{
  return DstType(0);
}

template<typename SrcType, typename DstType>
static inline DstType ColorPack(const SrcType r, const SrcType g, const SrcType b)
{
  return DstType(0);
}


template<typename SrcType, typename DstType>
static inline void ColorUNPack(const DstType colorOld, SrcType& r, SrcType& g, SrcType& b, SrcType& a)
{
  
}

//////////////////////////////////////////////////////////////////////////////////////// scalar
//////////////////////////////////////////////////////////////////////////////////////// float to uint32_t

template<> 
inline uint32_t ColorPack<float,uint32_t>(const float r, const float g, const float b, const float a)
{
  constexpr float c_255 = 255.0f;
  return (uint32_t(r * c_255) << 16) | // BGRA
         (uint32_t(g * c_255) << 8)  |
         (uint32_t(b * c_255) << 0)  |
         (uint32_t(a * c_255) << 24);
}


template<> 
inline uint32_t ColorPack<float,uint32_t>(const float r, const float g, const float b)
{
  constexpr float c_255 = 255.0f;
  return (uint32_t(r * c_255) << 16) | // BGRA
         (uint32_t(g * c_255) << 8)  |
         (uint32_t(b * c_255) << 0);
}



template<>
inline void ColorUNPack<float,uint32_t>(const uint32_t colorOld, 
                                        float& r, float& g, float& b, float& a)
{
  constexpr float c_255Inv = 1.0f/255.0f;
  r = float( (colorOld & 0x00FF0000) >> 16)*c_255Inv;
  g = float( (colorOld & 0x0000FF00) >> 8 )*c_255Inv;
  b = float( (colorOld & 0x000000FF) >> 0 )*c_255Inv;
  a = float( (colorOld & 0xFF000000) >> 24)*c_255Inv;
}

//////////////////////////////////////////////////////////////////////////////////////// vector4
//////////////////////////////////////////////////////////////////////////////////////// float to uint32_t

template<> 
inline cvex::vuint4 ColorPack<cvex::vfloat4,cvex::vuint4>(const cvex::vfloat4 r, const cvex::vfloat4 g, const cvex::vfloat4 b, const cvex::vfloat4 a)
{
  const cvex::vfloat4 c_255 = cvex::splat(255.0f);
  return (cvex::to_uint32(r * c_255) << 16) | // BGRA
         (cvex::to_uint32(g * c_255) << 8)  |
         (cvex::to_uint32(b * c_255) << 0)  |
         (cvex::to_uint32(a * c_255) << 24);
}

template<> 
inline cvex::vuint4 ColorPack<cvex::vfloat4,cvex::vuint4>(const cvex::vfloat4 r, const cvex::vfloat4 g, const cvex::vfloat4 b)
{
  const cvex::vfloat4 c_255 = cvex::splat(255.0f);
  return (cvex::to_uint32(r * c_255) << 16) | // BGRA
         (cvex::to_uint32(g * c_255) << 8)  |
         (cvex::to_uint32(b * c_255) << 0);
}

template<>
inline void ColorUNPack<cvex::vfloat4,cvex::vuint4>(const cvex::vuint4 colorOld, 
                                                    cvex::vfloat4& r, cvex::vfloat4& g, cvex::vfloat4& b, cvex::vfloat4& a)
{
  const cvex::vfloat4 c_255Inv = cvex::splat(1.0f/255.0f);
  r = cvex::to_float32( (colorOld & 0x00FF0000) >> 16)*c_255Inv;
  g = cvex::to_float32( (colorOld & 0x0000FF00) >> 8 )*c_255Inv;
  b = cvex::to_float32( (colorOld & 0x000000FF) >> 0 )*c_255Inv;
  a = cvex::to_float32( (colorOld & 0xFF000000) >> 24)*c_255Inv;
}

//////////////////////////////////////////////////////////////////////////////////////// vector8
//////////////////////////////////////////////////////////////////////////////////////// float to uint32_t

#ifndef WIN32

template<> 
inline cvex8::vuint8 ColorPack<cvex8::vfloat8,cvex8::vuint8>(const cvex8::vfloat8 r, const cvex8::vfloat8 g, const cvex8::vfloat8 b, const cvex8::vfloat8 a)
{
  const cvex8::vfloat8 c_255 = cvex8::splat(255.0f);
  return (cvex8::to_uint32(r * c_255) << 16) | // BGRA
         (cvex8::to_uint32(g * c_255) << 8)  |
         (cvex8::to_uint32(b * c_255) << 0)  |
         (cvex8::to_uint32(a * c_255) << 24);
}


template<> 
inline cvex8::vuint8 ColorPack<cvex8::vfloat8,cvex8::vuint8>(const cvex8::vfloat8 r, const cvex8::vfloat8 g, const cvex8::vfloat8 b)
{
  const cvex8::vfloat8 c_255 = cvex8::splat(255.0f);
  return (cvex8::to_uint32(r * c_255) << 16) | // BGRA
         (cvex8::to_uint32(g * c_255) << 8)  |
         (cvex8::to_uint32(b * c_255) << 0);
}

template<>
inline void ColorUNPack<cvex8::vfloat8,cvex8::vuint8>(const cvex8::vuint8 colorOld, 
                                                      cvex8::vfloat8& r, cvex8::vfloat8& g, cvex8::vfloat8& b, cvex8::vfloat8& a)
{
  const cvex8::vfloat8 c_255Inv = cvex8::splat(1.0f/255.0f);
  r = cvex8::to_float32( (colorOld & 0x00FF0000) >> 16)*c_255Inv;
  g = cvex8::to_float32( (colorOld & 0x0000FF00) >> 8 )*c_255Inv;
  b = cvex8::to_float32( (colorOld & 0x000000FF) >> 0 )*c_255Inv;
  a = cvex8::to_float32( (colorOld & 0xFF000000) >> 24)*c_255Inv;
}


//////////////////////////////////////////////////////////////////////////////////////// vector16
//////////////////////////////////////////////////////////////////////////////////////// float to uint32_t


template<> 
inline cvex16::vuint16 ColorPack<cvex16::vfloat16,cvex16::vuint16>(const cvex16::vfloat16 r, const cvex16::vfloat16 g, const cvex16::vfloat16 b, const cvex16::vfloat16 a)
{
  const cvex16::vfloat16 c_255 = cvex16::splat(255.0f);
  return (cvex16::to_uint32(r * c_255) << 16) | // BGRA
         (cvex16::to_uint32(g * c_255) << 8)  |
         (cvex16::to_uint32(b * c_255) << 0)  |
         (cvex16::to_uint32(a * c_255) << 24);
}

template<> 
inline cvex16::vuint16 ColorPack<cvex16::vfloat16,cvex16::vuint16>(const cvex16::vfloat16 r, const cvex16::vfloat16 g, const cvex16::vfloat16 b)
{
  const cvex16::vfloat16 c_255 = cvex16::splat(255.0f);
  return (cvex16::to_uint32(r * c_255) << 16) | // BGRA
         (cvex16::to_uint32(g * c_255) << 8)  |
         (cvex16::to_uint32(b * c_255) << 0);
}


template<>
inline void ColorUNPack<cvex16::vfloat16,cvex16::vuint16>(const cvex16::vuint16 colorOld, 
                                                          cvex16::vfloat16& r, cvex16::vfloat16& g, cvex16::vfloat16& b, cvex16::vfloat16& a)
{
  const cvex16::vfloat16 c_255Inv = cvex16::splat(1.0f/255.0f);
  r = cvex16::to_float32( (colorOld & 0x00FF0000) >> 16)*c_255Inv;
  g = cvex16::to_float32( (colorOld & 0x0000FF00) >> 8 )*c_255Inv;
  b = cvex16::to_float32( (colorOld & 0x000000FF) >> 0 )*c_255Inv;
  a = cvex16::to_float32( (colorOld & 0xFF000000) >> 24)*c_255Inv;
}

#endif



};