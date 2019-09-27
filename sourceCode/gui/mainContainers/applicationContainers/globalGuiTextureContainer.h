
#pragma once

#include "textureObject.h"

class CGlobalGuiTextureContainer
{
public:
    CGlobalGuiTextureContainer();
    virtual ~CGlobalGuiTextureContainer();

    void addObject(int pictureIndex,int sizeX,int sizeY,bool rgba,bool horizFlip,bool vertFlip,unsigned char* data);
    void startTextureDisplay(int pictureIndex);
    void endTextureDisplay();
    void removeAllObjects();

private:
    std::vector<CTextureObject*> _allTextureObjects;
    std::vector<int> _allPictureIndices;
};
