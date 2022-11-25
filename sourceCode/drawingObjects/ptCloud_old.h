#pragma once

#include "simTypes.h"
#include <vector>

class CPtCloud_old  
{
public:
    CPtCloud_old(int pageMask,int layerMask,int parentHandle,int options,floatDouble pointSize,int ptCnt,const floatDouble* vertices,const unsigned char* colors,const floatDouble* normals,const unsigned char* defaultColors);
    virtual ~CPtCloud_old();

    void draw(int displayAttrib);
    bool isPersistent() const;
    void setObjectID(int id);
    int getObjectID() const;
    void setObjectUniqueId();
    long long int getObjectUniqueId() const;

    void pushAddEvent();
    bool announceObjectWillBeErased(int objectHandleAttachedTo);

    int _id;
    long long int _uniqueId;
    int _pageMask;
    int _layerMask;
    int _parentHandle;
    long long int _parentUniqueId;
    int _options;
    floatDouble _pointSize;
    std::vector<floatDouble> _vertices;
    std::vector<floatDouble> _colors;
    std::vector<floatDouble> _normals;
    floatDouble _defaultColors[16];
};
