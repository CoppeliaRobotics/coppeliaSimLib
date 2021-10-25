#include "_sceneObject_.h"
#include "sceneObject.h"
#include "app.h"

int _CSceneObject_::_objectUniqueIDCounter=0;

_CSceneObject_::_CSceneObject_()
{
    _selected=false;
    _isInScene=false;
    _modelInvisible=false;
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

void _CSceneObject_::setObjectUniqueId()
{
    _objectUniqueId=_objectUniqueIDCounter++;
}

int _CSceneObject_::getObjectUniqueId() const
{
    return(_objectUniqueId);
}

bool _CSceneObject_::setParent(CSceneObject* parent)
{
    bool diff=(_parentObject!=parent);
    if (diff)
    {
        _parentObject=parent;
        if (_isInScene)
        {
            const char* cmd="parent";
            CInterfaceStackTable* event=App::worldContainer->createEvent(EVENTTYPE_OBJECTCHANGED,cmd,this,true);
            int pHandle=-1;
            int pUid=-1;
            if (_parentObject!=nullptr)
            {
                pHandle=_parentObject->getObjectHandle();
                pUid=_parentObject->getObjectUniqueId();
            }
            CInterfaceStackTable* data=new CInterfaceStackTable();
            event->appendMapObject_stringObject(cmd,data);
            data->appendMapObject_stringInt32("handle",pHandle);
            data->appendMapObject_stringInt32("uid",pUid);
            App::worldContainer->pushEvent();
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
        _visibilityLayer=l;
        if (_isInScene)
        {
            const char* cmd="layer";
            CInterfaceStackTable* event=App::worldContainer->createEvent(EVENTTYPE_OBJECTCHANGED,cmd,this,true);
            event->appendMapObject_stringInt32(cmd,l);
            App::worldContainer->pushEvent();
        }
    }
    return(diff&&getObjectCanChange());
}

bool _CSceneObject_::setChildOrder(int order)
{
    bool diff=(_childOrder!=order);
    if (diff)
    {
        _childOrder=order;
        if (_isInScene)
        {
            const char* cmd="childOrder";
            CInterfaceStackTable* event=App::worldContainer->createEvent(EVENTTYPE_OBJECTCHANGED,cmd,this,true);
            event->appendMapObject_stringInt32(cmd,order);
            App::worldContainer->pushEvent();
        }
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

bool _CSceneObject_::getIsInScene() const
{
    return(_isInScene);
}

bool _CSceneObject_::getModelBase() const
{
    return(_modelBase);
}

bool _CSceneObject_::getModelInvisible() const
{
    return(_modelInvisible);
}

void _CSceneObject_::_setModelInvisible(bool inv)
{
    bool diff=(_modelInvisible!=inv);
    if (diff)
    {
        _modelInvisible=inv;
        if (_isInScene)
        {
            const char* cmd="modelInvisible";
            CInterfaceStackTable* event=App::worldContainer->createEvent(EVENTTYPE_OBJECTCHANGED,cmd,this,true);
            event->appendMapObject_stringBool(cmd,inv);
            App::worldContainer->pushEvent();
        }
    }
}

void _CSceneObject_::setSelected(bool s)
{
    _selected=s;
}

void _CSceneObject_::setIsInScene(bool s)
{
    _isInScene=s;
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

std::string _CSceneObject_::getObjectName_old() const
{
    return(_objectName_old);
}

std::string _CSceneObject_::getObjectAlias() const
{
    return(_objectAlias);
}

std::string _CSceneObject_::getObjectAlias_fullPath() const
{
    std::string retVal;
    if (_parentObject==nullptr)
        retVal="/"+getObjectAliasAndOrderIfRequired();
    else
        retVal=_parentObject->getObjectAlias_fullPath()+"/"+getObjectAliasAndOrderIfRequired();
    return(retVal);
}

std::string _CSceneObject_::getObjectAlias_shortPath() const
{
    std::string previousAlias=getObjectAliasAndOrderIfRequired();
    size_t cnt=1;
    std::string retVal("/"+previousAlias);
    if (_parentObject!=nullptr)
    {
        CSceneObject* it=_parentObject;

        CSceneObject* itBeforeSkipping=nullptr;
        std::string retValBeforeSkipping;
        bool previouslySkipped=false;
        bool doNotSkip=false;
        while (it!=nullptr)
        {
            if (cnt>8)
                return(getObjectAlias_fullPath());
            std::string itAlias=it->getObjectAliasAndOrderIfRequired();
            if ( (itAlias==previousAlias)&&previouslySkipped )
            {
                it=itBeforeSkipping;
                retVal=retValBeforeSkipping;
                doNotSkip=true;
                previouslySkipped=false;
            }
            else
            {
                if (it->getParent()==nullptr)
                {
                    if ( it->getModelBase()||doNotSkip||(App::currentWorld->sceneObjects->getObjectFromPath(nullptr,retVal.c_str(),0,nullptr)!=this) )
                    {
                        previousAlias=itAlias;
                        retVal="/"+previousAlias+retVal;
                        cnt++;
                    }
                    else
                    {
                        if (!previouslySkipped)
                        {
                            retValBeforeSkipping=retVal;
                            itBeforeSkipping=it;
                        }
                        previouslySkipped=true;
                    }
                }
                else
                {
                    std::string tmp(".");
                    tmp+=retVal;
                    if ( it->getModelBase()||doNotSkip||(App::currentWorld->sceneObjects->getObjectFromPath(nullptr,tmp.c_str(),0,it->getParent())!=this) )
                    {
                        previousAlias=itAlias;
                        retVal="/"+previousAlias+retVal;
                        cnt++;
                    }
                    else
                    {
                        if (!previouslySkipped)
                        {
                            retValBeforeSkipping=retVal;
                            itBeforeSkipping=it;
                        }
                        previouslySkipped=true;
                    }
                }
                doNotSkip=false;
                it=it->getParent();
            }
        }
    }
    return(retVal);
}

std::string _CSceneObject_::getObjectAlias_printPath() const
{
    std::string retVal=getObjectAlias_shortPath();
    if (retVal.size()>40)
    {
        size_t cnt=0;
        size_t p2;
        for (size_t i=0;i<retVal.size();i++)
        {
            if (retVal[i]=='/')
            {
                cnt++;
                if (cnt==2)
                    p2=i;
                if (cnt>=3)
                    break;
            }
        }
        if (cnt>=3)
        {
            retVal=retVal.substr(0,p2+1)+" ... /";
            retVal+=getObjectAliasAndOrderIfRequired();
        }
    }
    return(retVal);
}

std::string _CSceneObject_::getObjectAliasAndOrderIfRequired() const
{
    std::string retVal(_objectAlias);
    if (_childOrder>=0)
    {
        retVal+="[";
        retVal+=std::to_string(_childOrder);
        retVal+="]";
    }
    return(retVal);
}

std::string _CSceneObject_::getObjectAliasAndHandle() const
{
    std::string retVal(_objectAlias);
    retVal+="-";
    retVal+=std::to_string(_objectHandle);
    return(retVal);
}

bool _CSceneObject_::setObjectAlias_direct(const char* newName)
{
    bool diff=(_objectAlias!=newName);
    if (diff)
    {
        _objectAlias=newName;
        if (_isInScene)
        {
            const char* cmd="alias";
            CInterfaceStackTable* event=App::worldContainer->createEvent(EVENTTYPE_OBJECTCHANGED,cmd,this,true);
            event->appendMapObject_stringString(cmd,newName,0);
            App::worldContainer->pushEvent();
        }
    }
    return(diff&&getObjectCanChange());
}

bool _CSceneObject_::setObjectName_direct_old(const char* newName)
{
    bool diff=(_objectName_old!=newName);
    if (diff)
    {
        if (getObjectCanChange())
            _objectName_old=newName;
        if (getObjectCanSync())
            _setObjectName_send(newName);
    }
    return(diff&&getObjectCanChange());
}

std::string _CSceneObject_::getObjectAltName_old() const
{
    return(_objectAltName_old);
}

bool _CSceneObject_::setObjectAltName_direct_old(const char* newAltName)
{
    bool diff=(_objectAltName_old!=newAltName);
    if (diff)
    {
        if (getObjectCanChange())
            _objectAltName_old=newAltName;
        if (getObjectCanSync())
            _setObjectAltName_send(newAltName);
    }
    return(diff&&getObjectCanChange());
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

void _CSceneObject_::recomputeModelInfluencedValues(int flags/*=-1*/)
{ // -2: parent has correct values, -1: recompute for this object + children
    if (flags==-1)
    {
        if (_parentObject==nullptr)
            flags=_modelProperty;
        else
        {
            _parentObject->recomputeModelInfluencedValues(-2);
            return;
        }
    }
    if (flags==-2)
        flags=_calculatedModelProperty;
    if (_modelBase)
        flags|=_modelProperty;
    _calculatedModelProperty=flags;

    _setModelInvisible((_calculatedModelProperty&sim_modelproperty_not_visible)!=0);

    _calculatedObjectProperty=_objectProperty;
    if ((_calculatedModelProperty&sim_modelproperty_not_showasinsidemodel)!=0)
        _calculatedObjectProperty|=sim_objectproperty_dontshowasinsidemodel;

    for (size_t i=0;i<_childList.size();i++)
        _childList[i]->recomputeModelInfluencedValues(flags);
}

void _CSceneObject_::_setObjectAlias_send(const char* newName) const
{
    sendString(newName,sim_syncobj_sceneobject_setalias);
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
        _localTransformation=tr;
        if (_isInScene)
        {
            const char* cmd="pose";
            CInterfaceStackTable* event=App::worldContainer->createEvent(EVENTTYPE_OBJECTCHANGED,cmd,this,true);
            float p[7]={tr.X(0),tr.X(1),tr.X(2),tr.Q(1),tr.Q(2),tr.Q(3),tr.Q(0)};
            event->appendMapObject_stringFloatArray(cmd,p,7);
            App::worldContainer->pushEvent();
        }
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
        if (msg.msg==sim_syncobj_sceneobject_setalias)
        {
            setObjectAlias_direct(((char*)msg.data));
            return;
        }
        if (msg.msg==sim_syncobj_sceneobject_setname)
        {
            setObjectName_direct_old(((char*)msg.data));
            return;
        }
        if (msg.msg==sim_syncobj_sceneobject_setaltname)
        {
            setObjectAltName_direct_old(((char*)msg.data));
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

