#include "_syncObject_.h"

bool _CSyncObject_::_overallSyncEnabled=true;

_CSyncObject_::_CSyncObject_()
{
    _objectCanSync=false;

    _rt.objHandles[0]=-1;
    _rt.objTypes[0]=255;
    _rt.objHandles[1]=-1;
    _rt.objTypes[1]=255;
    _rt.objHandles[2]=-1;
    _rt.objTypes[2]=255;
}

_CSyncObject_::~_CSyncObject_()
{
}

void _CSyncObject_::setSyncMsgRouting(const std::vector<SSyncRoute>* ancestorRoute,const SSyncRoute& ownIdentity)
{
    if (ancestorRoute!=nullptr)
        _routing.assign(ancestorRoute->begin(),ancestorRoute->end());
    else
        _routing.clear();
    _routing.push_back(ownIdentity);
    for (size_t i=0;i<_routing.size();i++)
    {
        _rt.objHandles[i]=_routing[i].objHandle;
        _rt.objTypes[i]=_routing[i].objType;
    }
}

const std::vector<SSyncRoute>* _CSyncObject_::getSyncMsgRouting() const
{
    return(&_routing);
}

bool _CSyncObject_::isRoutingSet() const
{
    return(_routing.size()>0);
}

bool _CSyncObject_::getObjectCanSync() const
{
    return(_objectCanSync); // slave can sync when not incoming msg underway
}

bool _CSyncObject_::getObjectCanChange() const
{
    return(true); // master always true, slave can change when incoming msg underway
}

void _CSyncObject_::synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg)
{
}

void _CSyncObject_::sendVoid(unsigned char itemId) const
{
}

void _CSyncObject_::sendBool(bool v,unsigned char itemId) const
{
}

void _CSyncObject_::sendInt32(int v,unsigned char itemId) const
{
}

void _CSyncObject_::sendUInt16(unsigned short v,unsigned char itemId) const
{
}

void _CSyncObject_::sendFloat(float v,unsigned char itemId) const
{
}

void _CSyncObject_::sendString(const char* str,unsigned char itemId) const
{
}

void _CSyncObject_::sendInt32Array(const int* arr,size_t count,unsigned char itemId) const
{
}

void _CSyncObject_::sendFloatArray(const float* arr,size_t count,unsigned char itemId) const
{
}

void _CSyncObject_::sendRandom(void* data,size_t size,unsigned char itemId) const
{
}

void _CSyncObject_::sendQuaternion(const C4Vector* q,unsigned char itemId) const
{
}

void _CSyncObject_::sendTransformation(const C7Vector* tr,unsigned char itemId) const
{
}

bool _CSyncObject_::setObjectCanSync(bool s)
{
    bool retVal=true;
    if (s&&(!_overallSyncEnabled))
        retVal=false;
    else
    {
        _objectCanSync=s;
        retVal=s;
    }
    return(retVal);
}

void _CSyncObject_::setOverallSyncEnabled(bool e)
{ // static function
    _overallSyncEnabled=e;
}

bool _CSyncObject_::getOverallSyncEnabled()
{ // static function
    return(_overallSyncEnabled);
}
