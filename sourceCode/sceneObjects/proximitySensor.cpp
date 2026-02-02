#include <simInternal.h>
#include <proximitySensor.h>
#include <tt.h>
#include <proxSensorRoutine.h>
#include <vDateTime.h>
#include <utils.h>
#include <app.h>
#ifdef SIM_WITH_GUI
#include <proximitySensorRendering.h>
#endif

static std::string OBJECT_META_INFO = R"(
{
    "superclass": "sceneObject",
    "namespaces": {
        "refs": {"newPropertyForcedType": "sim.propertytype_handlearray"},
        "origRefs": {"newPropertyForcedType": "sim.propertytype_handlearray"},
        "customData": {},
        "signal": {}
    },
    "methods": {
        )" PROXIMITYSENSOR_META_METHODS R"(,
        )" SCENEOBJECT_META_METHODS R"(
    }
}
)";

CProxSensor::CProxSensor(int theType)
{
    commonInit();

    sensorType = theType;
    if (theType == sim_proximitysensor_ray)
        convexVolume->setVolumeType(RAY_TYPE_CONVEX_VOLUME, _objectType, _proxSensorSize);
    else
        _randomizedDetection = false;

    if (theType == sim_proximitysensor_cylinder)
        convexVolume->setVolumeType(CYLINDER_TYPE_CONVEX_VOLUME, _objectType, _proxSensorSize);
    if (theType == sim_proximitysensor_disc)
        convexVolume->setVolumeType(DISC_TYPE_CONVEX_VOLUME, _objectType, _proxSensorSize);
    if (theType == sim_proximitysensor_pyramid)
        convexVolume->setVolumeType(PYRAMID_TYPE_CONVEX_VOLUME, _objectType, _proxSensorSize);
    if (theType == sim_proximitysensor_cone)
        convexVolume->setVolumeType(CONE_TYPE_CONVEX_VOLUME, _objectType, _proxSensorSize);

    computeBoundingBox();
}

CProxSensor::CProxSensor()
{ // needed by the serialization routine only!
    commonInit();
}

CProxSensor::~CProxSensor()
{
    delete convexVolume;
}

void CProxSensor::setRandomizedDetection(bool enable)
{
    if ((sensorType == sim_proximitysensor_ray) && (enable != _randomizedDetection))
    {
        _randomizedDetection = enable;
        double off = convexVolume->getOffset();
        double radius = convexVolume->getRadius();
        double range = convexVolume->getRange();
        double forbiddenDist = convexVolume->getSmallestDistanceAllowed();

        if (enable)
        {
            convexVolume->setVolumeType(CONE_TYPE_CONVEX_VOLUME, _objectType, _proxSensorSize);
            convexVolume->setOffset(0.0);
            convexVolume->setRadius(off);
            convexVolume->setRange(range);
            convexVolume->setSmallestDistanceAllowed(forbiddenDist);
            convexVolume->setAngle(60.0 * degToRad);
            convexVolume->setFaceNumber(32);
            convexVolume->setSubdivisions(3);
            convexVolume->setSubdivisionsFar(16);
            convexVolume->setInsideAngleThing(0.0);
        }
        else
        {
            convexVolume->setVolumeType(RAY_TYPE_CONVEX_VOLUME, _objectType, _proxSensorSize);
            convexVolume->setOffset(radius);
            convexVolume->setRange(range);
            convexVolume->setSmallestDistanceAllowed(forbiddenDist);
        }
    }
}

bool CProxSensor::getRandomizedDetection() const
{
    return (_randomizedDetection);
}

void CProxSensor::setRandomizedDetectionSampleCount(int c)
{
    _randomizedDetectionSampleCount_deprecated = tt::getLimitedInt(1, 5000, c);
    _randomizedDetectionCountForDetection_deprecated =
        tt::getLimitedInt(1, _randomizedDetectionSampleCount_deprecated, _randomizedDetectionCountForDetection_deprecated);
}

int CProxSensor::getRandomizedDetectionSampleCount() const
{
    return (_randomizedDetectionSampleCount_deprecated);
}

void CProxSensor::setRandomizedDetectionCountForDetection(int c)
{
    _randomizedDetectionCountForDetection_deprecated = tt::getLimitedInt(1, _randomizedDetectionSampleCount_deprecated, c);
}

int CProxSensor::getRandomizedDetectionCountForDetection() const
{
    return (_randomizedDetectionCountForDetection_deprecated);
}

void CProxSensor::setShowVolume(bool s)
{
    if (_showVolume != s)
    {
        _showVolume = s;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propProximitySensor_showVolume.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _showVolume);
            App::worldContainer->pushEvent();
        }
    }
}

bool CProxSensor::getShowVolume() const
{
    return _showVolume;
}

std::string CProxSensor::getObjectTypeInfo() const
{
    return ("proximitySensor");
}

std::string CProxSensor::getObjectTypeInfoExtended() const
{
    if (sensorType == sim_proximitysensor_pyramid)
        return ("proximity sensor (pyramid)");
    if (sensorType == sim_proximitysensor_cylinder)
        return ("proximity sensor (cylinder)");
    if (sensorType == sim_proximitysensor_disc)
        return ("proximity sensor (disc)");
    if (sensorType == sim_proximitysensor_cone)
        return ("proximity sensor (cone)");
    return ("proximity sensor (ray)");
}
bool CProxSensor::isPotentiallyCollidable() const
{
    return (false);
}
bool CProxSensor::isPotentiallyMeasurable() const
{
    return (false);
}
bool CProxSensor::isPotentiallyDetectable() const
{
    return (false);
}
bool CProxSensor::isPotentiallyRenderable() const
{
    return (false);
}

void CProxSensor::commonInit()
{
    convexVolume = new CConvexVolume();
    _explicitHandling = false;
    _objectType = sim_sceneobject_proximitysensor;
    _frontFaceDetection = true;
    _backFaceDetection = true;
    _exactMode = true;
    _angleThreshold = 0.0; // means angle check disabled
    _hideDetectionRay_deprecated = false;

    _randomizedDetection = false;
    _randomizedDetectionSampleCount_deprecated = 1;
    _randomizedDetectionCountForDetection_deprecated = 1;

    _proxSensorSize = 0.01;
    _showVolume = true;
    _localObjectSpecialProperty = 0;

    _sensableObject_deprecated = -1;
    _sensableType_deprecated = sim_objectspecialproperty_detectable_ultrasonic;
    _detectedObjectHandle = -1;
    _calcTimeInMs = 0;

    volumeColor.setColorsAllBlack();
    volumeColor.setColor(0.9f, 0.0f, 0.5f, sim_colorcomponent_ambient_diffuse);
    detectionRayColor.setColorsAllBlack();
    detectionRayColor.setColor(1.0f, 1.0f, 0.0f, sim_colorcomponent_emission);
    detectionRayColor.setEventParams(true, -1, -1, "ray");

    _visibilityLayer = PROXIMITY_SENSOR_LAYER;
    _objectAlias = getObjectTypeInfo();
    _objectName_old = getObjectTypeInfo();
    _objectAltName_old = tt::getObjectAltNameFromObjectName(_objectName_old.c_str());
}

void CProxSensor::setSensableType(int theType)
{
    _sensableType_deprecated = theType;
}

int CProxSensor::getSensableType() const
{
    return (_sensableType_deprecated);
}

void CProxSensor::_setDetectedObjectAndInfo(int h, const C3Vector* detectedPt /*= nullptr*/, const C3Vector* detectedN /*= nullptr*/)
{
    bool diff = (_detectedObjectHandle != h);
    if ((h >= 0) && (!diff))
        diff = ((_detectedPoint != detectedPt[0]) || (_detectedNormalVector != detectedN[0]));
    if (diff)
    {
        _detectedObjectHandle = h;
        if (_detectedObjectHandle >= 0)
        {
            _detectedPoint = detectedPt[0];
            _detectedNormalVector = detectedN[0];
        }
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propProximitySensor_detectedObjectHandle.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyInt(cmd, _detectedObjectHandle);
            ev->appendKeyDoubleArray(propProximitySensor_detectedPoint.name, _detectedPoint.data, 3);
            ev->appendKeyDoubleArray(propProximitySensor_detectedNormal.name, _detectedNormalVector.data, 3);
            App::worldContainer->pushEvent();
        }
    }
}

void CProxSensor::setHideDetectionRay(bool hide)
{
    _hideDetectionRay_deprecated = hide;
}

bool CProxSensor::getHideDetectionRay() const
{
    return (_hideDetectionRay_deprecated);
}

void CProxSensor::setExplicitHandling(bool setExplicit)
{
    if (_explicitHandling != setExplicit)
    {
        _explicitHandling = setExplicit;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propProximitySensor_explicitHandling.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _explicitHandling);
            App::worldContainer->pushEvent();
        }
    }
}

bool CProxSensor::getExplicitHandling() const
{
    return _explicitHandling;
}

int CProxSensor::getSensableObject()
{
    return (_sensableObject_deprecated);
}

void CProxSensor::setSensableObject(int objectID)
{
    _sensableObject_deprecated = objectID;
}

void CProxSensor::removeSceneDependencies()
{
    CSceneObject::removeSceneDependencies();
    _sensableObject_deprecated = -1;
}

void CProxSensor::addSpecializedObjectEventData(CCbor* ev)
{
#if SIM_EVENT_PROTOCOL_VERSION == 2
    ev->openKeyMap("proxSensor");
#else
    volumeColor.addGenesisEventData(ev);
    detectionRayColor.addGenesisEventData(ev);
#endif
    ev->appendKeyDouble(propProximitySensor_size.name, _proxSensorSize);
    ev->appendKeyBool(propProximitySensor_frontFaceDetection.name, _frontFaceDetection);
    ev->appendKeyBool(propProximitySensor_backFaceDetection.name, _backFaceDetection);
    ev->appendKeyBool(propProximitySensor_exactMode.name, _exactMode);
    ev->appendKeyBool(propProximitySensor_explicitHandling.name, _explicitHandling);
    ev->appendKeyBool(propProximitySensor_showVolume.name, _showVolume);
    ev->appendKeyBool(propProximitySensor_randomizedDetection.name, _randomizedDetection);
    ev->appendKeyInt(propProximitySensor_sensorType.name, sensorType);
    ev->appendKeyInt(propProximitySensor_detectedObjectHandle.name, _detectedObjectHandle);
    ev->appendKeyDouble(propProximitySensor_angleThreshold.name, _angleThreshold);
    ev->appendKeyDoubleArray(propProximitySensor_detectedPoint.name, _detectedPoint.data, 3);
    ev->appendKeyDoubleArray(propProximitySensor_detectedNormal.name, _detectedNormalVector.data, 3);
    convexVolume->sendEventData(ev);
#if SIM_EVENT_PROTOCOL_VERSION == 2
    ev->closeArrayOrMap(); // proxSensor
#endif
}

CSceneObject* CProxSensor::copyYourself()
{
    CProxSensor* newSensor = (CProxSensor*)CSceneObject::copyYourself();

    newSensor->_sensableObject_deprecated = _sensableObject_deprecated;
    newSensor->_angleThreshold = _angleThreshold;
    newSensor->_proxSensorSize = _proxSensorSize;
    newSensor->_exactMode = _exactMode;
    newSensor->_frontFaceDetection = _frontFaceDetection;
    newSensor->_backFaceDetection = _backFaceDetection;
    newSensor->_explicitHandling = _explicitHandling;
    newSensor->sensorType = sensorType;
    newSensor->_sensableType_deprecated = _sensableType_deprecated;
    newSensor->_showVolume = _showVolume;

    newSensor->_randomizedDetection = _randomizedDetection;
    newSensor->_randomizedDetectionSampleCount_deprecated = _randomizedDetectionSampleCount_deprecated;
    newSensor->_randomizedDetectionCountForDetection_deprecated = _randomizedDetectionCountForDetection_deprecated;

    delete newSensor->convexVolume;
    newSensor->convexVolume = convexVolume->copyYourself();

    volumeColor.copyYourselfInto(&newSensor->volumeColor);
    detectionRayColor.copyYourselfInto(&newSensor->detectionRayColor);

    newSensor->_initialValuesInitialized = _initialValuesInitialized;
    newSensor->_initialExplicitHandling = _initialExplicitHandling;

    return (newSensor);
}

void CProxSensor::announceObjectWillBeErased(const CSceneObject* object, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceObjectWillBeErased(object, copyBuffer);
    if (_sensableObject_deprecated == object->getObjectHandle())
        _sensableObject_deprecated = -1;
}

void CProxSensor::announceCollectionWillBeErased(int groupID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollectionWillBeErased(groupID, copyBuffer);
    if (_sensableObject_deprecated == groupID)
        _sensableObject_deprecated = -1;
}
void CProxSensor::announceCollisionWillBeErased(int collisionID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollisionWillBeErased(collisionID, copyBuffer);
}
void CProxSensor::announceDistanceWillBeErased(int distanceID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceDistanceWillBeErased(distanceID, copyBuffer);
}
void CProxSensor::announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceIkObjectWillBeErased(ikGroupID, copyBuffer);
}

void CProxSensor::performObjectLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performObjectLoadingMapping(map, opType);
    if (_sensableObject_deprecated <= SIM_IDEND_SCENEOBJECT)
        _sensableObject_deprecated = CWorld::getLoadingMapping(map, _sensableObject_deprecated);
}
void CProxSensor::performCollectionLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performCollectionLoadingMapping(map, opType);
    if (_sensableObject_deprecated > SIM_IDEND_SCENEOBJECT)
        _sensableObject_deprecated = CWorld::getLoadingMapping(map, _sensableObject_deprecated);
}
void CProxSensor::performCollisionLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performCollisionLoadingMapping(map, opType);
}
void CProxSensor::performDistanceLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performDistanceLoadingMapping(map, opType);
}
void CProxSensor::performIkLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performIkLoadingMapping(map, opType);
}

void CProxSensor::performTextureObjectLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performTextureObjectLoadingMapping(map, opType);
}

void CProxSensor::performDynMaterialObjectLoadingMapping(const std::map<int, int>* map)
{
    CSceneObject::performDynMaterialObjectLoadingMapping(map);
}

void CProxSensor::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
    _initialExplicitHandling = _explicitHandling;
}

void CProxSensor::simulationAboutToStart()
{
    initializeInitialValues(false);
    CSceneObject::simulationAboutToStart();
}

void CProxSensor::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it
    // ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized)
    {
        if ((getCumulativeModelProperty() & sim_modelproperty_not_reset) == 0)
            setExplicitHandling(_initialExplicitHandling);
    }
    CSceneObject::simulationEnded();
}

void CProxSensor::serialize(CSer& ar)
{
    CSceneObject::serialize(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            ar.storeDataName("Snt");
            ar << sensorType;
            ar.flush();

            ar.storeDataName("Vod");
            ar.setCountingMode();
            convexVolume->serialize(ar);
            if (ar.setWritingMode())
                convexVolume->serialize(ar);

            ar.storeDataName("_ns");
            ar << _proxSensorSize;
            ar.flush();

            ar.storeDataName("al3");
            ar << _angleThreshold;
            ar.flush();

            ar.storeDataName("_l2"); // for backw. comp. (before V4.8)
            ar << _angleThreshold;
            ar.flush();

            ar.storeDataName("Pr4");
            unsigned char nothing = 0;
            SIM_SET_CLEAR_BIT(nothing, 0, _showVolume);
            SIM_SET_CLEAR_BIT(nothing, 1, _exactMode);
            SIM_SET_CLEAR_BIT(nothing, 2, _angleThreshold > 0.0); // for backw. comp. Now an angle of 0.0 means no angle check
            // 12/12/2011       SIM_SET_CLEAR_BIT(nothing,3,_detectAllDetectable);
            SIM_SET_CLEAR_BIT(nothing, 4, !_frontFaceDetection);
            SIM_SET_CLEAR_BIT(nothing, 5, !_backFaceDetection);
            SIM_SET_CLEAR_BIT(nothing, 6, false); //_showVolumeWhenDetecting);
            SIM_SET_CLEAR_BIT(nothing, 7, _explicitHandling);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Pr5");
            nothing = 0;
            //            SIM_SET_CLEAR_BIT(nothing,0,_checkOcclusions);
            SIM_SET_CLEAR_BIT(nothing, 1, _randomizedDetection);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Rad");
            ar << _randomizedDetectionSampleCount_deprecated << _randomizedDetectionCountForDetection_deprecated;
            ar.flush();

            ar.storeDataName("Cl1");
            ar.setCountingMode();
            volumeColor.serialize(ar, 0);
            if (ar.setWritingMode())
                volumeColor.serialize(ar, 0);

            ar.storeDataName("Cl3");
            ar.setCountingMode();
            detectionRayColor.serialize(ar, 1);
            if (ar.setWritingMode())
                detectionRayColor.serialize(ar, 1);

            ar.storeDataName("Sox");
            ar << _sensableObject_deprecated;
            ar.flush();

            ar.storeDataName("Sst");
            ar << _sensableType_deprecated;
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        { // Loading
            int byteQuantity;
            std::string theName = "";
            bool usingAl3 = false;
            while (theName.compare(SER_END_OF_OBJECT) != 0)
            {
                theName = ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT) != 0)
                {
                    bool noHit = true;
                    if (theName.compare("Snt") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> sensorType;
                    }
                    if (theName.compare("Vod") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        if (convexVolume != nullptr)
                            delete convexVolume;
                        convexVolume = new CConvexVolume();
                        convexVolume->serialize(ar);
                    }
                    if (theName.compare("Sns") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla;
                        ar >> bla;
                        _proxSensorSize = (double)bla;
                    }

                    if (theName.compare("_ns") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _proxSensorSize;
                    }

                    if (theName.compare("Sox") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _sensableObject_deprecated;
                    }
                    if (theName.compare("Sst") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _sensableType_deprecated;
                    }
                    if (theName.compare("Al2") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla;
                        ar >> bla;
                        _angleThreshold = (double)bla;
                    }

                    if (theName.compare("_l2") == 0)
                    { // for backward compatibility
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _angleThreshold;
                    }

                    if (theName.compare("al3") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _angleThreshold;
                        usingAl3 = true;
                    }

                    if (theName == "Pr4")
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _showVolume = SIM_IS_BIT_SET(nothing, 0);
                        _exactMode = SIM_IS_BIT_SET(nothing, 1);
                        if ((!SIM_IS_BIT_SET(nothing, 2)) && (!usingAl3))
                            _angleThreshold = 0.0;
                        _frontFaceDetection = !SIM_IS_BIT_SET(nothing, 4);
                        _backFaceDetection = !SIM_IS_BIT_SET(nothing, 5);
                        //_showVolumeWhenDetecting=SIM_IS_BIT_SET(nothing,6);
                        _explicitHandling = SIM_IS_BIT_SET(nothing, 7);
                    }
                    if (theName == "Pr5")
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        //                        _checkOcclusions=SIM_IS_BIT_SET(nothing,0);
                        _randomizedDetection = SIM_IS_BIT_SET(nothing, 1);
                    }
                    if (theName.compare("Rad") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _randomizedDetectionSampleCount_deprecated >> _randomizedDetectionCountForDetection_deprecated;
                    }
                    if (theName.compare("Cl1") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        volumeColor.serialize(ar, 0);
                    }
                    if (theName.compare("Cl3") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        detectionRayColor.serialize(ar, 1);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }

            if (ar.getSerializationVersionThatWroteThisFile() < 17)
            { // on 29/08/2013 we corrected all default lights. So we need to correct for that change:
                utils::scaleColorUp_(volumeColor.getColorsPtr());
                utils::scaleColorUp_(detectionRayColor.getColorsPtr());
            }
            computeBoundingBox();
        }
    }
    else
    {
        bool exhaustiveXml = ((ar.getFileType() != CSer::filetype_csim_xml_simplescene_file) &&
                              (ar.getFileType() != CSer::filetype_csim_xml_simplemodel_file));
        if (ar.isStoring())
        {
            ar.xmlAddNode_comment(" 'type' tag: can be 'pyramid', 'cylinder', 'disc', 'cone' or 'ray' ", exhaustiveXml);
            ar.xmlAddNode_enum("type", sensorType, sim_proximitysensor_pyramid, "pyramid",
                               sim_proximitysensor_cylinder, "cylinder", sim_proximitysensor_disc,
                               "disc", sim_proximitysensor_cone, "cone", sim_proximitysensor_ray,
                               "ray");

            ar.xmlAddNode_float("size", _proxSensorSize);

            if (exhaustiveXml)
                ar.xmlAddNode_int("detectableEntity", _sensableObject_deprecated);
            else
            {
                std::string str;
                CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(_sensableObject_deprecated);
                if (it != nullptr)
                    str = it->getObjectName_old();
                else
                {
                    CCollection* coll = App::currentWorld->collections->getObjectFromHandle(_sensableObject_deprecated);
                    if (coll != nullptr)
                        str = "@collection@" + coll->getCollectionName();
                }
                ar.xmlAddNode_comment(" 'detectableEntity' tag only provided for backward compatibility, use instead "
                                      "'detectableObjectAlias' tag",
                                      exhaustiveXml);
                ar.xmlAddNode_string("detectableEntity", str.c_str());
                if (it != nullptr)
                {
                    str = it->getObjectAlias() + "*";
                    str += std::to_string(it->getObjectHandle());
                }
                ar.xmlAddNode_string("detectableObjectAlias", str.c_str());
            }

            ar.xmlAddNode_comment(
                " 'detectionType' tag: can be 'ultrasonic', 'infrared', 'laser', 'inductive' or 'capacitive' ",
                exhaustiveXml);
            ar.xmlAddNode_enum("detectionType", _sensableType_deprecated, sim_objectspecialproperty_detectable_ultrasonic,
                               "ultrasonic", sim_objectspecialproperty_detectable_infrared, "infrared",
                               sim_objectspecialproperty_detectable_laser, "laser",
                               sim_objectspecialproperty_detectable_inductive, "inductive",
                               sim_objectspecialproperty_detectable_capacitive, "capacitive");

            ar.xmlAddNode_float("allowedAngle", _angleThreshold * 180.0 / piValue);
            ar.xmlAddNode_comment(" 'allowedNormalAngle' tag: used for backward compatibility ", exhaustiveXml);
            ar.xmlAddNode_float("allowedNormalAngle", _angleThreshold * 180.0 / piValue);

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("showVolumeWhenNotDetecting", _showVolume);
            ar.xmlAddNode_bool("closestObjectMode", _exactMode);
            ar.xmlAddNode_comment(" 'normalCheck' tag: used for backward compatibility ", exhaustiveXml);
            ar.xmlAddNode_bool("normalCheck", _angleThreshold > 0.0);
            ar.xmlAddNode_bool("frontFaceDetection", _frontFaceDetection);
            ar.xmlAddNode_bool("backFaceDetection", _backFaceDetection);
            ar.xmlAddNode_bool("explicitHandling", _explicitHandling);
            ar.xmlPopNode();

            ar.xmlPushNewNode("randomizedDetection");
            ar.xmlAddNode_bool("enabled", _randomizedDetection);
            ar.xmlAddNode_int("sampleCount", _randomizedDetectionSampleCount_deprecated);
            ar.xmlAddNode_int("countForTrigger", _randomizedDetectionCountForDetection_deprecated);
            ar.xmlPopNode();

            ar.xmlPushNewNode("color");
            if (exhaustiveXml)
            {
                ar.xmlPushNewNode("volume");
                volumeColor.serialize(ar, 0);
                ar.xmlPopNode();
                ar.xmlPushNewNode("detectionRay");
                detectionRayColor.serialize(ar, 0);
                ar.xmlPopNode();
            }
            else
            {
                int rgb[3];
                for (size_t l = 0; l < 3; l++)
                    rgb[l] = int(volumeColor.getColorsPtr()[l] * 127.0 + volumeColor.getColorsPtr()[9 + l] * 127.0);
                ar.xmlAddNode_ints("volume", rgb, 3);
                for (size_t l = 0; l < 3; l++)
                    rgb[l] = int(detectionRayColor.getColorsPtr()[l] * 127.1 +
                                 detectionRayColor.getColorsPtr()[9 + l] * 127.1);
                ar.xmlAddNode_ints("detectionRay", rgb, 3);
            }
            ar.xmlPopNode();

            ar.xmlPushNewNode("volume");
            convexVolume->serialize(ar);
            ar.xmlPopNode();
        }
        else
        {
            ar.xmlGetNode_enum("type", sensorType, exhaustiveXml, "pyramid", sim_proximitysensor_pyramid,
                               "cylinder", sim_proximitysensor_cylinder, "disc",
                               sim_proximitysensor_disc, "cone", sim_proximitysensor_cone, "ray",
                               sim_proximitysensor_ray);

            ar.xmlGetNode_float("size", _proxSensorSize, exhaustiveXml);

            if (exhaustiveXml)
                ar.xmlGetNode_int("detectableEntity", _sensableObject_deprecated);
            else
            {
                ar.xmlGetNode_string("detectableObjectAlias", _sensableObjectLoadAlias, exhaustiveXml);
                ar.xmlGetNode_string("detectableEntity", _sensableObjectLoadName_old, exhaustiveXml);
            }

            ar.xmlGetNode_enum("detectionType", _sensableType_deprecated, exhaustiveXml, "ultrasonic",
                               sim_objectspecialproperty_detectable_ultrasonic, "infrared",
                               sim_objectspecialproperty_detectable_infrared, "laser",
                               sim_objectspecialproperty_detectable_laser, "inductive",
                               sim_objectspecialproperty_detectable_inductive, "capacitive",
                               sim_objectspecialproperty_detectable_capacitive);

            bool usingAllowedAngle = ar.xmlGetNode_float("allowedAngle", _angleThreshold, exhaustiveXml);
            if (usingAllowedAngle)
                _angleThreshold *= piValue / 180.0;
            else
            {
                if (ar.xmlGetNode_float("allowedNormalAngle", _angleThreshold, exhaustiveXml))
                    _angleThreshold *= piValue / 180.0;
            }

            if (ar.xmlPushChildNode("switches", exhaustiveXml))
            {
                ar.xmlGetNode_bool("showVolumeWhenNotDetecting", _showVolume, exhaustiveXml);
                ar.xmlGetNode_bool("closestObjectMode", _exactMode, exhaustiveXml);
                if (!usingAllowedAngle)
                {
                    bool ac;
                    if (ar.xmlGetNode_bool("normalCheck", ac, exhaustiveXml))
                    {
                        if (!ac)
                            _angleThreshold = 0.0;
                    }
                    else
                        _angleThreshold = 0.0;
                }
                ar.xmlGetNode_bool("frontFaceDetection", _frontFaceDetection, exhaustiveXml);
                ar.xmlGetNode_bool("backFaceDetection", _backFaceDetection, exhaustiveXml);
                ar.xmlGetNode_bool("explicitHandling", _explicitHandling, exhaustiveXml);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("randomizedDetection", exhaustiveXml))
            {
                ar.xmlGetNode_bool("enabled", _randomizedDetection, exhaustiveXml);
                ar.xmlGetNode_int("sampleCount", _randomizedDetectionSampleCount_deprecated, exhaustiveXml);
                ar.xmlGetNode_int("countForTrigger", _randomizedDetectionCountForDetection_deprecated, exhaustiveXml);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("color", exhaustiveXml))
            {
                if (exhaustiveXml)
                {
                    if (ar.xmlPushChildNode("passiveVolume", false))
                    { // for backward compatibility
                        volumeColor.serialize(ar, 0);
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("volume", false))
                    {
                        volumeColor.serialize(ar, 0);
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("detectionRay"))
                    {
                        detectionRayColor.serialize(ar, 0);
                        ar.xmlPopNode();
                    }
                }
                else
                {
                    int rgb[3];
                    if (ar.xmlGetNode_ints("passiveVolume", rgb, 3, false))
                        volumeColor.setColor(float(rgb[0]) / 255.1, float(rgb[1]) / 255.1, float(rgb[2]) / 255.1,
                                             sim_colorcomponent_ambient_diffuse);
                    if (ar.xmlGetNode_ints("volume", rgb, 3, false))
                        volumeColor.setColor(float(rgb[0]) / 255.1, float(rgb[1]) / 255.1, float(rgb[2]) / 255.1,
                                             sim_colorcomponent_ambient_diffuse);
                    if (ar.xmlGetNode_ints("detectionRay", rgb, 3, false))
                        detectionRayColor.setColor(float(rgb[0]) / 255.1, float(rgb[1]) / 255.1, float(rgb[2]) / 255.1,
                                                   sim_colorcomponent_ambient_diffuse);
                }
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("volume", exhaustiveXml))
            {
                if (convexVolume != nullptr)
                    delete convexVolume;
                convexVolume = new CConvexVolume();

                if (!exhaustiveXml)
                {
                    if (sensorType == sim_proximitysensor_ray)
                        convexVolume->setVolumeType(RAY_TYPE_CONVEX_VOLUME, _objectType, _proxSensorSize);
                    else
                        _randomizedDetection = false;
                    if (sensorType == sim_proximitysensor_cylinder)
                        convexVolume->setVolumeType(CYLINDER_TYPE_CONVEX_VOLUME, _objectType, _proxSensorSize);
                    if (sensorType == sim_proximitysensor_disc)
                        convexVolume->setVolumeType(DISC_TYPE_CONVEX_VOLUME, _objectType, _proxSensorSize);
                    if (sensorType == sim_proximitysensor_pyramid)
                        convexVolume->setVolumeType(PYRAMID_TYPE_CONVEX_VOLUME, _objectType, _proxSensorSize);
                    if (sensorType == sim_proximitysensor_cone)
                        convexVolume->setVolumeType(CONE_TYPE_CONVEX_VOLUME, _objectType, _proxSensorSize);
                }

                convexVolume->serialize(ar);
                ar.xmlPopNode();
            }
            computeBoundingBox();
        }
    }
}

bool CProxSensor::getSensingVolumeBoundingBox(C3Vector& minV, C3Vector& maxV) const
{
    return (convexVolume->getVolumeBoundingBox(minV, maxV));
}

void CProxSensor::getSensingVolumeOBB(C7Vector& tr, C3Vector& halfSizes)
{
    C3Vector minV, maxV, center;
    convexVolume->getVolumeBoundingBox(minV, maxV);
    center = (minV + maxV) * 0.5;
    C4X4Matrix m(getFullCumulativeTransformation().getMatrix());
    center = m * center;
    m.X = center;
    halfSizes = (maxV - minV) * 0.5;
    tr = m.getTransformation();
}

void CProxSensor::computeBoundingBox()
{
    _setBB(C7Vector::identityTransformation, C3Vector(1.0, 1.0, 1.0) * _proxSensorSize * 0.5);
}

void CProxSensor::setIsInScene(bool s)
{
    CSceneObject::setIsInScene(s);
    if (s)
    {
        volumeColor.setEventParams(true, _objectHandle);
        detectionRayColor.setEventParams(true, _objectHandle);
        convexVolume->setParentObjHandleForEvents(_objectHandle);
    }
    else
    {
        volumeColor.setEventParams(true, -1);
        detectionRayColor.setEventParams(true, -1);
        convexVolume->setParentObjHandleForEvents(-1);
    }
}

void CProxSensor::calculateFreshRandomizedRays()
{
    _randomizedVectors.clear();
    _randomizedVectorDetectionStates.clear();
    // Build the random rays (only direction)
    double angle = convexVolume->getAngle();
    for (int i = 0; i < _randomizedDetectionSampleCount_deprecated; i++)
    {
        double rZ, sZ, cZ;
        if (angle > 1.1 * piValD2)
        { // this means we have 360x180 degrees. We compute it as 2 half-spheres, in order to have a perfect direction
            // distribution:
            rZ = acos(1.0 - SIM_RAND_FLOAT);
            sZ = sin(rZ);
            cZ = cos(rZ);
            if (SIM_RAND_FLOAT > 0.5)
                cZ = -cZ;
        }
        else
        { // this means we have 360xA degrees, where A<=90.
            rZ = angle * acos(1.0 - SIM_RAND_FLOAT) / piValue;
            sZ = sin(rZ);
            cZ = cos(rZ);
        }
        double rXY = SIM_RAND_FLOAT * piValT2;
        double sXY = sin(rXY);
        double cXY = cos(rXY);
        C3Vector v(sZ * cXY, sZ * sXY, cZ);
        _randomizedVectors.push_back(v);
        _randomizedVectorDetectionStates.push_back(0.0);
    }
}

const std::vector<C3Vector>* CProxSensor::getPointerToRandomizedRays() const
{
    return (&_randomizedVectors);
}

std::vector<double>* CProxSensor::getPointerToRandomizedRayDetectionStates()
{
    return (&_randomizedVectorDetectionStates);
}

void CProxSensor::resetSensor(bool exceptExplicitHandling)
{
    if ((!exceptExplicitHandling) || (!getExplicitHandling()))
    {
        _setDetectedObjectAndInfo(-1);
        _calcTimeInMs = 0;
    }
}

bool CProxSensor::handleSensor(bool exceptExplicitHandling, int& detectedObjectHandle, C3Vector& detectedNormalVector)
{
    if (exceptExplicitHandling && getExplicitHandling())
        return (false); // We don't want to handle those
    _calcTimeInMs = 0;
    if (!App::currentWorld->mainSettings_old->proximitySensorsEnabled)
        return (false);
    if (!App::worldContainer->pluginContainer->isGeomPluginAvailable())
        return (false);
    int stTime = (int)VDateTime::getTimeInMs();

    double treshhold = DBL_MAX;
    double minThreshold = -1.0;
    if (convexVolume->getSmallestDistanceAllowed() > 0.0)
        minThreshold = convexVolume->getSmallestDistanceAllowed();
    C3Vector detectedP;
    _randomizedVectors.clear();
    _randomizedVectorDetectionStates.clear();
    bool detectedPointValid = CProxSensorRoutine::detectEntity(
        _objectHandle, _sensableObject_deprecated, _exactMode, _angleThreshold > 0.0, _angleThreshold, detectedP, treshhold,
        _frontFaceDetection, _backFaceDetection, detectedObjectHandle, minThreshold, detectedNormalVector, false);
    int detectedObject = detectedObjectHandle;
    C3Vector detectedN = detectedNormalVector;
    _calcTimeInMs = VDateTime::getTimeDiffInMs(stTime);
    if (detectedPointValid && (detectedObject >= 0) && VThread::isSimThread())
    {
        std::vector<CScriptObject*> scripts;
        getAttachedScripts(scripts, -1, true);
        getAttachedScripts(scripts, -1, false);

        if (scripts.size() > 0)
        {
            CInterfaceStack* inStack = App::worldContainer->interfaceStackContainer->createStack();
            inStack->pushTableOntoStack();

            inStack->insertKeyInt32IntoStackTable("handle", getObjectHandle());
            inStack->insertKeyInt32IntoStackTable("detectedObjectHandle", detectedObject);
            inStack->insertKeyDoubleArrayIntoStackTable("detectedPoint", detectedP.data, 3);
            inStack->insertKeyDoubleArrayIntoStackTable("normalVector", detectedN.data, 3);

            for (size_t i = 0; i < scripts.size(); i++)
            {
                CScriptObject* script = scripts[i];
                if (script->hasSystemFunctionOrHook(sim_syscb_trigger))
                {
                    bool hasTriggerWord = false;
                    CInterfaceStack* outStack = App::worldContainer->interfaceStackContainer->createStack();
                    script->systemCallScript(sim_syscb_trigger, inStack, outStack);
                    if (outStack->getStackSize() >= 1)
                    {
                        outStack->moveStackItemToTop(0);
                        bool trig = false;
                        if (outStack->getStackMapBoolValue("trigger", trig))
                        {
                            hasTriggerWord = true;
                            if (!trig)
                                detectedObject = -1;
                        }
                    }
                    App::worldContainer->interfaceStackContainer->destroyStack(outStack);
                    if (hasTriggerWord)
                        break;
                }
            }
            App::worldContainer->interfaceStackContainer->destroyStack(inStack);
        }
    }
    _setDetectedObjectAndInfo(detectedObject, &detectedP, &detectedN);
    return (detectedObject >= 0);
}

int CProxSensor::readSensor(C3Vector& detectPt, int& detectedObjectHandle, C3Vector& detectedNormalVector)
{
    int retVal = 0;
    if (_detectedObjectHandle >= 0)
    {
        detectPt = _detectedPoint;
        detectedObjectHandle = _detectedObjectHandle;
        detectedNormalVector = _detectedNormalVector;
        retVal = 1;
    }
    return retVal;
}

double CProxSensor::getCalculationTime() const
{
    return (double(_calcTimeInMs) * 0.001);
}

bool CProxSensor::getFrontFaceDetection() const
{
    return _frontFaceDetection;
}

bool CProxSensor::getBackFaceDetection() const
{
    return _backFaceDetection;
}

void CProxSensor::setFrontFaceDetection(bool faceOn)
{
    if (_frontFaceDetection != faceOn)
    {
        _frontFaceDetection = faceOn;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propProximitySensor_frontFaceDetection.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _frontFaceDetection);
            App::worldContainer->pushEvent();
        }
        if (!faceOn)
            setBackFaceDetection(true);
    }
}

void CProxSensor::setBackFaceDetection(bool faceOn)
{
    if (_backFaceDetection != faceOn)
    {
        _backFaceDetection = faceOn;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propProximitySensor_backFaceDetection.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _backFaceDetection);
            App::worldContainer->pushEvent();
        }
        if (!faceOn)
            setFrontFaceDetection(true);
    }
}

void CProxSensor::setAllowedNormal(double al)
{
    tt::limitValue(0.0 * degToRad, 90.0 * degToRad, al);
    if (_angleThreshold != al)
    {
        _angleThreshold = al;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propProximitySensor_angleThreshold.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDouble(cmd, _angleThreshold);
            App::worldContainer->pushEvent();
        }
    }
}

double CProxSensor::getAllowedNormal() const
{
    return _angleThreshold;
}

void CProxSensor::setExactMode(bool closestObjMode)
{
    if (_exactMode != closestObjMode)
    {
        _exactMode = closestObjMode;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propProximitySensor_exactMode.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _exactMode);
            App::worldContainer->pushEvent();
        }
        _setDetectedObjectAndInfo(-1);
        _calcTimeInMs = 0;
    }
}

bool CProxSensor::getExactMode()
{
    return _exactMode;
}

void CProxSensor::setProxSensorSize(double newSize)
{
    tt::limitValue(0.0001, 10.0, newSize);
    bool diff = (_proxSensorSize != newSize);
    if (diff)
    {
        _proxSensorSize = newSize;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propProximitySensor_size.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDouble(cmd, _proxSensorSize);
            App::worldContainer->pushEvent();
        }
        computeBoundingBox();
    }
}

double CProxSensor::getProxSensorSize()
{
    return (_proxSensorSize);
}

bool CProxSensor::getSensedData(C3Vector& pt)
{
    bool retVal = false;
    if (_detectedObjectHandle >= 0)
    {
        pt = _detectedPoint;
        retVal = true;
    }
    return retVal;
}

void CProxSensor::scaleObject(double scalingFactor)
{
    convexVolume->scaleVolume(scalingFactor);
    setProxSensorSize(_proxSensorSize * scalingFactor);

    CSceneObject::scaleObject(scalingFactor);
}

C3Vector CProxSensor::getDetectedPoint() const
{
    return _detectedPoint;
}

bool CProxSensor::getIsDetectedPointValid() const
{
    return (_detectedObjectHandle >= 0);
}

std::string CProxSensor::getSensableObjectLoadAlias() const
{
    return (_sensableObjectLoadAlias);
}

std::string CProxSensor::getSensableObjectLoadName_old() const
{
    return (_sensableObjectLoadName_old);
}

CColorObject* CProxSensor::getColor(int index)
{
    if (index == 0)
        return (&volumeColor);
    if (index == 1)
        return (&detectionRayColor);
    return (nullptr);
}

int CProxSensor::getSensorType() const
{
    return sensorType;
}

#ifdef SIM_WITH_GUI
void CProxSensor::display(CViewableBase* renderingObject, int displayAttrib)
{
    displayProximitySensor(this, renderingObject, displayAttrib);
}
#endif

int CProxSensor::setBoolProperty(const char* ppName, bool pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setBoolProperty(ppName, pState);
    if (retVal == -1)
        retVal = convexVolume->setBoolProperty(ppName, pState);
    if (retVal == -1)
    {
        if (_pName == propProximitySensor_frontFaceDetection.name)
        {
            retVal = 1;
            setFrontFaceDetection(pState);
        }
        else if (_pName == propProximitySensor_backFaceDetection.name)
        {
            retVal = 1;
            setBackFaceDetection(pState);
        }
        else if (_pName == propProximitySensor_exactMode.name)
        {
            retVal = 1;
            setExactMode(pState);
        }
        else if (_pName == propProximitySensor_explicitHandling.name)
        {
            retVal = 1;
            setExplicitHandling(pState);
        }
        else if (_pName == propProximitySensor_showVolume.name)
        {
            retVal = 1;
            setShowVolume(pState);
        }
    }

    return retVal;
}

int CProxSensor::getBoolProperty(const char* ppName, bool& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getBoolProperty(ppName, pState);
    if (retVal == -1)
        retVal = convexVolume->getBoolProperty(ppName, pState);
    if (retVal == -1)
    {
        if (_pName == propProximitySensor_frontFaceDetection.name)
        {
            retVal = 1;
            pState = _frontFaceDetection;
        }
        else if (_pName == propProximitySensor_backFaceDetection.name)
        {
            retVal = 1;
            pState = _backFaceDetection;
        }
        else if (_pName == propProximitySensor_exactMode.name)
        {
            retVal = 1;
            pState = _exactMode;
        }
        else if (_pName == propProximitySensor_explicitHandling.name)
        {
            retVal = 1;
            pState = _explicitHandling;
        }
        else if (_pName == propProximitySensor_showVolume.name)
        {
            retVal = 1;
            pState = _showVolume;
        }
        else if (_pName == propProximitySensor_randomizedDetection.name)
        {
            retVal = 1;
            pState = _randomizedDetection;
        }
    }

    return retVal;
}

int CProxSensor::setIntProperty(const char* ppName, int pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setIntProperty(ppName, pState);
    if (retVal == -1)
        retVal = convexVolume->setIntProperty(ppName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CProxSensor::getIntProperty(const char* ppName, int& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getIntProperty(ppName, pState);
    if (retVal == -1)
        retVal = convexVolume->getIntProperty(ppName, pState);
    if (retVal == -1)
    {
        if (_pName == propProximitySensor_sensorType.name)
        {
            retVal = 1;
            pState = sensorType;
        }
        else if (_pName == propProximitySensor_detectedObjectHandle.name)
        {
            retVal = 1;
            pState = _detectedObjectHandle;
        }
    }

    return retVal;
}

int CProxSensor::setFloatProperty(const char* ppName, double pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setFloatProperty(ppName, pState);
    if (retVal == -1)
        retVal = volumeColor.setFloatProperty(ppName, pState);
    if (retVal == -1)
        retVal = detectionRayColor.setFloatProperty(ppName, pState);
    if (retVal == -1)
        retVal = convexVolume->setFloatProperty(ppName, pState);
    if (retVal == -1)
    {
        if (_pName == propProximitySensor_size.name)
        {
            setProxSensorSize(pState);
            retVal = 1;
        }
        else if (_pName == propProximitySensor_angleThreshold.name)
        {
            retVal = 1;
            setAllowedNormal(pState);
        }
    }

    return retVal;
}

int CProxSensor::getFloatProperty(const char* ppName, double& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getFloatProperty(ppName, pState);
    if (retVal == -1)
        retVal = volumeColor.getFloatProperty(ppName, pState);
    if (retVal == -1)
        retVal = detectionRayColor.getFloatProperty(ppName, pState);
    if (retVal == -1)
        retVal = convexVolume->getFloatProperty(ppName, pState);
    if (retVal == -1)
    {
        if (_pName == propProximitySensor_size.name)
        {
            pState = _proxSensorSize;
            retVal = 1;
        }
        else if (_pName == propProximitySensor_angleThreshold.name)
        {
            retVal = 1;
            pState = _angleThreshold;
        }
    }

    return retVal;
}

int CProxSensor::getStringProperty(const char* ppName, std::string& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getStringProperty(ppName, pState);
    if (retVal == -1)
    {
        if (_pName == propProximitySensor_objectMetaInfo.name)
        {
            pState = OBJECT_META_INFO;
            retVal = 1;
        }
    }

    return retVal;
}

int CProxSensor::setIntArray2Property(const char* ppName, const int* pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setIntArray2Property(ppName, pState);
    if (retVal == -1)
        retVal = convexVolume->setIntArray2Property(ppName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CProxSensor::getIntArray2Property(const char* ppName, int* pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getIntArray2Property(ppName, pState);
    if (retVal == -1)
        retVal = convexVolume->getIntArray2Property(ppName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CProxSensor::setVector2Property(const char* ppName, const double* pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setVector2Property(ppName, pState);
    if (retVal == -1)
        retVal = convexVolume->setVector2Property(ppName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CProxSensor::getVector2Property(const char* ppName, double* pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getVector2Property(ppName, pState);
    if (retVal == -1)
        retVal = convexVolume->getVector2Property(ppName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CProxSensor::setVector3Property(const char* ppName, const C3Vector& pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setVector3Property(ppName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CProxSensor::getVector3Property(const char* ppName, C3Vector& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getVector3Property(ppName, pState);
    if (retVal == -1)
    {
        if (_pName == propProximitySensor_detectedPoint.name)
        {
            pState = _detectedPoint;
            retVal = 1;
        }
        else if (_pName == propProximitySensor_detectedNormal.name)
        {
            retVal = 1;
            pState = _detectedNormalVector;
        }
    }

    return retVal;
}

int CProxSensor::setColorProperty(const char* ppName, const float* pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setColorProperty(ppName, pState);
    if (retVal == -1)
        retVal = volumeColor.setColorProperty(ppName, pState);
    if (retVal == -1)
        retVal = detectionRayColor.setColorProperty(ppName, pState);
    return retVal;
}

int CProxSensor::getColorProperty(const char* ppName, float* pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getColorProperty(ppName, pState);
    if (retVal == -1)
        retVal = volumeColor.getColorProperty(ppName, pState);
    if (retVal == -1)
        retVal = detectionRayColor.getColorProperty(ppName, pState);
    return retVal;
}

int CProxSensor::setFloatArrayProperty(const char* ppName, const double* v, int vL)
{
    std::string _pName(ppName);
    if (v == nullptr)
        vL = 0;
    int retVal = CSceneObject::setFloatArrayProperty(ppName, v, vL);
    if (retVal == -1)
        retVal = convexVolume->setFloatArrayProperty(ppName, v, vL);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CProxSensor::getFloatArrayProperty(const char* ppName, std::vector<double>& pState) const
{
    std::string _pName(ppName);
    pState.clear();
    int retVal = CSceneObject::getFloatArrayProperty(ppName, pState);
    if (retVal == -1)
        retVal = convexVolume->getFloatArrayProperty(ppName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CProxSensor::setIntArrayProperty(const char* ppName, const int* v, int vL)
{
    std::string _pName(ppName);
    if (v == nullptr)
        vL = 0;
    int retVal = CSceneObject::setIntArrayProperty(ppName, v, vL);
    if (retVal == -1)
        retVal = convexVolume->setIntArrayProperty(ppName, v, vL);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CProxSensor::getIntArrayProperty(const char* ppName, std::vector<int>& pState) const
{
    std::string _pName(ppName);
    pState.clear();
    int retVal = CSceneObject::getIntArrayProperty(ppName, pState);
    if (retVal == -1)
        retVal = convexVolume->getIntArrayProperty(ppName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CProxSensor::getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const
{
    int retVal = CSceneObject::getPropertyName(index, pName, appartenance, excludeFlags);
    if (retVal == -1)
    {
        appartenance = "proximitySensor";
        retVal = volumeColor.getPropertyName(index, pName, excludeFlags);
    }
    if (retVal == -1)
        retVal = detectionRayColor.getPropertyName(index, pName, excludeFlags);
    if (retVal == -1)
        retVal = convexVolume->getPropertyName(index, pName, excludeFlags);
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_proximitySensor.size(); i++)
        {
            if ((pName.size() == 0) || utils::startsWith(allProps_proximitySensor[i].name, pName.c_str()))
            {
                if ((allProps_proximitySensor[i].flags & excludeFlags) == 0)
                {
                    index--;
                    if (index == -1)
                    {
                        pName = allProps_proximitySensor[i].name;
                        retVal = 1;
                        break;
                    }
                }
            }
        }
    }
    return retVal;
}

int CProxSensor::getPropertyName_static(int& index, std::string& pName, std::string& appartenance, int excludeFlags)
{
    int retVal = CSceneObject::getPropertyName_bstatic(index, pName, appartenance, excludeFlags);
    if (retVal == -1)
    {
        appartenance = "proximitySensor";
        retVal = CColorObject::getPropertyName_static(index, pName, 1 + 4 + 8, "", excludeFlags);
    }
    if (retVal == -1)
        retVal = CColorObject::getPropertyName_static(index, pName, 1 + 4 + 8, "ray", excludeFlags);
    if (retVal == -1)
        retVal = CConvexVolume::getPropertyName_static(index, pName, excludeFlags);
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_proximitySensor.size(); i++)
        {
            if ((pName.size() == 0) || utils::startsWith(allProps_proximitySensor[i].name, pName.c_str()))
            {
                if ((allProps_proximitySensor[i].flags & excludeFlags) == 0)
                {
                    index--;
                    if (index == -1)
                    {
                        pName = allProps_proximitySensor[i].name;
                        retVal = 1;
                        break;
                    }
                }
            }
        }
    }
    return retVal;
}

int CProxSensor::getPropertyInfo(const char* ppName, int& info, std::string& infoTxt) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getPropertyInfo(ppName, info, infoTxt);
    if (retVal == -1)
        retVal = volumeColor.getPropertyInfo(ppName, info, infoTxt);
    if (retVal == -1)
        retVal = detectionRayColor.getPropertyInfo(ppName, info, infoTxt);
    if (retVal == -1)
        retVal = convexVolume->getPropertyInfo(ppName, info, infoTxt);
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_proximitySensor.size(); i++)
        {
            if (strcmp(allProps_proximitySensor[i].name, ppName) == 0)
            {
                retVal = allProps_proximitySensor[i].type;
                info = allProps_proximitySensor[i].flags;
                if (infoTxt == "j")
                    infoTxt = allProps_proximitySensor[i].shortInfoTxt;
                else
                {
                    auto w = QJsonDocument::fromJson(allProps_proximitySensor[i].shortInfoTxt.c_str()).object();
                    std::string descr = w["description"].toString().toStdString();
                    std::string label = w["label"].toString().toStdString();
                    if ( (infoTxt == "s") || (descr == "") )
                        infoTxt = label;
                    else
                        infoTxt = descr;
                }
                break;
            }
        }
    }
    return retVal;
}

int CProxSensor::getPropertyInfo_static(const char* ppName, int& info, std::string& infoTxt)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getPropertyInfo_bstatic(ppName, info, infoTxt);
    if (retVal == -1)
        retVal = CColorObject::getPropertyInfo_static(ppName, info, infoTxt, 1 + 4 + 8, "");
    if (retVal == -1)
        retVal = CColorObject::getPropertyInfo_static(ppName, info, infoTxt, 1 + 4 + 8, "_ray");
    if (retVal == -1)
        retVal = CConvexVolume::getPropertyInfo_static(ppName, info, infoTxt);
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_proximitySensor.size(); i++)
        {
            if (strcmp(allProps_proximitySensor[i].name, ppName) == 0)
            {
                retVal = allProps_proximitySensor[i].type;
                info = allProps_proximitySensor[i].flags;
                if (infoTxt == "j")
                    infoTxt = allProps_proximitySensor[i].shortInfoTxt;
                else
                {
                    auto w = QJsonDocument::fromJson(allProps_proximitySensor[i].shortInfoTxt.c_str()).object();
                    std::string descr = w["description"].toString().toStdString();
                    std::string label = w["label"].toString().toStdString();
                    if ( (infoTxt == "s") || (descr == "") )
                        infoTxt = label;
                    else
                        infoTxt = descr;
                }
                break;
            }
        }
    }
    return retVal;
}
