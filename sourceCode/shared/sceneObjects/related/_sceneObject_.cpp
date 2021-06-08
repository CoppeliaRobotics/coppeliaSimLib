#include "_sceneObject_.h"
#include "sceneObject.h"
#include "app.h"

_CSceneObject_::_CSceneObject_()
{
    _selected=false;
    _parentObject=nullptr;
    _childOrder=-1;
    _localTransformation.setIdentity();
}

_CSceneObject_::~_CSceneObject_()
{
}

std::string _CSceneObject_::getExtensionString() const
{
    return(_extensionString);
}

void _CSceneObject_::setParentPtr(CSceneObject* parent)
{
    _parentObject=parent;
}

bool _CSceneObject_::setParent(CSceneObject* parent)
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
            _setParent_send(h);
        }
    }
    return(diff&&getObjectCanChange());
}

void _CSceneObject_::_setParent_send(int parentHandle) const
{
    sendInt32(parentHandle,sim_syncobj_sceneobject_setparent);
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

int _CSceneObject_::getChildOrder() const
{
    return(_childOrder);
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

bool _CSceneObject_::setChildOrder(int order)
{
    bool diff=(_childOrder!=order);
    if (diff)
    {
        if (getObjectCanChange())
            _childOrder=order;
        if (getObjectCanSync())
            _setChildOrder_send(order);
    }
    return(diff&&getObjectCanChange());
}

void _CSceneObject_::_setVisibilityLayer_send(unsigned short l) const
{
    sendUInt16(l,sim_syncobj_sceneobject_setvisibilitylayer);
}

void _CSceneObject_::_setChildOrder_send(int order) const
{
    sendInt32(order,sim_syncobj_sceneobject_setchildorder);
}

bool _CSceneObject_::getSelected() const
{
    return(_selected);
}

bool _CSceneObject_::hiddenInSceneHierarchy() const
{
    return((_localObjectProperty&sim_objectproperty_hierarchyhiddenmodelchild)&&(_parentObject!=nullptr)&&_parentObject->getModelBase());
}

bool _CSceneObject_::getModelBase() const
{
    return(_modelBase);
}

void _CSceneObject_::setSelected(bool s)
{
    _selected=s;
}

int _CSceneObject_::getHierarchyTreeObjects(std::vector<CSceneObject*>& allObjects)
{
    int retVal=1;
    allObjects.push_back((CSceneObject*)this);
    for (size_t i=0;i<_childList.size();i++)
        retVal+=_childList[i]->getHierarchyTreeObjects(allObjects);
    return(retVal);
}

CSceneObject* _CSceneObject_::getParent() const
{
    return(_parentObject);
}

int _CSceneObject_::getObjectType() const
{
    return(_objectType);
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

std::string _CSceneObject_::getObjectHashlessName() const
{
    return(_objectName.substr(0,_objectName.find('#')));
}

std::string _CSceneObject_::getObjectHashlessNameAndOrder() const
{
    std::string retVal(getObjectHashlessName());
    if (_childOrder>=0)
    {
        retVal+="[";
        retVal+=std::to_string(_childOrder);
        retVal+="]";
    }
//    size_t h=_objectName.find('#');
//    if (h!=std::string::npos)
//        retVal+=_objectName.substr(h);
    return(retVal);
}

bool _CSceneObject_::setObjectName_direct(const char* newName)
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

bool _CSceneObject_::setObjectAltName_direct(const char* newAltName)
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
        if (msg.msg==sim_syncobj_sceneobject_setchildorder)
        {
            setChildOrder(((int*)msg.data)[0]);
            return;
        }
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
            setObjectName_direct(((char*)msg.data));
            return;
        }
        if (msg.msg==sim_syncobj_sceneobject_setaltname)
        {
            setObjectAltName_direct(((char*)msg.data));
            return;
        }
        if (msg.msg==sim_syncobj_sceneobject_setparent)
        {
            int h=((int*)msg.data)[0];
            CSceneObject* parent=App::currentWorld->sceneObjects->getObjectFromHandle(h);
            setParent(parent);
            return;
        }
    }
}

