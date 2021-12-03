#include "_sceneObject_.h"
#include "sceneObject.h"
#include "app.h"

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
    _objectUid=App::getFreshUniqueId();
}

long long int _CSceneObject_::getObjectUid() const
{
    return(_objectUid);
}

bool _CSceneObject_::setParent(CSceneObject* parent)
{
    bool diff=(_parentObject!=parent);
    if (diff)
    {
        _parentObject=parent;
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            const char* cmd="parent";
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,true,cmd,true);
            long long int pUid=-1;
            if (_parentObject!=nullptr)
                pUid=_parentObject->getObjectUid();
            data->appendMapObject_stringInt64(cmd,pUid);
            App::worldContainer->pushEvent(event);
        }
        if (getObjectCanSync())
        {
            int h=-1;
            if (parent!=nullptr)
                h=parent->getObjectHandle();
            _setParent_send(h);
        }
    }
    return(diff);
}

void _CSceneObject_::_setParent_send(int parentHandle) const
{
}

bool _CSceneObject_::setExtensionString(const char* str)
{
    bool diff=(_extensionString!=str);
    if (diff)
    {
        _extensionString=str;
        if (getObjectCanSync())
            _setExtensionString_send(str);
    }
    return(diff);
}

void _CSceneObject_::_setExtensionString_send(const char* str) const
{
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
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            const char* cmd="layer";
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,true,cmd,true);
            data->appendMapObject_stringInt32(cmd,l);
            App::worldContainer->pushEvent(event);
        }
    }
    return(diff);
}

bool _CSceneObject_::setChildOrder(int order)
{
    bool diff=(_childOrder!=order);
    if (diff)
    {
        _childOrder=order;
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            const char* cmd="childOrder";
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,true,cmd,true);
            data->appendMapObject_stringInt32(cmd,order);
            App::worldContainer->pushEvent(event);
        }
    }
    return(diff);
}

void _CSceneObject_::_setVisibilityLayer_send(unsigned short l) const
{
}

void _CSceneObject_::_setChildOrder_send(int order) const
{
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

void _CSceneObject_::_setModelInvisible(bool inv)
{
    bool diff=(_modelInvisible!=inv);
    if (diff)
    {
        _modelInvisible=inv;
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            const char* cmd="modelInvisible";
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,true,cmd,true);
            data->appendMapObject_stringBool(cmd,inv);
            App::worldContainer->pushEvent(event);
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
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            const char* cmd="alias";
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,true,cmd,true);
            data->appendMapObject_stringString(cmd,newName,0);
            App::worldContainer->pushEvent(event);
        }
    }
    return(diff);
}

bool _CSceneObject_::setObjectName_direct_old(const char* newName)
{
    bool diff=(_objectName_old!=newName);
    if (diff)
    {
        _objectName_old=newName;
        if (getObjectCanSync())
            _setObjectName_send(newName);
    }
    return(diff);
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
        _objectAltName_old=newAltName;
        if (getObjectCanSync())
            _setObjectAltName_send(newAltName);
    }
    return(diff);
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

void _CSceneObject_::recomputeModelInfluencedValues(int overrideFlags/*=-1*/)
{
    if (overrideFlags==-1)
    {
        if (_parentObject==nullptr)
        {
            if (_modelBase)
                overrideFlags=_modelProperty;
            else
                overrideFlags=0;
        }
        else
        {
            _parentObject->recomputeModelInfluencedValues(-2);
            return;
        }
    }
    if (overrideFlags!=-2)
    {
        if (_modelBase)
            overrideFlags|=_modelProperty;
        _calculatedModelProperty=overrideFlags;
        _setModelInvisible((_calculatedModelProperty&sim_modelproperty_not_visible)!=0);

        _calculatedObjectProperty=_objectProperty;
        if ((_calculatedModelProperty&sim_modelproperty_not_showasinsidemodel)!=0)
            _calculatedObjectProperty|=sim_objectproperty_dontshowasinsidemodel;
    }

    for (size_t i=0;i<_childList.size();i++)
        _childList[i]->recomputeModelInfluencedValues(_calculatedModelProperty);
}

void _CSceneObject_::_setObjectAlias_send(const char* newName) const
{
}

void _CSceneObject_::_setObjectName_send(const char* newName) const
{
}

void _CSceneObject_::_setObjectAltName_send(const char* newAltName) const
{
}

bool _CSceneObject_::setLocalTransformation(const C7Vector& tr)
{
    bool diff=(_localTransformation!=tr);
    if (diff)
    {
        _localTransformation=tr;
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            const char* cmd="pose";
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,true,cmd,true);
            float p[7]={tr.X(0),tr.X(1),tr.X(2),tr.Q(1),tr.Q(2),tr.Q(3),tr.Q(0)};
            data->appendMapObject_stringFloatArray(cmd,p,7);
            App::worldContainer->pushEvent(event);
        }
        if (getObjectCanSync())
            _setLocalTransformation_send(_localTransformation);
    }
    return(diff);
}

bool _CSceneObject_::setLocalTransformation(const C4Vector& q)
{
    bool diff=(_localTransformation.Q!=q);
    if (diff)
    {
        _localTransformation.Q=q;
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            const char* cmd="pose";
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,true,cmd,true);
            float p[7]={_localTransformation.X(0),_localTransformation.X(1),_localTransformation.X(2),_localTransformation.Q(1),_localTransformation.Q(2),_localTransformation.Q(3),_localTransformation.Q(0)};
            data->appendMapObject_stringFloatArray(cmd,p,7);
            App::worldContainer->pushEvent(event);
        }
        if (getObjectCanSync())
        {
            C7Vector tr(_localTransformation);
            tr.Q=q;
            _setLocalTransformation_send(tr);
        }
    }
    return(diff);
}

bool _CSceneObject_::setLocalTransformation(const C3Vector& x)
{
    bool diff=(_localTransformation.X!=x);
    if (diff)
    {
        _localTransformation.X=x;
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            const char* cmd="pose";
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,true,cmd,true);
            float p[7]={_localTransformation.X(0),_localTransformation.X(1),_localTransformation.X(2),_localTransformation.Q(1),_localTransformation.Q(2),_localTransformation.Q(3),_localTransformation.Q(0)};
            data->appendMapObject_stringFloatArray(cmd,p,7);
            App::worldContainer->pushEvent(event);
        }
        if (getObjectCanSync())
        {
            C7Vector tr(_localTransformation);
            tr.X=x;
            _setLocalTransformation_send(tr);
        }
    }
    return(diff);
}

void _CSceneObject_::_setLocalTransformation_send(const C7Vector& tr) const
{
}

void _CSceneObject_::synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg)
{ // Overridden from _CSyncObject_
}

