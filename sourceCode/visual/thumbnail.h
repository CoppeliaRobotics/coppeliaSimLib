
#pragma once

#include "ser.h"
#include "7Vector.h"

#define MODEL_BROWSER_TEXT_SPACE_HEIGHT (32)

class CThumbnail
{
public:
    CThumbnail();
    virtual ~CThumbnail();
    static void compressThumbnail(const char* uncompressedRGBAImage,char* compressedRGBImage);
    static void uncompressThumbnail(const char* compressedRGBImage,char* uncompressedRGBAImage);
    void display(int posX,int posY,const char* name,int sc);
    void setUncompressedThumbnailImage(const char* uncompressedRGBAImage,bool rgba,bool verticalFlip);
    void setCompressedThumbnailImage(const char* compressedRGBImage);
    void setUncompressedThumbnailImageFromFloat(const float* uncompressedRGBAImage);
    void clearThumbnailImage();
    void setRandomImage();
    bool hasImage();
    bool getCompressedImage(char* compressedRGBImage);
    char* getPointerToUncompressedImage();
    bool copyUncompressedImageToBuffer(char* buffer);
    void serialize(CSer& ar,bool forceCompressedSaving=false);
    void serializeAdditionalModelInfos(CSer& ar,C7Vector& modelTr,C3Vector& modelBBSize,float& modelNonDefaultTranslationStepSize);
    CThumbnail* copyYourself();
    void copyFrom(CThumbnail* it);

protected:
    char* _thumbnailRGBAImage;
    bool _compressData;
};
