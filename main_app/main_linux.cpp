// -- Written in C++ -- //

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<math.h>
#include<time.h>
#include<sys/time.h>

#include "tests_sc1.h"

#ifdef USE_SWGL
  #include "../gl_sc_swr/gl_sc.h"
  #include "../gl_sc_swr/glx_sc.h"
#endif // USE_SWGL


#include <memory.h>
#include <stdexcept>
#include <iostream>

#include "../gl_sc_swr/Timer.h"
#include "../gl_sc_swr/config.h"

const int WIN_WIDTH_INITIAL  = 1024;
const int WIN_HEIGHT_INITIAL = 1024;


void InfoGL() // check custome extentions here
{
  CHECK_GL_ERRORS;
}

#ifdef LINUX_PPC

#else

#include<X11/Xlib.h>
#include<X11/XKBlib.h>

//////////////////////////////////////////////////////////////////////////////////
//				GLOBAL IDENTIFIERS				                                            //
//////////////////////////////////////////////////////////////////////////////////
Display                 *dpy;
Window                  root, win;
GLint                   att[]   = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
XVisualInfo             *vi;
GLXContext              glc;
Colormap                cmap;
XSetWindowAttributes    swa;
XWindowAttributes	wa;
XEvent			xev;

float	TimeCounter, LastFrameTimeCounter, DT, prevTime = 0.0, FPS;
int			Frame = 1, FramesPerFPS;

float			rot_z_vel = 50.0, rot_y_vel = 30.0;

extern int g_localWidth;
extern int g_localHeight;

Timer g_timer(false);

void ExposeFunc()
{
  g_localHeight = wa.height;
  g_localWidth  = wa.width;
  
  if(NOWINDOW)
  {
    glViewport(0, 0, WIN_WIDTH_INITIAL, WIN_HEIGHT_INITIAL);
    wa.width  = WIN_WIDTH_INITIAL;
    wa.height = WIN_HEIGHT_INITIAL;
  }
  else
  {
    XGetWindowAttributes(dpy, win, &wa);
    glViewport(0, 0, wa.width, wa.height);
  }
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  static float angle1 = 30.0f;
  static float angle2 = 50.0f;

  static int   frameCounter = -1;
  static float frameTime    = 0.0f;

  if(frameCounter < 0 && !NOWINDOW)
  {
    frameCounter = 0;
    g_timer.start();
  }

  try
  {
    //test01_colored_triangle();
    //demo01_colored_triangle(angle1);
    //test02_nehe_lesson1_simplified();

    // paper demos

    demo04_pyramid_and_cube_3d(wa.width, wa.height, 40.0f, 20.0f);
    //demo03_many_small_dynamic_triangles();
    //demo19_cubes(wa.width, wa.height, 0.0f, 50.0f);
    //demo25_teapot(wa.width, wa.height, 0.0f, 0.0f);
    //demo26_teapots9(wa.width, wa.height, 0.0f, 60.0f);
    //demo24_draw_elements_terrain(wa.width, wa.height, 0.0f, 0.0f);

    // \\ paper demos

    //demo04_pyramid_and_cube_3d(wa.width, wa.height, angle1, angle2);
    //demo14_transparent_cube(wa.width, wa.height, 10.0f, 13.0f);


    //test11_alpha_tex_and_transp();
    //test12_rect_tex();

    //demo14_transparent_cube(wa.width, wa.height, angle1, angle2);
    //demo05_texture_3D(wa.width, wa.height, angle1, angle2);
    //demo19_cubes(wa.width, wa.height, 10.0f, 13.0f);
    //demo19_cubes(wa.width, wa.height, angle1, angle2);

    //demo24_draw_elements_terrain(wa.width, wa.height, angle1, angle2);
    //demo25_teapot(wa.width, wa.height, angle1, angle2);
    //demo26_teapots9(wa.width, wa.height, angle1, angle2);

    //demo04_pyramid_and_cube_3d(wa.width, wa.height, 20, 30);
    //test11_alpha_tex_and_transp(); // 2D Blending test ...

    //test25_clip_triangles(wa.width, wa.height, 0.0f);

    angle1 += 25.0f/(FPS+1.0f);
    angle2 += 50.0f/(FPS+1.0f);

    if(!NOWINDOW)
    {
      frameCounter++;
      frameTime += g_timer.getElapsed();
      g_timer.start();
  
      if (frameCounter >= 10)
      {
        FPS = (float) frameCounter / frameTime;
        frameCounter = 0;
        frameTime = 0.0f;
    
        char temp[64];
        sprintf(temp, "FPS = %f", FPS);
        XStoreName(dpy, win, temp);
      }
    }
  
    //glFinish();
    //std::vector<int>   pixels1(WIN_WIDTH_INITIAL*WIN_HEIGHT_INITIAL);
    //glReadPixels(0, 0, WIN_WIDTH_INITIAL, WIN_HEIGHT_INITIAL, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)&pixels1[0]);
    //SaveBMP("zscreen.bmp", pixels1.data(), WIN_WIDTH_INITIAL, WIN_HEIGHT_INITIAL);
    //exit(0);
  }
  catch(std::runtime_error& e)
  {
    std::cout << "err : " << e.what() << std::endl;
  }

  glXSwapBuffers(dpy, win);
}

//////////////////////////////////////////////////////////////////////////////////
//				CREATE A GL CAPABLE WINDOW			//
//////////////////////////////////////////////////////////////////////////////////
void CreateWindow()
{
  if(NOWINDOW)
    return;
  
  if((dpy = XOpenDisplay(NULL)) == NULL)
  {
    printf("\n\tcannot connect to x server\n\n");
    exit(0);
  }

  root = DefaultRootWindow(dpy);

  if((vi = glXChooseVisual(dpy, 0, att)) == NULL)
  {
    printf("\n\tno matching visual\n\n");
    exit(0);
  }

  if((cmap = XCreateColormap(dpy, root, vi->visual, AllocNone)) == 0)
  {
    printf("\n\tcannot create colormap\n\n");
    exit(0);
  }

  swa.event_mask = KeyPressMask;
  swa.colormap 	= cmap;
  win = XCreateWindow(dpy, root, 0, 0, WIN_WIDTH_INITIAL, WIN_HEIGHT_INITIAL, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
  XStoreName(dpy, win, "OpenGL Animation");
  XMapWindow(dpy, win);
}
//////////////////////////////////////////////////////////////////////////////////
//				SETUP GL CONTEXT				//
//////////////////////////////////////////////////////////////////////////////////

void SetupGL()
{
  char		font_string[128];
  XFontStruct	*font_struct;

  glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);

  if(glc == NULL)
  {
    printf("\n\tcannot create gl context\n\n");
    exit(0);
  }

  glXMakeCurrent(dpy, win, glc);
  glClearColor(0.00, 0.00, 0.40, 1.00);
}


//////////////////////////////////////////////////////////////////////////////////
//				EXIT PROGRAM					//
//////////////////////////////////////////////////////////////////////////////////
void ExitProgram()
{
  glXMakeCurrent(dpy, None, NULL);
  glXDestroyContext(dpy, glc);
  XDestroyWindow(dpy, win);
  XCloseDisplay(dpy);
  exit(0);
}
//////////////////////////////////////////////////////////////////////////////////
//				CHECK EVENTS					//
//////////////////////////////////////////////////////////////////////////////////
void CheckKeyboard()
{
  if(NOWINDOW)
    return;
  
  if(XCheckWindowEvent(dpy, win, KeyPressMask, &xev))
  {
    char	*key_string = XKeysymToString(XkbKeycodeToKeysym(dpy, xev.xkey.keycode, 0, 0));

    if(strncmp(key_string, "Left", 4) == 0)
      rot_z_vel -= 200.0*DT;
    else if(strncmp(key_string, "Right", 5) == 0)
      rot_z_vel += 200.0*DT;
    else if(strncmp(key_string, "Up", 2) == 0)
      rot_y_vel -= 200.0*DT;
    else if(strncmp(key_string, "Down", 4) == 0)
      rot_y_vel += 200.0*DT;
    else if(strncmp(key_string, "F1", 2) == 0)
    {
      rot_y_vel = 0.0;
      rot_z_vel = 0.0;
    }
    else if(strncmp(key_string, "Escape", 5) == 0)
    {
      ExitProgram();
    }
  }
}
//////////////////////////////////////////////////////////////////////////////////
//				MAIN PROGRAM					                                                //
//////////////////////////////////////////////////////////////////////////////////
SWGL_Timings _swglGetStats();

int main(int argc, char *argv[]) // 222
{
  //freopen("stdout.txt", "wt", stdout);
  //freopen("stderr.txt", "wt", stderr);
  
  CreateWindow();
  SetupGL();
  InfoGL();
  
  size_t frameCounter = 0;
  while(true)
  {
    ExposeFunc();
    //usleep(1000);
    CheckKeyboard();
  
    frameCounter++;
    if(NOWINDOW && frameCounter >= 100)
      break;
  }
  
  if(NOWINDOW)
  {
    auto timings = _swglGetStats();
    
    std::cout << std::endl;
    std::cout << "Time stats: " << std::endl;
  
    std::cout << "Stats at frame " <<  frameCounter << ": " << std::endl;
    std::cout << "msCL      = " << 0.01f*(timings.msClear) << std::endl;
    std::cout << "msVS      = " << 0.01f*(timings.msVertexShader) << std::endl;
    std::cout << "msTS      = " << 0.01f*(timings.msTriSetUp) << std::endl;
    std::cout << "msRS      = " << 0.01f*(timings.msRasterAndPixelShader) << std::endl;
    std::cout << "ms(TS+RS) = " << 0.01f*(timings.msTriSetUp + timings.msRasterAndPixelShader);
    std::cout << std::endl;
    
    glFinish();
    std::vector<int>   pixels1(WIN_WIDTH_INITIAL*WIN_HEIGHT_INITIAL);
    glReadPixels(0, 0, WIN_WIDTH_INITIAL, WIN_HEIGHT_INITIAL, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)&pixels1[0]);
    SaveBMP("zscreen.bmp", pixels1.data(), WIN_WIDTH_INITIAL, WIN_HEIGHT_INITIAL);
    std::cout.flush();
  }
 
  return 0;
}

#endif
