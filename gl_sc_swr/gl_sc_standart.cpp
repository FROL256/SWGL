#include "gl_sc.h"
#include "gl_std.h"
#include "swgl.h"

extern SWGL_Context* g_pContext;

GLAPI void APIENTRY glActiveTexture(GLenum texture) //// ## EMPTY ##, consider to remove from header
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glActiveTexture(" << texture << ")" << std::endl;
}


GLAPI void APIENTRY glAlphaFunc(GLenum func, GLclampf ref) //// ## EMPTY ## 100% remove from header because we are not going to support alpha test
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glAlphaFunc(" << func << ", " << ref << ")" << std::endl;
}


GLAPI void APIENTRY glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap) //// ## PIECE OF SHIT ## 100% remove from header
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glBitmap(" << width << ", " << height << ", ... )" << std::endl;
} 


GLAPI void APIENTRY glCallLists(GLsizei n, GLenum type, const GLvoid *lists) //// ## EMPTY ## 100% remove from header; don't support display lists
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glCallLists(" << n << ", " << type << ", " << lists << ")" << std::endl;
}



GLAPI void APIENTRY glColorSubTableEXT(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *table) //// ## EMPTY ## 100% remove from header; don't support paletted textures
{
  if (g_pContext == nullptr)
    return;

}

GLAPI void APIENTRY glColorTableEXT(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table) //// ## EMPTY ## 100% remove from header; don't support paletted textures
{
  if (g_pContext == nullptr)
    return;

}

GLAPI void APIENTRY glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type) //// ## PIECE OF SHIT ## 100% remove from header
{
  if (g_pContext == nullptr)
    return;

}


GLAPI void APIENTRY glDepthRangef(GLclampf zNear, GLclampf zFar)  //// ## EMPTY ## 100% remove from header; don't support 
{

}

GLAPI void APIENTRY glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)  //// ## EMPTY ## 100% remove from header; don't support 
{

}


GLAPI void APIENTRY glEndList(void) //// ## EMPTY ## 100% remove from header; don't support display lists
{

}

GLAPI void APIENTRY glFrontFace(GLenum mode) //// ## EMPTY ## probably we have to implement it, but better to omit
{

}


GLAPI void APIENTRY glFrustumf(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)//// ## EMPTY ## not sure
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glFrustumf()" << std::endl;

  const float A = (right + left)  / (right - left);
  const float B = (top + bottom)  / (top - bottom);
  const float C = -(zFar + zNear) / (zFar - zNear);
  const float D = -(2*zFar*zNear) / (zFar - zNear);
  const float E = (2 * zNear)     / (right - left);
  const float F = (2 * zNear)     / (top - bottom);

  float M[4][4];
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      M[i][j] = 0.0f;

  M[0][0] = E;
  M[1][1] = F;
  M[2][2] = C;

  M[2][0] = A;
  M[2][1] = B;
  M[2][3] = -1.0f;

  M[3][2] = D;

  glMultMatrixf(&M[0][0]);

}

GLAPI GLuint APIENTRY glGenLists(GLsizei range)  //// ## EMPTY ## 100% remove from header; don't support display lists
{
  return 0;
}


GLAPI void APIENTRY glGetColorTableEXT(GLenum target, GLenum format, GLenum type, GLvoid *table) //// ## EMPTY ## 100% remove from header; don't support paletted textures
{

}

GLAPI void APIENTRY glGetColorTableParameterivEXT(GLenum target, GLenum pname, GLint *params) //// ## EMPTY ## 100% remove from header; don't support paletted textures
{

}


GLAPI void APIENTRY glGetLightfv(GLenum light, GLenum pname, GLfloat *params) //// ## EMPTY ## 100% remove from header; don't support lights
{

}

GLAPI void APIENTRY glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params) //// ## EMPTY ## 100% remove from header; don't support materials
{

}

GLAPI void APIENTRY glGetPointerv(GLenum pname, GLvoid **params) //// ## PIECE OF SHIT ## 100% remove from header
{

}

GLAPI void APIENTRY glGetPolygonStipple(GLubyte *mask)  //// ## EMPTY ## 100% remove from header; don't support it
{

}

GLAPI void APIENTRY glGetTexEnvfv(GLenum target, GLenum pname, GLfloat *params) //// ## EMPTY ## 100% remove from header;
{

}



GLAPI void APIENTRY glLightfv(GLenum light, GLenum pname, const GLfloat *params) //// ## EMPTY ## 100% remove from header; don't support lights
{

}

GLAPI void APIENTRY glLightModelfv(GLenum pname, const GLfloat *params) //// ## EMPTY ## 100% remove from header; don't support lights
{

}

GLAPI void APIENTRY glLineStipple(GLint factor, GLushort pattern) //// ## EMPTY ## 100% remove from header; don't support it
{

}

GLAPI void APIENTRY glListBase(GLuint base) //// ## EMPTY ## 100% remove from header; don't support display lists
{

}

GLAPI void APIENTRY glMaterialf(GLenum face, GLenum pname, GLfloat param) //// ## EMPTY ## 100% remove from header; don't support materials
{

}

GLAPI void APIENTRY glMaterialfv(GLenum face, GLenum pname, const GLfloat *params) //// ## EMPTY ## 100% remove from header; don't support materials
{

}

GLAPI void APIENTRY glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t) //// ## EMPTY ## probably remove from header;
{
  if (g_pContext == nullptr)
    return;

  if (target >= GL_TEXTURE0 && target <= GL_TEXTURE31)
    g_pContext->input.currInputTexCoord[target - GL_TEXTURE0] = float2(s, t);
}

GLAPI void APIENTRY glMultiTexCoord2fv(GLenum target, const GLfloat *v) //// ## EMPTY ## probably remove from header;
{
  if (g_pContext == nullptr || v == nullptr)
    return;

  if (target >= GL_TEXTURE0 && target <= GL_TEXTURE31)
    g_pContext->input.currInputTexCoord[target - GL_TEXTURE0] = float2(v[0], v[1]);
}

GLAPI void APIENTRY glNewList(GLuint list, GLenum mode) //// ## EMPTY ## 100% remove from header; don't support display lists
{

}

GLAPI void APIENTRY glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz) //// ## EMPTY ## don't need normals because we do not suport lighting
{
  if (g_pContext == nullptr)
    return;

  g_pContext->input.currInputNormal = float4(nx, ny, nz, 0.0f);
}

GLAPI void APIENTRY glNormal3fv(const GLfloat *v) //// ## EMPTY ## don't need normals because we do not suport lighting
{
  if (g_pContext == nullptr || v == nullptr)
    return;

  g_pContext->input.currInputNormal = float4(v[0], v[1], v[2], 0.0f);
}

GLAPI void APIENTRY glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer) //// ## EMPTY ## don't need normals because we do not suport lighting
{
  if (g_pContext == nullptr)
    return;

  if (g_pContext->logMode <= LOG_ALL)
    *(g_pContext->m_pLog) << "glNormalPointer(" << type << ", " << stride << ", " << stride << ", " << pointer << ")" << std::endl;

#ifdef STRICT_CLIENT_STATE_ARRAYS
  if (!g_pContext->input.vertexNormalPtrEnabled)
  {
    if (g_pContext->logMode <= LOG_ALL)
      *(g_pContext->m_pLog) << "glNormalPointer, WARNING: input.vertexNormalPtrEnabled is disabled" << std::endl;
    return;
  }
#endif

  if (type != GL_FLOAT || stride != 0) // not supported in SC profile
  {
    if (g_pContext->logMode <= LOG_ALL)
      *(g_pContext->m_pLog) << "glNormalPointer, WARNING: (type != GL_FLOAT || stride != 0)" << std::endl;
    g_pContext->input.vertexNormalPointer = nullptr;
    return;
  }

  g_pContext->input.vertexNormalPointer = (float*)pointer;
  g_pContext->input.vertNormalComponents = 2;
}


GLAPI void APIENTRY glPixelStorei(GLenum pname, GLint param)  //// ## PIECE OF SHIT ## 100% remove from header
{

}

GLAPI void APIENTRY glPolygonOffset(GLfloat factor, GLfloat units) //// ## EMPTY ## probably remove from header; don't support polygon offsets
{

}

GLAPI void APIENTRY glPolygonStipple(const GLubyte *mask) //// ## EMPTY ## probably remove from header; don't support polygon sttripple
{

}


GLAPI void APIENTRY glRasterPos3f(GLfloat x, GLfloat y, GLfloat z)  //// ## PIECE OF SHIT ## 100% remove from header
{

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// matrix
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// matrix 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// matrix
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// matrix


GLAPI void APIENTRY glShadeModel(GLenum mode) /// ## EMPTY ## probably remove from header; don't support shading
{

}

GLAPI void APIENTRY glTexEnvfv(GLenum target, GLenum pname, const GLfloat *params)  /// ## EMPTY ##, remove from header
{

}

GLAPI void APIENTRY glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar) /// ## EMPTY ##, remove from header
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// matrix
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// matrix
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// matrix
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// matrix

GLAPI void APIENTRY glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels) /// ## EMPTY ##, remove from header
{

}



