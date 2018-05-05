#ifndef GLX_SC_H_INCLUDED
#define GLX_SC_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef int GLint;
typedef float GLfloat;

#ifndef APIENTRY
#define APIENTRY
#endif

#ifndef GLAPI
#define GLAPI
#endif

typedef float GLclampf;
typedef unsigned int GLbitfield;
typedef int GLsizei;

///////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef LINUX_PPC


#else

#include <X11/Xlib.h>
#include <X11/Xutil.h>


extern "C"
{
  GLAPI void APIENTRY glClear (GLbitfield mask);
  GLAPI void APIENTRY glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
  GLAPI void APIENTRY glViewport (GLint x, GLint y, GLsizei width, GLsizei height);
};


/* ClearBufferMask */
#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_STENCIL_BUFFER_BIT             0x00000400
#define GL_COLOR_BUFFER_BIT               0x00004000

/* Boolean */
#define GL_FALSE                          0
#define GL_TRUE                           1

///////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GLX_RGBA		4
#define GLX_DOUBLEBUFFER	5
#define GLX_DEPTH_SIZE		12


typedef struct __GLXcontextRec *GLXContext;
typedef XID GLXPixmap;
typedef XID GLXDrawable;


class X11FrameBuffer
{
public:

  X11FrameBuffer() : m_width(0), m_height(0), m_data(nullptr), m_image(nullptr), m_pixmap_id(0), m_display(nullptr) {}
  ~X11FrameBuffer() {freeBothImageAndData();}

  void resize(Display* display, int w,int h, int bpp);
  unsigned int* data() { return m_data; }

  void present(Window win);

protected:

  void freeBothImageAndData();

  int m_width;
  int m_height;

  unsigned int* m_data;
  XImage*       m_image;

  Pixmap        m_pixmap_id;
  Display*      m_display;
  GC            m_gc;
};

struct __GLXcontextRec
{
  X11FrameBuffer framebuff;
  Display *dpy;
  Window   win;
  int ctxId;
};

extern "C"
{

  int glXQueryExtension(Display *dpy, int *errorb, int *event );

  XVisualInfo* glXChooseVisual( Display *dpy, int screen, int *attribList);

  GLXContext glXCreateContext( Display *dpy, XVisualInfo *vis, GLXContext shareList, int direct );

  void glXDestroyContext( Display *dpy, GLXContext ctx );

  int glXMakeCurrent( Display *dpy, GLXDrawable drawable, GLXContext ctx);

  void glXSwapBuffers( Display *dpy, GLXDrawable drawable );

};

#endif

#endif // GLX_SC_H_INCLUDED
