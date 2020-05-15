#pragma once

#include "simTypes.h"
#include <vector>
#include "4Vector.h"

struct SSyncRoute
{
    unsigned char objType;
    int objHandle;
};

class _CSyncObject_
{
public:

    _CSyncObject_();
    virtual ~_CSyncObject_();

    void setSyncMsgRouting(const std::vector<SSyncRoute>* ancestorRoute,const SSyncRoute& ownIdentity);
    const std::vector<SSyncRoute>* getSyncMsgRouting() const;
    bool isRoutingSet() const;

    bool getObjectCanSync() const;
    bool setObjectCanSync(bool s);
    bool getObjectCanChange() const;

    virtual void synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg);

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

    static void setOverallSyncEnabled(bool e);
    static bool getOverallSyncEnabled();

protected:  

    bool _objectCanSync;

    std::vector<SSyncRoute> _routing;

    SSyncMsg _msg;
    SSyncRt _rt;

    static bool _overallSyncEnabled;
};
