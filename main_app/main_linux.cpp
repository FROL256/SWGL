// -- Written in C++ -- //

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<math.h>
#include<time.h>
#include<sys/time.h>


#ifdef USE_SWGL

  #include "../gl_sc_swr/gl_sc.h"
  #include "../gl_sc_swr/glx_sc.h"


  #ifdef LINUX_PPC

  extern "C"
  {
    #define WRM_LIBWRM
    #include <libwrm_device.h>
    #include <libwrm_bm.h>
    #include <libwrm_scm.h>
  }
  #endif


#else
  #include<GL/glx.h>
#endif // USE_SWGL


#include <memory.h>
#include <stdexcept>
#include <iostream>

#include "tests_sc1.h"
#include "../gl_sc_swr/Timer.h"



void InfoGL() // check custome extentions here
{
  CHECK_GL_ERRORS;

  //std::cout << "GPU Vendor: " << glGetString(GL_VENDOR) << std::endl;
  //std::cout << "GPU Name  : " << glGetString(GL_RENDERER) << std::endl;
  //std::cout << "GL_VER    : " << glGetString(GL_VERSION) << std::endl;
  //std::cout << "GLSL_VER  : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

}

#ifdef LINUX_PPC

#define BILLION  1000000000L

int swglCreateContextWRM();
void swglSwapBuffersWRM();

float			TimeCounter, LastFrameTimeCounter, DT, prevTime = 0.0, FPS = 30.0f;
struct timeval		tv, tv0;
int			Frame = 1, FramesPerFPS;

GLfloat		rotation_matrix[16];
float			rot_z_vel = 50.0, rot_y_vel = 30.0;

extern int g_localWidth;
extern int g_localHeight;

Timer g_timer(false);

int getTimeMS()
{
  struct timeval tmval;
  struct timezone tmzone;

  // Receive current time
  gettimeofday(&tmval, &tmzone);

  int a = (int)(tmval.tv_sec);
  int b = (int)((long)tmval.tv_usec / 1000L);

  return a*1000 + b;
}

int main(int argc, char *argv[]) // 222
{
  int device = swglCreateContextWRM();

  std::cout << "wrm deviceId = " << device << std::endl;
  glViewport(0,0,1024,768);

  g_localWidth  = 1024;
  g_localHeight = 768;

  const int g_width  = g_localWidth;
  const int g_height = g_localHeight;

  float angle1 = 30.0f;
  float angle2 = 50.0f;

  int   frameCounter = 0;
  int   frameCounterTotal = 0;
  float frameTime    = 0.0f;

  g_timer.start();

  double accum = 0.0;

  //int oldTime = getTimeMS();
  struct timeval t0,t1;
  gettimeofday(&t0, 0);


  while(true)
  {
    //struct timespec start, stop;
    //clock_gettime( CLOCK_REALTIME, &start);

    glClearColor(0.0f,0.0f,0.0f,1.0f);

    //demo05_texture_3D(g_width, g_height, angle1, angle2);

    //demo04_pyramid_and_cube_3d(1024, 768, angle1, angle2);
    //demo03_many_small_dynamic_triangles();
    //demo19_cubes(1024, 768, angle1, angle2);

    demo25_teapot(1024, 768, angle1, angle2);
    //demo26_teapots9(g_width, g_height, angle1, angle2);

    //demo24_draw_elements_terrain(1024, 768, angle1, angle2);

    swglSwapBuffersWRM();

    //
    //
    angle1 += 50.0f/(FPS+1.0f);
    angle2 += 100.0f/(FPS+1.0f);



    frameCounter++;
    frameCounterTotal++;

    if(frameCounter >= 10)
    {
      gettimeofday(&t1, 0);

      long long elapsed = (t1.tv_sec-t0.tv_sec)*1000000LL + t1.tv_usec-t0.tv_usec;
      float FPS = float(frameCounterTotal)/( float(elapsed) / float(1000000));

      printf("FPS = %f\n", FPS);
      //std::cout << "Frames = " << frameCounterTotal << std::endl;
      frameCounter = 0;
    }

  };

  std::cout << "hello PPC world(333) " << std::endl;

}


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

float			TimeCounter, LastFrameTimeCounter, DT, prevTime = 0.0, FPS;
struct timeval		tv, tv0;
int			Frame = 1, FramesPerFPS;

GLfloat		rotation_matrix[16];
float			rot_z_vel = 50.0, rot_y_vel = 30.0;

extern int g_localWidth;
extern int g_localHeight;

Timer g_timer(false);

void ExposeFunc()
{
    g_localWidth  = wa.width;
    g_localHeight = wa.height;

    float	aspect_ratio;
    char	info_string[256];

    XGetWindowAttributes(dpy, win, &wa);
    glViewport(0, 0, wa.width, wa.height);
    aspect_ratio = (float)(wa.width) / (float)(wa.height);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static float angle1 = 30.0f;
    static float angle2 = 50.0f;

    static int   frameCounter = -1;
    static float frameTime    = 0.0f;

    if(frameCounter < 0)
    {
      frameCounter = 0;
      g_timer.start();
    }

    try
    {
      //test01_colored_triangle();
      //test02_nehe_lesson1_simplified();

      //demo04_pyramid_and_cube_3d(wa.width, wa.height, angle1, angle2);
      demo03_many_small_dynamic_triangles();
      //demo14_transparent_cube(wa.width, wa.height, angle1, angle2);
      //demo19_cubes(wa.width, wa.height, angle1, angle2);

      //demo24_draw_elements_terrain(wa.width, wa.height, angle1, angle2);
      //demo05_texture_3D(wa.width, wa.height, angle1, angle2);
      //demo25_teapot(wa.width, wa.height, angle1, angle2);

      //demo04_pyramid_and_cube_3d(wa.width, wa.height, 20, 30);


      angle1 += 50.0f/(FPS+1.0f);
      angle2 += 100.0f/(FPS+1.0f);

      frameCounter++;
      frameTime += g_timer.getElapsed();
      g_timer.start();

      if(frameCounter >= 10)
      {
        FPS = (float)frameCounter / frameTime;
        frameCounter = 0;
        frameTime    = 0.0f;

        char temp[64];
        sprintf(temp,"FPS = %f", FPS);
        XStoreName(dpy, win, temp);
      }

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
    win = XCreateWindow(dpy, root, 0, 0, 800, 600, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
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
//				TIME COUNTER FUNCTIONS				//
//////////////////////////////////////////////////////////////////////////////////
void InitTimeCounter()
{
    gettimeofday(&tv0, NULL);
    FramesPerFPS = 5;
}

void UpdateTimeCounter()
{
    LastFrameTimeCounter = TimeCounter;
    gettimeofday(&tv, NULL);
    TimeCounter = (float)(tv.tv_sec-tv0.tv_sec) + 0.000001*((float)(tv.tv_usec-tv0.tv_usec));
    DT = TimeCounter - LastFrameTimeCounter;
}

void CalculateFPS()
{
    Frame++;

    if((Frame%FramesPerFPS) == 0)
    {
        FPS = ((float)(FramesPerFPS)) / (TimeCounter-prevTime);
        prevTime = TimeCounter;
    }
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

    if(XCheckWindowEvent(dpy, win, KeyPressMask, &xev))
    {
        char	*key_string = XKeysymToString(XkbKeycodeToKeysym(dpy, xev.xkey.keycode, 0, 0));

        if(strncmp(key_string, "Left", 4) == 0)
        {
            rot_z_vel -= 200.0*DT;
        }

        else if(strncmp(key_string, "Right", 5) == 0)
        {
            rot_z_vel += 200.0*DT;
        }

        else if(strncmp(key_string, "Up", 2) == 0)
        {
            rot_y_vel -= 200.0*DT;
        }

        else if(strncmp(key_string, "Down", 4) == 0)
        {
            rot_y_vel += 200.0*DT;
        }

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
//				MAIN PROGRAM					//
//////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) // 222
{
    CreateWindow();
    SetupGL();
    InfoGL();
    InitTimeCounter();

    while(true)
    {
        UpdateTimeCounter();
        CalculateFPS();

        ExposeFunc();
        //usleep(1000);
        CheckKeyboard();
    }
}


#endif




