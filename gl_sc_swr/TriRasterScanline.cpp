#include "TriRaster.h"
#include "TexSampler.h"

#include <cstdint>
#include <memory.h>

#ifdef ENABLE_SSE_FOR_RASTER

typedef __m128 Color;

#include <xmmintrin.h> // SSE
#include <emmintrin.h> // SSE2
#include <smmintrin.h> // SSE4.1

static inline __m128 operator+(__m128 a, __m128 b) { return _mm_add_ps(a,b); }
static inline __m128 operator-(__m128 a, __m128 b) { return _mm_sub_ps(a, b); }

static inline __m128 operator*(__m128 a, __m128 b) { return _mm_mul_ps(a, b); }
static inline __m128 operator*(__m128 a, float b)  { return _mm_mul_ps(a, _mm_set_ps(b,b,b,b)); }
static inline __m128 operator*(float b, __m128 a)  { return _mm_mul_ps(a, _mm_set_ps(b,b,b,b)); }

#else

  typedef float4 Color;

#endif


class Edge
{
public:
  Color Color1, Color2;
  int X1, Y1, X2, Y2;

  Edge(const Color &color1, int x1, int y1, const Color &color2, int x2, int y2);
};

class Span
{
public:
  Color Color1, Color2;
  int X1, X2;

  Span(const Color &color1, int x1, const Color &color2, int x2);
};

enum DRAW_SPAN_TYPE{ DRAW_SPAN_COLORED  = 0, DRAW_SPAN_COLORED_2D  = 1,
                     DRAW_SPAN_TEXTURED = 2, DRAW_SPAN_TEXTURED_2D = 3,
};

class Rasterizer // http://joshbeam.com/articles/triangle_rasterization/
{
protected:
  uint32_t* m_FrameBuffer;
  float*    m_ZBuffer;
  uint8_t*  m_SBuffer;
  unsigned int m_Width, m_Height;


  template<int draw_type>
  inline void SetPixel(unsigned int x, unsigned int y, const Color &color = Color());
  template<int draw_type>
  inline void DrawSpan(const Span &span, int y);
  inline void DrawSpansBetweenEdges(const Edge &e1, const Edge &e2);

public:

  void SetFrameBuffer(uint32_t *frameBuffer, unsigned int width, unsigned int height);
  void SetZBuffer(float* a_zbuffer)   { m_ZBuffer = a_zbuffer; }
  void SetSBuffer(uint8_t* a_zbuffer) { m_SBuffer = a_zbuffer; }

  inline void SetPixel(float x, float y, const Color &color = Color());

  void Clear();
  void DrawTriangle(const Color &color1, float x1, float y1, const Color &color2, float x2, float y2, const Color &color3, float x3, float y3);
  void DrawLine(const Color &color1, float x1, float y1, const Color &color2, float x2, float y2);

  const Triangle* pTri;
  TexSampler      sampler;

  uint8_t sval;
  uint8_t smask;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Edge::Edge(const Color &color1, int x1, int y1,
           const Color &color2, int x2, int y2)
{
	if(y1 < y2)
  {
		Color1 = color1;
    Color2 = color2;
		X1     = x1;
		Y1     = y1;
		X2     = x2;
		Y2     = y2;
	}
  else
  {
		Color1 = color2;
		Color2 = color1;
    X1     = x2;
    Y1     = y2;
    X2     = x1;
		Y2     = y1;
	}
}

Span::Span(const Color &color1, int x1, const Color &color2, int x2)
{
	if(x1 < x2)
  {
		Color1 = color1;
    Color2 = color2;
		X1     = x1;
		X2     = x2;
	}
  else
  {
		Color1 = color2;
    Color2 = color1;
		X1     = x2;
		X2     = x1;
	}
}

void Rasterizer::SetFrameBuffer(uint32_t *frameBuffer, unsigned int width, unsigned int height)
{
	m_FrameBuffer = frameBuffer;
	m_Width       = width;
	m_Height      = height;
}


void Rasterizer::Clear()
{
	memset(m_FrameBuffer, 0, sizeof(uint32_t) * m_Height * m_Width);
}

template<>
void Rasterizer::SetPixel<DRAW_SPAN_TEXTURED>(unsigned int x, unsigned int y, const Color &color)
{

#ifdef ENABLE_SSE_FOR_RASTER

  const int     offset    = y * m_Width + x;
  const float   zInZBuff  = m_ZBuffer[offset];
  const __m128  zInZBuffv = _mm_set_ps(zInZBuff, 0.0f, 0.0f, 0.0f);

  const __m128 cmpRes = _mm_and_ps(_mm_cmpge_ps(color, zInZBuffv), _mm_castsi128_ps(_mm_set_epi32(0xFFFFFFFF, 0, 0, 0)));

  if (_mm_movemask_ps(cmpRes) != 0)
  {
  #ifdef RASTER_FAST_SCANLINE_W0W1W2

    const __m128 w0   =  _mm_shuffle_ps(color, color, _MM_SHUFFLE(0, 0, 0, 0));
    const __m128 w1   =  _mm_shuffle_ps(color, color, _MM_SHUFFLE(1, 1, 1, 1));
    const __m128 w2   =  _mm_shuffle_ps(color, color, _MM_SHUFFLE(2, 2, 2, 2));
    const __m128 zInv =  _mm_shuffle_ps(color, color, _MM_SHUFFLE(3, 3, 3, 3));

    __m128 clr = _mm_add_ps(_mm_mul_ps(pTri->cv1, w0), _mm_add_ps(_mm_mul_ps(pTri->cv2, w1), _mm_mul_ps(pTri->cv3, w2)));

    const __m128 t1 = _mm_mul_ps(pTri->tv1, w0);
    const __m128 t2 = _mm_mul_ps(pTri->tv2, w1);
    const __m128 t3 = _mm_mul_ps(pTri->tv3, w2);
    __m128 tc       = _mm_add_ps(t1, _mm_add_ps(t2, t3));

    #ifdef PERSP_CORRECT
    {
      const __m128 z1 = _mm_rcp_ss(zInv);
      const __m128 z  = _mm_shuffle_ps(z1, z1, _MM_SHUFFLE(0, 0, 0, 0));

      clr = _mm_mul_ps(clr, z);
      tc  = _mm_mul_ps(tc, z);
    }
    #endif

    const __m128 texColor = tex2D_sse(this->sampler, tc); 

    m_FrameBuffer[offset] = RealColorToUint32_BGRA(_mm_mul_ps(clr, texColor));
    _mm_store_ss(m_ZBuffer + offset, zInv);

  #else
    m_FrameBuffer[offset] = RealColorToUint32_BGRA(color);
    _mm_store_ss(m_ZBuffer + offset, _mm_shuffle_ps(color, color, _MM_SHUFFLE(3, 3, 3, 3)));
  #endif
  }

#else

  const int offset = y * m_Width + x;
  const float zInv = color.w;

  if (zInv > m_ZBuffer[offset])
  {
  #ifdef RASTER_FAST_SCANLINE_W0W1W2
    const float w0 = color.x;
    const float w1 = color.y;
    const float w2 = color.z;
    float4 color2  = w0*pTri->c1 + w1*pTri->c2 + w2*pTri->c3;
    m_FrameBuffer[offset] = RealColorToUint32_BGRA(color2);
  #else
    m_FrameBuffer[offset] = RealColorToUint32_BGRA(color);
  #endif
    m_ZBuffer    [offset] = zInv;
  }

#endif

}


template<>
void Rasterizer::SetPixel<DRAW_SPAN_COLORED>(unsigned int x, unsigned int y, const Color &color)
{

#ifdef ENABLE_SSE_FOR_RASTER

  const int     offset    = y * m_Width + x;
  const float   zInZBuff  = m_ZBuffer[offset];
  const __m128  zInZBuffv = _mm_set_ps(zInZBuff, 0.0f, 0.0f, 0.0f);

  const __m128 cmpRes = _mm_and_ps(_mm_cmpge_ps(color, zInZBuffv), _mm_castsi128_ps(_mm_set_epi32(0xFFFFFFFF, 0, 0, 0)));

  if (_mm_movemask_ps(cmpRes) != 0)
  {
  #ifdef RASTER_FAST_SCANLINE_W0W1W2

    const __m128 w0   =  _mm_shuffle_ps(color, color, _MM_SHUFFLE(0, 0, 0, 0));
    const __m128 w1   =  _mm_shuffle_ps(color, color, _MM_SHUFFLE(1, 1, 1, 1));
    const __m128 w2   =  _mm_shuffle_ps(color, color, _MM_SHUFFLE(2, 2, 2, 2));
    const __m128 zInv =  _mm_shuffle_ps(color, color, _MM_SHUFFLE(3, 3, 3, 3));

    __m128 clr = _mm_add_ps(_mm_mul_ps(pTri->cv1, w0), _mm_add_ps(_mm_mul_ps(pTri->cv2, w1), _mm_mul_ps(pTri->cv3, w2)));

    #ifdef PERSP_CORRECT
    {
      const __m128 z1 = _mm_rcp_ss(zInv);
      const __m128 z  = _mm_shuffle_ps(z1, z1, _MM_SHUFFLE(0, 0, 0, 0));

      clr = _mm_mul_ps(clr, z);
    }
    #endif

    m_FrameBuffer[offset] = RealColorToUint32_BGRA(clr);
    _mm_store_ss(m_ZBuffer + offset, zInv);

  #else
    m_FrameBuffer[offset] = RealColorToUint32_BGRA(color);
    _mm_store_ss(m_ZBuffer + offset, _mm_shuffle_ps(color, color, _MM_SHUFFLE(3, 3, 3, 3)));
  #endif
  }

#else

  const int offset = y * m_Width + x;
  const float zInv = color.w;

  if (zInv > m_ZBuffer[offset])
  {
  #ifdef RASTER_FAST_SCANLINE_W0W1W2
    const float w0 = color.x;
    const float w1 = color.y;
    const float w2 = color.z;
    float4 color2  = w0*pTri->c1 + w1*pTri->c2 + w2*pTri->c3;
    m_FrameBuffer[offset] = RealColorToUint32_BGRA(color2);
  #else
    m_FrameBuffer[offset] = RealColorToUint32_BGRA(color);
  #endif
    m_ZBuffer    [offset] = zInv;
  }

#endif

}

template<>
void Rasterizer::SetPixel<DRAW_SPAN_COLORED_2D>(unsigned int x, unsigned int y, const Color &color)
{
  m_FrameBuffer[y * m_Width + x] = RealColorToUint32_BGRA(color);
}

inline void Rasterizer::SetPixel(float x, float y, const Color &color)
{
  unsigned int xi = (int)x;
  unsigned int yi = (int)y;

  if (xi < 0 || yi < 0 || xi > m_Width-1 || yi > m_Height-1)
    return;

  const bool spass = (m_SBuffer == nullptr) ? true : (m_SBuffer[yi * m_Width + xi] & smask) != (sval & smask); // curr stencil func here ...

  if (spass)
    m_FrameBuffer[yi * m_Width + xi] = RealColorToUint32_BGRA(color);
}

template<int draw_type>
inline void Rasterizer::DrawSpan(const Span &span, int y)
{
	const int xdiff = span.X2 - span.X1;
	if(xdiff == 0)
		return;

	const Color colordiff  = span.Color2 - span.Color1;
	const float factorStep = 1.0f / (float)xdiff;

  float factor = 0.0f;

  // draw each pixel in the span
  for (int x = span.X1; x < span.X2; x++)
  {
    SetPixel<draw_type>((unsigned int)x, (unsigned int)y, span.Color1 + (colordiff * factor));
    factor += factorStep;
  }

}


inline void Rasterizer::DrawSpansBetweenEdges(const Edge &e1, const Edge &e2)
{
	// calculate difference between the y coordinates
	// of the first edge and return if 0
	const float e1ydiff = (float)(e1.Y2 - e1.Y1);
	if(e1ydiff == 0.0f)
		return;

	// calculate difference between the y coordinates
	// of the second edge and return if 0
	const float e2ydiff = (float)(e2.Y2 - e2.Y1);
	if(e2ydiff == 0.0f)
		return;

	// calculate differences between the x coordinates
	// and colors of the points of the edges
	const float e1xdiff     = (float)(e1.X2 - e1.X1);
	const float e2xdiff     = (float)(e2.X2 - e2.X1);
	const Color e1colordiff = (e1.Color2 - e1.Color1);
	const Color e2colordiff = (e2.Color2 - e2.Color1);

	// calculate factors to use for interpolation
	// with the edges and the step values to increase
	// them by after drawing each span
  const float factorStep1 = 1.0f / e1ydiff;
  const float factorStep2 = 1.0f / e2ydiff;

	float factor1 = (float)(e2.Y1 - e1.Y1) / e1ydiff;
	float factor2 = 0.0f;

  if (m_ZBuffer != nullptr && pTri->texS.data != nullptr)                          // Draw2DTexturedTriangle()
  {
    // loop through the lines between the edges and draw spans
    for (int y = e2.Y1; y < e2.Y2; y++)
    {
      // create and draw span
      Span span(e1.Color1 + (e1colordiff * factor1),
                e1.X1     + (int)(e1xdiff * factor1),
                e2.Color1 + (e2colordiff * factor2),
                e2.X1     + (int)(e2xdiff * factor2));

      DrawSpan<DRAW_SPAN_TEXTURED>(span, y);

      // increase factors
      factor1 += factorStep1;
      factor2 += factorStep2;
    }
  }
  else if (m_ZBuffer == nullptr)                                                 // Draw2DColoredTriangle()
  {
    // loop through the lines between the edges and draw spans
    for (int y = e2.Y1; y < e2.Y2; y++)
    {
      // create and draw span
      Span span(e1.Color1 + (e1colordiff * factor1), e1.X1 + (int)(e1xdiff * factor1),
                e2.Color1 + (e2colordiff * factor2), e2.X1 + (int)(e2xdiff * factor2));

      DrawSpan<DRAW_SPAN_COLORED_2D>(span, y);

      // increase factors
      factor1 += factorStep1;
      factor2 += factorStep2;
    }
  }
  else                                                                          // Draw2DColoredTriangleNoZBuffer()
  {
    // loop through the lines between the edges and draw spans
    for (int y = e2.Y1; y < e2.Y2; y++)
    {
      // create and draw span
      Span span(e1.Color1 + (e1colordiff * factor1),
                e1.X1 + (int)(e1xdiff * factor1),
                e2.Color1 + (e2colordiff * factor2),
                e2.X1 + (int)(e2xdiff * factor2));

      DrawSpan<DRAW_SPAN_COLORED>(span, y);

      // increase factors
      factor1 += factorStep1;
      factor2 += factorStep2;
    }
  }
}

void Rasterizer::DrawTriangle(const Color &color1, float x1, float y1,
                              const Color &color2, float x2, float y2,
                              const Color &color3, float x3, float y3)
{
	// create edges for the triangle
	Edge edges[3] = {
		Edge(color1, (int)x1, (int)y1, color2, (int)x2, (int)y2),
		Edge(color2, (int)x2, (int)y2, color3, (int)x3, (int)y3),
		Edge(color3, (int)x3, (int)y3, color1, (int)x1, (int)y1)
	};

	int maxLength = 0;
	int longEdge = 0;

	// find edge with the greatest length in the y axis and clamp edges coords
	for(int i = 0; i < 3; i++)
  {
    ///////////////////////////////////////////////////////////////////////////////////// inserted by vfrolov to original algorithm
    if (edges[i].X1 < 0) edges[i].X1 = 0;
    if (edges[i].X2 < 0) edges[i].X2 = 0;

    if (edges[i].Y1 < 0) edges[i].Y1 = 0;
    if (edges[i].Y2 < 0) edges[i].Y2 = 0;

    if (edges[i].X1 >= (int)m_Width) edges[i].X1 = m_Width-1;
    if (edges[i].X2 >= (int)m_Width) edges[i].X2 = m_Width-1;

    if (edges[i].Y1 >= (int)m_Height) edges[i].Y1 = m_Height - 1;
    if (edges[i].Y2 >= (int)m_Height) edges[i].Y2 = m_Height - 1;
    ///////////////////////////////////////////////////////////////////////////////////// inserted by vfrolov to original algorithm


		int length = edges[i].Y2 - edges[i].Y1;
		if(length > maxLength)
    {
			maxLength = length;
			longEdge = i;
		}

	}

	const int shortEdge1 = (longEdge + 1) % 3;
	const int shortEdge2 = (longEdge + 2) % 3;

	// draw spans between edges; the long edge can be drawn
	// with the shorter edges to draw the full triangle
	DrawSpansBetweenEdges(edges[longEdge], edges[shortEdge1]);
	DrawSpansBetweenEdges(edges[longEdge], edges[shortEdge2]);
}

void Rasterizer::DrawLine(const Color &color1, float x1, float y1,
                          const Color &color2, float x2, float y2)
{
	const float xdiff = (x2 - x1);
	const float ydiff = (y2 - y1);

	if(xdiff == 0.0f && ydiff == 0.0f)
  {
		SetPixel(x1, y1, color1);
		return;
	}

	if(fabs(xdiff) > fabs(ydiff))
  {
		float xmin, xmax;

		// set xmin to the lower x value given
		// and xmax to the higher value
		if(x1 < x2)
    {
			xmin = x1;
			xmax = x2;
		}
    else
    {
			xmin = x2;
			xmax = x1;
		}

		// draw line in terms of y slope
		float slope = ydiff / xdiff;
		for(float x = xmin; x <= xmax; x += 1.0f)
    {
			float y = y1 + ((x - x1) * slope);
			Color color = color1 + ((color2 - color1) * ((x - x1) / xdiff));
			SetPixel(x, y, color);
		}
	}
  else
  {
		float ymin, ymax;

		// set ymin to the lower y value given
		// and ymax to the higher value
		if(y1 < y2)
    {
			ymin = y1;
			ymax = y2;
		}
    else
    {
			ymin = y2;
			ymax = y1;
		}

		// draw line in terms of x slope
		float slope = xdiff / ydiff;
		for(float y = ymin; y <= ymax; y += 1.0f)
    {
			float x = x1 + ((y - y1) * slope);
			Color color = color1 + ((color2 - color1) * ((y - y1) / ydiff));
			SetPixel(x, y, color);
		}
	}
}


void rasterizeTri2(FrameBuffer* frameBuf, const Triangle& tri)
{
  Rasterizer scnRast;
  scnRast.SetFrameBuffer((uint32_t*)frameBuf->data, frameBuf->w, frameBuf->h);
  scnRast.SetZBuffer(frameBuf->getZBuffer());

  scnRast.pTri    = &tri;
  scnRast.sampler = tri.texS;

#ifdef ENABLE_SSE_FOR_RASTER

  #ifdef RASTER_FAST_SCANLINE_W0W1W2
  __m128 c1 = _mm_set_ps(tri.v1.z, 0.0f, 0.0f, 1.0f);
  __m128 c2 = _mm_set_ps(tri.v2.z, 0.0f, 1.0f, 0.0f);
  __m128 c3 = _mm_set_ps(tri.v3.z, 1.0f, 0.0f, 0.0f);
  #else
  __m128 c1 = _mm_set_ps(tri.v1.z, tri.c1.z, tri.c1.y, tri.c1.x);
  __m128 c2 = _mm_set_ps(tri.v2.z, tri.c2.z, tri.c2.y, tri.c2.x);
  __m128 c3 = _mm_set_ps(tri.v3.z, tri.c3.z, tri.c3.y, tri.c3.x);
  #endif

#else

  #ifdef RASTER_FAST_SCANLINE_W0W1W2
  float4 c1 = float4(1, 0, 0, 1);
  float4 c2 = float4(0, 1, 0, 1);
  float4 c3 = float4(0, 0, 1, 1);
  #else
  #ifdef ENABLE_SSE
  float4 c1 = float4(1, 0, 0, 1);
  float4 c2 = float4(0, 1, 0, 1);
  float4 c3 = float4(0, 0, 1, 1);
  #else
  float4 c1 = tri.c1;
  float4 c2 = tri.c2;
  float4 c3 = tri.c3;
  #endif
  #endif

  c1.w = tri.v1.z;
  c2.w = tri.v2.z;
  c3.w = tri.v3.z;

#endif

  scnRast.DrawTriangle(c1, tri.v1.x, tri.v1.y,
                       c2, tri.v2.x, tri.v2.y,
                       c3, tri.v3.x, tri.v3.y);

}


void rasterizeLine(FrameBuffer* frameBuf, float2 p1, float2 p2, float4 c1, float4 c2) // pre (frameBuf != nullptr)
{
  Rasterizer scnRast;
  scnRast.SetFrameBuffer((uint32_t*)frameBuf->data, frameBuf->w, frameBuf->h);
  scnRast.SetZBuffer(nullptr);
  scnRast.pTri = nullptr;

  scnRast.SetSBuffer(frameBuf->getSBuffer());

#ifdef ENABLE_SSE_FOR_RASTER
  scnRast.DrawLine(_mm_set_ps(c1.w, c1.z, c1.y, c1.x), p1.x, p1.y, _mm_set_ps(c2.w, c2.z, c2.y, c2.x), p2.x, p2.y);
#else
  scnRast.DrawLine(c1, p1.x, p1.y, c2, p2.x, p2.y);
#endif

}

void rasterizePoint(FrameBuffer* frameBuf, float2 p1, float4 c1, float size) // pre (frameBuf != nullptr)
{
  int minX = (int)(p1.x - 0.5f*size + 0.5f) + 1;
  int maxX = (int)(p1.x + 0.5f*size + 0.5f) + 1;
  int minY = (int)(p1.y - 0.5f*size + 0.5f) + 1;
  int maxY = (int)(p1.y + 0.5f*size + 0.5f) + 1;

  for (int y = minY; y < maxY; y++)
  {
    const int offset = y*frameBuf->w;
    for (int x = minX; x < maxX; x++)
      frameBuf->data[offset+x] = RealColorToUint32_BGRA(c1);
  }
}


