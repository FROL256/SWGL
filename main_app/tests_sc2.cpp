#include "tests_sc1.h"

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

#include "../gl_sc_swr/LiteMath.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern int g_localWidth;
extern int g_localHeight;



void test11_alpha_tex_and_transp()
{
  static bool firstFrame = true;

  static GLuint texture  = (GLuint)(-1);
  static GLuint texture2 = (GLuint)(-1);
  static GLuint texture3 = (GLuint)(-1);

  if (firstFrame)
  {
    int w = 80, h = 80;
    std::vector<uint8_t> pixels(w*h);

    float2 c((float)w / 2.0f, (float)h / 2.0f);

    for (int y = 0; y < h; y++)
    {
      for (int x = 0; x < w; x++)
      {
        float2 p((float)x, (float)y);

        float   fval  = clamp(length(p - c) / 40.0f, 0.0f, 1.0f);
        uint8_t ival  = (uint8_t)255 - (uint8_t)(fval*255.0f);
        pixels[y*w+x] = ival;
      }
    }

    glGenTextures(1, &texture);					// Create The Texture

    //Typical Texture Generation Using Data From The Bitmap
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, &pixels[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    int w2, h2;
    std::vector<int> pixels2 = LoadBMP(L"data/texture1.bmp", &w2, &h2);

    glGenTextures(1, &texture2);					// Create The Texture

    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w2, h2, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels2[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int w3, h3;
    std::vector<int> pixels3 = LoadBMP(L"data/RedBrick.bmp", &w3, &h3);

    c = float2((float)w3 / 2.0f, (float)h3 / 2.0f);

    struct pixel2 { uint8_t r, g, b, a; };
    pixel2* pxx2 = (pixel2*)&pixels3[0];

    for (int y = 0; y < h3; y++)
    {
      for (int x = 0; x < w3; x++)
      {
        const float2  p((float)x, (float)y);
        const float2  diff = p - c;
        const float   fval = clamp(length(diff) / 128.0f, 0.0f, 1.0f);
        const uint8_t ival = (uint8_t)255 - (uint8_t)(fval*fval*255.0f);

        pxx2[y*w3 + x].a = ival;
      }
    }

    glGenTextures(1, &texture3);					// Create The Texture

    glBindTexture(GL_TEXTURE_2D, texture3);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w3, h3, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels3[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    firstFrame = false;
  }


  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glColor4f(1, 1, 1, 1);

  glTranslatef(-0.5f, 0.25f, 0.0f);

  const float polySize = 0.4f;

  glTranslatef(1.0f, -0.5f, 0.0f);

  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glBindTexture(GL_TEXTURE_2D, texture);

  glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-polySize, polySize, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(polySize, polySize, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(polySize, -polySize, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-polySize, -polySize, 0.0f);
  glEnd();

  glTranslatef(-0.4f, 0.4f, 0.0f);

  glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-polySize, polySize, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(polySize, polySize, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(polySize, -polySize, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-polySize, -polySize, 0.0f);
  glEnd();


  glLoadIdentity();
  glTranslatef(-0.5f, -0.5f, 0.0f);

  glBindTexture(GL_TEXTURE_2D, texture2);

  glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-polySize, polySize, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(polySize, polySize, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(polySize, -polySize, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-polySize, -polySize, 0.0f);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, texture3);

  glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-polySize, polySize, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(polySize, polySize, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(polySize, -polySize, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-polySize, -polySize, 0.0f);
  glEnd();

  glDisable(GL_BLEND);


  glLoadIdentity();
  glTranslatef(-0.6f, 0.6f, 0.0f);
  glScalef(0.5f, 0.5f, 0.5f);
  glRotatef(25.0f, 0, 0, 1);

  glBindTexture(GL_TEXTURE_2D, texture2);

  glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-polySize, polySize, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(polySize, polySize, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(polySize, -polySize, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-polySize, -polySize, 0.0f);
  glEnd();
}


void test12_rect_tex()
{
  static bool firstFrame = true;

  static GLuint texture2 = (GLuint)(-1);

  if (firstFrame)
  {

    int w2, h2;
    std::vector<int> pixels2 = LoadBMP(L"data/rect_tex.bmp", &w2, &h2);

    glGenTextures(1, &texture2);					// Create The Texture

    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w2, h2, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels2[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    firstFrame = false;
  }


  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glColor4f(1, 1, 1, 1);

  const float polySize = 0.4f;

  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);


  glLoadIdentity();
  glBindTexture(GL_TEXTURE_2D, texture2);

  glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-polySize, polySize, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(polySize, polySize, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(polySize, -polySize, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-polySize, -polySize, 0.0f);
  glEnd();
}


void test13_lines()
{
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glColor4f(1, 1, 1, 1);

  const float polySize = 0.4f;

  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_TEXTURE_2D);

  glTranslatef(-0.5f, 0.25f, 0.0f);

  glBegin(GL_LINES);
  glVertex3f(0.0f, polySize, 0.0f);
  glVertex3f(-polySize, -polySize, 0.0f);
  glVertex3f(polySize, -polySize, 0.0f);
  glVertex3f(polySize, polySize, 0.0f);
  glEnd();

  glTranslatef(1.0f, -0.5f, 0.0f);

  glBegin(GL_LINES);
  glVertex3f(-polySize, polySize, 0.0f);
  glVertex3f(polySize, polySize, 0.0f);
  glVertex3f(polySize, -polySize, 0.0f);
  glVertex3f(-polySize, -polySize, 0.0f);
  glEnd();

  glColor4f(0, 1, 0, 1);

  glLoadIdentity();
  glScalef(0.75f, 0.75f, 0.5f);
  glTranslatef(-0.5f, 0.25f, 0.0f);

  glBegin(GL_LINES);
  glColor4f(1, 1, 0, 1); glVertex3f(0.0f, polySize, 0.0f);
  glColor4f(0, 0, 1, 1); glVertex3f(-polySize, -polySize, 0.0f);
  glColor4f(0, 1, 0, 1); glVertex3f(polySize, -polySize, 0.0f);
  glColor4f(1, 0, 0, 1); glVertex3f(polySize, polySize, 0.0f);
  glEnd();

  glTranslatef(1.0f, -0.5f, 0.0f);

  glBegin(GL_LINES);
  glColor4f(1, 0, 0, 1); glVertex3f(-polySize, polySize, 0.0f);
  glColor4f(0, 1, 0, 1); glVertex3f(polySize, polySize, 0.0f);
  glColor4f(0, 0, 1, 1); glVertex3f(polySize, -polySize, 0.0f);
  glColor4f(1, 1, 0, 1); glVertex3f(-polySize, -polySize, 0.0f);
  glEnd();


}

void DrawCube()
{
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

void test14_transparent_cube()
{
  demo14_transparent_cube(g_localWidth, g_localHeight, 60.0f, 40.0f);
}

void demo14_transparent_cube(int width, int height, const float xRot, const float yRot)
{
  const float rtri = xRot;
  const float rquad = yRot;

  static bool firstFrame = true;
  static GLuint texture = (GLuint)(-1);


  if (firstFrame)
  {
    int w, h;
    std::vector<int> pixels = LoadBMP(L"data/Glass2.bmp", &w, &h);

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
  const float xrot = xRot*0.75f;
  const float yrot = yRot*0.25f;
  const float zrot = -yRot*0.5f;

  glTranslatef(0.0f, 0.0f, -5.0f);

  glRotatef(xrot, 1.0f, 0.0f, 0.0f);
  glRotatef(yrot, 0.0f, 1.0f, 0.0f);
  glRotatef(zrot, 0.0f, 0.0f, 1.0f);

  glBindTexture(GL_TEXTURE_2D, texture);
  glColor3f(1.0f, 1.0f, 1.0f);

  glEnable(GL_CULL_FACE);

  glColor4f(1.0f, 1.0f, 1.0f, 0.5);					         // Full Brightness.  50% Alpha
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Set The Blending Function For Translucency
  glEnable(GL_BLEND);

  glCullFace(GL_FRONT);
  DrawCube();

  glCullFace(GL_BACK);
  DrawCube();
}

void demo24_draw_elements_terrain(int width, int height, float algle1, float angle2)
{
  const float rtri  = algle1;
  const float rquad = angle2;

  static bool firstFrame = true;
  static GLuint texture = (GLuint)(-1);

  static std::vector<float3> vertPos;
  static std::vector<float2> texCoord;
  static std::vector<int>    triInd;


  static int triTop = 0;

  if (firstFrame)
  {
    int w, h;
    std::vector<int> pixels = LoadBMP(L"data/terrain.bmp", &w, &h);

    glGenTextures(1, &texture);					// Create The Texture

    // Typical Texture Generation Using Data From The Bitmap
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // generate mesh from heightmap
    //
    std::vector<int> heighpMap = LoadBMP(L"data/heightmap.bmp", &w, &h);

    vertPos.resize(w*h);
    texCoord.resize(w*h);
    triInd.resize(w*h*6);

    float2 hmInv(1.0f/float(w), 1.0f/float(h));
    float3 cornerLeft(float(w) / 2.0f, 0.0, float(h) / 2.0f);

    for (int y = 0; y < h; y++)
    {
      int y0 = y;
      int y1 = y + 1;
      if (y1 >= h) y1 = y;

      for (int x = 0; x < w; x++)
      {
        int x0 = x;
        int x1 = x + 1;
        if (x1 >= w) x1 = x;

        const int id0 = y0*w + x0;
        const int id1 = y0*w + x1;
        const int id2 = y1*w + x0;
        const int id3 = y1*w + x1;

        const int px0 = heighpMap[id0];
        const int px1 = heighpMap[id1];
        const int px2 = heighpMap[id2];
        const int px3 = heighpMap[id3];

        const float4 px0f = Uint32_BGRAToRealColor(px0);
        const float4 px1f = Uint32_BGRAToRealColor(px1);
        const float4 px2f = Uint32_BGRAToRealColor(px2);
        const float4 px3f = Uint32_BGRAToRealColor(px3);

        const float4 one(1, 1, 1, 0);

        const float h0 = dot(px0f, one);
        const float h1 = dot(px1f, one);
        const float h2 = dot(px2f, one);
        const float h3 = dot(px3f, one);

        vertPos[id0]  = float3((float)x0, h0, (float)y0) - cornerLeft;
        vertPos[id1]  = float3((float)x1, h1, (float)y0) - cornerLeft;
        vertPos[id2]  = float3((float)x0, h2, (float)y1) - cornerLeft;
        vertPos[id3]  = float3((float)x1, h3, (float)y1) - cornerLeft;

        texCoord[id0] = float2((float)x0, (float)y0)*hmInv;
        texCoord[id1] = float2((float)x1, (float)y0)*hmInv;
        texCoord[id2] = float2((float)x0, (float)y1)*hmInv;
        texCoord[id3] = float2((float)x1, (float)y1)*hmInv;

        if (x != w - 1 && y != h - 1)
        {
          triInd[triTop + 0] = id0;
          triInd[triTop + 1] = id2;
          triInd[triTop + 2] = id1;

          triInd[triTop + 3] = id1;
          triInd[triTop + 4] = id2;
          triInd[triTop + 5] = id3;
          triTop += 6;
        }
      }
    }

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

  glTranslatef(0.0f, 0.0f, -15.0f);

  glRotatef(25.0f, 1.0f, 0.0f, 0.0f);
  glRotatef(yrot, 0.0f, 1.0f, 0.0f);
  //glRotatef(zrot, 0.0f, 0.0f, 1.0f);

  glBindTexture(GL_TEXTURE_2D, texture);
  glColor3f(1.0f, 1.0f, 1.0f);

  glColor4f(1.0f, 1.0f, 1.0f, 0.0);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glDisableClientState(GL_COLOR_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);

  glTexCoordPointer(2, GL_FLOAT, 0, &texCoord[0]);
  glVertexPointer  (3, GL_FLOAT, 0, &vertPos[0]);

  //
  glPushMatrix();
  glScalef(1.0f / 16.0f, 1.0f, 1.0f / 16.0f);
  glDrawElements(GL_TRIANGLES, triTop, GL_UNSIGNED_INT, &triInd[0]);
  glPopMatrix();

}

#include "HydraExport.h"

void demo25_teapot(int width, int height, float algle1, float angle2)
{
  const float rtri = algle1;
  const float rquad = angle2;

  static bool firstFrame = true;
  static GLuint texture = (GLuint)(-1);
  static GLuint texture2 = (GLuint)(-1);
  static HydraGeomData data;

  if (firstFrame)
  {
    int w, h;
    std::vector<int> pixels = LoadBMP(L"data/Glass2.bmp", &w, &h);

    glGenTextures(1, &texture);					// Create The Texture

    // Typical Texture Generation Using Data From The Bitmap
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    pixels = LoadBMP(L"data/board10.bmp", &w, &h);

    glGenTextures(1, &texture2);					// Create The Texture

    // Typical Texture Generation Using Data From The Bitmap
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    data.read("data/teapot2.vsgf");
    std::cout << "demo25_teapot: data was loaded" << std::endl;
    firstFrame = false;
  }

  glMatrixMode(GL_PROJECTION);			// Select The Projection Matrix
  glLoadIdentity();									// Reset The Projection Matrix

  // Calculate The Aspect Ratio Of The Window
  gluPerspective2(45.0f, (GLfloat)width / (GLfloat)height, 0.01f, 1000.0f);

  glMatrixMode(GL_MODELVIEW);			       // Select The Modelview Matrix
  glLoadIdentity();									     // Reset The Modelview Matrix

  glShadeModel(GL_SMOOTH);							 // Enable Smooth Shading
  glClearColor(0.0f, 0.0f, 0.0f, 0.5f);	 // Black Background
  glClearDepth(1.0f);									   // Depth Buffer Setup
  glEnable(GL_DEPTH_TEST);							 // Enables Depth Testing
  glDepthFunc(GL_LEQUAL);								 // The Type Of Depth Testing To Do
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations


  glEnable(GL_TEXTURE_2D);
  //glDisable(GL_TEXTURE_2D);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer

  //
  //
  const float xrot = algle1;
  const float yrot = angle2;
  const float zrot = -angle2;

  //glTranslatef(0.0f, -0.5f, -4.0f);
  //glTranslatef(0.0f, -0.5f, -4.5f);
  //glTranslatef(0.0f, -0.5f, -4.8f);
  glTranslatef(0.0f, -0.5f, -5.0f);

  glRotatef(20.0f, 1.0f, 0.0f, 0.0f);
  glRotatef(yrot, 0.0f, 1.0f, 0.0f);

  glBindTexture(GL_TEXTURE_2D, texture);
  glColor4f(0.75f, 0.75f, 0.75f, 0.0);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glDisableClientState(GL_COLOR_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);

  glTexCoordPointer(2, GL_FLOAT, 0, data.getVertexTexcoordFloat2Array());
  glVertexPointer(4, GL_FLOAT, 0,   data.getVertexPositionsFloat4Array());

  glEnable(GL_CULL_FACE);

  glPushMatrix();
    glDrawElements(GL_TRIANGLES, data.getIndicesNumber(), GL_UNSIGNED_INT, data.getTriangleVertexIndicesArray());
  glPopMatrix();

  // draw floor
  glColor4f(0.75f, 0.75f, 0.75f, 0.0);
  glBindTexture(GL_TEXTURE_2D, texture2);

  glPushMatrix();
    glTranslatef(0.0f, 3.0f, 0.0f);
    glScalef(3.0f, 3.0f, 3.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, -1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
    glEnd();
  glPopMatrix();

}


void demo26_teapots9(int width, int height, float algle1, float angle2)
{
  const float rtri = algle1;
  const float rquad = angle2;

  static bool firstFrame = true;
  static GLuint texture = (GLuint)(-1);
  static GLuint texture2 = (GLuint)(-1);
  static HydraGeomData data;

  if (firstFrame)
  {
    int w, h;
    std::vector<int> pixels = LoadBMP(L"data/Glass2.bmp", &w, &h);

    glGenTextures(1, &texture);					// Create The Texture

    // Typical Texture Generation Using Data From The Bitmap
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    pixels = LoadBMP(L"data/board10.bmp", &w, &h);

    glGenTextures(1, &texture2);					// Create The Texture

    // Typical Texture Generation Using Data From The Bitmap
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    data.read("data/teapot2.vsgf");

    firstFrame = false;
  }


  glMatrixMode(GL_PROJECTION);			// Select The Projection Matrix
  glLoadIdentity();									// Reset The Projection Matrix

  // Calculate The Aspect Ratio Of The Window
  gluPerspective2(45.0f, (GLfloat)width / (GLfloat)height, 0.01f, 1000.0f);

  glMatrixMode(GL_MODELVIEW);			       // Select The Modelview Matrix
  glLoadIdentity();									     // Reset The Modelview Matrix

  glShadeModel(GL_SMOOTH);							 // Enable Smooth Shading
  glClearColor(0.0f, 0.0f, 0.0f, 0.5f);	 // Black Background
  glClearDepth(1.0f);									   // Depth Buffer Setup
  glEnable(GL_DEPTH_TEST);							 // Enables Depth Testing
  glDepthFunc(GL_LEQUAL);								 // The Type Of Depth Testing To Do
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations


  glEnable(GL_TEXTURE_2D);
  //glDisable(GL_TEXTURE_2D);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer

  //
  //
  const float xrot = algle1*0.75f;
  const float yrot = angle2*0.25f*10.0f;
  const float zrot = -angle2*0.5f;

  glTranslatef(0.0f, -0.5f, -5.0f);

  glRotatef(20.0f, 1.0f, 0.0f, 0.0f);
  glRotatef(yrot, 0.0f, 1.0f, 0.0f);

  glBindTexture(GL_TEXTURE_2D, texture);
  glColor4f(0.75f, 0.75f, 0.75f, 0.0);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glDisableClientState(GL_COLOR_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);

  glTexCoordPointer(2, GL_FLOAT, 0, data.getVertexTexcoordFloat2Array());
  glVertexPointer(4, GL_FLOAT, 0, data.getVertexPositionsFloat4Array());

  glEnable(GL_CULL_FACE);

  for (int i = -1; i <= 1; i++)
  {
    for (int j = -1; j <= 1; j++)
    {
      glPushMatrix();

      glTranslatef(0.0f + 2.0f*float(i), 0.0f, 0.0f + 2.0f*float(j));
      glScalef(0.5f, 0.5f, 0.5f);

      glDrawElements(GL_TRIANGLES, data.getIndicesNumber(), GL_UNSIGNED_INT, data.getTriangleVertexIndicesArray());
      glPopMatrix();
    }
  }

  // draw floor
  glColor4f(0.75f, 0.75f, 0.75f, 0.0);
  glBindTexture(GL_TEXTURE_2D, texture2);

  glPushMatrix();
  glTranslatef(0.0f, 3.0f, 0.0f);
  glScalef(3.0f, 3.0f, 3.0f);

  glBegin(GL_QUADS);
  glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
  glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
  glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, -1.0f, -1.0f);
  glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
  glEnd();
  glPopMatrix();

}



void test15_simple_stencil()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
  glDisable(GL_CULL_FACE);

  //
  //
  glColor4f(1, 1, 1, 1);

  glLoadIdentity();
  glTranslatef(0.0f, -0.2f, 0.0f);
  glScalef(0.4f, 0.4f, 0.4f);

  const float polySize = 0.85f;

  glClearColor(0.0f, 0.0f, 0.25f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_STENCIL_TEST);

  glStencilFunc(GL_ALWAYS, 65535, 1);
  glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

  glStencilMask(1);
  glClearStencil(0);
  glClear(GL_STENCIL_BUFFER_BIT);

  glStencilFunc(GL_ALWAYS, 65535, 1);
  glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);


  glColor4f(1, 1, 1, 1);

  glBegin(GL_QUADS);
  glVertex3f(-polySize, polySize, 0.0f);
  glVertex3f(polySize, polySize, 0.0f);
  glVertex3f(polySize, -polySize, 0.0f);
  glVertex3f(-polySize, -polySize, 0.0f);
  glEnd();

  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

  glStencilMask(0);
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
  glStencilFunc(GL_NOTEQUAL, 0, 1);

  //
  //
  glLoadIdentity();
  glBegin(GL_TRIANGLES);
  glColor4f(1, 0, 0, 1);
  glVertex2f(-0.5f, -0.5f);

  glColor4f(0, 1, 0, 1);
  glVertex2f(0.0f, 0.5f);

  glColor4f(0, 0, 1, 1);
  glVertex2f(0.5f, -0.5f);
  glEnd();

  glDisable(GL_STENCIL_TEST);
}


void test16_tri_strip()
{
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glDisable(GL_DEPTH_TEST);

  glColor4f(0.75f, 0.0f, 0.55f, 1);

  const float sz = 0.05f;

  for (int line = -5; line <= 5; line+=2)
  {
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = -5; i <= 5; i++)
    {
      if(i%2 == 0)
        glColor4f(0.75f, 0.0f, 0.15f, 1);
      else
        glColor4f(0.0f, 0.75f, 0.15f, 1);

      glVertex2f(sz*i*2, (line+0)*sz*2);

      if (i % 2 == 0)
        glColor4f(0.0f, 0.0f, 0.75f, 1);
      else
        glColor4f(0.7f, 0.5f, 0.5f, 1);

      glVertex2f(sz*i*2, (line+1)*sz*2);
    }
    glEnd();
  }

}

void test17_line_strip()
{
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glRotatef(90.0f, 0, 0, 1);

  glDisable(GL_DEPTH_TEST);

  glColor4f(1.0, 1.0, 1.0, 1);

  const float sz = 0.05f;

  for (int line = -5; line <= 5; line += 2)
  {
    glBegin(GL_LINE_STRIP);
    for (int i = -5; i <= 5; i++)
    {
      if (i % 2 == 0)
        glColor4f(0.75f, 0.0f, 0.15f, 1);
      else
        glColor4f(0.0f, 0.75f, 0.15f, 1);

      glVertex2f(sz*i * 2, (line + 0)*sz * 2);

      if (i % 2 == 0)
        glColor4f(0.0f, 0.0f, 0.75f, 1);
      else
        glColor4f(0.7f, 0.5f, 0.5f, 1);

      glVertex2f(sz*i * 2, (line + 1)*sz * 2);
    }
    glEnd();
  }


}

void test18_line_points()
{
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glRotatef(90.0f, 0, 0, 1);

  glDisable(GL_DEPTH_TEST);
  glPointSize(3.0f);

  glColor4f(1.0, 1.0, 1.0, 1);

  const float sz = 0.05f;

  for (int line = -5; line <= 5; line += 2)
  {
    glBegin(GL_POINTS);
    for (int i = -5; i <= 5; i++)
    {
      if (i % 2 == 0)
        glColor4f(0.75f, 0.0f, 0.15f, 1);
      else
        glColor4f(0.0f, 0.75f, 0.15f, 1);

      glVertex2f(sz*i * 2, (line + 0)*sz * 2);

      if (i % 2 == 0)
        glColor4f(0.0f, 0.0f, 0.75f, 1);
      else
        glColor4f(0.7f, 0.5f, 0.5f, 1);

      glVertex2f(sz*i * 2, (line + 1)*sz * 2);
    }
    glEnd();
  }



}


void myglUnitCube()
{
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

void myglColoredPyramid()
{
  glDisable(GL_TEXTURE_2D);

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
}

void myglColoredCube()
{
  glDisable(GL_TEXTURE_2D);

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


void demo19_cubes(int width, int height, float algle1, float angle2)
{
  const float rtri = algle1;
  const float rquad = angle2;

  static bool firstFrame = true;
  static GLuint texture  = (GLuint)(-1);
  static GLuint texture2 = (GLuint)(-1);
  static GLuint texture3 = (GLuint)(-1);


  if (firstFrame)
  {
    int w, h;
    std::vector<int> pixels = LoadBMP(L"data/texture1.bmp", &w, &h);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    pixels = LoadBMP(L"data/RedBrick.bmp", &w, &h);

    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    pixels = LoadBMP(L"data/Glass2.bmp", &w, &h);

    glGenTextures(1, &texture3);
    glBindTexture(GL_TEXTURE_2D, texture3);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    firstFrame = false;
  }


  glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
  glLoadIdentity();									// Reset The Projection Matrix

  // Calculate The Aspect Ratio Of The Window
  gluPerspective2(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 1000.0f);

  glMatrixMode(GL_MODELVIEW);			  // Select The Modelview Matrix
  glLoadIdentity();									// Reset The Modelview Matrix

  glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
  glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
  glClearDepth(1.0f);									// Depth Buffer Setup
  glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
  glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations


  glEnable(GL_TEXTURE_2D);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer

  //
  //

  glBindTexture(GL_TEXTURE_2D, texture);
  glColor3f(1.0f, 1.0f, 1.0f);


  const float xrot = algle1*0.75f;
  const float yrot = angle2*0.5f;
  const float zrot = -angle2*0.5f;


  glLoadIdentity();
  glTranslatef(0.0f, 0.0f, -20.0f);
  glRotatef(yrot, 0.0f, 1.0f, 0.0f);

  // draw floor
  //
  glPushMatrix();

    glTranslatef(0.0f, 6.0f, 0.0f);
    glScalef(10.0f, 10.0f, 10.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, -1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
    glEnd();


  glPopMatrix();

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  // draw center cube
  //
  glPushMatrix();

    glTranslatef(0.0f, -1.0f, 0.0f);
    glScalef(3.0f, 3.0f, 3.0f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture2);
    myglUnitCube();


    glPushMatrix();

    glTranslatef(0.0f, 1.5f, 0.0f);
    glScalef(0.5f, 0.5f, 0.5f);

      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, texture);
      glColor3f(1.0f, 1.0f, 1.0f);
      myglUnitCube();


      glPushMatrix();
      glTranslatef(0.0f, 2.0f, 0.0f);
      myglColoredPyramid();
      glPopMatrix();

    glPopMatrix();

  glPopMatrix();


  // draw 4 pyramids
  //

  for (int x = -1; x <= 1; x++)
  {
    for (int y = -1; y <= 1; y++)
    {
      if (x == 0 || y == 0)
        continue;

      glPushMatrix();

        glTranslatef(x*6.0f, -1.0f, y*6.0f);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture3);
        glColor3f(1.0f, 1.0f, 1.0f);

        glPushMatrix();
        glTranslatef(0.0f, -2.0f, 0.0f);
        myglUnitCube();
        glPopMatrix();

        myglColoredPyramid();

      glPopMatrix();
    }
  }

  glDisable(GL_CULL_FACE);

}


void test19_push_pop_matrix()
{
  demo19_cubes(g_localWidth, g_localHeight, 47.0f, 58.0f);
}

void test20_glftustum()
{
  const int width  = g_localWidth;
  const int height = g_localHeight;

  const float rtri  = 45.0f;
  const float rquad = 80.0f;

  const GLfloat aspect = (GLfloat)width / (GLfloat)height;

  glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
  glLoadIdentity();									// Reset The Projection Matrix

  // Calculate The Aspect Ratio Of The Window
  //gluPerspective2(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
  glFrustum(-0.05*aspect, 0.05*aspect, -0.05, 0.05, 0.1, 100.0);

  glMatrixMode(GL_MODELVIEW);			  // Select The Modelview Matrix
  glLoadIdentity();									// Reset The Modelview Matrix

  glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
  glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
  glClearDepth(1.0f);									// Depth Buffer Setup
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


void drawMyTriangle()
{
  glBegin(GL_TRIANGLES);
    glColor4f(1, 0, 0, 1); glVertex2f(-0.5f, -0.5f);
    glColor4f(0, 1, 0, 1); glVertex2f(0.0f, 0.5f);
    glColor4f(0, 0, 1, 1); glVertex2f(0.5f, -0.5f);
  glEnd();
}

void test21_clip_2d()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glDisable(GL_DEPTH_TEST);

  for (float y = -1.0f; y <= 1.0f; y += 0.2f)
  {
    for (float x = -1.0f; x <= 1.0f; x += 0.2f)
    {
      glPushMatrix();
      glTranslatef(x, y + 0.05f, 0);
      glScalef(0.25f, 0.25f, 0.25f);
      glRotatef((x + y + 1.0f)*55.0f, 0, 0, 1);
      drawMyTriangle();
      glPopMatrix();
    }

  }

}


void test22_change_viewort_size()
{
  return; // this is temporary, test works, but i need to change window size and resize framebuffer in my demo

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glViewport(0, 0, 800, 600);

  glClearColor(0.25f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glDisable(GL_DEPTH_TEST);

  drawMyTriangle();

  glViewport(0, 0, 200, 200);
  drawMyTriangle();

  glViewport(600, 400, 200, 200);

  glPushMatrix();
  glRotatef(45, 0, 0, 1);
  drawMyTriangle();
  glPopMatrix();

}

void test23_draw_elements()
{
/*
  glViewport(0, 0, 800, 600);

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

  GLfloat colors[] = { 1.0, 0.2, 0.2,
                       0.2, 0.2, 1.0,
                       0.8, 1.0, 0.2,
                       0.75, 0.75, 0.75,
                       0.35, 0.35, 0.35,
                       0.5, 0.5, 0.5 };

  int indices[6] = { 0, 1, 2, 3, 4, 5 };

  glEnableClientState(GL_COLOR_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);
  glColorPointer(3, GL_FLOAT, 0, colors);
  glVertexPointer(2, GL_FLOAT, 0, vertices);

  glPushMatrix();
  glTranslatef(0.25f, 0.25f, 0.0f);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices);
  glPopMatrix();

  //
  //
  struct Vertex { float4 pos; float4 color; };

  float4 VerticesPos[] =
  {
    { 0.0f, 0.0f, 0.0f, 1.0f },
    // Top
    { -0.2f, 0.8f, 0.0f, 1.0f },
    { 0.2f, 0.8f, 0.0f, 1.0f },
    { 0.0f, 0.8f, 0.0f, 1.0f },
    { 0.0f, 1.0f, 0.0f, 1.0f },
    // Bottom
    { -0.2f, -0.8f, 0.0f, 1.0f },
    { 0.2f, -0.8f, 0.0f, 1.0f },
    { 0.0f, -0.8f, 0.0f, 1.0f },
    { 0.0f, -1.0f, 0.0f, 1.0f },
    // Left
    { -0.8f, -0.2f, 0.0f, 1.0f },
    { -0.8f, 0.2f, 0.0f, 1.0f },
    { -0.8f, 0.0f, 0.0f, 1.0f },
    { -1.0f, 0.0f, 0.0f, 1.0f },
    // Right
    { 0.8f, -0.2f, 0.0f, 1.0f },
    { 0.8f, 0.2f, 0.0f, 1.0f },
    { 0.8f, 0.0f, 0.0f, 1.0f },
    { 1.0f, 0.0f, 0.0f, 1.0f },
  };

  float4 VerticesColors[] =
  {
    { 1.0f, 1.0f, 1.0f, 1.0f },

    { 0.0f, 1.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 1.0f, 1.0f },
    { 0.0f, 1.0f, 1.0f, 1.0f },
    { 1.0f, 0.0f, 0.0f, 1.0f },

    { 0.0f, 0.0f, 1.0f, 1.0f },
    { 0.0f, 1.0f, 0.0f, 1.0f },
    { 0.0f, 1.0f, 1.0f, 1.0f },
    { 1.0f, 0.0f, 0.0f, 1.0f },

    { 0.0f, 1.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 1.0f, 1.0f },
    { 0.0f, 1.0f, 1.0f, 1.0f },
    { 1.0f, 0.0f, 0.0f, 1.0f },

    { 0.0f, 0.0f, 1.0f, 1.0f },
    { 0.0f, 1.0f, 0.0f, 1.0f },
    { 0.0f, 1.0f, 1.0f, 1.0f },
    { 1.0f, 0.0f, 0.0f, 1.0f },
  };

  glEnableClientState(GL_COLOR_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);

  glColorPointer(4, GL_FLOAT, 0, VerticesColors);
  glVertexPointer(4, GL_FLOAT, 0, VerticesPos);

  int Indices[] = {
    // Top
    0, 1, 3,
    0, 3, 2,
    3, 1, 4,
    3, 4, 2,
    // Bottom
    0, 5, 7,
    0, 7, 6,
    7, 5, 8,
    7, 8, 6,
    // Left
    0, 9, 11,
    0, 11, 10,
    11, 9, 12,
    11, 12, 10,
    // Right
    0, 13, 15,
    0, 15, 14,
    15, 13, 16,
    15, 16, 14
  };


  glDrawElements(GL_TRIANGLES, 48, GL_UNSIGNED_INT, (GLvoid*)Indices);


  // test  glDrawElements(GL_TRIANGLE_STRIP, ... );
  //
  glPushMatrix();
  {
    glTranslatef(-0.6f, -0.6f, 0.0f);
    glScalef(0.2f, 0.2f, 0.2f);

    glDisableClientState(GL_COLOR_ARRAY);
    glColor4f(0.5f, 0.25f, 0.75f, 1);

    std::vector<float4> vertPos;
    std::vector<int>    indices;

    const int N = 6;

    for (int i = 0; i < N; i++)
    {
      vertPos.push_back(float4(-1 + i, -1, 0, 1));
      vertPos.push_back(float4(-1 + i,  1, 0, 1));
      vertPos.push_back(float4( 1 + i,  1, 0, 1));
      vertPos.push_back(float4( 1 + i,  1, 0, 1));

      indices.push_back(i + 0);
      indices.push_back(i + 1);
      indices.push_back(i + 2);
      indices.push_back(i + 3);

    }

    glVertexPointer(4, GL_FLOAT, 0, &vertPos[0]);
    glDrawElements(GL_TRIANGLE_STRIP, indices.size(), GL_UNSIGNED_INT, (GLvoid*)&indices[0]);

  }
  glPopMatrix();


  */

}

void DrawBox(float2 bmin, float2 bmax)
{
  const float2 t  = bmax - bmin;
  const float2 v0 = bmin + float2(0, 0)*t;
  const float2 v1 = bmin + float2(0, 1)*t;
  const float2 v2 = bmin + float2(1, 1)*t;
  const float2 v3 = bmin + float2(1, 0)*t;

  glBegin(GL_LINE_STRIP);
  glVertex2f(v0.x, v0.y);
  glVertex2f(v1.x, v1.y);
  glVertex2f(v2.x, v2.y);
  glVertex2f(v3.x, v3.y);
  glVertex2f(v0.x, v0.y);
  glEnd();
}


inline bool pointInBox(float2 p, float2 bmin, float2 bmax)
{
  return (bmin.x <= p.x) && (p.x <= bmax.x) && (bmin.y <= p.y) && (p.y <= bmax.y);
}

inline float2 rayBoxTest(float2 rpos1, float2 rdirInv1, float2 bmin, float2 bmax)
{
  float lo = rdirInv1.x*(bmin.x - rpos1.x);
  float hi = rdirInv1.x*(bmin.x - rpos1.x);

  float tmin = fmin(lo, hi);
  float tmax = fmax(lo, hi);

  float lo1 = rdirInv1.y*(bmin.y - rpos1.y);
  float hi1 = rdirInv1.y*(bmin.y - rpos1.y);

  tmin = fmax(tmin, fmin(lo1, hi1));
  tmax = fmin(tmax, fmax(lo1, hi1));

  return float2(tmin, tmax);
}


inline bool triBoxOverlap(float2 A, float2 B, float2 C, float2 bmin, float2 bmax)
{
  if (pointInBox(A, bmin, bmax) || pointInBox(B, bmin, bmax) || pointInBox(C, bmin, bmax)) // test box-box intersection instead of this
    return true;

  const float2 rpos1 = A;
  const float2 rpos2 = A;
  const float2 rpos3 = B;

  const float dist1 = length(B - A);
  const float dist2 = length(C - A);
  const float dist3 = length(C - B);

  const float2 rdirInv1 = float2(1.0f,1.0f)/( (B - A)/dist1 );
  const float2 rdirInv2 = float2(1.0f,1.0f)/( (C - A)/dist2 );
  const float2 rdirInv3 = float2(1.0f,1.0f)/( (C - B)/dist3 );

  float2 t1 = rayBoxTest(rpos1, rdirInv1, bmin, bmax);
  float2 t2 = rayBoxTest(rpos2, rdirInv2, bmin, bmax);
  float2 t3 = rayBoxTest(rpos3, rdirInv3, bmin, bmax);

  return ((t1.x >= 0 && t1.x <= dist1) || (t1.y >= 0 && t1.y <= dist1)) ||
         ((t2.x >= 0 && t2.x <= dist1) || (t2.y >= 0 && t2.y <= dist1)) ||
         ((t3.x >= 0 && t3.x <= dist1) || (t3.y >= 0 && t3.y <= dist1));
}


void test_box_tri_overlap()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0.0f, 0.0f, 0.25f, 0.5f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  float2 A(-0.5f, -0.85f);
  float2 B(0.0f, 0.5f);
  float2 C(0.5f, -0.85f);

  float2 bmin(-0.75f, -0.75f);
  float2 bmax(-0.25f, -0.25f);

  float2 bmin2(-0.65f, -0.85f);
  float2 bmax2(-0.45f, 0.55f);

  if (IntersectBoxBox(bmin, bmax, bmin2, bmax2))
    glColor4f(0, 1, 0, 1);
  else
    glColor4f(1, 0, 0, 1);

  DrawBox(bmin2, bmax2);

  glColor4f(1, 1, 1, 1);

  DrawBox(bmin, bmax);

}

