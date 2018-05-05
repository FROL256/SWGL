#pragma once

#include "gl_sc.h"

#define GL_QUADS 0x0007

typedef double GLdouble;
typedef unsigned char GLboolean;


GLAPI void APIENTRY glDeleteTextures(GLsizei n, const GLuint * textures);

GLAPI void APIENTRY glTexCoord2f(GLfloat x, GLfloat y);
GLAPI void APIENTRY glTexCoord2d(GLdouble s, GLdouble t);
GLAPI void APIENTRY glColor3f(GLfloat red, GLfloat green, GLfloat blue);
GLAPI void APIENTRY glClearDepth(GLclampf depth);
GLAPI void APIENTRY glClipPlane(GLenum plane, const GLdouble *equation);

GLAPI void APIENTRY glMultMatrixd(const GLdouble *m);

GLAPI void APIENTRY glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
GLAPI void APIENTRY glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble nearVal, GLdouble farVal);
GLAPI void APIENTRY glTranslated(GLdouble x, GLdouble y, GLdouble z);
GLAPI void APIENTRY glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);

// mostly not supported, empty or convert types
//

GLAPI void APIENTRY glEvalMesh1(GLenum mode, GLint i1, GLint  i2);

GLAPI void APIENTRY glEvalCoord1f(GLfloat u);
GLAPI void APIENTRY glEvalCoord2f(GLfloat u, GLfloat v);
GLAPI void APIENTRY glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
GLAPI void APIENTRY glEvalPoint2(GLint i, GLint j);

GLAPI void APIENTRY glMap1f(GLenum  target, GLfloat u1, GLfloat u2, GLint stride,  GLint order, const GLfloat *points);
GLAPI void APIENTRY glMap2f(GLenum  target,GLfloat u1,  GLfloat u2, GLint ustride, GLint uorder, GLfloat v1,GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);

GLAPI void APIENTRY glMapGrid1f(GLint un, GLfloat u1, GLfloat u2);
GLAPI void APIENTRY glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
GLAPI void APIENTRY glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);

GLAPI void APIENTRY glPolygonMode(GLenum face, GLenum mode);
GLAPI void APIENTRY glPushAttrib(GLbitfield mask);
GLAPI void APIENTRY glPopAttrib(GLbitfield mask);

GLAPI void APIENTRY glTexImage1D(GLenum  target, GLint  level, GLint  internalFormat, GLsizei  width, GLint  border, GLenum  format, GLenum  type, const GLvoid*  data);

GLAPI void APIENTRY glDrawBuffer(GLenum buf);
GLAPI void APIENTRY glReadBuffer(GLenum mode);

GLAPI GLboolean APIENTRY glIsTexture(GLuint texture);

// utils

void gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar);


