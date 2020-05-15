#pragma once

#include "_syncObject_.h"

class CSyncObject : public _CSyncObject_
{
public:
    CSyncObject();
    virtual ~CSyncObject();

    virtual void buildUpdateAndPopulateSynchronizationObject(const std::vector<SSyncRoute>* parentRouting);
    virtual void connectSynchronizationObject();
    virtual void removeSynchronizationObject();

    // Overridden from _CSyncObject_:
    virtual void sendVoid(unsigned char itemId) const;
    virtual void sendBool(bool v,unsigned char itemId) const;
    virtual void sendInt32(int v,unsigned char itemId) const;
    virtual void sendUInt16(unsigned short v,unsigned char itemId) const;
    virtual void sendFloat(float v,unsigned char itemId) const;
    virtual void sendString(const char* str,unsigned char itemId) const;
    virtual void sendInt32Array(const int* arr,size_t count,unsigned char itemId) const;
    virtual void sendFloatArray(const float* arr,size_t count,unsigned char itemId) const;
    virtual void sendQuaternion(const C4Vector* q,unsigned char itemId) const;
    virtual void sendTransformation(const C7Vector* tr,unsigned char itemId) const;
    virtual void sendRandom(void* data,size_t size,unsigned char itemId) const;
};
