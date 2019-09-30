#pragma once

#include <vector>

class CExtIkSer  
{
public:
    CExtIkSer(); // writing

    virtual ~CExtIkSer();

    void writeFloat(float a);
    void writeInt(int a);
    void writeUInt(unsigned int a);
    void writeWord(unsigned short a);
    void writeByte(unsigned char a);
    void writeChar(char a);
    void writeString(const char* string);

    unsigned char* getBuffer(int& bufferLength);

private:
    bool _reading;
    std::vector<unsigned char> _writeBuffer;
};
