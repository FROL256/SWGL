#include "glx_sc.h"
#include "swgl.h"
#include <iostream>

#include "config.h"


////////////////////////////////////////////////////////////////////////
extern "C"
{


// Note that when the image is created using XCreateImage(), XGetImage(), or XSubImage(),
// the destroy procedure that this macro calls frees both (!!!) the image structure and
// the data pointed to by the image structure.
//
void X11FrameBuffer::freeBothImageAndData()
{
  if(m_image != nullptr)
    XDestroyImage (m_image);

  m_data   = nullptr;
  m_image  = nullptr;
  m_width  = 0;
  m_height = 0;

  // free server side image
  //
  if(m_display != nullptr && m_pixmap_id != 0)
    XFreePixmap(m_display, m_pixmap_id);

  if(m_display != nullptr && m_gc != 0)
    XFreeGC (m_display, m_gc);
}

void X11FrameBuffer::resize(Display* display, int w,int h, int bpp)
{
  freeBothImageAndData();

  m_width  = w;
  m_height = h;

  int s    = DefaultScreen(display);
  m_gc     = XCreateGC (display, RootWindow(display, s), 0, NULL);

  /* tell server that start managing my pixmap */
  m_pixmap_id = XCreatePixmap (display, RootWindow(display, s), w, h, bpp);
  m_display   = display;


  //
  //
  m_data = (unsigned int*)malloc(w*h*sizeof(unsigned int));

  for(int i=0;i<w*h;i++)
    m_data[i]= 0x000000FF;


  const int my_pix_padding = 32;
  const int bytes_per_line = 0;

  //
  //
  m_image = XCreateImage (display, NULL, bpp, ZPixmap, 0,
                          (char*)m_data, w, h, my_pix_padding, bytes_per_line);

}

void X11FrameBuffer::present(Window win)
{
  /* copy from client to server */
  XPutImage (m_display, m_pixmap_id, m_gc, m_image, 0,0, 0, 0, m_width, m_height);

  /* copy the server copy of the pixmap into the window */
  XCopyArea (m_display, m_pixmap_id, win, m_gc, 0,0, m_width, m_height, 0, 0);
}


//////////////////////////////////////////////////////////////////////////////


int glXQueryExtension(Display *dpy, int *errorb, int *event )
{
  return 1;
}

XVisualInfo g_visualInfo;
int         g_fbColorDepth;

XVisualInfo* glXChooseVisual(Display *dpy, int screen, int *attribList)
{
  int myDepth      = DefaultDepth(dpy, screen);  /// Глубина цветности экрана
  //Visual* myVisual = DefaultVisual(dpy, screen); /// Визуальные характеристики

  if(!XMatchVisualInfo(dpy, screen, myDepth, TrueColor, &g_visualInfo))
  {
    std::cout << "XMatchVisualInfo failed !" << std::endl;
    // log("XMatchVisualInfo failed");
  }

  g_fbColorDepth = myDepth;

  return &g_visualInfo; // XGetVisualInfo(dpy, vinfo_mask, vinfo_template, nitems_return);
}


void SWGL_Context::Create(Display *dpy, XVisualInfo *vis, int width, int height)
{
  m_width  = width;
  m_height = height;

  m_fwidth  = float(m_width);
  m_fheight = float(m_height);

  this->glxrec.framebuff.resize(dpy, width, height, g_fbColorDepth);

  m_pixels  = (int*)this->glxrec.framebuff.data(); // #TODO: _aligned_malloc !
  m_pixels2 = (int*)    malloc((width + FB_BILLET_SIZE)*height*sizeof(int));
  m_zbuffer = (float*)  malloc((width + FB_BILLET_SIZE)*height*sizeof(float));
  m_sbuffer = (uint8_t*)malloc((width + FB_BILLET_SIZE)*height*sizeof(uint8_t));
  
  m_tiledFrameBuffer.Resize(m_width, m_height);
  m_tiledFrameBuffer.TestClearChessBoard();

}

void SWGL_Context::Destroy()
{
  free(m_pixels2); m_pixels2 = nullptr;
  free(m_zbuffer); m_zbuffer = nullptr;
  free(m_sbuffer); m_sbuffer = nullptr;
}

void SWGL_Context::CopyToScreeen()
{
  if (m_useTiledFB)
    m_tiledFrameBuffer.CopyToRowPitch(m_pixels);
  else 
  {
    const int pitch = (m_width + FB_BILLET_SIZE);

    for (int y = 0; y < m_height; y++)
    {
      int offset0 = y * m_width;
      int offset1 = (m_height - y - 1) * pitch;

      for (int x = 0; x < m_width; x++)
        m_pixels[offset0 + x] = m_pixels2[offset1 + x];
    }
  }

}

#define MAX_CONTEXTS_COUNT 4

SWGL_Context  g_allCtx[MAX_CONTEXTS_COUNT];
int           g_ctxTop = 1;

extern SWGL_Context* g_pContext;

GLXContext glXCreateContext(Display *dpy, XVisualInfo *vis, GLXContext shareList, int direct )
{
  GLXContext ptr = &g_allCtx[g_ctxTop].glxrec;

  ptr->dpy   = dpy;
  ptr->ctxId = g_ctxTop;

  g_ctxTop++;
  return ptr;
}

void glXDestroyContext( Display *dpy, GLXContext ctx )
{
  if(ctx == nullptr)
    return;

  g_allCtx[ctx->ctxId].Destroy();
}

Bool glXMakeCurrent( Display *dpy, GLXDrawable drawable, GLXContext ctx)
{
  if(ctx == nullptr)
    return False;

  ctx->win   = drawable;
  g_pContext = &g_allCtx[ctx->ctxId];
  return True;
}

void glXSwapBuffers( Display *dpy, GLXDrawable drawable )
{
  static int totalFrameCounter = 0;

  if(g_pContext == nullptr)
    return;

  glFlush();

  g_pContext->CopyToScreeen();
  g_pContext->glxrec.framebuff.present(g_pContext->glxrec.win);

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
    totalFrameCounter = 0;
  }

  g_pContext->m_timeStats.clear();
  totalFrameCounter++;
}



};


