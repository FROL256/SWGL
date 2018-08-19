#include "tests_sc1.h"
#include "../gl_sc_swr/LiteMath.h"

#ifdef WIN32
  #include <windows.h>
#else
#define USE_SWGL
#endif

#ifdef USE_SWGL
#include "../gl_sc_swr/gl_sc.h"
#include "../gl_sc_swr/gl_std.h"
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdint>

#include <memory.h>
#include <stdexcept>

#pragma warning(disable:4996)
void ThrowExceptionOnGLError(int line, const char *file)
{

  static char errMsg[512];

  GLenum gl_error = glGetError();

  if(gl_error == GL_NO_ERROR)
    return;

  switch(gl_error)
  {
  case GL_INVALID_ENUM:
    sprintf(errMsg, "GL_INVALID_ENUM file %s line %d\n", file, line);
    break;

  case GL_INVALID_VALUE:
    sprintf(errMsg, "GL_INVALID_VALUE file %s line %d\n",  file, line);
    break;

  case GL_INVALID_OPERATION:
    sprintf(errMsg, "GL_INVALID_OPERATION file %s line %d\n",  file, line);
    break;

  case GL_STACK_OVERFLOW:
    sprintf(errMsg, "GL_STACK_OVERFLOW file %s line %d\n",  file, line);
    break;

  case GL_STACK_UNDERFLOW:
    sprintf(errMsg, "GL_STACK_UNDERFLOW file %s line %d\n",  file, line);
    break;

  case GL_OUT_OF_MEMORY:
    sprintf(errMsg, "GL_OUT_OF_MEMORY file %s line %d\n",  file, line);
    break;

  //case GL_TABLE_TOO_LARGE:
  //  sprintf(errMsg, "GL_TABLE_TOO_LARGE file %s line %d\n",  file, line);
  //  break;

  case GL_NO_ERROR:
    break;

  default:
    sprintf(errMsg, "Unknown error @ file %s line %d\n",  file, line);
    break;
  }

  if(gl_error != GL_NO_ERROR)
    throw std::runtime_error(errMsg);
}



void glhFrustumf3(float *matrix, float left, float right, float bottom, float top, float znear, float zfar)
{
  float temp, temp2, temp3, temp4;
  temp = 2.0f * znear;
  temp2 = right - left;
  temp3 = top - bottom;
  temp4 = zfar - znear;
  matrix[0] = temp / temp2;
  matrix[1] = 0.0;
  matrix[2] = 0.0;
  matrix[3] = 0.0;
  matrix[4] = 0.0;
  matrix[5] = temp / temp3;
  matrix[6] = 0.0;
  matrix[7] = 0.0;
  matrix[8] = (right + left) / temp2;
  matrix[9] = (top + bottom) / temp3;
  matrix[10] = (-zfar - znear) / temp4;
  matrix[11] = -1.0;
  matrix[12] = 0.0;
  matrix[13] = 0.0;
  matrix[14] = (-temp * zfar) / temp4;
  matrix[15] = 0.0;
}

// matrix will receive the calculated perspective matrix.
//You would have to upload to your shader
// or use glLoadMatrixf if you aren't using shaders.
void glhPerspectivef3(float *matrix, float fovy, float aspectRatio, float znear, float zfar)
{
  const float ymax = znear * tanf(fovy * 3.14159265358979323846f / 360.0f);
  const float xmax = ymax * aspectRatio;
  glhFrustumf3(matrix, -xmax, xmax, -ymax, ymax, znear, zfar);
}

void gluPerspective2(float fovy, float aspect, float zNear, float zFar)
{
  float matrixData[16];
  glhPerspectivef3(matrixData, fovy, aspect, zNear, zFar);

  int oldMatrixMode = GL_MODELVIEW;
  glGetIntegerv(GL_MATRIX_MODE, &oldMatrixMode);

  glMatrixMode(GL_PROJECTION);
  glMultMatrixf(matrixData);

  glMatrixMode(oldMatrixMode); //
}


struct Pixel
{
  unsigned char r, g, b;
};

#ifndef WIN32

typedef unsigned short WORD;
typedef unsigned long  DWORD;

#pragma pack(1)
struct BITMAPFILEHEADER
{
  WORD    bfType;
  DWORD   bfSize;
  WORD    bfReserved1;
  WORD    bfReserved2;
  DWORD   bfOffBits;
};

#pragma pack(1)
struct BITMAPINFOHEADER
{
  DWORD    biSize;
  DWORD    biWidth;
  DWORD    biHeight;
  WORD     biPlanes;
  WORD     biBitCount;
  DWORD    biCompression;
  DWORD    biSizeImage;
  DWORD    biXPelsPerMeter;
  DWORD    biYPelsPerMeter;
  DWORD    biClrUsed;
  DWORD    biClrImportant;
};

#endif

void WriteBMP(const wchar_t* fname, Pixel* a_pixelData, int width, int height)
{
  BITMAPFILEHEADER bmfh;
  BITMAPINFOHEADER info;

  memset(&bmfh, 0, sizeof(BITMAPFILEHEADER));
  memset(&info, 0, sizeof(BITMAPINFOHEADER));

  int paddedsize = (width*height)*sizeof(Pixel);

  bmfh.bfType      = 0x4d42; // 0x4d42 = 'BM'
  bmfh.bfReserved1 = 0;
  bmfh.bfReserved2 = 0;
  bmfh.bfSize      = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + paddedsize;
  bmfh.bfOffBits   = 0x36;

  info.biSize          = sizeof(BITMAPINFOHEADER);
  info.biWidth         = width;
  info.biHeight        = height;
  info.biPlanes        = 1;
  info.biBitCount      = 24;
  info.biCompression   = 0;
  info.biSizeImage     = 0;
  info.biXPelsPerMeter = 0x0ec4;
  info.biYPelsPerMeter = 0x0ec4;
  info.biClrUsed       = 0;
  info.biClrImportant  = 0;

#ifdef WIN32
  std::ofstream out(fname, std::ios::out | std::ios::binary);
#else
  std::wstring s1(fname);
  std::string  s2(s1.begin(), s1.end());
  std::ofstream out(s2.c_str(), std::ios::out | std::ios::binary);
#endif

  out.write((const char*)&bmfh, sizeof(BITMAPFILEHEADER));
  out.write((const char*)&info, sizeof(BITMAPINFOHEADER));
  out.write((const char*)a_pixelData, paddedsize);
  out.flush();
  out.close();
}

void SaveBMP(const wchar_t* fname, const int* pixels, int w, int h)
{
  std::vector<Pixel> pixels2(w*h);

  for (size_t i = 0; i < pixels2.size(); i++)
  {
    int pxData = pixels[i];
    Pixel px;
    px.r = (pxData & 0x00FF0000) >> 16;
    px.g = (pxData & 0x0000FF00) >> 8;
    px.b = (pxData & 0x000000FF);
    pixels2[i] = px;
  }

  WriteBMP(fname, &pixels2[0], w, h);
}

inline const int readInt(unsigned char* ptr) // THIS IS CORRECT BOTH FOR X86 AND PPC !!!
{
  const unsigned char b0 = ptr[0];
  const unsigned char b1 = ptr[1];
  const unsigned char b2 = ptr[2];
  const unsigned char b3 = ptr[3];

  return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}


unsigned char* ReadBMP(const char* filename, int* pW, int* pH)
{
    //int i;
    FILE* f = fopen(filename, "rb");

    if(f == NULL)
      throw "Argument Exception";

    unsigned char info[54];
    fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header

    // extract image height and width from header
    //int width  = *(int*)&info[18];
    //int height = *(int*)&info[22];

    int width  = readInt(&info[18]);
    int height = readInt(&info[22]);

    std::cout << std::endl;
    std::cout << "  Name: " << filename << std::endl;
    std::cout << " Width: " << width << std::endl;
    std::cout << "Height: " << height << std::endl;

    int row_padded = (width*3 + 3) & (~3);
    unsigned char* data  = new unsigned char[row_padded]; ///  THIS IS PIECI OF SHIT !!!!!!! <==== <---- !!!!!
    //unsigned char tmp;

    unsigned char* data2 = new unsigned char[width*height*3];

    for(int i = 0; i < height; i++)
    {
        fread(data, sizeof(unsigned char), row_padded, f);

        /*
        for(int j = 0; j < width*3; j += 3)
        {
            // Convert (B, G, R) to (R, G, B)
            tmp = data[j];
            data[j] = data[j+2];
            data[j+2] = tmp;
            //std::cout << "R: "<< (int)data[j] << " G: " << (int)data[j+1]<< " B: " << (int)data[j+2]<< std::endl;
        }
        */

        for(int j=0;j<width;j++)
        {
          int index = i*width+j;
          data2[index*3 + 0] = data[j*3+0];
          data2[index*3 + 1] = data[j*3+1];
          data2[index*3 + 2] = data[j*3+2];
        }

    }

    fclose(f);

    if(pW != 0)
      (*pW) = width;

    if(pH != 0)
      (*pH) = height;

    delete [] data;

    return data2;
}


std::vector<int> LoadBMP(const wchar_t* fname, int* w, int* h)
{

  BITMAPFILEHEADER bmfh;
  BITMAPINFOHEADER info;

  memset(&bmfh, 0, sizeof(BITMAPFILEHEADER));
  memset(&info, 0, sizeof(BITMAPINFOHEADER));

  std::wstring  s1(fname);
  std::string   s2(s1.begin(), s1.end());
  std::ifstream in(s2.c_str(), std::ios::binary);

  if (!in.is_open())
  {
    std::cerr << "LoadBMP: can't open file" << std::endl;
    (*w) = 0;
    (*h) = 0;
    return std::vector<int>();
  }

  in.close();

  int testW = 0, testH = 0;

  unsigned char* bitdata24 = ReadBMP(s2.c_str(),&testW,&testH);


  /*
  in.read((char*)&bmfh, sizeof(BITMAPFILEHEADER));
  in.read((char*)&info, sizeof(BITMAPINFOHEADER));

  if (bmfh.bfType != 0x4D42)
  {
    std::cerr << "LoadBMP: bad header (bmfh.bfType != 0x4D42)" << std::endl;
    (*w) = 0;
    (*h) = 0;
    return std::vector<int>();
  }

  if (info.biSizeImage == 0)
  {
    std::cerr << "LoadBMP: bad header (info.biSizeImage == 0)" << std::endl;
    (*w) = 0;
    (*h) = 0;
    return std::vector<int>();
  }

  if (info.biWidth == 0 || info.biHeight == 0)
  {
    std::cerr << "LoadBMP: bad header (info.biWidth/info.biHeight)" << std::endl;
    (*w) = 0;
    (*h) = 0;
    return std::vector<int>();
  }

  (*w) = info.biWidth;
  (*h) = info.biHeight;

  std::vector<uint8_t> pixels(info.biSizeImage);

  // Go to where image data starts, then read in image data
  in.seekg(bmfh.bfOffBits);
  in.read((char*)&pixels[0], info.biSizeImage);
  */

  std::vector<int> res(testW*testH);
  uint8_t* pixels2 = (uint8_t*)&res[0];

  uint8_t* pixels  = (uint8_t*)bitdata24;

  for (size_t i = 0; i < res.size(); i++)
  {
    pixels2[i * 4 + 0] = pixels[i * 3 + 2];
    pixels2[i * 4 + 1] = pixels[i * 3 + 1];
    pixels2[i * 4 + 2] = pixels[i * 3 + 0];
    pixels2[i * 4 + 3] = 255;
  }

  delete [] bitdata24;

  if(w != 0)
    (*w) = testW;

  if(h != 0)
    (*h) = testH;

  return res;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int g_localWidth;
int g_localHeight;

typedef void(*TestFunc)();

void test_all(int width, int height)
{
  g_localWidth  = width;
  g_localHeight = height;


  TestFunc tests[] = { &test01_colored_triangle,
                       &test02_nehe_lesson1_simplified,
                       &test03_several_triangles,
                       &test04_pyramid_and_cube_3d,
                       &test05_texture,
                       &test06_triangle_line,
                       &test07_triangle_fan,
                       &test08_vert_pointer1,
                       &test09_vert_pointer2_and_several_textures,
                       &test10_load_matrix_mult_matrix,
                       &test11_alpha_tex_and_transp,
                       &test12_rect_tex,
                       &test13_lines,
                       &test14_transparent_cube,
                       &test15_simple_stencil,
                       &test16_tri_strip,
                       &test17_line_strip,
                       &test18_line_points,
                       &test19_push_pop_matrix,
                       &test20_glftustum,
                       &test21_clip_2d,
                       &test22_change_viewort_size,
                       &test23_draw_elements };


  int testNum = sizeof(tests) / sizeof(TestFunc);

  std::vector<int>   pixels1(width*height);

  for (int i = 0; i < testNum; i++)
  {
    std::wstringstream fileNameOut;

    fileNameOut.fill('0');
    fileNameOut.width(2);
    fileNameOut << i;

#ifdef USE_SWGL

    std::wstring fileName = std::wstring(L"z_test_out\\z_test_") + fileNameOut.str() + L".bmp";
#else

    std::wstring fileName = std::wstring(L"z_test_out\\z_ref_") + fileNameOut.str() + L".bmp";

#endif

    tests[i]();

    glFlush();

    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)&pixels1[0]);
    SaveBMP(fileName.c_str(), &pixels1[0], width, height);

  }

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void test01_colored_triangle()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0.0f, 0.0f, 0.25f, 0.5f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBegin(GL_TRIANGLES);
  glColor4f(1, 0, 0, 1);
  glVertex2f(-0.5f, -0.5f);

  glColor4f(0, 1, 0, 1);
  glVertex2f(0.0f, 0.5f);

  glColor4f(0, 0, 1, 1);
  glVertex2f(0.5f, -0.5f);
  glEnd();

}

void demo01_colored_triangle(float rtri)
{

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0.0f, 0.0f, 0.25f, 0.5f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glRotatef(rtri, 0.0f, 0.0f, 1.0f);

  glBegin(GL_TRIANGLES);
  glColor4f(1, 0, 0, 1);
  glVertex2f(-0.433012f, -0.25f);

  glColor4f(0, 1, 0, 1);
  glVertex2f(0.0f, 0.5f);

  glColor4f(0, 0, 1, 1);
  glVertex2f(0.433012f, -0.25f);
  glEnd();

}


void test02_nehe_lesson1_simplified()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glColor4f(1, 1, 1, 1);

  glTranslatef(-0.5f, 0.25f, 0.0f);

  const float polySize = 0.4f;

  glBegin(GL_TRIANGLES);
  glVertex3f(0.0f, polySize, 0.0f);
  glVertex3f(-polySize, -polySize, 0.0f);
  glVertex3f(polySize, -polySize, 0.0f);
  glEnd();

  glTranslatef(1.0f, -0.5f, 0.0f);

  glBegin(GL_QUADS);
  glVertex3f(-polySize, polySize, 0.0f);
  glVertex3f(polySize, polySize, 0.0f);
  glVertex3f(polySize, -polySize, 0.0f);
  glVertex3f(-polySize, -polySize, 0.0f);
  glEnd();

}



void test03_several_triangles()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0.0f, 0.0f, 0.25f, 0.5f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glColor4f(0.15f, 1.0f, 0.15f, 1);

  const float sz  = 0.025f;
  const float eps = 0.001f;

  glBegin(GL_TRIANGLES);

  for (float y = -0.9f; y <= 0.0f + eps; y += 0.1f)
  {
    for (float x = -0.9f; x <= 0.9f + eps; x += 0.1f)
    {
      glVertex3f(x, y + sz, 0.0f);
      glVertex3f(x - sz, y - sz, 0.0f);
      glVertex3f(x + sz, y - sz, 0.0f);
    }
  }

  glEnd();

  for (float y = 0.1f; y <= 0.4f + eps; y += 0.1f)
  {
    for (float x = -0.9f; x <= 0.9f + eps; x += 0.1f)
    {
      glBegin(GL_TRIANGLES);
      glVertex3f(x, y + sz, 0.0f);
      glVertex3f(x - sz, y - sz, 0.0f);
      glVertex3f(x + sz, y - sz, 0.0f);
      glEnd();
    }
  }

  glBegin(GL_TRIANGLES);

  for (float y = 0.6f; y <= 0.9f + eps; y += 0.1f)
  {
    for (float x = -0.9f; x <= 0.9f + eps; x += 0.1f)
    {
      glVertex3f(x, y + sz, 0.0f);
      glVertex3f(x - sz, y - sz, 0.0f);
      glVertex3f(x + sz, y - sz, 0.0f);
    }
  }

  glEnd();


}

void test04_pyramid_and_cube_3d()
{
  demo04_pyramid_and_cube_3d(g_localWidth, g_localHeight, 30.0f, 50.0f);
}

void test05_texture()
{
  demo05_texture_3D(g_localWidth, g_localHeight, 80.0f, 85.0f);
}

void test07_triangle_fan()
{
  demo07_triangle_fan(g_localWidth, g_localHeight, 150.0f, 45.0f);
}

void test06_triangle_line()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);

  glBegin(GL_TRIANGLES);
  glColor4f(1, 0, 0, 1);
  glVertex2f(-0.5f, -0.5f);

  glColor4f(0, 1, 0, 1);
  glVertex2f(0.0f, -0.495f);

  glColor4f(0, 0, 1, 1);
  glVertex2f(0.5f, -0.5f);
  glEnd();

}


void test08_vert_pointer1()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);

  glColor4f(0, 1, 0, 1);

  GLfloat vertices[] = { 25.0f / 512.0f, 25.0f / 512.0f,
                         100.0f / 512.0f, 325.0f / 512.0f,
                         175.0f / 512.0f, 25.0f / 512.0f,
                         175.0f / 512.0f, 325.0f / 512.0f,
                         250.0f / 512.0f, 25.0f / 512.0f,
                         325.0f / 512.0f, 325.0f / 512.0f };

  GLfloat colors[] = { 1.0f,  0.2f,  0.2f,
                       0.2f,  0.2f,  1.0f,
                       0.8f,  1.0f,  0.2f,
                       0.75f, 0.75f, 0.75f,
                       0.35f, 0.35f, 0.35f,
                       0.5f,  0.5f,  0.5f };

  glEnableClientState(GL_COLOR_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);
  glColorPointer(3, GL_FLOAT, 0, colors);
  glVertexPointer(2, GL_FLOAT, 0, vertices);

  glDrawArrays(GL_TRIANGLES, 0, 6);


  const int coordsNum = sizeof(vertices) / sizeof(GLfloat);
  for (int i = 0; i < coordsNum; i++)
    vertices[i] -= 0.65f;

  glDisableClientState(GL_COLOR_ARRAY);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

}




void demo04_pyramid_and_cube_3d(int width, int height, float rtri, float rquad)
{
  glMatrixMode(GL_PROJECTION);			// Select The Projection Matrix
  glLoadIdentity();									// Reset The Projection Matrix

  // Calculate The Aspect Ratio Of The Window
  gluPerspective2(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

  glMatrixMode(GL_MODELVIEW);			  // Select The Modelview Matrix
  glLoadIdentity();									// Reset The Modelview Matrix

  glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
  glClearColor(0.0f, 0.0f, 0.0f, 0.5f);	// Black Background
  glClearDepth(1.0f);									  // Depth Buffer Setup
  glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
  glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer

  glDisable(GL_TEXTURE_2D);


  glLoadIdentity();									// Reset The Current Modelview Matrix
  glTranslatef(-1.5f, 0.0f, -6.0f);						// Move Left 1.5 Units And Into The Screen 6.0
  glRotatef(rtri, 0.0f, 1.0f, 0.0f);						// Rotate The Triangle On The Y axis ( NEW )
  glBegin(GL_TRIANGLES);								// Start Drawing A Triangle
  glColor3f(1.0f, 0.0f, 0.0f);						// Red
  glVertex3f(0.0f, 1.0f, 0.0f);					// Top Of Triangle (Front)
  glColor3f(0.0f, 1.0f, 0.0f);						// Green
  glVertex3f(-1.0f, -1.0f, 1.0f);					// Left Of Triangle (Front)
  glColor3f(0.0f, 0.0f, 1.0f);						// Blue
  glVertex3f(1.0f, -1.0f, 1.0f);					// Right Of Triangle (Front)
  glColor3f(1.0f, 0.0f, 0.0f);						// Red
  glVertex3f(0.0f, 1.0f, 0.0f);					// Top Of Triangle (Right)
  glColor3f(0.0f, 0.0f, 1.0f);						// Blue
  glVertex3f(1.0f, -1.0f, 1.0f);					// Left Of Triangle (Right)
  glColor3f(0.0f, 1.0f, 0.0f);						// Green
  glVertex3f(1.0f, -1.0f, -1.0f);					// Right Of Triangle (Right)
  glColor3f(1.0f, 0.0f, 0.0f);						// Red
  glVertex3f(0.0f, 1.0f, 0.0f);					// Top Of Triangle (Back)
  glColor3f(0.0f, 1.0f, 0.0f);						// Green
  glVertex3f(1.0f, -1.0f, -1.0f);					// Left Of Triangle (Back)
  glColor3f(0.0f, 0.0f, 1.0f);						// Blue
  glVertex3f(-1.0f, -1.0f, -1.0f);					// Right Of Triangle (Back)
  glColor3f(1.0f, 0.0f, 0.0f);						// Red
  glVertex3f(0.0f, 1.0f, 0.0f);					// Top Of Triangle (Left)
  glColor3f(0.0f, 0.0f, 1.0f);						// Blue
  glVertex3f(-1.0f, -1.0f, -1.0f);					// Left Of Triangle (Left)
  glColor3f(0.0f, 1.0f, 0.0f);						// Green
  glVertex3f(-1.0f, -1.0f, 1.0f);					// Right Of Triangle (Left)
  glEnd();											// Done Drawing The Pyramid


  glLoadIdentity();									// Reset The Current Modelview Matrix
  glTranslatef(1.5f, 0.0f, -7.0f);						// Move Right 1.5 Units And Into The Screen 7.0
  glRotatef(rquad, 1.0f, 1.0f, 1.0f);					// Rotate The Quad On The X axis ( NEW )
  glBegin(GL_QUADS);									// Draw A Quad
  glColor3f(0.0f, 1.0f, 0.0f);						// Set The Color To Green
  glVertex3f(1.0f, 1.0f, -1.0f);					// Top Right Of The Quad (Top)
  glVertex3f(-1.0f, 1.0f, -1.0f);					// Top Left Of The Quad (Top)
  glVertex3f(-1.0f, 1.0f, 1.0f);					// Bottom Left Of The Quad (Top)
  glVertex3f(1.0f, 1.0f, 1.0f);					// Bottom Right Of The Quad (Top)
  glColor3f(1.0f, 0.5f, 0.0f);						// Set The Color To Orange
  glVertex3f(1.0f, -1.0f, 1.0f);					// Top Right Of The Quad (Bottom)
  glVertex3f(-1.0f, -1.0f, 1.0f);					// Top Left Of The Quad (Bottom)
  glVertex3f(-1.0f, -1.0f, -1.0f);					// Bottom Left Of The Quad (Bottom)
  glVertex3f(1.0f, -1.0f, -1.0f);					// Bottom Right Of The Quad (Bottom)
  glColor3f(1.0f, 0.0f, 0.0f);						// Set The Color To Red
  glVertex3f(1.0f, 1.0f, 1.0f);					// Top Right Of The Quad (Front)
  glVertex3f(-1.0f, 1.0f, 1.0f);					// Top Left Of The Quad (Front)
  glVertex3f(-1.0f, -1.0f, 1.0f);					// Bottom Left Of The Quad (Front)
  glVertex3f(1.0f, -1.0f, 1.0f);					// Bottom Right Of The Quad (Front)
  glColor3f(1.0f, 1.0f, 0.0f);						// Set The Color To Yellow
  glVertex3f(1.0f, -1.0f, -1.0f);					// Top Right Of The Quad (Back)
  glVertex3f(-1.0f, -1.0f, -1.0f);					// Top Left Of The Quad (Back)
  glVertex3f(-1.0f, 1.0f, -1.0f);					// Bottom Left Of The Quad (Back)
  glVertex3f(1.0f, 1.0f, -1.0f);					// Bottom Right Of The Quad (Back)
  glColor3f(0.0f, 0.0f, 1.0f);						// Set The Color To Blue
  glVertex3f(-1.0f, 1.0f, 1.0f);					// Top Right Of The Quad (Left)
  glVertex3f(-1.0f, 1.0f, -1.0f);					// Top Left Of The Quad (Left)
  glVertex3f(-1.0f, -1.0f, -1.0f);					// Bottom Left Of The Quad (Left)
  glVertex3f(-1.0f, -1.0f, 1.0f);					// Bottom Right Of The Quad (Left)
  glColor3f(1.0f, 0.0f, 1.0f);						// Set The Color To Violet
  glVertex3f(1.0f, 1.0f, -1.0f);					// Top Right Of The Quad (Right)
  glVertex3f(1.0f, 1.0f, 1.0f);					// Top Left Of The Quad (Right)
  glVertex3f(1.0f, -1.0f, 1.0f);					// Bottom Left Of The Quad (Right)
  glVertex3f(1.0f, -1.0f, -1.0f);					// Bottom Right Of The Quad (Right)
  glEnd();											// Done Drawing The Quad

}

void demo05_texture_3D(int width, int height, float algle1, float angle2)
{
  const float rtri = algle1;
  const float rquad = angle2;

  static bool firstFrame = true;
  static GLuint texture = (GLuint)(-1);


  if (firstFrame)
  {
    int w, h;
    std::vector<int> pixels = LoadBMP(L"data/texture1.bmp", &w, &h);

    glGenTextures(1, &texture);					// Create The Texture

    // Typical Texture Generation Using Data From The Bitmap
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    firstFrame = false;
  }


  glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
  glLoadIdentity();									// Reset The Projection Matrix

  // Calculate The Aspect Ratio Of The Window
  gluPerspective2(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

  glMatrixMode(GL_MODELVIEW);			  // Select The Modelview Matrix
  glLoadIdentity();									// Reset The Modelview Matrix

  glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
  glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
  glClearDepth(1.0f);									// Depth Buffer Setup
  glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
  glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations


  glEnable(GL_TEXTURE_2D);
  //glDisable(GL_TEXTURE_2D);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer

  //
  //

  const float xrot = algle1*0.75f;
  const float yrot = angle2*0.25f;
  const float zrot = -angle2*0.5f;

  glTranslatef(0.0f, 0.0f, -5.0f);

  glRotatef(xrot, 1.0f, 0.0f, 0.0f);
  glRotatef(yrot, 0.0f, 1.0f, 0.0f);
  glRotatef(zrot, 0.0f, 0.0f, 1.0f);

  glBindTexture(GL_TEXTURE_2D, texture);
  glColor3f(1.0f, 1.0f, 1.0f);
  //glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  glBegin(GL_QUADS);
  // Front Face
  glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
  glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
  glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f);
  glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
  // Back Face
  glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
  glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
  glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
  glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
  // Top Face
  glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
  glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
  glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 1.0f, 1.0f);
  glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
  // Bottom Face
  glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
  glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, -1.0f, -1.0f);
  glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
  glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
  // Right face
  glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
  glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
  glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f);
  glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
  // Left Face
  glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
  glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
  glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
  glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
  glEnd();



}



void demo07_triangle_fan(int width, int height, const float xRot, const float yRot)
{
  const float aspect = float(width) / float(height);
  const float GL_PI = 3.14159265359f;

  // Black background
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);


  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glTranslatef(-0.5f, 0.0f, 0.0f);
  glScalef(0.0075f, 0.0075f*aspect, 0.0f);
  glRotatef(yRot, 0.0f, 0.0f, 1.0f);

  glBegin(GL_TRIANGLE_FAN);

  // Center of fan is at the origin
  glColor3f(0.0f, 1.0f, 0.0f);
  glVertex2f(0.0f, 0.0f);

  int iPivot = 1;
  int counter = 0;
  for (float angle = 0.0f; angle < (2.0f*GL_PI) + 1e-4f; angle += (GL_PI / 8.0f))
  {
    // Calculate x and y position of the next vertex
    GLfloat x = 50.0f*sin(angle);
    GLfloat y = 50.0f*cos(angle);

    // Alternate color between red and green
    if ((iPivot % 2) == 0)
      glColor3f(0.0f, 1.0f, 0.0f);
    else
      glColor3f(1.0f, 0.0f, 0.0f);

    // Increment pivot to change color next time
    iPivot++;

    // Specify the next vertex for the triangle fan
    glVertex2f(x, y);
    counter++;
  }

  // Done drawing the fan that covers the bottom
  glEnd();


  //////////////////////////////////////////////////////////////////////////////////////////////////////////


  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glTranslatef(0.5f, 0.0f, 0.0f);
  glScalef(0.0075f, 0.0075f*aspect, 0.0f);
  glRotatef(xRot, 0.0f, 0.0f, 1.0f);

  glBegin(GL_TRIANGLE_FAN);

  // Center of fan is at the origin
  glColor3f(0.0f, 1.0f, 0.0f);
  glVertex2f(0.0f, 0.0f);

  iPivot = 1;
  counter = 0;
  for (float angle = 0.0f; angle < (2.0f*GL_PI) - 1e-4f; angle += (GL_PI / 8.0f))
  {
    // Calculate x and y position of the next vertex
    GLfloat x = 50.0f*sin(angle);
    GLfloat y = 50.0f*cos(angle);

    // Alternate color between red and green
    if ((iPivot % 2) == 0)
      glColor3f(0.0f, 1.0f, 0.0f);
    else
      glColor3f(1.0f, 0.0f, 0.0f);

    // Increment pivot to change color next time
    iPivot++;

    // Specify the next vertex for the triangle fan
    glVertex2f(x, y);
    counter++;
  }

  // Done drawing the fan that covers the bottom
  glEnd();

}


void demo03_many_small_dynamic_triangles()
{
  const static int N = 8192;

  static std::vector<GLfloat> vertices(N*6);
  static std::vector<GLfloat> colors(N*9);
  static bool firstFrame = true;

  if (firstFrame)
  {
    for (int i = 0; i < N; i++)
    {
      float cx = rnd(-1.0f, 1.0f);
      float cy = rnd(-1.0f, 1.0f);

      const float size = 0.025f;

      float3 v1(cx + size*rnd(-1.0f, 1.0f), cy + size*rnd(-1.0f, 1.0f), 0.0f);
      float3 v2(cx + size*rnd(-1.0f, 1.0f), cy + size*rnd(-1.0f, 1.0f), 0.0f);
      float3 v3(cx + size*rnd(-1.0f, 1.0f), cy + size*rnd(-1.0f, 1.0f), 0.0f);

      float3 n = normalize(cross(v1-v3, v2-v3));

      if (n.z < 0.0f)
        std::swap(v1, v2);

      vertices[i * 6 + 0] = v1.x;
      vertices[i * 6 + 1] = v1.y;

      vertices[i * 6 + 2] = v2.x;
      vertices[i * 6 + 3] = v2.y;
      
      vertices[i * 6 + 4] = v3.x;
      vertices[i * 6 + 5] = v3.y;

      for (int j = 0; j<9;j++)
        colors[i*9+j] = rnd(0.0f, 1.0f) > 0.5f ? 1.0f : 0.0f;
    }

    firstFrame = false;
  }


  glClear(GL_COLOR_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);

  glColor4f(1, 1, 1, 1);


  glEnableClientState(GL_COLOR_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);

  glColorPointer(3, GL_FLOAT, 0, &colors[0]);
  glVertexPointer(2, GL_FLOAT, 0, &vertices[0]);

  glDrawArrays(GL_TRIANGLES, 0, N*3);

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

}


void test09_vert_pointer2_and_several_textures()
{
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0.0f, 0.0f, 0.25f, 0.5f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBegin(GL_TRIANGLES);
  glColor4f(1, 0, 0, 1);
  glVertex2f(-0.5f, -0.5f);

  glColor4f(0, 1, 0, 1);
  glVertex2f(0.0f, 0.5f);

  glColor4f(0, 0, 1, 1);
  glVertex2f(0.5f, -0.5f);
  glEnd();


  int width = g_localWidth;
  int height = g_localHeight;

  const float aspect = float(width) / float(height);
  const float GL_PI = 3.14159265359f;

  static bool firstFrame = true;

  static GLuint texture1 = (GLuint)(-1);
  static GLuint texture2 = (GLuint)(-1);
  static GLuint texture3 = (GLuint)(-1);

  if (firstFrame)
  {
    int w, h;
    std::vector<int> pixels = LoadBMP(L"data/texture1.bmp", &w, &h);

    glGenTextures(1, &texture1);					// Create The Texture

    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int w2, h2;
    std::vector<int> pixels2 = LoadBMP(L"data/chess.bmp", &w2, &h2);

    glGenTextures(1, &texture2);					// Create The Texture

    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w2, h2, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels2[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int w3, h3;
    std::vector<int> pixels3 = LoadBMP(L"data/RedBrick.bmp", &w3, &h3);

    glGenTextures(1, &texture3);					// Create The Texture

    glBindTexture(GL_TEXTURE_2D, texture3);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w3, h3, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels3[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    firstFrame = false;
  }

  CHECK_GL_ERRORS;

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);

  //glDisable(GL_TEXTURE_2D);



  std::vector<float> positions; positions.reserve(400);
  std::vector<float> texCoords; texCoords.reserve(100);

  positions.push_back(0.0f);
  positions.push_back(0.0f);
  positions.push_back(1.0f);
  positions.push_back(1.0f);

  texCoords.push_back(0.5f);
  texCoords.push_back(0.5f);

  int counter = 0;
  for (float angle = 0.0f; angle < (2.0f*GL_PI) + 1e-4f; angle += (GL_PI / 8.0f))
  {
    // Calculate x and y position of the next vertex
    GLfloat x = 50.0f*sin(angle);
    GLfloat y = 50.0f*cos(angle);

    // Specify the next vertex for the triangle fan
    //
    positions.push_back(x);
    positions.push_back(y);
    positions.push_back(0.0f);
    positions.push_back(1.0f);

    texCoords.push_back(x / 50.0f);
    texCoords.push_back(y / 50.0f);
    counter++;
  }



  //
  //
  glColor3f(1.0f, 1.0f, 1.0f);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glTranslatef(0.5f, 0.25f, 0.0f);
  glScalef(0.0075f, 0.0075f*aspect, 0.0f);
  glRotatef(-60.0f, 0.0f, 0.0f, 1.0f);

  glEnableClientState(GL_TEXTURE_COORD_ARRAY);  CHECK_GL_ERRORS;
  glEnableClientState(GL_VERTEX_ARRAY);         CHECK_GL_ERRORS;

  // draw 1 circle
  //
  glColor3f(1.0f, 1.0f, 1.0f);
  glBindTexture(GL_TEXTURE_2D, texture1);

  glTexCoordPointer(2, GL_FLOAT, 0, &texCoords[0]);  CHECK_GL_ERRORS;
  glVertexPointer(4, GL_FLOAT, 0, &positions[0]);    CHECK_GL_ERRORS;
  glDrawArrays(GL_TRIANGLE_FAN, 0, counter + 1);     CHECK_GL_ERRORS;


  // draw 2 circle
  //
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glTranslatef(-0.5f, 0.25f, 0.0f);
  glScalef(0.0055f, 0.0055f*aspect, 0.0f);
  glRotatef(120.0f, 0.0f, 0.0f, 1.0f);

  glBindTexture(GL_TEXTURE_2D, texture2);
  glDrawArrays(GL_TRIANGLE_FAN, 0, counter + 1);

  // draw 2 circle
  //
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glTranslatef(0.0f, -0.5f, 0.0f);
  glScalef(0.0065f, 0.0065f*aspect, 0.0f);
  glRotatef(120.0f, 0.0f, 0.0f, 1.0f);

  glBindTexture(GL_TEXTURE_2D, texture3);
  glDrawArrays(GL_TRIANGLE_FAN, 0, counter + 1);

  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
}


void test10_load_matrix_mult_matrix()
{
  float m1[16] = { 0.5258428f, 0.11663668f, 0.0f, 0.0f,
                  -0.12218947f, 0.59435131f,  0.0f, 0.0f,
                  0.0f, 0.0f, 1.0f, 0.0f,
                  -0.664218f, 0.3286369f, 0.0f, 1.0f };

  float m2[16] = { 0.4277841f, -5.125949e-01f, 0.0f, 0.0f,
                  7.151983e-01f, 0.39580684f, 0.0f, 0.0f,
                  0.0f, 0.0f, 1.0f, 0.0f,
                  0.2644247f, 0.4798293f, 0.0f, 1.0f };


  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);

  glColor4f(0, 1, 0, 1);

  GLfloat vertices[] = { -0.5f, 0.0f,
                         0.0f, 0.5f,
                         0.5f, 0.0f,
                         175.0f / 512.0f, 325.0f / 512.0f,
                         250.0f / 512.0f, 25.0f / 512.0f,
                         325.0f / 512.0f, 325.0f / 512.0f };

  GLfloat colors[] = { 1.0f,  0.2f,  0.2f,
                       0.2f,  0.2f,  1.0f,
                       0.8f,  1.0f,  0.2f,
                       0.75f, 0.75f, 0.75f,
                       0.35f, 0.35f, 0.35f,
                       0.5f,  0.5f,  0.5f };

  glEnableClientState(GL_COLOR_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);
  glColorPointer(3, GL_FLOAT, 0, colors);
  glVertexPointer(2, GL_FLOAT, 0, vertices);


  glMatrixMode(GL_MODELVIEW);

  glLoadMatrixf(m1);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glLoadMatrixf(m2);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glLoadIdentity();
  glTranslatef(0.0f, -0.5f, 0.0f);
  glMultMatrixf(m1);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glLoadIdentity();
  glTranslatef(-0.25f, -0.5f, 0.0f);
  glMultMatrixf(m2);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}




