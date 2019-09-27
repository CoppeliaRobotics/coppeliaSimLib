
#pragma once

#include <string>

// FULLY STATIC CLASS
class CImageLoaderSaver
{
public:
    static unsigned char* load(const char* filename,int* resX,int* resY,int* colorComponents,int desiredColorComponents,int scaleTo=0);
    static unsigned char* loadQTgaImageData(const char* fileAndPath,int& resX,int& resY,bool& rgba,unsigned char invisibleColor[3]=nullptr,int bitsPerPixel[1]=nullptr);
    static unsigned char* getScaledImage(const unsigned char* originalImg,int colorComponents,int originalX,int originalY,int newX,int newY);
    static unsigned char* getScaledImage(const unsigned char* originalImg,const int resolIn[2],int resolOut[2],int options);
    static bool transformImage(unsigned char* img,int resX,int resY,int options);
    static bool save(const unsigned char* data,const int resolution[2],int options,const char* filename,int quality,std::string* buffer);
    static unsigned char* load(int resolution[2],int options,const char* filename,void* reserved);
};
