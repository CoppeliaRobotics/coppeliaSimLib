
#pragma once

#include <undoBufferCameras.h>

class CUndoBuffer
{
public:
    CUndoBuffer(const std::vector<char>& fullBuffer,int bufferId,CUndoBufferCameras* camBuff);
    virtual ~CUndoBuffer();
    bool finalize(const std::vector<char>& previousFullBuffer);
    void updateWithFullBuffer(const std::vector<char>& fullBuffer);
    void getRestored(const std::vector<char>* previousFullBuffer,std::vector<char>& restoredBuffer);
    CUndoBufferCameras* getCameraBuffers();
    int getBufferId();

    std::vector<char> buffer;

private:
    CUndoBufferCameras* _cameraBuffers;

    int _sameCountFromBeginning;
    int _sameCountFromEnd;
    int _bufferId;
};
