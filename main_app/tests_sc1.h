#pragma once

#include "../gl_sc_swr/config.h"

#ifdef WIN32
  #include <windows.h>
#else
  #define USE_SWGL
#endif

void ThrowExceptionOnGLError(int line, const char *file);

#undef CHECK_GL_ERRORS
#define CHECK_GL_ERRORS ThrowExceptionOnGLError(__LINE__,__FILE__)

void gluPerspective2(float fovy, float aspect, float zNear, float zFar);

void test_all(int width, int height);

void test01_colored_triangle();
void test02_nehe_lesson1_simplified();
void test03_several_triangles();
void test04_pyramid_and_cube_3d();
void test05_texture();
void test06_triangle_line();
void test07_triangle_fan();
void test08_vert_pointer1();
void test09_vert_pointer2_and_several_textures();
void test10_load_matrix_mult_matrix();

void test11_alpha_tex_and_transp();
void test12_rect_tex();
void test13_lines();
void test14_transparent_cube();
void test15_simple_stencil();

void test16_tri_strip();
void test17_line_strip();
void test18_line_points();
void test19_push_pop_matrix();

void test20_glftustum();
void test21_clip_2d();
//void test22_clip_3d();
void test22_change_viewort_size();
void test23_draw_elements();
//void test24_draw_elements_terrain();

void test25_clip_triangles(int width, int height, float a_rot);

void test_box_tri_overlap();

void demo01_colored_triangle(float rtri);
void demo03_many_small_dynamic_triangles();
void demo04_pyramid_and_cube_3d(int width, int height, float algle1, float angle2);
void demo05_texture_3D(int width, int height, float algle1, float angle2);
void demo07_triangle_fan(int width, int height, const float xRot, const float yRot);
void demo14_transparent_cube(int width, int height, const float xRot, const float yRot);
void demo19_cubes(int width, int height, float algle1, float angle2);
void demo24_draw_elements_terrain(int width, int height, float algle1, float angle2);

void demo25_teapot(int width, int height, float algle1, float angle2);
void demo26_teapots9(int width, int height, float algle1, float angle2);

#include <vector>

void SaveBMP(const wchar_t* fname, const int* pixels, int w, int h);
std::vector<int> LoadBMP(const wchar_t* fname, int* w, int* h);

