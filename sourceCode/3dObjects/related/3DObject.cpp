#include "vrepMainHeader.h"
#include "v_rep_internal.h"
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
#include "3DObject.h"
#include "dummy.h"
#include "global.h"
#include "miscBase.h"
#ifdef SIM_WITH_GUI
    #include "oGL.h"
    #include "oglSurface.h"
#endif

int C3DObject::_modelPropertyValidityNumber=0;

C3DObject::C3DObject()
{
    _parentObject=nullptr;
    _parentHandle=-1;
    _objectHandle=0;
    _transformation.setIdentity();
    generateDnaString();
    _assemblingLocalTransformation.setIdentity();
    _assemblingLocalTransformationIsUsed=false;

    _authorizedViewableObjects=-1; // all
    _assemblyMatchValuesChild.push_back("default");
    _assemblyMatchValuesParent.push_back("default");
    _forceAlwaysVisible_tmp=false;

    _localObjectProperty=sim_objectproperty_selectable;
    _hierarchyColorIndex=-1;
    _collectionSelfCollisionIndicator=0;

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

    _specificLight=-1; // default, i.e. all lights

    _sizeFactor=1.0f;
    _sizeValues[0]=1.0f;
    _sizeValues[1]=1.0f;
    _sizeValues[2]=1.0f;
    _dynamicsFullRefreshFlag=true;
    _ignoredByViewFitting=false;
}

C3DObject::~C3DObject() 
{
    delete _customObjectData;
    delete _customObjectData_tempData;
}

void C3DObject::setForceAlwaysVisible_tmp(bool force)
{
    _forceAlwaysVisible_tmp=force;
}

void C3DObject::measureVelocity(float dt)
{
    C7Vector abs(getCumulativeTransformationPart1());
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

void C3DObject::setAuthorizedViewableObjects(int objOrCollHandle)
{
    _authorizedViewableObjects=objOrCollHandle;
}

int C3DObject::getAuthorizedViewableObjects() const
{
    return(_authorizedViewableObjects);
}


void C3DObject::setTransparentObjectDistanceOffset(float d)
{
    _transparentObjectDistanceOffset=d;
}

float C3DObject::getTransparentObjectDistanceOffset() const
{
    return(_transparentObjectDistanceOffset);
}


float C3DObject::getMeasuredAngularVelocity() const
{
    return(_measuredAngularVelocity_velocityMeasurement);
}

C3Vector C3DObject::getMeasuredLinearVelocity() const
{
    return(_measuredLinearVelocity_velocityMeasurement);
}

C3Vector C3DObject::getMeasuredAngularVelocity3() const
{
    return(_measuredAngularVelocity3_velocityMeasurement);
}

C3Vector C3DObject::getMeasuredAngularVelocityAxis() const
{
    return(_measuredAngularVelocityAxis_velocityMeasurement);
}

void C3DObject::setHierarchyColorIndex(int c)
{
    _hierarchyColorIndex=c;
}

int C3DObject::getHierarchyColorIndex() const
{
    return(_hierarchyColorIndex);
}

void C3DObject::setCollectionSelfCollisionIndicator(int c)
{
    c=tt::getLimitedInt(0,99999999,c);
    _collectionSelfCollisionIndicator=c;
}

int C3DObject::getCollectionSelfCollisionIndicator() const
{
    return(_collectionSelfCollisionIndicator);
}

void C3DObject::setDynamicObjectFlag_forVisualization(int isDynamicallySimulatedFlag)
{
    _dynamicObjectFlag_forVisualization=isDynamicallySimulatedFlag;
}

int C3DObject::getDynamicObjectFlag_forVisualization() const
{
    return(_dynamicObjectFlag_forVisualization);
}

int C3DObject::getDynamicObjectFlag_forVisualization_forDisplay(bool guiIsRendering) const
{
    if (guiIsRendering)
        return(_dynamicObjectFlag_forVisualization_forDisplay);
    else
        return(_dynamicObjectFlag_forVisualization);
}

bool C3DObject::getShouldObjectBeDisplayed(int viewableHandle,int displayAttrib)
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
        {
            if ((displayAttrib&sim_displayattribute_forvisionsensor)==0)
                display=(_dynamicObjectFlag_forVisualization_forDisplay!=0);
            else
                display=(_dynamicObjectFlag_forVisualization!=0);
        }
        else
        {
            display=( (!isObjectPartOfInvisibleModel())&&((App::ct->mainSettings->getActiveLayers()&layer)||(displayAttrib&sim_displayattribute_ignorelayer)) );

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
                            CRegCollection* gr=App::ct->collections->getCollection(_authorizedViewableObjects);
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

void C3DObject::setIgnoredByViewFitting(bool ignored)
{
    _ignoredByViewFitting=ignored;
}

bool C3DObject::getIgnoredByViewFitting() const
{
    return(_ignoredByViewFitting);
}

void C3DObject::setModelAcknowledgement(const std::string& a)
{
    _modelAcknowledgement=a;
    if (_modelAcknowledgement.length()>3000)
        _modelAcknowledgement.erase(_modelAcknowledgement.begin()+2999,_modelAcknowledgement.end());
}

std::string C3DObject::getModelAcknowledgement() const
{
    return(_modelAcknowledgement);
}

void C3DObject::setDynamicsFullRefreshFlag(bool refresh)
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

bool C3DObject::getDynamicsFullRefreshFlag() const
{
    return(_dynamicsFullRefreshFlag);
}


void C3DObject::setSizeFactor(float f)
{
    f=tt::getLimitedFloat(0.000001f,1000000.0f,f);
    _sizeFactor=f;
}

float C3DObject::getSizeFactor() const
{
    return(_sizeFactor);
}

void C3DObject::setSizeValues(const float s[3])
{
    _sizeValues[0]=s[0];
    _sizeValues[1]=s[1];
    _sizeValues[2]=s[2];
}

void C3DObject::getSizeValues(float s[3]) const
{
    s[0]=_sizeValues[0];
    s[1]=_sizeValues[1];
    s[2]=_sizeValues[2];
}


int C3DObject::getParentCount() const
{
    if (getParentObject()==nullptr)
        return(0);
    return(1+getParentObject()->getParentCount());
}

int C3DObject::getUniqueID() const
{
    return(_uniqueID);
}

int C3DObject::getAllowedObjectSpecialProperties()
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
    if (isPotentiallyCuttable())
        retVal|=sim_objectspecialproperty_cuttable;
    return(retVal);
}

void C3DObject::incrementMemorizedConfigurationValidCounter()
{
    _memorizedConfigurationValidCounter++;
}

int C3DObject::getMemorizedConfigurationValidCounter()
{
    return(_memorizedConfigurationValidCounter);
}

bool C3DObject::getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const
{
    return(false);
}

C3DObject* C3DObject::copyYourself() 
{
    return(nullptr);
}

void C3DObject::removeSceneDependencies()
{
}

std::string C3DObject::getObjectTypeInfo() const
{
    return("");
}
std::string C3DObject::getObjectTypeInfoExtended() const
{
    return("");
}
bool C3DObject::isPotentiallyCollidable() const
{
    return(false);
}
bool C3DObject::isPotentiallyMeasurable() const
{
    return(false);
}
bool C3DObject::isPotentiallyDetectable() const
{
    return(false);
}
bool C3DObject::isPotentiallyRenderable() const
{
    return(false);
}
bool C3DObject::isPotentiallyCuttable() const
{
    return(false);
}


void C3DObject::setModelBase(bool m)
{ // is also called from the ungroup/divide shape routines!!
    _modelBase=m;
    _localModelProperty=0; // Nothing is overridden!
    _modelAcknowledgement="";
    incrementModelPropertyValidityNumber();
}

bool C3DObject::getModelBase() const
{
    return(_modelBase);
}

void C3DObject::setLocalObjectProperty(int p)
{
    _localObjectProperty=p;
}

int C3DObject::getLocalObjectProperty()
{
    return(_localObjectProperty);
}

int C3DObject::getCumulativeObjectProperty()
{
    int o=getCumulativeModelProperty();
    if (o==0)
        return(_localObjectProperty); // nothing is overridden!
    int p=_localObjectProperty;

    if (o&sim_modelproperty_not_showasinsidemodel)
        p=(p|sim_objectproperty_dontshowasinsidemodel);

    return(p);
}

void C3DObject::setLocalObjectSpecialProperty(int prop)
{ // sets the local value
    _localObjectSpecialProperty=prop;
    int allowed=getAllowedObjectSpecialProperties();
    /*
    int allowed=0;
    if (_objectType==sim_object_shape_type)
    {
        allowed=sim_objectspecialproperty_collidable|sim_objectspecialproperty_measurable|sim_objectspecialproperty_detectable_all|sim_objectspecialproperty_renderable|sim_objectspecialproperty_pathplanning_ignored;
        if ( (!((CShape*)this)->isCompound())&&(!((CShape*)this)->geomData->geomInfo->isPure()) )
            allowed|=sim_objectspecialproperty_cuttable;
    }
    if (_objectType==sim_object_octree_type)
        allowed=sim_objectspecialproperty_collidable|sim_objectspecialproperty_measurable|sim_objectspecialproperty_detectable_all;
    if (_objectType==sim_object_pointcloud_type)
        allowed=sim_objectspecialproperty_collidable|sim_objectspecialproperty_measurable|sim_objectspecialproperty_detectable_all;
    if (_objectType==sim_object_joint_type)
        allowed=0;
    if (_objectType==sim_object_graph_type)
        allowed=sim_objectspecialproperty_renderable;
    if (_objectType==sim_object_path_type)
        allowed=sim_objectspecialproperty_renderable;
    if (_objectType==sim_object_proximitysensor_type)
        allowed=0;
    if (_objectType==sim_object_visionsensor_type)
        allowed=0;
    if (_objectType==sim_object_camera_type)
        allowed=0;
    if (_objectType==sim_object_light_type)
        allowed=0;
    if (_objectType==sim_object_mirror_type)
        allowed=sim_objectspecialproperty_renderable;
    if (_objectType==sim_object_mill_type)
        allowed=0;
    if (_objectType==sim_object_forcesensor_type)
        allowed=0;
    if (_objectType==sim_object_dummy_type)
        allowed=sim_objectspecialproperty_collidable|sim_objectspecialproperty_measurable|sim_objectspecialproperty_detectable_all|sim_objectspecialproperty_pathplanning_ignored;
        */
    _localObjectSpecialProperty&=allowed;
}

int C3DObject::getLocalObjectSpecialProperty() const
{ // returns the local value
    return(_localObjectSpecialProperty);
}

int C3DObject::getCumulativeObjectSpecialProperty()
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
    if (o&sim_modelproperty_not_cuttable)
        p=(p|sim_objectspecialproperty_cuttable)-sim_objectspecialproperty_cuttable;

    return(p);
}


void C3DObject::setLocalModelProperty(int prop)
{ // model properties are actually override properties. This func. returns the local value
    _localModelProperty=prop;
    incrementModelPropertyValidityNumber();
}

int C3DObject::getLocalModelProperty() const
{ // model properties are actually override properties. This func. returns the local value
    return(_localModelProperty);
}

int C3DObject::getCumulativeModelProperty()
{ // model properties are actually override properties. This func. returns the cumulative value
    int vn=_modelPropertyValidityNumber;
    if (vn!=_cumulativeModelPropertyValidityNumber)
    { // the cumulative value is not up-to-date
        if (getParentObject()==nullptr)
            _cumulativeModelProperty=_localModelProperty;
        else
        {
            int parentCumul=getParentObject()->getCumulativeModelProperty();
            _cumulativeModelProperty=_localModelProperty|parentCumul;
        }
        if (_dynamicsTemporarilyDisabled)
            _cumulativeModelProperty|=sim_modelproperty_not_dynamic;
        _cumulativeModelPropertyValidityNumber=vn;
    }
    return(_cumulativeModelProperty);
}

void C3DObject::incrementModelPropertyValidityNumber()
{ // static
    _modelPropertyValidityNumber++;
}


bool C3DObject::isObjectPartOfInvisibleModel()
{
    return((getCumulativeModelProperty()&sim_modelproperty_not_visible)!=0);
}

int C3DObject::getTreeDynamicProperty() // combination of sim_objdynprop_dynamic and sim_objdynprop_respondable
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

void C3DObject::getChildScriptsToRun_OLD(std::vector<int>& childScriptIDs)
{
    CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_child(getObjectHandle());
    if (it!=nullptr)
        childScriptIDs.push_back(it->getScriptID());
    else
    { // we have to explore the children:
        for (int i=0;i<int(childList.size());i++)
            childList[i]->getChildScriptsToRun_OLD(childScriptIDs);
    }
}

int C3DObject::getModelSelectionHandle(bool firstObject)
{ // firstObject is true by default
    if (CMiscBase::handleVerSpec_canCtrlShiftSelectObjects())
        return(getObjectHandle());

    if (_modelBase)
    {
        if ( ((_localObjectProperty&sim_objectproperty_selectmodelbaseinstead)==0) )
            return(getObjectHandle());
        if (getParentObject()==nullptr)
            return(getObjectHandle());
        int retV=getParentObject()->getModelSelectionHandle(false);
        if (retV==-1)
            retV=getObjectHandle();
        return(retV);
    }
    else
    {
        if (getParentObject()==nullptr)
        {
            if (firstObject)
                return(getObjectHandle());
            return(-1);
        }
        int retV=getParentObject()->getModelSelectionHandle(false);
        if (retV!=-1)
            return(retV);
        if (firstObject)
            return(getObjectHandle());
        return(-1);
    }
}

void C3DObject::clearManipulationModeOverlayGridFlag()
{
    _objectManipulationMode_flaggedForGridOverlay=0;
}

void C3DObject::scaleObject(float scalingFactor)
{
}

void C3DObject::scaleObjectNonIsometrically(float x,float y,float z)
{
}

void C3DObject::scaleObjectMain(float scalingFactor)
{
    _sizeFactor*=scalingFactor;
    _sizeValues[0]*=scalingFactor;
    _sizeValues[1]*=scalingFactor;
    _sizeValues[2]*=scalingFactor;
    incrementMemorizedConfigurationValidCounter();
    App::ct->drawingCont->adjustForScaling(_objectHandle,scalingFactor,scalingFactor,scalingFactor);
    App::ct->setModificationFlag(256); // object scaled
}

void C3DObject::scaleObjectNonIsometricallyMain(float x,float y,float z)
{
    _sizeFactor*=cbrt(x*y*z);
    _sizeValues[0]*=x;
    _sizeValues[1]*=y;
    _sizeValues[2]*=z;
    incrementMemorizedConfigurationValidCounter();
    App::ct->drawingCont->adjustForScaling(_objectHandle,x,y,z);
    App::ct->setModificationFlag(256); // object scaled
}

void C3DObject::scalePosition(float scalingFactor) 
{ // This routine will scale an object's position. The object itself keeps the same size.
    C7Vector local(getLocalTransformationPart1());
    setLocalTransformation(local.X*scalingFactor);
    _assemblingLocalTransformation.X=_assemblingLocalTransformation.X*scalingFactor;
}

void C3DObject::setSpecificLight(int h)
{
    _specificLight=h;
}

int C3DObject::getSpecificLight() const
{
    return(_specificLight);
}

bool C3DObject::getGlobalMarkingBoundingBox(const C7Vector& baseCoordInv,C3Vector& min,C3Vector& max,bool& minMaxNotYetDefined,bool first,bool guiIsRendering)
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
            ctm=getCumulativeTransformationPart1_forDisplay(guiIsRendering);
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
        for (size_t i=0;i<childList.size();i++)
        {
            if (childList[i]->getGlobalMarkingBoundingBox(baseCoordInv,min,max,minMaxNotYetDefined,first,guiIsRendering))
                retVal=true;
        }
    }
    return(retVal);
}

void C3DObject::getBoundingBoxEncompassingBoundingBox(const C7Vector& baseCoordInv,C3Vector& min,C3Vector& max,bool guiIsRendering)
{
    C3Vector smi,sma;
    getFullBoundingBox(smi,sma);
    bool minMaxNotYetDefined=true;
    C3Vector sm;
    C7Vector ctm;
    ctm=getCumulativeTransformationPart1_forDisplay(guiIsRendering);
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

void C3DObject::announceCollectionWillBeErased(int collectionID,bool copyBuffer)
{
}
void C3DObject::announceCollisionWillBeErased(int collisionID,bool copyBuffer)
{
}
void C3DObject::announceDistanceWillBeErased(int distanceID,bool copyBuffer)
{
}
void C3DObject::announceGcsObjectWillBeErased(int gcsObjectID,bool copyBuffer)
{
}

void C3DObject::performIkLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{
}
void C3DObject::performCollectionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{
}
void C3DObject::performCollisionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{
}
void C3DObject::performDistanceLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{
}
void C3DObject::performGcsLoadingMapping(std::vector<int>* map)
{
}

void C3DObject::performTextureObjectLoadingMapping(std::vector<int>* map)
{
}

void C3DObject::performDynMaterialObjectLoadingMapping(std::vector<int>* map)
{
}

void C3DObject::bufferMainDisplayStateVariables()
{
}

void C3DObject::bufferedMainDisplayStateVariablesToDisplay()
{
}

void C3DObject::bufferMainDisplayStateVariablesMain()
{
    _transformation_buffered=_transformation;
    _dynamicObjectFlag_forVisualization_buffered=_dynamicObjectFlag_forVisualization;
}

void C3DObject::bufferedMainDisplayStateVariablesToDisplayMain()
{
    _transformation_forDisplay=_transformation_buffered;
    _dynamicObjectFlag_forVisualization_forDisplay=_dynamicObjectFlag_forVisualization_buffered;
}

void C3DObject::announceCollectionWillBeErasedMain(int collectionID,bool copyBuffer)
{
    if (_authorizedViewableObjects==collectionID)
        _authorizedViewableObjects=-2; // not visible anymore!
    // This routine can be called for objCont-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'ct::objCont->getObject(id)'-call or similar
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

void C3DObject::announceCollisionWillBeErasedMain(int collisionID,bool copyBuffer)
{
    // This routine can be called for objCont-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'ct::objCont->getObject(id)'-call or similar
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
void C3DObject::announceDistanceWillBeErasedMain(int distanceID,bool copyBuffer)
{
    // This routine can be called for objCont-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'ct::objCont->getObject(id)'-call or similar
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
void C3DObject::announceGcsObjectWillBeErasedMain(int gcsObjectID,bool copyBuffer)
{
    // This routine can be called for objCont-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'ct::objCont->getObject(id)'-call or similar
    for (size_t i=0;i<_customReferencedHandles.size();i++)
    {
        if (_customReferencedHandles[i].generalObjectType==sim_appobj_constraintsolver_type)
        {
            if (_customReferencedHandles[i].generalObjectHandle==gcsObjectID)
                _customReferencedHandles[i].generalObjectHandle=-1;
        }
    }
}

void C3DObject::performIkLoadingMappingMain(std::vector<int>* map,bool loadingAmodel)
{
    for (size_t i=0;i<_customReferencedHandles.size();i++)
    {
        if (_customReferencedHandles[i].generalObjectType==sim_appobj_ik_type)
            _customReferencedHandles[i].generalObjectHandle=App::ct->objCont->getLoadingMapping(map,_customReferencedHandles[i].generalObjectHandle);
    }
    if (!loadingAmodel)
    {
        for (size_t i=0;i<_customReferencedOriginalHandles.size();i++)
        {
            if (_customReferencedOriginalHandles[i].generalObjectType==sim_appobj_ik_type)
                _customReferencedOriginalHandles[i].generalObjectHandle=App::ct->objCont->getLoadingMapping(map,_customReferencedOriginalHandles[i].generalObjectHandle);
        }
    }
}

void C3DObject::performCollectionLoadingMappingMain(std::vector<int>* map,bool loadingAmodel)
{
    if ( (_authorizedViewableObjects>=0)&&(_authorizedViewableObjects>=SIM_IDSTART_COLLECTION) )
        _authorizedViewableObjects=App::ct->objCont->getLoadingMapping(map,_authorizedViewableObjects);
    for (size_t i=0;i<_customReferencedHandles.size();i++)
    {
        if (_customReferencedHandles[i].generalObjectType==sim_appobj_collection_type)
            _customReferencedHandles[i].generalObjectHandle=App::ct->objCont->getLoadingMapping(map,_customReferencedOriginalHandles[i].generalObjectHandle);
    }
    if (!loadingAmodel)
    {
        for (size_t i=0;i<_customReferencedOriginalHandles.size();i++)
        {
            if (_customReferencedOriginalHandles[i].generalObjectType==sim_appobj_collection_type)
                _customReferencedOriginalHandles[i].generalObjectHandle=App::ct->objCont->getLoadingMapping(map,_customReferencedHandles[i].generalObjectHandle);
        }
    }
}

void C3DObject::performCollisionLoadingMappingMain(std::vector<int>* map,bool loadingAmodel)
{
    for (size_t i=0;i<_customReferencedHandles.size();i++)
    {
        if (_customReferencedHandles[i].generalObjectType==sim_appobj_collision_type)
            _customReferencedHandles[i].generalObjectHandle=App::ct->objCont->getLoadingMapping(map,_customReferencedHandles[i].generalObjectHandle);
    }
    if (!loadingAmodel)
    {
        for (size_t i=0;i<_customReferencedOriginalHandles.size();i++)
        {
            if (_customReferencedOriginalHandles[i].generalObjectType==sim_appobj_collision_type)
                _customReferencedOriginalHandles[i].generalObjectHandle=App::ct->objCont->getLoadingMapping(map,_customReferencedOriginalHandles[i].generalObjectHandle);
        }
    }
}

void C3DObject::performDistanceLoadingMappingMain(std::vector<int>* map,bool loadingAmodel)
{
    for (size_t i=0;i<_customReferencedHandles.size();i++)
    {
        if (_customReferencedHandles[i].generalObjectType==sim_appobj_distance_type)
            _customReferencedHandles[i].generalObjectHandle=App::ct->objCont->getLoadingMapping(map,_customReferencedHandles[i].generalObjectHandle);
    }
    if (!loadingAmodel)
    {
        for (size_t i=0;i<_customReferencedOriginalHandles.size();i++)
        {
            if (_customReferencedOriginalHandles[i].generalObjectType==sim_appobj_distance_type)
                _customReferencedOriginalHandles[i].generalObjectHandle=App::ct->objCont->getLoadingMapping(map,_customReferencedOriginalHandles[i].generalObjectHandle);
        }
    }
}

void C3DObject::performGcsLoadingMappingMain(std::vector<int>* map)
{
    for (size_t i=0;i<_customReferencedHandles.size();i++)
    {
        if (_customReferencedHandles[i].generalObjectType==sim_appobj_constraintsolver_type)
            _customReferencedHandles[i].generalObjectHandle=App::ct->objCont->getLoadingMapping(map,_customReferencedHandles[i].generalObjectHandle);
    }
}

void C3DObject::performTextureObjectLoadingMappingMain(std::vector<int>* map)
{
}

void C3DObject::performDynMaterialObjectLoadingMappingMain(std::vector<int>* map)
{
}

void C3DObject::getAllShapeObjectsRecursive(std::vector<CShape*>* shapeList,bool baseIncluded,bool start) const
{   // baseIncluded and start are true by default.
    // Returns all shape-objects build on this (including children of children of...)
    if ( (baseIncluded||(!start))&&(getObjectType()==sim_object_shape_type) )
        shapeList->push_back((CShape*)this);
    for (size_t i=0;i<childList.size();i++)
        childList[i]->getAllShapeObjectsRecursive(shapeList,true,false);
}

void C3DObject::getAllObjectsRecursive(std::vector<C3DObject*>* objectList,bool baseIncluded,bool start) const
{   // baseIncluded and start are true by default.
    // Returns all objects build on this (including children of children of...)
    if (baseIncluded||(!start))
        objectList->push_back((C3DObject*)this);
    for (size_t i=0;i<childList.size();i++)
        childList[i]->getAllObjectsRecursive(objectList,true,false);
}

void C3DObject::getChain(std::vector<C3DObject*>& objectList,bool tipIncluded,bool start) const
{   // tipIncluded and start are true by default.
    // Returns the chain with this object as tip
    if (tipIncluded||(!start))
        objectList.push_back((C3DObject*)this);
    if (getParentObject()!=nullptr)
        getParentObject()->getChain(objectList,true,false);
}

void C3DObject::setObjectType(int theNewType)
{ // Be very careful with this function!!
    _objectType=theNewType;
}

C3DObject* C3DObject::getFirstParentInSelection(const std::vector<C3DObject*>* sel) const
{
    C3DObject* it=getParentObject();
    if (it==nullptr) 
        return(nullptr);
    for (size_t i=0;i<sel->size();i++)
    {
        if (sel->at(i)==it) 
            return(it);
    }
    return(it->getFirstParentInSelection(sel));
}

C3DObject* C3DObject::getLastParentInSelection(const std::vector<C3DObject*>* sel) const
{
    C3DObject* it=getParentObject();
    C3DObject* retVal=nullptr;
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
        it=it->getParentObject();
    }
    return(retVal);
}

void C3DObject::removeSceneDependenciesMain()
{
    _customReferencedHandles.clear();
    _customReferencedOriginalHandles.clear();
}

C3DObject* C3DObject::copyYourselfMain()
{ // This routine should be called in the very first line of function
  // "copyYourself" in every joint, camera, etc. !

    C3DObject* theNewObject=nullptr;
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

    theNewObject->_authorizedViewableObjects=_authorizedViewableObjects;
    theNewObject->layer=layer;
    theNewObject->_transformation=_transformation;
    theNewObject->_objectHandle=_objectHandle;
    theNewObject->_objectName=_objectName;
    theNewObject->_objectAltName=_objectAltName;
    theNewObject->_localObjectProperty=_localObjectProperty;
    theNewObject->_hierarchyColorIndex=_hierarchyColorIndex;
    theNewObject->_collectionSelfCollisionIndicator=_collectionSelfCollisionIndicator;
    theNewObject->_modelBase=_modelBase;
    theNewObject->_parentHandle=_parentHandle;
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

    theNewObject->_customReferencedHandles.assign(_customReferencedHandles.begin(),_customReferencedHandles.end());
    theNewObject->_customReferencedOriginalHandles.assign(_customReferencedOriginalHandles.begin(),_customReferencedOriginalHandles.end());


    // The following line is important to properly copy the copy buffer!!!
    theNewObject->setParentHandleLoading(getParentHandle_loading());

    return(theNewObject);
}

void C3DObject::setObjectCustomData(int header,const char* data,int dataLength)
{
    if (_customObjectData==nullptr)
        _customObjectData=new CCustomData();
    _customObjectData->setData(header,data,dataLength);
}
int C3DObject::getObjectCustomDataLength(int header)
{
    if (_customObjectData==nullptr)
        return(0);
    return(_customObjectData->getDataLength(header));
}
void C3DObject::getObjectCustomData(int header,char* data)
{
    if (_customObjectData==nullptr)
        return;
    _customObjectData->getData(header,data);
}

bool C3DObject::getObjectCustomDataHeader(int index,int& header)
{
    if (_customObjectData==nullptr)
        return(false);
    return(_customObjectData->getHeader(index,header));
}

void C3DObject::setObjectCustomData_tempData(int header,const char* data,int dataLength)
{
    if (_customObjectData_tempData==nullptr)
        _customObjectData_tempData=new CCustomData();
    _customObjectData_tempData->setData(header,data,dataLength);
}
int C3DObject::getObjectCustomDataLength_tempData(int header)
{
    if (_customObjectData_tempData==nullptr)
        return(0);
    return(_customObjectData_tempData->getDataLength(header));
}
void C3DObject::getObjectCustomData_tempData(int header,char* data)
{
    if (_customObjectData_tempData==nullptr)
        return;
    _customObjectData_tempData->getData(header,data);
}

bool C3DObject::getObjectCustomDataHeader_tempData(int index,int& header)
{
    if (_customObjectData_tempData==nullptr)
        return(false);
    return(_customObjectData_tempData->getHeader(index,header));
}

void C3DObject::setObjectTranslationDisabledDuringSimulation(bool d)
{
    _objectTranslationDisabledDuringSimulation=d;
}

bool C3DObject::getObjectTranslationDisabledDuringSimulation() const
{
    return(_objectTranslationDisabledDuringSimulation);
}

void C3DObject::setObjectTranslationDisabledDuringNonSimulation(bool d)
{
    _objectTranslationDisabledDuringNonSimulation=d;
}

bool C3DObject::getObjectTranslationDisabledDuringNonSimulation() const
{
    return(_objectTranslationDisabledDuringNonSimulation);
}

void C3DObject::setObjectTranslationSettingsLocked(bool l)
{
    _objectTranslationSettingsLocked=l;
}

bool C3DObject::getObjectTranslationSettingsLocked()
{
    return(_objectTranslationSettingsLocked);
}

void C3DObject::setObjectRotationDisabledDuringSimulation(bool d)
{
    _objectRotationDisabledDuringSimulation=d;
}

bool C3DObject::getObjectRotationDisabledDuringSimulation() const
{
    return(_objectRotationDisabledDuringSimulation);
}

void C3DObject::setObjectRotationDisabledDuringNonSimulation(bool d)
{
    _objectRotationDisabledDuringNonSimulation=d;
}

bool C3DObject::getObjectRotationDisabledDuringNonSimulation() const
{
    return(_objectRotationDisabledDuringNonSimulation);
}

void C3DObject::setObjectRotationSettingsLocked(bool l)
{
    _objectRotationSettingsLocked=l;
}

bool C3DObject::getObjectRotationSettingsLocked()
{
    return(_objectRotationSettingsLocked);
}


void C3DObject::setObjectManipulationModePermissions(int p)
{ // bits 0-2: position x,y,z (relative to parent frame), bits 3-5: Euler e9,e1,e2 (relative to own frame) 
    p&=0x3f;
    _objectManipulationModePermissions=p;
}

int C3DObject::getObjectManipulationModePermissions() const
{ // bits 0-2: position x,y,z (relative to parent frame), bits 3-5: Euler e9,e1,e2 (relative to own frame) 
    return(_objectManipulationModePermissions);
}

void C3DObject::setObjectManipulationTranslationRelativeTo(int p)
{
    _objectManipulationTranslationRelativeTo=p;
}

int C3DObject::getObjectManipulationTranslationRelativeTo() const
{
    return(_objectManipulationTranslationRelativeTo);
}

void C3DObject::setObjectManipulationRotationRelativeTo(int p)
{
    _objectManipulationRotationRelativeTo=p;
}

int C3DObject::getObjectManipulationRotationRelativeTo() const
{
    return(_objectManipulationRotationRelativeTo);
}

void C3DObject::setNonDefaultTranslationStepSize(float s)
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

float C3DObject::getNonDefaultTranslationStepSize() const
{
    return(_objectTranslationNonDefaultStepSize);
}

void C3DObject::setNonDefaultRotationStepSize(float s)
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

float C3DObject::getNonDefaultRotationStepSize() const
{
    return(_objectRotationNonDefaultStepSize);
}

void C3DObject::setMechanismID(int id)
{
    _mechanismID=id;
}

int C3DObject::getMechanismID() const
{ // Make sure setMechanismID was called just before!!
    return (_mechanismID);
}

void C3DObject::simulationAboutToStart()
{ // not called, see the ...Main routine instead
}

void C3DObject::simulationEnded()
{ // not called, see the ...Main routine instead
}

void C3DObject::initializeInitialValues(bool simulationIsRunning)
{ // not called, see the ...Main routine instead
}

void C3DObject::initializeInitialValuesMain(bool simulationIsRunning)
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
//    _previousAbsTransf_velocityMeasurement=getCumulativeTransformationPart1();

    if (simulationIsRunning)
    {
        // this section is special and reserved to local configuration restoration!
        //********************************
        _initialConfigurationMemorized=true;
        _initialMemorizedConfigurationValidCounter=_memorizedConfigurationValidCounter;
        _initialParentUniqueID=-1; // -1 means there was no parent at begin
        C3DObject* p=getParentObject();
        if (p!=nullptr)
            _initialParentUniqueID=p->getUniqueID();
        _initialLocalTransformationPart1=_transformation;
        //********************************

        _initialMainPropertyOverride=_localModelProperty;
    }
}

void C3DObject::simulationEndedMain()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    _dynamicSimulationIconCode=sim_dynamicsimicon_none;
    _dynamicObjectFlag_forVisualization=0;
    if (_initialValuesInitializedMain&&App::ct->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
    {
        if (_initialConfigurationMemorized)
        { // this section is special and reserved to local configuration restoration!
            if (_initialMemorizedConfigurationValidCounter==_memorizedConfigurationValidCounter)
            { // the object wasn't resized/didn't change frame
                int puid=-1;
                C3DObject* p=getParentObject();
                if (p!=nullptr)
                    puid=p->getUniqueID();
                // Changed following on 24/04/2011 (because we also wanna reset the parenting to the initial state!)
                if (puid!=_initialParentUniqueID)
                { // Not sure following instructions are not problematic here.
                    C3DObject* oldParent=App::ct->objCont->getObjectWithUniqueID(_initialParentUniqueID);
                    if ( (oldParent!=nullptr)||(_initialParentUniqueID==-1) )
                    {
                        // Inverted following 2 lines on 24/2/2012:
                        App::ct->objCont->makeObjectChildOf(this,oldParent);
                        _transformation=_initialLocalTransformationPart1;
                    }
                }
                else
                    _transformation=_initialLocalTransformationPart1;
            }
            _localModelProperty=_initialMainPropertyOverride;
            _initialConfigurationMemorized=false;
        }
    }
    _initialValuesInitializedMain=false;
}

bool C3DObject::getFullBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    return(false);
}

bool C3DObject::getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    return(getFullBoundingBox(minV,maxV));
}

void C3DObject::disableDynamicTreeForManipulation(bool d)
{
    if (d!=_dynamicsTemporarilyDisabled)
        incrementModelPropertyValidityNumber(); // we want the cumulative values all recalculated
    _dynamicsTemporarilyDisabled=d;
}

void C3DObject::setDynamicSimulationIconCode(int c)
{
    if (c!=_dynamicSimulationIconCode)
        App::setRefreshHierarchyViewFlag();
    _dynamicSimulationIconCode=c;
}

int C3DObject::getDynamicSimulationIconCode() const
{
    return(_dynamicSimulationIconCode);
}

void C3DObject::setAssemblingLocalTransformation(const C7Vector& tr)
{
    _assemblingLocalTransformation=tr;
}
C7Vector C3DObject::getAssemblingLocalTransformation() const
{
    return(_assemblingLocalTransformation);
}
void C3DObject::setAssemblingLocalTransformationIsUsed(bool u)
{
    _assemblingLocalTransformationIsUsed=u;
}

bool C3DObject::getAssemblingLocalTransformationIsUsed()
{
    return(_assemblingLocalTransformationIsUsed);
}


void C3DObject::setEnableCustomizationScript(bool c,const char* scriptContent)
{
    // _customizationScriptEnabled=c;

    // We remove a script that might be associated:
    CLuaScriptObject* script=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_customization(getObjectHandle());
    if (script)
        App::ct->luaScriptContainer->removeScript(script->getScriptID());

    if (c)
    { // we have to add a script
        CLuaScriptObject* script=new CLuaScriptObject(sim_scripttype_customizationscript);
        if (scriptContent)
            script->setScriptText(scriptContent);
        App::ct->luaScriptContainer->insertScript(script);
        script->setObjectIDThatScriptIsAttachedTo(getObjectHandle());
    }
}

bool C3DObject::getEnableCustomizationScript()
{
    return(App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_customization(getObjectHandle())!=nullptr);
//  return(_customizationScriptEnabled);
}

void C3DObject::setAssemblyMatchValues(bool asChild,const char* str)
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

std::string C3DObject::getAssemblyMatchValues(bool asChild) const
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

const std::vector<std::string>* C3DObject::getChildAssemblyMatchValuesPointer() const
{
    return(&_assemblyMatchValuesChild);
}

bool C3DObject::doesParentAssemblingMatchValuesMatchWithChild(const std::vector<std::string>* assemblingChildMatchValues,bool ignoreDefaultNames/*=false*/) const
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

int C3DObject::getAllChildrenThatMayBecomeAssemblyParent(const std::vector<std::string>* assemblingChildMatchValues,std::vector<C3DObject*>& objects) const
{
    for (size_t i=0;i<childList.size();i++)
    {
        if (childList[i]->doesParentAssemblingMatchValuesMatchWithChild(assemblingChildMatchValues,true))
            objects.push_back(childList[i]);
        childList[i]->getAllChildrenThatMayBecomeAssemblyParent(assemblingChildMatchValues,objects);
    }
    return(int(objects.size()));
}

void C3DObject::generateDnaString()
{
    _dnaString=CTTUtil::generateUniqueString();
}

std::string C3DObject::getDnaString() const
{
    return(_dnaString);
}

std::string C3DObject::getUniquePersistentIdString() const
{
    return(_uniquePersistentIdString);
}

std::string C3DObject::getExtensionString() const
{
    return(_extensionString);
}

void C3DObject::setExtensionString(const char* str)
{
    _extensionString=str;
}

int C3DObject::getScriptExecutionOrder(int scriptType) const
{
    if (scriptType==sim_scripttype_customizationscript)
    {
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_customization(_objectHandle);
        if (it!=nullptr)
            return(it->getExecutionOrder());
    }
    else if ((scriptType&sim_scripttype_childscript)!=0)
    {
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_child(_objectHandle);
        if (it!=nullptr)
        {
            if ( it->getThreadedExecution()==((scriptType&sim_scripttype_threaded)!=0) )
                return(it->getExecutionOrder());
        }
    }
    return(sim_scriptexecorder_normal);
}

int C3DObject::getScriptsToExecute(int scriptType,int parentTraversalDirection,std::vector<CLuaScriptObject*>& scripts,std::vector<int>& uniqueIds)
{
    int cnt=0;
    CLuaScriptObject* attachedScript=nullptr;
    if (scriptType==sim_scripttype_customizationscript)
        attachedScript=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_customization(_objectHandle);
    else if ((scriptType&sim_scripttype_childscript)!=0)
    {
        attachedScript=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_child(_objectHandle);
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

        std::vector<C3DObject*> orderFirst;
        std::vector<C3DObject*> orderNormal;
        std::vector<C3DObject*> orderLast;
        std::vector<std::vector<C3DObject*>* > toHandle;
        toHandle.push_back(&orderFirst);
        toHandle.push_back(&orderNormal);
        toHandle.push_back(&orderLast);
        for (size_t i=0;i<childList.size();i++)
        {
            C3DObject* it=childList[i];
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

void C3DObject::serialize(CSer& ar)
{
}

void C3DObject::serializeMain(CSer& ar)
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
            C7Vector tr=getLocalTransformationPart1();
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
            if (getParentObject()!=nullptr)
                parentID=getParentObject()->getObjectHandle();
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

            // Keep a while for backward compatibility (19/4/2017) (in case people want to return to a previous V-REP version):
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
            ar << layer;
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
                        ar >> _objectHandle >> _parentHandle;
                        _parentObject=nullptr;
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
                        ar >> layer;
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
                if (layer<256)
                    layer=layer*256;
            }
            else
                _localObjectProperty-=sim_objectproperty_reserved5;
            //*************************************************************

            //*************************************************************
            // For old models to support the DNA-thing by default:
            if ( (ar.getVrepVersionThatWroteThisFile()<30003)&&getModelBase() )
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
}


void C3DObject::serializeWExtIk(CExtIkSer& ar)
{
}

void C3DObject::serializeWExtIkMain(CExtIkSer& ar)
{
    C7Vector tr=getLocalTransformationPart1();
    ar.writeFloat(tr.Q(0));
    ar.writeFloat(tr.Q(1));
    ar.writeFloat(tr.Q(2));
    ar.writeFloat(tr.Q(3));
    ar.writeFloat(tr.X(0));
    ar.writeFloat(tr.X(1));
    ar.writeFloat(tr.X(2));
    
    int parentID=-1;
    if (getParentObject()!=nullptr)
        parentID=getParentObject()->getObjectHandle();
    ar.writeInt(_objectHandle);
    ar.writeInt(parentID);

    ar.writeString(_objectName.c_str());
}

int C3DObject::_uniqueIDCounter=0;

void C3DObject::performObjectLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{
}

void C3DObject::performObjectLoadingMappingMain(std::vector<int>* map,bool loadingAmodel)
{
    int newParentID=App::ct->objCont->getLoadingMapping(map,_parentHandle);
    setParentObject(App::ct->objCont->getObjectFromHandle(newParentID));

    if ( (_authorizedViewableObjects>=0)&&(_authorizedViewableObjects<SIM_IDSTART_COLLECTION) )
        _authorizedViewableObjects=App::ct->objCont->getLoadingMapping(map,_authorizedViewableObjects);

    for (size_t i=0;i<_customReferencedHandles.size();i++)
    {
        if (_customReferencedHandles[i].generalObjectType==sim_appobj_object_type)
            _customReferencedHandles[i].generalObjectHandle=App::ct->objCont->getLoadingMapping(map,_customReferencedHandles[i].generalObjectHandle);
    }
    if (!loadingAmodel)
    {
        for (size_t i=0;i<_customReferencedOriginalHandles.size();i++)
        {
            if (_customReferencedOriginalHandles[i].generalObjectType==sim_appobj_object_type)
                _customReferencedOriginalHandles[i].generalObjectHandle=App::ct->objCont->getLoadingMapping(map,_customReferencedOriginalHandles[i].generalObjectHandle);
        }
    }
}

std::string C3DObject::getObjectName() const
{
    return(_objectName);
}
void C3DObject::setObjectName_objectNotYetInScene(std::string newName)
{ 
    _objectName=newName;
}

std::string C3DObject::getObjectAltName() const
{
    return(_objectAltName);
}
void C3DObject::setObjectAltName_objectNotYetInScene(std::string newAltName)
{
    _objectAltName=newAltName;
}
std::string C3DObject::getDisplayName() const
{
    if (CMiscBase::handleVerSpec_isDisplayNameAltName())
        return(_objectAltName);
    return(_objectName);
}

bool C3DObject::announceObjectWillBeErased(int objHandle,bool copyBuffer)
{
    return(false);
}
void C3DObject::announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer)
{
}

bool C3DObject::announceObjectWillBeErasedMain(int objHandle,bool copyBuffer)
{ 
    // This routine can be called for objCont-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'ct::objCont->getObject(objHandle)'-call or similar
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
    if ( (getParentObject()!=nullptr)&&(!copyBuffer) )
    {
        if (getParentObject()->getObjectHandle()==objHandle)
            App::ct->objCont->makeObjectChildOf(this,getParentObject()->getParentObject());
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
void C3DObject::announceIkObjectWillBeErasedMain(int ikGroupID,bool copyBuffer)
{
    // This routine can be called for objCont-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'ct::objCont->getObject(id)'-call or similar
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

void C3DObject::setReferencedHandles(int cnt,const int* handles)
{
    _customReferencedHandles.clear();
    for (int i=0;i<cnt;i++)
    {
        SCustomRefs r;
        r.generalObjectType=sim_appobj_object_type;
        r.generalObjectHandle=-1;
        if (handles[i]>=0)
        {
            if (App::ct->objCont->getObjectFromHandle(handles[i])!=nullptr)
                r.generalObjectHandle=handles[i];
            else
            {
                if (App::ct->collisions->getObject(handles[i])!=nullptr)
                {
                    r.generalObjectType=sim_appobj_collision_type;
                    r.generalObjectHandle=handles[i];
                }
                if (App::ct->distances->getObject(handles[i])!=nullptr)
                {
                    r.generalObjectType=sim_appobj_distance_type;
                    r.generalObjectHandle=handles[i];
                }
                if (App::ct->ikGroups->getIkGroup(handles[i])!=nullptr)
                {
                    r.generalObjectType=sim_appobj_ik_type;
                    r.generalObjectHandle=handles[i];
                }
                if (App::ct->constraintSolver->getObject(handles[i])!=nullptr)
                {
                    r.generalObjectType=sim_appobj_constraintsolver_type;
                    r.generalObjectHandle=handles[i];
                }
                if (App::ct->collections->getCollection(handles[i])!=nullptr)
                {
                    r.generalObjectType=sim_appobj_collection_type;
                    r.generalObjectHandle=handles[i];
                }
            }
        }
        _customReferencedHandles.push_back(r);
    }
}

int C3DObject::getReferencedHandlesCount()
{
    return(int(_customReferencedHandles.size()));
}

int C3DObject::getReferencedHandles(int* handles)
{
    for (size_t i=0;i<_customReferencedHandles.size();i++)
        handles[i]=_customReferencedHandles[i].generalObjectHandle;
    return(int(_customReferencedHandles.size()));
}


void C3DObject::setReferencedOriginalHandles(int cnt,const int* handles)
{
    _customReferencedOriginalHandles.clear();
    for (int i=0;i<cnt;i++)
    {
        SCustomOriginalRefs r;
        r.generalObjectType=sim_appobj_object_type;
        r.generalObjectHandle=-1;
        if (handles[i]>=0)
        {
            if (App::ct->objCont->getObjectFromHandle(handles[i])!=nullptr)
            {
                r.generalObjectHandle=handles[i];
                r.uniquePersistentIdString=App::ct->objCont->getObjectFromHandle(handles[i])->getUniquePersistentIdString();
            }
            else
            {
                if (App::ct->collisions->getObject(handles[i])!=nullptr)
                {
                    r.generalObjectType=sim_appobj_collision_type;
                    r.generalObjectHandle=handles[i];
                    r.uniquePersistentIdString=App::ct->collisions->getObject(handles[i])->getUniquePersistentIdString();
                }
                if (App::ct->distances->getObject(handles[i])!=nullptr)
                {
                    r.generalObjectType=sim_appobj_distance_type;
                    r.generalObjectHandle=handles[i];
                    r.uniquePersistentIdString=App::ct->distances->getObject(handles[i])->getUniquePersistentIdString();
                }
                if (App::ct->ikGroups->getIkGroup(handles[i])!=nullptr)
                {
                    r.generalObjectType=sim_appobj_ik_type;
                    r.generalObjectHandle=handles[i];
                    r.uniquePersistentIdString=App::ct->ikGroups->getIkGroup(handles[i])->getUniquePersistentIdString();
                }
                if (App::ct->collections->getCollection(handles[i])!=nullptr)
                {
                    r.generalObjectType=sim_appobj_collection_type;
                    r.generalObjectHandle=handles[i];
                    r.uniquePersistentIdString=App::ct->collections->getCollection(handles[i])->getUniquePersistentIdString();
                }
            }
        }
        _customReferencedOriginalHandles.push_back(r);
    }
}

int C3DObject::getReferencedOriginalHandlesCount()
{
    return(int(_customReferencedOriginalHandles.size()));
}

int C3DObject::getReferencedOriginalHandles(int* handles)
{
    for (size_t i=0;i<_customReferencedOriginalHandles.size();i++)
        handles[i]=_customReferencedOriginalHandles[i].generalObjectHandle;
    return(int(_customReferencedOriginalHandles.size()));
}

void C3DObject::checkReferencesToOriginal(const std::map<std::string,int>& allUniquePersistentIdStrings)
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

void C3DObject::getCumulativeTransformationMatrix(float m[4][4],bool useTempValues) const
{ // useTempValues is false by default. This is used by the IK-routine
    getCumulativeTransformation(useTempValues).copyTo(m);
}

void C3DObject::getCumulativeTransformationMatrixPart1(float m[4][4],bool useTempValues) const
{ // useTempValues is false by default. This is used by the IK-routine
    getCumulativeTransformationPart1(useTempValues).copyTo(m);
}

C7Vector C3DObject::getParentCumulativeTransformation(bool useTempValues) const
{ // useTempValues is false by default. This is used by the IK-routine
    if (getParentObject()==nullptr)
    {
        C7Vector retV;
        retV.setIdentity();
        return(retV);
    }
    else
        return(getParentObject()->getCumulativeTransformation(useTempValues));
}

C7Vector C3DObject::getParentCumulativeTransformation_forDisplay(bool guiIsRendering) const
{
    if (getParentObject()==nullptr)
    {
        C7Vector retV;
        retV.setIdentity();
        return(retV);
    }
    else
        return(getParentObject()->getCumulativeTransformation_forDisplay(guiIsRendering));
}


C7Vector C3DObject::getCumulativeTransformation(bool useTempValues) const
{ // useTempValues is false by default. This is used by the IK-routine
    if (getParentObject()==nullptr)
        return(getLocalTransformation(useTempValues));
    else
        return(getParentCumulativeTransformation(useTempValues)*getLocalTransformation(useTempValues));
}
C7Vector C3DObject::getCumulativeTransformation_forDisplay(bool guiIsRendering) const
{
    if (getParentObject()==nullptr)
        return(getLocalTransformation_forDisplay(guiIsRendering));
    else
        return(getParentCumulativeTransformation_forDisplay(guiIsRendering)*getLocalTransformation_forDisplay(guiIsRendering));
}

C7Vector C3DObject::getLocalTransformation(bool useTempValues) const
{ // useTempValues is false by default. This is used by the IK-routine
    if (getObjectType()==sim_object_joint_type)
    {
        CJoint* it=(CJoint*)this;
        C7Vector jointTr;
        jointTr.setIdentity();
        float val;
        val=it->getPosition(useTempValues);
        if (it->getJointType()==sim_joint_revolute_subtype)
        {
            jointTr.Q.setAngleAndAxis(val,C3Vector(0.0f,0.0f,1.0f));
            jointTr.X(2)=val*it->getScrewPitch();
        }
        if (it->getJointType()==sim_joint_prismatic_subtype)
            jointTr.X(2)=val;
        if (it->getJointType()==sim_joint_spherical_subtype)
        {
            if (useTempValues)
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
            else
                jointTr.Q=it->getSphericalTransformation();
        }
        return(_transformation*jointTr);
    }
    else if (getObjectType()==sim_object_dummy_type)
    {
        CDummy* it=(CDummy*)this;
        if (useTempValues)
            return(it->getTempLocalTransformation()); // used for IK when dummy is freely sliding on a path object
        return(_transformation);
    }
    else
        return(_transformation);
}
C7Vector C3DObject::getLocalTransformation_forDisplay(bool guiIsRendering) const
{
    if (getObjectType()==sim_object_joint_type)
    {
        CJoint* it=(CJoint*)this;
        C7Vector jointTr;
        jointTr.setIdentity();
        float val;
        val=it->getPosition_forDisplay(guiIsRendering);
        if (it->getJointType()==sim_joint_revolute_subtype)
        {
            jointTr.Q.setAngleAndAxis(val,C3Vector(0.0f,0.0f,1.0f));
            jointTr.X(2)=val*it->getScrewPitch();
        }
        if (it->getJointType()==sim_joint_prismatic_subtype)
            jointTr.X(2)=val;
        if (it->getJointType()==sim_joint_spherical_subtype)
            jointTr.Q=it->getSphericalTransformation_forDisplay(guiIsRendering);
        if (guiIsRendering)
            return(_transformation_forDisplay*jointTr);
        else
            return(_transformation*jointTr);
    }
    if (guiIsRendering)
        return(_transformation_forDisplay);
    else
        return(_transformation);
}

C7Vector C3DObject::getCumulativeTransformationPart1(bool useTempValues) const
{ // useTempValues is false by default. This is used by the IK-routine
    if (getObjectType()==sim_object_joint_type)
    {
        if (getParentObject()==nullptr)
            return(getLocalTransformationPart1(useTempValues));
        else
            return(getParentCumulativeTransformation(useTempValues)*getLocalTransformationPart1(useTempValues));
    }
    else
        return(getCumulativeTransformation(useTempValues));
}

C7Vector C3DObject::getCumulativeTransformationPart1_forDisplay(bool guiIsRendering) const
{ 
    if (getObjectType()==sim_object_joint_type)
    {
        if (getParentObject()==nullptr)
            return(getLocalTransformationPart1_forDisplay(guiIsRendering));
        else
            return(getParentCumulativeTransformation_forDisplay(guiIsRendering)*getLocalTransformationPart1_forDisplay(guiIsRendering));
    }
    else
        return(getCumulativeTransformation_forDisplay(guiIsRendering));
}

C7Vector C3DObject::getLocalTransformationPart1(bool useTempValues) const
{ // useTempValues is false by default. This is used by the IK-routine
    return(_transformation);
}

C7Vector C3DObject::getLocalTransformationPart1_forDisplay(bool guiIsRendering) const
{
    if (guiIsRendering)
        return(_transformation_forDisplay);
    else
        return(_transformation);
}

void C3DObject::setLocalTransformation(const C7Vector& v)
{
    _transformation=v;
}

void C3DObject::setLocalTransformation(const C4Vector& q)
{
    _transformation.Q=q;
}
void C3DObject::setLocalTransformation(const C3Vector& x)
{
    _transformation.X=x;
}
void C3DObject::setAbsoluteTransformation(const C7Vector& v)
{
    C7Vector pInv(getParentCumulativeTransformation().getInverse());
    _transformation.setMultResult(pInv,v);
}
void C3DObject::setAbsoluteTransformation(const C4Vector& q)
{
    C7Vector pInv(getParentCumulativeTransformation().getInverse());
    _transformation.Q=pInv.Q*q;
}
void C3DObject::setAbsoluteTransformation(const C3Vector& x)
{
    C7Vector pInv(getParentCumulativeTransformation().getInverse());
    _transformation.X=pInv*x;
}

int C3DObject::getObjectHandle() const
{
    return(_objectHandle);
}

void C3DObject::setObjectHandle(int newObjectHandle)
{ // Be very careful with this function!!!
    _objectHandle=newObjectHandle;
}

bool C3DObject::isObjectParentedWith(C3DObject* thePotentialParent) const
{
    if (getParentObject()==nullptr)
        return(false);
    if (getParentObject()==thePotentialParent)
        return(true);
    return(getParentObject()->isObjectParentedWith(thePotentialParent));
}

int C3DObject::getParentHandle_loading() const
{
    return(_parentHandle);
}

void C3DObject::setParentHandleLoading(int pHandle)
{ // Be very careful with this function!!!
    _parentHandle=pHandle;
}

void C3DObject::setParentObject(C3DObject* newParent,bool actualizeInfo)
{ // actualizeInfo is true by default
    if (newParent==this)
        return;
    EASYLOCK(_objectMutex);
    _parentObject=newParent;
    if (actualizeInfo) 
        App::ct->objCont->actualizeObjectInformation();
}

C3DObject* C3DObject::getParentObject() const
{
    return(_parentObject);
}

int C3DObject::getFirstModelRelatives(std::vector<C3DObject*>& firstModelRelatives,bool visibleModelsOnly) const
{
    int cnt=0;
    for (size_t i=0;i<childList.size();i++)
    {
        C3DObject* child=childList[i];
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

int C3DObject::countFirstModelRelatives(bool visibleModelsOnly) const
{
    int cnt=0;
    for (size_t i=0;i<childList.size();i++)
    {
        C3DObject* child=childList[i];
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

int C3DObject::getObjectType() const
{
    return(_objectType);
}

void C3DObject::setRestoreToDefaultLights(bool s)
{
    _restoreToDefaultLights=s;
}

bool C3DObject::getRestoreToDefaultLights() const
{
    return(_restoreToDefaultLights);
}

void C3DObject::display(CViewableBase* renderingObject,int displayAttrib)
{
}

#ifdef SIM_WITH_GUI
void C3DObject::displayManipulationModeOverlayGrid(bool transparentAndOverlay)
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
        sizeValueForPath=SIM_MAX((maxCoord-minCoord).getLength()/3.0f,pc->getSquareSize()*2.0f);
    }

    C4X4Matrix tr;
    int axisInfo;
    if (_objectManipulationMode_flaggedForGridOverlay&8)
    { // rotation.
        axisInfo=_objectManipulationMode_flaggedForGridOverlay-8;
        tr=getCumulativeTransformationPart1().getMatrix();

        if (getObjectManipulationRotationRelativeTo()==0) // world
            tr.M.setIdentity();
        if (getObjectManipulationRotationRelativeTo()==1) // parent frame
            tr.M=getParentCumulativeTransformation().getMatrix().M;
        if ((getObjectManipulationRotationRelativeTo()==2)||isPathPoints) // own frame
            tr.M=getCumulativeTransformationPart1().getMatrix().M;
    }
    else
    { // translation
        axisInfo=_objectManipulationMode_flaggedForGridOverlay-16;
        tr=getCumulativeTransformationPart1().getMatrix();
        if (getObjectManipulationTranslationRelativeTo()==0) // world
            tr.M.setIdentity();
        if (getObjectManipulationTranslationRelativeTo()==1) // parent frame
            tr.M=getParentCumulativeTransformation().getMatrix().M;
        if ((getObjectManipulationTranslationRelativeTo()==2)||isPathPoints) // own frame
            tr.M=getCumulativeTransformationPart1().getMatrix().M;
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
            ctmi=getCumulativeTransformationPart1().getInverse(); // actually maybe tr.getInverse would even be better?

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
            halfSize=1.5f*SIM_MAX(bbs(1),bbs(2))/2.0f;
        }
        if (axisInfo==1)
        { // rotation around the y-axis
            rot.buildXRotation(-piValD2_f);
            bbsavg(0)=0.0f;
            bbsavg(2)=0.0f;
            halfSize=1.5f*SIM_MAX(bbs(0),bbs(2))/2.0f;
        }
        if (axisInfo==2)
        { // rotation around the z-axis
            rot.setIdentity();
            bbsavg(0)=0.0f;
            bbsavg(1)=0.0f;
            halfSize=1.5f*SIM_MAX(bbs(0),bbs(1))/2.0f;
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

        tr.X+=getCumulativeTransformationPart1().Q*bbsavg;
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

bool C3DObject::setLocalTransformationFromObjectRotationMode(const C4X4Matrix& cameraAbsConf,float rotationAmount,bool perspective,int eventID)
{ // bits 0-2: position x,y,z (relative to parent frame), bits 3-5: Euler e9,e1,e2 (relative to own frame)
    if ( (!App::ct->simulation->isSimulationStopped())&&getObjectRotationDisabledDuringSimulation())
    {
        _objectManipulationMode_flaggedForGridOverlay=0;
        _objectManipulationModeEventId=-1;
        return(false);
    }
    if (App::ct->simulation->isSimulationStopped()&&getObjectRotationDisabledDuringNonSimulation())
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
        C4X4Matrix objAbs(getCumulativeTransformationPart1().getMatrix());
        C3X3Matrix rotAxes;
        if (getObjectManipulationRotationRelativeTo()==2)
            rotAxes=objAbs.M; // own frame
        if (getObjectManipulationRotationRelativeTo()==1)
            rotAxes=getParentCumulativeTransformation().getMatrix().M; // parent frame
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
    if (getObjectManipulationRotationRelativeTo()==2)
        _transformation.Q*=rot; // relative to own frame
    if (getObjectManipulationRotationRelativeTo()==1)
        _transformation.Q=rot*_transformation.Q; // relative to parent frame
    if (getObjectManipulationRotationRelativeTo()==0)
    { // relative to world frame
        C4Vector trq(getCumulativeTransformationPart1().Q);
        trq=rot*trq;
        C4Vector pinv(getParentCumulativeTransformation().Q.getInverse());
        _transformation.Q=pinv*trq;
    }
    _objectManipulationMode_flaggedForGridOverlay=_objectManipulationModeAxisIndex+8;
    return(true);
}


bool C3DObject::setLocalTransformationFromObjectTranslationMode(const C4X4Matrix& cameraAbsConf,const C3Vector& clicked3DPoint,float prevPos[2],float pos[2],float screenHalfSizes[2],float halfSizes[2],bool perspective,int eventID)
{ // bits 0-2: position x,y,z (relative to parent frame), bits 3-5: Euler e9,e1,e2 (relative to own frame)

    if ( (!App::ct->simulation->isSimulationStopped())&&getObjectTranslationDisabledDuringSimulation())
    {
        _objectManipulationMode_flaggedForGridOverlay=0;
        _objectManipulationModeEventId=-1;
        return(false);
    }
    if (App::ct->simulation->isSimulationStopped()&&getObjectTranslationDisabledDuringNonSimulation())
    {
        _objectManipulationMode_flaggedForGridOverlay=0;
        _objectManipulationModeEventId=-1;
        return(false);
    }

    C4X4Matrix objAbs;
    objAbs.X=getCumulativeTransformationPart1().X;
    if (getObjectManipulationTranslationRelativeTo()==0)
        objAbs.M.setIdentity();
    if (getObjectManipulationTranslationRelativeTo()==1)
        objAbs.M=getParentCumulativeTransformation().getMatrix().M;
    if (getObjectManipulationTranslationRelativeTo()==2)
        objAbs.M=getCumulativeTransformationPart1().getMatrix().M;
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
    C4X4Matrix m(getCumulativeTransformationPart1());
    m.X+=v;
    _transformation=getParentCumulativeTransformation().getInverse().getMatrix()*m;
    _objectManipulationMode_flaggedForGridOverlay=_objectManipulationModeAxisIndex+16;
    return(true);
}
#endif
