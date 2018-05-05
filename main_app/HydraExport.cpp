#include "HydraExport.h"

#include <fstream>
#include <sstream>

HydraGeomData::HydraGeomData()
{
  m_ownMemory = false;

  m_data      = NULL;
  m_positions = NULL;
  m_normals   = NULL;
  m_tangents  = NULL;
  m_texcoords = NULL;

  m_triVertIndices = NULL;
  m_triMaterialIndices = NULL;
  m_materialNames = NULL;

  m_matNamesTotalStringSize = 0;
  fileSizeInBytes = 0;
  verticesNum = 0;
  indicesNum = 0;
  materialsNum = 0;
  flags = 0;
}


HydraGeomData::~HydraGeomData()
{
  freeMemIfNeeded();
}

void HydraGeomData::freeMemIfNeeded()
{
  if(m_ownMemory)
    free(m_data);
}

const std::string& HydraGeomData::getMaterialNameByIndex(uint32 a_index)  const { return m_matNameByIndex[a_index]; }
uint32 HydraGeomData::getMaterialIndexByName(const std::string& a_name)  { return m_matIndexByName[a_name]; }

const std::vector<std::string>& HydraGeomData::_getMaterialsNamesVector() const { return m_matNameByIndex; }

uint32 HydraGeomData::getVerticesNumber() const { return verticesNum; }
const float* HydraGeomData::getVertexPositionsFloat4Array() const { return (const float*)m_positions; }
const float* HydraGeomData::getVertexNormalsFloat4Array()   const { return (const float*)m_normals;   }
const float* HydraGeomData::getVertexTangentsFloat4Array()  const { return (const float*)m_tangents;  }
const float* HydraGeomData::getVertexTexcoordFloat2Array()  const { return (const float*)m_texcoords; }

uint32 HydraGeomData::getIndicesNumber() const { return indicesNum; }
const uint32* HydraGeomData::getTriangleVertexIndicesArray()    const { return m_triVertIndices; }
const uint32* HydraGeomData::getTriangleMaterialIndicesArray()  const { return m_triMaterialIndices; }

const float* HydraGeomData::getVertexLightmapTexcoordFloat2Array()  const { return NULL; }
const float* HydraGeomData::getVertexSphericalHarmonicCoeffs()  const {return NULL; }

void HydraGeomData::setData(uint32 a_vertNum, float* a_pos, float* a_norm, float* a_tangent, float* a_texCoord,
                            uint32 a_indicesNum, uint32* a_triVertIndices, uint32* a_triMatIndices)
{
  verticesNum = a_vertNum;
  indicesNum  = a_indicesNum;

  m_positions = a_pos;
  m_normals   = a_norm;
  m_tangents  = a_tangent;

  m_texcoords = a_texCoord;

  m_triVertIndices = a_triVertIndices;
  m_triMaterialIndices = a_triMatIndices;
}

void HydraGeomData::setNewGeometryObject(uint32 a_vertNum, float* a_pos, float* a_norm, float* a_tangent, float* a_texCoord,
                            uint32 a_indicesNum, uint32* a_triVertIndices, uint32* a_triMatIndices)
{
	verticesNum = a_vertNum;
  indicesNum  = a_indicesNum;

  m_positions = a_pos;
  m_normals   = a_norm;
  m_tangents  = a_tangent;

  m_texcoords = a_texCoord;

  m_triVertIndices = a_triVertIndices;
  m_triMaterialIndices = a_triMatIndices;
}


void HydraGeomData::setMaterialNameIndex(const std::string a_name, uint32 a_index)
{
  m_matIndexByName[a_name] = a_index;
}


void HydraGeomData::write(std::ostream& a_out)
{
  if(m_tangents != NULL)
    flags |= HAS_TANGENT;

  fileSizeInBytes = verticesNum*( sizeof(float)*4*2 + sizeof(float)*2 );
  if(flags & HAS_TANGENT)
    fileSizeInBytes += verticesNum*sizeof(float)*4;

  fileSizeInBytes += (indicesNum/3)*4*sizeof(uint32); // 3*num_triangles + num_triangles = 4*num_triangles

  // contruct m_matNameByIndex
  //
  {
    m_matNameByIndex.resize( m_matIndexByName.size() );
    for(auto p = m_matIndexByName.begin(); p != m_matIndexByName.end(); ++p)
      m_matNameByIndex[p->second] = p->first;
  }

  std::stringstream strstr;

  for(size_t i=0; i < m_matNameByIndex.size(); i++)
    strstr << m_matNameByIndex[i].c_str() << "\n";

  const std::string& strData = strstr.str();
  fileSizeInBytes += strData.size();

  materialsNum = m_matNameByIndex.size();

  // write data
  //
  a_out.write((const char*)&fileSizeInBytes, sizeof(uint64));
  a_out.write((const char*)&verticesNum, sizeof(uint32));
  a_out.write((const char*)&indicesNum, sizeof(uint32));
  a_out.write((const char*)&materialsNum, sizeof(uint32));
  a_out.write((const char*)&flags, sizeof(uint32));

  a_out.write((const char*)m_positions, sizeof(float)*4*verticesNum);
  a_out.write((const char*)m_normals, sizeof(float)*4*verticesNum);

  if(flags & HAS_TANGENT)
    a_out.write((const char*)m_tangents, sizeof(float)*4*verticesNum);

  a_out.write((const char*)m_texcoords, sizeof(float)*2*verticesNum);

  a_out.write((const char*)m_triVertIndices, sizeof(uint32)*indicesNum);
  a_out.write((const char*)m_triMaterialIndices, sizeof(uint32)*(indicesNum/3));

  a_out.write(strData.c_str(), strData.size());

}


void HydraGeomData::write(const std::string& a_fileName)
{
  std::ofstream fout(a_fileName.c_str(), std::ios::out | std::ios::binary);
  write(fout);
  fout.flush();
  fout.close();
}


inline const int readInt32(unsigned char* ptr) // THIS IS CORRECT BOTH FOR X86 AND PPC !!!
{
  const unsigned char b0 = ptr[0];
  const unsigned char b1 = ptr[1];
  const unsigned char b2 = ptr[2];
  const unsigned char b3 = ptr[3];

  return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}


inline const uint64 readInt64(unsigned char* ptr) // THIS IS CORRECT BOTH FOR X86 AND PPC !!!
{
  const unsigned char b0 = ptr[0];
  const unsigned char b1 = ptr[1];
  const unsigned char b2 = ptr[2];
  const unsigned char b3 = ptr[3];
  const unsigned char b4 = ptr[4];
  const unsigned char b5 = ptr[5];
  const unsigned char b6 = ptr[6];
  const unsigned char b7 = ptr[7];

  return  (uint64(b7) << 56) | (uint64(b6) << 48) | (uint64(b5) << 40) | (uint64(b4) << 32) | (uint64(b3) << 24) | (uint64(b2) << 16) | (uint64(b1) << 8) | uint64(b0);
}

void convertLittleBigEndian(unsigned int* a_buffer, int a_size)
{
  unsigned char* bbuffer = (unsigned char*)a_buffer;

  for(int i=0;i<a_size;i++)
    a_buffer[i] = readInt32(bbuffer + i*4);
}


#include <iostream>

void HydraGeomData::read(std::istream& a_input)
{
  freeMemIfNeeded();
  m_ownMemory = true;

  struct Info
  {
    uint64 fileSizeInBytes;
    uint32 verticesNum;
    uint32 indicesNum;
    uint32 materialsNum;
    uint32 flags;
  } info;

  unsigned char temp1[sizeof(Info)];
  a_input.read((char*)&temp1[0], sizeof(Info));

  info.fileSizeInBytes = readInt64(&temp1[0]);
  info.verticesNum     = readInt32(&temp1[8]);
  info.indicesNum      = readInt32(&temp1[12]);
  info.materialsNum    = readInt32(&temp1[16]);
  info.flags           = readInt32(&temp1[20]);

  std::cout << "HydraGeomData import:" << std::endl;
  std::cout << "info.fileSizeInBytes = " << info.fileSizeInBytes << std::endl;
  std::cout << "info.verticesNum     = " << info.verticesNum << std::endl;
  std::cout << "info.indicesNum      = " << info.indicesNum << std::endl;
  std::cout << "info.materialsNum    = " << info.materialsNum << std::endl;
  std::cout << "info.flags           = " << info.flags << std::endl;

  fileSizeInBytes = info.fileSizeInBytes;
  verticesNum     = info.verticesNum;
  indicesNum      = info.indicesNum;
  materialsNum    = info.materialsNum;
  flags           = info.flags;

  m_data = new char [info.fileSizeInBytes];
  a_input.read(m_data, info.fileSizeInBytes);

  // std::cout << "[HydraGeomData] data was read" << std::endl;

  char* ptr = m_data;

  m_positions = (float*)ptr; ptr += sizeof(float)*4*verticesNum;
  m_normals   = (float*)ptr; ptr += sizeof(float)*4*verticesNum;

  if(flags & HAS_TANGENT)
  {
    m_tangents = (float*)ptr;
    ptr += sizeof(float)*4*verticesNum;
  }

  m_texcoords   = (float*)ptr; ptr += sizeof(float)*2*verticesNum;

  m_triVertIndices = (uint32*)ptr; ptr += sizeof(uint32)*indicesNum;
  m_triMaterialIndices = (uint32*)ptr; ptr += sizeof(uint32)*(indicesNum/3);


  convertLittleBigEndian((unsigned int*)m_positions, verticesNum*4);
  convertLittleBigEndian((unsigned int*)m_normals, verticesNum*4);
  convertLittleBigEndian((unsigned int*)m_texcoords, verticesNum*2);
  convertLittleBigEndian((unsigned int*)m_triVertIndices, indicesNum);
  convertLittleBigEndian((unsigned int*)m_triMaterialIndices, (indicesNum/3));

  char* matNames = ptr;

  m_matNameByIndex.reserve(materialsNum);
  m_matNameByIndex.resize(0);

  std::stringstream strstr(matNames);
  char temp[256];

  while( m_matNameByIndex.size() < materialsNum )
  {
    strstr.getline(temp, 256);
    m_matNameByIndex.push_back(std::string(temp));
    m_matIndexByName[std::string(temp)] = m_matNameByIndex.size() - 1;
  }

}

void HydraGeomData::read(const std::string& a_fileName)
{
  std::ifstream fin(a_fileName.c_str(), std::ios::binary);

  if (!fin.is_open())
    return;

  read(fin);

  fin.close();
}


