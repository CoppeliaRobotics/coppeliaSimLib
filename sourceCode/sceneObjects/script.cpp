#include <simInternal.h>
#include <simStrings.h>
#include <utils.h>
#include <script.h>
#include <global.h>
#include <app.h>
#include <tt.h>
#ifdef SIM_WITH_GUI
#include <scriptRendering.h>
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

CScript::CScript()
{
    _commonInit(sim_scripttype_simulation, "", 0, nullptr);
}

CScript::CScript(int scriptType, const char* text, int options, const char* lang)
{
    _commonInit(scriptType, text, options, lang);
}

CScript::CScript(CDetachedScript* scrObj)
{
    _commonInit(sim_scripttype_simulation, "", 0, nullptr);
    delete detachedScript;
    detachedScript = scrObj;
    detachedScript->_sceneObjectScript = true;
}

void CScript::_commonInit(int scriptType, const char* text, int options, const char* lang)
{
    _objectTypeStr = "script";
    _originalObjectTypeStr = _objectTypeStr;
    _objectMetaInfo = OBJECT_META_INFO;
    detachedScript = new CDetachedScript(scriptType);
    detachedScript->_scriptText = text;
    detachedScript->_sceneObjectScript = true;
    if ((scriptType != sim_scripttype_simulation) && (scriptType != sim_scripttype_customization))
        options |= 1;
    detachedScript->setScriptIsDisabled(options & 1);
    detachedScript->setLang(lang);
    _objectType = sim_sceneobject_script;
    _localObjectSpecialProperty = 0;
    _objectProperty |= sim_objectproperty_dontshowasinsidemodel;
    _scriptSize = 0.01;
    _resetAfterSimError = false;

    _visibilityLayer = SCRIPT_LAYER;
    _objectAlias = _objectTypeStr;
    _objectName_old = _objectTypeStr;
    _objectAltName_old = tt::getObjectAltNameFromObjectName(_objectName_old.c_str());

    _scriptColor.setDefaultValues();
    _scriptColor.setColor(1.0f, 1.0f, 1.0f, sim_materialcomponent_diffuse);

    computeBoundingBox();
}

CScript::~CScript()
{
}

void CScript::setIsInScene(bool s)
{
    CSceneObject::setIsInScene(s);
    if (s)
        _scriptColor.setEventParams(true, _objectHandle);
    else
        _scriptColor.setEventParams(true, -1);
}

void CScript::setObjectHandle(int newObjectHandle)
{
    CSceneObject::setObjectHandle(newObjectHandle);
    detachedScript->_scriptHandle = newObjectHandle;
    detachedScript->_sceneObjectHandle = newObjectHandle;
}

bool CScript::canDestroyNow()
{ // overridden from CSceneObject
    bool retVal = CSceneObject::canDestroyNow();
    if (detachedScript != nullptr)
    {
#ifdef SIM_WITH_GUI
        if (GuiApp::mainWindow != nullptr)
            GuiApp::mainWindow->codeEditorContainer->closeFromScriptUid(detachedScript->getScriptUid(), detachedScript->_previousEditionWindowPosAndSize, true);
#endif
        if (detachedScript->getExecutionDepth() != 0)
            retVal = false;
        if (retVal)
        {
            if (detachedScript->_scriptState == CDetachedScript::scriptState_initialized)
                detachedScript->systemCallScript(sim_syscb_cleanup, nullptr, nullptr);
            detachedScript->_scriptState = CDetachedScript::scriptState_ended; // just in case
            detachedScript->resetScript();
            // Announcements need to happen immediately after calling cleanup!
            App::scenes->announceScriptStateWillBeErased(detachedScript->getScriptHandle(), detachedScript->getScriptUid(), detachedScript->isSimulationOrMainScript(), detachedScript->isSceneSwitchPersistentScript());
            App::scenes->announceScriptWillBeErased(detachedScript->getScriptHandle(), detachedScript->getScriptUid(), detachedScript->isSimulationOrMainScript(), detachedScript->isSceneSwitchPersistentScript());
            App::scenes->setModificationFlag(16384);
            CDetachedScript::destroy(detachedScript, true, true);
            detachedScript = nullptr;
            if (_isInScene && App::scenes->getEventsEnabled())
            { // indicate that this object does not have any detachedScript attached anymore
                const char* cmd = propScript_detachedScript.name;
                CCbor* ev = App::scenes->createSceneObjectChangedEvent(this, false, cmd, true);
                if (App::getEventProtocolVersion() <= 3)
                    ev->appendKeyInt64(cmd, -1);
                else
                    ev->appendKeyHandle(cmd, -1);
                App::scenes->pushEvent();
            }
        }
    }
    return retVal;
}

std::string CScript::getObjectTypeInfoExtended() const
{
    return _objectTypeStr;
}

bool CScript::isPotentiallyCollidable() const
{
    return (false);
}

bool CScript::isPotentiallyMeasurable() const
{
    return (false);
}

bool CScript::isPotentiallyDetectable() const
{
    return (false);
}

void CScript::computeBoundingBox()
{
    _setBB(C7Vector::identityTransformation, C3Vector(1.05, 1.05, 1.05) * _scriptSize * 0.5);
}

void CScript::scaleObject(double scalingFactor)
{
    setScriptSize(_scriptSize * scalingFactor);
    CSceneObject::scaleObject(scalingFactor);
}

void CScript::removeSceneDependencies()
{
    CSceneObject::removeSceneDependencies();
}

void CScript::addObjectEventData(CCbor* ev)
{
    if (App::getEventProtocolVersion() == 2)
    {
        ev->openKeyMap(_objectTypeStr.c_str());
        ev->openKeyArray("colors");
        float c[9];
        _scriptColor.getColor(c, sim_materialcomponent_diffuse);
        _scriptColor.getColor(c + 3, sim_materialcomponent_specular);
        _scriptColor.getColor(c + 6, sim_materialcomponent_emission);
        ev->appendFloatArray(c, 9);
        ev->closeArrayOrMap(); // colors
    }
    else
        _scriptColor.addGenesisEventData(ev);
    ev->appendKeyDouble(propScript_size.name, _scriptSize);
    ev->appendKeyBool(propScript_resetAfterSimError.name, _resetAfterSimError);
    if (App::getEventProtocolVersion() <= 3)
        ev->appendKeyInt64(propScript_detachedScript.name, detachedScript->getObjectHandle());
    else
        ev->appendKeyHandle(propScript_detachedScript.name, detachedScript->getObjectHandle());
    if (App::getEventProtocolVersion() == 2)
        ev->closeArrayOrMap(); // script
    CSceneObject::addObjectEventData(ev);
}

CSceneObject* CScript::copyYourself()
{
    CScript* newScript = (CScript*)CSceneObject::copyYourself();

    _scriptColor.copyYourselfInto(&newScript->_scriptColor);
    newScript->_scriptSize = _scriptSize;
    newScript->_resetAfterSimError = _resetAfterSimError;

    newScript->detachedScript = detachedScript->copyYourself();
    newScript->detachedScript->_sceneObjectScript = true;

    return (newScript);
}

void CScript::announceCollectionWillBeErased(int groupID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollectionWillBeErased(groupID, copyBuffer);
}

void CScript::announceCollisionWillBeErased(int collisionID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollisionWillBeErased(collisionID, copyBuffer);
}

void CScript::announceDistanceWillBeErased(int distanceID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceDistanceWillBeErased(distanceID, copyBuffer);
}

void CScript::performIkLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performIkLoadingMapping(map, opType);
}

void CScript::performCollectionLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performCollectionLoadingMapping(map, opType);
}

void CScript::performCollisionLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performCollisionLoadingMapping(map, opType);
}

void CScript::performDistanceLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performDistanceLoadingMapping(map, opType);
}

void CScript::performTextureObjectLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performTextureObjectLoadingMapping(map, opType);
}

void CScript::performDynMaterialObjectLoadingMapping(const std::map<int, int>* map)
{
    CSceneObject::performDynMaterialObjectLoadingMapping(map);
}

void CScript::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
    detachedScript->initializeInitialValues(simulationAlreadyRunning);
}

void CScript::simulationAboutToStart()
{
    initializeInitialValues(false);
    CSceneObject::simulationAboutToStart();
    detachedScript->simulationAboutToStart();
}

void CScript::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it
    // ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized)
    {
        if ((getCumulativeModelProperty() & sim_modelproperty_not_reset) == 0)
        {
        }
    }
    detachedScript->simulationEnded();
    CSceneObject::simulationEnded();
}

void CScript::serialize(CSer& ar)
{
    CSceneObject::serialize(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            ar.storeDataName("Sos");
            ar << _scriptSize;
            ar.flush();

            ar.storeDataName("Var");
            unsigned char dummy = 0;
            SIM_SET_CLEAR_BIT(dummy, 0, _resetAfterSimError);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Soc");
            ar.setCountingMode();
            _scriptColor.serialize(ar, 0);
            if (ar.setWritingMode())
                _scriptColor.serialize(ar, 0);

            ar.storeDataName("Soo");
            ar.setCountingMode();
            detachedScript->serialize(ar);
            if (ar.setWritingMode())
                detachedScript->serialize(ar);

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
                    if (theName.compare("Sos") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _scriptSize;
                    }

                    if (theName.compare("Var") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _resetAfterSimError = SIM_IS_BIT_SET(dummy, 0);
                    }

                    if (theName.compare("Soc") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        _scriptColor.serialize(ar, 0);
                    }

                    if (theName.compare("Soo") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        detachedScript->serialize(ar);
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
            ar.xmlAddNode_float("size", _scriptSize);
            ar.xmlAddNode_bool("resetAfterSimError", _resetAfterSimError);

            ar.xmlPushNewNode("color");
            if (exhaustiveXml)
                _scriptColor.serialize(ar, 0);
            else
            {
                int rgb[3];
                for (size_t l = 0; l < 3; l++)
                    rgb[l] = int(_scriptColor.getColorsPtr()[l] * 255.1);
                ar.xmlAddNode_ints("object", rgb, 3);
            }
            ar.xmlPopNode();

            detachedScript->serialize(ar);
        }
        else
        {
            ar.xmlGetNode_float("size", _scriptSize, exhaustiveXml);
            ar.xmlGetNode_bool("resetAfterSimError", _resetAfterSimError, exhaustiveXml);

            if (ar.xmlPushChildNode("color", exhaustiveXml))
            {
                if (exhaustiveXml)
                    _scriptColor.serialize(ar, 0);
                else
                {
                    int rgb[3];
                    if (ar.xmlGetNode_ints("object", rgb, 3, exhaustiveXml))
                        _scriptColor.setColor(float(rgb[0]) / 255.1, float(rgb[1]) / 255.1, float(rgb[2]) / 255.1,
                                              sim_materialcomponent_diffuse);
                }
                ar.xmlPopNode();
            }

            detachedScript->serialize(ar);

            computeBoundingBox();
        }
    }
}

void CScript::performObjectLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performObjectLoadingMapping(map, opType);
}

void CScript::announceObjectWillBeErased(const CSceneObject* object, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceObjectWillBeErased(object, copyBuffer);
}

void CScript::announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceIkObjectWillBeErased(ikGroupID, copyBuffer);
}

double CScript::getScriptSize() const
{
    return (_scriptSize);
}

void CScript::reinitAfterSimulationIfNeeded()
{
    if (detachedScript != nullptr)
    {
        if (detachedScript->getScriptType() == sim_scripttype_customization)
        {
            if ((detachedScript->getScriptState() & CDetachedScript::scriptState_error) && _resetAfterSimError)
                detachedScript->initScript();
        }
    }
}

bool CScript::getResetAfterSimError() const
{
    return _resetAfterSimError;
}

CColorObject* CScript::getScriptColor()
{
    return (&_scriptColor);
}

void CScript::setScriptSize(double s)
{
    bool diff = (_scriptSize != s);
    if (diff)
    {
        _scriptSize = s;
        computeBoundingBox();
        if (_isInScene && App::scenes->getEventsEnabled())
        {
            const char* cmd = propScript_size.name;
            CCbor* ev = App::scenes->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDouble(cmd, _scriptSize);
            App::scenes->pushEvent();
        }
    }
}

void CScript::resetAfterSimError(bool r)
{
    bool diff = (_resetAfterSimError != r);
    if (diff)
    {
        _resetAfterSimError = r;
        if (_isInScene && App::scenes->getEventsEnabled())
        {
            const char* cmd = propScript_resetAfterSimError.name;
            CCbor* ev = App::scenes->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _resetAfterSimError);
            App::scenes->pushEvent();
        }
    }
}

int CScript::getDetachedScriptHandle() const
{
    return detachedScript->getObjectHandle();
}

#ifdef SIM_WITH_GUI
void CScript::display(CViewableBase* renderingObject, int displayAttrib)
{
    displayScript(this, renderingObject, displayAttrib);
}
#endif

int CScript::setBoolProperty(const char* ppName, bool pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setBoolProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (_pName == propScript_resetAfterSimError.name)
        {
            resetAfterSimError(pState);
            retVal = sim_propertyret_ok;
        }
    }

    // for backw. compatibility
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (strcmp(propScript_DEPRECATED_scriptDisabled.name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            detachedScript->setScriptIsDisabled(pState);
        }
        else if (strcmp(propScript_DEPRECATED_restartOnError.name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            detachedScript->setAutoRestartOnError(pState);
        }
    }

    return retVal;
}

int CScript::getBoolProperty(const char* ppName, bool& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getBoolProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (_pName == propScript_resetAfterSimError.name)
        {
            pState = _resetAfterSimError;
            retVal = sim_propertyret_ok;
        }
    }

    // for backw. compatibility
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (strcmp(propScript_DEPRECATED_scriptDisabled.name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            pState = detachedScript->getScriptIsDisabled();
        }
        else if (strcmp(propScript_DEPRECATED_restartOnError.name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            pState = detachedScript->getAutoRestartOnError();
        }
    }

    return retVal;
}

int CScript::setIntProperty(const char* ppName, int pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setIntProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }

    // for backw. compatibility
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (strcmp(propScript_DEPRECATED_execPriority.name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            detachedScript->setScriptExecPriority(pState);
        }
    }

    return retVal;
}

int CScript::getIntProperty(const char* ppName, int& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getIntProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }

    // for backw. compatibility
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (strcmp(propScript_DEPRECATED_execPriority.name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            pState = detachedScript->getScriptExecPriority();
        }
        else if (strcmp(propScript_DEPRECATED_scriptType.name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            pState = detachedScript->getScriptType();
        }
        else if (strcmp(propScript_DEPRECATED_executionDepth.name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            pState = detachedScript->getExecutionDepth();
        }
        else if (strcmp(propScript_DEPRECATED_scriptState.name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            pState = detachedScript->getScriptState();
        }
    }

    return retVal;
}

int CScript::setLongProperty(const char* ppName, long long int pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setLongProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }

    return retVal;
}

int CScript::getLongProperty(const char* ppName, long long int& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getLongProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }

    return retVal;
}

int CScript::getHandleProperty(const char* ppName, long long int& pState) const
{
    int retVal = CSceneObject::getHandleProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (strcmp(propScript_detachedScript.name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            pState = -1;
            if (detachedScript != nullptr)
                pState = detachedScript->getObjectHandle();
        }
     }

    return retVal;
}

int CScript::setFloatProperty(const char* ppName, double pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setFloatProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (_pName == propScript_size.name)
        {
            setScriptSize(pState);
            retVal = sim_propertyret_ok;
        }
    }

    return retVal;
}

int CScript::getFloatProperty(const char* ppName, double& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getFloatProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _scriptColor.getFloatProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (_pName == propScript_size.name)
        {
            pState = _scriptSize;
            retVal = sim_propertyret_ok;
        }
    }

    return retVal;
}

int CScript::setStringProperty(const char* ppName, const std::string& pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setStringProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }

    // for backw. compatibility
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (strcmp(propScript_DEPRECATED_code.name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            detachedScript->setScriptText(pState.c_str());
        }
    }

    return retVal;
}

int CScript::getStringProperty(const char* ppName, std::string& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getStringProperty(ppName, pState);

    // for backw. compatibility
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (strcmp(propScript_DEPRECATED_code.name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
#ifdef SIM_WITH_GUI
            if (GuiApp::mainWindow != nullptr)
                GuiApp::mainWindow->codeEditorContainer->saveOrCopyOperationAboutToHappen();
#endif
            pState = detachedScript->getScriptText();
        }
        else if (strcmp(propScript_DEPRECATED_language.name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            pState = detachedScript->getLang();
        }
        else if (strcmp(propScript_DEPRECATED_scriptName.name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            pState = detachedScript->getScriptName();
        }
        else if (strcmp(propScript_DEPRECATED_addOnPath.name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            pState = detachedScript->getAddOnPath();
        }
        else if (strcmp(propScript_DEPRECATED_addOnMenuPath.name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            pState = detachedScript->getAddOnMenuPath();;
        }
    }

    return retVal;
}

int CScript::setColorProperty(const char* ppName, const float* pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setColorProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _scriptColor.setColorProperty(ppName, pState);
    return retVal;
}

int CScript::getColorProperty(const char* ppName, float* pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getColorProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _scriptColor.getColorProperty(ppName, pState);
    return retVal;
}

int CScript::getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const
{
    int retVal = CSceneObject::getPropertyName(index, pName, appartenance, excludeFlags);
    if (_isInScene && (retVal == sim_propertyret_unknownproperty))
    {
        appartenance = _originalObjectTypeStr;
        retVal = _scriptColor.getPropertyName(index, pName, excludeFlags);
        if (retVal == sim_propertyret_unknownproperty)
        {
            for (size_t i = 0; i < allProps_script.size(); i++)
            {
                if ((pName.size() == 0) || utils::startsWith(allProps_script[i].name, pName.c_str()))
                {
                    if ((allProps_script[i].flags & excludeFlags) == 0)
                    {
                        index--;
                        if (index == -1)
                        {
                            pName = allProps_script[i].name;
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

int CScript::getPropertyInfo(const char* ppName, int& info, std::string& infoTxt) const
{
    int retVal = CSceneObject::getPropertyInfo(ppName, info, infoTxt);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _scriptColor.getPropertyInfo(ppName, info, infoTxt);
    if (retVal == sim_propertyret_unknownproperty)
    {
        for (size_t i = 0; i < allProps_script.size(); i++)
        {
            if (strcmp(allProps_script[i].name, ppName) == 0)
            {
                retVal = allProps_script[i].type;
                info = allProps_script[i].flags;
                if (infoTxt == "j")
                    infoTxt = allProps_script[i].shortInfoTxt;
                else
                {
                    auto w = QJsonDocument::fromJson(allProps_script[i].shortInfoTxt.c_str()).object();
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
