#include "simInternal.h"
#include "tt.h"
#include "shape.h"
#include "camera.h"
#include "graph.h"
#include "path_old.h"
#include "customData_old.h"
#include "visionSensor.h"
#include "mill.h"
#include "light.h"
#include "gV.h"
#include "vDateTime.h"
#include "ttUtil.h"
#include <boost/lexical_cast.hpp>
#include "app.h"
#include "easyLock.h"
#include "jointObject.h"
#include "sceneObject.h"
#include "dummy.h"
#include "global.h"
#include "simFlavor.h"
#include "base64.h"
#include <boost/algorithm/string.hpp>
#include "pluginContainer.h"
#ifdef SIM_WITH_GUI
    #include "oGL.h"
    #include "oglSurface.h"
#endif

CSceneObject::CSceneObject()
{
    _selected=false;
    _isInScene=false;
    _modelInvisible=false;
    _parentObject=nullptr;
    _childOrder=-1;
    _localTransformation.setIdentity();
    _parentObjectHandle_forSerializationOnly=-1;
    _objectHandle=-1;
    _beforeDeleteCallbackSent=false;
    _ikPluginCounterpartHandle=-1;
    _dnaString=CTTUtil::generateUniqueString();
    _assemblingLocalTransformation.setIdentity();
    _assemblingLocalTransformationIsUsed=false;
    _userScriptParameters=nullptr;
    _dynamicsTemporarilyDisabled=false;

    _authorizedViewableObjects=-1; // all
    _assemblyMatchValuesChild.push_back("default");
    _assemblyMatchValuesParent.push_back("default");
    _forceAlwaysVisible_tmp=false;

    _objectProperty=sim_objectproperty_selectable;
    _hierarchyColorIndex=-1;
    _collectionSelfCollisionIndicator=0;
    _ignorePosAndCameraOrthoviewSize_forUndoRedo=false;

    _modelBase=false;
    _dynamicFlag=0;

    _transparentObjectDistanceOffset=0;
    _objectManipulationMode_flaggedForGridOverlay=0;

    _measuredAngularVelocity_velocityMeasurement=0.0f;
    _measuredLinearVelocity_velocityMeasurement.clear();
    _measuredAngularVelocity3_velocityMeasurement.clear();
    _measuredAngularVelocityAxis_velocityMeasurement.clear();
    _previousPositionOrientationIsValid=false;


    _dynamicsTemporarilyDisabled=false;
    _initialValuesInitialized=false;
    _initialConfigurationMemorized=false;
    _objectMovementPreferredAxes=0x023; // about Z and in the X-Y plane!       0x03f; // full
    _objectManipulationModeEventId=-1;

    _objectMovementOptions=0;
    _objectMovementRelativity[0]=0; // relative to world by default
    _objectMovementRelativity[1]=2; // relative to own frame by default
    _objectMovementStepSize[0]=0.0f; // i.e. use default
    _objectMovementStepSize[1]=0.0f; // i.e. use default

    _customObjectData_old=nullptr;
    _localObjectSpecialProperty=0;
    _modelProperty=0; // By default, the main properties are not overriden! (0 means we inherit from parents)

    _dynamicSimulationIconCode=sim_dynamicsimicon_none;

    _uniquePersistentIdString=CTTUtil::generateUniqueReadableString(); // persistent
    _modelAcknowledgement="";
    _objectTempAlias="_*_object_*_";
    _objectTempName_old="_*_object_*_";

    _specificLight=-1; // default, i.e. all lights

    _sizeFactor=1.0f;
    _sizeValues[0]=1.0f;
    _sizeValues[1]=1.0f;
    _sizeValues[2]=1.0f;
    _dynamicsResetFlag=true;
}

CSceneObject::~CSceneObject() 
{
    delete _customObjectData_old;
    delete _userScriptParameters;
}

void CSceneObject::setIgnorePosAndCameraOrthoviewSize_forUndoRedo(bool s)
{
    _ignorePosAndCameraOrthoviewSize_forUndoRedo=s;
}

void CSceneObject::setForceAlwaysVisible_tmp(bool force)
{
    _forceAlwaysVisible_tmp=force;
}

void CSceneObject::measureVelocity(float dt)
{
    C7Vector abs(getCumulativeTransformation());
    if (_previousPositionOrientationIsValid)
    {
        _measuredLinearVelocity_velocityMeasurement=(abs.X-_previousAbsTransf_velocityMeasurement.X)*(1.0f/dt);
        _measuredAngularVelocity_velocityMeasurement=abs.Q.getAngleBetweenQuaternions(_previousAbsTransf_velocityMeasurement.Q)/dt;
        _measuredAngularVelocity3_velocityMeasurement=(_previousAbsTransf_velocityMeasurement.getInverse()*abs).Q.getEulerAngles()*(1.0f/dt);

        C4Vector AA(_previousAbsTransf_velocityMeasurement.Q);
        C4Vector BB(abs.Q);
        if (AA(0)*BB(0)+AA(1)*BB(1)+AA(2)*BB(2)+AA(3)*BB(3)<0.0f)
            AA=AA*-1.0f;
        C4Vector r((AA.getInverse()*BB).getAngleAndAxis());
        _measuredAngularVelocityAxis_velocityMeasurement.set(r(1),r(2),r(3));
        _measuredAngularVelocityAxis_velocityMeasurement=AA*_measuredAngularVelocityAxis_velocityMeasurement;
        _measuredAngularVelocityAxis_velocityMeasurement.normalize();
        _measuredAngularVelocityAxis_velocityMeasurement*=r(0)/dt;
    }
    _previousAbsTransf_velocityMeasurement=abs;
    _previousPositionOrientationIsValid=true;
}

void CSceneObject::setAuthorizedViewableObjects(int objOrCollHandle)
{
    _authorizedViewableObjects=objOrCollHandle;
}

int CSceneObject::getAuthorizedViewableObjects() const
{
    return(_authorizedViewableObjects);
}


void CSceneObject::setTransparentObjectDistanceOffset(float d)
{
    _transparentObjectDistanceOffset=d;
}

float CSceneObject::getTransparentObjectDistanceOffset() const
{
    return(_transparentObjectDistanceOffset);
}


float CSceneObject::getMeasuredAngularVelocity() const
{
    return(_measuredAngularVelocity_velocityMeasurement);
}

C3Vector CSceneObject::getMeasuredLinearVelocity() const
{
    return(_measuredLinearVelocity_velocityMeasurement);
}

C3Vector CSceneObject::getMeasuredAngularVelocity3() const
{
    return(_measuredAngularVelocity3_velocityMeasurement);
}

C3Vector CSceneObject::getMeasuredAngularVelocityAxis() const
{
    return(_measuredAngularVelocityAxis_velocityMeasurement);
}

void CSceneObject::setHierarchyColorIndex(int c)
{
    _hierarchyColorIndex=c;
}

int CSceneObject::getHierarchyColorIndex() const
{
    return(_hierarchyColorIndex);
}

void CSceneObject::setCollectionSelfCollisionIndicator(int c)
{
    _collectionSelfCollisionIndicator=c;
}

int CSceneObject::getCollectionSelfCollisionIndicator() const
{
    return(_collectionSelfCollisionIndicator);
}

void CSceneObject::setDynamicFlag(int flag)
{
    bool diff=(_dynamicFlag!=flag);
    if (diff)
    {
        _dynamicFlag=flag;
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            const char* cmd="dynamicFlag";
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,true,cmd,true);
            data->appendMapObject_stringInt32(cmd,_dynamicFlag);
            App::worldContainer->pushEvent(event);
        }
    }
}

CSceneObject* CSceneObject::getParent() const
{
    return(_parentObject);
}

int CSceneObject::getObjectType() const
{
    return(_objectType);
}

int CSceneObject::getObjectHandle() const
{
    return(_objectHandle);
}

long long int CSceneObject::getObjectUid() const
{
    return(_objectUid);
}

bool CSceneObject::getSelected() const
{
    return(_selected);
}

bool CSceneObject::getIsInScene() const
{
    return(_isInScene);
}

bool CSceneObject::getModelBase() const
{
    return(_modelBase);
}

std::string CSceneObject::getExtensionString() const
{
    return(_extensionString);
}

unsigned short CSceneObject::getVisibilityLayer() const
{
    return(_visibilityLayer);
}

int CSceneObject::getChildOrder() const
{
    return(_childOrder);
}

int CSceneObject::getHierarchyTreeObjects(std::vector<CSceneObject*>& allObjects)
{
    int retVal=1;
    allObjects.push_back((CSceneObject*)this);
    for (size_t i=0;i<_childList.size();i++)
        retVal+=_childList[i]->getHierarchyTreeObjects(allObjects);
    return(retVal);
}

std::string CSceneObject::getObjectName_old() const
{
    return(_objectName_old);
}

std::string CSceneObject::getObjectAltName_old() const
{
    return(_objectAltName_old);
}

std::string CSceneObject::getObjectAlias() const
{
    return(_objectAlias);
}

std::string CSceneObject::getObjectAlias_fullPath() const
{
    std::string retVal;
    if (_parentObject==nullptr)
        retVal="/"+getObjectAliasAndOrderIfRequired();
    else
        retVal=_parentObject->getObjectAlias_fullPath()+"/"+getObjectAliasAndOrderIfRequired();
    return(retVal);
}

std::string CSceneObject::getObjectAlias_shortPath() const
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

std::string CSceneObject::getObjectAlias_printPath() const
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

std::string CSceneObject::getObjectAliasAndOrderIfRequired() const
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

std::string CSceneObject::getObjectAliasAndHandle() const
{
    std::string retVal(_objectAlias);
    retVal+="-";
    retVal+=std::to_string(_objectHandle);
    return(retVal);
}

C7Vector CSceneObject::getLocalTransformation() const
{
    return(_localTransformation);
}

C7Vector CSceneObject::getFullLocalTransformation() const
{
    return(_localTransformation);
}

C7Vector CSceneObject::getFullParentCumulativeTransformation() const
{
    C7Vector retVal;
    if (_parentObject==nullptr)
        retVal.setIdentity();
    else
        retVal=_parentObject->getFullCumulativeTransformation();
    return(retVal);
}

C7Vector CSceneObject::getCumulativeTransformation() const
{
    C7Vector retVal;
    if (_parentObject==nullptr)
        retVal=getLocalTransformation();
    else
        retVal=getFullParentCumulativeTransformation()*getLocalTransformation();
    return(retVal);
}

C7Vector CSceneObject::getFullCumulativeTransformation() const
{
    return(getFullParentCumulativeTransformation()*getFullLocalTransformation());
}

void CSceneObject::recomputeModelInfluencedValues(int overrideFlags/*=-1*/)
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

void CSceneObject::setObjectUniqueId()
{
    _objectUid=App::getFreshUniqueId();
}

void CSceneObject::setSelected(bool s)
{
    _selected=s;
}

void CSceneObject::setIsInScene(bool s)
{
    _isInScene=s;
}

void CSceneObject::setParentPtr(CSceneObject* parent)
{
    _parentObject=parent;
}

void CSceneObject::_setModelInvisible(bool inv)
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

int CSceneObject::getDynamicFlag() const
{
    return(_dynamicFlag);
}

bool CSceneObject::getShouldObjectBeDisplayed(int viewableHandle,int displayAttrib)
{
    if (_forceAlwaysVisible_tmp)
        return(true);

    if (displayAttrib&sim_displayattribute_inertiaonly)
    {
        if (_objectType!=sim_object_shape_type)
            return(false);
        if (((CShape*)this)->getShapeIsDynamicallyStatic())
            return(false);
        if ((getTreeDynamicProperty()&sim_objdynprop_dynamic)==0)
            return(false);
        return(true);
    }

    bool display=false;
    if (((displayAttrib&sim_displayattribute_pickpass)==0)||((_objectProperty&sim_objectproperty_selectinvisible)==0))
    { // ok, no pickpass and select invisible
        if (displayAttrib&sim_displayattribute_dynamiccontentonly)
            display=(_dynamicFlag!=0);
        else
        {
            display=( (!isObjectPartOfInvisibleModel())&&((App::currentWorld->environment->getActiveLayers()&getVisibilityLayer())||(displayAttrib&sim_displayattribute_ignorelayer)) );

            if (display)
            {
                if (_authorizedViewableObjects==-2)
                    display=false;
                else
                {
                    if ( (_authorizedViewableObjects>=0)&&(viewableHandle!=-1) )
                    {
                        if (_authorizedViewableObjects<SIM_IDSTART_COLLECTION)
                            display=(_authorizedViewableObjects==viewableHandle);
                        else
                        {
                            CCollection* gr=App::currentWorld->collections->getObjectFromHandle(_authorizedViewableObjects);
                            if (gr!=nullptr)
                                display=gr->isObjectInCollection(viewableHandle);
                            else
                                display=false; // should normally never happen
                        }
                    }
                }
            }
        }
    }

    return(display);
}

void CSceneObject::setModelAcknowledgement(const char* a)
{
    _modelAcknowledgement=a;
    if (_modelAcknowledgement.length()>3000)
        _modelAcknowledgement.erase(_modelAcknowledgement.begin()+2999,_modelAcknowledgement.end());
}

std::string CSceneObject::getModelAcknowledgement() const
{
    return(_modelAcknowledgement);
}

void CSceneObject::setDynamicsResetFlag(bool reset,bool fullHierarchyTree)
{
    _dynamicsResetFlag=reset;
    if (reset)
    {
        _dynamicFlag=0;
        if (_objectType==sim_object_joint_type)
        {
            CJoint* joint=(CJoint*)this;
            joint->setForceOrTorqueNotValid();
            joint->setIntrinsicTransformationError(C7Vector::identityTransformation);
        }
        if (_objectType==sim_object_forcesensor_type)
        {
            CForceSensor* sensor=(CForceSensor*)this;
            sensor->setForceAndTorqueNotValid();
            sensor->setIntrinsicTransformationError(C7Vector::identityTransformation);
        }
    }
    if (fullHierarchyTree)
    {
        for (size_t i=0;i<getChildCount();i++)
            getChildFromIndex(i)->setDynamicsResetFlag(reset,fullHierarchyTree);
    }
}

bool CSceneObject::getDynamicsResetFlag() const
{
    return(_dynamicsResetFlag);
}


void CSceneObject::setSizeFactor(float f)
{
    f=tt::getLimitedFloat(0.000001f,1000000.0f,f);
    _sizeFactor=f;
}

float CSceneObject::getSizeFactor() const
{
    return(_sizeFactor);
}

void CSceneObject::setSizeValues(const float s[3])
{
    _sizeValues[0]=s[0];
    _sizeValues[1]=s[1];
    _sizeValues[2]=s[2];
}

void CSceneObject::getSizeValues(float s[3]) const
{
    s[0]=_sizeValues[0];
    s[1]=_sizeValues[1];
    s[2]=_sizeValues[2];
}


int CSceneObject::getParentCount() const
{
    if (getParent()==nullptr)
        return(0);
    return(1+getParent()->getParentCount());
}

int CSceneObject::_getAllowedObjectSpecialProperties() const
{
    int retVal=0;
    if (isPotentiallyCollidable())
        retVal|=sim_objectspecialproperty_collidable;
    if (isPotentiallyMeasurable())
        retVal|=sim_objectspecialproperty_measurable;
    if (isPotentiallyDetectable())
        retVal|=sim_objectspecialproperty_detectable;
    if (isPotentiallyRenderable())
        retVal|=sim_objectspecialproperty_renderable;
    return(retVal);
}

std::string CSceneObject::getObjectTypeInfo() const
{
    return("");
}
std::string CSceneObject::getObjectTypeInfoExtended() const
{
    return("");
}

bool CSceneObject::isPotentiallyCollidable() const
{
    return(false);
}

bool CSceneObject::isPotentiallyMeasurable() const
{
    return(false);
}

bool CSceneObject::isPotentiallyDetectable() const
{
    return(false);
}

bool CSceneObject::isPotentiallyRenderable() const
{
    return(false);
}


void CSceneObject::setModelBase(bool m)
{ // is also called from the ungroup/divide shape routines!!
    bool diff=(_modelBase!=m);
    if (diff)
    {
        _modelBase=m;
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            const char* cmd="modelBase";
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,true,cmd,true);
            data->appendMapObject_stringBool(cmd,m);
            App::worldContainer->pushEvent(event);
        }
       _modelProperty=0; // Nothing is overridden!
        _modelAcknowledgement="";
        recomputeModelInfluencedValues();
    }
}

void CSceneObject::setObjectProperty(int p)
{
    bool diff=(_objectProperty!=p);
    if (diff)
    {
        _objectProperty=p;
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            const char* cmd="objectProperty";
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,true,cmd,true);
            data->appendMapObject_stringInt32(cmd,_objectProperty);
            App::worldContainer->pushEvent(event);
        }
        recomputeModelInfluencedValues();
    }
}

int CSceneObject::getObjectProperty() const
{
    return(_objectProperty);
}

int CSceneObject::getCumulativeObjectProperty()
{
    return(_calculatedObjectProperty);
    /*
    int retVal=_objectProperty;
    int o=getCumulativeModelProperty();
    if (o&sim_modelproperty_not_showasinsidemodel!=0)
        retVal|=sim_objectproperty_dontshowasinsidemodel;
    return(retVal);
    */
}

void CSceneObject::setLocalObjectSpecialProperty(int prop)
{ // sets the local value
    _localObjectSpecialProperty=prop;
    int allowed=_getAllowedObjectSpecialProperties();
    _localObjectSpecialProperty&=allowed;
}

int CSceneObject::getLocalObjectSpecialProperty() const
{ // returns the local value
    return(_localObjectSpecialProperty);
}

int CSceneObject::getCumulativeObjectSpecialProperty()
{ // returns the cumulative value
    int o=getCumulativeModelProperty();
    if (o==0)
        return(_localObjectSpecialProperty); // nothing is overridden!
    int p=_localObjectSpecialProperty;

    if (o&sim_modelproperty_not_collidable)
        p=(p|sim_objectspecialproperty_collidable)-sim_objectspecialproperty_collidable;
    if (o&sim_modelproperty_not_measurable)
        p=(p|sim_objectspecialproperty_measurable)-sim_objectspecialproperty_measurable;
    if (o&sim_modelproperty_not_renderable)
        p=(p|sim_objectspecialproperty_renderable)-sim_objectspecialproperty_renderable;
    if (o&sim_modelproperty_not_detectable)
        p=(p|sim_objectspecialproperty_detectable)-sim_objectspecialproperty_detectable;

    return(p);
}


void CSceneObject::setModelProperty(int prop)
{ // model properties are actually override properties. This func. returns the local value
    if (_modelProperty!=prop)
    {
        _modelProperty=prop;
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            const char* cmd="modelProperty";
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,true,cmd,true);
            data->appendMapObject_stringInt32(cmd,_modelProperty);
            App::worldContainer->pushEvent(event);
        }
        recomputeModelInfluencedValues();
    }
}

int CSceneObject::getModelProperty() const
{ // model properties are actually override properties. This func. returns the local value
    return(_modelProperty);
}

int CSceneObject::getCumulativeModelProperty()
{ // model properties are actually override properties. This func. returns the cumulative value
    return(_calculatedModelProperty);
}

bool CSceneObject::isObjectVisible()
{
    return( isObjectInVisibleLayer()&&(!isObjectPartOfInvisibleModel()) );
}

bool CSceneObject::isObjectInVisibleLayer()
{
    return( (int(App::currentWorld->environment->getActiveLayers())&_visibilityLayer)!=0 );
}

bool CSceneObject::isObjectPartOfInvisibleModel()
{
    return((getCumulativeModelProperty()&sim_modelproperty_not_visible)!=0);
}

int CSceneObject::getTreeDynamicProperty() // combination of sim_objdynprop_dynamic and sim_objdynprop_respondable
{
    int retVal=0;
    if (!_dynamicsTemporarilyDisabled)
    {
        int o=getCumulativeModelProperty();
        if ((o&sim_modelproperty_not_dynamic)==0)
            retVal|=sim_objdynprop_dynamic;
        if ((o&sim_modelproperty_not_respondable)==0)
            retVal|=sim_objdynprop_respondable;
    }
    return(retVal);
}

int CSceneObject::getModelSelectionHandle(bool firstObject)
{ // firstObject is true by default
#ifdef SIM_WITH_GUI
    if (CSimFlavor::getBoolVal(9))
    {
        if ( (App::mainWindow!=nullptr)&&(App::mainWindow->getKeyDownState()&1)&&(App::mainWindow->getKeyDownState()&2) )
            return(getObjectHandle());
    }
#endif

    if (_modelBase)
    {
        if ( ((_objectProperty&sim_objectproperty_selectmodelbaseinstead)==0) )
            return(getObjectHandle());
        if (getParent()==nullptr)
            return(getObjectHandle());
        int retV=getParent()->getModelSelectionHandle(false);
        if (retV==-1)
            retV=getObjectHandle();
        return(retV);
    }
    else
    {
        if (getParent()==nullptr)
        {
            if (firstObject)
                return(getObjectHandle());
            return(-1);
        }
        int retV=getParent()->getModelSelectionHandle(false);
        if (retV!=-1)
            return(retV);
        if (firstObject)
            return(getObjectHandle());
        return(-1);
    }
}

void CSceneObject::clearManipulationModeOverlayGridFlag()
{
    _objectManipulationMode_flaggedForGridOverlay=0;
}

void CSceneObject::scaleObject(float scalingFactor)
{
    _sizeFactor*=scalingFactor;
    _sizeValues[0]*=scalingFactor;
    _sizeValues[1]*=scalingFactor;
    _sizeValues[2]*=scalingFactor;
    computeBoundingBox();

    App::currentWorld->drawingCont->adjustForScaling(_objectHandle,scalingFactor,scalingFactor,scalingFactor);
    App::worldContainer->setModificationFlag(256); // object scaled
}

void CSceneObject::scaleObjectNonIsometrically(float x,float y,float z)
{
    _sizeFactor*=cbrt(x*y*z);
    _sizeValues[0]*=x;
    _sizeValues[1]*=y;
    _sizeValues[2]*=z;
    computeBoundingBox();
    pushObjectRefreshEvent();
    App::currentWorld->drawingCont->adjustForScaling(_objectHandle,x,y,z);
    App::worldContainer->setModificationFlag(256); // object scaled
}

void CSceneObject::scalePosition(float scalingFactor) 
{ // This routine will scale an object's position. The object itself keeps the same size.
    C7Vector local(getLocalTransformation());
    setLocalTransformation(local.X*scalingFactor);
    _assemblingLocalTransformation.X=_assemblingLocalTransformation.X*scalingFactor;
    _dynamicsResetFlag=true;
}

void CSceneObject::setSpecificLight(int h)
{
    _specificLight=h;
}

int CSceneObject::getSpecificLight() const
{
    return(_specificLight);
}

bool CSceneObject::setBeforeDeleteCallbackSent()
{
    bool retVal=!_beforeDeleteCallbackSent;
    _beforeDeleteCallbackSent=true;
    return(retVal);
}

bool CSceneObject::getGlobalMarkingBoundingBox(const C7Vector& baseCoordInv,C3Vector& min,C3Vector& max,bool& minMaxNotYetDefined,bool first,bool guiIsRendering)
{ // For root selection display! Return value false means there is no global marking bounding box and min/max values are not valid
    bool retVal=false;
    int objProp=getObjectProperty();
    int modProp=getModelProperty();

    bool exploreChildren=((modProp&sim_modelproperty_not_showasinsidemodel)==0)||first;
    bool includeThisBox=(objProp&sim_objectproperty_dontshowasinsidemodel)==0;
    first=false;

    if (includeThisBox&&exploreChildren)
    {
        C3Vector smi,sma;
        getBoundingBox(smi,sma);
        retVal=true;
        C3Vector sm;
        C7Vector ctm;
        ctm=getCumulativeTransformation();
        ctm=baseCoordInv*ctm;
        for (int i=0;i<2;i++)
        {
            sm(0)=smi(0)*i+sma(0)*(1-i);
            for (int j=0;j<2;j++)
            {
                sm(1)=smi(1)*j+sma(1)*(1-j);
                for (int k=0;k<2;k++)
                {
                    sm(2)=smi(2)*k+sma(2)*(1-k);
                    if (minMaxNotYetDefined)
                    {
                        max=ctm*sm;
                        min=max;
                        minMaxNotYetDefined=false;
                    }
                    else
                    {
                        max.keepMax(ctm*sm);
                        min.keepMin(ctm*sm);
                    }
                }
            }
        }
    }

    if (exploreChildren)
    {
        for (size_t i=0;i<getChildCount();i++)
        {
            if (getChildFromIndex(i)->getGlobalMarkingBoundingBox(baseCoordInv,min,max,minMaxNotYetDefined,first,guiIsRendering))
                retVal=true;
        }
    }
    return(retVal);
}

void CSceneObject::getBoundingBoxEncompassingBoundingBox(const C7Vector& baseCoordInv,C3Vector& min,C3Vector& max,bool guiIsRendering)
{
    C3Vector smi,sma;
    getBoundingBox(smi,sma);
    bool minMaxNotYetDefined=true;
    C3Vector sm;
    C7Vector ctm;
    ctm=getCumulativeTransformation();
    ctm=baseCoordInv*ctm;
    for (int i=0;i<2;i++)
    {
        sm(0)=smi(0)*i+sma(0)*(1-i);
        for (int j=0;j<2;j++)
        {
            sm(1)=smi(1)*j+sma(1)*(1-j);
            for (int k=0;k<2;k++)
            {
                sm(2)=smi(2)*k+sma(2)*(1-k);
                if (minMaxNotYetDefined)
                {
                    max=ctm*sm;
                    min=max;
                    minMaxNotYetDefined=false;
                }
                else
                {
                    max.keepMax(ctm*sm);
                    min.keepMin(ctm*sm);
                }
            }
        }
    }
}

void CSceneObject::performGcsLoadingMapping(const std::vector<int>* map)
{
}

void CSceneObject::announceCollectionWillBeErased(int collectionID,bool copyBuffer)
{
    if (_authorizedViewableObjects==collectionID)
        _authorizedViewableObjects=-2; // not visible anymore!
    // This routine can be called for sceneObjects-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'ct::sceneObjects->getObject(id)'-call or similar
    for (size_t i=0;i<_customReferencedHandles.size();i++)
    {
        if (_customReferencedHandles[i].generalObjectType==sim_appobj_collection_type)
        {
            if (_customReferencedHandles[i].generalObjectHandle==collectionID)
                _customReferencedHandles[i].generalObjectHandle=-1;
        }
    }
    if (!copyBuffer)
    {
        for (size_t i=0;i<_customReferencedOriginalHandles.size();i++)
        {
            if (_customReferencedOriginalHandles[i].generalObjectType==sim_appobj_collection_type)
            {
                if (_customReferencedOriginalHandles[i].generalObjectHandle==collectionID)
                    _customReferencedOriginalHandles[i].generalObjectHandle=-1;
            }
        }
    }
}

void CSceneObject::announceCollisionWillBeErased(int collisionID,bool copyBuffer)
{
    // This routine can be called for sceneObjects-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'ct::sceneObjects->getObject(id)'-call or similar
    for (size_t i=0;i<_customReferencedHandles.size();i++)
    {
        if (_customReferencedHandles[i].generalObjectType==sim_appobj_collision_type)
        {
            if (_customReferencedHandles[i].generalObjectHandle==collisionID)
                _customReferencedHandles[i].generalObjectHandle=-1;
        }
    }
    if (!copyBuffer)
    {
        for (size_t i=0;i<_customReferencedOriginalHandles.size();i++)
        {
            if (_customReferencedOriginalHandles[i].generalObjectType==sim_appobj_collision_type)
            {
                if (_customReferencedOriginalHandles[i].generalObjectHandle==collisionID)
                    _customReferencedOriginalHandles[i].generalObjectHandle=-1;
            }
        }
    }
}
void CSceneObject::announceDistanceWillBeErased(int distanceID,bool copyBuffer)
{
    // This routine can be called for sceneObjects-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'ct::sceneObjects->getObject(id)'-call or similar
    for (size_t i=0;i<_customReferencedHandles.size();i++)
    {
        if (_customReferencedHandles[i].generalObjectType==sim_appobj_distance_type)
        {
            if (_customReferencedHandles[i].generalObjectHandle==distanceID)
                _customReferencedHandles[i].generalObjectHandle=-1;
        }
    }
    if (!copyBuffer)
    {
        for (size_t i=0;i<_customReferencedOriginalHandles.size();i++)
        {
            if (_customReferencedOriginalHandles[i].generalObjectType==sim_appobj_distance_type)
            {
                if (_customReferencedOriginalHandles[i].generalObjectHandle==distanceID)
                    _customReferencedOriginalHandles[i].generalObjectHandle=-1;
            }
        }
    }
}
void CSceneObject::performIkLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{
    for (size_t i=0;i<_customReferencedHandles.size();i++)
    {
        if (_customReferencedHandles[i].generalObjectType==sim_appobj_ik_type)
            _customReferencedHandles[i].generalObjectHandle=CWorld::getLoadingMapping(map,_customReferencedHandles[i].generalObjectHandle);
    }
    if (!loadingAmodel)
    {
        for (size_t i=0;i<_customReferencedOriginalHandles.size();i++)
        {
            if (_customReferencedOriginalHandles[i].generalObjectType==sim_appobj_ik_type)
                _customReferencedOriginalHandles[i].generalObjectHandle=CWorld::getLoadingMapping(map,_customReferencedOriginalHandles[i].generalObjectHandle);
        }
    }
}

void CSceneObject::performCollectionLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{
    if ( (_authorizedViewableObjects>=0)&&(_authorizedViewableObjects>=SIM_IDSTART_COLLECTION) )
        _authorizedViewableObjects=CWorld::getLoadingMapping(map,_authorizedViewableObjects);
    for (size_t i=0;i<_customReferencedHandles.size();i++)
    {
        if (_customReferencedHandles[i].generalObjectType==sim_appobj_collection_type)
            _customReferencedHandles[i].generalObjectHandle=CWorld::getLoadingMapping(map,_customReferencedOriginalHandles[i].generalObjectHandle);
    }
    if (!loadingAmodel)
    {
        for (size_t i=0;i<_customReferencedOriginalHandles.size();i++)
        {
            if (_customReferencedOriginalHandles[i].generalObjectType==sim_appobj_collection_type)
                _customReferencedOriginalHandles[i].generalObjectHandle=CWorld::getLoadingMapping(map,_customReferencedHandles[i].generalObjectHandle);
        }
    }
}

void CSceneObject::performCollisionLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{
    for (size_t i=0;i<_customReferencedHandles.size();i++)
    {
        if (_customReferencedHandles[i].generalObjectType==sim_appobj_collision_type)
            _customReferencedHandles[i].generalObjectHandle=CWorld::getLoadingMapping(map,_customReferencedHandles[i].generalObjectHandle);
    }
    if (!loadingAmodel)
    {
        for (size_t i=0;i<_customReferencedOriginalHandles.size();i++)
        {
            if (_customReferencedOriginalHandles[i].generalObjectType==sim_appobj_collision_type)
                _customReferencedOriginalHandles[i].generalObjectHandle=CWorld::getLoadingMapping(map,_customReferencedOriginalHandles[i].generalObjectHandle);
        }
    }
}

void CSceneObject::performDistanceLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{
    for (size_t i=0;i<_customReferencedHandles.size();i++)
    {
        if (_customReferencedHandles[i].generalObjectType==sim_appobj_distance_type)
            _customReferencedHandles[i].generalObjectHandle=CWorld::getLoadingMapping(map,_customReferencedHandles[i].generalObjectHandle);
    }
    if (!loadingAmodel)
    {
        for (size_t i=0;i<_customReferencedOriginalHandles.size();i++)
        {
            if (_customReferencedOriginalHandles[i].generalObjectType==sim_appobj_distance_type)
                _customReferencedOriginalHandles[i].generalObjectHandle=CWorld::getLoadingMapping(map,_customReferencedOriginalHandles[i].generalObjectHandle);
        }
    }
}

void CSceneObject::performTextureObjectLoadingMapping(const std::vector<int>* map)
{
}

void CSceneObject::performDynMaterialObjectLoadingMapping(const std::vector<int>* map)
{
}

void CSceneObject::getAllObjectsRecursive(std::vector<CSceneObject*>* objectList,bool baseIncluded,bool start) const
{   // baseIncluded and start are true by default.
    // Returns all objects build on this (including children of children of...)
    if (baseIncluded||(!start))
        objectList->push_back((CSceneObject*)this);
    for (size_t i=0;i<getChildCount();i++)
        getChildFromIndex(i)->getAllObjectsRecursive(objectList,true,false);
}

void CSceneObject::getChain(std::vector<CSceneObject*>& objectList,bool tipIncluded,bool start) const
{   // tipIncluded and start are true by default.
    // Returns the chain with this object as tip
    if (tipIncluded||(!start))
        objectList.push_back((CSceneObject*)this);
    if (getParent()!=nullptr)
        getParent()->getChain(objectList,true,false);
}

CSceneObject* CSceneObject::getFirstParentInSelection(const std::vector<CSceneObject*>* sel) const
{
    CSceneObject* it=getParent();
    if (it==nullptr) 
        return(nullptr);
    for (size_t i=0;i<sel->size();i++)
    {
        if (sel->at(i)==it) 
            return(it);
    }
    return(it->getFirstParentInSelection(sel));
}

CSceneObject* CSceneObject::getLastParentInSelection(const std::vector<CSceneObject*>* sel) const
{
    CSceneObject* it=getParent();
    CSceneObject* retVal=nullptr;
    while (it!=nullptr)
    {
        for (size_t i=0;i<sel->size();i++)
        {
            if (sel->at(i)==it)
            {
                retVal=it;
                break;
            }
        }
        it=it->getParent();
    }
    return(retVal);
}

void CSceneObject::removeSceneDependencies()
{
    _customReferencedHandles.clear();
    _customReferencedOriginalHandles.clear();
}

void CSceneObject::addSpecializedObjectEventData(CInterfaceStackTable* data) const
{
}

void CSceneObject::pushObjectCreationEvent() const
{
    if ( _isInScene&&App::worldContainer->getEventsEnabled() )
    {
        auto [event,data]=App::worldContainer->prepareSceneObjectAddEvent(this);
        CSceneObject::_addCommonObjectEventData(data);
        addSpecializedObjectEventData(data);
        App::worldContainer->pushEvent(event);
    }
}

void CSceneObject::pushObjectRefreshEvent() const
{
    if ( _isInScene&&App::worldContainer->getEventsEnabled() )
    {
        auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,true,nullptr,false);
        CSceneObject::_addCommonObjectEventData(data);
        addSpecializedObjectEventData(data);
        App::worldContainer->pushEvent(event);
    }
}

void CSceneObject::_addCommonObjectEventData(CInterfaceStackTable* data) const
{
    data->appendMapObject_stringInt32("layer",_visibilityLayer);
    data->appendMapObject_stringInt32("childOrder",_childOrder);
    float p[7]={_localTransformation.X(0),_localTransformation.X(1),_localTransformation.X(2),_localTransformation.Q(1),_localTransformation.Q(2),_localTransformation.Q(3),_localTransformation.Q(0)};
    data->appendMapObject_stringFloatArray("pose",p,7);
    data->appendMapObject_stringString("alias",_objectAlias.c_str(),0);
    data->appendMapObject_stringString("oldName",_objectName_old.c_str(),0);
    data->appendMapObject_stringBool("modelInvisible",_modelInvisible);
    data->appendMapObject_stringBool("modelBase",_modelBase);
    data->appendMapObject_stringInt32("objectProperty",_objectProperty);
    data->appendMapObject_stringInt32("modelProperty",_modelProperty);
    long long int pUid=-1;
    if (_parentObject!=nullptr)
        pUid=_parentObject->getObjectUid();
    data->appendMapObject_stringInt64("parentUid",pUid);
    CInterfaceStackTable* subC=new CInterfaceStackTable();
    data->appendMapObject_stringObject("boundingBox",subC);
    subC->appendMapObject_stringFloatArray("min",_boundingBoxMin.data,3);
    subC->appendMapObject_stringFloatArray("max",_boundingBoxMax.data,3);
    _appendObjectMovementEventData(data);
    subC=new CInterfaceStackTable();
    data->appendMapObject_stringObject("customData",subC);
    _customObjectData.appendEventData(subC);
    _customObjectData_tempData.appendEventData(subC);
}

void CSceneObject::_appendObjectMovementEventData(CInterfaceStackTable* data) const
{
    data->appendMapObject_stringInt32("movementOptions",_objectMovementOptions);
    data->appendMapObject_stringInt32("movementPreferredAxes",_objectMovementPreferredAxes);
    data->appendMapObject_stringFloatArray("movementStepSize",_objectMovementStepSize,2);
    data->appendMapObject_stringInt32Array("movementRelativity",_objectMovementRelativity,2);
}

CSceneObject* CSceneObject::copyYourself()
{ // This routine should be called in the very first line of function
  // "copyYourself" in every joint, camera, etc. !

    CSceneObject* theNewObject=nullptr;
    if (getObjectType()==sim_object_shape_type)
        theNewObject=new CShape();
    if (getObjectType()==sim_object_octree_type)
        theNewObject=new COctree();
    if (getObjectType()==sim_object_pointcloud_type)
        theNewObject=new CPointCloud();
    if (getObjectType()==sim_object_joint_type)
        theNewObject=new CJoint();
    if (getObjectType()==sim_object_graph_type)
        theNewObject=new CGraph();
    if (getObjectType()==sim_object_dummy_type)
        theNewObject=new CDummy();
    if (getObjectType()==sim_object_proximitysensor_type)
        theNewObject=new CProxSensor();
    if (getObjectType()==sim_object_camera_type)
        theNewObject=new CCamera();
    if (getObjectType()==sim_object_light_type)
        theNewObject=new CLight(((CLight*)this)->getLightType());
    if (getObjectType()==sim_object_path_type)
        theNewObject=new CPath_old();
    if (getObjectType()==sim_object_mirror_type)
        theNewObject=new CMirror();
    if (getObjectType()==sim_object_visionsensor_type)
        theNewObject=new CVisionSensor();
    if (getObjectType()==sim_object_mill_type)
        theNewObject=new CMill();
    if (getObjectType()==sim_object_forcesensor_type)
        theNewObject=new CForceSensor();

    theNewObject->_objectHandle=_objectHandle; // important for copy operations connections
    theNewObject->_authorizedViewableObjects=_authorizedViewableObjects;
    theNewObject->_visibilityLayer=_visibilityLayer;
    theNewObject->_childOrder=_childOrder;
    theNewObject->_localTransformation=_localTransformation;
    theNewObject->_objectAlias=_objectAlias;
    theNewObject->_objectName_old=_objectName_old;
    theNewObject->_objectAltName_old=_objectAltName_old;
    theNewObject->_objectProperty=_objectProperty;
    theNewObject->_hierarchyColorIndex=_hierarchyColorIndex;
    theNewObject->_collectionSelfCollisionIndicator=_collectionSelfCollisionIndicator;
    theNewObject->_modelBase=_modelBase;
    theNewObject->_objectType=_objectType;
    theNewObject->_localObjectSpecialProperty=_localObjectSpecialProperty;
    theNewObject->_modelProperty=_modelProperty;
    theNewObject->_extensionString=_extensionString;

    theNewObject->_dnaString=_dnaString;
    theNewObject->_copyString=_copyString;

    theNewObject->_assemblingLocalTransformation=_assemblingLocalTransformation;
    theNewObject->_assemblingLocalTransformationIsUsed=_assemblingLocalTransformationIsUsed;

    theNewObject->_assemblyMatchValuesChild.assign(_assemblyMatchValuesChild.begin(),_assemblyMatchValuesChild.end());
    theNewObject->_assemblyMatchValuesParent.assign(_assemblyMatchValuesParent.begin(),_assemblyMatchValuesParent.end());
    theNewObject->_objectMovementPreferredAxes=_objectMovementPreferredAxes;
    theNewObject->_objectMovementOptions=_objectMovementOptions;
    theNewObject->_objectMovementRelativity[0]=_objectMovementRelativity[0];
    theNewObject->_objectMovementRelativity[1]=_objectMovementRelativity[1];
    theNewObject->_objectMovementStepSize[0]=_objectMovementStepSize[0];
    theNewObject->_objectMovementStepSize[1]=_objectMovementStepSize[1];

    theNewObject->_boundingBoxMin=_boundingBoxMin;
    theNewObject->_boundingBoxMax=_boundingBoxMax;
    theNewObject->_sizeFactor=_sizeFactor;
    theNewObject->_sizeValues[0]=_sizeValues[0];
    theNewObject->_sizeValues[1]=_sizeValues[1];
    theNewObject->_sizeValues[2]=_sizeValues[2];
    theNewObject->_modelAcknowledgement=_modelAcknowledgement;
    theNewObject->_transparentObjectDistanceOffset=_transparentObjectDistanceOffset;
    _customObjectData.copyYourselfInto(theNewObject->_customObjectData);
    _customObjectData_tempData.copyYourselfInto(theNewObject->_customObjectData_tempData);

    delete theNewObject->_customObjectData_old;
    theNewObject->_customObjectData_old=nullptr;
    if (_customObjectData_old!=nullptr)
        theNewObject->_customObjectData_old=_customObjectData_old->copyYourself();

    delete theNewObject->_userScriptParameters;
    theNewObject->_userScriptParameters=nullptr;
    if (_userScriptParameters!=nullptr)
        theNewObject->_userScriptParameters=_userScriptParameters->copyYourself();

    theNewObject->_customReferencedHandles.assign(_customReferencedHandles.begin(),_customReferencedHandles.end());
    theNewObject->_customReferencedOriginalHandles.assign(_customReferencedOriginalHandles.begin(),_customReferencedOriginalHandles.end());

    // Important:
    theNewObject->_parentObjectHandle_forSerializationOnly=_parentObjectHandle_forSerializationOnly; // and not -1!!! (objects can also be copied in the Copy buffer)
    if (_parentObject!=nullptr)
        theNewObject->_parentObjectHandle_forSerializationOnly=_parentObject->getObjectHandle();

    return(theNewObject);
}

void CSceneObject::clearObjectCustomData_old()
{
    delete _customObjectData_old;
    _customObjectData_old=nullptr;
}


void CSceneObject::writeCustomDataBlock(bool tmpData,const char* dataName,const char* data,size_t dataLength)
{
    bool diff=false;
    if (tmpData)
        diff=_customObjectData_tempData.setData(dataName,data,dataLength);
    else
        diff=_customObjectData.setData(dataName,data,dataLength);

    if ( diff&&_isInScene&&App::worldContainer->getEventsEnabled() )
    {
        const char* cmd="customData";
        auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,true,cmd,false);
        CInterfaceStackTable* subC=new CInterfaceStackTable();
        data->appendMapObject_stringObject(cmd,subC);
        _customObjectData.appendEventData(subC);
        _customObjectData_tempData.appendEventData(subC);
        App::worldContainer->pushEvent(event);
    }
}

std::string CSceneObject::readCustomDataBlock(bool tmpData,const char* dataName) const
{
    std::string retVal;
    if (tmpData)
        retVal=_customObjectData_tempData.getData(dataName);
    else
        retVal=_customObjectData.getData(dataName);
    return(retVal);
}

std::string CSceneObject::getAllCustomDataBlockTags(bool tmpData,size_t* cnt) const
{
    std::string retVal;
    if (tmpData)
        retVal=_customObjectData_tempData.getAllTags(cnt);
    else
        retVal=_customObjectData.getAllTags(cnt);
    return(retVal);
}

void CSceneObject::setObjectCustomData_old(int header,const char* data,int dataLength)
{
    if (_customObjectData_old==nullptr)
        _customObjectData_old=new CCustomData_old();
    _customObjectData_old->setData(header,data,dataLength);
}

int CSceneObject::getObjectCustomDataLength_old(int header) const
{
    if (_customObjectData_old==nullptr)
        return(0);
    return(_customObjectData_old->getDataLength(header));
}

void CSceneObject::getObjectCustomData_old(int header,char* data) const
{
    if (_customObjectData_old==nullptr)
        return;
    _customObjectData_old->getData(header,data);
}

void CSceneObject::setObjectMovementPreferredAxes(int p)
{ // bits 0-2: position x,y,z, bits 3-5: Euler e9,e1,e2
    p&=0x3f;
    bool diff=(_objectMovementPreferredAxes!=p);
    if (diff)
    {
        _objectMovementPreferredAxes=p;
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,true,nullptr,true);
            _appendObjectMovementEventData(data);
            App::worldContainer->pushEvent(event);
        }
    }
}

int CSceneObject::getObjectMovementPreferredAxes() const
{ // bits 0-2: position x,y,z, bits 3-5: Euler e9,e1,e2
    return(_objectMovementPreferredAxes);
}


void CSceneObject::setObjectMovementOptions(int p)
{
    bool diff=(_objectMovementOptions!=p);
    if (diff)
    {
        _objectMovementOptions=p;
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,true,nullptr,true);
            _appendObjectMovementEventData(data);
            App::worldContainer->pushEvent(event);
        }
    }
}

int CSceneObject::getObjectMovementOptions() const
{
    return(_objectMovementOptions);
}

void CSceneObject::setObjectMovementRelativity(int index,int p)
{
    bool diff=(_objectMovementRelativity[index]!=p);
    if (diff)
    {
        _objectMovementRelativity[index]=p;
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,true,nullptr,true);
            _appendObjectMovementEventData(data);
            App::worldContainer->pushEvent(event);
        }
    }
}

int CSceneObject::getObjectMovementRelativity(int index) const
{
    return(_objectMovementRelativity[index]);
}

void CSceneObject::setObjectMovementStepSize(int index,float s)
{
    if (index==0)
    {
        if (s<0.0005)
            s=0.0f; // default
        else
        {
            float sc=1.0f;
            if ((s>=0.0075f)&&(s<0.075f))
                sc=10.0f;
            if (s>=0.075f)
                sc=100.0f;
            if (s<0.0015f*sc)
                s=0.001f*sc;
            else if (s<0.00375f*sc)
            {
                if (sc<2.0f)
                    s=0.002f*sc;
                else
                    s=0.0025f*sc;
            }
            else
                s=0.005f*sc;
        }
    }
    else
    {
        if (s<0.05f*degToRad_f)
            s=0.0f; // default
        else if (s<1.5f*degToRad_f)
            s=1.0f*degToRad_f;
        else if (s<3.5f*degToRad_f)
            s=2.0f*degToRad_f;
        else if (s<7.5f*degToRad_f)
            s=5.0f*degToRad_f;
        else if (s<12.5f*degToRad_f)
            s=10.0f*degToRad_f;
        else if (s<22.5f*degToRad_f)
            s=15.0f*degToRad_f;
        else if (s<37.5f*degToRad_f)
            s=30.0f*degToRad_f;
        else
            s=45.0f*degToRad_f;
    }
    bool diff=(_objectMovementStepSize[index]!=s);
    if (diff)
    {
        _objectMovementStepSize[index]=s;
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,true,nullptr,true);
            _appendObjectMovementEventData(data);
            App::worldContainer->pushEvent(event);
        }
    }
}

float CSceneObject::getObjectMovementStepSize(int index) const
{
    return(_objectMovementStepSize[index]);
}

void CSceneObject::setMechanismID(int id)
{
    _mechanismID=id;
}

int CSceneObject::getMechanismID() const
{ // Make sure setMechanismID was called just before!!
    return (_mechanismID);
}

void CSceneObject::simulationAboutToStart()
{
}

void CSceneObject::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    _dynamicSimulationIconCode=sim_dynamicsimicon_none;
    _initialValuesInitialized=true;
    _modelProperty=(_modelProperty|sim_modelproperty_not_reset)-sim_modelproperty_not_reset;
    setDynamicFlag(0);

    _measuredAngularVelocity_velocityMeasurement=0.0f;
    _measuredAngularVelocity3_velocityMeasurement.clear();
    _measuredAngularVelocityAxis_velocityMeasurement.clear();
    _measuredLinearVelocity_velocityMeasurement.clear();
    _previousPositionOrientationIsValid=false;
    if (_userScriptParameters!=nullptr)
        _userScriptParameters->initializeInitialValues(simulationAlreadyRunning);

    // this section is special and reserved to local configuration restoration!
    //********************************
    _initialConfigurationMemorized=true;
    _initialParentUniqueId=-1; // -1 means there was no parent at begin
    CSceneObject* p=getParent();
    if (p!=nullptr)
        _initialParentUniqueId=p->getObjectUid();
    _initialLocalPose=_localTransformation;
    _initialAbsPose=getCumulativeTransformation();
    //********************************

    _dynamicsTemporarilyDisabled=false;

    _initialMainPropertyOverride=_modelProperty;
}

void CSceneObject::simulationEnded_restoreHierarchy()
{ // called before simulationEnded
    if (_initialValuesInitialized&&_initialConfigurationMemorized)
    {
        if (App::currentWorld->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
        {
            long long int puid=-1;
            CSceneObject* p=getParent();
            if (p!=nullptr)
                puid=p->getObjectUid();
            if (puid!=_initialParentUniqueId)
            {
                CSceneObject* oldParent=App::currentWorld->sceneObjects->getObjectFromUid(_initialParentUniqueId);
                App::currentWorld->sceneObjects->setObjectParent(this,oldParent,true);
            }
        }
    }
}

void CSceneObject::simulationEnded()
{   // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    // called after simulationEnded_restoreHierarchy
    _dynamicSimulationIconCode=sim_dynamicsimicon_none;
    setDynamicFlag(0);
    if (_userScriptParameters!=nullptr)
        _userScriptParameters->simulationEnded();
    if (_initialValuesInitialized)
    {
        if (App::currentWorld->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
        {
            if (_initialConfigurationMemorized)
            { // this section is special and reserved to local configuration restoration!
                long long int puid=-1;
                CSceneObject* p=getParent();
                if (p!=nullptr)
                    puid=p->getObjectUid();
                if (puid!=_initialParentUniqueId)
                    setAbsoluteTransformation(_initialAbsPose);
                else
                    setLocalTransformation(_initialLocalPose);
                _modelProperty=_initialMainPropertyOverride;
                _initialConfigurationMemorized=false;
            }
        }
    }
    _initialValuesInitialized=false;
}

void CSceneObject::computeBoundingBox()
{ // overridden
}

void CSceneObject::_setBoundingBox(const C3Vector& vmin,const C3Vector& vmax)
{
    if ( (_boundingBoxMin!=vmin)||(_boundingBoxMax!=vmax) )
    {
        _boundingBoxMin=vmin;
        _boundingBoxMax=vmax;
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            const char* cmd="boundingBox";
            auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,true,cmd,true);
            CInterfaceStackTable* subC=new CInterfaceStackTable();
            data->appendMapObject_stringObject(cmd,subC);
            subC->appendMapObject_stringFloatArray("min",_boundingBoxMin.data,3);
            subC->appendMapObject_stringFloatArray("max",_boundingBoxMax.data,3);
            App::worldContainer->pushEvent(event);
        }
    }
}

void CSceneObject::getBoundingBox(C3Vector& vmin,C3Vector& vmax) const
{
    vmin=_boundingBoxMin;
    vmax=_boundingBoxMax;
}

void CSceneObject::temporarilyDisableDynamicTree()
{
    _dynamicsTemporarilyDisabled=true;
    for (size_t i=0;i<_childList.size();i++)
        _childList[i]->temporarilyDisableDynamicTree();
}

void CSceneObject::setDynamicSimulationIconCode(int c)
{
    if (c!=_dynamicSimulationIconCode)
        App::setRefreshHierarchyViewFlag();
    _dynamicSimulationIconCode=c;
}

int CSceneObject::getDynamicSimulationIconCode() const
{
    return(_dynamicSimulationIconCode);
}

void CSceneObject::setAssemblingLocalTransformation(const C7Vector& tr)
{
    _assemblingLocalTransformation=tr;
}
C7Vector CSceneObject::getAssemblingLocalTransformation() const
{
    return(_assemblingLocalTransformation);
}
void CSceneObject::setAssemblingLocalTransformationIsUsed(bool u)
{
    _assemblingLocalTransformationIsUsed=u;
}

bool CSceneObject::getAssemblingLocalTransformationIsUsed()
{
    return(_assemblingLocalTransformationIsUsed);
}

void CSceneObject::setAssemblyMatchValues(bool asChild,const char* str)
{
    std::vector<std::string>* v;
    if (asChild)
        v=&_assemblyMatchValuesChild;
    else
        v=&_assemblyMatchValuesParent;
    v->clear();
    std::vector<std::string> words;
    tt::separateWords(str,',',words);
    std::map<std::string,bool> hist;
    for (size_t i=0;i<words.size();i++)
    {
        std::string str(words[i]);
        tt::removeSpacesAtBeginningAndEnd(str);
        tt::removeIllegalCharacters(str,false);
        if (str.size()>0)
        {
            std::map<std::string,bool>::iterator it=hist.find(str);
            if (it==hist.end())
            { // avoid doubles
                hist[str]=true;
                v->push_back(str);
            }
        }
    }
}

std::string CSceneObject::getAssemblyMatchValues(bool asChild) const
{
    const std::vector<std::string>* v;
    if (asChild)
        v=&_assemblyMatchValuesChild;
    else
        v=&_assemblyMatchValuesParent;
    std::string retVal;
    for (size_t i=0;i<v->size();i++)
    {
        if (retVal.size()>0)
            retVal+=",";
        retVal+=v->at(i);
    }
    return(retVal);
}

const std::vector<std::string>* CSceneObject::getChildAssemblyMatchValuesPointer() const
{
    return(&_assemblyMatchValuesChild);
}

bool CSceneObject::doesParentAssemblingMatchValuesMatchWithChild(const std::vector<std::string>* assemblingChildMatchValues,bool ignoreDefaultNames/*=false*/) const
{
    for (size_t i=0;i<_assemblyMatchValuesParent.size();i++)
    {
        for (size_t j=0;j<assemblingChildMatchValues->size();j++)
        {
            if ( (!ignoreDefaultNames)||(_assemblyMatchValuesParent[i].compare("default")!=0) )
            {
                if (assemblingChildMatchValues->at(j).compare(_assemblyMatchValuesParent[i])==0)
                    return(true);
            }
        }
    }
    return(false);
}

int CSceneObject::getAllChildrenThatMayBecomeAssemblyParent(const std::vector<std::string>* assemblingChildMatchValues,std::vector<CSceneObject*>& objects) const
{
    for (size_t i=0;i<getChildCount();i++)
    {
        CSceneObject* child=getChildFromIndex(i);
        if (child->doesParentAssemblingMatchValuesMatchWithChild(assemblingChildMatchValues,true))
            objects.push_back(child);
        child->getAllChildrenThatMayBecomeAssemblyParent(assemblingChildMatchValues,objects);
    }
    return(int(objects.size()));
}

void CSceneObject::generateDnaString()
{
    _dnaString=CTTUtil::generateUniqueString();
}

std::string CSceneObject::getDnaString() const
{
    return(_dnaString);
}

void CSceneObject::setCopyString(const char* str)
{ // multipurpose. Will be copied during copy op.
    _copyString=str;
}

std::string CSceneObject::getCopyString() const
{
    return(_copyString);
}

std::string CSceneObject::getUniquePersistentIdString() const
{
    return(_uniquePersistentIdString);
}

int CSceneObject::getScriptExecutionOrder(int scriptType) const
{
    if (scriptType==sim_scripttype_customizationscript)
    {
        CScriptObject* it=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo_customization(_objectHandle);
        if (it!=nullptr)
            return(it->getExecutionPriority());
    }
    else if ((scriptType&sim_scripttype_childscript)!=0)
    {
        CScriptObject* it=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo_child(_objectHandle);
        if (it!=nullptr)
        {
            if ( it->getThreadedExecution_oldThreads()==((scriptType&sim_scripttype_threaded_old)!=0) )
                return(it->getExecutionPriority());
        }
    }
    return(sim_scriptexecorder_normal);
}

int CSceneObject::getScriptsToExecute(int scriptType,int parentTraversalDirection,std::vector<CScriptObject*>& scripts,std::vector<int>& uniqueIds)
{
    int cnt=0;
    CScriptObject* attachedScript=nullptr;
    if (scriptType==sim_scripttype_customizationscript)
        attachedScript=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo_customization(_objectHandle);
    else if ((scriptType&sim_scripttype_childscript)!=0)
    {
        attachedScript=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo_child(_objectHandle);
        if (attachedScript!=nullptr)
        {
            if ( attachedScript->getThreadedExecution_oldThreads()!=((scriptType&sim_scripttype_threaded_old)!=0) )
                attachedScript=nullptr;
        }
    }
    int traversalDir=parentTraversalDirection;
    if (attachedScript!=nullptr)
    {
        int tdir=attachedScript->getTreeTraversalDirection();
        if (tdir!=sim_scripttreetraversal_parent)
            traversalDir=tdir;
    }

    if ((getCumulativeModelProperty()&sim_modelproperty_scripts_inactive)==0)
    {
        if ( (traversalDir==sim_scripttreetraversal_forward)&&(attachedScript!=nullptr)&&(!attachedScript->getScriptIsDisabled()) )
        {
            cnt++;
            scripts.push_back(attachedScript);
            uniqueIds.push_back(attachedScript->getScriptUniqueID());
        }

        std::vector<CSceneObject*> orderFirst;
        std::vector<CSceneObject*> orderNormal;
        std::vector<CSceneObject*> orderLast;
        std::vector<std::vector<CSceneObject*>* > toHandle;
        toHandle.push_back(&orderFirst);
        toHandle.push_back(&orderNormal);
        toHandle.push_back(&orderLast);
        for (size_t i=0;i<getChildCount();i++)
        {
            CSceneObject* it=getChildFromIndex(i);
            toHandle[it->getScriptExecutionOrder(scriptType)]->push_back(it);
        }
        for (size_t i=0;i<toHandle.size();i++)
        {
            for (size_t j=0;j<toHandle[i]->size();j++)
                cnt+=toHandle[i]->at(j)->getScriptsToExecute(scriptType,traversalDir,scripts,uniqueIds);
        }

        if ( (traversalDir==sim_scripttreetraversal_reverse)&&(attachedScript!=nullptr)&&(!attachedScript->getScriptIsDisabled()) )
        {
            cnt++;
            scripts.push_back(attachedScript);
            uniqueIds.push_back(attachedScript->getScriptUniqueID());
        }
    }
    return(cnt);
}

void CSceneObject::_setLocalTransformation_send(const C7Vector& tr) const
{
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setObjectLocalTransformation(_ikPluginCounterpartHandle,_localTransformation);
}

void CSceneObject::_setParent_send(int parentHandle) const
{
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
    {
        int p=-1;
        if (getParent()!=nullptr)
            p=getParent()->getIkPluginCounterpartHandle();
        CPluginContainer::ikPlugin_setObjectParent(_ikPluginCounterpartHandle,p);
    }
}

bool CSceneObject::setParent(CSceneObject* parent)
{
    bool diff=(_parentObject!=parent);
    if (diff)
    {
        _parentObject=parent;
        if ( _isInScene&&App::worldContainer->getEventsEnabled() )
        {
            const char* cmd="parentUid";
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

void CSceneObject::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            // KEEP FOLLOWING ALWAYS AT THE BEGINNING!!!!!!! (3do)
            ar.storeDataName("3do"); // 3D object identifier, added on 2009/12/09. Needed for forward compatibility when trying to load an object type that doesn't yet exist!
            ar << ((unsigned char)57) << ((unsigned char)58) << ((unsigned char)59);
            ar.flush();

            ar.storeDataName("Cfq");
            C7Vector tr=getLocalTransformation();
            if (_ignorePosAndCameraOrthoviewSize_forUndoRedo)
                tr.setIdentity();
            ar << tr.Q(0) << tr.Q(1) << tr.Q(2) << tr.Q(3) << tr.X(0) << tr.X(1) << tr.X(2);
            ar.flush();

            ar.storeDataName("Alt");
            ar << _assemblingLocalTransformation.Q(0) << _assemblingLocalTransformation.Q(1) << _assemblingLocalTransformation.Q(2) << _assemblingLocalTransformation.Q(3) << _assemblingLocalTransformation.X(0) << _assemblingLocalTransformation.X(1) << _assemblingLocalTransformation.X(2);
            ar.flush();

            ar.storeDataName("Am2");
            ar << int(_assemblyMatchValuesChild.size());
            for (size_t i=0;i<_assemblyMatchValuesChild.size();i++)
                ar << _assemblyMatchValuesChild[i];
            ar << int(_assemblyMatchValuesParent.size());
            for (size_t i=0;i<_assemblyMatchValuesParent.size();i++)
                ar << _assemblyMatchValuesParent[i];
            ar.flush();

            ar.storeDataName("Ids");
            int parentID=-1;
            if (getParent()!=nullptr)
                parentID=getParent()->getObjectHandle();
            ar << _objectHandle << parentID;
            ar.flush();

            ar.storeDataName("Ali"); // keep this before "Nme"
            ar << _objectAlias;
            ar.flush();

            ar.storeDataName("Anm"); // keep this before "Nme"
            ar << _objectAltName_old;
            ar.flush();

            ar.storeDataName("Nme");
            ar << _objectName_old;
            ar.flush();

            ar.storeDataName("Hci");
            ar << _hierarchyColorIndex;
            ar.flush();

            ar.storeDataName("Sci");
            ar << _collectionSelfCollisionIndicator;
            ar.flush();

            ar.storeDataName("Op2");
            int objProp=_objectProperty|sim_objectproperty_reserved5; // Needed for backward compatibility (still in serialization version 15)
            ar << objProp;
            ar.flush();

            // Keep a while for backward compatibility (19/4/2017) (in case people want to return to a previous CoppeliaSim version):
            ar.storeDataName("Va2");
            unsigned char dummy=0;
            SIM_SET_CLEAR_BIT(dummy,0,_modelBase);
            SIM_SET_CLEAR_BIT(dummy,1,(_objectMovementOptions&2)!=0);
            SIM_SET_CLEAR_BIT(dummy,2,(_objectMovementOptions&1)!=0);
            SIM_SET_CLEAR_BIT(dummy,7,_assemblingLocalTransformationIsUsed);
            ar << dummy;
            ar.flush();


            ar.storeDataName("Va3");
            dummy=0;
            SIM_SET_CLEAR_BIT(dummy,0,_modelBase);
            SIM_SET_CLEAR_BIT(dummy,1,(_objectMovementOptions&2)!=0);
            SIM_SET_CLEAR_BIT(dummy,2,(_objectMovementOptions&1)!=0);
            // 3 is reserved and should remain at false
            SIM_SET_CLEAR_BIT(dummy,4,(_objectMovementOptions&8)!=0);
            SIM_SET_CLEAR_BIT(dummy,5,(_objectMovementOptions&4)!=0);
            SIM_SET_CLEAR_BIT(dummy,7,_assemblingLocalTransformationIsUsed);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Va4");
            dummy=0;
            SIM_SET_CLEAR_BIT(dummy,0,(_objectMovementOptions&16)!=0);
            SIM_SET_CLEAR_BIT(dummy,1,(_objectMovementOptions&32)!=0);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Omp");
            ar << _localObjectSpecialProperty;
            ar.flush();

            ar.storeDataName("Mpo");
            ar << _modelProperty;
            ar.flush();

            ar.storeDataName("Lar");
            ar << _visibilityLayer;
            ar.flush();

            ar.storeDataName("Cor");
            ar << _childOrder;
            ar.flush();

            ar.storeDataName("Om5");
            ar << _objectMovementPreferredAxes << _objectMovementRelativity[0] << _objectMovementStepSize[0];
            ar.flush();

            ar.storeDataName("Omr");
            ar << _objectMovementRelativity[1] << _objectMovementStepSize[1];
            ar.flush();

            ar.storeDataName("Sfa");
            ar << _sizeFactor;
            ar.flush();

            ar.storeDataName("Sfb");
            ar << _sizeValues[0] << _sizeValues[1] << _sizeValues[2];
            ar.flush();

            if (_customObjectData.getDataCount()!=0)
            {
                ar.storeDataName("Cda");
                ar.setCountingMode();
                _customObjectData.serializeData(ar,nullptr);
                if (ar.setWritingMode())
                    _customObjectData.serializeData(ar,nullptr);
            }

            if (_customObjectData_old!=nullptr)
            { // keep for backward compatibility (e.g. until V4.4.0)
                ar.storeDataName("Cod");
                ar.setCountingMode();
                _customObjectData_old->serializeData(ar,nullptr,-1);
                if (ar.setWritingMode())
                    _customObjectData_old->serializeData(ar,nullptr,-1);
            }

            if (_userScriptParameters!=nullptr)
            {
                ar.storeDataName("Lsp");
                ar.setCountingMode();
                _userScriptParameters->serialize(ar);
                if (ar.setWritingMode())
                    _userScriptParameters->serialize(ar);
            }

            if (_customReferencedHandles.size()>0)
            {
                ar.storeDataName("Crh");
                ar << int(_customReferencedHandles.size());
                for (size_t i=0;i<_customReferencedHandles.size();i++)
                {
                    ar << _customReferencedHandles[i].generalObjectType;
                    ar << _customReferencedHandles[i].generalObjectHandle;
                    ar << int(0);
                }
                ar.flush();
            }

            if (_customReferencedOriginalHandles.size()>0)
            {
                ar.storeDataName("Orh");
                ar << int(_customReferencedOriginalHandles.size());
                for (size_t i=0;i<_customReferencedOriginalHandles.size();i++)
                {
                    ar << _customReferencedOriginalHandles[i].generalObjectType;
                    ar << _customReferencedOriginalHandles[i].generalObjectHandle;
                    if (_customReferencedOriginalHandles[i].generalObjectHandle>=0)
                        ar << _customReferencedOriginalHandles[i].uniquePersistentIdString;
                }
                ar.flush();
            }

            ar.storeDataName("Ack");
            ar << _modelAcknowledgement;
            ar.flush();

            ar.storeDataName("Ups");
            ar << _dnaString;
            ar.flush();

            ar.storeDataName("Uis");
            ar << _uniquePersistentIdString;
            ar.flush();

            ar.storeDataName("Tdo");
            ar << _transparentObjectDistanceOffset;
            ar.flush();

            ar.storeDataName("Avo");
            ar << _authorizedViewableObjects;
            ar.flush();

            ar.storeDataName("Rst");
            ar << _extensionString;
            ar.flush();

            ar.storeDataName(SER_NEXT_STEP);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            bool hasAltName=false;
            bool hasAlias=false;
            bool _assemblingLocalTransformationIsUsed_compatibility=false;
            while (theName.compare(SER_NEXT_STEP)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_NEXT_STEP)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("3do")==0)
                    { // 3D object identifier. Needed for forward compatibility when trying to load an object type that doesn't yet exist!
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy >> dummy >> dummy;
                    }
                    if (theName.compare("Cfq")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        C7Vector tr;
                        ar >> tr.Q(0) >> tr.Q(1) >> tr.Q(2) >> tr.Q(3) >> tr.X(0) >> tr.X(1) >> tr.X(2);
                        setLocalTransformation(tr);
                    }
                    if (theName.compare("Hci")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _hierarchyColorIndex;
                    }
                    if (theName.compare("Sci")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _collectionSelfCollisionIndicator;
                    }
                    if (theName.compare("Alt")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _assemblingLocalTransformation.Q(0) >> _assemblingLocalTransformation.Q(1) >> _assemblingLocalTransformation.Q(2) >> _assemblingLocalTransformation.Q(3) >> _assemblingLocalTransformation.X(0) >> _assemblingLocalTransformation.X(1) >> _assemblingLocalTransformation.X(2);
                        if (ar.getSerializationVersionThatWroteThisFile()<20)
                        {
                            C3Vector v(_assemblingLocalTransformation.Q(1),_assemblingLocalTransformation.Q(2),_assemblingLocalTransformation.Q(3));
                            if ( (_assemblingLocalTransformation.X.getLength()>0.0)||(v.getLength()>0.0) )
                                _assemblingLocalTransformationIsUsed_compatibility=true;
                        }
                    }
                    if (theName.compare("Amv")==0)
                    { // Keep for backward compatibility (31/3/2017)
                        noHit=false;
                        ar >> byteQuantity;
                        int child,parent;
                        ar >> child >> parent;
                        _assemblyMatchValuesChild.clear();
                        if (child==0)
                            _assemblyMatchValuesChild.push_back("default");
                        else
                            _assemblyMatchValuesChild.push_back(std::to_string(child));
                        _assemblyMatchValuesParent.clear();
                        if (parent==0)
                            _assemblyMatchValuesParent.push_back("default");
                        else
                            _assemblyMatchValuesParent.push_back(std::to_string(parent));
                    }
                    if (theName.compare("Am2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        int childCnt,parentCnt;
                        std::string word;
                        ar >> childCnt;
                        _assemblyMatchValuesChild.clear();
                        for (int i=0;i<childCnt;i++)
                        {
                            ar >> word;
                            _assemblyMatchValuesChild.push_back(word);
                        }
                        ar >> parentCnt;
                        _assemblyMatchValuesParent.clear();
                        for (int i=0;i<parentCnt;i++)
                        {
                            ar >> word;
                            _assemblyMatchValuesParent.push_back(word);
                        }
                    }
                    if (theName.compare("Ids")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _objectHandle >> _parentObjectHandle_forSerializationOnly;
                    }
                    if (theName.compare("Ali")==0)
                    {
                        hasAltName=true;
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _objectAlias;
                        _objectTempAlias=_objectAlias;
                        hasAlias=true;
                    }
                    if (theName.compare("Anm")==0)
                    {
                        hasAltName=true;
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _objectAltName_old;
                    }
                    if (theName.compare("Nme")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _objectName_old;
                        if (!hasAltName)
                            _objectAltName_old=tt::getObjectAltNameFromObjectName(_objectName_old.c_str());
                        _objectTempName_old=_objectName_old;
                        if (!hasAlias)
                            _objectAlias=_objectName_old.substr(0,_objectName_old.find('#'));
                    }
                    if (theName.compare("Op2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _objectProperty;
                    }
                    if (theName.compare("Var")==0)
                    { // Keep for backward compatibility (31/3/2017)
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _modelBase=SIM_IS_BIT_SET(dummy,0);
                        if (SIM_IS_BIT_SET(dummy,1))
                            _objectMovementOptions=_objectMovementOptions|10;
                        if (SIM_IS_BIT_SET(dummy,2))
                            _objectMovementOptions=_objectMovementOptions|5;
                        if (SIM_IS_BIT_SET(dummy,3))
                            _objectProperty|=sim_objectproperty_ignoreviewfitting;
                        // reserved since 9/6/2013   _useSpecialLocalTransformationWhenAssembling=SIM_IS_BIT_SET(dummy,4);
                        bool assemblyCanHaveChildRole=!SIM_IS_BIT_SET(dummy,5);
                        bool assemblyCanHaveParentRole=!SIM_IS_BIT_SET(dummy,6);
                        _assemblingLocalTransformationIsUsed=SIM_IS_BIT_SET(dummy,7);

                        if (ar.getSerializationVersionThatWroteThisFile()<20)
                            _assemblingLocalTransformationIsUsed=_assemblingLocalTransformationIsUsed_compatibility;

                        if (!assemblyCanHaveChildRole)
                            _assemblyMatchValuesChild.clear();
                        if (!assemblyCanHaveParentRole)
                            _assemblyMatchValuesParent.clear();
                    }
                    if (theName.compare("Va2")==0)
                    { // Keep for backward compatibility (19/4/2017)
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _modelBase=SIM_IS_BIT_SET(dummy,0);
                        if (SIM_IS_BIT_SET(dummy,1))
                            _objectMovementOptions=_objectMovementOptions|10;
                        if (SIM_IS_BIT_SET(dummy,2))
                            _objectMovementOptions=_objectMovementOptions|5;
                        if (SIM_IS_BIT_SET(dummy,3))
                            _objectProperty|=sim_objectproperty_ignoreviewfitting;
                        _assemblingLocalTransformationIsUsed=SIM_IS_BIT_SET(dummy,7);
                    }
                    if (theName.compare("Va3")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _modelBase=SIM_IS_BIT_SET(dummy,0);
                        if (SIM_IS_BIT_SET(dummy,1))
                            _objectMovementOptions=_objectMovementOptions|2;
                        if (SIM_IS_BIT_SET(dummy,2))
                            _objectMovementOptions=_objectMovementOptions|1;
                        if (SIM_IS_BIT_SET(dummy,3))
                            _objectProperty|=sim_objectproperty_ignoreviewfitting;
                        if (SIM_IS_BIT_SET(dummy,4))
                            _objectMovementOptions=_objectMovementOptions|8;
                        if (SIM_IS_BIT_SET(dummy,5))
                            _objectMovementOptions=_objectMovementOptions|4;
                        _assemblingLocalTransformationIsUsed=SIM_IS_BIT_SET(dummy,7);
                    }
                    if (theName.compare("Va4")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        if (SIM_IS_BIT_SET(dummy,0))
                            _objectMovementOptions=_objectMovementOptions|16;
                        if (SIM_IS_BIT_SET(dummy,1))
                            _objectMovementOptions=_objectMovementOptions|32;
                    }

                    if (theName.compare("Omp")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _localObjectSpecialProperty;
                    }
                    if (theName.compare("Mpo")==0)
                    { // from 2010/08/06
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _modelProperty;
                    }
                    if (theName.compare("Lar")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _visibilityLayer;
                    }
                    if (theName.compare("Cor")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _childOrder;
                    }
                    if (theName.compare("Om5")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _objectMovementPreferredAxes >> _objectMovementRelativity[0] >> _objectMovementStepSize[0];
                    }
                    if (theName.compare("Omr")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _objectMovementRelativity[1] >> _objectMovementStepSize[1];
                    }
                    if (theName.compare("Cda")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        _customObjectData.serializeData(ar,nullptr);
                    }
                    if (theName.compare("Cod")==0)
                    { // keep for backward compatibility
                        noHit=false;
                        ar >> byteQuantity;
                        _customObjectData_old=new CCustomData_old();
                        _customObjectData_old->serializeData(ar,nullptr,-1);
                        if (_customObjectData.getDataCount()==0)
                            _customObjectData_old->initNewFormat(_customObjectData,true);
                    }
                    if (theName.compare("Lsp")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        _userScriptParameters=new CUserParameters();
                        _userScriptParameters->serialize(ar);
                        if (_userScriptParameters->userParamEntries.size()==0)
                        {
                            delete _userScriptParameters;
                            _userScriptParameters=nullptr;
                        }
                        else
                        {
                            if (CSimFlavor::getBoolVal(18))
                                App::logMsg(sim_verbosity_errors,"Contains script simulation parameters...");
                        }
                    }
                    if (theName.compare("Crh")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        int cnt,dummy;
                        ar >> cnt;
                        for (int i=0;i<cnt;i++)
                        {
                            SCustomRefs r;
                            ar >> r.generalObjectType;
                            ar >> r.generalObjectHandle;
                            ar >> dummy;
                            _customReferencedHandles.push_back(r);
                        }
                    }
                    if (theName.compare("Orh")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        int cnt;
                        ar >> cnt;
                        for (int i=0;i<cnt;i++)
                        {
                            SCustomOriginalRefs r;
                            ar >> r.generalObjectType;
                            ar >> r.generalObjectHandle;
                            if (r.generalObjectHandle>=0)
                                ar >> r.uniquePersistentIdString;
                            _customReferencedOriginalHandles.push_back(r);
                        }
                    }
                    if (theName.compare("Sfa")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _sizeFactor;
                    }

                    if (theName.compare("Sfb")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _sizeValues[0] >> _sizeValues[1] >> _sizeValues[2];
                    }

                    if (theName.compare("Ack")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _modelAcknowledgement;
                        if (CSimFlavor::getBoolVal(18))
                        {
                        }
                    }
                    if (theName.compare("Ups")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _dnaString;
                        if (_dnaString.size()==0)
                            generateDnaString();
                    }
                    if (theName.compare("Uis")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _uniquePersistentIdString;
                    }
                    if (theName.compare("Tdo")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _transparentObjectDistanceOffset;
                    }
                    if (theName.compare("Avo")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _authorizedViewableObjects;
                    }
                    if (theName.compare("Rst")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _extensionString;
                    }

                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            //*************************************************************
            // For backward compatibility 13/09/2011
            if ((_objectProperty&sim_objectproperty_reserved5)==0)
            { // this used to be the sim_objectproperty_visible property. If it wasn't set in the past, we now try to hide it in a hidden layer:
                if (_visibilityLayer<256)
                    _visibilityLayer=_visibilityLayer*256;
            }
            else
                _objectProperty-=sim_objectproperty_reserved5;
            //*************************************************************

            //*************************************************************
            // For old models to support the DNA-thing by default:
            if ( (ar.getCoppeliaSimVersionThatWroteThisFile()<30003)&&getModelBase() )
            {
                _dnaString="1234567890123456";
                std::string a(CTTUtil::generateUniqueReadableString());
                while (a.length()<16)
                    a=a+"*";
                std::string b("1234567890123456");
                int fbp=ar.getFileBufferReadPointer();
                b[2]=((unsigned char*)&fbp)[0];
                b[3]=((unsigned char*)&fbp)[1];
                b[4]=((unsigned char*)&fbp)[2];
                b[5]=((unsigned char*)&fbp)[3];
                for (int i=0;i<16;i++)
                {
                    _dnaString[i]+=a[i];
                    _dnaString[i]+=b[i];
                }
            }
            //*************************************************************
        }
    }
    else
    {
        bool exhaustiveXml=( (ar.getFileType()!=CSer::filetype_csim_xml_simplescene_file)&&(ar.getFileType()!=CSer::filetype_csim_xml_simplemodel_file) );
        if (ar.isStoring())
        {
            ar.xmlPushNewNode("common");

            if (exhaustiveXml)
                ar.xmlAddNode_string("alias",_objectAlias.c_str());
            else
                ar.xmlAddNode_string("alias",(_objectAlias+"*"+std::to_string(_objectHandle)).c_str());
            ar.xmlAddNode_comment(" 'name' and 'altName' tags only used for backward compatibility:",exhaustiveXml);
            ar.xmlAddNode_string("name",_objectName_old.c_str());
            ar.xmlAddNode_string("altName",_objectAltName_old.c_str());

            if (exhaustiveXml)
            {
                ar.xmlAddNode_int("handle",_objectHandle);
                int parentID=-1;
                if (getParent()!=nullptr)
                    parentID=getParent()->getObjectHandle();
                ar.xmlAddNode_int("parentHandle",parentID);
            }

            ar.xmlPushNewNode("localFrame");
            C7Vector tr=getLocalTransformation();
            if (getObjectType()==sim_object_shape_type)
            {
                ar.xmlAddNode_comment(" 'position' tag (in case of a shape): the value of this tag will be used to correctly build the shape, relative to its parent (or children), ",exhaustiveXml);
                ar.xmlAddNode_comment(" however, when load operation is finished, the local position of the shape will very probably be different (because the position of the shape ",exhaustiveXml);
                ar.xmlAddNode_comment(" is automatically selected to be at the geometric center of the shape) ",exhaustiveXml);
            }
            ar.xmlAddNode_3float("position", tr.X(0), tr.X(1), tr.X(2));
            if (exhaustiveXml)
                ar.xmlAddNode_4float("quaternion", tr.Q(0), tr.Q(1), tr.Q(2), tr.Q(3));
            else
            {
                if (getObjectType()==sim_object_shape_type)
                {
                    ar.xmlAddNode_comment(" 'euler' tag (in case of a shape): the value of this tag will be used to correctly build the shape, relative to its parent (or children), ",exhaustiveXml);
                    ar.xmlAddNode_comment(" however, when load operation is finished, the local orientation of the shape might be different (primitive shapes have a fixed orientation) ",exhaustiveXml);
                }
                C3Vector euler(tr.Q.getEulerAngles());
                euler*=180.0f/piValue_f;
                ar.xmlAddNode_floats("euler",euler.data,3);
            }
            ar.xmlPopNode();

            if (exhaustiveXml)
            {
                ar.xmlPushNewNode("assembling");
                ar.xmlPushNewNode("localFrame");
                tr=_assemblingLocalTransformation;
                ar.xmlAddNode_3float("position", tr.X(0), tr.X(1), tr.X(2));
                ar.xmlAddNode_4float("quaternion", tr.Q(0), tr.Q(1), tr.Q(2), tr.Q(3));
                ar.xmlPopNode();
                ar.xmlPushNewNode("matchValues");
                ar.xmlAddNode_strings("child", _assemblyMatchValuesChild);
                ar.xmlAddNode_strings("parent", _assemblyMatchValuesParent);
                ar.xmlPopNode();
                ar.xmlAddNode_bool("localFrameIsUsed",_assemblingLocalTransformationIsUsed);
                ar.xmlPopNode();
            }

            ar.xmlAddNode_int("hierarchyColorIndex",_hierarchyColorIndex);

            ar.xmlAddNode_int("collectionSelfCollisionIndicator",_collectionSelfCollisionIndicator);

            ar.xmlPushNewNode("localObjectProperty");
            ar.xmlAddNode_bool("hierarchyCollapsed",_objectProperty&sim_objectproperty_collapsed);
            ar.xmlAddNode_bool("selectable",_objectProperty&sim_objectproperty_selectable);
            ar.xmlAddNode_bool("selectModelBaseInstead",_objectProperty&sim_objectproperty_selectmodelbaseinstead);
            ar.xmlAddNode_bool("dontShowAsInsideModel",_objectProperty&sim_objectproperty_dontshowasinsidemodel);
            ar.xmlAddNode_bool("selectInvisible",_objectProperty&sim_objectproperty_selectinvisible);
            ar.xmlAddNode_bool("depthInvisible",_objectProperty&sim_objectproperty_depthinvisible);
            ar.xmlAddNode_bool("cannotDelete",_objectProperty&sim_objectproperty_cannotdelete);
            ar.xmlAddNode_bool("cannotDeleteDuringSimulation",_objectProperty&sim_objectproperty_cannotdeleteduringsim);
            ar.xmlAddNode_bool("ignoreViewFitting",_objectProperty&sim_objectproperty_ignoreviewfitting);
            ar.xmlPopNode();

            ar.xmlPushNewNode("localObjectSpecialProperty");
            ar.xmlAddNode_bool("collidable",_localObjectSpecialProperty&sim_objectspecialproperty_collidable);
            ar.xmlAddNode_bool("measurable",_localObjectSpecialProperty&sim_objectspecialproperty_measurable);
            if ((_localObjectSpecialProperty&sim_objectspecialproperty_detectable)==sim_objectspecialproperty_detectable)
                ar.xmlAddNode_bool("detectable",true);
            if ((_localObjectSpecialProperty&sim_objectspecialproperty_detectable)==0)
                ar.xmlAddNode_bool("detectable",false);
            // OLD:
            ar.xmlAddNode_comment(" 'renderable' tag for backward compatibility, set to 'true':",exhaustiveXml);
            ar.xmlAddNode_bool("renderable",_localObjectSpecialProperty&sim_objectspecialproperty_renderable); // for backward compatibility
            ar.xmlAddNode_comment(" following 5 for backward compatibility:",exhaustiveXml);
            ar.xmlAddNode_bool("ultrasonicDetectable",_localObjectSpecialProperty&sim_objectspecialproperty_detectable_ultrasonic);
            ar.xmlAddNode_bool("infraredDetectable",_localObjectSpecialProperty&sim_objectspecialproperty_detectable_infrared);
            ar.xmlAddNode_bool("laserDetectable",_localObjectSpecialProperty&sim_objectspecialproperty_detectable_laser);
            ar.xmlAddNode_bool("inductiveDetectable",_localObjectSpecialProperty&sim_objectspecialproperty_detectable_inductive);
            ar.xmlAddNode_bool("capacitiveDetectable",_localObjectSpecialProperty&sim_objectspecialproperty_detectable_capacitive);

            ar.xmlPopNode();

            ar.xmlPushNewNode("localModelProperty");
            ar.xmlAddNode_bool("notCollidable",_modelProperty&sim_modelproperty_not_collidable);
            ar.xmlAddNode_bool("notMeasurable",_modelProperty&sim_modelproperty_not_measurable);
            ar.xmlAddNode_bool("notDetectable",_modelProperty&sim_modelproperty_not_detectable);
            ar.xmlAddNode_bool("notDynamic",_modelProperty&sim_modelproperty_not_dynamic);
            ar.xmlAddNode_bool("notRespondable",_modelProperty&sim_modelproperty_not_respondable);
            ar.xmlAddNode_bool("notReset",_modelProperty&sim_modelproperty_not_reset);
            ar.xmlAddNode_bool("notVisible",_modelProperty&sim_modelproperty_not_visible);
            ar.xmlAddNode_bool("scriptsInactive",_modelProperty&sim_modelproperty_scripts_inactive);
            ar.xmlAddNode_bool("notShowAsInsideModel",_modelProperty&sim_modelproperty_not_showasinsidemodel);

            // For backward compatibility:
            ar.xmlAddNode_comment(" 'notRenderable' tag for backward compatibility, set to 'false':",exhaustiveXml);
            ar.xmlAddNode_bool("notRenderable",_modelProperty&sim_modelproperty_not_renderable);

            ar.xmlPopNode();

            ar.xmlAddNode_int("layer",_visibilityLayer);
            ar.xmlAddNode_int("childOrder",_childOrder);

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("modelBase",_modelBase);
            ar.xmlPopNode();

            if (exhaustiveXml)
            {
                ar.xmlPushNewNode("manipulation");
                ar.xmlAddNode_int("permissions",_objectMovementPreferredAxes);
                ar.xmlPushNewNode("translation");
                ar.xmlAddNode_bool("disabledDuringSimulation",(_objectMovementOptions&2)!=0);
                ar.xmlAddNode_bool("disabledDuringNonSimulation",(_objectMovementOptions&1)!=0);
                ar.xmlAddNode_bool("settingsLocked",(_objectMovementOptions&16)!=0);
                ar.xmlAddNode_int("relativeTo",_objectMovementRelativity[0]);
                ar.xmlAddNode_float("nonDefaultStepSize",_objectMovementStepSize[0]);
                ar.xmlPopNode();
                ar.xmlPushNewNode("rotation");
                ar.xmlAddNode_bool("disabledDuringSimulation",(_objectMovementOptions&8)!=0);
                ar.xmlAddNode_bool("disabledDuringNonSimulation",(_objectMovementOptions&4)!=0);
                ar.xmlAddNode_bool("settingsLocked",(_objectMovementOptions&32)!=0);
                ar.xmlAddNode_int("relativeTo",_objectMovementRelativity[1]);
                ar.xmlAddNode_float("nonDefaultStepSize",_objectMovementStepSize[1]);
                ar.xmlPopNode();
                ar.xmlPopNode();
            }

            if (exhaustiveXml)
            {
                ar.xmlAddNode_float("sizeFactor",_sizeFactor);

                ar.xmlAddNode_floats("sizeValues",_sizeValues,3);

                std::string str(base64_encode((unsigned char*)_dnaString.c_str(),_dnaString.size()));
                ar.xmlAddNode_string("dnaString_base64Coded",str.c_str());

                str=base64_encode((unsigned char*)_uniquePersistentIdString.c_str(),_uniquePersistentIdString.size());
                ar.xmlAddNode_string("uniquePersistentString_base64Coded",str.c_str());

                ar.xmlAddNode_float("transparentObjectDistanceOffset",_transparentObjectDistanceOffset);

                ar.xmlAddNode_int("authorizedViewableObjects",_authorizedViewableObjects);
            }

            ar.xmlAddNode_string("extensionString",_extensionString.c_str());

            ar.xmlAddNode_string("modelAcknowledgement",_modelAcknowledgement.c_str());

            if (exhaustiveXml)
            {
                if (_customObjectData.getDataCount()!=0)
                {
                    ar.xmlPushNewNode("customObjectData");
                    _customObjectData.serializeData(ar,getObjectAliasAndHandle().c_str());
                    ar.xmlPopNode();
                }
                if (_customObjectData_old!=nullptr)
                { // keep a while for backward compatibility (e.g. until V4.4.0)
                    ar.xmlPushNewNode("customData");
                    _customObjectData_old->serializeData(ar,getObjectAliasAndHandle().c_str(),-1);
                    ar.xmlPopNode();
                }
                if (_userScriptParameters!=nullptr)
                {
                    ar.xmlPushNewNode("userParameters");
                    _userScriptParameters->serialize(ar);
                    ar.xmlPopNode();
                }
                if (_customReferencedHandles.size()>0)
                {
                    ar.xmlPushNewNode("customReferencedHandles");
                    ar.xmlAddNode_int("count",int(_customReferencedHandles.size()));
                    std::vector<int> tmp;
                    for (size_t i=0;i<_customReferencedHandles.size();i++)
                        tmp.push_back(_customReferencedHandles[i].generalObjectType);
                    ar.xmlAddNode_ints("generalObjectTypes",&tmp[0],tmp.size());
                    tmp.clear();
                    for (size_t i=0;i<_customReferencedHandles.size();i++)
                        tmp.push_back(_customReferencedHandles[i].generalObjectHandle);
                    ar.xmlAddNode_ints("generalObjectHandles",&tmp[0],tmp.size());
                    ar.xmlPopNode();
                }

                if (_customReferencedOriginalHandles.size()>0)
                {
                    ar.xmlPushNewNode("customReferencedOriginalHandles");
                    ar.xmlAddNode_int("count",int(_customReferencedOriginalHandles.size()));
                    std::vector<int> tmp;
                    for (size_t i=0;i<_customReferencedOriginalHandles.size();i++)
                        tmp.push_back(_customReferencedOriginalHandles[i].generalObjectType);
                    ar.xmlAddNode_ints("generalObjectTypes",&tmp[0],tmp.size());
                    tmp.clear();
                    for (size_t i=0;i<_customReferencedOriginalHandles.size();i++)
                        tmp.push_back(_customReferencedOriginalHandles[i].generalObjectHandle);
                    ar.xmlAddNode_ints("generalObjectHandles",&tmp[0],tmp.size());
                    std::vector<std::string> sTmp;
                    for (size_t i=0;i<_customReferencedOriginalHandles.size();i++)
                        sTmp.push_back(base64_encode((unsigned char*)_customReferencedOriginalHandles[i].uniquePersistentIdString.c_str(),_customReferencedOriginalHandles[i].uniquePersistentIdString.size()));
                    ar.xmlAddNode_strings("uniquePersistentIdString_base64Coded",sTmp);

                    ar.xmlPopNode();
                }
            }

            ar.xmlPopNode();
        }
        else
        {
            bool aliasFound=false;
            bool _ignoredByViewFitting_backCompat=false;
            if (ar.xmlPushChildNode("common",exhaustiveXml))
            {
                aliasFound=ar.xmlGetNode_string("alias",_objectAlias,false); // keep false for compatibility with older versions! exhaustiveXml);
                if (aliasFound)
                {
                    _objectTempAlias=_objectAlias;
                    _objectAlias=_objectAlias.substr(0,_objectAlias.find('*'));
                    _objectAlias=tt::getValidAlias(_objectAlias.c_str());
                }
                if ( ar.xmlGetNode_string("name",_objectName_old,exhaustiveXml)&&(!exhaustiveXml) )
                {
                    tt::removeIllegalCharacters(_objectName_old,true);
                    _objectTempName_old=_objectName_old;
                    _objectName_old="XYZ___"+_objectName_old+"___XYZ";
                }
                if ( ar.xmlGetNode_string("altName",_objectAltName_old,exhaustiveXml)&&(!exhaustiveXml) )
                {
                    tt::removeAltNameIllegalCharacters(_objectAltName_old);
                    _objectTempAltName_old=_objectAltName_old;
                    _objectAltName_old="XYZ___"+_objectAltName_old+"___XYZ";
                }

                if (exhaustiveXml)
                {
                    ar.xmlGetNode_int("handle",_objectHandle);
                    ar.xmlGetNode_int("parentHandle",_parentObjectHandle_forSerializationOnly);
                }

                if (ar.xmlPushChildNode("localFrame",exhaustiveXml))
                {
                    C7Vector tr;
                    tr.setIdentity();
                    ar.xmlGetNode_floats("position",tr.X.data,3,exhaustiveXml);
                    if (exhaustiveXml)
                    {
                        ar.xmlGetNode_floats("quaternion",tr.Q.data,4);
                        tr.Q.normalize(); // just in case
                    }
                    else
                    {
                        C3Vector euler;
                        if (ar.xmlGetNode_floats("euler",euler.data,3,exhaustiveXml))
                        {
                            euler(0)*=piValue_f/180.0f;
                            euler(1)*=piValue_f/180.0f;
                            euler(2)*=piValue_f/180.0f;
                            tr.Q.setEulerAngles(euler);
                        }
                    }
                    setLocalTransformation(tr);
                    ar.xmlPopNode();
                }

                if (exhaustiveXml&&ar.xmlPushChildNode("assembling"))
                {
                    if (ar.xmlPushChildNode("localFrame"))
                    {
                        ar.xmlGetNode_floats("position",_assemblingLocalTransformation.X.data,3);
                        ar.xmlGetNode_floats("quaternion",_assemblingLocalTransformation.Q.data,4);
                        _assemblingLocalTransformation.Q.normalize(); // just in case
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("matchValues"))
                    {
                        _assemblyMatchValuesChild.clear();
                        ar.xmlGetNode_strings("child",_assemblyMatchValuesChild);
                        _assemblyMatchValuesParent.clear();
                        ar.xmlGetNode_strings("parent",_assemblyMatchValuesParent);
                        ar.xmlPopNode();
                    }
                    ar.xmlGetNode_bool("localFrameIsUsed",_assemblingLocalTransformationIsUsed);
                    ar.xmlPopNode();
                }

                ar.xmlGetNode_int("hierarchyColorIndex",_hierarchyColorIndex,exhaustiveXml);

                ar.xmlGetNode_int("collectionSelfCollisionIndicator",_collectionSelfCollisionIndicator,exhaustiveXml);

                if (ar.xmlPushChildNode("localObjectProperty",exhaustiveXml))
                {
                    _objectProperty=0;
                    ar.xmlGetNode_flags("hierarchyCollapsed",_objectProperty,sim_objectproperty_collapsed,exhaustiveXml);
                    ar.xmlGetNode_flags("selectable",_objectProperty,sim_objectproperty_selectable,exhaustiveXml);
                    ar.xmlGetNode_flags("selectModelBaseInstead",_objectProperty,sim_objectproperty_selectmodelbaseinstead,exhaustiveXml);
                    ar.xmlGetNode_flags("dontShowAsInsideModel",_objectProperty,sim_objectproperty_dontshowasinsidemodel,exhaustiveXml);
                    ar.xmlGetNode_flags("selectInvisible",_objectProperty,sim_objectproperty_selectinvisible,exhaustiveXml);
                    ar.xmlGetNode_flags("depthInvisible",_objectProperty,sim_objectproperty_depthinvisible,exhaustiveXml);
                    ar.xmlGetNode_flags("cannotDelete",_objectProperty,sim_objectproperty_cannotdelete,exhaustiveXml);
                    ar.xmlGetNode_flags("cannotDeleteDuringSimulation",_objectProperty,sim_objectproperty_cannotdeleteduringsim,exhaustiveXml);
                    ar.xmlGetNode_flags("ignoreViewFitting",_objectProperty,sim_objectproperty_ignoreviewfitting,false);
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("localObjectSpecialProperty",exhaustiveXml))
                {
                    _localObjectSpecialProperty=0;
                    ar.xmlGetNode_flags("collidable",_localObjectSpecialProperty,sim_objectspecialproperty_collidable,exhaustiveXml);
                    ar.xmlGetNode_flags("measurable",_localObjectSpecialProperty,sim_objectspecialproperty_measurable,exhaustiveXml);
                    ar.xmlGetNode_flags("renderable",_localObjectSpecialProperty,sim_objectspecialproperty_renderable,false); // for backward compatibility
                    // Following 5 for backward compatibility:
                    ar.xmlGetNode_flags("ultrasonicDetectable",_localObjectSpecialProperty,sim_objectspecialproperty_detectable_ultrasonic,false);
                    ar.xmlGetNode_flags("infraredDetectable",_localObjectSpecialProperty,sim_objectspecialproperty_detectable_infrared,false);
                    ar.xmlGetNode_flags("laserDetectable",_localObjectSpecialProperty,sim_objectspecialproperty_detectable_laser,false);
                    ar.xmlGetNode_flags("inductiveDetectable",_localObjectSpecialProperty,sim_objectspecialproperty_detectable_inductive,false);
                    ar.xmlGetNode_flags("capacitiveDetectable",_localObjectSpecialProperty,sim_objectspecialproperty_detectable_capacitive,false);
                    bool dummyDet;
                    if (ar.xmlGetNode_bool("detectable",dummyDet,false))
                    {
                        _localObjectSpecialProperty|=sim_objectspecialproperty_detectable;
                        if (!dummyDet)
                            _localObjectSpecialProperty-=sim_objectspecialproperty_detectable;
                    }

                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("localModelProperty",exhaustiveXml))
                {
                    _modelProperty=0;
                    ar.xmlGetNode_flags("notCollidable",_modelProperty,sim_modelproperty_not_collidable,exhaustiveXml);
                    ar.xmlGetNode_flags("notMeasurable",_modelProperty,sim_modelproperty_not_measurable,exhaustiveXml);
                    ar.xmlGetNode_flags("notRenderable",_modelProperty,sim_modelproperty_not_renderable,false); // for backward compatibility
                    ar.xmlGetNode_flags("notDetectable",_modelProperty,sim_modelproperty_not_detectable,exhaustiveXml);
                    ar.xmlGetNode_flags("notDynamic",_modelProperty,sim_modelproperty_not_dynamic,exhaustiveXml);
                    ar.xmlGetNode_flags("notRespondable",_modelProperty,sim_modelproperty_not_respondable,exhaustiveXml);
                    ar.xmlGetNode_flags("notReset",_modelProperty,sim_modelproperty_not_reset,exhaustiveXml);
                    ar.xmlGetNode_flags("notVisible",_modelProperty,sim_modelproperty_not_visible,exhaustiveXml);
                    ar.xmlGetNode_flags("scriptsInactive",_modelProperty,sim_modelproperty_scripts_inactive,exhaustiveXml);
                    ar.xmlGetNode_flags("notShowAsInsideModel",_modelProperty,sim_modelproperty_not_showasinsidemodel,exhaustiveXml);
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("switches",exhaustiveXml))
                {
                    ar.xmlGetNode_bool("modelBase",_modelBase,exhaustiveXml);
                    ar.xmlGetNode_bool("ignoredByViewFitting",_ignoredByViewFitting_backCompat,false);
                    ar.xmlPopNode();
                }

                int l;
                if (ar.xmlGetNode_int("layer",l,exhaustiveXml))
                    _visibilityLayer=(unsigned short)l;

                if (ar.xmlGetNode_int("childOrder",l,false)) // Keep false for compatibility with older versions! exhaustiveXml))
                    _childOrder=l;

                if (exhaustiveXml&&ar.xmlPushChildNode("manipulation"))
                {
                    ar.xmlGetNode_int("permissions",_objectMovementPreferredAxes);
                    if (ar.xmlPushChildNode("translation"))
                    {
                        bool tmp;
                        if (ar.xmlGetNode_bool("disabledDuringSimulation",tmp,false))
                        {
                            if (tmp)
                                _objectMovementOptions=_objectMovementOptions|2;
                        }
                        if (ar.xmlGetNode_bool("disabledDuringNonSimulation",tmp,false))
                        {
                            if (tmp)
                                _objectMovementOptions=_objectMovementOptions|1;
                        }
                        if (ar.xmlGetNode_bool("settingsLocked",tmp,false))
                        {
                            if (tmp)
                                _objectMovementOptions=_objectMovementOptions|16;
                        }
                        ar.xmlGetNode_int("relativeTo",_objectMovementRelativity[0]);
                        ar.xmlGetNode_float("nonDefaultStepSize",_objectMovementStepSize[0]);
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("rotation"))
                    {
                        bool tmp;
                        if (ar.xmlGetNode_bool("disabledDuringSimulation",tmp,false))
                        {
                            if (tmp)
                                _objectMovementOptions=_objectMovementOptions|8;
                        }
                        if (ar.xmlGetNode_bool("disabledDuringNonSimulation",tmp,false))
                        {
                            if (tmp)
                                _objectMovementOptions=_objectMovementOptions|4;
                        }
                        if (ar.xmlGetNode_bool("settingsLocked",tmp,false))
                        {
                            if (tmp)
                                _objectMovementOptions=_objectMovementOptions|32;
                        }
                        ar.xmlGetNode_int("relativeTo",_objectMovementRelativity[1]);
                        ar.xmlGetNode_float("nonDefaultStepSize",_objectMovementStepSize[1]);
                        ar.xmlPopNode();
                    }
                    ar.xmlPopNode();
                }

                if (exhaustiveXml)
                    ar.xmlGetNode_float("sizeFactor",_sizeFactor,exhaustiveXml);

                if (exhaustiveXml)
                    ar.xmlGetNode_floats("sizeValues",_sizeValues,3,exhaustiveXml);

                if (exhaustiveXml&&ar.xmlGetNode_string("dnaString_base64Coded",_dnaString))
                    _dnaString=base64_decode(_dnaString);

                if (exhaustiveXml&&ar.xmlGetNode_string("uniquePersistentString_base64Coded",_uniquePersistentIdString))
                    _uniquePersistentIdString=base64_decode(_uniquePersistentIdString);

                if (exhaustiveXml)
                    ar.xmlGetNode_float("transparentObjectDistanceOffset",_transparentObjectDistanceOffset,exhaustiveXml);

                if (exhaustiveXml)
                    ar.xmlGetNode_int("authorizedViewableObjects",_authorizedViewableObjects,exhaustiveXml);

                ar.xmlGetNode_string("extensionString",_extensionString,exhaustiveXml);

                ar.xmlGetNode_string("modelAcknowledgement",_modelAcknowledgement,exhaustiveXml);

                if (exhaustiveXml&&ar.xmlPushChildNode("customObjectData",false))
                {
                    _customObjectData.serializeData(ar,getObjectAliasAndHandle().c_str());
                    ar.xmlPopNode();
                }
                if (exhaustiveXml&&ar.xmlPushChildNode("customData",false))
                {
                    _customObjectData_old=new CCustomData_old();
                    _customObjectData_old->serializeData(ar,getObjectAliasAndHandle().c_str(),-1);
                    if (_customObjectData.getDataCount()==0)
                        _customObjectData_old->initNewFormat(_customObjectData,true);
                    ar.xmlPopNode();
                }
                if (exhaustiveXml&&ar.xmlPushChildNode("userParameters",false))
                {
                    if (_userScriptParameters!=nullptr)
                        delete _userScriptParameters;
                    _userScriptParameters=new CUserParameters();
                    _userScriptParameters->serialize(ar);
                    ar.xmlPopNode();
                }

                if (exhaustiveXml&&ar.xmlPushChildNode("customReferencedHandles",false))
                {
                    int cnt;
                    ar.xmlGetNode_int("count",cnt);
                    std::vector<int> ot;
                    std::vector<int> oh;
                    if (cnt>0)
                    {
                        ot.resize(cnt,-1);
                        oh.resize(cnt,-1);
                        ar.xmlGetNode_ints("generalObjectTypes",&ot[0],cnt);
                        ar.xmlGetNode_ints("generalObjectHandles",&oh[0],cnt);
                        for (int i=0;i<cnt;i++)
                        {
                            SCustomRefs r;
                            r.generalObjectType=ot[i];
                            r.generalObjectHandle=oh[i];
                            _customReferencedHandles.push_back(r);
                        }
                    }
                    ar.xmlPopNode();
                }

                if (exhaustiveXml&&ar.xmlPushChildNode("customReferencedOriginalHandles",false))
                {
                    int cnt;
                    ar.xmlGetNode_int("count",cnt);
                    std::vector<int> ot;
                    std::vector<int> oh;
                    std::vector<std::string> oi;
                    if (cnt>0)
                    {
                        ot.resize(cnt,-1);
                        oh.resize(cnt,-1);
                        ar.xmlGetNode_ints("generalObjectTypes",&ot[0],cnt);
                        ar.xmlGetNode_ints("generalObjectHandles",&oh[0],cnt);
                        ar.xmlGetNode_strings("uniquePersistentIdString_base64Coded",oi);
                        for (int i=0;i<cnt;i++)
                        {
                            SCustomOriginalRefs r;
                            r.generalObjectType=ot[i];
                            r.generalObjectHandle=oh[i];
                            if (r.generalObjectHandle>=0)
                                r.uniquePersistentIdString=base64_decode(oi[i]);
                            _customReferencedOriginalHandles.push_back(r);
                        }
                    }
                    ar.xmlPopNode();
                }
                ar.xmlPopNode();
            }
            if (_ignoredByViewFitting_backCompat)
                _objectProperty|=sim_objectproperty_ignoreviewfitting; // for backward compatibility
            if (!aliasFound)
            {
                if (exhaustiveXml)
                    _objectAlias=_objectName_old.substr(0,_objectName_old.find('#'));
                else
                    _objectAlias=_objectTempName_old.substr(0,_objectTempName_old.find('#'));
            }
        }
    }
}

void CSceneObject::performObjectLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{
    int newParentID=CWorld::getLoadingMapping(map,_parentObjectHandle_forSerializationOnly);
    App::currentWorld->sceneObjects->setObjectParent(this,App::currentWorld->sceneObjects->getObjectFromHandle(newParentID),false);

    if ( (_authorizedViewableObjects>=0)&&(_authorizedViewableObjects<SIM_IDSTART_COLLECTION) )
        _authorizedViewableObjects=CWorld::getLoadingMapping(map,_authorizedViewableObjects);

    for (size_t i=0;i<_customReferencedHandles.size();i++)
    {
        if (_customReferencedHandles[i].generalObjectType==sim_appobj_object_type)
            _customReferencedHandles[i].generalObjectHandle=CWorld::getLoadingMapping(map,_customReferencedHandles[i].generalObjectHandle);
    }
    if (!loadingAmodel)
    {
        for (size_t i=0;i<_customReferencedOriginalHandles.size();i++)
        {
            if (_customReferencedOriginalHandles[i].generalObjectType==sim_appobj_object_type)
                _customReferencedOriginalHandles[i].generalObjectHandle=CWorld::getLoadingMapping(map,_customReferencedOriginalHandles[i].generalObjectHandle);
        }
    }
}

void CSceneObject::performScriptLoadingMapping(const std::vector<int>* map)
{

}

std::string CSceneObject::getDisplayName() const
{
    return(getObjectAlias_printPath());
}

void CSceneObject::announceObjectWillBeErased(const CSceneObject* object,bool copyBuffer)
{ 
    // This routine can be called for sceneObjects-objects, but also for objects
    // in the copy-buffer!!
#ifdef SIM_WITH_GUI
    // if we are in edit mode, we leave edit mode:
    if ( (App::getEditModeType()!=NO_EDIT_MODE)&&(!copyBuffer) )
    {
        if (App::mainWindow->editModeContainer->getEditModeObjectID()==object->getObjectHandle())
            App::mainWindow->editModeContainer->processCommand(ANY_EDIT_MODE_FINISH_AND_CANCEL_CHANGES_EMCMD,nullptr); // This is if we destroy the object being edited (shouldn't normally happen!)
    }
#endif

    if (_authorizedViewableObjects==object->getObjectHandle())
        _authorizedViewableObjects=-2; // not visible anymore!

    // If the object's parent will be erased, make the object child of its grand-parents
    if (!copyBuffer)
    {
        CSceneObject* parent=getParent();
        if (parent!=nullptr)
        {
            if (parent==object)
                App::currentWorld->sceneObjects->setObjectParent(this,parent->getParent(),true);
        }
        removeChild(object);
    }
    for (size_t i=0;i<_customReferencedHandles.size();i++)
    {
        if (_customReferencedHandles[i].generalObjectType==sim_appobj_object_type)
        {
            if (_customReferencedHandles[i].generalObjectHandle==object->getObjectHandle())
                _customReferencedHandles[i].generalObjectHandle=-1;
        }
    }
    if (!copyBuffer)
    {
        for (size_t i=0;i<_customReferencedOriginalHandles.size();i++)
        {
            if (_customReferencedOriginalHandles[i].generalObjectType==sim_appobj_object_type)
            {
                if (_customReferencedOriginalHandles[i].generalObjectHandle==object->getObjectHandle())
                    _customReferencedOriginalHandles[i].generalObjectHandle=-1;
            }
        }
    }
}

void CSceneObject::announceScriptWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript,bool copyBuffer)
{

}

void CSceneObject::announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer)
{
    // This routine can be called for sceneObjects-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'ct::sceneObjects->getObject(id)'-call or similar
    for (size_t i=0;i<_customReferencedHandles.size();i++)
    {
        if (_customReferencedHandles[i].generalObjectType==sim_appobj_ik_type)
        {
            if (_customReferencedHandles[i].generalObjectHandle==ikGroupID)
                _customReferencedHandles[i].generalObjectHandle=-1;
        }
    }
    if (!copyBuffer)
    {
        for (size_t i=0;i<_customReferencedOriginalHandles.size();i++)
        {
            if (_customReferencedOriginalHandles[i].generalObjectType==sim_appobj_ik_type)
            {
                if (_customReferencedOriginalHandles[i].generalObjectHandle==ikGroupID)
                    _customReferencedOriginalHandles[i].generalObjectHandle=-1;
            }
        }
    }
}

void CSceneObject::setReferencedHandles(size_t cnt,const int* handles)
{
    _customReferencedHandles.clear();
    for (int i=0;i<cnt;i++)
    {
        SCustomRefs r;
        r.generalObjectType=sim_appobj_object_type;
        r.generalObjectHandle=-1;
        if (handles[i]>=0)
        {
            if (App::currentWorld->sceneObjects->getObjectFromHandle(handles[i])!=nullptr)
                r.generalObjectHandle=handles[i];
            else
            {
                if (App::currentWorld->collisions->getObjectFromHandle(handles[i])!=nullptr)
                {
                    r.generalObjectType=sim_appobj_collision_type;
                    r.generalObjectHandle=handles[i];
                }
                if (App::currentWorld->distances->getObjectFromHandle(handles[i])!=nullptr)
                {
                    r.generalObjectType=sim_appobj_distance_type;
                    r.generalObjectHandle=handles[i];
                }
                if (App::currentWorld->ikGroups->getObjectFromHandle(handles[i])!=nullptr)
                {
                    r.generalObjectType=sim_appobj_ik_type;
                    r.generalObjectHandle=handles[i];
                }
                if (App::currentWorld->collections->getObjectFromHandle(handles[i])!=nullptr)
                {
                    r.generalObjectType=sim_appobj_collection_type;
                    r.generalObjectHandle=handles[i];
                }
            }
        }
        _customReferencedHandles.push_back(r);
    }
}

size_t CSceneObject::getReferencedHandlesCount() const
{
    return(_customReferencedHandles.size());
}

size_t CSceneObject::getReferencedHandles(int* handles) const
{
    for (size_t i=0;i<_customReferencedHandles.size();i++)
        handles[i]=_customReferencedHandles[i].generalObjectHandle;
    return(_customReferencedHandles.size());
}


void CSceneObject::setReferencedOriginalHandles(int cnt,const int* handles)
{
    _customReferencedOriginalHandles.clear();
    for (int i=0;i<cnt;i++)
    {
        SCustomOriginalRefs r;
        r.generalObjectType=sim_appobj_object_type;
        r.generalObjectHandle=-1;
        if (handles[i]>=0)
        {
            if (App::currentWorld->sceneObjects->getObjectFromHandle(handles[i])!=nullptr)
            {
                r.generalObjectHandle=handles[i];
                r.uniquePersistentIdString=App::currentWorld->sceneObjects->getObjectFromHandle(handles[i])->getUniquePersistentIdString();
            }
            else
            {
                if (App::currentWorld->collisions->getObjectFromHandle(handles[i])!=nullptr)
                {
                    r.generalObjectType=sim_appobj_collision_type;
                    r.generalObjectHandle=handles[i];
                    r.uniquePersistentIdString=App::currentWorld->collisions->getObjectFromHandle(handles[i])->getUniquePersistentIdString();
                }
                if (App::currentWorld->distances->getObjectFromHandle(handles[i])!=nullptr)
                {
                    r.generalObjectType=sim_appobj_distance_type;
                    r.generalObjectHandle=handles[i];
                    r.uniquePersistentIdString=App::currentWorld->distances->getObjectFromHandle(handles[i])->getUniquePersistentIdString();
                }
                if (App::currentWorld->ikGroups->getObjectFromHandle(handles[i])!=nullptr)
                {
                    r.generalObjectType=sim_appobj_ik_type;
                    r.generalObjectHandle=handles[i];
                    r.uniquePersistentIdString=App::currentWorld->ikGroups->getObjectFromHandle(handles[i])->getUniquePersistentIdString();
                }
                if (App::currentWorld->collections->getObjectFromHandle(handles[i])!=nullptr)
                {
                    r.generalObjectType=sim_appobj_collection_type;
                    r.generalObjectHandle=handles[i];
                    r.uniquePersistentIdString=App::currentWorld->collections->getObjectFromHandle(handles[i])->getUniquePersistentIdString();
                }
            }
        }
        _customReferencedOriginalHandles.push_back(r);
    }
}

size_t CSceneObject::getReferencedOriginalHandlesCount() const
{
    return(_customReferencedOriginalHandles.size());
}

size_t CSceneObject::getReferencedOriginalHandles(int* handles) const
{
    for (size_t i=0;i<_customReferencedOriginalHandles.size();i++)
        handles[i]=_customReferencedOriginalHandles[i].generalObjectHandle;
    return(_customReferencedOriginalHandles.size());
}

void CSceneObject::checkReferencesToOriginal(const std::map<std::string,int>& allUniquePersistentIdStrings)
{
    for (size_t i=0;i<_customReferencedOriginalHandles.size();i++)
    {
        if (_customReferencedOriginalHandles[i].generalObjectHandle>=0)
        {
            std::map<std::string,int>::const_iterator it=allUniquePersistentIdStrings.find(_customReferencedOriginalHandles[i].uniquePersistentIdString);
            if (it!=allUniquePersistentIdStrings.end())
                _customReferencedOriginalHandles[i].generalObjectHandle=it->second;
            else
                _customReferencedOriginalHandles[i].generalObjectHandle=-1;
        }
    }
}

void CSceneObject::getCumulativeTransformationMatrix(float m[4][4]) const
{
    getCumulativeTransformation().copyTo(m);
}

void CSceneObject::setAbsoluteTransformation(const C7Vector& v)
{
    setLocalTransformation(getFullParentCumulativeTransformation().getInverse()*v);
}

void CSceneObject::setAbsoluteTransformation(const C4Vector& q)
{
    setLocalTransformation(getFullParentCumulativeTransformation().getInverse().Q*q);
}

void CSceneObject::setAbsoluteTransformation(const C3Vector& x)
{
    C7Vector tr(getLocalTransformation());
    tr.X=getFullParentCumulativeTransformation().getInverse()*x;
    setLocalTransformation(tr);
}

int CSceneObject::getIkPluginCounterpartHandle() const
{
    return(_ikPluginCounterpartHandle);
}

bool CSceneObject::isObjectParentedWith(const CSceneObject* thePotentialParent) const
{
    bool retVal=false;
    if (getParent()!=nullptr)
    {
        if (getParent()==thePotentialParent)
            retVal=true;
        else
            retVal=getParent()->isObjectParentedWith(thePotentialParent);
    }
    return(retVal);
}

void CSceneObject::setParentHandle_forSerializationOnly(int pHandle)
{
    _parentObjectHandle_forSerializationOnly=pHandle;
}

void CSceneObject::getFirstModelRelatives(std::vector<CSceneObject*>& firstModelRelatives,bool visibleModelsOnly) const
{
    for (size_t i=0;i<getChildCount();i++)
    {
        CSceneObject* child=getChildFromIndex(i);
        if (child->getModelBase())
        {
            if (!child->isObjectPartOfInvisibleModel())
                firstModelRelatives.push_back(child);
        }
        else
            child->getFirstModelRelatives(firstModelRelatives,visibleModelsOnly);
    }
}

int CSceneObject::countFirstModelRelatives(bool visibleModelsOnly) const
{
    int cnt=0;
    for (size_t i=0;i<getChildCount();i++)
    {
        CSceneObject* child=getChildFromIndex(i);
        if (child->getModelBase())
        {
            if (!child->isObjectPartOfInvisibleModel())
                cnt++;
        }
        else
            cnt+=child->countFirstModelRelatives(visibleModelsOnly);
    }
    return(cnt);
}

std::string CSceneObject::getObjectTempAlias() const
{
    return(_objectTempAlias);
}

std::string CSceneObject::getObjectTempName_old() const
{
    return(_objectTempName_old);
}

std::string CSceneObject::getObjectTempAltName_old() const
{
    return(_objectTempAltName_old);
}

CUserParameters* CSceneObject::getUserScriptParameterObject()
{
    return(_userScriptParameters);
}

void CSceneObject::setUserScriptParameterObject(CUserParameters* obj)
{
    if (_userScriptParameters!=nullptr)
        delete _userScriptParameters;
    _userScriptParameters=obj;
}

void CSceneObject::acquireCommonPropertiesFromObject_simpleXMLLoading(const CSceneObject* obj)
{ // names can't be changed here, probably same with aliases!
//    _objectName=obj->_objectName;
//    _objectAlias=obj->_objectAlias;
    _objectTempAlias=obj->_objectTempAlias;
    _objectTempName_old=obj->_objectTempName_old;
//    _objectAltName=obj->_objectAltName;
    _localTransformation=obj->_localTransformation;
    _hierarchyColorIndex=obj->_hierarchyColorIndex;
    _collectionSelfCollisionIndicator=obj->_collectionSelfCollisionIndicator;
    _objectProperty=obj->_objectProperty;
    _localObjectSpecialProperty=obj->_localObjectSpecialProperty;
    _modelProperty=obj->_modelProperty;
    _modelBase=obj->_modelBase;
    setVisibilityLayer(obj->getVisibilityLayer());
    _extensionString=obj->_extensionString;
    _modelAcknowledgement=obj->_modelAcknowledgement;
}

void CSceneObject::setRestoreToDefaultLights(bool s)
{
    _restoreToDefaultLights=s;
}

bool CSceneObject::getRestoreToDefaultLights() const
{
    return(_restoreToDefaultLights);
}

void CSceneObject::display(CViewableBase* renderingObject,int displayAttrib)
{
}

#ifdef SIM_WITH_GUI
void CSceneObject::displayManipulationModeOverlayGrid(bool transparentAndOverlay)
{
    if (_objectManipulationMode_flaggedForGridOverlay==0)
        return;
    App::setLightDialogRefreshFlag(); // to actualize the position and orientation dialogs!
    bool isPath=false;
    bool isPathPoints=false;
    float sizeValueForPath=0.0f;
    C3Vector localPositionOnPath;
    localPositionOnPath.clear();
    if (_objectType==sim_object_path_type)
    {
        isPath=true;
        std::vector<int> pathPointsToTakeIntoAccount;
        CPathCont_old* pc;
        if ( ( (App::getEditModeType()&PATH_EDIT_MODE_OLD)||(App::mainWindow->editModeContainer->pathPointManipulation->getSelectedPathPointIndicesSize_nonEditMode()!=0) )&&((_objectManipulationMode_flaggedForGridOverlay&8)==0) )

        { // (path is in edition or path points are selected) and no rotation
            isPathPoints=true;
            if (App::getEditModeType()&PATH_EDIT_MODE_OLD)
            { // Path is in edition
                pc=App::mainWindow->editModeContainer->getEditModePathContainer_old();
                pathPointsToTakeIntoAccount.assign(App::mainWindow->editModeContainer->getEditModeBuffer()->begin(),App::mainWindow->editModeContainer->getEditModeBuffer()->end());
            }
            else
            { // Path points are selected (but not in path edit mode)
                pc=((CPath_old*)this)->pathContainer;
                pathPointsToTakeIntoAccount.assign(App::mainWindow->editModeContainer->pathPointManipulation->getPointerToSelectedPathPointIndices_nonEditMode()->begin(),App::mainWindow->editModeContainer->pathPointManipulation->getPointerToSelectedPathPointIndices_nonEditMode()->end());
            }
        }
        else
        { // Path is not in edition and no path points are selected
            pc=((CPath_old*)this)->pathContainer;
            int cnt=pc->getSimplePathPointCount();
            for (int i=0;i<cnt;i++)
                pathPointsToTakeIntoAccount.push_back(i);
        }
        C3Vector minCoord,maxCoord;
        for (int i=0;i<int(pathPointsToTakeIntoAccount.size());i++)
        {
            CSimplePathPoint_old* aPt=pc->getSimplePathPoint(pathPointsToTakeIntoAccount[i]);
            C3Vector c(aPt->getTransformation().X);
            if (i==0)
            {
                minCoord=c;
                maxCoord=c;
            }
            else
            {
                minCoord.keepMin(c);
                maxCoord.keepMax(c);
            }
            localPositionOnPath+=c;
        }
        if (pathPointsToTakeIntoAccount.size()!=0)
            localPositionOnPath/=float(pathPointsToTakeIntoAccount.size());
        else
            return; // Should normally never happen
        sizeValueForPath=std::max<float>((maxCoord-minCoord).getLength()/3.0f,pc->getSquareSize()*2.0f);
    }

    C4X4Matrix tr;
    int axisInfo;
    if (_objectManipulationMode_flaggedForGridOverlay&8)
    { // rotation.
        axisInfo=_objectManipulationMode_flaggedForGridOverlay-8;
        tr=getCumulativeTransformation().getMatrix();

        if (getObjectMovementRelativity(1)==0) // world
            tr.M.setIdentity();
        if (getObjectMovementRelativity(1)==1) // parent frame
            tr.M=getFullParentCumulativeTransformation().getMatrix().M;
        if ((getObjectMovementRelativity(1)==2)||isPathPoints) // own frame
            tr.M=getCumulativeTransformation().getMatrix().M;
    }
    else
    { // translation
        axisInfo=_objectManipulationMode_flaggedForGridOverlay-16;
        tr=getCumulativeTransformation().getMatrix();
        if (getObjectMovementRelativity(0)==0) // world
            tr.M.setIdentity();
        if (getObjectMovementRelativity(0)==1) // parent frame
            tr.M=getFullParentCumulativeTransformation().getMatrix().M;
        if ((getObjectMovementRelativity(0)==2)||isPathPoints) // own frame
            tr.M=getCumulativeTransformation().getMatrix().M;
        if (isPathPoints)
            tr.X=tr*localPositionOnPath;
    }
    // Get the average bounding box size (or model bounding box size):
    C3Vector bbMin,bbMax;
    bbMin.clear();
    bbMax.clear();
    if (_modelBase)
    {
        C7Vector ctmi;
        if (_objectManipulationMode_flaggedForGridOverlay&8)
            ctmi=tr.getInverse();
        else
            ctmi=getCumulativeTransformation().getInverse(); // actually maybe tr.getInverse would even be better?

        bool b=true;
        if (!getGlobalMarkingBoundingBox(ctmi,bbMin,bbMax,b,true,true))
            bbMax=C3Vector(0.1f,0.1f,0.1f); // shouldn't happen!
    }
    else
        getBoundingBox(bbMin,bbMax);
    C3Vector bbs(bbMax-bbMin);

    float halfSize=0.0f;
    // add the average size of the bounding box (important for models)
    C3Vector bbsavg((bbMax+bbMin)*0.5f);
    if (_objectManipulationMode_flaggedForGridOverlay&8)
    { // rotation
        // set the orientation according to the rotation axis:
        C3X3Matrix rot;
        if (axisInfo==0)
        { // rotation around the x-axis
            rot.buildYRotation(piValD2_f);
            bbsavg(1)=0.0f;
            bbsavg(2)=0.0f;
            halfSize=1.5f*std::max<float>(bbs(1),bbs(2))/2.0f;
        }
        if (axisInfo==1)
        { // rotation around the y-axis
            rot.buildXRotation(-piValD2_f);
            bbsavg(0)=0.0f;
            bbsavg(2)=0.0f;
            halfSize=1.5f*std::max<float>(bbs(0),bbs(2))/2.0f;
        }
        if (axisInfo==2)
        { // rotation around the z-axis
            rot.setIdentity();
            bbsavg(0)=0.0f;
            bbsavg(1)=0.0f;
            halfSize=1.5f*std::max<float>(bbs(0),bbs(1))/2.0f;
        }

        if (isPath)
            halfSize=sizeValueForPath;

        // adjust the z-position of the rotation center:
        tr.X+=tr.M*bbsavg;
        // now adjust the orientation
        tr.M*=rot;
    }
    else
    {
        if (!isPath)
            halfSize=1.5f*(bbs(0)+bbs(1)+bbs(2))/6.0f;
        else
            halfSize=sizeValueForPath;

        tr.X+=getCumulativeTransformation().Q*bbsavg;
    }

    glPushMatrix();

    if (transparentAndOverlay)
    {
        glDisable(GL_DEPTH_TEST);
        ogl::setBlending(true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    }

    if (_objectManipulationMode_flaggedForGridOverlay&8)
    { // rotation
    // Do the OGL transformation:
        glTranslatef(tr.X(0),tr.X(1),tr.X(2));
        C3X3Matrix rrot;
        if (getObjectMovementRelativity(1)==2) // own frame
            rrot=tr.M;
        else
        {
            rrot.buildZRotation(_objectManipulationModeTotalRotation);
            rrot=tr.M*rrot;
        }
        C4Vector axis=rrot.getQuaternion().getAngleAndAxisNoChecking();
        glRotatef(axis(0)*radToDeg_f,axis(1),axis(2),axis(3));

        float a=5.0f*piValue_f/180.0f-_objectManipulationModeTotalRotation;
        float oldX=cos(-_objectManipulationModeTotalRotation);
        float oldY=sin(-_objectManipulationModeTotalRotation);
        float h=halfSize*0.9f;
        float ha=halfSize*0.95f;
        int cnt=1;

        // First the flat green circle:
        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::MANIPULATION_MODE_OVERLAY_COLOR);
        glPolygonOffset(0.5f,0.0f); // Second argument set to 0.0 on 2009.01.05 (otherwise strange effects on some graphic cards)
        glEnable(GL_POLYGON_OFFSET_FILL);
        glBegin(GL_QUADS);
        for (int i=0;i<72;i++)
        {
            glVertex3f(oldX*h,oldY*h,0.0f);
            glVertex3f(oldX*halfSize,oldY*halfSize,0.0f);
            oldX=cos(a);
            oldY=sin(a);
            a+=5.0f*piValue_f/180.0f;
            glVertex3f(oldX*halfSize,oldY*halfSize,0.0f);
            glVertex3f(oldX*h,oldY*h,0.0f);
        }
        glEnd();
        glDisable(GL_POLYGON_OFFSET_FILL);

        // Now the graduation:
        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::MANIPULATION_MODE_OVERLAY_GRID_COLOR);
        a=5.0f*piValue_f/180.0f-_objectManipulationModeTotalRotation;
        oldX=cos(-_objectManipulationModeTotalRotation);
        oldY=sin(-_objectManipulationModeTotalRotation);
        for (int i=0;i<72;i++)
        {
            glBegin(GL_LINE_STRIP);
            glVertex3f(oldX*halfSize,oldY*halfSize,0.0f);
            oldX=cos(a);
            oldY=sin(a);
            a+=5.0f*piValue_f/180.0f;
            glVertex3f(oldX*halfSize,oldY*halfSize,0.0f);
            if (cnt==0)
                glVertex3f(oldX*h,oldY*h,0.0f);
            else
                glVertex3f(oldX*ha,oldY*ha,0.0f);
            cnt++;
            if (cnt==3)
                cnt=0;
            glEnd();
        }
        // Now the moving part:
        glLineWidth(3.0f);
        float h2=halfSize*0.8f;
        ogl::drawSingle3dLine(-h,0.0f,0.0f,h,0.0f,0.0f,nullptr);
        ogl::drawSingle3dLine(h,0.0f,0.0f,cos(0.1f)*h2,sin(0.1f)*h2,0.0f,nullptr);
        ogl::drawSingle3dLine(h,0.0f,0.0f,cos(-0.1f)*h2,sin(-0.1f)*h2,0.0f,nullptr);
        std::string s(gv::getAngleStr(true,_objectManipulationModeTotalRotation));
        float h3=halfSize*1.1f;

        if (transparentAndOverlay)
            ogl::setBlending(false);
        ogl::drawBitmapTextTo3dPosition(h3,0.0f,halfSize*0.05f,s.c_str(),nullptr);
        if (transparentAndOverlay)
            ogl::setBlending(true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

        ogl::drawSingle3dLine(0.0f,-h,0.0f,0.0f,h,0.0f,nullptr);
        glLineWidth(1.0f);
    }
    else
    { // translation
        C3X3Matrix rot;
        bool xAxisOnly=false;
        if (axisInfo==0)
        { // y-z plane
            C3X3Matrix r1;
            r1.buildZRotation(piValD2_f);
            C3X3Matrix r2;
            r2.buildXRotation(piValD2_f);
            rot=r1*r2;
        }
        if (axisInfo==1)
            rot.buildXRotation(piValD2_f); // z-x plane
        if (axisInfo==2)
            rot.setIdentity(); // x-y plane
        if (axisInfo==3)
        { // x-axis
            rot.setIdentity();
            xAxisOnly=true;
        }
        if (axisInfo==4)
        { // y-axis
            rot.buildZRotation(-piValD2_f);
            xAxisOnly=true;
        }
        if (axisInfo==5)
        { // y-axis
            rot.buildYRotation(piValD2_f);
            xAxisOnly=true;
        }
        tr.M*=rot;
        C3Vector totTransl(rot.getTranspose()*_objectManipulationModeTotalTranslation);
        glTranslatef(tr.X(0),tr.X(1),tr.X(2));
        C4Vector axis=tr.M.getQuaternion().getAngleAndAxisNoChecking();
        glRotatef(axis(0)*radToDeg_f,axis(1),axis(2),axis(3));

        glTranslatef(-totTransl(0),-totTransl(1),-totTransl(2));
        C3Vector dir[2]={C3Vector::unitXVector,C3Vector::unitYVector};
        C3Vector perp[2]={C3Vector::unitYVector,C3Vector::unitXVector};
        float unt=0.001f;
        float h=halfSize*2.0f;
        if (h/unt>20)
            unt=0.002f;
        if (h/unt>20)
            unt=0.005f;
        if (h/unt>20)
            unt=0.01f;
        if (h/unt>20)
            unt=0.02f;
        if (h/unt>20)
            unt=0.05f;
        if (h/unt>20)
            unt=0.1f;
        if (h/unt>20)
            unt=0.2f;
        if (h/unt>20)
            unt=0.5f;

        int grdCnt=int(h/unt);
        C3Vector v;

        // First the green bands:
        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::MANIPULATION_MODE_OVERLAY_COLOR);
        glPolygonOffset(0.5f,0.0f); // Second argument set to 0.0 on 2009.01.05 (otherwise strange effects on some graphic cards)
        glEnable(GL_POLYGON_OFFSET_FILL);
        glBegin(GL_QUADS);
        for (int axis=0;axis<2;axis++)
        {
            v=dir[axis]*-unt*float(grdCnt+1);
            C3Vector w(perp[axis]*-unt*0.6f);
            glVertex3f(v(0)+w(0),v(1)+w(1),v(2)+w(2));
            glVertex3f(v(0)-w(0),v(1)-w(1),v(2)-w(2));
            glVertex3f(-v(0)-w(0),-v(1)-w(1),-v(2)-w(2));
            glVertex3f(-v(0)+w(0),-v(1)+w(1),-v(2)+w(2));
            if (xAxisOnly)
                break;
        }
        glEnd();
        glDisable(GL_POLYGON_OFFSET_FILL);

        // Now the graduation:
        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::MANIPULATION_MODE_OVERLAY_GRID_COLOR);
        ogl::buffer.clear();
        for (int axis=0;axis<2;axis++)
        {
            v=dir[axis]*-unt*float(grdCnt+1);
            ogl::addBuffer3DPoints(v(0),v(1),v(2));
            v*=-1.0f;
            ogl::addBuffer3DPoints(v(0),v(1),v(2));
            for (int i=-grdCnt;i<=grdCnt;i++)
            {
                C3Vector w(dir[axis]*-unt*float(i));
                v=perp[axis]*-unt*0.6f;
                ogl::addBuffer3DPoints(v(0)+w(0),v(1)+w(1),v(2)+w(2));
                v*=-1.0f;
                ogl::addBuffer3DPoints(v(0)+w(0),v(1)+w(1),v(2)+w(2));
            }
            if (xAxisOnly)
                break;
        }
        if (ogl::buffer.size()!=0)
            ogl::drawRandom3dLines(&ogl::buffer[0],(int)ogl::buffer.size()/3,false,nullptr);
        ogl::buffer.clear();

        // Now the moving part:
        glTranslatef(totTransl(0),totTransl(1),totTransl(2));

        ogl::setBlending(false);
        ogl::buffer.clear();
        for (int axis=0;axis<2;axis++)
        {
            C3Vector w,s;
            w=perp[axis]*-unt*0.8f;
            s=dir[axis]*unt*0.8f;
            v=dir[axis]*-unt*float(grdCnt+1);
            ogl::addBuffer3DPoints(v(0),v(1),v(2));
            v*=-1.0f;
            ogl::addBuffer3DPoints(v(0)+s(0),v(1)+s(1),v(2)+s(2));
            ogl::addBuffer3DPoints(v(0)+s(0),v(1)+s(1),v(2)+s(2));
            ogl::addBuffer3DPoints(v(0)+w(0),v(1)+w(1),v(2)+w(2));
            ogl::addBuffer3DPoints(v(0)+s(0),v(1)+s(1),v(2)+s(2));
            ogl::addBuffer3DPoints(v(0)-w(0),v(1)-w(1),v(2)-w(2));
            std::string st(gv::getSizeStr(true,totTransl(axis),0));
            ogl::drawBitmapTextTo3dPosition(v(0)+s(0)*2.0f+w(0)*2.0f,v(1)+s(1)*2.0f+w(1)*2.0f,v(2)+s(2)*2.0f+w(2)*2.0f,st.c_str(),nullptr);
            if (xAxisOnly)
                break;
        }
        if (transparentAndOverlay)
            ogl::setBlending(true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glLineWidth(3.0f);
        if (ogl::buffer.size()!=0)
            ogl::drawRandom3dLines(&ogl::buffer[0],(int)ogl::buffer.size()/3,false,nullptr);
        ogl::buffer.clear();
        glLineWidth(1.0f);
    }
    glPopMatrix();

    if (transparentAndOverlay)
    {
        glEnable(GL_DEPTH_TEST);
        ogl::setBlending(false);
    }
}

bool CSceneObject::setLocalTransformationFromObjectRotationMode(const C4X4Matrix& cameraAbsConf,float rotationAmount,bool perspective,int eventID)
{ // bits 0-2: position x,y,z (relative to parent frame), bits 3-5: Euler e9,e1,e2 (relative to own frame)
    if ( (!App::currentWorld->simulation->isSimulationStopped())&&(getObjectMovementOptions()&8))
    {
        _objectManipulationMode_flaggedForGridOverlay=0;
        _objectManipulationModeEventId=-1;
        return(false);
    }
    if (App::currentWorld->simulation->isSimulationStopped()&&(getObjectMovementOptions()&4))
    {
        _objectManipulationMode_flaggedForGridOverlay=0;
        _objectManipulationModeEventId=-1;
        return(false);
    }
    static int  otherAxisMemorized=0;
    bool ctrlKeyDown=((App::mainWindow!=nullptr)&&(App::mainWindow->getKeyDownState()&1))&&((_objectMovementOptions&32)==0);
    if ( (!ctrlKeyDown)&&((getObjectMovementPreferredAxes()&56)==0) )
    { // This is special so that, when no manip is allowed but we held down the ctrl key and released it, the green manip disc doesn't appear
        _objectManipulationModeAxisIndex=-1;
        _objectManipulationMode_flaggedForGridOverlay=0;
        _objectMovementPreferredAxesPreviousCtrlKeyDown=ctrlKeyDown;
        return(false);
    }
    if ( (eventID!=_objectManipulationModeEventId)||(ctrlKeyDown!=_objectMovementPreferredAxesPreviousCtrlKeyDown) )
    {
        if ( (otherAxisMemorized>1)&&((getObjectMovementPreferredAxes()&56)!=0) )
            otherAxisMemorized=0;
        if (otherAxisMemorized>2)
            otherAxisMemorized=0;
        _objectManipulationModeSubTranslation.clear();
        _objectManipulationModeSubRotation=0.0f;
        _objectManipulationModeEventId=eventID;
        _objectManipulationModeTotalTranslation.clear();
        _objectManipulationModeTotalRotation=0.0f;
        // Let's first see around which axis we wanna rotate:
        int _objectMovementPreferredAxesTEMP=getObjectMovementPreferredAxes();
        bool specialMode=false;
        if (ctrlKeyDown)
            specialMode=true;
        C4X4Matrix objAbs(getCumulativeTransformation().getMatrix());
        C3X3Matrix rotAxes;
        if (getObjectMovementRelativity(1)==2)
            rotAxes=objAbs.M; // own frame
        if (getObjectMovementRelativity(1)==1)
            rotAxes=getFullParentCumulativeTransformation().getMatrix().M; // parent frame
        if (getObjectMovementRelativity(1)==0)
            rotAxes.setIdentity(); // absolute frame

        float ml=0.0f;
        _objectManipulationModeAxisIndex=-1;
        for (int i=0;i<3;i++)
        {
            float l;
            if (perspective)
                l=(cameraAbsConf.X-objAbs.X) *rotAxes.axis[i];
            else
                l=cameraAbsConf.M.axis[2]*rotAxes.axis[i];
            if ( (fabs(l)>=ml)&&(_objectMovementPreferredAxesTEMP&(8<<i)) )
            {
                ml=fabs(l);
                _objectManipulationModeAxisIndex=i;
            }
        }
        if (specialMode)
        {
            int ax=_objectManipulationModeAxisIndex;
            if (_objectManipulationModeAxisIndex==-1)
                ax=otherAxisMemorized;
            if (_objectManipulationModeAxisIndex==0)
                ax+=1+otherAxisMemorized;
            if (_objectManipulationModeAxisIndex==1)
            {
                if (otherAxisMemorized==0)
                    ax=0;
                else
                    ax=2;
            }
            if (_objectManipulationModeAxisIndex==2)
                ax-=(1+otherAxisMemorized);
            _objectManipulationModeAxisIndex=ax;
            otherAxisMemorized++;
        }
    }

    _objectMovementPreferredAxesPreviousCtrlKeyDown=ctrlKeyDown;

    if (_objectManipulationModeAxisIndex==-1)
        return(false); //rotation not allowed

    float ss=getObjectMovementStepSize(1);
    if (ss==0.0f)
        ss=App::userSettings->getRotationStepSize();
    if ((App::mainWindow!=nullptr)&&(App::mainWindow->getKeyDownState()&2))
    {
        ss=0.1f*degToRad_f;
        rotationAmount/=5.0f;
    }
    float axisEffectiveRotationAmount=0.0f;
    _objectManipulationModeSubRotation+=rotationAmount;
    float w=fmod(_objectManipulationModeSubRotation,ss);
    axisEffectiveRotationAmount=_objectManipulationModeSubRotation-w;
    _objectManipulationModeTotalRotation+=axisEffectiveRotationAmount;
    _objectManipulationModeSubRotation=w;

    C3Vector euler;
    euler.clear();
    euler(_objectManipulationModeAxisIndex)=axisEffectiveRotationAmount;
    C4Vector rot(euler);
    C7Vector tr(_localTransformation);
    if (getObjectMovementRelativity(1)==2)
        tr.Q*=rot; // relative to own frame
    if (getObjectMovementRelativity(1)==1)
        tr.Q=rot*tr.Q; // relative to parent frame
    if (getObjectMovementRelativity(1)==0)
    { // relative to world frame
        C4Vector trq(getCumulativeTransformation().Q);
        trq=rot*trq;
        C4Vector pinv(getFullParentCumulativeTransformation().Q.getInverse());
        tr.Q=pinv*trq;
    }
    setLocalTransformation(tr);
    if (getDynamicFlag()>1) // for non-static shapes, and other objects that are in the dyn. world
        setDynamicsResetFlag(true,true);
    _objectManipulationMode_flaggedForGridOverlay=_objectManipulationModeAxisIndex+8;
    return(true);
}


bool CSceneObject::setLocalTransformationFromObjectTranslationMode(const C4X4Matrix& cameraAbsConf,const C3Vector& clicked3DPoint,float prevPos[2],float pos[2],float screenHalfSizes[2],float halfSizes[2],bool perspective,int eventID)
{ // bits 0-2: position x,y,z (relative to parent frame), bits 3-5: Euler e9,e1,e2 (relative to own frame)

    if ( (!App::currentWorld->simulation->isSimulationStopped())&&(getObjectMovementOptions()&2))
    {
        _objectManipulationMode_flaggedForGridOverlay=0;
        _objectManipulationModeEventId=-1;
        return(false);
    }
    if (App::currentWorld->simulation->isSimulationStopped()&&(getObjectMovementOptions()&1))
    {
        _objectManipulationMode_flaggedForGridOverlay=0;
        _objectManipulationModeEventId=-1;
        return(false);
    }

    C4X4Matrix objAbs;
    objAbs.X=getCumulativeTransformation().X;
    if (getObjectMovementRelativity(0)==0)
        objAbs.M.setIdentity();
    if (getObjectMovementRelativity(0)==1)
        objAbs.M=getFullParentCumulativeTransformation().getMatrix().M;
    if (getObjectMovementRelativity(0)==2)
        objAbs.M=getCumulativeTransformation().getMatrix().M;
    static int  otherAxisMemorized=0;
    bool ctrlKeyDown=((App::mainWindow!=nullptr)&&(App::mainWindow->getKeyDownState()&1))&&((_objectMovementOptions&16)==0);
    if ( (!ctrlKeyDown)&&((getObjectMovementPreferredAxes()&7)==0) )
    { // This is special so that, when no manip is allowed but we held down the ctrl key and released it, the green manip bars don't appear
        _objectManipulationModeAxisIndex=-1;
        _objectManipulationMode_flaggedForGridOverlay=0;
        _objectMovementPreferredAxesPreviousCtrlKeyDown=ctrlKeyDown;
        return(false);
    }
    if (eventID!=_objectManipulationModeEventId)
        _objectManipulationModeRelativePositionOfClickedPoint=clicked3DPoint-objAbs.X; // Added on 2010/07/29
    if ( (eventID!=_objectManipulationModeEventId)||(ctrlKeyDown!=_objectMovementPreferredAxesPreviousCtrlKeyDown) )
    {
        if (otherAxisMemorized>1)
            otherAxisMemorized=0;
        _objectManipulationModeSubTranslation.clear();
        _objectManipulationModeSubRotation=0.0f;
        _objectManipulationModeEventId=eventID;
        _objectManipulationModeTotalTranslation.clear();
        _objectManipulationModeTotalRotation=0.0f;
        // Let's first see on which plane we wanna translate:
        int _objectMovementPreferredAxesTEMP=getObjectMovementPreferredAxes();
        bool specialMode=false;
        bool specialMode2=false;
        if (ctrlKeyDown)
        {
            if ((_objectMovementPreferredAxesTEMP&7)!=7)
            {
                _objectMovementPreferredAxesTEMP^=7;
                specialMode2=((_objectMovementPreferredAxesTEMP&7)==7);
            }
            else
                specialMode=true;
        }
        float ml=0.0f;
        _objectManipulationModeAxisIndex=-1;
        unsigned char planeComb[3]={6,5,3};
        for (int i=0;i<3;i++)
        {
            float l;
            if (perspective)
                l=(cameraAbsConf.X-objAbs.X)*objAbs.M.axis[i];
            else
                l=cameraAbsConf.M.axis[2]*objAbs.M.axis[i];
            if ( (fabs(l)>=ml)&&((_objectMovementPreferredAxesTEMP&(planeComb[i]))==planeComb[i]) )
            {
                ml=fabs(l);
                _objectManipulationModeAxisIndex=i;
            }
        }
        if (_objectManipulationModeAxisIndex==-1)
        { // maybe we are constrained to stay on a line?
            for (int i=0;i<3;i++)
            {
                if (_objectMovementPreferredAxesTEMP&(1<<i))
                    _objectManipulationModeAxisIndex=3+i;
            }
        }
        else
        {
            if (specialMode)
                _objectManipulationModeAxisIndex+=3;
        }
        if (specialMode2)
        {
            if (otherAxisMemorized!=0)
                _objectManipulationModeAxisIndex+=3;
            otherAxisMemorized++;
        }
    }

    _objectMovementPreferredAxesPreviousCtrlKeyDown=ctrlKeyDown;

    if (_objectManipulationModeAxisIndex==-1)
        return(false); //rotation not allowed
    C4X4Matrix originalPlane(objAbs); // x-y plane
    originalPlane.X+=_objectManipulationModeRelativePositionOfClickedPoint; // Added on 2010/07/29
    if (_objectManipulationModeAxisIndex==0)
    { // y-z plane
        C3X3Matrix rot;
        rot.buildYRotation(piValD2_f);
        originalPlane.M*=rot;
    }
    if (_objectManipulationModeAxisIndex==1)
    { // z-x plane
        C3X3Matrix rot;
        rot.buildXRotation(piValD2_f);
        originalPlane.M*=rot;
    }
    bool projectOntoXAxis=false;
    if (_objectManipulationModeAxisIndex==3)
    { // x axis
        projectOntoXAxis=true;
    }
    if (_objectManipulationModeAxisIndex==4)
    { // y axis
        projectOntoXAxis=true;
        C3X3Matrix rot;
        rot.buildZRotation(piValD2_f);
        originalPlane.M*=rot;
    }
    if (_objectManipulationModeAxisIndex==5)
    { // z axis
        projectOntoXAxis=true;
        C3X3Matrix rot;
        rot.buildYRotation(piValD2_f);
        originalPlane.M*=rot;
    }

    C4X4Matrix plane(originalPlane);
    C3Vector p[2]; // previous and current point on the plane
    float d=-(plane.X*plane.M.axis[2]);
    float screenP[2]={prevPos[0],prevPos[1]};
    C4X4Matrix cam(cameraAbsConf);
    bool singularityProblem=false;

    for (int pass=0;pass<2;pass++)
    {
        float tt[2];
        for (int i=0;i<2;i++)
        {
            if (i==1)
            {
                screenP[0]=pos[0];
                screenP[1]=pos[1];
            }
            C3Vector pp(cam.X);
            if (!perspective)
            {
                if (fabs(cam.M.axis[2]*plane.M.axis[2])<0.05f)
                {
                    singularityProblem=true;
                    break;
                }
                pp-=cam.M.axis[0]*halfSizes[0]*(screenP[0]/screenHalfSizes[0]);
                pp+=cam.M.axis[1]*halfSizes[1]*(screenP[1]/screenHalfSizes[1]);
                float t=(-d-(plane.M.axis[2]*pp))/(cam.M.axis[2]*plane.M.axis[2]);
                p[i]=pp+cam.M.axis[2]*t;
            }
            else
            {
                C3Vector v(cam.M.axis[2]+cam.M.axis[0]*tan(-screenP[0])+cam.M.axis[1]*tan(screenP[1]));
                v.normalize();
                pp+=v;
                if (fabs(v*plane.M.axis[2])<0.05f)
                {
                    singularityProblem=true;
                    break;
                }
                float t=(-d-(plane.M.axis[2]*pp))/(v*plane.M.axis[2]);
                tt[i]=t;
                p[i]=pp+v*t;
            }
        }
        if (!singularityProblem)
        {
            if ((!perspective)||(tt[0]*tt[1]>0.0f))
                break;
            singularityProblem=true;
        }
        plane.M=cam.M;
    }
    if (projectOntoXAxis)
    {
        C4X4Matrix inv(originalPlane.getInverse());
        p[0]*=inv;
        p[1]*=inv;
        p[0](1)=0.0f;
        p[0](2)=0.0f;
        p[1](1)=0.0f;
        p[1](2)=0.0f;
        p[0]*=originalPlane;
        p[1]*=originalPlane;
    }
    else
    {
        if (singularityProblem)
        { // we have to project the coordinates onto the original plane:
            C4X4Matrix inv(originalPlane.getInverse());
            p[0]*=inv;
            p[1]*=inv;
            p[0](2)=0.0f;
            p[1](2)=0.0f;
            p[0]*=originalPlane;
            p[1]*=originalPlane;
        }
    }

    // We snap the translation!
    C3Vector v(p[1]-p[0]);
    v=objAbs.getInverse().M*v;
    _objectManipulationModeSubTranslation+=v;
    for (int i=0;i<3;i++)
    {
        float ss=getObjectMovementStepSize(0);
        if (ss==0.0f)
            ss=App::userSettings->getTranslationStepSize();
        if ((App::mainWindow!=nullptr)&&(App::mainWindow->getKeyDownState()&2))
            ss=0.001f;
        float w=fmod(_objectManipulationModeSubTranslation(i),ss);
        v(i)=_objectManipulationModeSubTranslation(i)-w;
        _objectManipulationModeTotalTranslation(i)+=_objectManipulationModeSubTranslation(i)-w;
        _objectManipulationModeSubTranslation(i)=w;
    }
    v=objAbs.M*v;
    C4X4Matrix m(getCumulativeTransformation());
    m.X+=v;
    setLocalTransformation(getFullParentCumulativeTransformation().getInverse().getMatrix()*m);
    if (getDynamicFlag()>1) // for non-static shapes, and other objects that are in the dyn. world
        setDynamicsResetFlag(true,true);
    _objectManipulationMode_flaggedForGridOverlay=_objectManipulationModeAxisIndex+16;
    return(true);
}
#endif

void CSceneObject::buildUpdateAndPopulateSynchronizationObject(const std::vector<SSyncRoute>* parentRouting)
{ // Overridden from CSyncObject
    if (setObjectCanSync(true))
    {
        // Routing normally already done, and remote object already built (in the derived class, if implemented)
        if (!isRoutingSet())
        { // If not, do it here (and build a dummy)
            // Set routing:
            SSyncRoute r;
            r.objHandle=_objectHandle;
            r.objType=sim_syncobj_dummy;
            setSyncMsgRouting(parentRouting,r);
        }

        // Build IK plugin counterpart, if not a joint:
        if (_ikPluginCounterpartHandle==-1)
            _ikPluginCounterpartHandle=CPluginContainer::ikPlugin_createDummy();
        // Update the remote object:
        _setLocalTransformation_send(_localTransformation);
    }
}

void CSceneObject::connectSynchronizationObject()
{ // Overridden from CSyncObject
    if (getObjectCanSync())
    {
        int h=-1;
        if (getParent()!=nullptr)
            h=getParent()->getObjectHandle();
        _setParent_send(h);
    }
}

void CSceneObject::removeSynchronizationObject(bool localReferencesToItOnly)
{ // Overridden from CSyncObject
    if (getObjectCanSync())
    {
        setObjectCanSync(false);

        if (!localReferencesToItOnly)
        {
            // Synchronize with IK plugin:
            if (_ikPluginCounterpartHandle!=-1)
                CPluginContainer::ikPlugin_eraseObject(_ikPluginCounterpartHandle);
        }
    }
    // IK plugin part:
    _ikPluginCounterpartHandle=-1;
}

void CSceneObject::setExtensionString(const char* str)
{
    _extensionString=str;
}

void CSceneObject::setVisibilityLayer(unsigned short l)
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
}

void CSceneObject::setChildOrder(int order)
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
}


void CSceneObject::setObjectHandle(int newObjectHandle)
{
    _objectHandle=newObjectHandle;
}

void CSceneObject::setObjectAlias_direct(const char* newName)
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
}

void CSceneObject::setObjectName_direct_old(const char* newName)
{
    _objectName_old=newName;
}

void CSceneObject::setObjectAltName_direct_old(const char* newAltName)
{
    _objectAltName_old=newAltName;
}

void CSceneObject::setLocalTransformation(const C7Vector& tr)
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
}

void CSceneObject::setLocalTransformation(const C4Vector& q)
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
}

void CSceneObject::setLocalTransformation(const C3Vector& x)
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
}

size_t CSceneObject::getChildCount() const
{
    return(_childList.size());
}

CSceneObject* CSceneObject::getChildFromIndex(size_t index) const
{
    CSceneObject* retVal=nullptr;
    if (index<_childList.size())
        retVal=_childList[index];
    return(retVal);
}

const std::vector<CSceneObject*>* CSceneObject::getChildren() const
{
    return(&_childList);
}

void CSceneObject::addChild(CSceneObject* child)
{
    if (child==nullptr)
        _childList.clear();
    else
    {
        _childList.push_back(child);
        handleOrderIndexOfChildren();
    }
}

bool CSceneObject::removeChild(const CSceneObject* child)
{
    bool retVal=false;
    for (size_t i=0;i<_childList.size();i++)
    {
        if (_childList[i]==child)
        {
            _childList.erase(_childList.begin()+i);
            retVal=true;
            break;
        }
    }
    if (retVal)
        handleOrderIndexOfChildren();
    return(retVal);
}

void CSceneObject::handleOrderIndexOfChildren()
{
    std::map<std::string,int> nameMap;
    std::vector<int> co(_childList.size());
    for (size_t i=0;i<_childList.size();i++)
    {
        CSceneObject* child=_childList[i];
        std::string hn(child->getObjectAlias());
        std::map<std::string,int>::iterator it=nameMap.find(hn);
        if (it==nameMap.end())
            nameMap[hn]=0;
        else
            nameMap[hn]++;
        co[i]=nameMap[hn];
    }
    for (size_t i=0;i<_childList.size();i++)
    {
        CSceneObject* child=_childList[i];
        std::string hn(child->getObjectAlias());
        std::map<std::string,int>::iterator it=nameMap.find(hn);
        if (nameMap[hn]==0)
            co[i]=-1; // means unique with that name, with that parent
        child->setChildOrder(co[i]);
    }
}

int CSceneObject::getChildSequence(const CSceneObject* child) const
{
    for (size_t i=0;i<_childList.size();i++)
    {
        if (_childList[i]==child)
            return(int(i));
    }
    return(-1);
}

bool CSceneObject::setChildSequence(CSceneObject* child,int order)
{
    order=std::min<int>(int(_childList.size())-1,order);
    if (order<0)
        order=int(_childList.size())-1; // neg. value: put in last position
    for (size_t i=0;i<_childList.size();i++)
    {
        if (_childList[i]==child)
        {
            if (order!=i)
            {
                _childList.erase(_childList.begin()+i);
                _childList.insert(_childList.begin()+order,child);
                handleOrderIndexOfChildren();
                return(true);
            }
            break;
        }
    }
    return(false);
}


