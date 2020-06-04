#include "syncObject.h"
#include "pluginContainer.h"

#define MSG_SEND_ENABLED

CSyncObject::CSyncObject()
{
}

CSyncObject::~CSyncObject()
{
}

void CSyncObject::buildUpdateAndPopulateSynchronizationObject(const std::vector<SSyncRoute>* parentRouting)
{
    printf("CoppeliaSim warning: CSyncObject::buildUpdateAndPopulateSynchronizationObject was not overridden!\n");
}

void CSyncObject::connectSynchronizationObject()
{
    printf("CoppeliaSim warning: CSyncObject::connectSynchronizationObject was not overridden!\n");
}

void CSyncObject::removeSynchronizationObject(bool localReferencesToItOnly)
{
    printf("CoppeliaSim warning: CSyncObject::removeSynchronizationObject was not overridden!\n");
}

void CSyncObject::sendVoid(unsigned char itemId) const
{ // Overridden from _CSyncObject_
#ifdef MSG_SEND_ENABLED
    SSyncRt rt;
    for (size_t i=0;i<3;i++)
    {
        rt.objTypes[i]=_rt.objTypes[i];
        rt.objHandles[i]=_rt.objHandles[i];
    }
    SSyncMsg msg;
    msg.msg=itemId;
    msg.data=nullptr;
    msg.dataSize=0;
    CPluginContainer::syncMsg(&msg,&rt);
#endif
}

void CSyncObject::sendBool(bool v,unsigned char itemId) const
{ // Overridden from _CSyncObject_
#ifdef MSG_SEND_ENABLED
    SSyncRt rt;
    for (size_t i=0;i<3;i++)
    {
        rt.objTypes[i]=_rt.objTypes[i];
        rt.objHandles[i]=_rt.objHandles[i];
    }
    SSyncMsg msg;
    msg.msg=itemId;
    msg.data=&v;
    msg.dataSize=sizeof(v);
    CPluginContainer::syncMsg(&msg,&rt);
#endif
}

void CSyncObject::sendInt32(int v,unsigned char itemId) const
{ // Overridden from _CSyncObject_
#ifdef MSG_SEND_ENABLED
    SSyncRt rt;
    for (size_t i=0;i<3;i++)
    {
        rt.objTypes[i]=_rt.objTypes[i];
        rt.objHandles[i]=_rt.objHandles[i];
    }
    SSyncMsg msg;
    msg.msg=itemId;
    msg.data=&v;
    msg.dataSize=sizeof(v);
    CPluginContainer::syncMsg(&msg,&rt);
#endif
}

void CSyncObject::sendUInt16(unsigned short v,unsigned char itemId) const
{ // Overridden from _CSyncObject_
#ifdef MSG_SEND_ENABLED
    SSyncRt rt;
    for (size_t i=0;i<3;i++)
    {
        rt.objTypes[i]=_rt.objTypes[i];
        rt.objHandles[i]=_rt.objHandles[i];
    }
    SSyncMsg msg;
    msg.msg=itemId;
    msg.data=&v;
    msg.dataSize=sizeof(v);
    CPluginContainer::syncMsg(&msg,&rt);
#endif
}

void CSyncObject::sendFloat(float v,unsigned char itemId) const
{ // Overridden from _CSyncObject_
#ifdef MSG_SEND_ENABLED
    SSyncRt rt;
    for (size_t i=0;i<3;i++)
    {
        rt.objTypes[i]=_rt.objTypes[i];
        rt.objHandles[i]=_rt.objHandles[i];
    }
    SSyncMsg msg;
    msg.msg=itemId;
    msg.data=&v;
    msg.dataSize=sizeof(v);
    CPluginContainer::syncMsg(&msg,&rt);
#endif
}

void CSyncObject::sendString(const char* str,unsigned char itemId) const
{ // Overridden from _CSyncObject_
#ifdef MSG_SEND_ENABLED
    SSyncRt rt;
    for (size_t i=0;i<3;i++)
    {
        rt.objTypes[i]=_rt.objTypes[i];
        rt.objHandles[i]=_rt.objHandles[i];
    }
    SSyncMsg msg;
    msg.msg=itemId;
    msg.data=(char*)str;
    msg.dataSize=strlen(str)+1;
    CPluginContainer::syncMsg(&msg,&rt);
#endif
}

void CSyncObject::sendInt32Array(const int* arr,size_t count,unsigned char itemId) const
{ // Overridden from _CSyncObject_
#ifdef MSG_SEND_ENABLED
    SSyncRt rt;
    for (size_t i=0;i<3;i++)
    {
        rt.objTypes[i]=_rt.objTypes[i];
        rt.objHandles[i]=_rt.objHandles[i];
    }
    SSyncMsg msg;
    msg.msg=itemId;
    msg.data=(int*)arr;
    msg.dataSize=count;
    CPluginContainer::syncMsg(&msg,&rt);
#endif
}

void CSyncObject::sendFloatArray(const float* arr,size_t count,unsigned char itemId) const
{ // Overridden from _CSyncObject_
#ifdef MSG_SEND_ENABLED
    SSyncRt rt;
    for (size_t i=0;i<3;i++)
    {
        rt.objTypes[i]=_rt.objTypes[i];
        rt.objHandles[i]=_rt.objHandles[i];
    }
    SSyncMsg msg;
    msg.msg=itemId;
    msg.data=(float*)arr;
    msg.dataSize=count;
    CPluginContainer::syncMsg(&msg,&rt);
#endif
}

void CSyncObject::sendQuaternion(const C4Vector* q,unsigned char itemId) const
{ // Overridden from _CSyncObject_
#ifdef MSG_SEND_ENABLED
    SSyncRt rt;
    for (size_t i=0;i<3;i++)
    {
        rt.objTypes[i]=_rt.objTypes[i];
        rt.objHandles[i]=_rt.objHandles[i];
    }
    SSyncMsg msg;
    msg.msg=itemId;
    msg.data=(float*)q->data;
    msg.dataSize=4;
    CPluginContainer::syncMsg(&msg,&rt);
#endif
}

void CSyncObject::sendTransformation(const C7Vector* tr,unsigned char itemId) const
{ // Overridden from _CSyncObject_
#ifdef MSG_SEND_ENABLED
    SSyncRt rt;
    for (size_t i=0;i<3;i++)
    {
        rt.objTypes[i]=_rt.objTypes[i];
        rt.objHandles[i]=_rt.objHandles[i];
    }
    SSyncMsg msg;
    msg.msg=itemId;
    float a[7];
    tr->getInternalData(a);
    msg.data=a;
    msg.dataSize=7;
    CPluginContainer::syncMsg(&msg,&rt);
#endif
}

void CSyncObject::sendRandom(void* data,size_t size,unsigned char itemId) const
{ // Overridden from _CSyncObject_
#ifdef MSG_SEND_ENABLED
    SSyncRt rt;
    for (size_t i=0;i<3;i++)
    {
        rt.objTypes[i]=_rt.objTypes[i];
        rt.objHandles[i]=_rt.objHandles[i];
    }
    SSyncMsg msg;
    msg.msg=itemId;
    msg.data=data;
    msg.dataSize=size;
    CPluginContainer::syncMsg(&msg,&rt);
#endif
}

