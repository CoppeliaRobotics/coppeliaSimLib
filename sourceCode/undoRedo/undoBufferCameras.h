
#pragma once

#include "vrepMainHeader.h"
#include "7Vector.h"

struct SCamBuff
{
    C7Vector localTr;
    float orthoViewSize;
};

class CUndoBufferCameras
{
public:
    CUndoBufferCameras();
    virtual ~CUndoBufferCameras();

    void preRestoreCameras();
    void restoreCameras();
    void storeCameras();

private:
    std::map<std::string,SCamBuff> _cameraBuffers;
    std::map<std::string,SCamBuff> _preRestoreCameraBuffers;
    std::map<std::string,SCamBuff> _cameraProxyBuffers;
    std::map<std::string,SCamBuff> _preRestoreCameraProxyBuffers;
};
