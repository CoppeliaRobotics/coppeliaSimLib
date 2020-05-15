#include "extIkSer.h"
#include <string.h>

CExtIkSer::CExtIkSer()
{
    _reading=false;
}

CExtIkSer::~CExtIkSer()
{
}

void CExtIkSer::writeFloat(float a)
{
    _writeBuffer.push_back(((unsigned char*)&a)[0]);
    _writeBuffer.push_back(((unsigned char*)&a)[1]);
    _writeBuffer.push_back(((unsigned char*)&a)[2]);
    _writeBuffer.push_back(((unsigned char*)&a)[3]);
}

void CExtIkSer::writeInt(int a)
{
    _writeBuffer.push_back(((unsigned char*)&a)[0]);
    _writeBuffer.push_back(((unsigned char*)&a)[1]);
    _writeBuffer.push_back(((unsigned char*)&a)[2]);
    _writeBuffer.push_back(((unsigned char*)&a)[3]);
}

void CExtIkSer::writeUInt(unsigned int a)
{
    _writeBuffer.push_back(((unsigned char*)&a)[0]);
    _writeBuffer.push_back(((unsigned char*)&a)[1]);
    _writeBuffer.push_back(((unsigned char*)&a)[2]);
    _writeBuffer.push_back(((unsigned char*)&a)[3]);
}

void CExtIkSer::writeWord(unsigned short a)
{
    _writeBuffer.push_back(((unsigned char*)&a)[0]);
    _writeBuffer.push_back(((unsigned char*)&a)[1]);
}

void CExtIkSer::writeByte(unsigned char a)
{
    _writeBuffer.push_back(a);
}

void CExtIkSer::writeChar(char a)
{
    _writeBuffer.push_back((unsigned char)a);
}

void CExtIkSer::writeString(const char* string)
{
    for (int i=0;i<int(strlen(string));i++)
        _writeBuffer.push_back((unsigned char)string[i]);
    _writeBuffer.push_back(0);
}

unsigned char* CExtIkSer::getBuffer(int& bufferLength)
{
    bufferLength=int(_writeBuffer.size());
    return(&_writeBuffer[0]);
}
