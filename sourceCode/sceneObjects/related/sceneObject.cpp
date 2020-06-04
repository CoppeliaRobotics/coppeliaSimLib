#include "simInternal.h"
#include "tt.h"
#include "shape.h"
#include "camera.h"
#include "graph.h"
#include "path.h"
#include "customData.h"
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

int CSceneObject::_modelPropertyValidityNumber=0;

CSceneObject::CSceneObject()
{
    _parentObjectHandle_forSerializationOnly=-1;
    _objectHandle=-1;
    _ikPluginCounterpartHandle=-1;
    generateDnaString();
    _assemblingLocalTransformation.setIdentity();
    _assemblingLocalTransformationIsUsed=false;
    _userScriptParameters=nullptr;

    _authorizedViewableObjects=-1; // all
    _assemblyMatchValuesChild.push_back("default");
    _assemblyMatchValuesParent.push_back("default");
    _forceAlwaysVisible_tmp=false;

    _localObjectProperty=sim_objectproperty_selectable;
    _hierarchyColorIndex=-1;
    _collectionSelfCollisionIndicator=0;
    _ignorePosAndCameraOrthoviewSize_forUndoRedo=false;

    _modelBase=false;
    _dynamicObjectFlag_forVisualization=0;

    _transparentObjectDistanceOffset=0;


    _measuredAngularVelocity_velocityMeasurement=0.0f;
    _measuredLinearVelocity_velocityMeasurement.clear();
    _measuredAngularVelocity3_velocityMeasurement.clear();
    _measuredAngularVelocityAxis_velocityMeasurement.clear();
    _previousPositionOrientationIsValid=false;


    _dynamicsTemporarilyDisabled=false;
    _initialValuesInitializedMain=false;
    _initialConfigurationMemorized=false;
    _objectTranslationDisabledDuringSimulation=false;
    _objectTranslationDisabledDuringNonSimulation=false;
    _objectTranslationSettingsLocked=false;
    _objectRotationDisabledDuringSimulation=false;
    _objectRotationDisabledDuringNonSimulation=false;
    _objectRotationSettingsLocked=false;
    _objectManipulationModePermissions=0x023; // about Z and in the X-Y plane!       0x03f; // full
    _objectManipulationModeEventId=-1;
    _objectManipulationTranslationRelativeTo=0; // relative to world by default
    _objectTranslationNonDefaultStepSize=0.0f; // 0.0 means: use the default
    _objectManipulationRotationRelativeTo=2; // relative to own frame by default
    _objectRotationNonDefaultStepSize=0.0f; // 0.0 means: use the default

    _customObjectData=nullptr;
    _customObjectData_tempData=nullptr;
    _localObjectSpecialProperty=0;
    _localModelProperty=0; // By default, the main properties are not overriden! (0 means we inherit from parents)
    _cumulativeModelProperty=0;
    _cumulativeModelPropertyValidityNumber=-2;

    _memorizedConfigurationValidCounter=0;

    _dynamicSimulationIconCode=sim_dynamicsimicon_none;

    _uniqueID=_uniqueIDCounter++; // not persistent
    _uniquePersistentIdString=CTTUtil::generateUniqueReadableString(); // persistent
    _modelAcknowledgement="";
    _objectTempName="__object__";

    _specificLight=-1; // default, i.e. all lights

    _sizeFactor=1.0f;
    _sizeValues[0]=1.0f;
    _sizeValues[1]=1.0f;
    _sizeValues[2]=1.0f;
    _dynamicsFullRefreshFlag=true;
    _ignoredByViewFitting=false;
}

CSceneObject::~CSceneObject() 
{
    delete _customObjectData;
    delete _customObjectData_tempData;
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
        _measuredAngularVelocity3_velocityMeasurement=(abs*_previousAbsTransf_velocityMeasurement.getInverse()).Q.getEulerAngles()*(1.0f/dt);

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
    c=tt::getLimitedInt(0,99999999,c);
    _collectionSelfCollisionIndicator=c;
}

int CSceneObject::getCollectionSelfCollisionIndicator() const
{
    return(_collectionSelfCollisionIndicator);
}

void CSceneObject::setDynamicObjectFlag_forVisualization(int isDynamicallySimulatedFlag)
{
    _dynamicObjectFlag_forVisualization=isDynamicallySimulatedFlag;
}

int CSceneObject::getDynamicObjectFlag_forVisualization() const
{
    return(_dynamicObjectFlag_forVisualization);
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
    if (((displayAttrib&sim_displayattribute_pickpass)==0)||((_localObjectProperty&sim_objectproperty_selectinvisible)==0))
    { // ok, no pickpass and select invisible
        if (displayAttrib&sim_displayattribute_dynamiccontentonly)
            display=(_dynamicObjectFlag_forVisualization!=0);
        else
        {
            display=( (!isObjectPartOfInvisibleModel())&&((App::currentWorld->mainSettings->getActiveLayers()&getVisibilityLayer())||(displayAttrib&sim_displayattribute_ignorelayer)) );

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

void CSceneObject::setIgnoredByViewFitting(bool ignored)
{
    _ignoredByViewFitting=ignored;
}

bool CSceneObject::getIgnoredByViewFitting() const
{
    return(_ignoredByViewFitting);
}

void CSceneObject::setModelAcknowledgement(const std::string& a)
{
    _modelAcknowledgement=a;
    if (_modelAcknowledgement.length()>3000)
        _modelAcknowledgement.erase(_modelAcknowledgement.begin()+2999,_modelAcknowledgement.end());
}

std::string CSceneObject::getModelAcknowledgement() const
{
    return(_modelAcknowledgement);
}

void CSceneObject::setDynamicsFullRefreshFlag(bool refresh)
{
    _dynamicsFullRefreshFlag=refresh;
    if (refresh)
    {
        if (_objectType==sim_object_joint_type)
            ((CJoint*)this)->setForceOrTorqueNotValid();
        if (_objectType==sim_object_forcesensor_type)
            ((CForceSensor*)this)->setForceAndTorqueNotValid();
    }
}

bool CSceneObject::getDynamicsFullRefreshFlag() const
{
    return(_dynamicsFullRefreshFlag);
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

int CSceneObject::getUniqueID() const
{
    return(_uniqueID);
}

int CSceneObject::_getAllowedObjectSpecialProperties() const
{
    int retVal=0;
    if (isPotentiallyCollidable())
        retVal|=sim_objectspecialproperty_collidable;
    if (isPotentiallyMeasurable())
        retVal|=sim_objectspecialproperty_measurable;
    if (isPotentiallyDetectable())
        retVal|=sim_objectspecialproperty_detectable_all;
    if (isPotentiallyRenderable())
        retVal|=sim_objectspecialproperty_renderable;
    return(retVal);
}

void CSceneObject::incrementMemorizedConfigurationValidCounter()
{
    _memorizedConfigurationValidCounter++;
}

int CSceneObject::getMemorizedConfigurationValidCounter()
{
    return(_memorizedConfigurationValidCounter);
}

bool CSceneObject::getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const
{
    return(false);
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
    _modelBase=m;
    _localModelProperty=0; // Nothing is overridden!
    _modelAcknowledgement="";
    incrementModelPropertyValidityNumber();
}

bool CSceneObject::getModelBase() const
{
    return(_modelBase);
}

void CSceneObject::setLocalObjectProperty(int p)
{
    _localObjectProperty=p;
}

int CSceneObject::getLocalObjectProperty() const
{
    return(_localObjectProperty);
}

int CSceneObject::getCumulativeObjectProperty()
{
    int o=getCumulativeModelProperty();
    if (o==0)
        return(_localObjectProperty); // nothing is overridden!
    int p=_localObjectProperty;

    if (o&sim_modelproperty_not_showasinsidemodel)
        p=(p|sim_objectproperty_dontshowasinsidemodel);

    return(p);
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
        p=(p|sim_objectspecialproperty_detectable_all)-sim_objectspecialproperty_detectable_all;

    return(p);
}


void CSceneObject::setLocalModelProperty(int prop)
{ // model properties are actually override properties. This func. returns the local value
    _localModelProperty=prop;
    incrementModelPropertyValidityNumber();
}

int CSceneObject::getLocalModelProperty() const
{ // model properties are actually override properties. This func. returns the local value
    return(_localModelProperty);
}

int CSceneObject::getCumulativeModelProperty()
{ // model properties are actually override properties. This func. returns the cumulative value
    int vn=_modelPropertyValidityNumber;
    if (vn!=_cumulativeModelPropertyValidityNumber)
    { // the cumulative value is not up-to-date
        if (getParent()==nullptr)
            _cumulativeModelProperty=_localModelProperty;
        else
        {
            int parentCumul=getParent()->getCumulativeModelProperty();
            _cumulativeModelProperty=_localModelProperty|parentCumul;
        }
        if (_dynamicsTemporarilyDisabled)
            _cumulativeModelProperty|=sim_modelproperty_not_dynamic;
        _cumulativeModelPropertyValidityNumber=vn;
    }
    return(_cumulativeModelProperty);
}

void CSceneObject::incrementModelPropertyValidityNumber()
{ // static
    _modelPropertyValidityNumber++;
}


bool CSceneObject::isObjectPartOfInvisibleModel()
{
    return((getCumulativeModelProperty()&sim_modelproperty_not_visible)!=0);
}

int CSceneObject::getTreeDynamicProperty() // combination of sim_objdynprop_dynamic and sim_objdynprop_respondable
{ // returns the total
    int o=getCumulativeModelProperty();
    if (o==0)
        return(sim_objdynprop_dynamic|sim_objdynprop_respondable); // nothing is overridden!
    int ret=0;
    if ((o&sim_modelproperty_not_dynamic)==0)
        ret|=sim_objdynprop_dynamic;
    if ((o&sim_modelproperty_not_respondable)==0)
        ret|=sim_objdynprop_respondable;
    return(ret);
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
        if ( ((_localObjectProperty&sim_objectproperty_selectmodelbaseinstead)==0) )
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
    incrementMemorizedConfigurationValidCounter();
    App::currentWorld->drawingCont->adjustForScaling(_objectHandle,scalingFactor,scalingFactor,scalingFactor);
    App::worldContainer->setModificationFlag(256); // object scaled
}

void CSceneObject::scaleObjectNonIsometrically(float x,float y,float z)
{
    _sizeFactor*=cbrt(x*y*z);
    _sizeValues[0]*=x;
    _sizeValues[1]*=y;
    _sizeValues[2]*=z;
    incrementMemorizedConfigurationValidCounter();
    App::currentWorld->drawingCont->adjustForScaling(_objectHandle,x,y,z);
    App::worldContainer->setModificationFlag(256); // object scaled
}

void CSceneObject::scalePosition(float scalingFactor) 
{ // This routine will scale an object's position. The object itself keeps the same size.
    C7Vector local(getLocalTransformation());
    setLocalTransformation(local.X*scalingFactor);
    _assemblingLocalTransformation.X=_assemblingLocalTransformation.X*scalingFactor;
}

void CSceneObject::setSpecificLight(int h)
{
    _specificLight=h;
}

int CSceneObject::getSpecificLight() const
{
    return(_specificLight);
}

bool CSceneObject::getGlobalMarkingBoundingBox(const C7Vector& baseCoordInv,C3Vector& min,C3Vector& max,bool& minMaxNotYetDefined,bool first,bool guiIsRendering) const
{ // For root selection display! Return value false means there is no global marking bounding box and min/max values are not valid
    bool retVal=false;
    int objProp=getLocalObjectProperty();
    int modProp=getLocalModelProperty();

    bool exploreChildren=((modProp&sim_modelproperty_not_showasinsidemodel)==0)||first;
    bool includeThisBox=(objProp&sim_objectproperty_dontshowasinsidemodel)==0;
    first=false;

    if (includeThisBox&&exploreChildren)
    {
        C3Vector smi,sma;
        if (getMarkingBoundingBox(smi,sma))
        {
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

void CSceneObject::getBoundingBoxEncompassingBoundingBox(const C7Vector& baseCoordInv,C3Vector& min,C3Vector& max,bool guiIsRendering) const
{
    C3Vector smi,sma;
    getFullBoundingBox(smi,sma);
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

void CSceneObject::setObjectType(int theNewType)
{ // Be very careful with this function!!
    _objectType=theNewType;
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
        theNewObject=new CPath();
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
    theNewObject->_localTransformation=_localTransformation;
    theNewObject->_objectName=_objectName;
    theNewObject->_objectAltName=_objectAltName;
    theNewObject->_localObjectProperty=_localObjectProperty;
    theNewObject->_hierarchyColorIndex=_hierarchyColorIndex;
    theNewObject->_collectionSelfCollisionIndicator=_collectionSelfCollisionIndicator;
    theNewObject->_modelBase=_modelBase;
    theNewObject->_objectType=_objectType;
    theNewObject->_localObjectSpecialProperty=_localObjectSpecialProperty;
    theNewObject->_localModelProperty=_localModelProperty;
    theNewObject->_extensionString=_extensionString;

    if (_localObjectProperty&sim_objectproperty_canupdatedna)
        theNewObject->_dnaString=_dnaString;

    theNewObject->_assemblingLocalTransformation=_assemblingLocalTransformation;
    theNewObject->_assemblingLocalTransformationIsUsed=_assemblingLocalTransformationIsUsed;

    theNewObject->_assemblyMatchValuesChild.assign(_assemblyMatchValuesChild.begin(),_assemblyMatchValuesChild.end());
    theNewObject->_assemblyMatchValuesParent.assign(_assemblyMatchValuesParent.begin(),_assemblyMatchValuesParent.end());
    theNewObject->_objectTranslationDisabledDuringSimulation=_objectTranslationDisabledDuringSimulation;
    theNewObject->_objectTranslationDisabledDuringNonSimulation=_objectTranslationDisabledDuringNonSimulation;
    theNewObject->_objectTranslationSettingsLocked=_objectTranslationSettingsLocked;
    theNewObject->_objectRotationDisabledDuringSimulation=_objectRotationDisabledDuringSimulation;
    theNewObject->_objectRotationDisabledDuringNonSimulation=_objectRotationDisabledDuringNonSimulation;
    theNewObject->_objectRotationSettingsLocked=_objectRotationSettingsLocked;
    theNewObject->_objectManipulationModePermissions=_objectManipulationModePermissions;
    theNewObject->_objectManipulationTranslationRelativeTo=_objectManipulationTranslationRelativeTo;
    theNewObject->_objectManipulationRotationRelativeTo=_objectManipulationRotationRelativeTo;
    theNewObject->_objectTranslationNonDefaultStepSize=_objectTranslationNonDefaultStepSize;
    theNewObject->_objectRotationNonDefaultStepSize=_objectRotationNonDefaultStepSize;

    theNewObject->_sizeFactor=_sizeFactor;
    theNewObject->_sizeValues[0]=_sizeValues[0];
    theNewObject->_sizeValues[1]=_sizeValues[1];
    theNewObject->_sizeValues[2]=_sizeValues[2];
    theNewObject->_modelAcknowledgement=_modelAcknowledgement;
    theNewObject->_ignoredByViewFitting=_ignoredByViewFitting;
    theNewObject->_transparentObjectDistanceOffset=_transparentObjectDistanceOffset;

    delete theNewObject->_customObjectData;
    theNewObject->_customObjectData=nullptr;
    if (_customObjectData!=nullptr)
        theNewObject->_customObjectData=_customObjectData->copyYourself();

    delete theNewObject->_customObjectData_tempData;
    theNewObject->_customObjectData_tempData=nullptr;
    if (_customObjectData_tempData!=nullptr)
        theNewObject->_customObjectData_tempData=_customObjectData_tempData->copyYourself();

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

void CSceneObject::setObjectCustomData(int header,const char* data,int dataLength)
{
    if (_customObjectData==nullptr)
        _customObjectData=new CCustomData();
    _customObjectData->setData(header,data,dataLength);
}

int CSceneObject::getObjectCustomDataLength(int header) const
{
    if (_customObjectData==nullptr)
        return(0);
    return(_customObjectData->getDataLength(header));
}

void CSceneObject::getObjectCustomData(int header,char* data) const
{
    if (_customObjectData==nullptr)
        return;
    _customObjectData->getData(header,data);
}

bool CSceneObject::getObjectCustomDataHeader(int index,int& header) const
{
    if (_customObjectData==nullptr)
        return(false);
    return(_customObjectData->getHeader(index,header));
}

void CSceneObject::setObjectCustomData_tempData(int header,const char* data,int dataLength)
{
    if (_customObjectData_tempData==nullptr)
        _customObjectData_tempData=new CCustomData();
    _customObjectData_tempData->setData(header,data,dataLength);
}

int CSceneObject::getObjectCustomDataLength_tempData(int header) const
{
    if (_customObjectData_tempData==nullptr)
        return(0);
    return(_customObjectData_tempData->getDataLength(header));
}

void CSceneObject::getObjectCustomData_tempData(int header,char* data) const
{
    if (_customObjectData_tempData==nullptr)
        return;
    _customObjectData_tempData->getData(header,data);
}

bool CSceneObject::getObjectCustomDataHeader_tempData(int index,int& header) const
{
    if (_customObjectData_tempData==nullptr)
        return(false);
    return(_customObjectData_tempData->getHeader(index,header));
}

void CSceneObject::setObjectTranslationDisabledDuringSimulation(bool d)
{
    _objectTranslationDisabledDuringSimulation=d;
}

bool CSceneObject::getObjectTranslationDisabledDuringSimulation() const
{
    return(_objectTranslationDisabledDuringSimulation);
}

void CSceneObject::setObjectTranslationDisabledDuringNonSimulation(bool d)
{
    _objectTranslationDisabledDuringNonSimulation=d;
}

bool CSceneObject::getObjectTranslationDisabledDuringNonSimulation() const
{
    return(_objectTranslationDisabledDuringNonSimulation);
}

void CSceneObject::setObjectTranslationSettingsLocked(bool l)
{
    _objectTranslationSettingsLocked=l;
}

bool CSceneObject::getObjectTranslationSettingsLocked() const
{
    return(_objectTranslationSettingsLocked);
}

void CSceneObject::setObjectRotationDisabledDuringSimulation(bool d)
{
    _objectRotationDisabledDuringSimulation=d;
}

bool CSceneObject::getObjectRotationDisabledDuringSimulation() const
{
    return(_objectRotationDisabledDuringSimulation);
}

void CSceneObject::setObjectRotationDisabledDuringNonSimulation(bool d)
{
    _objectRotationDisabledDuringNonSimulation=d;
}

bool CSceneObject::getObjectRotationDisabledDuringNonSimulation() const
{
    return(_objectRotationDisabledDuringNonSimulation);
}

void CSceneObject::setObjectRotationSettingsLocked(bool l)
{
    _objectRotationSettingsLocked=l;
}

bool CSceneObject::getObjectRotationSettingsLocked() const
{
    return(_objectRotationSettingsLocked);
}


void CSceneObject::setObjectManipulationModePermissions(int p)
{ // bits 0-2: position x,y,z (relative to parent frame), bits 3-5: Euler e9,e1,e2 (relative to own frame) 
    p&=0x3f;
    _objectManipulationModePermissions=p;
}

int CSceneObject::getObjectManipulationModePermissions() const
{ // bits 0-2: position x,y,z (relative to parent frame), bits 3-5: Euler e9,e1,e2 (relative to own frame) 
    return(_objectManipulationModePermissions);
}

void CSceneObject::setObjectManipulationTranslationRelativeTo(int p)
{
    _objectManipulationTranslationRelativeTo=p;
}

int CSceneObject::getObjectManipulationTranslationRelativeTo() const
{
    return(_objectManipulationTranslationRelativeTo);
}

void CSceneObject::setObjectManipulationRotationRelativeTo(int p)
{
    _objectManipulationRotationRelativeTo=p;
}

int CSceneObject::getObjectManipulationRotationRelativeTo() const
{
    return(_objectManipulationRotationRelativeTo);
}

void CSceneObject::setNonDefaultTranslationStepSize(float s)
{
    if (s<0.0005)
    {
        _objectTranslationNonDefaultStepSize=0.0f; //default
        return;
    }
    float sc=1.0f;
    if ((s>=0.0075f)&&(s<0.075f))
        sc=10.0f;
    if (s>=0.075f)
        sc=100.0f;
    if (s<0.0015f*sc)
        s=0.001f*sc;
    else
    {
        if (s<0.00375f*sc)
        {
            if (sc<2.0f)
                s=0.002f*sc;
            else
                s=0.0025f*sc;
        }
        else
            s=0.005f*sc;
    }
    _objectTranslationNonDefaultStepSize=s;
}

float CSceneObject::getNonDefaultTranslationStepSize() const
{
    return(_objectTranslationNonDefaultStepSize);
}

void CSceneObject::setNonDefaultRotationStepSize(float s)
{
    if (s<0.05f*degToRad_f)
    {
        _objectRotationNonDefaultStepSize=0.0f; //default
        return;
    }
    if (s<1.5f*degToRad_f)
        s=1.0f*degToRad_f;
    else
    {
        if (s<3.5f*degToRad_f)
            s=2.0f*degToRad_f;
        else
        {
            if (s<7.5f*degToRad_f)
                s=5.0f*degToRad_f;
            else
            {
                if (s<12.5f*degToRad_f)
                    s=10.0f*degToRad_f;
                else
                {
                    if (s<22.5f*degToRad_f)
                        s=15.0f*degToRad_f;
                    else
                    {
                        if (s<37.5f*degToRad_f)
                            s=30.0f*degToRad_f;
                        else
                            s=45.0f*degToRad_f;
                    }
                }
            }
        }
    }
    _objectRotationNonDefaultStepSize=s;
}

float CSceneObject::getNonDefaultRotationStepSize() const
{
    return(_objectRotationNonDefaultStepSize);
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

void CSceneObject::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    _dynamicSimulationIconCode=sim_dynamicsimicon_none;
    _initialValuesInitializedMain=simulationIsRunning;
    _localModelProperty=(_localModelProperty|sim_modelproperty_not_reset)-sim_modelproperty_not_reset;
    incrementModelPropertyValidityNumber();
    _dynamicObjectFlag_forVisualization=0;

    _measuredAngularVelocity_velocityMeasurement=0.0f;
    _measuredAngularVelocity3_velocityMeasurement.clear();
    _measuredAngularVelocityAxis_velocityMeasurement.clear();
    _measuredLinearVelocity_velocityMeasurement.clear();
    _previousPositionOrientationIsValid=false;
    if (_userScriptParameters!=nullptr)
        _userScriptParameters->initializeInitialValues(simulationIsRunning);
//    _previousAbsTransf_velocityMeasurement=getCumulativeTransformationPart1();

    if (simulationIsRunning)
    {
        // this section is special and reserved to local configuration restoration!
        //********************************
        _initialConfigurationMemorized=true;
        _initialMemorizedConfigurationValidCounter=_memorizedConfigurationValidCounter;
        _initialParentUniqueID=-1; // -1 means there was no parent at begin
        CSceneObject* p=getParent();
        if (p!=nullptr)
            _initialParentUniqueID=p->getUniqueID();
        _initialLocalTransformationPart1=_localTransformation;
        //********************************

        _initialMainPropertyOverride=_localModelProperty;
    }
}

void CSceneObject::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    _dynamicSimulationIconCode=sim_dynamicsimicon_none;
    _dynamicObjectFlag_forVisualization=0;
    if (_userScriptParameters!=nullptr)
        _userScriptParameters->simulationEnded();
    if (_initialValuesInitializedMain&&App::currentWorld->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
    {
        if (_initialConfigurationMemorized)
        { // this section is special and reserved to local configuration restoration!
            if (_initialMemorizedConfigurationValidCounter==_memorizedConfigurationValidCounter)
            { // the object wasn't resized/didn't change frame
                int puid=-1;
                CSceneObject* p=getParent();
                if (p!=nullptr)
                    puid=p->getUniqueID();
                // Changed following on 24/04/2011 (because we also wanna reset the parenting to the initial state!)
                if (puid!=_initialParentUniqueID)
                { // Not sure following instructions are not problematic here.
                    CSceneObject* oldParent=App::currentWorld->sceneObjects->getObjectFromUniqueId(_initialParentUniqueID);
                    if ( (oldParent!=nullptr)||(_initialParentUniqueID==-1) )
                    {
                        // Inverted following 2 lines on 24/2/2012:
                        setParent(oldParent,true);
                        setLocalTransformation(_initialLocalTransformationPart1);
                    }
                }
                else
                    setLocalTransformation(_initialLocalTransformationPart1);
            }
            _localModelProperty=_initialMainPropertyOverride;
            _initialConfigurationMemorized=false;
        }
    }
    _initialValuesInitializedMain=false;
}

bool CSceneObject::getFullBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    return(false);
}

bool CSceneObject::getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    return(getFullBoundingBox(minV,maxV));
}

void CSceneObject::disableDynamicTreeForManipulation(bool d)
{
    if (d!=_dynamicsTemporarilyDisabled)
        incrementModelPropertyValidityNumber(); // we want the cumulative values all recalculated
    _dynamicsTemporarilyDisabled=d;
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


void CSceneObject::setEnableCustomizationScript(bool c,const char* scriptContent)
{
    // _customizationScriptEnabled=c;

    // We remove a script that might be associated:
    CLuaScriptObject* script=App::currentWorld->luaScriptContainer->getScriptFromObjectAttachedTo_customization(getObjectHandle());
    if (script)
    {
#ifdef SIM_WITH_GUI
        if (App::mainWindow!=nullptr)
            App::mainWindow->codeEditorContainer->closeFromScriptHandle(script->getScriptID(),nullptr,true);
#endif
        App::currentWorld->luaScriptContainer->removeScript(script->getScriptID());
    }

    if (c)
    { // we have to add a script
        CLuaScriptObject* script=new CLuaScriptObject(sim_scripttype_customizationscript);
        if (scriptContent)
            script->setScriptText(scriptContent);
        App::currentWorld->luaScriptContainer->insertScript(script);
        script->setObjectIDThatScriptIsAttachedTo(getObjectHandle());
    }
}

bool CSceneObject::getEnableCustomizationScript()
{
    return(App::currentWorld->luaScriptContainer->getScriptFromObjectAttachedTo_customization(getObjectHandle())!=nullptr);
//  return(_customizationScriptEnabled);
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

std::string CSceneObject::getUniquePersistentIdString() const
{
    return(_uniquePersistentIdString);
}

int CSceneObject::getScriptExecutionOrder(int scriptType) const
{
    if (scriptType==sim_scripttype_customizationscript)
    {
        CLuaScriptObject* it=App::currentWorld->luaScriptContainer->getScriptFromObjectAttachedTo_customization(_objectHandle);
        if (it!=nullptr)
            return(it->getExecutionOrder());
    }
    else if ((scriptType&sim_scripttype_childscript)!=0)
    {
        CLuaScriptObject* it=App::currentWorld->luaScriptContainer->getScriptFromObjectAttachedTo_child(_objectHandle);
        if (it!=nullptr)
        {
            if ( it->getThreadedExecution()==((scriptType&sim_scripttype_threaded)!=0) )
                return(it->getExecutionOrder());
        }
    }
    return(sim_scriptexecorder_normal);
}

int CSceneObject::getScriptsToExecute(int scriptType,int parentTraversalDirection,std::vector<CLuaScriptObject*>& scripts,std::vector<int>& uniqueIds)
{
    int cnt=0;
    CLuaScriptObject* attachedScript=nullptr;
    if (scriptType==sim_scripttype_customizationscript)
        attachedScript=App::currentWorld->luaScriptContainer->getScriptFromObjectAttachedTo_customization(_objectHandle);
    else if ((scriptType&sim_scripttype_childscript)!=0)
    {
        attachedScript=App::currentWorld->luaScriptContainer->getScriptFromObjectAttachedTo_child(_objectHandle);
        if (attachedScript!=nullptr)
        {
            if ( attachedScript->getThreadedExecution()!=((scriptType&sim_scripttype_threaded)!=0) )
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

bool CSceneObject::setObjectName(const char* newName,bool check)
{ // overridden from _CSceneObject_
    bool diff=false;
    CSceneObject* thisObject=nullptr;
    if (check)
        thisObject=App::currentWorld->sceneObjects->getObjectFromHandle(_objectHandle);
    if (thisObject!=this)
        diff=_CSceneObject_::setObjectName(newName,false); // no checking or object not yet in scene
    else
    { // object is in world
        std::string nm(newName);
        tt::removeIllegalCharacters(nm,true);
        if ( (nm.size()>0)&&(SIM_LOWCASE_STRING_COMPARE("world",nm.c_str())!=0)&&(SIM_LOWCASE_STRING_COMPARE("none",nm.c_str())!=0) )
        {
            if (getObjectName()!=nm)
            {
                while (App::currentWorld->sceneObjects->getObjectFromName(nm.c_str())!=nullptr)
                    nm=tt::generateNewName_hashOrNoHash(nm,!tt::isHashFree(nm.c_str()));
                std::string oldName(_objectName);
                diff=_CSceneObject_::setObjectName(nm.c_str(),check);
                if (diff)
                    App::currentWorld->sceneObjects->objectWasRenamed(_objectHandle,oldName.c_str(),nm.c_str(),false); // update name index
            }
        }
    }
    return(diff);
}

bool CSceneObject::setObjectAltName(const char* newName,bool check)
{ // overridden from _CSceneObject_
    bool diff=false;
    CSceneObject* thisObject=nullptr;
    if (check)
        thisObject=App::currentWorld->sceneObjects->getObjectFromHandle(_objectHandle);
    if (thisObject!=this)
        diff=_CSceneObject_::setObjectAltName(newName,false); // no checking or object not yet in scene
    else
    { // object is in world
        std::string nm(newName);
        tt::removeAltNameIllegalCharacters(nm);
        if (nm.size()>0)
        {
            if (getObjectName()!=nm)
            {
                while (App::currentWorld->sceneObjects->getObjectFromAltName(nm.c_str())!=nullptr)
                    nm=tt::generateNewName_noHash(nm);
                std::string oldName(_objectAltName);
                diff=_CSceneObject_::setObjectAltName(nm.c_str(),check);
                if (diff)
                    App::currentWorld->sceneObjects->objectWasRenamed(_objectHandle,oldName.c_str(),nm.c_str(),true); // update name index
            }
        }
    }
    return(diff);
}

void CSceneObject::_setLocalTransformation_send(const C7Vector& tr) const
{ // overridden from _CSceneObject_
    _CSceneObject_::_setLocalTransformation_send(tr);

    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setObjectLocalTransformation(_ikPluginCounterpartHandle,_localTransformation);
}

void CSceneObject::_setParent_send(int parentHandle,bool keepObjectInPlace) const
{ // overridden from _CSceneObject_
    _CSceneObject_::_setParent_send(parentHandle,keepObjectInPlace);

    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
    {
        int p=-1;
        if (getParent()!=nullptr)
            p=getParent()->getIkPluginCounterpartHandle();
        CPluginContainer::ikPlugin_setObjectParent(_ikPluginCounterpartHandle,p);
    }
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

            ar.storeDataName("Anm"); // keep this before "Nme"
            ar << _objectAltName;
            ar.flush();

            ar.storeDataName("Nme");
            ar << _objectName;
            ar.flush();

            ar.storeDataName("Hci");
            ar << _hierarchyColorIndex;
            ar.flush();

            ar.storeDataName("Sci");
            ar << _collectionSelfCollisionIndicator;
            ar.flush();

            ar.storeDataName("Op2");
            int objProp=_localObjectProperty|sim_objectproperty_reserved5; // Needed for backward compatibility (still in serialization version 15)
            ar << objProp;
            ar.flush();

            // Keep a while for backward compatibility (19/4/2017) (in case people want to return to a previous CoppeliaSim version):
            ar.storeDataName("Va2");
            unsigned char dummy=0;
            SIM_SET_CLEAR_BIT(dummy,0,_modelBase);
            SIM_SET_CLEAR_BIT(dummy,1,_objectTranslationDisabledDuringSimulation);
            SIM_SET_CLEAR_BIT(dummy,2,_objectTranslationDisabledDuringNonSimulation);
            SIM_SET_CLEAR_BIT(dummy,3,_ignoredByViewFitting);
            SIM_SET_CLEAR_BIT(dummy,7,_assemblingLocalTransformationIsUsed);
            ar << dummy;
            ar.flush();


            ar.storeDataName("Va3");
            dummy=0;
            SIM_SET_CLEAR_BIT(dummy,0,_modelBase);
            SIM_SET_CLEAR_BIT(dummy,1,_objectTranslationDisabledDuringSimulation);
            SIM_SET_CLEAR_BIT(dummy,2,_objectTranslationDisabledDuringNonSimulation);
            SIM_SET_CLEAR_BIT(dummy,3,_ignoredByViewFitting);
            SIM_SET_CLEAR_BIT(dummy,4,_objectRotationDisabledDuringSimulation);
            SIM_SET_CLEAR_BIT(dummy,5,_objectRotationDisabledDuringNonSimulation);
            SIM_SET_CLEAR_BIT(dummy,7,_assemblingLocalTransformationIsUsed);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Va4");
            dummy=0;
            SIM_SET_CLEAR_BIT(dummy,0,_objectTranslationSettingsLocked);
            SIM_SET_CLEAR_BIT(dummy,1,_objectRotationSettingsLocked);
            ar << dummy;
            ar.flush();



            ar.storeDataName("Omp");
            ar << _localObjectSpecialProperty;
            ar.flush();

            ar.storeDataName("Mpo");
            ar << _localModelProperty;
            ar.flush();

            ar.storeDataName("Lar");
            ar << _visibilityLayer;
            ar.flush();

            ar.storeDataName("Om5");
            ar << _objectManipulationModePermissions << _objectManipulationTranslationRelativeTo << _objectTranslationNonDefaultStepSize;
            ar.flush();

            ar.storeDataName("Omr");
            ar << _objectManipulationRotationRelativeTo << _objectRotationNonDefaultStepSize;
            ar.flush();

            ar.storeDataName("Sfa");
            ar << _sizeFactor;
            ar.flush();

            ar.storeDataName("Sfb");
            ar << _sizeValues[0] << _sizeValues[1] << _sizeValues[2];
            ar.flush();

            if (_customObjectData!=nullptr)
            {
                ar.storeDataName("Cod");
                ar.setCountingMode();
                _customObjectData->serializeData(ar,nullptr,-1);
                if (ar.setWritingMode())
                    _customObjectData->serializeData(ar,nullptr,-1);
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
                            _assemblyMatchValuesChild.push_back(tt::intToString(child));
                        _assemblyMatchValuesParent.clear();
                        if (parent==0)
                            _assemblyMatchValuesParent.push_back("default");
                        else
                            _assemblyMatchValuesParent.push_back(tt::intToString(parent));
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
                    if (theName.compare("Anm")==0)
                    {
                        hasAltName=true;
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _objectAltName;
                    }
                    if (theName.compare("Nme")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _objectName;
                        if (!hasAltName)
                            _objectAltName=tt::getObjectAltNameFromObjectName(_objectName);
                        _objectTempName=_objectName;
                    }
                    if (theName.compare("Op2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _localObjectProperty;
                    }
                    if (theName.compare("Var")==0)
                    { // Keep for backward compatibility (31/3/2017)
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _modelBase=SIM_IS_BIT_SET(dummy,0);
                        _objectTranslationDisabledDuringSimulation=SIM_IS_BIT_SET(dummy,1);
                        _objectTranslationDisabledDuringNonSimulation=SIM_IS_BIT_SET(dummy,2);
                        _ignoredByViewFitting=SIM_IS_BIT_SET(dummy,3);
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
                        _objectRotationDisabledDuringSimulation=_objectTranslationDisabledDuringSimulation;
                        _objectRotationDisabledDuringNonSimulation=_objectTranslationDisabledDuringNonSimulation;
                    }
                    if (theName.compare("Va2")==0)
                    { // Keep for backward compatibility (19/4/2017)
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _modelBase=SIM_IS_BIT_SET(dummy,0);
                        _objectTranslationDisabledDuringSimulation=SIM_IS_BIT_SET(dummy,1);
                        _objectTranslationDisabledDuringNonSimulation=SIM_IS_BIT_SET(dummy,2);
                        _ignoredByViewFitting=SIM_IS_BIT_SET(dummy,3);
                        _objectRotationDisabledDuringSimulation=_objectTranslationDisabledDuringSimulation;
                        _objectRotationDisabledDuringNonSimulation=_objectTranslationDisabledDuringNonSimulation;
                        _assemblingLocalTransformationIsUsed=SIM_IS_BIT_SET(dummy,7);
                    }
                    if (theName.compare("Va3")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _modelBase=SIM_IS_BIT_SET(dummy,0);
                        _objectTranslationDisabledDuringSimulation=SIM_IS_BIT_SET(dummy,1);
                        _objectTranslationDisabledDuringNonSimulation=SIM_IS_BIT_SET(dummy,2);
                        _ignoredByViewFitting=SIM_IS_BIT_SET(dummy,3);
                        _objectRotationDisabledDuringSimulation=SIM_IS_BIT_SET(dummy,4);
                        _objectRotationDisabledDuringNonSimulation=SIM_IS_BIT_SET(dummy,5);
                        _assemblingLocalTransformationIsUsed=SIM_IS_BIT_SET(dummy,7);
                    }
                    if (theName.compare("Va4")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _objectTranslationSettingsLocked=SIM_IS_BIT_SET(dummy,0);
                        _objectRotationSettingsLocked=SIM_IS_BIT_SET(dummy,1);
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
                        ar >> _localModelProperty;
                    }
                    if (theName.compare("Lar")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _visibilityLayer;
                    }
                    if (theName.compare("Om5")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _objectManipulationModePermissions >> _objectManipulationTranslationRelativeTo >> _objectTranslationNonDefaultStepSize;
                    }
                    if (theName.compare("Omr")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _objectManipulationRotationRelativeTo >> _objectRotationNonDefaultStepSize;
                    }

                    if (theName.compare("Cod")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        _customObjectData=new CCustomData();
                        _customObjectData->serializeData(ar,nullptr,-1);
                    }
                    if (theName.compare("Lsp")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        _userScriptParameters=new CUserParameters();
                        _userScriptParameters->serialize(ar);
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
            if ((_localObjectProperty&sim_objectproperty_reserved5)==0)
            { // this used to be the sim_objectproperty_visible property. If it wasn't set in the past, we now try to hide it in a hidden layer:
                if (_visibilityLayer<256)
                    _visibilityLayer=_visibilityLayer*256;
            }
            else
                _localObjectProperty-=sim_objectproperty_reserved5;
            //*************************************************************

            //*************************************************************
            // For old models to support the DNA-thing by default:
            if ( (ar.getCoppeliaSimVersionThatWroteThisFile()<30003)&&getModelBase() )
            {
                _localObjectProperty|=sim_objectproperty_canupdatedna;
                // We now create a "unique" id, that is always the same for the same file:
                _dnaString="1234567890123456";
                std::string a(_objectName);
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

            ar.xmlAddNode_string("name",_objectName.c_str());
            ar.xmlAddNode_string("altName",_objectAltName.c_str());

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
            ar.xmlAddNode_bool("hierarchyCollapsed",_localObjectProperty&sim_objectproperty_collapsed);
            ar.xmlAddNode_bool("selectable",_localObjectProperty&sim_objectproperty_selectable);
            ar.xmlAddNode_bool("selectModelBaseInstead",_localObjectProperty&sim_objectproperty_selectmodelbaseinstead);
            ar.xmlAddNode_bool("dontShowAsInsideModel",_localObjectProperty&sim_objectproperty_dontshowasinsidemodel);
            ar.xmlAddNode_bool("canUpdateDna",_localObjectProperty&sim_objectproperty_canupdatedna);
            ar.xmlAddNode_bool("selectInvisible",_localObjectProperty&sim_objectproperty_selectinvisible);
            ar.xmlAddNode_bool("depthInvisible",_localObjectProperty&sim_objectproperty_depthinvisible);
            ar.xmlAddNode_bool("cannotDelete",_localObjectProperty&sim_objectproperty_cannotdelete);
            ar.xmlAddNode_bool("cannotDeleteDuringSimulation",_localObjectProperty&sim_objectproperty_cannotdeleteduringsim);
            ar.xmlPopNode();

            ar.xmlPushNewNode("localObjectSpecialProperty");
            ar.xmlAddNode_bool("collidable",_localObjectSpecialProperty&sim_objectspecialproperty_collidable);
            ar.xmlAddNode_bool("measurable",_localObjectSpecialProperty&sim_objectspecialproperty_measurable);
            ar.xmlAddNode_bool("renderable",_localObjectSpecialProperty&sim_objectspecialproperty_renderable);
            ar.xmlAddNode_comment(" following 5 can be set at the same time with the 'detectable' tag",exhaustiveXml);
            ar.xmlAddNode_bool("ultrasonicDetectable",_localObjectSpecialProperty&sim_objectspecialproperty_detectable_ultrasonic);
            ar.xmlAddNode_bool("infraredDetectable",_localObjectSpecialProperty&sim_objectspecialproperty_detectable_infrared);
            ar.xmlAddNode_bool("laserDetectable",_localObjectSpecialProperty&sim_objectspecialproperty_detectable_laser);
            ar.xmlAddNode_bool("inductiveDetectable",_localObjectSpecialProperty&sim_objectspecialproperty_detectable_inductive);
            ar.xmlAddNode_bool("capacitiveDetectable",_localObjectSpecialProperty&sim_objectspecialproperty_detectable_capacitive);
            ar.xmlPopNode();

            ar.xmlPushNewNode("localModelProperty");
            ar.xmlAddNode_bool("notCollidable",_localModelProperty&sim_modelproperty_not_collidable);
            ar.xmlAddNode_bool("notMeasurable",_localModelProperty&sim_modelproperty_not_measurable);
            ar.xmlAddNode_bool("notRenderable",_localModelProperty&sim_modelproperty_not_renderable);
            ar.xmlAddNode_bool("notDetectable",_localModelProperty&sim_modelproperty_not_detectable);
            ar.xmlAddNode_bool("notDynamic",_localModelProperty&sim_modelproperty_not_dynamic);
            ar.xmlAddNode_bool("notRespondable",_localModelProperty&sim_modelproperty_not_respondable);
            ar.xmlAddNode_bool("notReset",_localModelProperty&sim_modelproperty_not_reset);
            ar.xmlAddNode_bool("notVisible",_localModelProperty&sim_modelproperty_not_visible);
            ar.xmlAddNode_bool("scriptsInactive",_localModelProperty&sim_modelproperty_scripts_inactive);
            ar.xmlAddNode_bool("notShowAsInsideModel",_localModelProperty&sim_modelproperty_not_showasinsidemodel);
            ar.xmlPopNode();

            ar.xmlAddNode_int("layer",_visibilityLayer);

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("modelBase",_modelBase);
            ar.xmlAddNode_bool("ignoredByViewFitting",_ignoredByViewFitting);
            ar.xmlPopNode();

            if (exhaustiveXml)
            {
                ar.xmlPushNewNode("manipulation");
                ar.xmlAddNode_int("permissions",_objectManipulationModePermissions);
                ar.xmlPushNewNode("translation");
                ar.xmlAddNode_bool("disabledDuringSimulation",_objectTranslationDisabledDuringSimulation);
                ar.xmlAddNode_bool("disabledDuringNonSimulation",_objectTranslationDisabledDuringNonSimulation);
                ar.xmlAddNode_bool("settingsLocked",_objectTranslationSettingsLocked);
                ar.xmlAddNode_int("relativeTo",_objectManipulationTranslationRelativeTo);
                ar.xmlAddNode_float("nonDefaultStepSize",_objectTranslationNonDefaultStepSize);
                ar.xmlPopNode();
                ar.xmlPushNewNode("rotation");
                ar.xmlAddNode_bool("disabledDuringSimulation",_objectRotationDisabledDuringSimulation);
                ar.xmlAddNode_bool("disabledDuringNonSimulation",_objectRotationDisabledDuringNonSimulation);
                ar.xmlAddNode_bool("settingsLocked",_objectRotationSettingsLocked);
                ar.xmlAddNode_int("relativeTo",_objectManipulationRotationRelativeTo);
                ar.xmlAddNode_float("nonDefaultStepSize",_objectRotationNonDefaultStepSize);
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
                if (_customObjectData!=nullptr)
                {
                    ar.xmlPushNewNode("customData");
                    _customObjectData->serializeData(ar,_objectName.c_str(),-1);
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
            if (ar.xmlPushChildNode("common",exhaustiveXml))
            {
                if ( ar.xmlGetNode_string("name",_objectName,exhaustiveXml)&&(!exhaustiveXml) )
                {
                    tt::removeIllegalCharacters(_objectName,true);
                    _objectTempName=_objectName;
                    _objectName="XYZ___"+_objectName+"___XYZ";
                }
                if ( ar.xmlGetNode_string("altName",_objectAltName,exhaustiveXml)&&(!exhaustiveXml) )
                {
                    tt::removeAltNameIllegalCharacters(_objectAltName);
                    _objectTempAltName=_objectAltName;
                    _objectAltName="XYZ___"+_objectAltName+"___XYZ";
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
                        ar.xmlGetNode_floats("quaternion",tr.Q.data,4);
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
                    _localObjectProperty=0;
                    ar.xmlGetNode_flags("hierarchyCollapsed",_localObjectProperty,sim_objectproperty_collapsed,exhaustiveXml);
                    ar.xmlGetNode_flags("selectable",_localObjectProperty,sim_objectproperty_selectable,exhaustiveXml);
                    ar.xmlGetNode_flags("selectModelBaseInstead",_localObjectProperty,sim_objectproperty_selectmodelbaseinstead,exhaustiveXml);
                    ar.xmlGetNode_flags("dontShowAsInsideModel",_localObjectProperty,sim_objectproperty_dontshowasinsidemodel,exhaustiveXml);
                    ar.xmlGetNode_flags("canUpdateDna",_localObjectProperty,sim_objectproperty_canupdatedna,exhaustiveXml);
                    ar.xmlGetNode_flags("selectInvisible",_localObjectProperty,sim_objectproperty_selectinvisible,exhaustiveXml);
                    ar.xmlGetNode_flags("depthInvisible",_localObjectProperty,sim_objectproperty_depthinvisible,exhaustiveXml);
                    ar.xmlGetNode_flags("cannotDelete",_localObjectProperty,sim_objectproperty_cannotdelete,exhaustiveXml);
                    ar.xmlGetNode_flags("cannotDeleteDuringSimulation",_localObjectProperty,sim_objectproperty_cannotdeleteduringsim,exhaustiveXml);
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("localObjectSpecialProperty",exhaustiveXml))
                {
                    _localObjectSpecialProperty=0;
                    ar.xmlGetNode_flags("collidable",_localObjectSpecialProperty,sim_objectspecialproperty_collidable,exhaustiveXml);
                    ar.xmlGetNode_flags("measurable",_localObjectSpecialProperty,sim_objectspecialproperty_measurable,exhaustiveXml);
                    if (!exhaustiveXml)
                    {
                        bool detectable;
                        if (ar.xmlGetNode_bool("detectable",detectable,exhaustiveXml))
                        {
                            _localObjectSpecialProperty|=sim_objectspecialproperty_detectable_all;
                            if (!detectable)
                                _localObjectSpecialProperty-=sim_objectspecialproperty_detectable_all;
                        }
                    }
                    ar.xmlGetNode_flags("ultrasonicDetectable",_localObjectSpecialProperty,sim_objectspecialproperty_detectable_ultrasonic,exhaustiveXml);
                    ar.xmlGetNode_flags("infraredDetectable",_localObjectSpecialProperty,sim_objectspecialproperty_detectable_infrared,exhaustiveXml);
                    ar.xmlGetNode_flags("laserDetectable",_localObjectSpecialProperty,sim_objectspecialproperty_detectable_laser,exhaustiveXml);
                    ar.xmlGetNode_flags("inductiveDetectable",_localObjectSpecialProperty,sim_objectspecialproperty_detectable_inductive,exhaustiveXml);
                    ar.xmlGetNode_flags("capacitiveDetectable",_localObjectSpecialProperty,sim_objectspecialproperty_detectable_capacitive,exhaustiveXml);
                    ar.xmlGetNode_flags("renderable",_localObjectSpecialProperty,sim_objectspecialproperty_renderable,exhaustiveXml);
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("localModelProperty",exhaustiveXml))
                {
                    _localModelProperty=0;
                    ar.xmlGetNode_flags("notCollidable",_localModelProperty,sim_modelproperty_not_collidable,exhaustiveXml);
                    ar.xmlGetNode_flags("notMeasurable",_localModelProperty,sim_modelproperty_not_measurable,exhaustiveXml);
                    ar.xmlGetNode_flags("notRenderable",_localModelProperty,sim_modelproperty_not_renderable,exhaustiveXml);
                    ar.xmlGetNode_flags("notDetectable",_localModelProperty,sim_modelproperty_not_detectable,exhaustiveXml);
                    ar.xmlGetNode_flags("notDynamic",_localModelProperty,sim_modelproperty_not_dynamic,exhaustiveXml);
                    ar.xmlGetNode_flags("notRespondable",_localModelProperty,sim_modelproperty_not_respondable,exhaustiveXml);
                    ar.xmlGetNode_flags("notReset",_localModelProperty,sim_modelproperty_not_reset,exhaustiveXml);
                    ar.xmlGetNode_flags("notVisible",_localModelProperty,sim_modelproperty_not_visible,exhaustiveXml);
                    ar.xmlGetNode_flags("scriptsInactive",_localModelProperty,sim_modelproperty_scripts_inactive,exhaustiveXml);
                    ar.xmlGetNode_flags("notShowAsInsideModel",_localModelProperty,sim_modelproperty_not_showasinsidemodel,exhaustiveXml);
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("switches",exhaustiveXml))
                {
                    ar.xmlGetNode_bool("modelBase",_modelBase,exhaustiveXml);
                    ar.xmlGetNode_bool("ignoredByViewFitting",_ignoredByViewFitting,exhaustiveXml);
                    ar.xmlPopNode();
                }

                int l;
                if (ar.xmlGetNode_int("layer",l,exhaustiveXml))
                    _visibilityLayer=(unsigned short)l;

                if (exhaustiveXml&&ar.xmlPushChildNode("manipulation"))
                {
                    ar.xmlGetNode_int("permissions",_objectManipulationModePermissions);
                    if (ar.xmlPushChildNode("translation"))
                    {
                        ar.xmlGetNode_bool("disabledDuringSimulation",_objectTranslationDisabledDuringSimulation);
                        ar.xmlGetNode_bool("disabledDuringNonSimulation",_objectTranslationDisabledDuringNonSimulation);
                        ar.xmlGetNode_bool("settingsLocked",_objectTranslationSettingsLocked);
                        ar.xmlGetNode_int("relativeTo",_objectManipulationTranslationRelativeTo);
                        ar.xmlGetNode_float("nonDefaultStepSize",_objectTranslationNonDefaultStepSize);
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("rotation"))
                    {
                        ar.xmlGetNode_bool("disabledDuringSimulation",_objectRotationDisabledDuringSimulation);
                        ar.xmlGetNode_bool("disabledDuringNonSimulation",_objectRotationDisabledDuringNonSimulation);
                        ar.xmlGetNode_bool("settingsLocked",_objectRotationSettingsLocked);
                        ar.xmlGetNode_int("relativeTo",_objectManipulationRotationRelativeTo);
                        ar.xmlGetNode_float("nonDefaultStepSize",_objectRotationNonDefaultStepSize);
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

                if (exhaustiveXml&&ar.xmlPushChildNode("customData",false))
                {
                    _customObjectData=new CCustomData();
                    _customObjectData->serializeData(ar,_objectName.c_str(),-1);
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
        }
    }
}

void CSceneObject::serializeWExtIk(CExtIkSer& ar)
{
    C7Vector tr=getLocalTransformation();
    ar.writeFloat(tr.Q(0));
    ar.writeFloat(tr.Q(1));
    ar.writeFloat(tr.Q(2));
    ar.writeFloat(tr.Q(3));
    ar.writeFloat(tr.X(0));
    ar.writeFloat(tr.X(1));
    ar.writeFloat(tr.X(2));
    
    int parentID=-1;
    if (getParent()!=nullptr)
        parentID=getParent()->getObjectHandle();
    ar.writeInt(_objectHandle);
    ar.writeInt(parentID);

    ar.writeString(_objectName.c_str());
}

int CSceneObject::_uniqueIDCounter=0;

void CSceneObject::performObjectLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{
    int newParentID=CWorld::getLoadingMapping(map,_parentObjectHandle_forSerializationOnly);
    setParent(App::currentWorld->sceneObjects->getObjectFromHandle(newParentID),false);

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

std::string CSceneObject::getDisplayName() const
{
    if (CSimFlavor::getBoolVal(8))
        return(_objectAltName);
    return(_objectName);
}

bool CSceneObject::announceObjectWillBeErased(int objHandle,bool copyBuffer)
{ 
    // This routine can be called for sceneObjects-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'ct::sceneObjects->getObject(objHandle)'-call or similar
    // Return value true means this needs to be destroyed
#ifdef SIM_WITH_GUI
    // if we are in edit mode, we leave edit mode:
    if ( (App::getEditModeType()!=NO_EDIT_MODE)&&(!copyBuffer) )
    {
        if (App::mainWindow->editModeContainer->getEditModeObjectID()==objHandle)
            App::mainWindow->editModeContainer->processCommand(ANY_EDIT_MODE_FINISH_AND_CANCEL_CHANGES_EMCMD,nullptr); // This is if we destroy the object being edited (shouldn't normally happen!)
    }
#endif

    if (_authorizedViewableObjects==objHandle)
        _authorizedViewableObjects=-2; // not visible anymore!

    // If the object's parent will be erased, make the object child of its grand-parents
    if (!copyBuffer)
    {
        CSceneObject* parent=getParent();
        CSceneObject* toRemove=App::currentWorld->sceneObjects->getObjectFromHandle(objHandle);
        if (parent!=nullptr)
        {
            if (parent->getObjectHandle()==objHandle)
                setParent(parent->getParent(),true);
        }
        removeChild(toRemove);
    }
    for (size_t i=0;i<_customReferencedHandles.size();i++)
    {
        if (_customReferencedHandles[i].generalObjectType==sim_appobj_object_type)
        {
            if (_customReferencedHandles[i].generalObjectHandle==objHandle)
                _customReferencedHandles[i].generalObjectHandle=-1;
        }
    }
    if (!copyBuffer)
    {
        for (size_t i=0;i<_customReferencedOriginalHandles.size();i++)
        {
            if (_customReferencedOriginalHandles[i].generalObjectType==sim_appobj_object_type)
            {
                if (_customReferencedOriginalHandles[i].generalObjectHandle==objHandle)
                    _customReferencedOriginalHandles[i].generalObjectHandle=-1;
            }
        }
    }
    return(false);
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

C7Vector CSceneObject::getFullParentCumulativeTransformation_ikOld() const
{
    if (getParent()==nullptr)
    {
        C7Vector retV;
        retV.setIdentity();
        return(retV);
    }
    else
        return(getParent()->getFullCumulativeTransformation_ikOld());
}

C7Vector CSceneObject::getFullCumulativeTransformation_ikOld() const
{
    C7Vector tr(getFullLocalTransformation_ikOld());
    if (getParent()==nullptr)
        return(tr);
    else
        return(getFullParentCumulativeTransformation_ikOld()*tr);
}

C7Vector CSceneObject::getFullLocalTransformation_ikOld() const
{
    if (getObjectType()==sim_object_joint_type)
    {
        CJoint* it=(CJoint*)this;
        C7Vector jointTr;
        jointTr.setIdentity();
        float val=it->getPosition_useTempValues();
        if (it->getJointType()==sim_joint_revolute_subtype)
        {
            jointTr.Q.setAngleAndAxis(val,C3Vector(0.0f,0.0f,1.0f));
            jointTr.X(2)=val*it->getScrewPitch();
        }
        if (it->getJointType()==sim_joint_prismatic_subtype)
            jointTr.X(2)=val;
        if (it->getJointType()==sim_joint_spherical_subtype)
        {
            if (it->getTempSphericalJointLimitations()==0)
            { // Used by the IK routine when away from joint limitations
                jointTr.Q.setEulerAngles(0.0f,0.0f,it->getTempParameterEx(2));
                C4Vector q2;
                q2.setEulerAngles(piValD2_f,0.0f,0.0f);
                jointTr.Q=q2*jointTr.Q;

                q2.setEulerAngles(0.0f,0.0f,it->getTempParameterEx(1));
                jointTr.Q=q2*jointTr.Q;
                q2.setEulerAngles(-piValD2_f,0.0f,-piValD2_f);
                jointTr.Q=q2*jointTr.Q;

                q2.setEulerAngles(0.0f,0.0f,it->getTempParameterEx(0));
                jointTr.Q=q2*jointTr.Q;
                q2.setEulerAngles(0.0f,piValD2_f,0.0f);
                jointTr.Q=q2*jointTr.Q;
                q2=it->getSphericalTransformation();
                jointTr.Q=q2*jointTr.Q;
            }
            else
            { // Used by the IK routine when close to joint limitations
                jointTr.Q.setEulerAngles(0.0f,0.0f,it->getTempParameterEx(2));
                C4Vector q2;
                q2.setEulerAngles(0.0f,-piValD2_f,0.0f);
                jointTr.Q=q2*jointTr.Q;

                q2.setEulerAngles(0.0f,0.0f,it->getTempParameterEx(1));
                jointTr.Q=q2*jointTr.Q;
                q2.setEulerAngles(0.0f,piValD2_f,0.0f);
                jointTr.Q=q2*jointTr.Q;

                q2.setEulerAngles(0.0f,0.0f,it->getTempParameterEx(0));
                jointTr.Q=q2*jointTr.Q;
            }
        }
        return(_localTransformation*jointTr);
    }
    else if (getObjectType()==sim_object_dummy_type)
    {
        CDummy* it=(CDummy*)this;
        return(it->getTempLocalTransformation()); // used for IK when dummy is freely sliding on a path object
    }
    else
        return(_localTransformation);
}

C7Vector CSceneObject::getCumulativeTransformation_ikOld() const
{
    if (getObjectType()==sim_object_joint_type)
    {
        if (getParent()==nullptr)
            return(getLocalTransformation());
        else
            return(getFullParentCumulativeTransformation_ikOld()*getLocalTransformation());
    }
    else
        return(getFullCumulativeTransformation_ikOld());
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

bool CSceneObject::setParent(CSceneObject* newParent,bool keepObjectInPlace)
{ // Overridden from _CSceneObject_
    bool retVal=false;
    CSceneObject* oldParent=getParent();
    if ( (newParent!=oldParent)&&( (newParent==nullptr)||(!newParent->isObjectParentedWith(this)) ) )
    {
        CSceneObject* thisObject=App::currentWorld->sceneObjects->getObjectFromHandle(_objectHandle);
        CSceneObject* _newParent=nullptr;
        if (newParent!=nullptr)
            _newParent=App::currentWorld->sceneObjects->getObjectFromHandle(newParent->getObjectHandle());
        if ( (thisObject!=this)||(_newParent!=newParent) )
            retVal=_CSceneObject_::setParent(newParent,false); // object and/or parent is not yet in world
        else
        { // objects are in world
            C7Vector oldCumulTransf(getCumulativeTransformation());
            if (oldParent!=nullptr)
                retVal=oldParent->removeChild(this);
            retVal=_CSceneObject_::setParent(newParent,keepObjectInPlace)||retVal;
            if (retVal)
            {
                if (newParent!=nullptr)
                    newParent->addChild(this);
                int h1=-1;
                if (oldParent!=nullptr)
                    h1=oldParent->getObjectHandle();
                int h2=-1;
                if (newParent!=nullptr)
                    h2=newParent->getObjectHandle();
                App::currentWorld->sceneObjects->objectGotNewParent(_objectHandle,h1,h2);
                CSceneObject::incrementModelPropertyValidityNumber();
                App::setRebuildHierarchyFlag();
                if (keepObjectInPlace)
                    setLocalTransformation(getFullParentCumulativeTransformation().getInverse()*oldCumulTransf);
            }
        }
    }
    return(retVal);
}

int CSceneObject::getFirstModelRelatives(std::vector<CSceneObject*>& firstModelRelatives,bool visibleModelsOnly) const
{
    int cnt=0;
    for (size_t i=0;i<getChildCount();i++)
    {
        CSceneObject* child=getChildFromIndex(i);
        if (child->getModelBase())
        {
            if (!child->isObjectPartOfInvisibleModel())
            {
                firstModelRelatives.push_back(child);
                cnt++;
            }
        }
        else
            cnt+=child->getFirstModelRelatives(firstModelRelatives,visibleModelsOnly);
    }
    return(cnt);
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
std::string CSceneObject::getObjectTempName() const
{
    return(_objectTempName);
}

std::string CSceneObject::getObjectTempAltName() const
{
    return(_objectTempAltName);
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
{ // names can't be changed here!
//    _objectName=obj->_objectName;
    _objectTempName=obj->_objectTempName;
//    _objectAltName=obj->_objectAltName;
    _localTransformation=obj->_localTransformation;
    _hierarchyColorIndex=obj->_hierarchyColorIndex;
    _collectionSelfCollisionIndicator=obj->_collectionSelfCollisionIndicator;
    _localObjectProperty=obj->_localObjectProperty;
    _localObjectSpecialProperty=obj->_localObjectSpecialProperty;
    _localModelProperty=obj->_localModelProperty;
    _modelBase=obj->_modelBase;
    _ignoredByViewFitting=obj->_ignoredByViewFitting;
    _CSceneObject_::setVisibilityLayer(obj->getVisibilityLayer());
    _extensionString=obj->_extensionString;
    _modelAcknowledgement=obj->_modelAcknowledgement;
}

int CSceneObject::getObjectType() const
{
    return(_objectType);
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
        CPathCont* pc;
        if ( ( (App::getEditModeType()&PATH_EDIT_MODE)||(App::mainWindow->editModeContainer->pathPointManipulation->getSelectedPathPointIndicesSize_nonEditMode()!=0) )&&((_objectManipulationMode_flaggedForGridOverlay&8)==0) )

        { // (path is in edition or path points are selected) and no rotation
            isPathPoints=true;
            if (App::getEditModeType()&PATH_EDIT_MODE)
            { // Path is in edition
                pc=App::mainWindow->editModeContainer->getEditModePathContainer();
                pathPointsToTakeIntoAccount.assign(App::mainWindow->editModeContainer->getEditModeBuffer()->begin(),App::mainWindow->editModeContainer->getEditModeBuffer()->end());
            }
            else
            { // Path points are selected (but not in path edit mode)
                pc=((CPath*)this)->pathContainer;
                pathPointsToTakeIntoAccount.assign(App::mainWindow->editModeContainer->pathPointManipulation->getPointerToSelectedPathPointIndices_nonEditMode()->begin(),App::mainWindow->editModeContainer->pathPointManipulation->getPointerToSelectedPathPointIndices_nonEditMode()->end());
            }
        }
        else
        { // Path is not in edition and no path points are selected
            pc=((CPath*)this)->pathContainer;
            int cnt=pc->getSimplePathPointCount();
            for (int i=0;i<cnt;i++)
                pathPointsToTakeIntoAccount.push_back(i);
        }
        C3Vector minCoord,maxCoord;
        for (int i=0;i<int(pathPointsToTakeIntoAccount.size());i++)
        {
            CSimplePathPoint* aPt=pc->getSimplePathPoint(pathPointsToTakeIntoAccount[i]);
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

        if (getObjectManipulationRotationRelativeTo()==0) // world
            tr.M.setIdentity();
        if (getObjectManipulationRotationRelativeTo()==1) // parent frame
            tr.M=getFullParentCumulativeTransformation().getMatrix().M;
        if ((getObjectManipulationRotationRelativeTo()==2)||isPathPoints) // own frame
            tr.M=getCumulativeTransformation().getMatrix().M;
    }
    else
    { // translation
        axisInfo=_objectManipulationMode_flaggedForGridOverlay-16;
        tr=getCumulativeTransformation().getMatrix();
        if (getObjectManipulationTranslationRelativeTo()==0) // world
            tr.M.setIdentity();
        if (getObjectManipulationTranslationRelativeTo()==1) // parent frame
            tr.M=getFullParentCumulativeTransformation().getMatrix().M;
        if ((getObjectManipulationTranslationRelativeTo()==2)||isPathPoints) // own frame
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
    {
        if (!getMarkingBoundingBox(bbMin,bbMax))
            bbMax=C3Vector(0.1f,0.1f,0.1f); // shouldn't happen!
    }
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
        if (getObjectManipulationRotationRelativeTo()==2) // own frame
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
    if ( (!App::currentWorld->simulation->isSimulationStopped())&&getObjectRotationDisabledDuringSimulation())
    {
        _objectManipulationMode_flaggedForGridOverlay=0;
        _objectManipulationModeEventId=-1;
        return(false);
    }
    if (App::currentWorld->simulation->isSimulationStopped()&&getObjectRotationDisabledDuringNonSimulation())
    {
        _objectManipulationMode_flaggedForGridOverlay=0;
        _objectManipulationModeEventId=-1;
        return(false);
    }
    static int  otherAxisMemorized=0;
    bool ctrlKeyDown=((App::mainWindow!=nullptr)&&(App::mainWindow->getKeyDownState()&1))&&(!_objectRotationSettingsLocked);
    if ( (!ctrlKeyDown)&&((getObjectManipulationModePermissions()&56)==0) )
    { // This is special so that, when no manip is allowed but we held down the ctrl key and released it, the green manip disc doesn't appear
        _objectManipulationModeAxisIndex=-1;
        _objectManipulationMode_flaggedForGridOverlay=0;
        _objectManipulationModePermissionsPreviousCtrlKeyDown=ctrlKeyDown;
        return(false);
    }
    if ( (eventID!=_objectManipulationModeEventId)||(ctrlKeyDown!=_objectManipulationModePermissionsPreviousCtrlKeyDown) )
    {
        if ( (otherAxisMemorized>1)&&((getObjectManipulationModePermissions()&56)!=0) )
            otherAxisMemorized=0;
        if (otherAxisMemorized>2)
            otherAxisMemorized=0;
        _objectManipulationModeSubTranslation.clear();
        _objectManipulationModeSubRotation=0.0f;
        _objectManipulationModeEventId=eventID;
        _objectManipulationModeTotalTranslation.clear();
        _objectManipulationModeTotalRotation=0.0f;
        // Let's first see around which axis we wanna rotate:
        int _objectManipulationModePermissionsTEMP=getObjectManipulationModePermissions();
        bool specialMode=false;
        if (ctrlKeyDown)
            specialMode=true;
        C4X4Matrix objAbs(getCumulativeTransformation().getMatrix());
        C3X3Matrix rotAxes;
        if (getObjectManipulationRotationRelativeTo()==2)
            rotAxes=objAbs.M; // own frame
        if (getObjectManipulationRotationRelativeTo()==1)
            rotAxes=getFullParentCumulativeTransformation().getMatrix().M; // parent frame
        if (getObjectManipulationRotationRelativeTo()==0)
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
            if ( (fabs(l)>=ml)&&(_objectManipulationModePermissionsTEMP&(8<<i)) )
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

    _objectManipulationModePermissionsPreviousCtrlKeyDown=ctrlKeyDown;

    if (_objectManipulationModeAxisIndex==-1)
        return(false); //rotation not allowed

    float ss=getNonDefaultRotationStepSize();
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
    if (getObjectManipulationRotationRelativeTo()==2)
        tr.Q*=rot; // relative to own frame
    if (getObjectManipulationRotationRelativeTo()==1)
        tr.Q=rot*tr.Q; // relative to parent frame
    if (getObjectManipulationRotationRelativeTo()==0)
    { // relative to world frame
        C4Vector trq(getCumulativeTransformation().Q);
        trq=rot*trq;
        C4Vector pinv(getFullParentCumulativeTransformation().Q.getInverse());
        tr.Q=pinv*trq;
    }
    setLocalTransformation(tr);
    _objectManipulationMode_flaggedForGridOverlay=_objectManipulationModeAxisIndex+8;
    return(true);
}


bool CSceneObject::setLocalTransformationFromObjectTranslationMode(const C4X4Matrix& cameraAbsConf,const C3Vector& clicked3DPoint,float prevPos[2],float pos[2],float screenHalfSizes[2],float halfSizes[2],bool perspective,int eventID)
{ // bits 0-2: position x,y,z (relative to parent frame), bits 3-5: Euler e9,e1,e2 (relative to own frame)

    if ( (!App::currentWorld->simulation->isSimulationStopped())&&getObjectTranslationDisabledDuringSimulation())
    {
        _objectManipulationMode_flaggedForGridOverlay=0;
        _objectManipulationModeEventId=-1;
        return(false);
    }
    if (App::currentWorld->simulation->isSimulationStopped()&&getObjectTranslationDisabledDuringNonSimulation())
    {
        _objectManipulationMode_flaggedForGridOverlay=0;
        _objectManipulationModeEventId=-1;
        return(false);
    }

    C4X4Matrix objAbs;
    objAbs.X=getCumulativeTransformation().X;
    if (getObjectManipulationTranslationRelativeTo()==0)
        objAbs.M.setIdentity();
    if (getObjectManipulationTranslationRelativeTo()==1)
        objAbs.M=getFullParentCumulativeTransformation().getMatrix().M;
    if (getObjectManipulationTranslationRelativeTo()==2)
        objAbs.M=getCumulativeTransformation().getMatrix().M;
    static int  otherAxisMemorized=0;
    bool ctrlKeyDown=((App::mainWindow!=nullptr)&&(App::mainWindow->getKeyDownState()&1))&&(!_objectTranslationSettingsLocked);
    if ( (!ctrlKeyDown)&&((getObjectManipulationModePermissions()&7)==0) )
    { // This is special so that, when no manip is allowed but we held down the ctrl key and released it, the green manip bars don't appear
        _objectManipulationModeAxisIndex=-1;
        _objectManipulationMode_flaggedForGridOverlay=0;
        _objectManipulationModePermissionsPreviousCtrlKeyDown=ctrlKeyDown;
        return(false);
    }
    if (eventID!=_objectManipulationModeEventId)
        _objectManipulationModeRelativePositionOfClickedPoint=clicked3DPoint-objAbs.X; // Added on 2010/07/29
    if ( (eventID!=_objectManipulationModeEventId)||(ctrlKeyDown!=_objectManipulationModePermissionsPreviousCtrlKeyDown) )
    {
        if (otherAxisMemorized>1)
            otherAxisMemorized=0;
        _objectManipulationModeSubTranslation.clear();
        _objectManipulationModeSubRotation=0.0f;
        _objectManipulationModeEventId=eventID;
        _objectManipulationModeTotalTranslation.clear();
        _objectManipulationModeTotalRotation=0.0f;
        // Let's first see on which plane we wanna translate:
        int _objectManipulationModePermissionsTEMP=getObjectManipulationModePermissions();
        bool specialMode=false;
        bool specialMode2=false;
        if (ctrlKeyDown)
        {
            if ((_objectManipulationModePermissionsTEMP&7)!=7)
            {
                _objectManipulationModePermissionsTEMP^=7;
                specialMode2=((_objectManipulationModePermissionsTEMP&7)==7);
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
            if ( (fabs(l)>=ml)&&((_objectManipulationModePermissionsTEMP&(planeComb[i]))==planeComb[i]) )
            {
                ml=fabs(l);
                _objectManipulationModeAxisIndex=i;
            }
        }
        if (_objectManipulationModeAxisIndex==-1)
        { // maybe we are constrained to stay on a line?
            for (int i=0;i<3;i++)
            {
                if (_objectManipulationModePermissionsTEMP&(1<<i))
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

    _objectManipulationModePermissionsPreviousCtrlKeyDown=ctrlKeyDown;

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
        float ss=getNonDefaultTranslationStepSize();
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

            // Build remote dummy:
            sendVoid(sim_syncobj_sceneobject_create);
        }

        // Build IK plugin counterpart, if not a joint:
        if (_ikPluginCounterpartHandle==-1)
            _ikPluginCounterpartHandle=CPluginContainer::ikPlugin_createDummy();

        // Update the remote object:
        _setExtensionString_send(_extensionString.c_str());
        _setVisibilityLayer_send(_visibilityLayer);
        _setObjectName_send(_objectName.c_str());
        _setObjectAltName_send(_objectAltName.c_str());
        _setLocalTransformation_send(_localTransformation);

        // Update sub-objects:
    }
}

void CSceneObject::connectSynchronizationObject()
{ // Overridden from CSyncObject
    if (getObjectCanSync())
    {
        int h=-1;
        if (getParent()!=nullptr)
            h=getParent()->getObjectHandle();
        _setParent_send(h,false);
    }
}

void CSceneObject::removeSynchronizationObject(bool localReferencesToItOnly)
{ // Overridden from CSyncObject
    if (getObjectCanSync())
    {
        setObjectCanSync(false);

        if (!localReferencesToItOnly)
        {
            // Delete remote object:
            sendVoid(sim_syncobj_sceneobject_delete);

            // Synchronize with IK plugin:
            if (_ikPluginCounterpartHandle!=-1)
                CPluginContainer::ikPlugin_eraseObject(_ikPluginCounterpartHandle);
        }
    }
    // IK plugin part:
    _ikPluginCounterpartHandle=-1;
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
        _childList.push_back(child);
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
    return(retVal);
}

