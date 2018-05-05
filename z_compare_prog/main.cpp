#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "FreeImage.h"
#pragma comment(lib, "FreeImage.lib")

struct RGBA
{
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;
};

void LoadImageLDR(const std::string& a_fileName, std::vector<RGBA>* pOutData, int* pW, int* pH)
{
  const char* filename = a_fileName.c_str();

  FREE_IMAGE_FORMAT fif = FIF_UNKNOWN; // image format
  FIBITMAP *dib(NULL), *converted(NULL);
  BYTE* bits(NULL);                    // pointer to the image data
  unsigned int width(0), height(0);    //image width and height


  //check the file signature and deduce its format
  //if still unknown, try to guess the file format from the file extension
  //
  fif = FreeImage_GetFileType(filename, 0);
  if (fif == FIF_UNKNOWN)
    fif = FreeImage_GetFIFFromFilename(filename);

  if (fif == FIF_UNKNOWN)
  {
    std::cerr << "FreeImage failed to guess file image format: " << a_fileName.c_str() << std::endl;
    return;
  }

  //check that the plugin has reading capabilities and load the file
  //
  if (FreeImage_FIFSupportsReading(fif))
    dib = FreeImage_Load(fif, filename);
  else
  {
    std::cerr << "FreeImage does not support file image format: " << a_fileName.c_str() << std::endl;
    return;
  }

  bool invertY = false; //(fif != FIF_BMP);

  if (!dib)
  {
    std::cerr << "FreeImage failed to load image: " << a_fileName.c_str() << std::endl;
    return;
  }

  //unsigned int bitsPerPixel = FreeImage_GetBPP(dib);

  converted = FreeImage_ConvertTo32Bits(dib);

  bits      = FreeImage_GetBits(converted);
  width     = FreeImage_GetWidth(converted);
  height    = FreeImage_GetHeight(converted);

  if ((bits == 0) || (width == 0) || (height == 0))
  {
    std::cerr << "FreeImage failed for undefined reason, file : " << a_fileName.c_str() << std::endl;
    return;
  }

  pOutData->resize(width*height);

  
  for (size_t i = 0; i < pOutData->size(); i++)
  {
    RGBA color;

    color.r = bits[4 * i + 0];
    color.g = bits[4 * i + 1];
    color.b = bits[4 * i + 2];
    color.a = bits[4 * i + 3];

    pOutData->at(i) = color;
  }

  /*
  for (int y = 0; y<height; y++)
  {
    int lineOffset1 = y*width;
    int lineOffset2 = y*width;
    if (invertY)
      lineOffset2 = (height - y - 1)*width;

    for (int x = 0; x<width; x++)
    {
      RGBA color;

      int offset1 = lineOffset1 + x;
      int offset2 = lineOffset2 + x;

      color.r = bits[4 * offset2 + 2];
      color.g = bits[4 * offset2 + 1];
      color.b = bits[4 * offset2 + 0];
      color.a = bits[4 * offset2 + 3];

      pOutData->at(offset1) = color;
    }
  }*/

  FreeImage_Unload(converted);
  FreeImage_Unload(dib);

  *pW = width;
  *pH = height;
}

struct RGBA32F
{
  float r;
  float g;
  float b;
  float a;
};


RGBA ToneMaping(RGBA32F a_color)
{
  if (a_color.r > 1.0f) a_color.r = 1.0f; 
  if (a_color.r < 0.0f) a_color.r = 0.0f;

  if (a_color.g > 1.0f) a_color.g = 1.0f;
  if (a_color.g < 0.0f) a_color.g = 0.0f;

  if (a_color.b > 1.0f) a_color.b = 1.0f;
  if (a_color.b < 0.0f) a_color.b = 0.0f;

  if (a_color.a > 1.0f) a_color.a = 1.0f;
  if (a_color.a < 0.0f) a_color.a = 0.0f;

  RGBA color2;

  color2.r = a_color.r * 255;
  color2.g = a_color.g * 255;
  color2.b = a_color.b * 255;
  color2.a = a_color.a * 255;

  return color2;
}


void LoadImageHDR(const std::string& a_fileName, std::vector<RGBA32F>* pOutData, int* pW, int* pH)
{
  const char* filename = a_fileName.c_str();

  FREE_IMAGE_FORMAT fif = FIF_UNKNOWN; // image format
  FIBITMAP *dib(NULL), *converted(NULL);
  BYTE* bits(NULL);                    // pointer to the image data
  unsigned int width(0), height(0);    //image width and height


  //check the file signature and deduce its format
  //if still unknown, try to guess the file format from the file extension
  //
  fif = FreeImage_GetFileType(filename, 0);
  if (fif == FIF_UNKNOWN)
    fif = FreeImage_GetFIFFromFilename(filename);

  if (fif == FIF_UNKNOWN)
  {
    std::cerr << "FreeImage failed to guess file image format: " << a_fileName.c_str() << std::endl;
    return;
  }

  //check that the plugin has reading capabilities and load the file
  //
  if (FreeImage_FIFSupportsReading(fif))
    dib = FreeImage_Load(fif, filename);
  else
  {
    std::cerr << "FreeImage does not support file image format: " << a_fileName.c_str() << std::endl;
    return;
  }

  bool invertY = false; //(fif != FIF_BMP);

  if (!dib)
  {
    std::cerr << "FreeImage failed to load image: " << a_fileName.c_str() << std::endl;
    return;
  }

  //unsigned int bitsPerPixel = FreeImage_GetBPP(dib);

  converted = FreeImage_ConvertToRGBF(dib);

  bits   = FreeImage_GetBits(converted);
  width  = FreeImage_GetWidth(converted);
  height = FreeImage_GetHeight(converted);

  if ((bits == 0) || (width == 0) || (height == 0))
  {
    std::cerr << "FreeImage failed for undefined reason, file : " << a_fileName.c_str() << std::endl;
    return;
  }

  float* fbits = (float*)bits;

  pOutData->resize(width*height);

  for (size_t i = 0; i < pOutData->size(); i++)
  {
    RGBA32F color;

    color.r = fbits[3 * i + 0];
    color.g = fbits[3 * i + 1];
    color.b = fbits[3 * i + 2];
    color.a = 0.0f;

    pOutData->at(i) = color;
  }

  FreeImage_Unload(converted);
  FreeImage_Unload(dib);

  *pW = width;
  *pH = height;
}

void SaveImageToFile(const std::string& a_fileName, int w, int h, unsigned int* data)
{
  FIBITMAP* dib = FreeImage_Allocate(w, h, 32);

  BYTE* bits = FreeImage_GetBits(dib);
  BYTE* data2 = (BYTE*)data;
  for (int i = 0; i<w*h; i++)
  {
    bits[4 * i + 0] = data2[4 * i + 0];
    bits[4 * i + 1] = data2[4 * i + 1];
    bits[4 * i + 2] = data2[4 * i + 2];
    bits[4 * i + 3] = 255;
  }

  if (!FreeImage_Save(FIF_PNG, dib, a_fileName.c_str()))
    std::cerr << "SaveImageToFile(): FreeImage_Save error on " << a_fileName.c_str() << std::endl;

  FreeImage_Unload(dib);
}


int compareImages(const std::vector<RGBA>& a_image1, int w, int h, const std::vector<RGBA32F>& a_image2, int w2, int h2)
{
  if (w != w2 || h != h2)
    return 100000.0f;

  if (a_image1.size() != a_image2.size())
    return 100000.0f;

  int diff = 0;

  for (int i = 0; i < a_image1.size(); i++)
  {
    RGBA color1 = a_image1[1];
    RGBA color2 = ToneMaping(a_image2[i]);

    diff += (abs(color1.r - color2.r) + abs(color1.g - color2.g) + abs(color1.b - color2.b));
  }

  return diff;
}


int sqr(int x) { return x*x; }

float mse(const std::vector<RGBA>& image1, const std::vector<RGBA>& image2)
{
  if (image1.size() != image2.size())
    return 100000.0f;

  float summ = 0;

  for (int i = 0; i < image1.size(); i++)
  {
    RGBA c1 = image1[i];
    RGBA c2 = image2[i];

    summ += 0.25f*float( sqr(c1.r - c2.r) + sqr(c1.g - c2.g) + sqr(c1.b - c2.b) + sqr(c1.a - c2.a) );
  }

  return float(summ) / float(image1.size());
}

int main(int agrc, const char** argv)
{
  const int NTests = 23;

  for (int i = 0; i < NTests; i++)
  {
    std::stringstream fileNameOut;

    fileNameOut.fill('0');
    fileNameOut.width(2);
    fileNameOut << i;

    std::string fileName1 = std::string("../main_app/z_test_out/z_test_") + fileNameOut.str() + ".bmp";
    std::string fileName2 = std::string("../main_app/z_test_out/z_ref_")  + fileNameOut.str() + ".bmp";

    std::string fileName3 = std::string("../main_app/z_test_out2/z_test_") + fileNameOut.str() + ".png";
    std::string fileName4 = std::string("../main_app/z_test_out2/z_ref_")  + fileNameOut.str() + ".png";

    std::vector<RGBA>    ldrData;
    int w = 0, h = 0;

    std::vector<RGBA>    ldrData2;
    int w2 = 0, h2 = 0;

    // if(w == 0 || w2 == 0)

    LoadImageLDR(fileName1.c_str(), &ldrData, &w, &h);
    LoadImageLDR(fileName2.c_str(), &ldrData2, &w2, &h2);

    SaveImageToFile(fileName3.c_str(), w, h,   (unsigned int*)&ldrData[0]);
    SaveImageToFile(fileName4.c_str(), w2, h2, (unsigned int*)&ldrData2[0]);

    std::cout.fill('0');
    std::cout.width(2);

    if (w != w2 || h != h2)
    {
      std::cout << "test (" << i << "); mse = 100000.0" << std::endl;
    }
    else
    {
      float mseDiff = mse(ldrData, ldrData2);
      std::cout << "test (";
      std::cout.fill('0');
      std::cout.width(2);
      std::cout << i;
      std::cout << "); mse = " << mseDiff << std::endl;
    }
  }

  return 0;
}


