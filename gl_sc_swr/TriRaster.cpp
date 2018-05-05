#include "TriRaster.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline void fillRow(FillFuncPtr pf, FrameBuffer* frameBuff, const float fx1, const float fx2, const int y, const Triangle& t)  // pre (y >= 0) && (y <= frameBuff->h)
{
  const int offset = frameBuff->w*y;

  const int x1  = (int)(fx1 - 0.0f);
  const int x2  = (int)(fx2 + 0.5f);

  const int x1c = clamp(x1, t.bb_iminX, t.bb_imaxX);
  const int x2c = clamp(x2, t.bb_iminX, t.bb_imaxX);

  const float  fy = (float)y;
  const float3 k2 = t.triAreaInv*float3((fy - t.v2.y)*(t.v3.x - t.v2.x), 
                                        (fy - t.v3.y)*(t.v1.x - t.v3.x), 
                                        (fy - t.v1.y)*(t.v2.x - t.v1.x));

#ifdef ENABLE_SSE
  pf(frameBuff, x1c, x2c, offset, t, _mm_set_ps(0.0f, k2.z, k2.y, k2.x));
#else
  pf(frameBuff, x1c, x2c, offset, t, k2);
#endif

}

void fillBottomFlatTriangle(FillFuncPtr pf, FrameBuffer* frameBuff, const float4 v1, const float4 v2, const float4 v3, const Triangle& t) // pre (v2.y == v3.y) && (v2.y != v1.y) && (v3.y != v1.y) && ( v1.y <= v2.y <= v3.y )
{
  float deltaX1 = (v2.x - v1.x) / (v2.y - v1.y); // #--> what if  v2.y == v1.y; see pre cond;        
  float deltaX2 = (v3.x - v1.x) / (v3.y - v1.y); // #--> what if  v3.y == v1.y; see pre cond;

  float x1 = v1.x;
  float x2 = v1.x;

  if (x1 + deltaX1 > x2 + deltaX2)
    std::swap(deltaX1, deltaX2);

  if (deltaX1*deltaX2 > 0.0f)
  {
    x1 -= fabs(deltaX1);
    x2 += fabs(deltaX2);
  }

  int startY = (int)v1.y;
  int endY   = min((int)v2.y, frameBuff->vy + frameBuff->vh - 1);

  if (startY < frameBuff->vy)
  {
    int skip = frameBuff->vy - startY;
    x1 += deltaX1*float(skip);
    x2 += deltaX2*float(skip);
    startY = frameBuff->vy;
  }

  for (int scanlineY = startY; scanlineY <= endY; scanlineY++)
  {
    fillRow(pf, frameBuff, x1, x2, scanlineY, t);
    x1 += deltaX1;
    x2 += deltaX2;
  }
}

void fillTopFlatTriangle(FillFuncPtr pf, FrameBuffer* frameBuff, const float4 v1, const float4 v2, const float4 v3, const Triangle& t) // pre (v1.y == v2.y) && (v3.y != v1.y) && (v3.y != v2.y) && ( v1.y <= v2.y <= v3.y )
{
  float deltaX1 = (v3.x - v1.x) / (v3.y - v1.y); // #--> what if  v3.y == v1.y; see pre cond;     
  float deltaX2 = (v3.x - v2.x) / (v3.y - v2.y); // #--> what if  v3.y == v2.y; see pre cond;

  float x1 = v3.x;
  float x2 = v3.x;

  if (x1 - deltaX1 > x2 - deltaX2)
    std::swap(deltaX1, deltaX2);

  if (deltaX1*deltaX2 > 0.0f)
  {
    x1 -= fabs(deltaX1);
    x2 += fabs(deltaX2);
  }

  int startY = (int)v3.y;
  int endY   = max((int)v1.y, frameBuff->vy); 

  if (startY > frameBuff->vy + frameBuff->vh - 1) // (frameBuff->vy + frameBuff->vh - 1) instead of (frameBuff->h - 1)
  {
    int skip = startY - (frameBuff->vy + frameBuff->vh - 1);
    x1 -= deltaX1*float(skip);
    x2 -= deltaX2*float(skip);
    startY = frameBuff->vy + frameBuff->vh - 1;
  }

  for (int scanlineY = startY; scanlineY >= endY; scanlineY--)
  {
    x1 -= deltaX1;
    x2 -= deltaX2;
    fillRow(pf, frameBuff, x1, x2, scanlineY, t);
  }

}

inline void sortVerticesAscendingByY(float3& v1, float3& v2, float3& v3) // post (v1.y <= v2.y) && (v2.y <= v3.y) 
{
  if (v1.y > v2.y) std::swap(v1, v2);
  if (v2.y > v3.y) std::swap(v2, v3);
  if (v1.y > v2.y) std::swap(v1, v2);
}

inline void sortVerticesAscendingByY(float4& v1, float4& v2, float4& v3) // post (v1.y <= v2.y) && (v2.y <= v3.y) 
{
  if (v1.y > v2.y) std::swap(v1, v2);
  if (v2.y > v3.y) std::swap(v2, v3);
  if (v1.y > v2.y) std::swap(v1, v2);
}


void rasterizeTri(FillFuncPtr pf, FrameBuffer* frameBuf, const Triangle& tri)
{
  auto v1 = tri.v1;
  auto v2 = tri.v2;
  auto v3 = tri.v3;

  if ((v1.y == v2.y) && (v2.y == v3.y)) // v1.y == v2.y == v3.y
    return;

  // at first sort the three vertices by y-coordinate ascending, so p1 is the topmost vertice 
  
  sortVerticesAscendingByY(v1, v2, v3);

  // here we know that v1.y <= v2.y <= v3.y 

  if (v2.y == v3.y)        // bottom-flat triangle, may omit (v2.y != v1.y) && (v3.y != v1.y) because this lead to v1.y == v2.y == v3.y which was processed early 
  {
    fillBottomFlatTriangle(pf, frameBuf, v1, v2, v3, tri);
  }
  else if (v1.y == v2.y)   // top-flat triangle, may omit (v3.y != v1.y) && (v3.y != v2.y) because this lead to v1.y == v2.y == v3.y which was processed early
  {
    fillTopFlatTriangle   (pf, frameBuf, v1, v2, v3, tri);
  }
  else
  {
    // general case - split the triangle in a top-flat and bottom-flat one 

    const float ty   = (v2.y - v1.y) / (v3.y - v1.y); // #--> what if v3.y == v1.y ; not possible because (v1.y <= v2.y <= v3.y) && (v1.y != v2.y) && (v2.y != v3.y)
    const float tmpX = (v1.x + ty*(v3.x - v1.x));     // 
    const float tmpZ = (v1.z + ty*(v3.z - v1.z));     //
    const float4 vTmp(tmpX, v2.y, tmpZ, 1.0f);

    fillBottomFlatTriangle(pf, frameBuf, v1, v2, vTmp, tri);
    fillTopFlatTriangle   (pf, frameBuf, v2, vTmp, v3, tri);
  }
}
