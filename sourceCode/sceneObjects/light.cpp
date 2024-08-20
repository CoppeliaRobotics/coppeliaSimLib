#include <light.h>
#include <simInternal.h>
#include <tt.h>
#include <simStrings.h>
#include <utils.h>
#include <app.h>
#ifdef SIM_WITH_GUI
#include <lightRendering.h>
#include <guiApp.h>
#endif

int CLight::_maximumOpenGlLights = 8;

CLight::CLight()
{
    _lightType = sim_light_omnidirectional;
    _commonInit();
}

CLight::CLight(int theType)
{
    _lightType = theType;
    _commonInit();
}

std::string CLight::getObjectTypeInfo() const
{
    return ("light");
}
std::string CLight::getObjectTypeInfoExtended() const
{
    if (_lightType == sim_light_omnidirectional)
        return ("light (omnidirectional)");
    if (_lightType == sim_light_spot)
        return ("light (spot light)");
    return ("light (directional)");
}
bool CLight::isPotentiallyCollidable() const
{
    return (false);
}
bool CLight::isPotentiallyMeasurable() const
{
    return (false);
}
bool CLight::isPotentiallyDetectable() const
{
    return (false);
}
bool CLight::isPotentiallyRenderable() const
{
    return (false);
}

void CLight::_commonInit()
{
    _objectType = sim_sceneobject_light;
    _lightSize = 0.10;
    _spotExponent = 5;
    _spotCutoffAngle = 90.0 * degToRad;
    _visibilityLayer = CAMERA_LIGHT_LAYER;
    _localObjectSpecialProperty = 0;
    _setDefaultColors();
    constantAttenuation = 0.25;
    linearAttenuation = 0.0;
    quadraticAttenuation = 0.1;

    lightActive = true;
    _lightIsLocal = false;
    if (_extensionString.size() != 0)
        _extensionString += " ";
    if (_lightType == sim_light_omnidirectional)
        _extensionString += "openGL3 {lightProjection {nearPlane {0.1} farPlane {10} orthoSize {8} bias {0.001} "
                            "normalBias {0.012} shadowTextureSize {2048}}} povray {shadow {true} fadeXDist {0.00}}";
    if (_lightType == sim_light_spot)
        _extensionString += "openGL3 {lightProjection {nearPlane {0.1} farPlane {10} orthoSize {8} bias {0.0} "
                            "normalBias {0.00008} shadowTextureSize {2048}}} povray {shadow {true} fadeXDist {0.00}}";
    if (_lightType == sim_light_directional)
        _extensionString += "openGL3 {lightProjection {nearPlane {0.1} farPlane {10} orthoSize {8} bias {0.001} "
                            "normalBias {0.005} shadowTextureSize {2048}}} povray {shadow {true} fadeXDist {0.00}}";

    _objectMovementPreferredAxes = 0x013;

    _objectAlias = getObjectTypeInfo();
    _objectName_old = getObjectTypeInfo();
    _objectAltName_old = tt::getObjectAltNameFromObjectName(_objectName_old.c_str());
    computeBoundingBox();
}

void CLight::computeBoundingBox()
{
    C3Vector minV, maxV;
    if (_lightType == sim_light_omnidirectional)
    {
        minV(0) = -0.5 * _lightSize;
        maxV(0) = 0.5 * _lightSize;
        minV(1) = -0.5 * _lightSize;
        maxV(1) = 0.5 * _lightSize;
        minV(2) = -0.5 * _lightSize;
        maxV(2) = 0.5 * _lightSize;
        _setBB(C7Vector::identityTransformation, C3Vector(1.0, 1.0, 1.0) * _lightSize * 0.5);
    }
    if (_lightType == sim_light_spot)
    {
        minV(0) = -0.8 * _lightSize;
        maxV(0) = 0.8 * _lightSize;
        minV(1) = -0.8 * _lightSize;
        maxV(1) = 0.8 * _lightSize;
        minV(2) = -1.5 * _lightSize;
        maxV(2) = 0.5 * _lightSize;
        C7Vector fr;
        fr.Q.setIdentity();
        fr.X = C3Vector(0.0, 0.0, -0.5) * _lightSize;
        _setBB(fr, C3Vector(1.6, 1.6, 2.0) * _lightSize * 0.5);
    }
    if (_lightType == sim_light_directional)
    {
        minV(0) = -_lightSize * 0.5;
        maxV(0) = _lightSize * 0.5;
        minV(1) = -_lightSize * 0.5;
        maxV(1) = _lightSize * 0.5;
        minV(2) = -0.5 * _lightSize;
        maxV(2) = 0.5 * _lightSize;
        _setBB(C7Vector::identityTransformation, C3Vector(1.0, 1.0, 1.0) * _lightSize * 0.5);
    }
}

void CLight::setIsInScene(bool s)
{
    CSceneObject::setIsInScene(s);
    if (s)
    {
        objectColor.setEventParams(_objectHandle);
        lightColor.setEventParams(_objectHandle);
    }
    else
    {
        objectColor.setEventParams(-1);
        lightColor.setEventParams(-1);
    }
}

void CLight::_setDefaultColors()
{
    if (_lightType == sim_light_omnidirectional)
    {
        objectColor.setDefaultValues();
        lightColor.setDefaultValues();
        lightColor.setColor(0.5, 0.5, 0.5, sim_colorcomponent_diffuse);
        lightColor.setColor(0.5, 0.5, 0.5, sim_colorcomponent_specular);
    }
    if (_lightType == sim_light_spot)
    {
        objectColor.setDefaultValues();
        objectColor.setColor(1.0f, 0.375f, 0.25f, sim_colorcomponent_ambient_diffuse);
        lightColor.setDefaultValues();
        lightColor.setColor(0.5f, 0.5f, 0.5f, sim_colorcomponent_diffuse);
        lightColor.setColor(0.5f, 0.5f, 0.5f, sim_colorcomponent_specular);
    }
    if (_lightType == sim_light_directional)
    {
        objectColor.setDefaultValues();
        objectColor.setColor(0.45f, 0.45f, 0.75f, sim_colorcomponent_ambient_diffuse);
        lightColor.setDefaultValues();
        lightColor.setColor(0.5f, 0.5f, 0.5f, sim_colorcomponent_diffuse);
        lightColor.setColor(0.5f, 0.5f, 0.5f, sim_colorcomponent_specular);
    }
    lightColor.setEventParams(-1, -1, "_light");
}

CLight::~CLight()
{
}

void CLight::scaleObject(double scalingFactor)
{
    setLightSize(_lightSize * scalingFactor);
    linearAttenuation /= scalingFactor;
    quadraticAttenuation /= scalingFactor * scalingFactor;
    std::string val;
    if (tt::getValueOfKey("fadeXDist@povray", _extensionString.c_str(), val))
    {
        double f;
        if (tt::getValidFloat(val.c_str(), f))
        {
            f *= scalingFactor;
            tt::insertKeyAndValue("fadeXDist@povray", utils::getSizeString(false, f).c_str(), _extensionString);
        }
    }

    CSceneObject::scaleObject(scalingFactor);
}

void CLight::setLightSize(double size)
{
    tt::limitValue(0.001, 100.0, size);
    if (_lightSize != size)
    {
        _lightSize = size;
        computeBoundingBox();
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propLight_size.name;
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDouble(cmd, _lightSize);
            App::worldContainer->pushEvent();
        }
    }
}

double CLight::getLightSize() const
{
    return (_lightSize);
}

double CLight::getAttenuationFactor(int type) const
{
    double retVal = 0.0;
    if (type == CONSTANT_ATTENUATION)
        retVal = constantAttenuation;
    if (type == LINEAR_ATTENUATION)
        retVal = linearAttenuation;
    if (type == QUADRATIC_ATTENUATION)
        retVal = quadraticAttenuation;
    return (retVal);
}

void CLight::setAttenuationFactor(int type, double value)
{
    if (type == CONSTANT_ATTENUATION)
        constantAttenuation = value;
    if (type == LINEAR_ATTENUATION)
        linearAttenuation = value;
    if (type == QUADRATIC_ATTENUATION)
        quadraticAttenuation = value;
}

void CLight::setLightActive(bool active)
{
#ifdef SIM_WITH_GUI
    if (active != lightActive)
        GuiApp::setRefreshHierarchyViewFlag();
#endif
    lightActive = active;
}

bool CLight::getLightActive() const
{
    return (lightActive);
}

void CLight::setSpotExponent(int e)
{
    _spotExponent = tt::getLimitedInt(0, 128, e);
}

int CLight::getSpotExponent() const
{
    return (_spotExponent);
}

void CLight::setSpotCutoffAngle(double co)
{
    _spotCutoffAngle = tt::getLimitedFloat(5.0 * degToRad, 90.0 * degToRad, co);
}

double CLight::getSpotCutoffAngle() const
{
    return (_spotCutoffAngle);
}

int CLight::getLightType() const
{
    return (_lightType);
}

void CLight::removeSceneDependencies()
{
    CSceneObject::removeSceneDependencies();
}

void CLight::addSpecializedObjectEventData(CCbor *ev) const
{
#if SIM_EVENT_PROTOCOL_VERSION == 2
    ev->openKeyMap(getObjectTypeInfo().c_str());
    ev->openKeyArray("colors");
    float c[9];
    objectColor.getColor(c, sim_colorcomponent_ambient_diffuse);
    objectColor.getColor(c + 3, sim_colorcomponent_specular);
    objectColor.getColor(c + 6, sim_colorcomponent_emission);
    ev->appendFloatArray(c, 9);
    lightColor.getColor(c, sim_colorcomponent_diffuse);
    lightColor.getColor(c + 3, sim_colorcomponent_specular);
    lightColor.getColor(c + 6, sim_colorcomponent_emission);
    ev->appendFloatArray(c, 9);
    ev->closeArrayOrMap(); // colors
#else
    objectColor.addGenesisEventData(ev);
    lightColor.addGenesisEventData(ev);
#endif
    ev->appendKeyDouble(propLight_size.name, _lightSize);
    // todo
#if SIM_EVENT_PROTOCOL_VERSION == 2
    ev->closeArrayOrMap(); // light
#endif
}

CSceneObject *CLight::copyYourself()
{
    CLight *newLight = (CLight *)CSceneObject::copyYourself();

    // Various
    newLight->_lightSize = _lightSize;
    newLight->_lightType = _lightType;
    newLight->_spotExponent = _spotExponent;
    newLight->_spotCutoffAngle = _spotCutoffAngle;
    newLight->lightActive = lightActive;
    newLight->constantAttenuation = constantAttenuation;
    newLight->linearAttenuation = linearAttenuation;
    newLight->quadraticAttenuation = quadraticAttenuation;
    newLight->_lightIsLocal = _lightIsLocal;

    // Colors:
    objectColor.copyYourselfInto(&newLight->objectColor);
    lightColor.copyYourselfInto(&newLight->lightColor);

    return (newLight);
}

void CLight::announceObjectWillBeErased(const CSceneObject *object, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceObjectWillBeErased(object, copyBuffer);
}

void CLight::announceCollectionWillBeErased(int groupID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollectionWillBeErased(groupID, copyBuffer);
}
void CLight::announceCollisionWillBeErased(int collisionID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollisionWillBeErased(collisionID, copyBuffer);
}
void CLight::announceDistanceWillBeErased(int distanceID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceDistanceWillBeErased(distanceID, copyBuffer);
}
void CLight::announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceIkObjectWillBeErased(ikGroupID, copyBuffer);
}

void CLight::performObjectLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performObjectLoadingMapping(map, loadingAmodel);
}
void CLight::performCollectionLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performCollectionLoadingMapping(map, loadingAmodel);
}
void CLight::performCollisionLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performCollisionLoadingMapping(map, loadingAmodel);
}
void CLight::performDistanceLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performDistanceLoadingMapping(map, loadingAmodel);
}
void CLight::performIkLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performIkLoadingMapping(map, loadingAmodel);
}

void CLight::performTextureObjectLoadingMapping(const std::map<int, int> *map)
{
    CSceneObject::performTextureObjectLoadingMapping(map);
}

void CLight::performDynMaterialObjectLoadingMapping(const std::map<int, int> *map)
{
    CSceneObject::performDynMaterialObjectLoadingMapping(map);
}

void CLight::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
    _initialLightActive = lightActive;
}

void CLight::simulationAboutToStart()
{
    initializeInitialValues(false);
    CSceneObject::simulationAboutToStart();
}

void CLight::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it
  // ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized)
    {
        if (App::currentWorld->simulation->getResetSceneAtSimulationEnd() &&
            ((getCumulativeModelProperty() & sim_modelproperty_not_reset) == 0))
        {
            lightActive = _initialLightActive;
        }
    }
    CSceneObject::simulationEnded();
}

void CLight::setLightIsLocal(bool l)
{
    _lightIsLocal = l;
}

bool CLight::getLightIsLocal() const
{
    return (_lightIsLocal);
}

void CLight::setMaxAvailableOglLights(int c)
{
    _maximumOpenGlLights = c;
}

int CLight::getMaxAvailableOglLights()
{
    return (_maximumOpenGlLights);
}

CColorObject *CLight::getColor(bool getLightColor)
{
    if (getLightColor)
        return (&lightColor);
    return (&objectColor);
}

void CLight::serialize(CSer &ar)
{
    CSceneObject::serialize(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing

            ar.storeDataName("_p2");
            ar << _lightType;
            ar << _spotExponent;
            ar << _lightSize;
            ar.flush();

            ar.storeDataName("_p3");
            ar << _spotCutoffAngle;
            ar.flush();

            ar.storeDataName("_af");
            ar << constantAttenuation << linearAttenuation << quadraticAttenuation;
            ar.flush();

            ar.storeDataName("Cas");
            unsigned char nothing = 0;
            SIM_SET_CLEAR_BIT(nothing, 0, lightActive);
            SIM_SET_CLEAR_BIT(nothing, 1, _lightIsLocal);
            // RESERVED SIM_SET_CLEAR_BIT(nothing,2,!povShadow);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Cl1");
            ar.setCountingMode();
            objectColor.serialize(ar, 0);
            if (ar.setWritingMode())
                objectColor.serialize(ar, 0);

            ar.storeDataName("Cl2");
            ar.setCountingMode();
            lightColor.serialize(ar, 3);
            if (ar.setWritingMode())
                lightColor.serialize(ar, 3);

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        { // Loading
            int byteQuantity;
            std::string theName = "";
            bool povShadow_backwardCompatibility_3_2_2016 = true;
            double povFadeXDist_backwardCompatibility_3_2_2016 = -1.0;
            while (theName.compare(SER_END_OF_OBJECT) != 0)
            {
                theName = ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT) != 0)
                {
                    bool noHit = true;
                    if (theName.compare("Cp2") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _lightType;
                        ar >> _spotExponent;
                        float bla;
                        ar >> bla;
                        _lightSize = (double)bla;
                    }

                    if (theName.compare("_p2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _lightType;
                        ar >> _spotExponent;
                        ar >> _lightSize;
                    }

                    if (theName.compare("Cp3") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla;
                        ar >> bla;
                        _spotCutoffAngle = (double)bla;
                    }

                    if (theName.compare("_p3") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _spotCutoffAngle;
                    }

                    if (theName.compare("Caf") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla, bli, blo;
                        ar >> bla >> bli >> blo;
                        constantAttenuation = (double)bla;
                        linearAttenuation = (double)bli;
                        quadraticAttenuation = (double)blo;
                    }

                    if (theName.compare("_af") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> constantAttenuation >> linearAttenuation >> quadraticAttenuation;
                    }

                    if (theName.compare("Pfd") == 0)
                    { // keep for backward compatibility (3/2/2016)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla;
                        ar >> bla;
                        povFadeXDist_backwardCompatibility_3_2_2016 = (double)bla;
                    }
                    if (theName == "Cas")
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        lightActive = SIM_IS_BIT_SET(nothing, 0);
                        _lightIsLocal = SIM_IS_BIT_SET(nothing, 1);
                        povShadow_backwardCompatibility_3_2_2016 = !SIM_IS_BIT_SET(nothing, 2);
                    }
                    if (theName.compare("Cl1") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        objectColor.serialize(ar, 0);
                    }
                    if (theName.compare("Cl2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        lightColor.serialize(ar, 3);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            if (ar.getSerializationVersionThatWroteThisFile() < 17)
            { // on 29/08/2013 we corrected all default lights. So we need to correct for that change:
                utils::scaleColorUp_(objectColor.getColorsPtr());
                utils::scaleLightDown_(lightColor.getColorsPtr());
            }

            if (povFadeXDist_backwardCompatibility_3_2_2016 >= 0.0)
            { // keep for backward compatibility (3/2/2016)
                _extensionString = "povray {shadow {";
                if (povShadow_backwardCompatibility_3_2_2016)
                    _extensionString += "true} fadeXDist {";
                else
                    _extensionString += "false} fadeXDist {";
                _extensionString += utils::getSizeString(false, povFadeXDist_backwardCompatibility_3_2_2016);
                _extensionString += "}}";
            }

            if (ar.getCoppeliaSimVersionThatWroteThisFile() <= 30601)
            {
                if (_extensionString.find("openGL3") == std::string::npos)
                {
                    if (_extensionString.size() != 0)
                        _extensionString += " ";
                    if (_lightType == sim_light_omnidirectional)
                        _extensionString += "openGL3 {lightProjection {nearPlane {0.1} farPlane {10} orthoSize {8} "
                                            "bias {0.001} normalBias {0.012} shadowTextureSize {2048}}}";
                    if (_lightType == sim_light_spot)
                        _extensionString += "openGL3 {lightProjection {nearPlane {0.1} farPlane {10} orthoSize {8} "
                                            "bias {0.0} normalBias {0.00008} shadowTextureSize {2048}}}";
                    if (_lightType == sim_light_directional)
                        _extensionString += "openGL3 {lightProjection {nearPlane {0.1} farPlane {10} orthoSize {8} "
                                            "bias {0.001} normalBias {0.005} shadowTextureSize {2048}}}";
                }
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
            ar.xmlAddNode_comment(" 'type' tag: can be 'omnidirectional', 'spotlight' or 'directional' ",
                                  exhaustiveXml);
            ar.xmlAddNode_enum("type", _lightType, sim_light_omnidirectional, "omnidirectional",
                               sim_light_spot, "spotlight", sim_light_directional, "directional");

            ar.xmlAddNode_float("size", _lightSize);

            ar.xmlAddNode_int("spotExponent", _spotExponent);

            ar.xmlAddNode_float("cutoffAngle", _spotCutoffAngle * 180.0 / piValue);

            ar.xmlPushNewNode("attenuationFactors");
            ar.xmlAddNode_float("constant", constantAttenuation);
            ar.xmlAddNode_float("linear", linearAttenuation);
            ar.xmlAddNode_float("quadratic", quadraticAttenuation);
            ar.xmlPopNode();

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("lightIsActive", lightActive);
            if (exhaustiveXml)
                ar.xmlAddNode_bool("lightIsLocal", _lightIsLocal);
            ar.xmlPopNode();

            ar.xmlPushNewNode("color");
            if (exhaustiveXml)
            {
                ar.xmlPushNewNode("object");
                objectColor.serialize(ar, 0);
                ar.xmlPopNode();
                ar.xmlPushNewNode("light");
                lightColor.serialize(ar, 0);
                ar.xmlPopNode();
            }
            else
            {
                int rgb[3];
                for (size_t l = 0; l < 3; l++)
                    rgb[l] = int(objectColor.getColorsPtr()[l] * 255.1);
                ar.xmlAddNode_ints("object", rgb, 3);
                ar.xmlPushNewNode("light");
                for (size_t l = 0; l < 3; l++)
                    rgb[l] = int(lightColor.getColorsPtr()[3 + l] * 255.1);
                ar.xmlAddNode_ints("ambientDiffuse", rgb, 3);
                for (size_t l = 0; l < 3; l++)
                    rgb[l] = int(lightColor.getColorsPtr()[6 + l] * 255.1);
                ar.xmlAddNode_ints("specular", rgb, 3);
                ar.xmlPopNode();
            }
            ar.xmlPopNode();
        }
        else
        {
            ar.xmlGetNode_enum("type", _lightType, exhaustiveXml, "omnidirectional", sim_light_omnidirectional,
                               "spotlight", sim_light_spot, "directional", sim_light_directional);

            if (ar.xmlGetNode_float("size", _lightSize, exhaustiveXml))
                setLightSize(_lightSize);

            if (ar.xmlGetNode_int("spotExponent", _spotExponent, exhaustiveXml))
                setSpotExponent(_spotExponent);

            if (ar.xmlGetNode_float("cutoffAngle", _spotCutoffAngle, exhaustiveXml))
                setSpotCutoffAngle(_spotCutoffAngle * piValue / 180.0);

            if (ar.xmlPushChildNode("attenuationFactors", exhaustiveXml))
            {
                if (ar.xmlGetNode_float("constant", constantAttenuation, exhaustiveXml))
                    setAttenuationFactor(CONSTANT_ATTENUATION, constantAttenuation);
                if (ar.xmlGetNode_float("linear", linearAttenuation, exhaustiveXml))
                    setAttenuationFactor(LINEAR_ATTENUATION, linearAttenuation);
                if (ar.xmlGetNode_float("quadratic", quadraticAttenuation, exhaustiveXml))
                    setAttenuationFactor(QUADRATIC_ATTENUATION, quadraticAttenuation);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("switches", exhaustiveXml))
            {
                ar.xmlGetNode_bool("lightIsActive", lightActive, exhaustiveXml);
                if (exhaustiveXml)
                    ar.xmlGetNode_bool("lightIsLocal", _lightIsLocal, exhaustiveXml);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("color", exhaustiveXml))
            {
                if (exhaustiveXml)
                {
                    if (ar.xmlPushChildNode("object"))
                    {
                        objectColor.serialize(ar, 0);
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("light"))
                    {
                        lightColor.serialize(ar, 0);
                        ar.xmlPopNode();
                    }
                }
                else
                {
                    int rgb[3];
                    if (ar.xmlGetNode_ints("object", rgb, 3, exhaustiveXml))
                        objectColor.setColor(float(rgb[0]) / 255.0, float(rgb[1]) / 255.0, float(rgb[2]) / 255.0,
                                             sim_colorcomponent_ambient_diffuse);
                    if (ar.xmlPushChildNode("light", exhaustiveXml))
                    {
                        if (ar.xmlGetNode_ints("ambientDiffuse", rgb, 3, exhaustiveXml))
                            lightColor.setColor(float(rgb[0]) / 255.0, float(rgb[1]) / 255.0, float(rgb[2]) / 255.0,
                                                sim_colorcomponent_diffuse);
                        if (ar.xmlGetNode_ints("specular", rgb, 3, exhaustiveXml))
                            lightColor.setColor(float(rgb[0]) / 255.0, float(rgb[1]) / 255.0, float(rgb[2]) / 255.0,
                                                sim_colorcomponent_specular);
                        ar.xmlPopNode();
                    }
                }
                ar.xmlPopNode();
            }
            computeBoundingBox();
        }
    }
}

#ifdef SIM_WITH_GUI
void CLight::display(CViewableBase *renderingObject, int displayAttrib)
{
    displayLight(this, renderingObject, displayAttrib);
}
#endif

int CLight::setFloatProperty(const char* ppName, double pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "light."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::setFloatProperty(pName, pState);
    if (retVal == -1)
        retVal = objectColor.setFloatProperty(pName, pState);
    if (retVal == -1)
        retVal = lightColor.setFloatProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propLight_size.name)
        {
            setLightSize(pState);
            retVal = 1;
        }
    }

    return retVal;
}

int CLight::getFloatProperty(const char* ppName, double& pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "light."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getFloatProperty(pName, pState);
    if (retVal == -1)
        retVal = objectColor.getFloatProperty(pName, pState);
    if (retVal == -1)
        retVal = lightColor.getFloatProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propLight_size.name)
        {
            pState = _lightSize;
            retVal = 1;
        }
    }

    return retVal;
}

int CLight::setColorProperty(const char* ppName, const float* pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "light."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::setColorProperty(pName, pState);
    if (retVal == -1)
        retVal = objectColor.setColorProperty(pName, pState);
    if (retVal == -1)
        retVal = lightColor.setColorProperty(pName, pState);
    if (retVal != -1)
    {

    }
    return retVal;
}

int CLight::getColorProperty(const char* ppName, float* pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "light."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getColorProperty(pName, pState);
    if (retVal == -1)
        retVal = objectColor.getColorProperty(pName, pState);
    if (retVal == -1)
        retVal = lightColor.getColorProperty(pName, pState);
    if (retVal != -1)
    {

    }
    return retVal;
}

int CLight::getPropertyName(int& index, std::string& pName, std::string& appartenance)
{
    int retVal = CSceneObject::getPropertyName(index, pName, appartenance);
    if (retVal == -1)
    {
        appartenance += ".light";
        retVal = objectColor.getPropertyName(index, pName);
    }
    if (retVal == -1)
        retVal = lightColor.getPropertyName(index, pName);
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_light.size(); i++)
        {
            index--;
            if (index == -1)
            {
                pName = allProps_light[i].name;
                retVal = 1;
                break;
            }
        }
    }
    return retVal;
}

int CLight::getPropertyName_static(int& index, std::string& pName, std::string& appartenance)
{
    int retVal = CSceneObject::getPropertyName_bstatic(index, pName, appartenance);
    if (retVal == -1)
    {
        appartenance += ".light";
        retVal = CColorObject::getPropertyName_static(index, pName, 1 + 4 + 8, "");
    }
    if (retVal == -1)
        retVal = CColorObject::getPropertyName_static(index, pName, 1 + 4 + 8, "_light");
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_light.size(); i++)
        {
            index--;
            if (index == -1)
            {
                pName = allProps_light[i].name;
                retVal = 1;
                break;
            }
        }
    }
    return retVal;
}

int CLight::getPropertyInfo(const char* ppName, int& info, int& size)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "light."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getPropertyInfo(pName, info, size);
    if (retVal == -1)
        retVal = objectColor.getPropertyInfo(pName, info, size);
    if (retVal == -1)
        retVal = lightColor.getPropertyInfo(pName, info, size);
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_light.size(); i++)
        {
            if (strcmp(allProps_light[i].name, pName) == 0)
            {
                retVal = allProps_light[i].type;
                info = allProps_light[i].flags;
                size = 0;
                break;
            }
        }
    }
    return retVal;
}

int CLight::getPropertyInfo_static(const char* ppName, int& info, int& size)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "light."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getPropertyInfo_bstatic(pName, info, size);
    if (retVal == -1)
        retVal = CColorObject::getPropertyInfo_static(pName, info, size, 1 + 4 + 8, "");
    if (retVal == -1)
        retVal = CColorObject::getPropertyInfo_static(pName, info, size, 1 + 4 + 8, "_light");
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_light.size(); i++)
        {
            if (strcmp(allProps_light[i].name, pName) == 0)
            {
                retVal = allProps_light[i].type;
                info = allProps_light[i].flags;
                size = 0;
                break;
            }
        }
    }
    return retVal;
}

