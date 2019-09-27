// This code was heavily inspired by a code snippet I found on the internet. 
// Unfortunately credit cannot be given, since I forgot the original author.!
// Should you know, please let me know!     -marc

#include "vrepMainHeader.h"
#include "tGAFormat.h"
#include "tt.h"
#include <strstream>
#include <fstream>

unsigned char* CTGAFormat::getRGBA(std::istream* inputStream,int size)
{
    unsigned char* rgba=new unsigned char[size*4];
    inputStream->read((char*)rgba,size*4);
    for (int i=0;i<size*4;i+=4)
    {
        unsigned char tmp=rgba[i];
        rgba[i]=rgba[i+2];
        rgba[i+2]=tmp;
    }
    return(rgba);
}

unsigned char* CTGAFormat::getRGB(std::istream* inputStream,int size,bool& isRgba,unsigned char invisibleColor[3])
{
    unsigned char* rgb=new unsigned char[size*3];
    inputStream->read((char*)rgb,size*3);
    for (int i=0;i<size*3;i+=3)
    {
        unsigned char tmp=rgb[i];
        rgb[i]=rgb[i+2];
        rgb[i+2]=tmp;
    }
    if (invisibleColor==nullptr)
    {
        isRgba=false;
        return(rgb);
    }
    else
    {
        isRgba=true;
        unsigned char* rgba=new unsigned char[size*4];
        for (int i=0;i<size;i++)
        {
            rgba[4*i+0]=rgb[3*i+0];
            rgba[4*i+1]=rgb[3*i+1];
            rgba[4*i+2]=rgb[3*i+2];
            if ( (rgb[3*i+0]==invisibleColor[0])&&(rgb[3*i+1]==invisibleColor[1])&&(rgb[3*i+2]==invisibleColor[2]) )
                rgba[4*i+3]=0;
            else
                rgba[4*i+3]=255;
        }
        delete[] rgb;
        return(rgba);
    }
}

unsigned char* CTGAFormat::getImageData(std::string name,int& resX,int& resY,bool& isRgba,unsigned char invisibleColor[3],int bitsPerPixel[1])
{
    std::ifstream* fileStream=new std::ifstream(name.c_str(),std::ios::in|std::ios::binary);
    if (fileStream->fail()) 
    {
        printf("Texture could not be loaded! (getImageData#1)\n");
        delete fileStream;
        return(nullptr);
    }
    unsigned char* retVal=getImageData(fileStream,resX,resY,isRgba,invisibleColor,bitsPerPixel);
    delete fileStream;
    return(retVal);
}

unsigned char* CTGAFormat::getImageData(std::istream* inputStream,int& resX,int& resY,bool& isRgba,unsigned char invisibleColor[3],int bitsPerPixel[1])
{
    unsigned char type[4];
    unsigned char info[7];
    unsigned char *imageData=nullptr;
    int imageBits,size;

    inputStream->read((char*)type,3);
    inputStream->seekg(12,std::ios::beg);
    inputStream->read((char*)info,6);

    if (type[1]!=0||type[2]!=2)
        return(nullptr);

    resX=info[0]+info[1]*256;
    resY=info[2]+info[3]*256;
    imageBits=info[4];
    size=resX*resY;

    if (imageBits!=32&&imageBits!=24)
        return(nullptr);
    if (imageBits==32)
    {
        imageData=getRGBA(inputStream,size);
        isRgba=true;
    }
    else if (imageBits==24)
        imageData=getRGB(inputStream,size,isRgba,invisibleColor);
    if (bitsPerPixel!=nullptr)
        bitsPerPixel[0]=imageBits;
    return(imageData);
}

unsigned char* CTGAFormat::getV_RGBA(VArchive& inputStream,int size)
{
    unsigned char* rgba=new unsigned char[size*4];
    for (int j=0;j<size*4;j++)
        inputStream>>((char*)rgba)[j];
    for (int i=0;i<size*4;i+=4)
    {
        unsigned char tmp=rgba[i];
        rgba[i]=rgba[i+2];
        rgba[i+2]=tmp;
    }
    return(rgba);
}

unsigned char* CTGAFormat::getV_RGB(VArchive& inputStream,int size,bool& isRgba,unsigned char invisibleColor[3])
{
    unsigned char* rgb=new unsigned char[size*3];
    
    for (int j=0;j<size*3;j++)
        inputStream >> ((char*)rgb)[j];

    for (int i=0;i<size*3;i+=3)
    {
        unsigned char tmp=rgb[i];
        rgb[i]=rgb[i+2];
        rgb[i+2]=tmp;
    }

    if (invisibleColor==nullptr)
    {
        isRgba=false;
        return(rgb);
    }
    else
    {
        isRgba=true;
        unsigned char* rgba=new unsigned char[size*4];
        for (int i=0;i<size;i++)
        {
            rgba[4*i+0]=rgb[3*i+0];
            rgba[4*i+1]=rgb[3*i+1];
            rgba[4*i+2]=rgb[3*i+2];
            if ( (rgb[3*i+0]==invisibleColor[0])&&(rgb[3*i+1]==invisibleColor[1])&&(rgb[3*i+2]==invisibleColor[2]) )
                rgba[4*i+3]=0;
            else
                rgba[4*i+3]=255;
        }
        delete[] rgb;
        return(rgba);
    }
}

unsigned char* CTGAFormat::getV_ImageData(VArchive& inputStream,int& resX,int& resY,bool& isRgba,unsigned char invisibleColor[3],int bitsPerPixel[1])
{
    unsigned char type[4];
    unsigned char info[7];
    unsigned char *imageData=nullptr;
    int imageBits,size;

    for (int i=0;i<3;i++)
        inputStream >> ((char*)type)[i];

    char dummy;
    for (int i=0;i<9;i++)
        inputStream >> dummy;

    for (int i=0;i<6;i++)
        inputStream >> ((char*)info)[i];

    if (type[1]!=0||type[2]!=2)
        return(nullptr);

    resX=info[0]+info[1]*256;
    resY=info[2]+info[3]*256;
    imageBits=info[4];
    size=resX*resY;

    if (imageBits!=32 && imageBits!=24)
        return(nullptr);

    if (imageBits==32)
    {
        imageData=getV_RGBA(inputStream,size);
        isRgba=true;
    }
    else if (imageBits==24)
        imageData=getV_RGB(inputStream,size,isRgba,invisibleColor);
    if (bitsPerPixel!=nullptr)
        bitsPerPixel[0]=imageBits;
    return(imageData);
}

unsigned char* CTGAFormat::getQ_ImageData(const std::string& fileAndPathName,int& resX,int& resY,bool& isRgba,unsigned char invisibleColor[3],int bitsPerPixel[1])
{
    unsigned char* retVal=nullptr;
    try
    {
        VFile file(fileAndPathName.c_str(),VFile::READ|VFile::SHARE_DENY_NONE);
        VArchive archive(&file,VArchive::LOAD);
        retVal=getV_ImageData(archive,resX,resY,isRgba,invisibleColor,bitsPerPixel);
        archive.close();
        file.close();
    }
    catch(VFILE_EXCEPTION_TYPE e)
    {
        printf("Failed loading resource: %s\n",fileAndPathName.c_str());
        VFile::reportAndHandleFileExceptionError(e);
    }
    return(retVal);
}
