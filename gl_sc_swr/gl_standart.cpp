#include "gl_std.h"
#include "swgl.h"

extern SWGL_Context* g_pContext;


GLAPI void APIENTRY glDeleteTextures(GLsizei n, const GLuint * textures)
{
  if (g_pContext == nullptr)
    return;


}


GLAPI void APIENTRY glTexCoord2f(GLfloat x, GLfloat y)
{
  if (g_pContext == nullptr)
    return;

  g_pContext->input.currInputTexCoord[0] = float2(x, y);
}

GLAPI void APIENTRY glTexCoord2d(GLdouble s, GLdouble t)
{
  if (g_pContext == nullptr)
    return;

  g_pContext->input.currInputTexCoord[0] = float2((float)s, (float)t);
}

GLAPI void APIENTRY glColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
  if (g_pContext == nullptr)
    return;

  g_pContext->input.currInputColor.x = red;
  g_pContext->input.currInputColor.y = green;
  g_pContext->input.currInputColor.z = blue;
}

GLAPI void APIENTRY glClearDepth(GLclampf depth) { glClearDepthf(depth); }

GLAPI void APIENTRY glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
  glFrustumf((GLfloat)left, (GLfloat)right, (GLfloat)bottom, (GLfloat)top, (GLfloat)zNear, (GLfloat)zFar);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void glhFrustumf2(float *matrix, float left, float right, float bottom, float top, float znear, float zfar)
{
  float temp, temp2, temp3, temp4;
  temp  = 2.0f * znear;
  temp2 = right - left;
  temp3 = top   - bottom;
  temp4 = zfar  - znear;
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
void glhPerspectivef2(float *matrix, float fovy, float aspectRatio, float znear, float zfar)
{
  const float ymax = znear * tanf(fovy * 3.14159265358979323846f / 360.0f);
  const float xmax = ymax * aspectRatio;
  glhFrustumf2(matrix, -xmax, xmax, -ymax, ymax, znear, zfar);
}

void gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar)
{
  if (g_pContext == nullptr)
    return;

  float matrixData[16];
  glhPerspectivef2(matrixData, fovy, aspect, zNear, zFar);

  int oldMatrixMode = g_pContext->input.inputMatrixMode;

  glMatrixMode(GL_PROJECTION);
  glMultMatrixf(matrixData);

  glMatrixMode(oldMatrixMode); // 
}


GLAPI void APIENTRY glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble nearVal, GLdouble farVal)
{
  glOrthof((GLfloat)left, (GLfloat)right, (GLfloat)bottom, (GLfloat)top, (GLfloat)nearVal, (GLfloat)farVal);
}

GLAPI void APIENTRY glMap1f(GLenum  target, GLfloat u1, GLfloat u2, GLint   stride, GLint   order, const GLfloat *points)
{

}

GLAPI void APIENTRY glMap2f(GLenum  target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points)
{

}

GLAPI void APIENTRY glTranslated(GLdouble x, GLdouble y, GLdouble z)
{
  glTranslatef((GLfloat)x, (GLfloat)y, (GLfloat)z);
}

GLAPI void APIENTRY glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
  glRotatef((GLfloat)angle, (GLfloat)x, (GLfloat)y, (GLfloat)z);
}


GLAPI void APIENTRY glMultMatrixd(const GLdouble *m)
{
  if (m == nullptr)
    return;

  float fvals[16];
  for (int i = 0; i < 16; i++)
    fvals[i] = float(m[i]);

  glMultMatrixf(fvals);
}

GLAPI GLboolean APIENTRY glIsTexture(GLuint texture)
{
  if (g_pContext == nullptr)
    return 0;

  if (texture >= 0 && texture < (GLuint)g_pContext->m_texTop)
    return 1;
  else
    return 0;
}


GLAPI void APIENTRY glMapGrid1f(GLint un, GLfloat u1, GLfloat u2)
{

}

GLAPI void APIENTRY glEvalCoord1f(GLfloat u)
{

}

GLAPI void APIENTRY glEvalCoord2f(GLfloat u, GLfloat v)
{

}

GLAPI void APIENTRY glEvalMesh1(GLenum mode, GLint i1, GLint  i2)
{

}

GLAPI void APIENTRY glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)
{

}

GLAPI void APIENTRY glEvalPoint2(GLint i, GLint j)
{

}

GLAPI void APIENTRY glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2)
{

}

GLAPI void APIENTRY glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)
{

}


GLAPI void APIENTRY glClipPlane(GLenum plane, const GLdouble *equation)
{

}

GLAPI void APIENTRY glPolygonMode(GLenum face, GLenum mode)
{

}

GLAPI void APIENTRY glPushAttrib(GLbitfield mask)
{

}

GLAPI void APIENTRY glPopAttrib(GLbitfield mask)
{

}

GLAPI void APIENTRY glTexImage1D(GLenum  target, GLint  level, GLint  internalFormat, GLsizei  width, GLint  border, GLenum  format, GLenum  type, const GLvoid*  data)
{

}


GLAPI void APIENTRY glDrawBuffer(GLenum buf)
{

}

GLAPI void APIENTRY glReadBuffer(GLenum mode)
{

}
