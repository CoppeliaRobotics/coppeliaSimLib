#include <simInternal.h>
#include <simStrings.h>
#include <utils.h>
#include <customSceneObject.h>
#include <global.h>
#include <app.h>
#include <tt.h>
#ifdef SIM_WITH_GUI
#include <dummyRendering.h>
#include <guiApp.h>
#endif

static std::string OBJECT_META_INFO = R"(
{
    "superclass": "sceneObject",
    "namespaces": {
        "refs": {"newPropertyForcedType": )" + std::to_string(sim_propertytype_handlearray) + R"(},
        "origRefs": {"newPropertyForcedType": )" + std::to_string(sim_propertytype_handlearray) + R"(},
        "customData": {},
        "signal": {}
    }
}
)";

CCustomSceneObject::CCustomSceneObject()
{
    _objectTypeStr = "customSceneObject";
    _objectMetaInfo = OBJECT_META_INFO;
    _objectType = sim_sceneobject_customsceneobject;
    _localObjectSpecialProperty = 0;

    _objectSize = 0.05;

    _visibilityLayer = CUSTOMSCENEOBJECT_LAYER;
    _objectAlias = _objectTypeStr;
    _objectName_old = _objectTypeStr;
    _objectAltName_old = tt::getObjectAltNameFromObjectName(_objectName_old.c_str());

    _objectColor.setDefaultValues();
    _objectColor.setColor(0.8f, 0.5f, 0.0f, sim_materialcomponent_diffuse);
    computeBoundingBox();
}

CCustomSceneObject::~CCustomSceneObject()
{
}

std::string CCustomSceneObject::getObjectTypeInfoExtended() const
{
    return _objectTypeStr;
}

bool CCustomSceneObject::isPotentiallyCollidable() const
{
    return false;
}

bool CCustomSceneObject::isPotentiallyMeasurable() const
{
    return false;
}

bool CCustomSceneObject::isPotentiallyDetectable() const
{
    return false;
}

void CCustomSceneObject::computeBoundingBox()
{
    _setBB(C7Vector::identityTransformation, C3Vector(1.0, 1.0, 1.0) * _objectSize * 0.5);
}

void CCustomSceneObject::setIsInScene(bool s)
{
    CSceneObject::setIsInScene(s);
    if (s)
        _objectColor.setEventParams(true, _objectHandle);
    else
        _objectColor.setEventParams(true, -1);
}

void CCustomSceneObject::scaleObject(double scalingFactor)
{
    setObjectSize(_objectSize * scalingFactor);

    CSceneObject::scaleObject(scalingFactor);
}

void CCustomSceneObject::removeSceneDependencies()
{
    CSceneObject::removeSceneDependencies();
}

void CCustomSceneObject::addObjectEventData(CCbor* ev)
{
    _objectColor.addGenesisEventData(ev);
    ev->appendKeyDouble(propCustomSceneObject_size.name, _objectSize);
    CSceneObject::addObjectEventData(ev);
}

CSceneObject* CCustomSceneObject::copyYourself()
{
    CCustomSceneObject* newObject = (CCustomSceneObject*)CSceneObject::copyYourself();

    _objectColor.copyYourselfInto(&newObject->_objectColor);
    newObject->_objectSize = _objectSize;

    return newObject;
}

void CCustomSceneObject::announceCollectionWillBeErased(int groupID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollectionWillBeErased(groupID, copyBuffer);
}

void CCustomSceneObject::announceCollisionWillBeErased(int collisionID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollisionWillBeErased(collisionID, copyBuffer);
}

void CCustomSceneObject::announceDistanceWillBeErased(int distanceID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceDistanceWillBeErased(distanceID, copyBuffer);
}

void CCustomSceneObject::performIkLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performIkLoadingMapping(map, opType);
}

void CCustomSceneObject::performCollectionLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performCollectionLoadingMapping(map, opType);
}

void CCustomSceneObject::performCollisionLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performCollisionLoadingMapping(map, opType);
}

void CCustomSceneObject::performDistanceLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performDistanceLoadingMapping(map, opType);
}

void CCustomSceneObject::performTextureObjectLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performTextureObjectLoadingMapping(map, opType);
}

void CCustomSceneObject::performDynMaterialObjectLoadingMapping(const std::map<int, int>* map)
{
    CSceneObject::performDynMaterialObjectLoadingMapping(map);
}

void CCustomSceneObject::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
}

void CCustomSceneObject::simulationAboutToStart()
{
    initializeInitialValues(false);
    CSceneObject::simulationAboutToStart();
}

void CCustomSceneObject::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it
    // ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized)
    {
        if ((getCumulativeModelProperty() & sim_modelproperty_not_reset) == 0)
        {
        }
    }
    CSceneObject::simulationEnded();
}

void CCustomSceneObject::serialize(CSer& ar)
{
    CSceneObject::serialize(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            ar.storeDataName("_y2");
            ar << _objectSize;
            ar.flush();

            ar.storeDataName("Cl0");
            ar.setCountingMode();
            _objectColor.serialize(ar, 0);
            if (ar.setWritingMode())
                _objectColor.serialize(ar, 0);

            ar.storeDataName("Var");
            unsigned char dummy = 0;
            ar << dummy;
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
                    if (theName.compare("_y2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _objectSize;
                    }
                    if (theName.compare("Cl0") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        _objectColor.serialize(ar, 0);
                    }
                    if (theName.compare("Var") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                    }
                    if (noHit)
                        ar.loadUnknownData();
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
            ar.xmlAddNode_float("size", _objectSize);
            if (exhaustiveXml)
            {
                ar.xmlPushNewNode("switches");
                ar.xmlPopNode();
            }

            ar.xmlPushNewNode("color");
            if (exhaustiveXml)
                _objectColor.serialize(ar, 0);
            else
            {
                int rgb[3];
                for (size_t l = 0; l < 3; l++)
                    rgb[l] = int(_objectColor.getColorsPtr()[l] * 255.1);
                ar.xmlAddNode_ints("object", rgb, 3);
            }
            ar.xmlPopNode();
        }
        else
        {
            ar.xmlGetNode_float("size", _objectSize, exhaustiveXml);
            if (exhaustiveXml && ar.xmlPushChildNode("switches"))
            {
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("color", exhaustiveXml))
            {
                if (exhaustiveXml)
                    _objectColor.serialize(ar, 0);
                else
                {
                    int rgb[3];
                    if (ar.xmlGetNode_ints("object", rgb, 3, exhaustiveXml))
                        _objectColor.setColor(float(rgb[0]) / 255.1, float(rgb[1]) / 255.1, float(rgb[2]) / 255.1, sim_materialcomponent_diffuse);
                }
                ar.xmlPopNode();
            }
            computeBoundingBox();
        }
    }
}

void CCustomSceneObject::performObjectLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performObjectLoadingMapping(map, opType);
}

void CCustomSceneObject::announceObjectWillBeErased(const CSceneObject* object, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceObjectWillBeErased(object, copyBuffer);
}

void CCustomSceneObject::announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceIkObjectWillBeErased(ikGroupID, copyBuffer);
}

double CCustomSceneObject::getObjectSize() const
{
    return (_objectSize);
}

CColorObject* CCustomSceneObject::getObjectColor()
{
    return (&_objectColor);
}

void CCustomSceneObject::setObjectSize(double s)
{
    bool diff = (_objectSize != s);
    if (diff)
    {
        _objectSize = s;
        computeBoundingBox();
        if (_isInScene && App::scenes->getEventsEnabled())
        {
            const char* cmd = propCustomSceneObject_size.name;
            CCbor* ev = App::scenes->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDouble(cmd, _objectSize);
            App::scenes->pushEvent();
        }
    }
}

#ifdef SIM_WITH_GUI
void CCustomSceneObject::display(CViewableBase* renderingObject, int displayAttrib)
{
    //displayCustomSceneObject(this, renderingObject, displayAttrib);
}
#endif

int CCustomSceneObject::setBoolProperty(const char* ppName, bool pState)
{
    int retVal = CSceneObject::setBoolProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }
    return retVal;
}

int CCustomSceneObject::getBoolProperty(const char* ppName, bool& pState) const
{
    int retVal = CSceneObject::getBoolProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }
    return retVal;
}

int CCustomSceneObject::setFloatProperty(const char* ppName, double pState)
{
    int retVal = CSceneObject::setFloatProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _objectColor.setFloatProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (strcmp(ppName, propCustomSceneObject_size.name) == 0)
        {
            setObjectSize(pState);
            retVal = sim_propertyret_ok;
        }
    }
    return retVal;
}

int CCustomSceneObject::getFloatProperty(const char* ppName, double& pState) const
{
    int retVal = CSceneObject::getFloatProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _objectColor.getFloatProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (strcmp(ppName, propCustomSceneObject_size.name) == 0)
        {
            pState = _objectSize;
            retVal = sim_propertyret_ok;
        }
    }
    return retVal;
}

int CCustomSceneObject::setStringProperty(const char* ppName, const std::string& pState)
{
    int retVal = CSceneObject::setStringProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }
    return retVal;
}

int CCustomSceneObject::getStringProperty(const char* ppName, std::string& pState) const
{
    int retVal = CSceneObject::getStringProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }
    return retVal;
}

int CCustomSceneObject::setColorProperty(const char* ppName, const float* pState)
{
    int retVal = CSceneObject::setColorProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _objectColor.setColorProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }
    return retVal;
}

int CCustomSceneObject::getColorProperty(const char* ppName, float* pState) const
{
    int retVal = CSceneObject::getColorProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _objectColor.getColorProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }
    return retVal;
}

int CCustomSceneObject::getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const
{
    int retVal = CSceneObject::getPropertyName(index, pName, appartenance, excludeFlags);
    if (retVal == sim_propertyret_unknownproperty)
    {
        appartenance = _objectTypeStr;
        retVal = _objectColor.getPropertyName(index, pName, excludeFlags);
        if (retVal == sim_propertyret_unknownproperty)
        {
            for (size_t i = 0; i < allProps_customSceneObject.size(); i++)
            {
                if ((pName.size() == 0) || utils::startsWith(allProps_customSceneObject[i].name, pName.c_str()))
                {
                    if ((allProps_customSceneObject[i].flags & excludeFlags) == 0)
                    {
                        index--;
                        if (index == -1)
                        {
                            pName = allProps_customSceneObject[i].name;
                            retVal = sim_propertyret_ok;
                            break;
                        }
                    }
                }
            }
        }
    }
    return retVal;
}

int CCustomSceneObject::getPropertyInfo(const char* ppName, int& info, std::string& infoTxt) const
{
    int retVal = CSceneObject::getPropertyInfo(ppName, info, infoTxt);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _objectColor.getPropertyInfo(ppName, info, infoTxt);
    if (retVal == sim_propertyret_unknownproperty)
    {
        for (size_t i = 0; i < allProps_customSceneObject.size(); i++)
        {
            if (strcmp(allProps_customSceneObject[i].name, ppName) == 0)
            {
                retVal = allProps_customSceneObject[i].type;
                info = allProps_customSceneObject[i].flags;
                if (infoTxt == "j")
                    infoTxt = allProps_customSceneObject[i].shortInfoTxt;
                else
                {
                    auto w = QJsonDocument::fromJson(allProps_customSceneObject[i].shortInfoTxt.c_str()).object();
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
