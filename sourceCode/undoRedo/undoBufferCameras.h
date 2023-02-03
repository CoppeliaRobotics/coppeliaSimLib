#pragma once

#include <map>
#include <simMath/7Vector.h>

struct SCamBuff
{
    C7Vector localTr;
    double orthoViewSize;
};

class CUndoBufferCameras
{
public:
    CUndoBufferCameras();
    virtual ~CUndoBufferCameras();

    void preRestoreCameras();
    void restoreCameras();
    void storeCameras();
    void releaseCameras();

private:
    std::map<std::string,SCamBuff> _cameraBuffers;
    std::map<std::string,SCamBuff> _preRestoreCameraBuffers;
    std::map<std::string,SCamBuff> _cameraProxyBuffers;
    std::map<std::string,SCamBuff> _preRestoreCameraProxyBuffers;
};
