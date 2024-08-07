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
        bool forbiddenDistEnable = convexVolume->getSmallestDistanceEnabled();
        double forbiddenDist = convexVolume->getSmallestDistanceAllowed();

        if (enable)
        {
            convexVolume->setVolumeType(CONE_TYPE_CONVEX_VOLUME, _objectType, _proxSensorSize);
            convexVolume->setOffset(0.0);
            convexVolume->setRadius(off);
            convexVolume->setRange(range);
            convexVolume->setSmallestDistanceEnabled(forbiddenDistEnable);
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
            convexVolume->setSmallestDistanceEnabled(forbiddenDistEnable);
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
    _randomizedDetectionSampleCount = tt::getLimitedInt(1, 5000, c);
    _randomizedDetectionCountForDetection =
        tt::getLimitedInt(1, _randomizedDetectionSampleCount, _randomizedDetectionCountForDetection);
}

int CProxSensor::getRandomizedDetectionSampleCount() const
{
    return (_randomizedDetectionSampleCount);
}

void CProxSensor::setRandomizedDetectionCountForDetection(int c)
{
    _randomizedDetectionCountForDetection = tt::getLimitedInt(1, _randomizedDetectionSampleCount, c);
}

int CProxSensor::getRandomizedDetectionCountForDetection() const
{
    return (_randomizedDetectionCountForDetection);
}

void CProxSensor::setShowVolume(bool s)
{
    _showVolume = s;
}
bool CProxSensor::getShowVolume() const
{
    return (_showVolume);
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
    explicitHandling = false;
    displayNormals = false;
    _objectType = sim_sceneobject_proximitysensor;
    frontFaceDetection = true;
    backFaceDetection = true;
    closestObjectMode = true;
    normalCheck = false;
    allowedNormal = 45.0 * degToRad;
    _hideDetectionRay = false;

    _randomizedDetection = false;
    _randomizedDetectionSampleCount = 1;
    _randomizedDetectionCountForDetection = 1;

    _proxSensorSize = 0.01;
    _showVolume = true;
    _localObjectSpecialProperty = 0;

    _sensableObject = -1;
    _sensableType = sim_objectspecialproperty_detectable_ultrasonic;
    _sensorResultValid = false;
    _detectedPointValid = false;
    _calcTimeInMs = 0;

    volumeColor.setColorsAllBlack();
    volumeColor.setColor(0.9f, 0.0f, 0.5f, sim_colorcomponent_ambient_diffuse);
    detectionRayColor.setColorsAllBlack();
    detectionRayColor.setColor(1.0f, 1.0f, 0.0f, sim_colorcomponent_emission);
    detectionRayColor.setEventParams(-1, -1, "_ray");

    _visibilityLayer = PROXIMITY_SENSOR_LAYER;
    _objectAlias = getObjectTypeInfo();
    _objectName_old = getObjectTypeInfo();
    _objectAltName_old = tt::getObjectAltNameFromObjectName(_objectName_old.c_str());
}

void CProxSensor::setSensableType(int theType)
{
    _sensableType = theType;
}

int CProxSensor::getSensableType() const
{
    return (_sensableType);
}

void CProxSensor::setHideDetectionRay(bool hide)
{
    _hideDetectionRay = hide;
}

bool CProxSensor::getHideDetectionRay() const
{
    return (_hideDetectionRay);
}

void CProxSensor::setExplicitHandling(bool setExplicit)
{
    explicitHandling = setExplicit;
}

bool CProxSensor::getExplicitHandling() const
{
    return (explicitHandling);
}

int CProxSensor::getSensableObject()
{
    return (_sensableObject);
}

void CProxSensor::setSensableObject(int objectID)
{
    _sensableObject = objectID;
}

void CProxSensor::removeSceneDependencies()
{
    CSceneObject::removeSceneDependencies();
    _sensableObject = -1;
}

void CProxSensor::addSpecializedObjectEventData(CCbor *ev) const
{
#if SIM_EVENT_PROTOCOL_VERSION == 2
    ev->openKeyMap("proxSensor");
#else
    volumeColor.addGenesisEventData(ev);
    detectionRayColor.addGenesisEventData(ev);
#endif
    ev->appendKeyDouble(propProximitySensor_size.name, _proxSensorSize);
    // todo
#if SIM_EVENT_PROTOCOL_VERSION == 2
    ev->closeArrayOrMap(); // proxSensor
#endif
}

CSceneObject *CProxSensor::copyYourself()
{
    CProxSensor *newSensor = (CProxSensor *)CSceneObject::copyYourself();

    newSensor->_sensableObject = _sensableObject;
    newSensor->_detectedPoint = _detectedPoint;
    newSensor->_detectedPointValid = false;
    newSensor->allowedNormal = allowedNormal;
    newSensor->_proxSensorSize = _proxSensorSize;
    newSensor->normalCheck = normalCheck;
    newSensor->closestObjectMode = closestObjectMode;
    newSensor->frontFaceDetection = frontFaceDetection;
    newSensor->backFaceDetection = backFaceDetection;
    newSensor->explicitHandling = explicitHandling;
    newSensor->sensorType = sensorType;
    newSensor->_sensableType = _sensableType;
    newSensor->displayNormals = displayNormals;
    newSensor->_showVolume = _showVolume;

    newSensor->_randomizedDetection = _randomizedDetection;
    newSensor->_randomizedDetectionSampleCount = _randomizedDetectionSampleCount;
    newSensor->_randomizedDetectionCountForDetection = _randomizedDetectionCountForDetection;

    delete newSensor->convexVolume;
    newSensor->convexVolume = convexVolume->copyYourself();

    volumeColor.copyYourselfInto(&newSensor->volumeColor);
    detectionRayColor.copyYourselfInto(&newSensor->detectionRayColor);

    newSensor->_initialValuesInitialized = _initialValuesInitialized;
    newSensor->_initialExplicitHandling = _initialExplicitHandling;

    return (newSensor);
}

void CProxSensor::announceObjectWillBeErased(const CSceneObject *object, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceObjectWillBeErased(object, copyBuffer);
    if (_sensableObject == object->getObjectHandle())
        _sensableObject = -1;
}

void CProxSensor::announceCollectionWillBeErased(int groupID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollectionWillBeErased(groupID, copyBuffer);
    if (_sensableObject == groupID)
        _sensableObject = -1;
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

void CProxSensor::performObjectLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performObjectLoadingMapping(map, loadingAmodel);
    if (_sensableObject <= SIM_IDEND_SCENEOBJECT)
        _sensableObject = CWorld::getLoadingMapping(map, _sensableObject);
}
void CProxSensor::performCollectionLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performCollectionLoadingMapping(map, loadingAmodel);
    if (_sensableObject > SIM_IDEND_SCENEOBJECT)
        _sensableObject = CWorld::getLoadingMapping(map, _sensableObject);
}
void CProxSensor::performCollisionLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performCollisionLoadingMapping(map, loadingAmodel);
}
void CProxSensor::performDistanceLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performDistanceLoadingMapping(map, loadingAmodel);
}
void CProxSensor::performIkLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performIkLoadingMapping(map, loadingAmodel);
}

void CProxSensor::performTextureObjectLoadingMapping(const std::map<int, int> *map)
{
    CSceneObject::performTextureObjectLoadingMapping(map);
}

void CProxSensor::performDynMaterialObjectLoadingMapping(const std::map<int, int> *map)
{
    CSceneObject::performDynMaterialObjectLoadingMapping(map);
}

void CProxSensor::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
    _initialExplicitHandling = explicitHandling;
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
        if (App::currentWorld->simulation->getResetSceneAtSimulationEnd() &&
            ((getCumulativeModelProperty() & sim_modelproperty_not_reset) == 0))
        {
            explicitHandling = _initialExplicitHandling;
        }
    }
    CSceneObject::simulationEnded();
}

void CProxSensor::serialize(CSer &ar)
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

            ar.storeDataName("_l2");
            ar << allowedNormal;
            ar.flush();

            ar.storeDataName("Pr4");
            unsigned char nothing = 0;
            SIM_SET_CLEAR_BIT(nothing, 0, _showVolume);
            SIM_SET_CLEAR_BIT(nothing, 1, closestObjectMode);
            SIM_SET_CLEAR_BIT(nothing, 2, normalCheck);
            // 12/12/2011       SIM_SET_CLEAR_BIT(nothing,3,_detectAllDetectable);
            SIM_SET_CLEAR_BIT(nothing, 4, !frontFaceDetection);
            SIM_SET_CLEAR_BIT(nothing, 5, !backFaceDetection);
            SIM_SET_CLEAR_BIT(nothing, 6, false); //_showVolumeWhenDetecting);
            SIM_SET_CLEAR_BIT(nothing, 7, explicitHandling);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Pr5");
            nothing = 0;
            //            SIM_SET_CLEAR_BIT(nothing,0,_checkOcclusions);
            SIM_SET_CLEAR_BIT(nothing, 1, _randomizedDetection);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Rad");
            ar << _randomizedDetectionSampleCount << _randomizedDetectionCountForDetection;
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
            ar << _sensableObject;
            ar.flush();

            ar.storeDataName("Sst");
            ar << _sensableType;
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        { // Loading
            int byteQuantity;
            std::string theName = "";
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
                        ar >> _sensableObject;
                    }
                    if (theName.compare("Sst") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _sensableType;
                    }
                    if (theName.compare("Al2") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla;
                        ar >> bla;
                        allowedNormal = (double)bla;
                    }

                    if (theName.compare("_l2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> allowedNormal;
                    }

                    if (theName == "Pr4")
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _showVolume = SIM_IS_BIT_SET(nothing, 0);
                        closestObjectMode = SIM_IS_BIT_SET(nothing, 1);
                        normalCheck = SIM_IS_BIT_SET(nothing, 2);
                        frontFaceDetection = !SIM_IS_BIT_SET(nothing, 4);
                        backFaceDetection = !SIM_IS_BIT_SET(nothing, 5);
                        //_showVolumeWhenDetecting=SIM_IS_BIT_SET(nothing,6);
                        explicitHandling = SIM_IS_BIT_SET(nothing, 7);
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
                        ar >> _randomizedDetectionSampleCount >> _randomizedDetectionCountForDetection;
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
                ar.xmlAddNode_int("detectableEntity", _sensableObject);
            else
            {
                std::string str;
                CSceneObject *it = App::currentWorld->sceneObjects->getObjectFromHandle(_sensableObject);
                if (it != nullptr)
                    str = it->getObjectName_old();
                else
                {
                    CCollection *coll = App::currentWorld->collections->getObjectFromHandle(_sensableObject);
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
            ar.xmlAddNode_enum("detectionType", _sensableType, sim_objectspecialproperty_detectable_ultrasonic,
                               "ultrasonic", sim_objectspecialproperty_detectable_infrared, "infrared",
                               sim_objectspecialproperty_detectable_laser, "laser",
                               sim_objectspecialproperty_detectable_inductive, "inductive",
                               sim_objectspecialproperty_detectable_capacitive, "capacitive");

            ar.xmlAddNode_float("allowedNormalAngle", allowedNormal * 180.0 / piValue);

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("showVolumeWhenNotDetecting", _showVolume);
            ar.xmlAddNode_bool("closestObjectMode", closestObjectMode);
            ar.xmlAddNode_bool("normalCheck", normalCheck);
            ar.xmlAddNode_bool("frontFaceDetection", frontFaceDetection);
            ar.xmlAddNode_bool("backFaceDetection", backFaceDetection);
            ar.xmlAddNode_bool("explicitHandling", explicitHandling);
            ar.xmlPopNode();

            ar.xmlPushNewNode("randomizedDetection");
            ar.xmlAddNode_bool("enabled", _randomizedDetection);
            ar.xmlAddNode_int("sampleCount", _randomizedDetectionSampleCount);
            ar.xmlAddNode_int("countForTrigger", _randomizedDetectionCountForDetection);
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
                ar.xmlGetNode_int("detectableEntity", _sensableObject);
            else
            {
                ar.xmlGetNode_string("detectableObjectAlias", _sensableObjectLoadAlias, exhaustiveXml);
                ar.xmlGetNode_string("detectableEntity", _sensableObjectLoadName_old, exhaustiveXml);
            }

            ar.xmlGetNode_enum("detectionType", _sensableType, exhaustiveXml, "ultrasonic",
                               sim_objectspecialproperty_detectable_ultrasonic, "infrared",
                               sim_objectspecialproperty_detectable_infrared, "laser",
                               sim_objectspecialproperty_detectable_laser, "inductive",
                               sim_objectspecialproperty_detectable_inductive, "capacitive",
                               sim_objectspecialproperty_detectable_capacitive);

            if (ar.xmlGetNode_float("allowedNormalAngle", allowedNormal, exhaustiveXml))
                allowedNormal *= piValue / 180.0;

            if (ar.xmlPushChildNode("switches", exhaustiveXml))
            {
                ar.xmlGetNode_bool("showVolumeWhenNotDetecting", _showVolume, exhaustiveXml);
                ar.xmlGetNode_bool("closestObjectMode", closestObjectMode, exhaustiveXml);
                ar.xmlGetNode_bool("normalCheck", normalCheck, exhaustiveXml);
                ar.xmlGetNode_bool("frontFaceDetection", frontFaceDetection, exhaustiveXml);
                ar.xmlGetNode_bool("backFaceDetection", backFaceDetection, exhaustiveXml);
                ar.xmlGetNode_bool("explicitHandling", explicitHandling, exhaustiveXml);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("randomizedDetection", exhaustiveXml))
            {
                ar.xmlGetNode_bool("enabled", _randomizedDetection, exhaustiveXml);
                ar.xmlGetNode_int("sampleCount", _randomizedDetectionSampleCount, exhaustiveXml);
                ar.xmlGetNode_int("countForTrigger", _randomizedDetectionCountForDetection, exhaustiveXml);
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

bool CProxSensor::getSensingVolumeBoundingBox(C3Vector &minV, C3Vector &maxV) const
{
    return (convexVolume->getVolumeBoundingBox(minV, maxV));
}

void CProxSensor::getSensingVolumeOBB(C7Vector &tr, C3Vector &halfSizes)
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
        volumeColor.setEventParams(_objectHandle);
        detectionRayColor.setEventParams(_objectHandle);
    }
    else
    {
        volumeColor.setEventParams(-1);
        detectionRayColor.setEventParams(-1);
    }
}

void CProxSensor::calculateFreshRandomizedRays()
{
    _randomizedVectors.clear();
    _randomizedVectorDetectionStates.clear();
    // Build the random rays (only direction)
    double angle = convexVolume->getAngle();
    for (int i = 0; i < _randomizedDetectionSampleCount; i++)
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

const std::vector<C3Vector> *CProxSensor::getPointerToRandomizedRays() const
{
    return (&_randomizedVectors);
}

std::vector<double> *CProxSensor::getPointerToRandomizedRayDetectionStates()
{
    return (&_randomizedVectorDetectionStates);
}

void CProxSensor::resetSensor(bool exceptExplicitHandling)
{
    if ((!exceptExplicitHandling) || (!getExplicitHandling()))
    {
        _detectedPointValid = false;
        _sensorResultValid = false;
        _calcTimeInMs = 0;
    }
}

bool CProxSensor::handleSensor(bool exceptExplicitHandling, int &detectedObjectHandle, C3Vector &detectedNormalVector)
{
    if (exceptExplicitHandling && getExplicitHandling())
        return (false); // We don't want to handle those
    _sensorResultValid = false;
    _detectedPointValid = false;
    _calcTimeInMs = 0;
    if (!App::currentWorld->mainSettings->proximitySensorsEnabled)
        return (false);
    if (!App::worldContainer->pluginContainer->isGeomPluginAvailable())
        return (false);

    _sensorResultValid = true;

    int stTime = (int)VDateTime::getTimeInMs();

    double treshhold = DBL_MAX;
    double minThreshold = -1.0;
    if (convexVolume->getSmallestDistanceEnabled())
        minThreshold = convexVolume->getSmallestDistanceAllowed();

    _randomizedVectors.clear();
    _randomizedVectorDetectionStates.clear();
    _detectedPointValid = CProxSensorRoutine::detectEntity(
        _objectHandle, _sensableObject, closestObjectMode, normalCheck, allowedNormal, _detectedPoint, treshhold,
        frontFaceDetection, backFaceDetection, detectedObjectHandle, minThreshold, detectedNormalVector, false);
    _detectedObjectHandle = detectedObjectHandle;
    _detectedNormalVector = detectedNormalVector;
    _calcTimeInMs = VDateTime::getTimeDiffInMs(stTime);
    if (_sensorResultValid && _detectedPointValid && VThread::isSimThread())
    {
        std::vector<CScriptObject*> scripts;
        getAttachedScripts(scripts, -1, true);
        getAttachedScripts(scripts, -1, false);

        if (scripts.size() > 0)
        {
            CInterfaceStack *inStack = App::worldContainer->interfaceStackContainer->createStack();
            inStack->pushTableOntoStack();

            inStack->insertKeyInt32IntoStackTable("handle", getObjectHandle());
            inStack->insertKeyInt32IntoStackTable("detectedObjectHandle", _detectedObjectHandle);
            inStack->insertKeyDoubleArrayIntoStackTable("detectedPoint", _detectedPoint.data, 3);
            inStack->insertKeyDoubleArrayIntoStackTable("normalVector", _detectedNormalVector.data, 3);

            for (size_t i = 0; i < scripts.size(); i++)
            {
                CScriptObject* script = scripts[i];
                if (script->hasSystemFunctionOrHook(sim_syscb_trigger))
                {
                    bool hasTriggerWord = false;
                    CInterfaceStack *outStack = App::worldContainer->interfaceStackContainer->createStack();
                    script->systemCallScript(sim_syscb_trigger, inStack, outStack);
                    if (outStack->getStackSize() >= 1)
                    {
                        outStack->moveStackItemToTop(0);
                        bool trig = false;
                        if (outStack->getStackMapBoolValue("trigger", trig))
                        {
                            hasTriggerWord = true;
                            _detectedPointValid = trig;
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
    return (_detectedPointValid);
}

int CProxSensor::readSensor(C3Vector &detectPt, int &detectedObjectHandle, C3Vector &detectedNormalVector)
{
    if (_sensorResultValid)
    {
        if (_detectedPointValid)
        {
            detectPt = _detectedPoint;
            detectedObjectHandle = _detectedObjectHandle;
            detectedNormalVector = _detectedNormalVector;
            return (1);
        }
        return (0);
    }
    return (-1);
}

double CProxSensor::getCalculationTime() const
{
    return (double(_calcTimeInMs) * 0.001);
}

bool CProxSensor::getFrontFaceDetection() const
{
    return (frontFaceDetection);
}
bool CProxSensor::getBackFaceDetection() const
{
    return (backFaceDetection);
}
void CProxSensor::setFrontFaceDetection(bool faceOn)
{
    frontFaceDetection = faceOn;
    if (!faceOn)
        backFaceDetection = true;
}
void CProxSensor::setBackFaceDetection(bool faceOn)
{
    backFaceDetection = faceOn;
    if (!faceOn)
        frontFaceDetection = true;
}

void CProxSensor::setAllowedNormal(double al)
{
    tt::limitValue(0.1 * degToRad, 90.0 * degToRad, al);
    allowedNormal = al;
}
double CProxSensor::getAllowedNormal() const
{
    return (allowedNormal);
}
void CProxSensor::setNormalCheck(bool check)
{
    normalCheck = check;
}
bool CProxSensor::getNormalCheck() const
{
    return (normalCheck);
}
void CProxSensor::setClosestObjectMode(bool closestObjMode)
{
    closestObjectMode = closestObjMode;
    _detectedPointValid = false;
    _calcTimeInMs = 0;
}
bool CProxSensor::getClosestObjectMode()
{
    return (closestObjectMode);
}

void CProxSensor::setProxSensorSize(double newSize)
{
    tt::limitValue(0.0001, 10.0, newSize);
    bool diff = (_proxSensorSize != newSize);
    if (diff)
    {
        _proxSensorSize = newSize;
        computeBoundingBox();
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propProximitySensor_size.name;
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDouble(cmd, _proxSensorSize);
            App::worldContainer->pushEvent();
        }
    }
}

double CProxSensor::getProxSensorSize()
{
    return (_proxSensorSize);
}

bool CProxSensor::getSensedData(C3Vector &pt)
{
    if (!_detectedPointValid)
        return (false);
    pt = _detectedPoint;
    return (true);
}

void CProxSensor::scaleObject(double scalingFactor)
{
    convexVolume->scaleVolume(scalingFactor);
    setProxSensorSize(_proxSensorSize * scalingFactor);

    CSceneObject::scaleObject(scalingFactor);
}

C3Vector CProxSensor::getDetectedPoint() const
{
    return (_detectedPoint);
}

bool CProxSensor::getIsDetectedPointValid() const
{
    return (_detectedPointValid);
}

std::string CProxSensor::getSensableObjectLoadAlias() const
{
    return (_sensableObjectLoadAlias);
}

std::string CProxSensor::getSensableObjectLoadName_old() const
{
    return (_sensableObjectLoadName_old);
}

CColorObject *CProxSensor::getColor(int index)
{
    if (index == 0)
        return (&volumeColor);
    if (index == 1)
        return (&detectionRayColor);
    return (nullptr);
}

int CProxSensor::getSensorType() const
{
    return (sensorType);
}

#ifdef SIM_WITH_GUI
void CProxSensor::display(CViewableBase *renderingObject, int displayAttrib)
{
    displayProximitySensor(this, renderingObject, displayAttrib);
}
#endif

int CProxSensor::setFloatProperty(const char* ppName, double pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "proximitySensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::setFloatProperty(pName, pState);
    if (retVal == -1)
        retVal = volumeColor.setFloatProperty(pName, pState);
    if (retVal == -1)
        retVal = detectionRayColor.setFloatProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propProximitySensor_size.name)
        {
            setProxSensorSize(pState);
            retVal = 1;
        }
    }

    return retVal;
}

int CProxSensor::getFloatProperty(const char* ppName, double& pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "proximitySensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getFloatProperty(pName, pState);
    if (retVal == -1)
        retVal = volumeColor.getFloatProperty(pName, pState);
    if (retVal == -1)
        retVal = detectionRayColor.getFloatProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propProximitySensor_size.name)
        {
            pState = _proxSensorSize;
            retVal = 1;
        }
    }

    return retVal;
}

int CProxSensor::setColorProperty(const char* ppName, const float* pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "proximitySensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::setColorProperty(pName, pState);
    if (retVal == -1)
        retVal = volumeColor.setColorProperty(pName, pState);
    if (retVal == -1)
        retVal = detectionRayColor.setColorProperty(pName, pState);
    if (retVal != -1)
    {

    }
    return retVal;
}

int CProxSensor::getColorProperty(const char* ppName, float* pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "proximitySensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getColorProperty(pName, pState);
    if (retVal == -1)
        retVal = volumeColor.getColorProperty(pName, pState);
    if (retVal == -1)
        retVal = detectionRayColor.getColorProperty(pName, pState);
    if (retVal != -1)
    {

    }
    return retVal;
}

int CProxSensor::getPropertyName(int& index, std::string& pName, std::string& appartenance)
{
    int retVal = CSceneObject::getPropertyName(index, pName, appartenance);
    if (retVal == -1)
    {
        appartenance += ".proximitySensor";
        retVal = volumeColor.getPropertyName(index, pName);
    }
    if (retVal == -1)
        retVal = detectionRayColor.getPropertyName(index, pName);
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_proximitySensor.size(); i++)
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
    return retVal;
}

int CProxSensor::getPropertyName_static(int& index, std::string& pName, std::string& appartenance)
{
    int retVal = CSceneObject::getPropertyName_bstatic(index, pName, appartenance);
    if (retVal == -1)
    {
        appartenance += ".proximitySensor";
        retVal = CColorObject::getPropertyName_static(index, pName, 1 + 4 + 8, "");
    }
    if (retVal == -1)
        retVal = CColorObject::getPropertyName_static(index, pName, 1 + 4 + 8, "_ray");
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_proximitySensor.size(); i++)
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
    return retVal;
}

int CProxSensor::getPropertyInfo(const char* ppName, int& info, int& size)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "proximitySensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getPropertyInfo(pName, info, size);
    if (retVal == -1)
        retVal = volumeColor.getPropertyInfo(pName, info, size);
    if (retVal == -1)
        retVal = detectionRayColor.getPropertyInfo(pName, info, size);
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_proximitySensor.size(); i++)
        {
            if (strcmp(allProps_proximitySensor[i].name, pName) == 0)
            {
                retVal = allProps_proximitySensor[i].type;
                info = allProps_proximitySensor[i].flags;
                size = 0;
                break;
            }
        }
    }
    return retVal;
}

int CProxSensor::getPropertyInfo_static(const char* ppName, int& info, int& size)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "proximitySensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getPropertyInfo_bstatic(pName, info, size);
    if (retVal == -1)
        retVal = CColorObject::getPropertyInfo_static(pName, info, size, 1 + 4 + 8, "");
    if (retVal == -1)
        retVal = CColorObject::getPropertyInfo_static(pName, info, size, 1 + 4 + 8, "_ray");
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_proximitySensor.size(); i++)
        {
            if (strcmp(allProps_proximitySensor[i].name, pName) == 0)
            {
                retVal = allProps_proximitySensor[i].type;
                info = allProps_proximitySensor[i].flags;
                size = 0;
                break;
            }
        }
    }
    return retVal;
}

