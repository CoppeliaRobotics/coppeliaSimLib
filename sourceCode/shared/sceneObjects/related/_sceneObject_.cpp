#include "_sceneObject_.h"
#include "sceneObject.h"
#include "app.h"

_CSceneObject_::_CSceneObject_()
{
    _selected=false;
    _parentObject=nullptr;
    _localTransformation.setIdentity();
}

_CSceneObject_::~_CSceneObject_()
{
}

std::string _CSceneObject_::getExtensionString() const
{
    return(_extensionString);
}

bool _CSceneObject_::setParent(CSceneObject* parent,bool keepObjectInPlace)
{
    bool diff=(_parentObject!=parent);
    if (diff)
    {
        if (getObjectCanChange())
            _parentObject=parent;
        if (getObjectCanSync())
        {
            int h=-1;
            if (parent!=nullptr)
                h=parent->getObjectHandle();
            _setParent_send(h,keepObjectInPlace);
        }
    }
    return(diff&&getObjectCanChange());
}

void _CSceneObject_::_setParent_send(int parentHandle,bool keepObjectInPlace) const
{
    void* data[2];
    data[0]=&parentHandle;
    data[1]=&keepObjectInPlace;
    sendRandom(data,2,sim_syncobj_sceneobject_setparent);
}

bool _CSceneObject_::setExtensionString(const char* str)
{
    bool diff=(_extensionString!=str);
    if (diff)
    {
        if (getObjectCanChange())
            _extensionString=str;
        if (getObjectCanSync())
            _setExtensionString_send(str);
    }
    return(diff&&getObjectCanChange());
}

void _CSceneObject_::_setExtensionString_send(const char* str) const
{
    sendString(str,sim_syncobj_sceneobject_setextensionstring);
}

unsigned short _CSceneObject_::getVisibilityLayer() const
{
    return(_visibilityLayer);
}

bool _CSceneObject_::setVisibilityLayer(unsigned short l)
{
    bool diff=(_visibilityLayer!=l);
    if (diff)
    {
        if (getObjectCanChange())
            _visibilityLayer=l;
        if (getObjectCanSync())
            _setVisibilityLayer_send(l);
    }
    return(diff&&getObjectCanChange());
}

void _CSceneObject_::_setVisibilityLayer_send(unsigned short l) const
{
    sendUInt16(l,sim_syncobj_sceneobject_setvisibilitylayer);
}

bool _CSceneObject_::getSelected() const
{
    return(_selected);
}

void _CSceneObject_::setSelected(bool s)
{
    _selected=s;
}

CSceneObject* _CSceneObject_::getParent() const
{
    return(_parentObject);
}

int _CSceneObject_::getObjectHandle() const
{
    return(_objectHandle);
}

bool _CSceneObject_::setObjectHandle(int newObjectHandle)
{
    bool diff=(_objectHandle!=newObjectHandle);
    _objectHandle=newObjectHandle;
    return(diff);
}

std::string _CSceneObject_::getObjectName() const
{
    return(_objectName);
}

bool _CSceneObject_::setObjectName(const char* newName,bool check)
{
    bool diff=(_objectName!=newName);
    if (diff)
    {
        if (getObjectCanChange())
            _objectName=newName;
        if (getObjectCanSync())
            _setObjectName_send(newName);
    }
    return(diff&&getObjectCanChange());
}

std::string _CSceneObject_::getObjectAltName() const
{
    return(_objectAltName);
}

C7Vector _CSceneObject_::getLocalTransformation() const
{
    return(_localTransformation);
}

C7Vector _CSceneObject_::getFullLocalTransformation() const
{
    return(_localTransformation);
}

C7Vector _CSceneObject_::getFullParentCumulativeTransformation() const
{
    C7Vector retVal;
    if (_parentObject==nullptr)
        retVal.setIdentity();
    else
        retVal=_parentObject->getFullCumulativeTransformation();
    return(retVal);
}

C7Vector _CSceneObject_::getCumulativeTransformation() const
{
    C7Vector retVal;
    if (_parentObject==nullptr)
        retVal=getLocalTransformation();
    else
        retVal=getFullParentCumulativeTransformation()*getLocalTransformation();
    return(retVal);
}

C7Vector _CSceneObject_::getFullCumulativeTransformation() const
{
    return(getFullParentCumulativeTransformation()*getFullLocalTransformation());
}

bool _CSceneObject_::setObjectAltName(const char* newAltName,bool check)
{
    bool diff=(_objectAltName!=newAltName);
    if (diff)
    {
        if (getObjectCanChange())
            _objectAltName=newAltName;
        if (getObjectCanSync())
            _setObjectAltName_send(newAltName);
    }
    return(diff&&getObjectCanChange());
}

void _CSceneObject_::_setObjectName_send(const char* newName) const
{
    sendString(newName,sim_syncobj_sceneobject_setname);
}

void _CSceneObject_::_setObjectAltName_send(const char* newAltName) const
{
    sendString(newAltName,sim_syncobj_sceneobject_setaltname);
}

bool _CSceneObject_::setLocalTransformation(const C7Vector& tr)
{
    bool diff=(_localTransformation!=tr);
    if (diff)
    {
        if (getObjectCanChange())
            _localTransformation=tr;
        if (getObjectCanSync())
            _setLocalTransformation_send(tr);
    }
    return(diff&&getObjectCanChange());
}

bool _CSceneObject_::setLocalTransformation(const C4Vector& q)
{
    bool diff=(_localTransformation.Q!=q);
    if (diff)
    {
        if (getObjectCanChange())
            _localTransformation.Q=q;
        if (getObjectCanSync())
        {
            C7Vector tr(_localTransformation);
            tr.Q=q;
            _setLocalTransformation_send(tr);
        }
    }
    return(diff&&getObjectCanChange());
}

bool _CSceneObject_::setLocalTransformation(const C3Vector& x)
{
    bool diff=(_localTransformation.X!=x);
    if (diff)
    {
        if (getObjectCanChange())
            _localTransformation.X=x;
        if (getObjectCanSync())
        {
            C7Vector tr(_localTransformation);
            tr.X=x;
            _setLocalTransformation_send(tr);
        }
    }
    return(diff&&getObjectCanChange());
}

void _CSceneObject_::_setLocalTransformation_send(const C7Vector& tr) const
{
    sendTransformation(&tr,sim_syncobj_sceneobject_localtransf);
}

void _CSceneObject_::synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg)
{ // Overridden from _CSyncObject_
    if (routing.size()>0)
    {
    }
    else
    { // message is for this object
        if (msg.msg==sim_syncobj_sceneobject_setvisibilitylayer)
        {
            setVisibilityLayer(((int*)msg.data)[0]);
            return;
        }
        if (msg.msg==sim_syncobj_sceneobject_localtransf)
        {
            C7Vector tr;
            tr.setInternalData((float*)msg.data);
            setLocalTransformation(tr);
            return;
        }
        if (msg.msg==sim_syncobj_sceneobject_setextensionstring)
        {
            setExtensionString(((char*)msg.data));
            return;
        }
        if (msg.msg==sim_syncobj_sceneobject_setname)
        {
            setObjectName(((char*)msg.data),true);
            return;
        }
        if (msg.msg==sim_syncobj_sceneobject_setaltname)
        {
            setObjectAltName(((char*)msg.data),true);
            return;
        }
        if (msg.msg==sim_syncobj_sceneobject_setparent)
        {
            int h=((int*)msg.data)[0];
            bool keepInPlace=((bool*)msg.data)[1];
            CSceneObject* parent=App::currentWorld->sceneObjects->getObjectFromHandle(h);
            setParent(parent,keepInPlace);
            return;
        }
    }
}

