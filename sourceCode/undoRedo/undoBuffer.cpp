
#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "v_rep_internal.h"
#include "undoBuffer.h"



CUndoBuffer::CUndoBuffer(const std::vector<char>& fullBuffer,int bufferId,CUndoBufferCameras* camBuff)
{
    FUNCTION_DEBUG;
    buffer.insert(buffer.end(),fullBuffer.begin(),fullBuffer.end());
    _sameCountFromBeginning=0;
    _sameCountFromEnd=0;
    _bufferId=bufferId;
    _cameraBuffers=camBuff;
}

CUndoBuffer::~CUndoBuffer()
{
    FUNCTION_DEBUG;
    delete _cameraBuffers;
}

CUndoBufferCameras* CUndoBuffer::getCameraBuffers()
{
    return(_cameraBuffers);
}

int CUndoBuffer::getBufferId()
{
    return(_bufferId);
}

bool CUndoBuffer::finalize(const std::vector<char>& previousFullBuffer)
{ // Return value false means: this buffer is exactly the same as "previousFullBuffer"
    FUNCTION_DEBUG;
    _sameCountFromBeginning=0;
    _sameCountFromEnd=0;
    int minBufferLength=SIM_MIN(int(previousFullBuffer.size()),int(buffer.size()));
    for (int i=0;i<minBufferLength;i++)
    {
        if (previousFullBuffer[i]!=buffer[i])
            break;
        _sameCountFromBeginning++;
    }
    if ( (previousFullBuffer.size()==buffer.size())&&(_sameCountFromBeginning==int(buffer.size())) )
    { // Buffer content is same!
        return(false); // buffer content is same!
    }
    if (_sameCountFromBeginning==int(buffer.size()))
    { // Previous buffer's beginning is same as this buffer entirely!
        buffer.clear();
        {
            std::vector<char> ttmmpp(buffer);
            buffer.swap(ttmmpp); // release the reserved memory that is not used!
        }
        return(true);
    }
    // We release what is not needed anymore:
    buffer.erase(buffer.begin(),buffer.begin()+_sameCountFromBeginning);
    {
        std::vector<char> ttmmpp(buffer);
        buffer.swap(ttmmpp); // release the reserved memory that is not used!
    }
    if (_sameCountFromBeginning==int(previousFullBuffer.size()))
    { // Previous buffer is entirely same as this buffer's beginning!
        return(true);
    }

    // Now we check from the end:
    while (true)
    {
        int j=(int)previousFullBuffer.size()-1-_sameCountFromEnd;
        int k=(int)buffer.size()-1-_sameCountFromEnd;
        if ((j<0)||(k<0))
            break;
        if (previousFullBuffer[j]!=buffer[k])
            break;
        _sameCountFromEnd++;
    }
    // We release what is not needed anymore:
    buffer.erase(buffer.end()-_sameCountFromEnd,buffer.end());
    {
        std::vector<char> ttmmpp(buffer);
        buffer.swap(ttmmpp); // release the reserved memory that is not used!
    }
    return(true);
}

void CUndoBuffer::getRestored(const std::vector<char>* previousFullBuffer,std::vector<char>& restoredBuffer)
{ // previousFullBuffer can be nullptr. In that case _sameCountFromBeginning and _sameCountFromEnd should be 0
    FUNCTION_DEBUG;
    restoredBuffer.clear();
    if (previousFullBuffer!=nullptr)
        restoredBuffer.insert(restoredBuffer.end(),previousFullBuffer->begin(),previousFullBuffer->begin()+_sameCountFromBeginning);
    restoredBuffer.insert(restoredBuffer.end(),buffer.begin(),buffer.end());
    if (previousFullBuffer!=nullptr)
        restoredBuffer.insert(restoredBuffer.end(),previousFullBuffer->end()-_sameCountFromEnd,previousFullBuffer->end());
}

void CUndoBuffer::updateWithFullBuffer(const std::vector<char>& fullBuffer)
{
    FUNCTION_DEBUG;
    _sameCountFromBeginning=0;
    _sameCountFromEnd=0;
    buffer.clear();
    buffer.insert(buffer.end(),fullBuffer.begin(),fullBuffer.end());
    {
        std::vector<char> ttmmpp(buffer);
        buffer.swap(ttmmpp); // release the reserved memory that is not used!
    }
}
