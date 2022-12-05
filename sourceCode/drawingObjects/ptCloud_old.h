#pragma once

#include "simTypes.h"
#include <vector>

class CPtCloud_old  
{
public:
    CPtCloud_old(int pageMask,int layerMask,int parentHandle,int options,double pointSize,int ptCnt,const double* vertices,const unsigned char* colors,const double* normals,const unsigned char* defaultColors);
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
    double _pointSize;
    std::vector<double> _vertices;
    std::vector<float> _colors;
    std::vector<double> _normals;
    float _defaultColors[16];
};
