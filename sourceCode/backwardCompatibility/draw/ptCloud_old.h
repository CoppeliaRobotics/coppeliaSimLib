#pragma once

#include <simLib/simTypes.h>
#include <vector>

class CPtCloud_old
{
  public:
    CPtCloud_old(int pageMask, int layerMask, int parentHandle, int options, double pointSize, int ptCnt,
                 const double* vertices, const unsigned char* colors, const double* normals,
                 const unsigned char* defaultColors);
    virtual ~CPtCloud_old();

    bool isPersistent() const;
    void setObjectID(int id);
    int getObjectID() const;
    void setObjectUniqueId();
    int64_t getObjectUniqueId() const;

    void pushAddEvent();
    bool announceObjectWillBeErased(int objectHandleAttachedTo);

#ifdef SIM_WITH_GUI
    void draw(int displayAttrib);
#endif

    int _id;
    int64_t _uniqueId;
    int _pageMask;
    int _layerMask;
    int _parentHandle;
    int64_t _parentUniqueId;
    int _options;
    double _pointSize;
    std::vector<double> _vertices;
    std::vector<float> _colors;
    std::vector<double> _normals;
    float _defaultColors[16];
};
