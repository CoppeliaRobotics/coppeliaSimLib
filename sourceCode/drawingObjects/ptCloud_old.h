#pragma once

#include <vector>

class CPtCloud_old  
{
public:
    CPtCloud_old(int pageMask,int layerMask,int parentHandle,int options,float pointSize,int ptCnt,const float* vertices,const unsigned char* colors,const float* normals,const unsigned char* defaultColors);
    virtual ~CPtCloud_old();

    void draw(int displayAttrib);
    bool isPersistent() const;
    void setObjectID(int id);
    int getObjectID() const;
    void setObjectUniqueId();
    int getObjectUniqueId() const;

    void pushAddEvent();
    bool announceObjectWillBeErased(int objectHandleAttachedTo);

    int _id;
    int _uniqueId;
    int _pageMask;
    int _layerMask;
    int _parentHandle;
    int _parentUniqueId;
    int _options;
    float _pointSize;
    std::vector<float> _vertices;
    std::vector<float> _colors;
    std::vector<float> _normals;
    float _defaultColors[16];
};
