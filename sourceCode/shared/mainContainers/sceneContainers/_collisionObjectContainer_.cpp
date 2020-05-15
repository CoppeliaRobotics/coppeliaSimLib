#include "_collisionObjectContainer_.h"
#include "simConst.h"

_CCollisionObjectContainer_::_CCollisionObjectContainer_()
{
    SSyncRoute rt;
    rt.objHandle=-1;
    rt.objType=sim_syncobj_collision;
    setSyncMsgRouting(nullptr,rt);
    setObjectCanSync(true);
}

_CCollisionObjectContainer_::~_CCollisionObjectContainer_()
{ // beware, the current world could be nullptr
    while (_collisionObjects.size()!=0)
        _removeObject(_collisionObjects[0]->getObjectHandle());
}


void _CCollisionObjectContainer_::_addObject(CCollisionObject* newCollObj)
{
    _collisionObjects.push_back(newCollObj);
}

void _CCollisionObjectContainer_::_removeObject(int objectHandle)
{
    for (size_t i=0;i<_collisionObjects.size();i++)
    {
        if (_collisionObjects[i]->getObjectHandle()==objectHandle)
        {
            delete _collisionObjects[i];
            _collisionObjects.erase(_collisionObjects.begin()+i);
            break;
        }
    }
}

size_t _CCollisionObjectContainer_::getObjectCount() const
{
    return(_collisionObjects.size());
}

CCollisionObject* _CCollisionObjectContainer_::getObjectFromIndex(size_t index) const
{
    CCollisionObject* retVal=nullptr;
    if (index<_collisionObjects.size())
        retVal=_collisionObjects[index];
    return(retVal);
}

CCollisionObject* _CCollisionObjectContainer_::getObjectFromHandle(int objectHandle) const
{
    for (size_t i=0;i<_collisionObjects.size();i++)
    {
        if (_collisionObjects[i]->getObjectHandle()==objectHandle)
            return(_collisionObjects[i]);
    }
    return(nullptr);
}

CCollisionObject* _CCollisionObjectContainer_::getObjectFromName(const char* objName) const
{
    for (size_t i=0;i<_collisionObjects.size();i++)
    {
        if (_collisionObjects[i]->getObjectName().compare(objName)==0)
            return(_collisionObjects[i]);
    }
    return(nullptr);
}

void _CCollisionObjectContainer_::synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg)
{ // Overridden from _CSyncObject_
    if (routing[0].objType==sim_syncobj_collision)
    {
        if (routing[0].objHandle!=-1)
        { // Msg is for a collision object
            CCollisionObject* collObj=getObjectFromHandle(routing[0].objHandle);
            if (collObj==nullptr)
            {
                if ( (msg.msg==sim_syncobj_collisionobject_create)&&(routing.size()==1) ) // check also size (some msgs have same ids in different scopes)
                {
                    collObj=new CCollisionObject(((int*)msg.data)[0],((int*)msg.data)[1]);
                    collObj->setObjectHandle(routing[0].objHandle);
                    _addObject(collObj);
                }
            }
            else
            {
                if ( (msg.msg==sim_syncobj_collisionobject_delete)&&(routing.size()==1) ) // check also size (some msgs have same ids in different scopes)
                    _removeObject(routing[0].objHandle);
                else
                {
                    routing.erase(routing.begin());
                    collObj->synchronizationMsg(routing,msg);
                }
            }
        }
        else
        { // Msg is for this collision object container
        }
    }
}
