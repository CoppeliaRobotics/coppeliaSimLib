#include "simInternal.h"
#include "proximitySensor.h"
#include "tt.h"
#include "proxSensorRoutine.h"
#include "vDateTime.h"
#include "ttUtil.h"
#include "easyLock.h"
#include "app.h"
#include "pluginContainer.h"
#include "proximitySensorRendering.h"

CProxSensor::CProxSensor(int theType)
{
    commonInit();
    setSensorType(theType);
}

CProxSensor::CProxSensor()
{ // needed by the serialization routine only!
    commonInit();
}

CProxSensor::~CProxSensor()
{
    delete convexVolume;
}
/*
void CProxSensor::setCheckOcclusions(bool c)
{
    _checkOcclusions=c;
}

bool CProxSensor::getCheckOcclusions() const
{
    return(_checkOcclusions);
}
*/
void CProxSensor::setRandomizedDetection(bool enable)
{
    if ( (sensorType==sim_proximitysensor_ray_subtype)&&(enable!=_randomizedDetection) )
    {
        _randomizedDetection=enable;
        float off=convexVolume->getOffset();
        float radius=convexVolume->getRadius();
        float range=convexVolume->getRange();
        bool forbiddenDistEnable=convexVolume->getSmallestDistanceEnabled();
        float forbiddenDist=convexVolume->getSmallestDistanceAllowed();

        if (enable)
        {
            convexVolume->setVolumeType(CONE_TYPE_CONVEX_VOLUME,_objectType,size);
            convexVolume->setOffset(0.0f);
            convexVolume->setRadius(off);
            convexVolume->setRange(range);
            convexVolume->setSmallestDistanceEnabled(forbiddenDistEnable);
            convexVolume->setSmallestDistanceAllowed(forbiddenDist);
            convexVolume->setAngle(60.0f*degToRad_f);
            convexVolume->setFaceNumber(32);
            convexVolume->setSubdivisions(3);
            convexVolume->setSubdivisionsFar(16);
            convexVolume->setInsideAngleThing(0.0f);
        }
        else
        {
            convexVolume->setVolumeType(RAY_TYPE_CONVEX_VOLUME,_objectType,size);
            convexVolume->setOffset(radius);
            convexVolume->setRange(range);
            convexVolume->setSmallestDistanceEnabled(forbiddenDistEnable);
            convexVolume->setSmallestDistanceAllowed(forbiddenDist);
        }
    }
}

bool CProxSensor::getRandomizedDetection() const
{
    return(_randomizedDetection);
}

void CProxSensor::setRandomizedDetectionSampleCount(int c)
{
    _randomizedDetectionSampleCount=tt::getLimitedInt(1,5000,c);
    _randomizedDetectionCountForDetection=tt::getLimitedInt(1,_randomizedDetectionSampleCount,_randomizedDetectionCountForDetection);
}

int CProxSensor::getRandomizedDetectionSampleCount() const
{
    return(_randomizedDetectionSampleCount);
}

void CProxSensor::setRandomizedDetectionCountForDetection(int c)
{
    _randomizedDetectionCountForDetection=tt::getLimitedInt(1,_randomizedDetectionSampleCount,c);
}

int CProxSensor::getRandomizedDetectionCountForDetection() const
{
    return(_randomizedDetectionCountForDetection);
}

void CProxSensor::setShowVolumeWhenNotDetecting(bool s)
{
    _showVolumeWhenNotDetecting=s;
}
bool CProxSensor::getShowVolumeWhenNotDetecting() const
{
    return(_showVolumeWhenNotDetecting);
}
void CProxSensor::setShowVolumeWhenDetecting(bool s)
{
    _showVolumeWhenDetecting=s;
}
bool CProxSensor::getShowVolumeWhenDetecting() const
{
    return(_showVolumeWhenDetecting);
}

std::string CProxSensor::getObjectTypeInfo() const
{
    return("Proximity sensor");
}

std::string CProxSensor::getObjectTypeInfoExtended() const
{
    if (sensorType==sim_proximitysensor_pyramid_subtype)
        return("Proximity sensor (pyramid)");
    if (sensorType==sim_proximitysensor_cylinder_subtype)
        return("Proximity sensor (cylinder)");
    if (sensorType==sim_proximitysensor_disc_subtype)
        return("Proximity sensor (disc)");
    if (sensorType==sim_proximitysensor_cone_subtype)
        return("Proximity sensor (cone)");
    if (sensorType==sim_proximitysensor_ray_subtype)
        return("Proximity sensor (ray)");
    return("ERROR");
}
bool CProxSensor::isPotentiallyCollidable() const
{
    return(false);
}
bool CProxSensor::isPotentiallyMeasurable() const
{
    return(false);
}
bool CProxSensor::isPotentiallyDetectable() const
{
    return(false);
}
bool CProxSensor::isPotentiallyRenderable() const
{
    return(false);
}

void CProxSensor::commonInit()
{
    convexVolume=new CConvexVolume();
    explicitHandling=false;
    displayNormals=false;
    _objectType=sim_object_proximitysensor_type;
    frontFaceDetection=true;
    backFaceDetection=true;
    closestObjectMode=true;
    normalCheck=false;
    allowedNormal=45.0f*degToRad_f;
    _hideDetectionRay=false;

    _randomizedDetection=false;
    _randomizedDetectionSampleCount=20;
    _randomizedDetectionCountForDetection=5;

    size=0.01f;
    _showVolumeWhenNotDetecting=true;
    _showVolumeWhenDetecting=true;
    _localObjectSpecialProperty=0;

    _sensableObject=-1;
    _sensableType=sim_objectspecialproperty_detectable_ultrasonic;
    _detectedPointValid=false;
    _calcTimeInMs=0;

    passiveVolumeColor.setColorsAllBlack();
    passiveVolumeColor.setColor(0.9f,0.0f,0.5f,sim_colorcomponent_ambient_diffuse);
    activeVolumeColor.setColorsAllBlack();
    activeVolumeColor.setColor(1.0f,0.15f,0.75f,sim_colorcomponent_ambient_diffuse);
    detectionRayColor.setColorsAllBlack();
    detectionRayColor.setColor(1.0f,1.0f,0.0f,sim_colorcomponent_emission);
    detectionRayColor.setFlash(true);
    closestDistanceVolumeColor.setColorsAllBlack();
    closestDistanceVolumeColor.setColor(0.1f,0.1f,0.9f,sim_colorcomponent_ambient_diffuse);

    _objectManipulationModePermissions=0x013;

    _visibilityLayer=PROXIMITY_SENSOR_LAYER;
    _objectAlias="Proximity_sensor";
    _objectName_old="Proximity_sensor";
    _objectAltName_old=tt::getObjectAltNameFromObjectName(_objectName_old.c_str());
}

void CProxSensor::setSensableType(int theType)
{
    _sensableType=theType;
}

int CProxSensor::getSensableType() const
{
    return(_sensableType);
}

void CProxSensor::setHideDetectionRay(bool hide)
{
    _hideDetectionRay=hide;
}

bool CProxSensor::getHideDetectionRay() const
{
    return(_hideDetectionRay);
}

void CProxSensor::setExplicitHandling(bool setExplicit)
{
    explicitHandling=setExplicit;
}

bool CProxSensor::getExplicitHandling() const
{
    return(explicitHandling);
}

int CProxSensor::getSensableObject()
{
    return(_sensableObject);
}

void CProxSensor::setSensableObject(int objectID)
{
    _sensableObject=objectID;
}

bool CProxSensor::getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const
{
    vertices.clear();
    indices.clear();
    return(false); // for now
}

void CProxSensor::removeSceneDependencies()
{
    CSceneObject::removeSceneDependencies();
    _sensableObject=-1;
}

CSceneObject* CProxSensor::copyYourself()
{   
    CProxSensor* newSensor=(CProxSensor*)CSceneObject::copyYourself();

    newSensor->_sensableObject=_sensableObject;
    newSensor->_detectedPoint=_detectedPoint;
    newSensor->_detectedPointValid=false;
    newSensor->allowedNormal=allowedNormal;
    newSensor->size=size;
    newSensor->normalCheck=normalCheck;
    newSensor->closestObjectMode=closestObjectMode;
    newSensor->frontFaceDetection=frontFaceDetection;
    newSensor->backFaceDetection=backFaceDetection;
    newSensor->explicitHandling=explicitHandling;
    newSensor->sensorType=sensorType;
    newSensor->_sensableType=_sensableType;
    newSensor->displayNormals=displayNormals;
    newSensor->_showVolumeWhenNotDetecting=_showVolumeWhenNotDetecting;
    newSensor->_showVolumeWhenDetecting=_showVolumeWhenDetecting;
//    newSensor->_checkOcclusions=_checkOcclusions;

    newSensor->_randomizedDetection=_randomizedDetection;
    newSensor->_randomizedDetectionSampleCount=_randomizedDetectionSampleCount;
    newSensor->_randomizedDetectionCountForDetection=_randomizedDetectionCountForDetection;

    delete newSensor->convexVolume;
    newSensor->convexVolume=convexVolume->copyYourself();

    // Colors:
    passiveVolumeColor.copyYourselfInto(&newSensor->passiveVolumeColor);
    activeVolumeColor.copyYourselfInto(&newSensor->activeVolumeColor);
    detectionRayColor.copyYourselfInto(&newSensor->detectionRayColor);
    closestDistanceVolumeColor.copyYourselfInto(&newSensor->closestDistanceVolumeColor);

    newSensor->_initialValuesInitialized=_initialValuesInitialized;
    newSensor->_initialExplicitHandling=_initialExplicitHandling;

    return(newSensor);
}

bool CProxSensor::announceObjectWillBeErased(int objectHandle,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    // This routine can be called for objCont-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'ct::objCont->getObject(objectHandle)'-call or similar
    // Return value true means 'this' has to be erased too!
    bool retVal=CSceneObject::announceObjectWillBeErased(objectHandle,copyBuffer);
    if (_sensableObject==objectHandle)
        _sensableObject=-1;
    return(retVal);
}

void CProxSensor::announceCollectionWillBeErased(int groupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollectionWillBeErased(groupID,copyBuffer);
    if (_sensableObject==groupID)
        _sensableObject=-1;
}
void CProxSensor::announceCollisionWillBeErased(int collisionID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollisionWillBeErased(collisionID,copyBuffer);
}
void CProxSensor::announceDistanceWillBeErased(int distanceID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceDistanceWillBeErased(distanceID,copyBuffer);
}
void CProxSensor::announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceIkObjectWillBeErased(ikGroupID,copyBuffer);
}

void CProxSensor::performObjectLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{
    CSceneObject::performObjectLoadingMapping(map,loadingAmodel);
    if (_sensableObject<SIM_IDSTART_COLLECTION)
        _sensableObject=CWorld::getLoadingMapping(map,_sensableObject);
}
void CProxSensor::performCollectionLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    CSceneObject::performCollectionLoadingMapping(map,loadingAmodel);
    if (_sensableObject>=SIM_IDSTART_COLLECTION)
        _sensableObject=CWorld::getLoadingMapping(map,_sensableObject);
}
void CProxSensor::performCollisionLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    CSceneObject::performCollisionLoadingMapping(map,loadingAmodel);
}
void CProxSensor::performDistanceLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    CSceneObject::performDistanceLoadingMapping(map,loadingAmodel);
}
void CProxSensor::performIkLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{
    CSceneObject::performIkLoadingMapping(map,loadingAmodel);
}

void CProxSensor::performTextureObjectLoadingMapping(const std::vector<int>* map)
{
    CSceneObject::performTextureObjectLoadingMapping(map);
}

void CProxSensor::performDynMaterialObjectLoadingMapping(const std::vector<int>* map)
{
    CSceneObject::performDynMaterialObjectLoadingMapping(map);
}

void CProxSensor::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
    _initialExplicitHandling=explicitHandling;
}

void CProxSensor::simulationAboutToStart()
{
    initializeInitialValues(false);
    CSceneObject::simulationAboutToStart();
}

void CProxSensor::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized)
    {
        if (App::currentWorld->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
        {
            explicitHandling=_initialExplicitHandling;
        }
    }
    CSceneObject::simulationEnded();
}

void CProxSensor::serialize(CSer& ar)
{
    CSceneObject::serialize(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Snt");
            ar << sensorType;
            ar.flush();

            ar.storeDataName("Vod");
            ar.setCountingMode();
            convexVolume->serialize(ar);
            if (ar.setWritingMode())
                convexVolume->serialize(ar);

            ar.storeDataName("Sns");
            ar << size;
            ar.flush();

            ar.storeDataName("Al2");
            ar << allowedNormal;
            ar.flush();

            ar.storeDataName("Pr4");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,_showVolumeWhenNotDetecting);
            SIM_SET_CLEAR_BIT(nothing,1,closestObjectMode);
            SIM_SET_CLEAR_BIT(nothing,2,normalCheck);
    // 12/12/2011       SIM_SET_CLEAR_BIT(nothing,3,_detectAllDetectable);
            SIM_SET_CLEAR_BIT(nothing,4,!frontFaceDetection);
            SIM_SET_CLEAR_BIT(nothing,5,!backFaceDetection);
            SIM_SET_CLEAR_BIT(nothing,6,_showVolumeWhenDetecting);
            SIM_SET_CLEAR_BIT(nothing,7,explicitHandling);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Pr5");
            nothing=0;
//            SIM_SET_CLEAR_BIT(nothing,0,_checkOcclusions);
            SIM_SET_CLEAR_BIT(nothing,1,_randomizedDetection);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Rad");
            ar << _randomizedDetectionSampleCount << _randomizedDetectionCountForDetection;
            ar.flush();

            ar.storeDataName("Cl1");
            ar.setCountingMode();
            passiveVolumeColor.serialize(ar,0);
            if (ar.setWritingMode())
                passiveVolumeColor.serialize(ar,0);

            ar.storeDataName("Cl2");
            ar.setCountingMode();
            activeVolumeColor.serialize(ar,0);
            if (ar.setWritingMode())
                activeVolumeColor.serialize(ar,0);

            ar.storeDataName("Cl3");
            ar.setCountingMode();
            detectionRayColor.serialize(ar,1);
            if (ar.setWritingMode())
                detectionRayColor.serialize(ar,1);

            ar.storeDataName("Cl4");
            ar.setCountingMode();
            closestDistanceVolumeColor.serialize(ar,1);
            if (ar.setWritingMode())
                closestDistanceVolumeColor.serialize(ar,1);

            ar.storeDataName("Sox");
            ar << _sensableObject;
            ar.flush();

            ar.storeDataName("Sst");
            ar << _sensableType;
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
//            bool occlusionCheckThingWasLoaded_backwardCompatibility2010_08_09=false;
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Snt")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> sensorType;
                    }
                    if (theName.compare("Vod")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        if (convexVolume!=nullptr)
                            delete convexVolume;
                        convexVolume=new CConvexVolume();
                        convexVolume->serialize(ar);
                    }
                    if (theName.compare("Sns")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> size;
                    }
                    if (theName.compare("Sox")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _sensableObject;
                    }
                    if (theName.compare("Sst")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _sensableType;
                    }
                    if (theName.compare("Al2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> allowedNormal;
                    }
                    if (theName=="Pr4")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _showVolumeWhenNotDetecting=SIM_IS_BIT_SET(nothing,0);
                        closestObjectMode=SIM_IS_BIT_SET(nothing,1);
                        normalCheck=SIM_IS_BIT_SET(nothing,2);
                        frontFaceDetection=!SIM_IS_BIT_SET(nothing,4);
                        backFaceDetection=!SIM_IS_BIT_SET(nothing,5);
                        _showVolumeWhenDetecting=SIM_IS_BIT_SET(nothing,6);
                        explicitHandling=SIM_IS_BIT_SET(nothing,7);
                    }
                    if (theName=="Pr5")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
//                        _checkOcclusions=SIM_IS_BIT_SET(nothing,0);
                        _randomizedDetection=SIM_IS_BIT_SET(nothing,1);
//                        occlusionCheckThingWasLoaded_backwardCompatibility2010_08_09=true;
                    }
                    if (theName.compare("Rad")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _randomizedDetectionSampleCount >> _randomizedDetectionCountForDetection;
                    }
                    if (theName.compare("Cl1")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        passiveVolumeColor.serialize(ar,0);
                    }
                    if (theName.compare("Cl2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        activeVolumeColor.serialize(ar,0);
                    }
                    if (theName.compare("Cl3")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        detectionRayColor.serialize(ar,1);
                    }
                    if (theName.compare("Cl4")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        closestDistanceVolumeColor.serialize(ar,1);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
//            if (!occlusionCheckThingWasLoaded_backwardCompatibility2010_08_09)
//                _checkOcclusions=false;

            if (ar.getSerializationVersionThatWroteThisFile()<17)
            { // on 29/08/2013 we corrected all default lights. So we need to correct for that change:
                CTTUtil::scaleColorUp_(passiveVolumeColor.getColorsPtr());
                CTTUtil::scaleColorUp_(activeVolumeColor.getColorsPtr());
                CTTUtil::scaleColorUp_(detectionRayColor.getColorsPtr());
                CTTUtil::scaleColorUp_(closestDistanceVolumeColor.getColorsPtr());
            }
        }
    }
    else
    {
        bool exhaustiveXml=( (ar.getFileType()!=CSer::filetype_csim_xml_simplescene_file)&&(ar.getFileType()!=CSer::filetype_csim_xml_simplemodel_file) );
        if (ar.isStoring())
        {
            ar.xmlAddNode_comment(" 'type' tag: can be 'pyramid', 'cylinder', 'disc', 'cone' or 'ray' ",exhaustiveXml);
            ar.xmlAddNode_enum("type",sensorType,sim_proximitysensor_pyramid_subtype,"pyramid",sim_proximitysensor_cylinder_subtype,"cylinder",sim_proximitysensor_disc_subtype,"disc",sim_proximitysensor_cone_subtype,"cone",sim_proximitysensor_ray_subtype,"ray");

            ar.xmlAddNode_float("size",size);

            if (exhaustiveXml)
                ar.xmlAddNode_int("detectableEntity",_sensableObject);
            else
            {
                std::string str;
                CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_sensableObject);
                if (it!=nullptr)
                    str=it->getObjectName_old();
                else
                {
                    CCollection* coll=App::currentWorld->collections->getObjectFromHandle(_sensableObject);
                    if (coll!=nullptr)
                        str="@collection@"+coll->getCollectionName();
                }
                ar.xmlAddNode_comment(" 'detectableEntity' tag only used for backward compatibility, use instead 'detectableObjectAlias' tag",exhaustiveXml);
                ar.xmlAddNode_string("detectableEntity",str.c_str());
                if (it!=nullptr)
                {
                    str=it->getObjectAlias()+"*";
                    str+=std::to_string(it->getObjectHandle());
                }
                ar.xmlAddNode_string("detectableObjectAlias",str.c_str());
            }

            ar.xmlAddNode_comment(" 'detectionType' tag: can be 'ultrasonic', 'infrared', 'laser', 'inductive' or 'capacitive' ",exhaustiveXml);
            ar.xmlAddNode_enum("detectionType",_sensableType,sim_objectspecialproperty_detectable_ultrasonic,"ultrasonic",sim_objectspecialproperty_detectable_infrared,"infrared",sim_objectspecialproperty_detectable_laser,"laser",sim_objectspecialproperty_detectable_inductive,"inductive",sim_objectspecialproperty_detectable_capacitive,"capacitive");

            ar.xmlAddNode_float("allowedNormalAngle",allowedNormal*180.0f/piValue_f);

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("showVolumeWhenDetecting",_showVolumeWhenDetecting);
            ar.xmlAddNode_bool("showVolumeWhenNotDetecting",_showVolumeWhenNotDetecting);
            ar.xmlAddNode_bool("closestObjectMode",closestObjectMode);
            ar.xmlAddNode_bool("normalCheck",normalCheck);
            ar.xmlAddNode_bool("frontFaceDetection",frontFaceDetection);
            ar.xmlAddNode_bool("backFaceDetection",backFaceDetection);
            ar.xmlAddNode_bool("explicitHandling",explicitHandling);
            ar.xmlPopNode();

            ar.xmlPushNewNode("randomizedDetection");
            ar.xmlAddNode_bool("enabled",_randomizedDetection);
            ar.xmlAddNode_int("sampleCount",_randomizedDetectionSampleCount);
            ar.xmlAddNode_int("countForTrigger",_randomizedDetectionCountForDetection);
            ar.xmlPopNode();

            ar.xmlPushNewNode("color");
            if (exhaustiveXml)
            {
                ar.xmlPushNewNode("passiveVolume");
                passiveVolumeColor.serialize(ar,0);
                ar.xmlPopNode();
                ar.xmlPushNewNode("activeVolume");
                activeVolumeColor.serialize(ar,0);
                ar.xmlPopNode();
                ar.xmlPushNewNode("detectionRay");
                detectionRayColor.serialize(ar,0);
                ar.xmlPopNode();
                ar.xmlPushNewNode("closestDistVolume");
                closestDistanceVolumeColor.serialize(ar,0);
                ar.xmlPopNode();
            }
            else
            {
                int rgb[3];
                for (size_t l=0;l<3;l++)
                    rgb[l]=int(passiveVolumeColor.getColorsPtr()[l]*127.0f+passiveVolumeColor.getColorsPtr()[9+l]*127.0f);
                ar.xmlAddNode_ints("passiveVolume",rgb,3);
                for (size_t l=0;l<3;l++)
                    rgb[l]=int(activeVolumeColor.getColorsPtr()[l]*127.1f+activeVolumeColor.getColorsPtr()[9+l]*127.1f);
                ar.xmlAddNode_ints("activeVolume",rgb,3);
                for (size_t l=0;l<3;l++)
                    rgb[l]=int(detectionRayColor.getColorsPtr()[l]*127.1f+detectionRayColor.getColorsPtr()[9+l]*127.1f);
                ar.xmlAddNode_ints("detectionRay",rgb,3);
                for (size_t l=0;l<3;l++)
                    rgb[l]=int(closestDistanceVolumeColor.getColorsPtr()[l]*127.1f+closestDistanceVolumeColor.getColorsPtr()[9+l]*127.1f);
                ar.xmlAddNode_ints("closestDistVolume",rgb,3);
            }
            ar.xmlPopNode();

            ar.xmlPushNewNode("volume");
            convexVolume->serialize(ar);
            ar.xmlPopNode();
        }
        else
        {
            ar.xmlGetNode_enum("type",sensorType,exhaustiveXml,"pyramid",sim_proximitysensor_pyramid_subtype,"cylinder",sim_proximitysensor_cylinder_subtype,"disc",sim_proximitysensor_disc_subtype,"cone",sim_proximitysensor_cone_subtype,"ray",sim_proximitysensor_ray_subtype);

            ar.xmlGetNode_float("size",size,exhaustiveXml);

            if (exhaustiveXml)
                ar.xmlGetNode_int("detectableEntity",_sensableObject);
            else
            {
                ar.xmlGetNode_string("detectableObjectAlias",_sensableObjectLoadAlias,exhaustiveXml);
                ar.xmlGetNode_string("detectableEntity",_sensableObjectLoadName_old,exhaustiveXml);
            }

            ar.xmlGetNode_enum("detectionType",_sensableType,exhaustiveXml,"ultrasonic",sim_objectspecialproperty_detectable_ultrasonic,"infrared",sim_objectspecialproperty_detectable_infrared,"laser",sim_objectspecialproperty_detectable_laser,"inductive",sim_objectspecialproperty_detectable_inductive,"capacitive",sim_objectspecialproperty_detectable_capacitive);

            if (ar.xmlGetNode_float("allowedNormalAngle",allowedNormal,exhaustiveXml))
                allowedNormal*=piValue_f/180.0f;

            if (ar.xmlPushChildNode("switches",exhaustiveXml))
            {
                ar.xmlGetNode_bool("showVolumeWhenDetecting",_showVolumeWhenDetecting,exhaustiveXml);
                ar.xmlGetNode_bool("showVolumeWhenNotDetecting",_showVolumeWhenNotDetecting,exhaustiveXml);
                ar.xmlGetNode_bool("closestObjectMode",closestObjectMode,exhaustiveXml);
                ar.xmlGetNode_bool("normalCheck",normalCheck,exhaustiveXml);
                ar.xmlGetNode_bool("frontFaceDetection",frontFaceDetection,exhaustiveXml);
                ar.xmlGetNode_bool("backFaceDetection",backFaceDetection,exhaustiveXml);
                ar.xmlGetNode_bool("explicitHandling",explicitHandling,exhaustiveXml);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("randomizedDetection",exhaustiveXml))
            {
                ar.xmlGetNode_bool("enabled",_randomizedDetection,exhaustiveXml);
                ar.xmlGetNode_int("sampleCount",_randomizedDetectionSampleCount,exhaustiveXml);
                ar.xmlGetNode_int("countForTrigger",_randomizedDetectionCountForDetection,exhaustiveXml);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("color",exhaustiveXml))
            {
                if (exhaustiveXml)
                {
                    if (ar.xmlPushChildNode("passiveVolume"))
                    {
                        passiveVolumeColor.serialize(ar,0);
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("activeVolume"))
                    {
                        activeVolumeColor.serialize(ar,0);
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("detectionRay"))
                    {
                        detectionRayColor.serialize(ar,0);
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("closestDistVolume"))
                    {
                        closestDistanceVolumeColor.serialize(ar,0);
                        ar.xmlPopNode();
                    }
                }
                else
                {
                    int rgb[3];
                    if (ar.xmlGetNode_ints("passiveVolume",rgb,3,exhaustiveXml))
                        passiveVolumeColor.setColor(float(rgb[0])/255.1f,float(rgb[1])/255.1f,float(rgb[2])/255.1f,sim_colorcomponent_ambient_diffuse);
                    if (ar.xmlGetNode_ints("activeVolume",rgb,3,exhaustiveXml))
                        activeVolumeColor.setColor(float(rgb[0])/255.1f,float(rgb[1])/255.1f,float(rgb[2])/255.1f,sim_colorcomponent_ambient_diffuse);
                    if (ar.xmlGetNode_ints("detectionRay",rgb,3,exhaustiveXml))
                        detectionRayColor.setColor(float(rgb[0])/255.1f,float(rgb[1])/255.1f,float(rgb[2])/255.1f,sim_colorcomponent_ambient_diffuse);
                    if (ar.xmlGetNode_ints("closestDistVolume",rgb,3,exhaustiveXml))
                        closestDistanceVolumeColor.setColor(float(rgb[0])/255.1f,float(rgb[1])/255.1f,float(rgb[2])/255.1f,sim_colorcomponent_ambient_diffuse);
                }
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("volume",exhaustiveXml))
            {
                if (convexVolume!=nullptr)
                    delete convexVolume;
                convexVolume=new CConvexVolume();

                if (!exhaustiveXml)
                {
                    if (sensorType==sim_proximitysensor_ray_subtype)
                        convexVolume->setVolumeType(RAY_TYPE_CONVEX_VOLUME,_objectType,size);
                    else
                        _randomizedDetection=false;
                    if (sensorType==sim_proximitysensor_cylinder_subtype)
                        convexVolume->setVolumeType(CYLINDER_TYPE_CONVEX_VOLUME,_objectType,size);
                    if (sensorType==sim_proximitysensor_disc_subtype)
                        convexVolume->setVolumeType(DISC_TYPE_CONVEX_VOLUME,_objectType,size);
                    if (sensorType==sim_proximitysensor_pyramid_subtype)
                        convexVolume->setVolumeType(PYRAMID_TYPE_CONVEX_VOLUME,_objectType,size);
                    if (sensorType==sim_proximitysensor_cone_subtype)
                        convexVolume->setVolumeType(CONE_TYPE_CONVEX_VOLUME,_objectType,size);
                }

                convexVolume->serialize(ar);
                ar.xmlPopNode();
            }
        }
    }
}

void CProxSensor::serializeWExtIk(CExtIkSer& ar)
{
    CSceneObject::serializeWExtIk(ar);
    CDummy::serializeWExtIkStatic(ar);
}

bool CProxSensor::getSensingVolumeBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    return(convexVolume->getVolumeBoundingBox(minV,maxV));
}

void CProxSensor::getSensingVolumeOBB(C7Vector& tr,C3Vector& halfSizes)
{
    C3Vector minV,maxV,center;
    convexVolume->getVolumeBoundingBox(minV,maxV);
    center=(minV+maxV)*0.5f;
    C4X4Matrix m(getFullCumulativeTransformation().getMatrix());
    center=m*center;
    m.X=center;
    halfSizes=(maxV-minV)*0.5f;
    tr=m.getTransformation();
}

bool CProxSensor::getFullBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    getSensingVolumeBoundingBox(minV,maxV);
    C3Vector m(size*0.5f,size*0.5f,size*0.5f); // sensing sphere
    C3Vector n(-size*0.5f,-size*0.5f,-size*0.5f);
    minV.keepMin(n);
    maxV.keepMax(m);
    return(true);
}

bool CProxSensor::getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    return (getFullBoundingBox(minV,maxV));
}

void CProxSensor::calculateFreshRandomizedRays()
{
    _randomizedVectors.clear();
    _randomizedVectorDetectionStates.clear();
    // Build the random rays (only direction)
    float angle=convexVolume->getAngle();
    for (int i=0;i<_randomizedDetectionSampleCount;i++)
    {
        float rZ,sZ,cZ;
        if (angle>1.1f*piValD2_f)
        { // this means we have 360x180 degrees. We compute it as 2 half-spheres, in order to have a perfect direction distribution:
            rZ=acos(1.0f-SIM_RAND_FLOAT);
            sZ=sin(rZ);
            cZ=cos(rZ);
            if (SIM_RAND_FLOAT>0.5f)
                cZ=-cZ;
        }
        else
        { // this means we have 360xA degrees, where A<=90.
            rZ=angle*acos(1.0f-SIM_RAND_FLOAT)/piValue_f;
            sZ=sin(rZ);
            cZ=cos(rZ);
        }
        float rXY=SIM_RAND_FLOAT*piValTimes2_f;
        float sXY=sin(rXY);
        float cXY=cos(rXY);
        C3Vector v(sZ*cXY,sZ*sXY,cZ);
        _randomizedVectors.push_back(v);
        _randomizedVectorDetectionStates.push_back(0.0f);
    }
}

const std::vector<C3Vector>* CProxSensor::getPointerToRandomizedRays() const
{
    return(&_randomizedVectors);
}

std::vector<float>* CProxSensor::getPointerToRandomizedRayDetectionStates()
{
    return(&_randomizedVectorDetectionStates);
}

void CProxSensor::resetSensor(bool exceptExplicitHandling)
{
    if ( (!exceptExplicitHandling)||(!getExplicitHandling()) )
    {
        _detectedPointValid=false;
        _sensorResultValid=false;
        _calcTimeInMs=0;
    }
}

bool CProxSensor::handleSensor(bool exceptExplicitHandling,int& detectedObjectHandle,C3Vector& detectedNormalVector)
{
    if (exceptExplicitHandling&&getExplicitHandling())
        return(false); // We don't want to handle those
    _sensorResultValid=false;
    _detectedPointValid=false;
    _calcTimeInMs=0;
    if (!App::currentWorld->mainSettings->proximitySensorsEnabled)
        return(false);
    if (!CPluginContainer::isGeomPluginAvailable())
        return(false);

    _sensorResultValid=true;

    int stTime=VDateTime::getTimeInMs();

    float treshhold=SIM_MAX_FLOAT;
    float minThreshold=-1.0f;
    if (convexVolume->getSmallestDistanceEnabled())
        minThreshold=convexVolume->getSmallestDistanceAllowed();

    _randomizedVectors.clear();
    _randomizedVectorDetectionStates.clear();
    _detectedPointValid=CProxSensorRoutine::detectEntity(_objectHandle,_sensableObject,closestObjectMode,normalCheck,allowedNormal,_detectedPoint,treshhold,frontFaceDetection,backFaceDetection,detectedObjectHandle,minThreshold,detectedNormalVector,false);
    _detectedObjectHandle=detectedObjectHandle;
    _detectedNormalVector=detectedNormalVector;
    _calcTimeInMs=VDateTime::getTimeDiffInMs(stTime);
    if (_sensorResultValid&&_detectedPointValid)
    {
        CScriptObject* script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo_child(_objectHandle);
        if ( (script!=nullptr)&&(!script->getContainsTriggerCallbackFunction()) )
            script=nullptr;
        CScriptObject* cScript=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo_customization(_objectHandle);
        if ( (cScript!=nullptr)&&(!cScript->getContainsTriggerCallbackFunction()) )
            cScript=nullptr;
        if ( (script!=nullptr)||(cScript!=nullptr) )
        {
            CInterfaceStack* inStack=App::worldContainer->interfaceStackContainer->createStack();
            inStack->pushTableOntoStack();

            inStack->insertKeyInt32IntoStackTable("handle",getObjectHandle());
            inStack->insertKeyInt32IntoStackTable("detectedObjectHandle",_detectedObjectHandle);
            inStack->insertKeyFloatArrayIntoStackTable("detectedPoint",_detectedPoint.data,3);
            inStack->insertKeyFloatArrayIntoStackTable("normalVector",_detectedNormalVector.data,3);

            CInterfaceStack* outStack1=App::worldContainer->interfaceStackContainer->createStack();
            CInterfaceStack* outStack2=App::worldContainer->interfaceStackContainer->createStack();
            CInterfaceStack* outSt1=outStack1;
            CInterfaceStack* outSt2=outStack2;
            if (VThread::isCurrentThreadTheMainSimulationThread())
            { // we are in the main simulation thread. Call only scripts that live in the same thread
                if ( (script!=nullptr)&&(!script->getThreadedExecution_oldThreads()) )
                    script->systemCallScript(sim_syscb_trigger,inStack,outStack1);
                if (cScript!=nullptr)
                    cScript->systemCallScript(sim_syscb_trigger,inStack,outStack2);
            }
            else
            { // we are in the thread started by a threaded child script. Call only that script
                if ( (script!=nullptr)&&script->getThreadedExecution_oldThreads() )
                {
                    script->systemCallScript(sim_syscb_trigger,inStack,nullptr);
                    outSt1=inStack;
                }
            }
            CInterfaceStack* outStacks[2]={outSt1,outSt2};
            for (size_t cnt=0;cnt<2;cnt++)
            {
                CInterfaceStack* outStack=outStacks[cnt];
                if (outStack->getStackSize()>=1)
                {
                    outStack->moveStackItemToTop(0);
                    bool trig=false;
                    if (outStack->getStackMapBoolValue("trigger",trig))
                        _detectedPointValid=trig;
                }
            }
            App::worldContainer->interfaceStackContainer->destroyStack(outStack2);
            App::worldContainer->interfaceStackContainer->destroyStack(outStack1);
            App::worldContainer->interfaceStackContainer->destroyStack(inStack);
        }
    }
    return(_detectedPointValid);
}

int CProxSensor::readSensor(C3Vector& detectPt,int& detectedObjectHandle,C3Vector& detectedNormalVector)
{
    if (_sensorResultValid)
    {
        if (_detectedPointValid)
        {
            detectPt=_detectedPoint;
            detectedObjectHandle=_detectedObjectHandle;
            detectedNormalVector=_detectedNormalVector;
            return(1);
        }
        return(0);
    }
    return(-1);
}

float CProxSensor::getCalculationTime() const
{
    return(float(_calcTimeInMs)*0.001f);
}

bool CProxSensor::getFrontFaceDetection() const
{
    return(frontFaceDetection);
}
bool CProxSensor::getBackFaceDetection() const
{
    return(backFaceDetection);
}
void CProxSensor::setFrontFaceDetection(bool faceOn)
{
    frontFaceDetection=faceOn;
    if (!faceOn)
        backFaceDetection=true;
}
void CProxSensor::setBackFaceDetection(bool faceOn)
{
    backFaceDetection=faceOn;
    if (!faceOn)
        frontFaceDetection=true;
}

void CProxSensor::setAllowedNormal(float al)
{
    tt::limitValue(0.1f*degToRad_f,90.0f*degToRad_f,al);
    allowedNormal=al;
}
float CProxSensor::getAllowedNormal() const
{
    return(allowedNormal);
}
void CProxSensor::setNormalCheck(bool check)
{
    normalCheck=check;
}
bool CProxSensor::getNormalCheck() const
{
    return(normalCheck);
}
void CProxSensor::setClosestObjectMode(bool closestObjMode)
{
    closestObjectMode=closestObjMode;
    _detectedPointValid=false;
    _calcTimeInMs=0;
}
bool CProxSensor::getClosestObjectMode()
{
    return(closestObjectMode);
}

void CProxSensor::setSize(float newSize)
{
    tt::limitValue(0.0001f,10.0f,newSize);
    size=newSize;
}
float CProxSensor::getSize()
{
    return(size);
}

bool CProxSensor::getSensedData(C3Vector& pt)
{
    if (!_detectedPointValid)
        return(false);
    pt=_detectedPoint;
    return(true);
}

void CProxSensor::scaleObject(float scalingFactor)
{
    size*=scalingFactor;
    convexVolume->scaleVolume(scalingFactor);
    CSceneObject::scaleObject(scalingFactor);
}

void CProxSensor::scaleObjectNonIsometrically(float x,float y,float z)
{
    float xp,yp,zp;
    convexVolume->scaleVolumeNonIsometrically(x,y,z,xp,yp,zp);
    size*=cbrt(xp*yp*zp);
    CSceneObject::scaleObjectNonIsometrically(xp,yp,zp);
}

C3Vector CProxSensor::getDetectedPoint() const
{
    return(_detectedPoint);
}

bool CProxSensor::getIsDetectedPointValid() const
{
    return(_detectedPointValid);
}

std::string CProxSensor::getSensableObjectLoadAlias() const
{
    return(_sensableObjectLoadAlias);
}

std::string CProxSensor::getSensableObjectLoadName_old() const
{
    return(_sensableObjectLoadName_old);
}

CColorObject* CProxSensor::getColor(int index)
{
    if (index==0)
        return(&passiveVolumeColor);
    if (index==1)
        return(&activeVolumeColor);
    if (index==2)
        return(&detectionRayColor);
    if (index==3)
        return(&closestDistanceVolumeColor);
    return(nullptr);
}

void CProxSensor::setSensorType(int theType)
{
    sensorType=theType;
    if (theType==sim_proximitysensor_ray_subtype)
        convexVolume->setVolumeType(RAY_TYPE_CONVEX_VOLUME,_objectType,size);
    else
        _randomizedDetection=false;

    if (theType==sim_proximitysensor_cylinder_subtype)
        convexVolume->setVolumeType(CYLINDER_TYPE_CONVEX_VOLUME,_objectType,size);
    if (theType==sim_proximitysensor_disc_subtype)
        convexVolume->setVolumeType(DISC_TYPE_CONVEX_VOLUME,_objectType,size);
    if (theType==sim_proximitysensor_pyramid_subtype)
        convexVolume->setVolumeType(PYRAMID_TYPE_CONVEX_VOLUME,_objectType,size);
    if (theType==sim_proximitysensor_cone_subtype)
        convexVolume->setVolumeType(CONE_TYPE_CONVEX_VOLUME,_objectType,size);
}

int CProxSensor::getSensorType() const
{
    return(sensorType);
}

void CProxSensor::display(CViewableBase* renderingObject,int displayAttrib)
{
    EASYLOCK(_objectMutex);
    displayProximitySensor(this,renderingObject,displayAttrib);
}
