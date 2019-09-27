
#pragma once

class CGlobalGuiTextureContainerBase
{
public:
    static std::string handleVerSpec_getWatermarkStr1(int& tag)
    {
        tag=VREP_TAG;
        return(":/targaFiles/vrepWaterMark.tga");
    }
    static std::string handleVerSpec_getWatermarkStr2(int& tag)
    {
        tag=EDU_TAG;
        return(":/targaFiles/eduWaterMark.tga");
    }
};
