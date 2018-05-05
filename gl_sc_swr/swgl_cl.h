#pragma once

#include "LiteMath.h"
#include "swgl.h"

struct TriangleListGPU
{
  TriangleListGPU() : v1(0), v2(0), v3(0), c1(0), c2(0), c3(0), t1(0), t2(0), t3(0), triNum(0), bb(0) {}

  void resize(cl_context, size_t n);

  cl_mem v1;
  cl_mem v2;
  cl_mem v3;

  cl_mem c1;
  cl_mem c2;
  cl_mem c3;

  cl_mem t1;
  cl_mem t2;
  cl_mem t3;

  cl_mem bb;

  size_t triNum;
};

struct BatchCL;

struct ContextCL
{
  ContextCL() : ctx(0), cmdQueue(0), platform(0), device(0), matrices(0), matricesNumber(0),
                triangleBinCounters(0), triangleBins(0), colorBuffer(0), depthBuffer(0), currTex(0), width(0), height(0), totalTriNumber(0) {}

  cl_context       ctx;        // OpenCL context
  cl_command_queue cmdQueue;   // OpenCL command que
  cl_platform_id   platform;   // OpenCL platform
  cl_device_id     device;     // OpenCL device

  // mem objects
  //
  cl_mem matrices;
  int matricesNumber;


  cl_mem triangleBinCounters;
  cl_mem triangleBins;
  cl_mem colorBuffer;
  cl_mem depthBuffer;
  cl_mem currTex;

  int width;
  int height;
  int totalTriNumber;

  TriangleListGPU tlist;

  CLProgram programs;

  void memsetu32(cl_mem buff, unsigned int a_val, size_t a_size);
  void memsetf4(cl_mem buff, float4 a_val, size_t a_size, size_t a_offset);

  void runKernel_VertexShaderAndTriangleSetUp(BatchCL* a_pBatch, int a_batchId, int a_offset, int a_size, int fbW, int fbH);
  void runKernel_BinRasterizer(int a_size, int fbW, int fbH);
  void runKernel_FineRasterizer(int fbW, int fbH);

  void runKernel_TestCompaction();

  void waitIfDebug(const char* file, int line) const;
};


struct BatchCL
{
  BatchCL() : indices(0), vertPos(0), vertNorm(0), vertColor(0), vertTexCoord(0)
  {
    m_indicesSize  = 0;
    m_posSize      = 0;
    m_normSize     = 0;
    m_colorSize    = 0;
    m_texCoordSize = 0;
  }

  BatchCL(Batch* a_pBatch, ContextCL* a_pCL, int* a_texData, int a_texW, int a_texH, int a_pitch) : indices(0), vertPos(0), vertNorm(0), vertColor(0), vertTexCoord(0)
  {
    m_indicesSize  = int(a_pBatch->indices.size());
    m_posSize      = int(a_pBatch->vertPos.size());
    m_normSize     = int(a_pBatch->vertNorm.size());
    m_colorSize    = int(a_pBatch->vertColor.size());
    m_texCoordSize = int(a_pBatch->vertTexCoord.size());

    cl_mem_flags flags = CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR;

    cl_int ciErr1 = CL_SUCCESS;

    if (m_indicesSize > 0)
      indices = clCreateBuffer(a_pCL->ctx, flags, m_indicesSize*sizeof(int), (void*)&a_pBatch->indices[0], &ciErr1);

    if (m_posSize > 0)
      vertPos = clCreateBuffer(a_pCL->ctx, flags, m_posSize*sizeof(float4), (void*)&a_pBatch->vertPos[0], &ciErr1);

    if (m_normSize > 0)
      vertNorm = clCreateBuffer(a_pCL->ctx, flags, m_normSize*sizeof(float4), (void*)&a_pBatch->vertNorm[0], &ciErr1);

    if (m_colorSize > 0)
      vertColor = clCreateBuffer(a_pCL->ctx, flags, m_colorSize*sizeof(float4), (void*)&a_pBatch->vertColor[0], &ciErr1);

    if (m_texCoordSize > 0)
      vertTexCoord = clCreateBuffer(a_pCL->ctx, flags, m_texCoordSize*sizeof(float2), (void*)&a_pBatch->vertTexCoord[0], &ciErr1);

    worldViewMatrix = a_pBatch->state.worldViewMatrix;
    projMatrix      = a_pBatch->state.projMatrix;

    // create 2D texture
    //
    int dummyTex[4] = { int(0xFFFFFFFF), int(0xFFFFFFFF), int(0xFFFFFFFF), int(0xFFFFFFFF) };
    //int dummyTex[4] = { 0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFFFFFFFF };
    int texW = 2;
    int texH = 2;
    int* texData = &dummyTex[0];
    int pitch = 2;

    if (a_pBatch->state.texure2DEnabled && a_texData != nullptr)
    {
      texData = a_texData;
      texW    = a_texW;
      texH    = a_texH;
      pitch   = a_pitch;
    }

    cl_image_format imgFormat;

    imgFormat.image_channel_order     = CL_RGBA;
    imgFormat.image_channel_data_type = CL_UNORM_INT8;
   
    texture = clCreateImage2D(a_pCL->ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &imgFormat, size_t(texW), size_t(texH), size_t(pitch) * 4, texData, &ciErr1);

    int a = 2;
  }

  ~BatchCL()
  {
    // clReleaseMemObject(indices);      indices      = 0;
    // clReleaseMemObject(vertPos);      vertPos      = 0;
    // clReleaseMemObject(vertNorm);     vertNorm     = 0;
    // clReleaseMemObject(vertColor);    vertColor    = 0;
    // clReleaseMemObject(vertTexCoord); vertTexCoord = 0;
    // clReleaseMemObject(texture);      texture = 0;
  }

  cl_mem indices;       // std::vector<int>   
  cl_mem vertPos;       // std::vector<float4>
  cl_mem vertNorm;      // std::vector<float4>
  cl_mem vertColor;     // std::vector<float4>
  cl_mem vertTexCoord;  // std::vector<float2>

  cl_mem texture;

  int m_indicesSize;
  int m_posSize;
  int m_normSize;
  int m_colorSize;
  int m_texCoordSize;

  float4x4 worldViewMatrix;
  float4x4 projMatrix;
};


