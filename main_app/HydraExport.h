#pragma once

#include <vector>
#include <string>
#include <unordered_map>

typedef unsigned long long int uint64;
typedef unsigned int uint32;


struct HydraGeomData
{
  HydraGeomData();
  ~HydraGeomData();

  //
  //
  void write(const std::string& a_fileName);
  void write(std::ostream& a_out);

  void read(const std::string& a_fileName);
  void read(std::istream& a_input);

  const std::string& getMaterialNameByIndex(uint32 a_index)  const;
  uint32 getMaterialIndexByName(const std::string& a_name);

  const std::vector<std::string>& _getMaterialsNamesVector() const; // not recomended to use this, but possible

  // common vertex attributes
  //
  uint32 getVerticesNumber() const;
  const float* getVertexPositionsFloat4Array() const; 
  const float* getVertexNormalsFloat4Array()  const; 
  const float* getVertexTangentsFloat4Array()  const; 
  const float* getVertexTexcoordFloat2Array()  const; 

  // advanced attributes, for various types of lightmaps
  //
  const float* getVertexLightmapTexcoordFloat2Array()  const; 
  const float* getVertexSphericalHarmonicCoeffs()  const; 

  // per triangle data
  //
  uint32 getIndicesNumber() const;                       // return 3*num_triangles
  const uint32* getTriangleVertexIndicesArray() const;   // 3*num_triangles
  const uint32* getTriangleMaterialIndicesArray() const; // 1*num_triangles 

  //
  //
  void setData(uint32 a_vertNum, float* a_pos, float* a_norm, float* a_tangent, float* a_texCoord, 
               uint32 a_indicesNum, uint32* a_triVertIndices, uint32* a_triMatIndices);
	void setNewGeometryObject(uint32 a_vertNum, float* a_pos, float* a_norm, float* a_tangent, float* a_texCoord, 
               uint32 a_indicesNum, uint32* a_triVertIndices, uint32* a_triMatIndices);

  void setMaterialNameIndex(const std::string a_name, uint32 a_index);

protected:

  enum GEOM_FLAGS{ HAS_TANGENT = 1, 
                   HAS_LIGHTMAP_TEXCOORDS = 2, 
                   HAS_HARMONIC_COEFFS = 4 };

  // size info
  //
  uint64 fileSizeInBytes;
  uint32 verticesNum;
  uint32 indicesNum;
  uint32 materialsNum;
  uint32 flags;


  //
  //
  float* m_positions;
  float* m_normals;
  float* m_tangents; 

  float* m_texcoords;

  uint32* m_triVertIndices;
  uint32* m_triMaterialIndices;

  char*   m_materialNames;
  uint32  m_matNamesTotalStringSize;
  
  char*   m_data; // this is a full dump of the file

  //
  //
  void freeMemIfNeeded();
  bool m_ownMemory;
  
  std::unordered_map<std::string, int> m_matIndexByName;
  std::vector<std::string>             m_matNameByIndex;

};


