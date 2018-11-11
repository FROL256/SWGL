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
    g_localHeight = wa.height;
    g_localWidth  = wa.width;

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

    // cvex::vfloat4 src1 = {1.0f, 2.0f, 3.0f, 4.0f};
    // cvex::vfloat4 src2 = {5.0f, 6.0f, 7.0f, 8.0f};
    // cvex::vfloat4 shf1 = _mm_shuffle_ps(src1, src2, _MM_SHUFFLE(1, 0, 1, 0));
    // cvex::vfloat4 shf2 = _mm_shuffle_ps(src1, src2, _MM_SHUFFLE(3, 2, 1, 0));
    // int a = 2;

    try
    {
      //test01_colored_triangle();
      //demo01_colored_triangle(angle1);
      //test02_nehe_lesson1_simplified();

      //demo04_pyramid_and_cube_3d(wa.width, wa.height, 10.0f, 13.0f);
      //demo04_pyramid_and_cube_3d(wa.width, wa.height, angle1, angle2);
      //demo14_transparent_cube(wa.width, wa.height, 10.0f, 13.0f);
      //demo03_many_small_dynamic_triangles();

      //test11_alpha_tex_and_transp();
      //test12_rect_tex();

      //demo14_transparent_cube(wa.width, wa.height, angle1, angle2);
      demo05_texture_3D(wa.width, wa.height, angle1, angle2);
      //demo19_cubes(wa.width, wa.height, angle1, angle2);

      //demo24_draw_elements_terrain(wa.width, wa.height, angle1, angle2);
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
    win = XCreateWindow(dpy, root, 0, 0, 1024, 1024, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
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




