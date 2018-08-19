#include "glx_sc.h"
#include "swgl.h"
#include <iostream>

#include "config.h"

#ifdef LINUX_PPC
#define WRM_LIBWRM

extern "C"
{
  #include <wrm_scm_api.h>
  #include <libwrm_bm.h>
  #include <libwrm_scm.h>
};


extern "C" int fb_wrm_init(int* width, int* height);


SWGL_Context g_oneContext;
extern SWGL_Context* g_pContext;

int swglCreateContextWRM()
{
  g_pContext = &g_oneContext;
  g_pContext->Create();

  return g_pContext->device;
}

void swglSwapBuffersWRM()
{
  glFlush();
  g_pContext->CopyToScreeen();
}


void SWGL_Context::Create()
{
  int width  = 0;
  int height = 0;
  device = fb_wrm_init(&width, &height);


  int32_t id[2] = {WRM_SCM_ID_SCREEN0_PRIMARY_SURFACE, WRM_SCM_ID_SCREEN0_SECONDARY_SURFACE};
  libwrm_bm_write(device, id[0], WRM_BM_FLAG_DOMAIN_DEVICE, 0, m_pixels, height*width*4);
  libwrm_bm_write(device, id[1], WRM_BM_FLAG_DOMAIN_DEVICE, 0, m_pixels, height*width*4);

  m_width  = width;
  m_height = height;

  std::cout << "[gl_wrm]: width = "  << width << std::endl;
  std::cout << "[gl_wrm]: height = " << height << std::endl;

  m_fwidth  = float(m_width);
  m_fheight = float(m_height);

  m_pixels  = (int*)malloc(width*height*sizeof(int));
  m_pixels2 = (int*)malloc(width*height*sizeof(int));
  m_zbuffer = (float*)malloc(width*height*sizeof(float));
  m_sbuffer = (uint8_t*)malloc(width*height*sizeof(uint8_t));

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

  #ifdef ENABLE_MT_TASK_STEAL
  {
    if (ENABLE_MT)
    {
      if (m_width != 0 && m_height != 0 && m_pTaskPool == nullptr)
        m_pTaskPool = new TaskQueue(MT_TASK_STEAL_THREADS);
      else if (m_width == 0 && m_height == 0)
        m_pTaskPool = nullptr;
    }
    else
    {
      m_pTaskPool = nullptr;
    }
  }
  #endif

  swglInitDrawListAndTiles(&g_pContext->m_drawList, MAX_NUM_TRIANGLES_TOTAL); // crappy code ...

}

void SWGL_Context::Destroy()
{
  std::cout << "[swgl]: setroy context " << std::endl;
  free(m_pixels);  m_pixels  = nullptr;
  free(m_pixels2); m_pixels2 = nullptr;
  free(m_zbuffer); m_zbuffer = nullptr;
  free(m_sbuffer); m_sbuffer = nullptr;
  libwrm_device_close(device);
}

void SWGL_Context::CopyToScreeen()
{
  //static int file_num = 0;
  static uint32_t id[2] = {WRM_SCM_ID_SCREEN0_PRIMARY_SURFACE, WRM_SCM_ID_SCREEN0_SECONDARY_SURFACE};
  static int flip = 0;
  int y = 0, x = 0;
  int device = this->device;
  int buf_size = this->m_width * this->m_height * 4;

  //char buf[1024];
  //FILE *ff = NULL;

  //for (y = 0; y < this->m_height; y++)
  //{
  //  int offset1 = y * this->m_width;
  //  int offset2 = (this->m_height - y - 1) * this->m_width;
  //
  //  for (x = 0; x < this->m_width; x++)
  //    this->m_pixels[offset1 + x] = this->m_pixels2[offset2 + x];
  //}

  //libwrm_bm_write(device, id[flip], WRM_BM_FLAG_DOMAIN_DEVICE, 0, this->m_pixels, buf_size);
  //libwrm_scm_screen_flip(device, WRM_SCM_SCREEN0, id[flip], 0);


  libwrm_bm_write(device, id[flip], WRM_BM_FLAG_DOMAIN_DEVICE, 0, this->m_pixels2, buf_size);
  libwrm_scm_screen_flip(device, WRM_SCM_SCREEN0, id[flip], 0);

  flip = !flip;

  static int totalFrameCounter = 0;

  if (totalFrameCounter == 10)
  {
    *(g_pContext->m_pLog) << "Stats At frame 10: " << std::endl;
    *(g_pContext->m_pLog) << "msCL = " << 0.1f*g_pContext->m_timeStats.msClear << std::endl;
    *(g_pContext->m_pLog) << "msVS = " << 0.1f*g_pContext->m_timeStats.msVertexShader << std::endl;
    *(g_pContext->m_pLog) << "msTS = " << 0.1f*g_pContext->m_timeStats.msTriSetUp << std::endl;
    *(g_pContext->m_pLog) << "msRS = " << 0.1f*g_pContext->m_timeStats.msRasterAndPixelShader << std::endl;
    *(g_pContext->m_pLog) << "msSW = " << 0.1f*g_pContext->m_timeStats.msSwapBuffers << std::endl;
    *(g_pContext->m_pLog) << std::endl;
  }

  totalFrameCounter++;
}


#else


////////////////////////////////////////////////////////////////////////

extern "C"
{


//////////////////////////////////////////////////////////////////////////////



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
  m_pixels2 = (int*)malloc(width*height*sizeof(int));
  m_zbuffer = (float*)malloc(width*height*sizeof(float));
  m_sbuffer = (uint8_t*)malloc(width*height*sizeof(uint8_t));
  
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
    #pragma omp parallel for
    for (int y = 0; y < m_height; y++)
    {
      int offset0 = y * m_width;
      int offset1 = (m_height - y - 1) * m_width;

      for (int x = 0; x < m_width; x++)
      {
        int oldPx = m_pixels2[offset1 + x];
        // int red   = (oldPx & 0x000000FF);
        // int green = (oldPx & 0x0000FF00) >> 8;
        // int blue  = (oldPx & 0x00FF0000) >> 16;
        m_pixels[offset0 + x] = oldPx; // (blue << 16) | (green << 8) | (red);
      }
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
  if(g_pContext == nullptr)
    return;

  glFlush();

  g_pContext->CopyToScreeen();
  g_pContext->glxrec.framebuff.present(g_pContext->glxrec.win);

  g_pContext->m_timeStats.clear();
}




};

#endif // LINUX_PPC
