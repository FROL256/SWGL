#include "gl_sc.h"
#include "swgl.h"

#define MAX_CONTEXTS_COUNT 2

#ifdef WIN32

void wglMyLogLastError();

void SWGL_Context::Create(HDC a_hdc, int width, int height)
{
  m_hdc    = a_hdc;
  m_width  = width;
  m_height = height;

  m_fwidth  = float(m_width);
  m_fheight = float(m_height);

  BITMAPINFO bmi;
  bmi.bmiHeader.biSize          = sizeof(BITMAPINFO);
  bmi.bmiHeader.biWidth         = width;
  bmi.bmiHeader.biHeight        = height; // Order pixels from bottom to top 
  bmi.bmiHeader.biPlanes        = 1;
  bmi.bmiHeader.biBitCount      = 32;     // last byte not used, 32 bit for alignment
  bmi.bmiHeader.biCompression   = BI_RGB;
  bmi.bmiHeader.biSizeImage     = 0;
  bmi.bmiHeader.biXPelsPerMeter = 0;
  bmi.bmiHeader.biYPelsPerMeter = 0;
  bmi.bmiHeader.biClrUsed       = 0;
  bmi.bmiHeader.biClrImportant  = 0;
  bmi.bmiColors[0].rgbBlue      = 0;
  bmi.bmiColors[0].rgbGreen     = 0;
  bmi.bmiColors[0].rgbRed       = 0;
  bmi.bmiColors[0].rgbReserved  = 0;

  // Create DIB section to always give direct access to pixels
  //
  hbmp   = CreateDIBSection(a_hdc, &bmi, DIB_RGB_COLORS, (void**)&m_pixels, NULL, 0);
  hdcMem = CreateCompatibleDC(a_hdc);
  hbmOld = (HBITMAP)SelectObject(hdcMem, hbmp);

  m_pixels2 = m_pixels;
  //m_pixels2 = (int*)    _aligned_malloc(width*height*sizeof(int),     16);
  m_zbuffer = (float*)  _aligned_malloc(width*height*sizeof(float),   16);
  m_sbuffer = (uint8_t*)_aligned_malloc(width*height*sizeof(uint8_t), 16);

  // fill tiles data
  //

  m_drawList.tilesIds.clear();

  int ty = 0;

  for (int y = 0; y < m_height; y += TILE_SIZE)
  {
    int h = m_height - y;
    if (h > TILE_SIZE) h = TILE_SIZE;

    int tx = 0;
    for (int x = 0; x < m_width; x += TILE_SIZE)
    {
      int w = m_width - x;
      if (w > TILE_SIZE) w = TILE_SIZE;

      ScreenTile tile;

      tile.minX = x;
      tile.minY = y;
      tile.maxX = x + TILE_SIZE;
      tile.maxY = y + TILE_SIZE;
      tile.beginOffs = tile.endOffs = 0;

      m_drawList.tiles[tx][ty] = tile;
      m_drawList.tilesIds.push_back(int2(tx, ty));

      tx++;
    }
    m_drawList.m_tilesNumX = tx;

    ty++;
  }

  m_drawList.m_tilesNumY = ty;

  m_tiledFrameBuffer.Resize(m_width, m_height);
  m_tiledFrameBuffer.TestClearChessBoard();
}


void SWGL_Context::Destroy()
{
  SelectObject(hdcMem, hbmOld);
  DeleteDC(hdcMem);
  DeleteObject(hbmp);

  //_aligned_free(m_pixels2);
  _aligned_free(m_zbuffer);
  _aligned_free(m_sbuffer);

  hbmOld = NULL;
  hbmp   = NULL;
  hdcMem = NULL;
  m_hdc  = NULL;

  m_pixels2 = nullptr;
  m_zbuffer = nullptr;
  m_sbuffer = nullptr;
}

void SWGL_Context::CopyToScreeen()
{
  m_tiledFrameBuffer.CopyToRowPitch(m_pixels);
  //memset(m_pixels, 0xFFFFFFFF, m_width*m_height*sizeof(int32_t));
  BitBlt(m_hdc, 0, 0, m_width, m_height, hdcMem, 0, 0, SRCCOPY);
}


SWGL_Context  g_allCtx[MAX_CONTEXTS_COUNT];
int           g_ctxTop = 1;

extern SWGL_Context* g_pContext;
HGLRC g_currGLRC = 0;

//#include "../clew/clew.h"

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved )
{
  switch (ul_reason_for_call)
  {
  case DLL_PROCESS_ATTACH:
  {
    //int initRes = clewInit(L"opencl.dll");
    //if (initRes == -1)
    //  exit(0);

    break;
  }
  case DLL_PROCESS_DETACH:
    
    break;
  default:
    break;
  }
  return TRUE;
}


static PIXELFORMATDESCRIPTOR s_d3dPixelFormat =
{
  sizeof(PIXELFORMATDESCRIPTOR),
  1,                                  // Version Number
  PFD_DRAW_TO_WINDOW |                // Format Must Support Window
  PFD_SUPPORT_OPENGL |                // Format Must Support OpenGL
  PFD_DOUBLEBUFFER,					          // Must Support Double Buffering
  PFD_TYPE_RGBA,                      // Request An RGBA Format
  32,									                // Select Our Color Depth
  0, 0, 0, 0, 0, 0,                   // Color Bits Ignored
  8,                                  // 8-bit Alpha Buffer
  0,                                  // Shift Bit Ignored
  0,                                  // No Accumulation Buffer
  0, 0, 0, 0,                         // Accumulation Bits Ignored
  24,                                 // Z-Buffer (Depth Buffer) bits
  8,                                  // 8-bit Stencil Buffer
  0,                                  // No Auxiliary Buffer
  PFD_MAIN_PLANE,                     // Main Drawing Layer
  0,                                  // Reserved
  0, 0, 0                             // Layer Masks Ignored
};

extern "C" GLAPI int WINAPI wglChoosePixelFormat(HDC hdc, PIXELFORMATDESCRIPTOR *pfd)
{
  if (pfd)
    return 1;
  else
    return 0;
}

extern "C" GLAPI int WINAPI wglDescribePixelFormat(HDC hdc, int p, UINT up, LPPIXELFORMATDESCRIPTOR pfd)
{
  if (pfd) 
    memcpy(pfd, &s_d3dPixelFormat, sizeof(s_d3dPixelFormat));
  return 1;
}

extern "C" GLAPI int WINAPI wglGetPixelFormat(HDC hdc)
{
  return 1;
}

extern "C" GLAPI BOOL WINAPI wglSetPixelFormat(HDC hdc, int p, CONST PIXELFORMATDESCRIPTOR *pfd)
{
  // just silently pass the PFD through unmodified
  return TRUE;
}

extern "C" GLAPI BOOL WINAPI wglCopyContext(HGLRC hglrcSrc, HGLRC hglrcDst, UINT mask)
{
  return FALSE;
}

extern "C" GLAPI HGLRC WINAPI wglCreateLayerContext(HDC hdc, int iLayerPlane)
{
  return (HGLRC)0;
}

extern "C" GLAPI BOOL WINAPI wglDescribeLayerPlane(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nBytes, LPLAYERPLANEDESCRIPTOR plpd)
{
  return FALSE;
}

extern "C" GLAPI int WINAPI wglGetLayerPaletteEntries(HDC hdc, int iLayerPlane, int iStart, int cEntries, COLORREF *pcr)
{
  return 0;
}

extern "C" GLAPI int WINAPI wglSetLayerPaletteEntries(HDC, int, int, int, CONST COLORREF *)
{
  return 0;
}

extern "C" GLAPI BOOL WINAPI wglRealizeLayerPalette(HDC, int, BOOL)
{
  return FALSE;
}

extern "C" GLAPI BOOL WINAPI wglSwapLayerBuffers(HDC, UINT)
{
  return FALSE;
}

extern "C" GLAPI BOOL WINAPI wglShareLists(HGLRC, HGLRC)
{
  return TRUE;
}

extern "C" GLAPI BOOL WINAPI wglUseFontBitmapsA(HDC, DWORD, DWORD, DWORD)
{
  return FALSE;
}

extern "C" GLAPI BOOL WINAPI wglUseFontBitmapsW(HDC, DWORD, DWORD, DWORD)
{
  return FALSE;
}

extern "C" GLAPI BOOL WINAPI wglUseFontOutlinesA(HDC, DWORD, DWORD, DWORD, FLOAT, FLOAT, int, LPGLYPHMETRICSFLOAT)
{
  return FALSE;
}

extern "C" GLAPI BOOL WINAPI wglUseFontOutlinesW(HDC, DWORD, DWORD, DWORD, FLOAT, FLOAT, int, LPGLYPHMETRICSFLOAT)
{
  return FALSE;
}


extern "C" GLAPI BOOL WINAPI wglSwapInterval(int interval)
{
  //D3DGlobal.vSync = (interval > 0);
  return TRUE;
}

extern "C" GLAPI int WINAPI wglGetSwapInterval(void)
{
  return 1; // (D3DGlobal.vSync ? 1 : 0);
}


extern "C" GLAPI BOOL WINAPI wglSwapBuffers(HDC a_hdc)
{
  static int totalFrameCounter = 0;

  glFlush();

  if (g_pContext != nullptr)
  {
    g_pContext->CopyToScreeen();
  }

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "wglSwapBuffers(" << a_hdc << ")" << std::endl;

  // count FPS
  //
  if (g_pContext->m_currFrame == 0)
    g_pContext->m_lastNFramesT = clock();

  g_pContext->m_currFrame++;
  totalFrameCounter++;

  if (g_pContext->m_currFrame >= 60)
  {
    auto  timeT   = clock() - g_pContext->m_lastNFramesT;
    float timeInS = float(timeT) / float(CLOCKS_PER_SEC);
    float FPS     = 60.0f / timeInS;
    *(g_pContext->m_pLog) << "FPS = " << FPS << std::endl;
    g_pContext->m_currFrame = 0;
  }

  if (totalFrameCounter == 100)
  {
    *(g_pContext->m_pLog) << "Stats At frame 100: " << std::endl;
    *(g_pContext->m_pLog) << "msCL = " << g_pContext->m_timeStats.msClear << std::endl;
    *(g_pContext->m_pLog) << "msVS = " << g_pContext->m_timeStats.msVertexShader << std::endl;
    *(g_pContext->m_pLog) << "msTS = " << g_pContext->m_timeStats.msTriSetUp << std::endl;
    *(g_pContext->m_pLog) << "msBR = " << g_pContext->m_timeStats.msBinRaster << std::endl;
    *(g_pContext->m_pLog) << "msRS = " << g_pContext->m_timeStats.msRasterAndPixelShader << std::endl;
    *(g_pContext->m_pLog) << "msSW = " << g_pContext->m_timeStats.msSwapBuffers << std::endl;
    *(g_pContext->m_pLog) << std::endl;
  }

  g_pContext->m_timeStats.clear();

  return 1;
}

extern "C" GLAPI HGLRC APIENTRY wglCreateContext(HDC a_hdc)
{
  //
  //
  BITMAP structBitmapHeader;
  memset(&structBitmapHeader, 0, sizeof(BITMAP));

  HGDIOBJ hBitmap  = GetCurrentObject(a_hdc, OBJ_BITMAP);
  int bytesWritten = GetObjectA(hBitmap, sizeof(BITMAP), &structBitmapHeader);

  if (bytesWritten == 0)
    wglMyLogLastError();

  g_allCtx[g_ctxTop].Destroy();
  g_allCtx[g_ctxTop].Create(a_hdc, structBitmapHeader.bmWidth, structBitmapHeader.bmHeight);

  if (g_pContext != nullptr)
  {
    if (g_pContext->logMode <= LOG_ALL)
      *(g_pContext->m_pLog) << "wglCreateContext(" << a_hdc << ")" << std::endl;
  }

  HGLRC res = (HGLRC)g_ctxTop;
  g_ctxTop++;
  return res;
}

extern "C" GLAPI BOOL  APIENTRY wglMakeCurrent(HDC a_hdc, HGLRC a_gldc)
{
  if (a_hdc == NULL && a_gldc == NULL)
  {
    g_pContext = nullptr;
    g_currGLRC = 0;
  }
  else
  {
    g_pContext = &g_allCtx[(int)a_gldc];
    g_currGLRC = a_gldc;

    if (g_pContext->logMode <= LOG_ALL)
      *(g_pContext->m_pLog) << "wglMakeCurrent(" << a_hdc << ")" << std::endl;
  }

  return TRUE;
}

extern "C" GLAPI BOOL  APIENTRY wglDeleteContext(HGLRC a_gldc)
{
  if (g_pContext != nullptr && g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "wglDeleteContext(" << a_gldc << ")" << std::endl;

  g_allCtx[(int)a_gldc].Destroy();
  return TRUE;
}


extern "C" GLAPI HGLRC WINAPI wglGetCurrentContext(void)
{
  return g_currGLRC;
}

extern "C" GLAPI HDC WINAPI wglGetCurrentDC(void)
{
  if (g_pContext == nullptr)
    return 0;

  return g_pContext->m_hdc;
}


extern "C" GLAPI PROC WINAPI wglGetProcAddress(LPCSTR s)
{
  return GetProcAddress(GetModuleHandleA("opengl32.dll"), s);
}

extern "C" GLAPI PROC WINAPI wglGetDefaultProcAddress(LPCSTR s)
{
  return wglGetProcAddress(s);
}

#include <string>

std::string GetLastErrorAsString()
{
  //Get the error message, if any.
  DWORD errorMessageID = ::GetLastError();
  if (errorMessageID == 0)
    return std::string(); //No error message has been recorded

  LPSTR messageBuffer = nullptr;
  size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

  std::string message(messageBuffer, size);

  //Free the buffer.
  LocalFree(messageBuffer);

  return message;
}


void wglMyLogLastError()
{
  std::string errMsg = GetLastErrorAsString();

  if (g_pContext == nullptr)
    return;

  *(g_pContext->m_pLog) << "wglMyLogLastError() : " << errMsg.c_str() << std::endl;

  g_pContext->m_pLog->flush();
}

//extern "C" GLAPI const char* wglGetExtensionsStringARB(HDC hDC)
//{
//  return "WGL_ARB_buffer_region WGL_ARB_create_context WGL_ARB_create_context_profile WGL_ARB_create_context_robustness WGL_ARB_context_flush_control WGL_ARB_extensions_string WGL_ARB_make_current_read WGL_ARB_multisample WGL_ARB_pbuffer WGL_ARB_pixel_format WGL_ARB_pixel_format_float WGL_ARB_render_texture WGL_ATI_pixel_format_float WGL_EXT_create_context_es_profile WGL_EXT_create_context_es2_profile WGL_EXT_extensions_string WGL_EXT_framebuffer_sRGB WGL_EXT_pixel_format_packed_float WGL_EXT_swap_control WGL_EXT_swap_control_tear WGL_NVX_DX_interop WGL_NV_DX_interop WGL_NV_DX_interop2 WGL_NV_copy_image WGL_NV_delay_before_swap WGL_NV_float_buffer WGL_NV_multisample_coverage WGL_NV_render_depth_texture WGL_NV_render_texture_rectangle ";
//}

//extern "C" GLAPI const char* wglGetExtensionsStringARB(HDC hDC)
//{
//  return "";
//}



#endif

