#include "swgl.h"
#include "pugixml.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

static int g_chunk_id = 0;

template <typename VectorT>
void PutArrayToNode(const VectorT& a_vector, pugi::xml_node a_node)
{
  if (a_vector.size() == 0)
    return;

  std::stringstream nameStream;
  nameStream << "frame0_data/chunk_" << g_chunk_id << ".bin";

  a_node.append_attribute("size")     = a_vector.size();
  a_node.append_attribute("bytesize") = a_vector.size()*sizeof(a_vector[0]);

  std::string fileName = nameStream.str();
  a_node.append_attribute("location") = fileName.c_str();

  std::ofstream fout(fileName.c_str(), std::ios::binary);
  fout.write((const char*)&a_vector[0], a_vector.size() * sizeof(a_vector[0]));
  fout.close();

  g_chunk_id++;
}

std::string ToString(const float4x4& a_matrix)
{
  assert(false); // not implementes!
  std::stringstream matrixStream;
  //for(int y=0;i<16;i++)
  //  matrixStream << a_matrix.L()[i] << " ";
  return matrixStream.str();
}


void SavePSOToXMLNode(const Pipeline_State_Object& state, pugi::xml_node psoNode,
                      int* a_texData, int a_texW, int a_texH, int a_pitch)
{
  psoNode.append_child("alphaBlendEnabled").text() = state.alphaBlendEnabled;
  psoNode.append_child("colorWriteEnabled").text() = state.colorWriteEnabled;
  psoNode.append_child("cullFaceEnabled").text() = state.cullFaceEnabled;
  psoNode.append_child("cullFaceMode").text() = state.cullFaceMode;
  psoNode.append_child("depthTestEnabled").text() = state.depthTestEnabled;
  psoNode.append_child("depthWriteEnabled").text() = state.depthWriteEnabled;
  psoNode.append_child("lineSmoothWidthRange").text() = state.lineSmoothWidthRange;
  psoNode.append_child("lineWidth").text() = state.lineWidth;

  std::string projMatrix = ToString(state.projMatrix);
  psoNode.append_child("projMatrix").text() = projMatrix.c_str();
  
  // save 2D texture (state.slot_GL_TEXTURE_2D)
  //
  {
    int dummyTex[4] = { int(0xFFFFFFFF), int(0xFFFFFFFF), int(0xFFFFFFFF), int(0xFFFFFFFF) };
    int texW = 2;
    int texH = 2;
    int* texData = &dummyTex[0];
    int pitch = 2;

    if (state.texure2DEnabled && a_texData != nullptr)
    {
      texData = a_texData;
      texW    = a_texW;
      texH    = a_texH;
      pitch   = a_pitch;
    }
    
    auto texNode = psoNode.append_child("GL_TEXTURE_2D");

    texNode.append_attribute("width")  = texW;
    texNode.append_attribute("height") = a_texH;
    texNode.append_attribute("pitch")  = texW;

    std::vector<int> texDataV(texW*texH);
    for (int y = 0; y < texH; y++)
    {
      for (int x = 0; x < texW; x++)
        texDataV[y*texW + x] = texData[y*pitch + x];
    }

    PutArrayToNode(texDataV, texNode);
  }

  psoNode.append_child("stencilMask").text() = state.stencilMask;
  psoNode.append_child("stencilTestEnabled").text() = state.stencilTestEnabled;
  psoNode.append_child("stencilValue").text() = state.stencilValue;

  psoNode.append_child("stencilWriteEnabled").text() = state.stencilWriteEnabled;

  auto viewport = psoNode.append_child("viewport");

  viewport.append_attribute("x") = state.viewport[0];
  viewport.append_attribute("y") = state.viewport[1];
  viewport.append_attribute("z") = state.viewport[2];
  viewport.append_attribute("w") = state.viewport[3];

  std::string worldViewMatrix = ToString(state.worldViewMatrix);
  psoNode.append_child("worldViewMatrix").text() = worldViewMatrix.c_str();
}


void AppendBatchToFrame(const Batch* a_batch, pugi::xml_node a_frame, 
                        int* a_texData, int a_texW, int a_texH, int a_pitch)
{
  auto psoNode = a_frame.append_child("pipeline_state_object");
  SavePSOToXMLNode(a_batch->state, psoNode, /* texture data --> */ a_texData, a_texW, a_texH, a_pitch);

  auto posNode  = a_frame.append_child("vpos");
  auto normNode = a_frame.append_child("vnorm");
  auto colrNode = a_frame.append_child("vcolor");
  auto texcNode = a_frame.append_child("vtexc");
  auto indNode  = a_frame.append_child("indices");
 
  PutArrayToNode(a_batch->vertPos,      posNode);
  //PutArrayToNode(a_batch->vertNorm,     normNode);
  PutArrayToNode(a_batch->vertColor,    colrNode);
  PutArrayToNode(a_batch->vertTexCoord, texcNode);
  PutArrayToNode(a_batch->indices,      indNode);

  // save texture
}

void LoadBatch(Batch* a_batch, pugi::xml_node a_batchNode)
{

}


void SaveBatchToXML(const Batch* a_batch, char* a_fileName,
                    int* a_texData, int a_texW, int a_texH, int a_pitch) 
{
  pugi::xml_document doc;
  auto frame0 = doc.append_child("frame");

  frame0.append_attribute("id") = 0;

  AppendBatchToFrame(a_batch, frame0, /* texture data --> */ a_texData, a_texW, a_texH, a_pitch);

  doc.save_file(a_fileName, "  ");
}

void LoadBatchFromXML(Batch* a_batch, char* a_fileName)
{
  pugi::xml_document doc;
  auto loadResult = doc.load_file(a_fileName);
  if (!loadResult)
  {
    std::cout << "LoadBatchFromXML, failed to load xml from file " << a_fileName << std::endl;
    return;
  }

  auto frame0 = doc.child("frame");

  if(frame0 == nullptr)
  {
    std::cout << "LoadBatchFromXML, tag 'frame' didn't found " << std::endl;
    return;
  }

  auto batch0 = frame0.child("batch");

  if (batch0 == nullptr)
  {
    std::cout << "LoadBatchFromXML, tag 'frame->batch' didn't found " << std::endl;
    return;
  }

  LoadBatch(a_batch, batch0);

  // auto batch1 = batch0.next_sibling();

}
