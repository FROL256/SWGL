#include <windows.h>

#ifdef USE_SWGL
  #include "../gl_sc_swr/gl_sc.h"
  #include "../gl_sc_swr/gl_std.h"
#else
  #include <gl\gl.h>
  #include <gl\glu.h>
#endif

#ifndef NO_LOCAL_MATH
  #include "../gl_sc_swr/local_math.h"
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>

#include "Timer.h"
#include "tests_sc1.h"


HDC			hDC=NULL;		
HGLRC		hRC=NULL;		
HWND		hWnd=NULL;	
HINSTANCE	hInstance;

bool	keys[256];			
bool	active=TRUE;		
bool	fullscreen=TRUE;

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int g_width = 0;
int g_height = 0;

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height==0)									
		height=1;										  

	glViewport(0,0,width,height);		

	glMatrixMode(GL_PROJECTION);		
	glLoadIdentity();								

	glMatrixMode(GL_MODELVIEW);			
	glLoadIdentity();			


  g_width  = width;
  g_height = height;
}

int InitGL(GLvoid)								
{
	glClearColor(0.0f, 0.0f, 0.25f, 0.5f);			

	return TRUE;										          
}

GLfloat	rtri  = 25.0f;				// Angle For The Triangle ( NEW )
GLfloat	rquad = 40.0f;
float   g_FPS = 60.0f;

extern int g_localWidth;
extern int g_localHeight;

int DrawGLScene(GLvoid)									    
{
  static Timer timer(true);
  static int frameCounter = 1;

  if (frameCounter == 1)
  {
    g_localWidth  = g_width;
    g_localHeight = g_height;

    // std::cout << "begin tests --> " << std::endl;
    // test_all(g_width, g_height);
    // std::cout << "end   tests <-- " << std::endl;
  }

  //demo03_many_small_dynamic_triangles();
  demo04_pyramid_and_cube_3d(g_width, g_height, rtri, rquad); // rtri, rquad // 200.0f, -100.0f

  //demo05_texture_3D(g_width, g_height, rtri, rquad);
  //demo07_triangle_fan(g_width, g_height, rtri, rquad);
  //demo14_transparent_cube(g_width, g_height, rtri, rquad);
  //demo24_draw_elements_terrain(g_width, g_height, rtri, rquad);
  //demo19_cubes(g_width, g_height, rtri, rquad);

  //demo25_teapot(g_width, g_height, rtri, rquad);
  //demo26_teapots9(g_width, g_height, rtri, rquad);

  //test03_several_triangles();

  //test17_line_strip();
  //test15_simple_stencil();

  //test_box_tri_overlap();
  //test23_draw_elements();

  //test22_change_viewort_size();

  //demo04_pyramid_and_cube_3d(g_width, g_height, 20, 30);

  const float coeff = 100.0f/ fmax(g_FPS, 1.0f);

  rtri  += coeff*0.2f;											// Increase The Rotation Variable For The Triangle ( NEW )
  rquad -= coeff*0.15f;										  // Decrease The Rotation Variable For The Quad ( NEW )
 
  glFlush();

  if (frameCounter % 10 == 0)
  {
    std::stringstream out; out.precision(4);
    g_FPS = (10.0f / timer.getElapsed());
    out << "FPS = " << g_FPS;
    //out << rtri << " " << rquad << std::endl;
    SetWindowText(hWnd, out.str().c_str());
    timer.start();
  }

  frameCounter++;
	
	return TRUE;									
}

GLvoid KillGLWindow(GLvoid)						
{
	if (fullscreen)										  
	{
		ChangeDisplaySettings(NULL,0);		
		ShowCursor(TRUE);								  
	}

	if (hRC)											      
	{
		if (!wglMakeCurrent(NULL,NULL))		
		{
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass("OpenGL",hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;									// Set hInstance To NULL
	}
}


 
BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;			// Set Left Value To 0
	WindowRect.right=(long)width;		// Set Right Value To Requested Width
	WindowRect.top=(long)0;				// Set Top Value To 0
	WindowRect.bottom=(long)height;		// Set Bottom Value To Requested Height

	fullscreen=fullscreenflag;			// Set The Global Fullscreen Flag

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}
	
	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullscreen=FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}

	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle=WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle=WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle=WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd=CreateWindowEx(	dwExStyle,							// Extended Style For The Window
								"OpenGL",							// Class Name
								title,								// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								0, 0,								// Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								hInstance,							// Instance
								NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |	  // Format Must Support Window
		PFD_SUPPORT_OPENGL |		// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,				// Must Support Double Buffering
		PFD_TYPE_RGBA,					// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,				// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,						  // Accumulation Bits Ignored
		32,											// 32 Bit Z-Buffer (Depth Buffer)  
		8,											// 8 bit Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,					// Main Drawing Layer
		0,											// Reserved
		0, 0, 0								  // Layer Masks Ignored
	};
	
	if (!(hDC=GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC=wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!wglMakeCurrent(hDC,hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	return TRUE;									// Success
}

LRESULT CALLBACK WndProc(	HWND	  hWnd,			  // Handle For This Window
							            UINT	  uMsg,			  // Message For This Window
							            WPARAM	wParam,			// Additional Message Information
							            LPARAM	lParam)			// Additional Message Information
{
	switch (uMsg)									  // Check For Windows Messages
	{
		case WM_ACTIVATE:							// Watch For Window Activate Message
		{
			if (!HIWORD(wParam))				// Check Minimization State
			{
				active=TRUE;						  // Program Is Active
			}
			else
			{
				active=FALSE;						  // Program Is No Longer Active
			}

			return 0;								    // Return To The Message Loop
		}

		case WM_SYSCOMMAND:
		{
			switch (wParam)
			{
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:
					return 0;
			}
			break;
		}

		case WM_CLOSE:						
		{
			PostQuitMessage(0);			
			return 0;								
		}

		case WM_KEYDOWN:					
		{
			keys[wParam] = TRUE;		
			return 0;								
		}

		case WM_KEYUP:						
		{
			keys[wParam] = FALSE;		
			return 0;								
		}

		case WM_SIZE:							
		{
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));  
			return 0;								
		}
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

int WINAPI WinMain(	HINSTANCE	hInstance,HINSTANCE	hPrevInstance, LPSTR lpCmdLine, int	nCmdShow)		
{
	MSG		msg;									
	BOOL	done=FALSE;						
  fullscreen=FALSE;						


	// Create Our OpenGL Window
	if (!CreateGLWindow("OpenGL SWR Demo", 1024, 1024, 32, fullscreen))
	{
		return 0;									// Quit If Window Was Not Created
	}

	while(!done)									// Loop That Runs While done=FALSE
	{
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message==WM_QUIT)				// Have We Received A Quit Message?
			{
				done=TRUE;							// If So done=TRUE
			}
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else										// If There Are No Messages
		{
			                                                    // Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			if ((active && !DrawGLScene()) || keys[VK_ESCAPE])	// Active?  Was There A Quit Received?
			{
				done=TRUE;							                          // ESC or DrawGLScene Signalled A Quit
			}
			else									                              // Not Time To Quit, Update Screen
			{
				SwapBuffers(hDC);					// Swap Buffers (Double Buffering)
			}
		}
	}

	// Shutdown
	KillGLWindow();									// Kill The Window
	return (msg.wParam);							// Exit The Program
}
