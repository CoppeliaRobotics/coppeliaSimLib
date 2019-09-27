// This code was heavily inspired by a code snippet I found on the internet. 
// Unfortunately credit cannot be given, since I forgot the original author. Stupid stupid me!!!
// Should you know, please let me know!     -marc

#pragma once

#include <iostream>
#include "vArchive.h"

// FULLY STATIC CLASS
class CTGAFormat  
{
public:
    static unsigned char* getRGBA(std::istream* inputStream,int size);
    static unsigned char* getRGB(std::istream* inputStream,int size,bool& isRgba,unsigned char invisibleColor[3]=nullptr);
    static unsigned char* getImageData(std::string name,int& resX,int& resY,bool& isRgba,unsigned char invisibleColor[3]=nullptr,int bitsPerPixel[1]=nullptr);
    static unsigned char* getImageData(std::istream* inputStream,int& resX,int& resY,bool& isRgba,unsigned char invisibleColor[3]=nullptr,int bitsPerPixel[1]=nullptr);
    static unsigned char* getV_RGBA(VArchive& inputStream,int size);
    static unsigned char* getV_RGB(VArchive& inputStream,int size,bool& isRgba,unsigned char invisibleColor[3]=nullptr);
    static unsigned char* getV_ImageData(VArchive& inputStream,int& resX,int& resY,bool& isRgba,unsigned char invisibleColor[3]=nullptr,int bitsPerPixel[1]=nullptr);
    static unsigned char* getQ_ImageData(const std::string& fileAndPath,int& resX,int& resY,bool& isRgba,unsigned char invisibleColor[3]=nullptr,int bitsPerPixel[1]=nullptr);
};
