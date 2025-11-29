#include <mirror.h>
#include <simInternal.h>
#include <tt.h>
#include <simStrings.h>
#include <utils.h>
#include <app.h>
#ifdef SIM_WITH_GUI
#include <mirrorRendering.h>
#include <guiApp.h>
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
        )" MIRROR_META_METHODS R"(,
        )" SCENEOBJECT_META_METHODS R"(
    }
}
)";

int CMirror::currentMirrorContentBeingRendered = -1;

CMirror::CMirror()
{
    _commonInit();
}

std::string CMirror::getObjectTypeInfo() const
{
    return "mirror";
}
std::string CMirror::getObjectTypeInfoExtended() const
{
    return getObjectTypeInfo();
}
bool CMirror::isPotentiallyCollidable() const
{
    return (false);
}
bool CMirror::isPotentiallyMeasurable() const
{
    return (false);
}
bool CMirror::isPotentiallyDetectable() const
{
    return (false);
}
bool CMirror::isPotentiallyRenderable() const
{
    return (true);
}

void CMirror::_commonInit()
{
    _objectType = sim_sceneobject_mirror;
    _mirrorWidth = 0.5;
    _mirrorHeight = 1.0;
    _mirrorReflectance = 0.75;
    mirrorColor[0] = 0.0;
    mirrorColor[1] = 0.0;
    mirrorColor[2] = 0.0;

    clipPlaneColor.setDefaultValues();
    clipPlaneColor.setColor(0.0f, 0.525f, 0.6f, sim_colorcomponent_ambient_diffuse);
    clipPlaneColor.setTranslucid(true);
    clipPlaneColor.setOpacity(0.4f);

    _active = true;
    _isMirror = true;
    _clippingObjectOrCollection = -1; // clipping all
    _visibilityLayer = CAMERA_LIGHT_LAYER;
    _localObjectSpecialProperty = sim_objectspecialproperty_renderable;

    _objectMovementPreferredAxes = 0x013;

    _objectAlias = getObjectTypeInfo();
    _objectName_old = getObjectTypeInfo();
    _objectAltName_old = tt::getObjectAltNameFromObjectName(_objectName_old.c_str());
    computeBoundingBox();
}

bool CMirror::getContainsTransparentComponent() const
{
    if (_isMirror)
        return (false);
    return (clipPlaneColor.getTranslucid());
}

CColorObject* CMirror::getClipPlaneColor()
{
    return (&clipPlaneColor);
}

void CMirror::computeBoundingBox()
{
    _setBB(C7Vector::identityTransformation, C3Vector(_mirrorWidth, _mirrorHeight, 0.001) * 0.5);
}

void CMirror::setIsInScene(bool s)
{
    CSceneObject::setIsInScene(s);
    if (s)
        clipPlaneColor.setEventParams(true, _objectHandle);
    else
        clipPlaneColor.setEventParams(true, -1);
}

CMirror::~CMirror()
{
}

void CMirror::scaleObject(double scalingFactor)
{
    _mirrorWidth *= scalingFactor;
    _mirrorHeight *= scalingFactor;

    CSceneObject::scaleObject(scalingFactor);
}

void CMirror::setMirrorWidth(double w)
{
    _mirrorWidth = tt::getLimitedFloat(0.001, 100.0, w);
}

double CMirror::getMirrorWidth()
{
    return (_mirrorWidth);
}

void CMirror::setMirrorHeight(double h)
{
    _mirrorHeight = tt::getLimitedFloat(0.001, 100.0, h);
}

double CMirror::getMirrorHeight()
{
    return (_mirrorHeight);
}

void CMirror::setReflectance(double r)
{
    _mirrorReflectance = tt::getLimitedFloat(0.0, 1.0, r);
}

double CMirror::getReflectance()
{
    return (_mirrorReflectance);
}

void CMirror::setActive(bool a)
{
    _active = a;
}

bool CMirror::getActive()
{
    return (_active);
}

void CMirror::setIsMirror(bool m)
{
#ifdef SIM_WITH_GUI
    if (m != _isMirror)
        GuiApp::setRefreshHierarchyViewFlag();
#endif
    _isMirror = m;
}

bool CMirror::getIsMirror()
{
    return (_isMirror);
}

void CMirror::setClippingObjectOrCollection(int co)
{
    _clippingObjectOrCollection = co;
}

int CMirror::getClippingObjectOrCollection()
{
    return (_clippingObjectOrCollection);
}

void CMirror::removeSceneDependencies()
{
    CSceneObject::removeSceneDependencies();
}

void CMirror::addSpecializedObjectEventData(CCbor* ev)
{
#if SIM_EVENT_PROTOCOL_VERSION == 2
    ev->openKeyMap(getObjectTypeInfo().c_str());
    ev->closeArrayOrMap(); // mirror
#else
    clipPlaneColor.addGenesisEventData(ev);
#endif
}

CSceneObject* CMirror::copyYourself()
{
    CMirror* newMirror = (CMirror*)CSceneObject::copyYourself();

    // Various
    newMirror->_mirrorHeight = _mirrorHeight;
    newMirror->_mirrorWidth = _mirrorWidth;
    newMirror->_active = _active;
    newMirror->_isMirror = _isMirror;
    newMirror->_mirrorReflectance = _mirrorReflectance;
    newMirror->_clippingObjectOrCollection = _clippingObjectOrCollection;

    newMirror->mirrorColor[0] = mirrorColor[0];
    newMirror->mirrorColor[1] = mirrorColor[1];
    newMirror->mirrorColor[2] = mirrorColor[2];

    clipPlaneColor.copyYourselfInto(&newMirror->clipPlaneColor);

    newMirror->_initialMirrorActive = _initialMirrorActive;

    return (newMirror);
}

void CMirror::announceObjectWillBeErased(const CSceneObject* object, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    if (_clippingObjectOrCollection == object->getObjectHandle())
    {
        _clippingObjectOrCollection = -1;
        if (!_isMirror)
            _active = false;
    }
    CSceneObject::announceObjectWillBeErased(object, copyBuffer);
}

void CMirror::announceCollectionWillBeErased(int groupID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    if (_clippingObjectOrCollection == groupID)
    {
        _clippingObjectOrCollection = -1;
        if (!_isMirror)
            _active = false;
    }
    CSceneObject::announceCollectionWillBeErased(groupID, copyBuffer);
}
void CMirror::announceCollisionWillBeErased(int collisionID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollisionWillBeErased(collisionID, copyBuffer);
}
void CMirror::announceDistanceWillBeErased(int distanceID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceDistanceWillBeErased(distanceID, copyBuffer);
}
void CMirror::announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceIkObjectWillBeErased(ikGroupID, copyBuffer);
}

void CMirror::performObjectLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performObjectLoadingMapping(map, opType);
    if (_clippingObjectOrCollection <= SIM_IDEND_SCENEOBJECT)
        _clippingObjectOrCollection = CWorld::getLoadingMapping(map, _clippingObjectOrCollection);
}
void CMirror::performCollectionLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performCollectionLoadingMapping(map, opType);
    if (_clippingObjectOrCollection > SIM_IDEND_SCENEOBJECT)
        _clippingObjectOrCollection = CWorld::getLoadingMapping(map, _clippingObjectOrCollection);
}
void CMirror::performCollisionLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performCollisionLoadingMapping(map, opType);
}
void CMirror::performDistanceLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performDistanceLoadingMapping(map, opType);
}
void CMirror::performIkLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performIkLoadingMapping(map, opType);
}

void CMirror::performTextureObjectLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performTextureObjectLoadingMapping(map, opType);
}

void CMirror::performDynMaterialObjectLoadingMapping(const std::map<int, int>* map)
{
    CSceneObject::performDynMaterialObjectLoadingMapping(map);
}

void CMirror::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
    _initialMirrorActive = _active;
}

void CMirror::simulationAboutToStart()
{
    initializeInitialValues(false);
    CSceneObject::simulationAboutToStart();
}

void CMirror::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it
    // ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized)
    {
        if ((getCumulativeModelProperty() & sim_modelproperty_not_reset) == 0)
        {
            _active = _initialMirrorActive;
        }
    }
    CSceneObject::simulationEnded();
}

void CMirror::serialize(CSer& ar)
{
    CSceneObject::serialize(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            ar.storeDataName("_sz");
            ar << _mirrorWidth << _mirrorHeight;
            ar.flush();

            ar.storeDataName("Cas");
            unsigned char nothing = 0;
            SIM_SET_CLEAR_BIT(nothing, 0, _active);
            SIM_SET_CLEAR_BIT(nothing, 1, !_isMirror);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Clp");
            ar << _clippingObjectOrCollection;
            ar.flush();

            ar.storeDataName("Mcr");
            ar << _mirrorReflectance << mirrorColor[0] << mirrorColor[1] << mirrorColor[2];
            ar.flush();

            ar.storeDataName("Cpc");
            ar.setCountingMode();
            clipPlaneColor.serialize(ar, 0);
            if (ar.setWritingMode())
                clipPlaneColor.serialize(ar, 0);

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
                    if (theName.compare("Msz") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla, bli;
                        ar >> bla >> bli;
                        _mirrorWidth = (double)bla;
                        _mirrorHeight = (double)bli;
                    }

                    if (theName.compare("_sz") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _mirrorWidth >> _mirrorHeight;
                    }

                    if (theName == "Cas")
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _active = SIM_IS_BIT_SET(nothing, 0);
                        _isMirror = !SIM_IS_BIT_SET(nothing, 1);
                    }
                    if (theName.compare("Clp") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _clippingObjectOrCollection;
                    }
                    if (theName.compare("Mcr") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _mirrorReflectance >> mirrorColor[0] >> mirrorColor[1] >> mirrorColor[2];
                    }
                    if (theName.compare("Cpc") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        clipPlaneColor.serialize(ar, 0);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            if (ar.getSerializationVersionThatWroteThisFile() < 17)
            { // on 29/08/2013 we corrected all default lights. So we need to correct for that change:
                utils::scaleColorUp_(mirrorColor);
            }
            computeBoundingBox();
        }
    }
    else
    {
        if (ar.isStoring())
        {
            ar.xmlAddNode_2float("sizes", _mirrorWidth, _mirrorHeight);

            ar.xmlAddNode_int("clippingEntity", _clippingObjectOrCollection);

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("active", _active);
            ar.xmlAddNode_bool("isMirror", _isMirror);
            ar.xmlPopNode();

            ar.xmlPushNewNode("mirror");
            ar.xmlAddNode_float("reflectance", _mirrorReflectance);
            ar.xmlAddNode_floats("color", mirrorColor, 3);
            ar.xmlPopNode();

            ar.xmlPushNewNode("color");
            clipPlaneColor.serialize(ar, 0);
            ar.xmlPopNode();
        }
        else
        {
            ar.xmlGetNode_2float("sizes", _mirrorWidth, _mirrorHeight);

            ar.xmlGetNode_int("clippingEntity", _clippingObjectOrCollection);

            if (ar.xmlPushChildNode("switches"))
            {
                ar.xmlGetNode_bool("active", _active);
                ar.xmlGetNode_bool("isMirror", _isMirror);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("mirror"))
            {
                ar.xmlGetNode_float("reflectance", _mirrorReflectance);
                ar.xmlGetNode_floats("color", mirrorColor, 3);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("color"))
            {
                clipPlaneColor.serialize(ar, 0);
                ar.xmlPopNode();
            }
            computeBoundingBox();
        }
    }
}

#ifdef SIM_WITH_GUI
void CMirror::display(CViewableBase* renderingObject, int displayAttrib)
{
    displayMirror(this, renderingObject, displayAttrib);
}
#endif

int CMirror::setFloatProperty(const char* ppName, double pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setFloatProperty(ppName, pState);
    if (retVal == -1)
        retVal = clipPlaneColor.setFloatProperty(ppName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CMirror::getFloatProperty(const char* ppName, double& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getFloatProperty(ppName, pState);
    if (retVal == -1)
        retVal = clipPlaneColor.getFloatProperty(ppName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CMirror::getStringProperty(const char* ppName, std::string& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getStringProperty(ppName, pState);
    if (retVal == -1)
    {
        if (_pName == propMirror_objectMetaInfo.name)
        {
            pState = OBJECT_META_INFO;
            retVal = 1;
        }
    }

    return retVal;
}

int CMirror::setColorProperty(const char* ppName, const float* pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setColorProperty(ppName, pState);
    if (retVal == -1)
        retVal = clipPlaneColor.setColorProperty(ppName, pState);
    if (retVal != -1)
    {
    }
    return retVal;
}

int CMirror::getColorProperty(const char* ppName, float* pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getColorProperty(ppName, pState);
    if (retVal == -1)
        retVal = clipPlaneColor.getColorProperty(ppName, pState);
    if (retVal != -1)
    {
    }
    return retVal;
}

int CMirror::getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const
{
    int retVal = CSceneObject::getPropertyName(index, pName, appartenance, excludeFlags);
    if (retVal == -1)
    {
        appartenance = "mirror";
        retVal = clipPlaneColor.getPropertyName(index, pName, excludeFlags);
    }
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_mirror.size(); i++)
        {
            if ((pName.size() == 0) || utils::startsWith(allProps_mirror[i].name, pName.c_str()))
            {
                if ((allProps_mirror[i].flags & excludeFlags) == 0)
                {
                    index--;
                    if (index == -1)
                    {
                        pName = allProps_mirror[i].name;
                        retVal = 1;
                        break;
                    }
                }
            }
        }
    }
    return retVal;
}

int CMirror::getPropertyName_static(int& index, std::string& pName, std::string& appartenance, int excludeFlags)
{
    int retVal = CSceneObject::getPropertyName_bstatic(index, pName, appartenance, excludeFlags);
    if (retVal == -1)
    {
        appartenance = "mirror";
        retVal = CColorObject::getPropertyName_static(index, pName, 1 + 4 + 8, "", excludeFlags);
    }
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_mirror.size(); i++)
        {
            if ((pName.size() == 0) || utils::startsWith(allProps_mirror[i].name, pName.c_str()))
            {
                if ((allProps_mirror[i].flags & excludeFlags) == 0)
                {
                    index--;
                    if (index == -1)
                    {
                        pName = allProps_mirror[i].name;
                        retVal = 1;
                        break;
                    }
                }
            }
        }
    }
    return retVal;
}

int CMirror::getPropertyInfo(const char* ppName, int& info, std::string& infoTxt) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getPropertyInfo(ppName, info, infoTxt);
    if (retVal == -1)
        retVal = clipPlaneColor.getPropertyInfo(ppName, info, infoTxt);
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_mirror.size(); i++)
        {
            if (strcmp(allProps_mirror[i].name, ppName) == 0)
            {
                retVal = allProps_mirror[i].type;
                info = allProps_mirror[i].flags;
                if (infoTxt == "j")
                    infoTxt = allProps_mirror[i].shortInfoTxt;
                else
                {
                    auto w = QJsonDocument::fromJson(allProps_mirror[i].shortInfoTxt).object();
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

int CMirror::getPropertyInfo_static(const char* ppName, int& info, std::string& infoTxt)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getPropertyInfo_bstatic(ppName, info, infoTxt);
    if (retVal == -1)
        retVal = CColorObject::getPropertyInfo_static(ppName, info, infoTxt, 1 + 4 + 8, "");
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_mirror.size(); i++)
        {
            if (strcmp(allProps_mirror[i].name, ppName) == 0)
            {
                retVal = allProps_mirror[i].type;
                info = allProps_mirror[i].flags;
                if (infoTxt == "j")
                    infoTxt = allProps_mirror[i].shortInfoTxt;
                else
                {
                    auto w = QJsonDocument::fromJson(allProps_mirror[i].shortInfoTxt).object();
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
