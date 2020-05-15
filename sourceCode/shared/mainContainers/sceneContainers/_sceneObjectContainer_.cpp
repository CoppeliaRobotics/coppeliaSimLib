#include "_sceneObjectContainer_.h"
#include "jointObject.h"
#include "dummy.h"
#include "simConst.h"
#include "app.h"

_CSceneObjectContainer_::_CSceneObjectContainer_()
{
    SSyncRoute rt;
    rt.objHandle=-1;
    rt.objType=sim_syncobj_dummy; // doesn't matter, as long as it is a scene object
    setSyncMsgRouting(nullptr,rt);
    setObjectCanSync(true);
}

_CSceneObjectContainer_::~_CSceneObjectContainer_()
{
    while (_objects.size()!=0)
        _removeObject(_objects[0]->getObjectHandle());
}

bool _CSceneObjectContainer_::doesObjectExist(const CSceneObject* obj) const
{
    bool retVal=false;
    for (size_t i=0;i<_objects.size();i++)
    {
        if (obj==_objects[i])
        {
            retVal=true;
            break;
        }
    }
    return(retVal);
}

size_t _CSceneObjectContainer_::getObjectCount() const
{
    return(_objects.size());
}

CSceneObject* _CSceneObjectContainer_::getObjectFromIndex(size_t index) const
{
    CSceneObject* retVal=nullptr;
    if (index<_objects.size())
        retVal=_objects[index];
    return(retVal);
}

CSceneObject* _CSceneObjectContainer_::getObjectFromHandle(int objectHandle) const
{
    std::map<int,CSceneObject*>::const_iterator it=_objectHandleMap.find(objectHandle);
    if (it!=_objectHandleMap.end())
        return(it->second);
    return(nullptr);
}

void _CSceneObjectContainer_::_addObject(CSceneObject* object)
{
    _objects.push_back(object);
    object->setSelected(false);
    _objectHandleMap[object->getObjectHandle()]=object;
}

bool _CSceneObjectContainer_::_removeObject(int objectHandle)
{
    bool retVal=false;
    std::map<int,CSceneObject*>::iterator mapIt=_objectHandleMap.find(objectHandle);
    _objectHandleMap.erase(mapIt);

    for (size_t i=0;i<_objects.size();i++)
    {
        if (_objects[i]->getObjectHandle()==objectHandle)
        {
            delete _objects[i];
            _objects.erase(_objects.begin()+i);
            retVal=true;
            break;
        }
    }
    return(retVal);
}

const std::vector<int>* _CSceneObjectContainer_::getSelectedObjectHandlesPtr() const
{
    return(&_selectedObjectHandles);
}

bool _CSceneObjectContainer_::setSelectedObjectHandles(const std::vector<int>* v)
{
    bool diff=false;
    if (v==nullptr)
        diff=(_selectedObjectHandles.size()>0);
    else
    {
        diff=(v->size()!=_selectedObjectHandles.size());
        if (!diff)
        {
            for (size_t i=0;i<_selectedObjectHandles.size();i++)
            {
                if (_selectedObjectHandles[i]!=v->at(i))
                {
                    diff=true;
                    break;
                }
            }
        }
    }
    if (diff)
    {
        // First make sure that handles are valid
        std::vector<int> w;
        if (v!=nullptr)
        {
            for (size_t i=0;i<v->size();i++)
            {
                CSceneObject* it=getObjectFromHandle(v->at(i));
                if (it!=nullptr)
                    w.push_back(it->getObjectHandle());
            }
        }
        if (getObjectCanChange())
        {
            for (size_t i=0;i<_selectedObjectHandles.size();i++)
            {
                CSceneObject* it=getObjectFromHandle(_selectedObjectHandles[i]);
                if (it!=nullptr)
                    it->setSelected(false);
            }
            _selectedObjectHandles.clear();
            if (v!=nullptr)
            {
                for (size_t i=0;i<w.size();i++)
                {
                    CSceneObject* it=getObjectFromHandle(w[i]);
                    it->setSelected(true);
                }
                _selectedObjectHandles.assign(w.begin(),w.end());
            }
        }
        if (getObjectCanSync())
            _setSelectedObjectHandles_send(&w);
    }
    return(diff&&getObjectCanChange());
}

void _CSceneObjectContainer_::_setSelectedObjectHandles_send(const std::vector<int>* v) const
{
    if ( (v==nullptr)||(v->size()==0) )
        sendInt32Array(nullptr,0,sim_syncobj_sceneobjectcont_selection);
    else
        sendInt32Array(&v->at(0),v->size(),sim_syncobj_sceneobjectcont_selection);
}

size_t _CSceneObjectContainer_::getSelectionCount() const
{
    return(_selectedObjectHandles.size());
}

int _CSceneObjectContainer_::getObjectHandleFromSelectionIndex(size_t index) const
{
    return(_selectedObjectHandles[index]);
}

void _CSceneObjectContainer_::synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg)
{ // Overridden from _CSyncObject_
    if ( (routing[0].objType>=sim_syncobj_sceneobjectstart)&&(routing[0].objType<=sim_syncobj_sceneobjectend) )
    {
        if (routing[0].objHandle!=-1)
        { // Msg is for an object
            CSceneObject* obj=getObjectFromHandle(routing[0].objHandle);
            if (obj==nullptr)
            {
                if (routing.size()==1) // check also size (some msgs have same ids in different scopes)
                {
                    if (msg.msg==sim_syncobj_sceneobject_create)
                    {
                        int handle=routing[0].objHandle;
                        if (App::currentWorld->sceneObjects->getObjectFromHandle(handle)==nullptr)
                        {
                            if (routing[0].objType==sim_syncobj_joint)
                                obj=new CJoint(((int*)msg.data)[0]);
                            if (routing[0].objType==sim_syncobj_dummy)
                                obj=new CDummy();
                            obj->setObjectHandle(handle);
                            _addObject(obj);
                        }
                        else
                        {
#ifndef SIM_LIB
                            simAddLog("Sync",sim_verbosity_errors,"trying to create a scene object with a handle that already exists.");
#endif
                        }
                    }
                }
            }
            else
            {
                if ( (msg.msg==sim_syncobj_sceneobject_delete)&&(routing.size()==1) ) // check also size (some msgs have same ids in different scopes)
                {
                    if (!_removeObject(routing[0].objHandle))
                    {
#ifndef SIM_LIB
                        simAddLog("Sync",sim_verbosity_errors,"trying to remove an inexistent object.");
#endif
                    }
                }
                else
                {
                    routing.erase(routing.begin());
                    obj->synchronizationMsg(routing,msg);
                }
            }
        }
        else
        { // Msg is for this object container
            if (msg.msg==sim_syncobj_sceneobjectcont_selection)
            {
                std::vector<int> v(((int*)msg.data),((int*)msg.data)+msg.dataSize);
                setSelectedObjectHandles(&v);
                return;
            }
        }
    }
}
