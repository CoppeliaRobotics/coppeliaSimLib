#include "_distanceObjectContainer_old.h"
#include "simConst.h"

_CDistanceObjectContainer_old::_CDistanceObjectContainer_old()
{
    SSyncRoute rt;
    rt.objHandle=-1;
    rt.objType=sim_syncobj_distance;
    setSyncMsgRouting(nullptr,rt);
    setObjectCanSync(true);
}

_CDistanceObjectContainer_old::~_CDistanceObjectContainer_old()
{ // beware, the current world could be nullptr
    while (_distanceObjects.size()!=0)
        _removeObject(_distanceObjects[0]->getObjectHandle());
}

void _CDistanceObjectContainer_old::_addObject(CDistanceObject_old* newDistObj)
{
    _distanceObjects.push_back(newDistObj);
}

void _CDistanceObjectContainer_old::_removeObject(int objectHandle)
{
    for (size_t i=0;i<_distanceObjects.size();i++)
    {
        if (_distanceObjects[i]->getObjectHandle()==objectHandle)
        {
            delete _distanceObjects[i];
            _distanceObjects.erase(_distanceObjects.begin()+i);
            break;
        }
    }
}

size_t _CDistanceObjectContainer_old::getObjectCount() const
{
    return(_distanceObjects.size());
}

CDistanceObject_old* _CDistanceObjectContainer_old::getObjectFromIndex(size_t index) const
{
    CDistanceObject_old* retVal=nullptr;
    if (index<_distanceObjects.size())
        retVal=_distanceObjects[index];
    return(retVal);
}

CDistanceObject_old* _CDistanceObjectContainer_old::getObjectFromHandle(int objectHandle) const
{
    for (size_t i=0;i<_distanceObjects.size();i++)
    {
        if (_distanceObjects[i]->getObjectHandle()==objectHandle)
            return(_distanceObjects[i]);
    }
    return(nullptr);
}

CDistanceObject_old* _CDistanceObjectContainer_old::getObjectFromName(const char* objName) const
{
    for (size_t i=0;i<_distanceObjects.size();i++)
    {
        if (_distanceObjects[i]->getObjectName().compare(objName)==0)
            return(_distanceObjects[i]);
    }
    return(nullptr);
}

void _CDistanceObjectContainer_old::synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg)
{ // Overridden from _CSyncObject_
    if (routing[0].objType==sim_syncobj_distance)
    {
        if (routing[0].objHandle!=-1)
        { // Msg is for a distance object
            CDistanceObject_old* distObj=getObjectFromHandle(routing[0].objHandle);
            if (distObj==nullptr)
            {
                if ( (msg.msg==sim_syncobj_distanceobject_create)&&(routing.size()==1) ) // check also size (some msgs have same ids in different scopes)
                {
                    distObj=new CDistanceObject_old(((int*)msg.data)[0],((int*)msg.data)[1]);
                    distObj->setObjectHandle(routing[0].objHandle);
                    _addObject(distObj);
                }
            }
            else
            {
                if ( (msg.msg==sim_syncobj_distanceobject_delete)&&(routing.size()==1) ) // check also size (some msgs have same ids in different scopes)
                    _removeObject(routing[0].objHandle);
                else
                {
                    routing.erase(routing.begin());
                    distObj->synchronizationMsg(routing,msg);
                }
            }
        }
        else
        { // Msg is for this distance object container
        }
    }
}
