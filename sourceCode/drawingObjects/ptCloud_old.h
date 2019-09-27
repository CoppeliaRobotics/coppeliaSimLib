
#pragma once

#include "vrepMainHeader.h"

class CPtCloud_old  
{
public:
    CPtCloud_old(int pageMask,int layerMask,int objectHandle,int options,float pointSize,int ptCnt,const float* vertices,const unsigned char* colors,const float* normals,const unsigned char* defaultColors);
    virtual ~CPtCloud_old();

    void draw(int displayAttrib);
    bool isPersistent() const;
    void setObjectID(int id);
    int getObjectID() const;
    bool announceObjectWillBeErased(int objectHandleAttachedTo);

    int _id;
    int _pageMask;
    int _layerMask;
    int _objectHandle;
    int _options;
    float _pointSize;
    std::vector<float> _vertices;
    std::vector<float> _colors;
    std::vector<float> _normals;
    float _defaultColors[16];
};
