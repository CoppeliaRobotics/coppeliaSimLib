#include <simInternal.h>
#include <simStrings.h>
#include <utils.h>
#include <scriptObject.h>
#include <global.h>
#include <app.h>
#include <tt.h>
#ifdef SIM_WITH_GUI
#include <scriptRendering.h>
#include <guiApp.h>
#endif

CScriptObject::CScriptObject()
{
    _commonInit(sim_scripttype_simulation, "", 0, nullptr);
}

CScriptObject::CScriptObject(int scriptType, const char* text, int options, const char* lang)
{
    _commonInit(scriptType, text, options, lang);
}

CScriptObject::CScriptObject(CScript* script)
{
    _commonInit(sim_scripttype_simulation, "", 0, nullptr);
    delete nakedScript;
    nakedScript = script;
    nakedScript->_sceneObjectScript = true;
}

void CScriptObject::_commonInit(int scriptType, const char* text, int options, const char* lang)
{
    printf("bli %s\n", magic_enum::enum_name(magic_enum::enum_cast<sceneObjectType>(sim_sceneobject_scriptobject).value()).data());


//    printf("bla: %s\n", magic_enum::enum_name(magic_enum::enum_cast<sceneObjectType>(sim_sceneobject_scriptobject)).data().c_str());
    _objectTypeStr = "scriptObject";
    _originalObjectTypeStr = _objectTypeStr;
    nakedScript = new CScript(scriptType);
    nakedScript->_scriptText = text;
    nakedScript->_sceneObjectScript = true;
    if ((scriptType != sim_scripttype_simulation) && (scriptType != sim_scripttype_customization))
        options |= 1;
    nakedScript->setScriptIsDisabled(options & 1);
    nakedScript->setLang(lang);
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

CScriptObject::~CScriptObject()
{
}

void CScriptObject::setIsInScene(bool s)
{
    CSceneObject::setIsInScene(s);
    if (s)
        _scriptColor.setEventParams(true, _objectHandle);
    else
        _scriptColor.setEventParams(true, -1);
}

void CScriptObject::setObjectHandle(int newObjectHandle)
{
    CSceneObject::setObjectHandle(newObjectHandle);
    nakedScript->_sceneObjectOrnakedScriptHandle = newObjectHandle;
    nakedScript->_sceneObjectHandle = newObjectHandle;
}

bool CScriptObject::canDestroyNow()
{ // overridden from CSceneObject
    bool retVal = CSceneObject::canDestroyNow();
    if (nakedScript != nullptr)
    {
#ifdef SIM_WITH_GUI
        if (GuiApp::mainWindow != nullptr)
            GuiApp::mainWindow->codeEditorContainer->closeFromScriptUid(nakedScript->getScriptUid(), nakedScript->_previousEditionWindowPosAndSize, true);
#endif
        if (nakedScript->getExecutionDepth() != 0)
            retVal = false;
        if (retVal)
        {
            if (nakedScript->_scriptState == sim_scriptstate_initialized)
                nakedScript->systemCallScript(sim_syscb_cleanup, nullptr, nullptr);
            nakedScript->_scriptState = sim_scriptstate_ended; // just in case
            nakedScript->resetScript();
            // Announcements need to happen immediately after calling cleanup!
            App::scenes->announceScriptStateWillBeErased(nakedScript->getObjectHandle(), nakedScript->getScriptUid(), nakedScript->isSimulationOrMainScript(), nakedScript->isSceneSwitchPersistentScript());
            App::scenes->announceScriptWillBeErased(_objectHandle, nakedScript->getScriptUid(), nakedScript->isSimulationOrMainScript(), nakedScript->isSceneSwitchPersistentScript());
            App::scenes->setModificationFlag(16384);
            CScript::destroy(nakedScript, true, true);
            nakedScript = nullptr;
            if (_isInScene && App::scenes->getEventsEnabled())
            { // indicate that this object does not have any nakedScript attached anymore
                const char* cmd = prop(PropScriptObject::script).name;
                CCbor* ev = App::scenes->createSceneObjectChangedEvent(this, false, cmd, true);
                ev->appendKeyHandle(cmd, -1);
                App::scenes->pushEvent();
            }
        }
    }
    return retVal;
}

std::string CScriptObject::getObjectTypeInfoExtended() const
{
    return _objectTypeStr;
}

bool CScriptObject::isPotentiallyCollidable() const
{
    return (false);
}

bool CScriptObject::isPotentiallyMeasurable() const
{
    return (false);
}

bool CScriptObject::isPotentiallyDetectable() const
{
    return (false);
}

void CScriptObject::computeBoundingBox()
{
    _setBB(CPose::identityTransformation, C3Vector(1.05, 1.05, 1.05) * _scriptSize * 0.5);
}

void CScriptObject::scaleObject(double scalingFactor)
{
    setScriptSize(_scriptSize * scalingFactor);
    CSceneObject::scaleObject(scalingFactor);
}

void CScriptObject::removeSceneDependencies()
{
    CSceneObject::removeSceneDependencies();
}

void CScriptObject::pushNakedGenesisEvents(CCbor* ev /*= nullptr*/)
{
    if (_isInScene && App::scenes->getEventsEnabled())
    {
        bool createdHere = (ev == nullptr);
        if (createdHere)
            ev = App::scenes->createSceneObjectAddEvent(this);
        _scriptColor.addGenesisEventData(ev);
        ev->appendKeyBool(prop(PropScriptObject::resetAfterSimError).name, _resetAfterSimError);
        ev->appendKeyDouble(prop(PropScriptObject::size).name, _scriptSize);
        ev->appendKeyHandle(prop(PropScriptObject::script).name, -1); // because 'naked'
        std::string st;
        auto enum_value = magic_enum::enum_cast<scriptType>(nakedScript->getScriptType());
        if (enum_value.has_value())
            st = magic_enum::enum_name(enum_value.value()).data();
        ev->appendKeyText(prop(PropScriptObject::type).name, st.c_str());
        CSceneObject::pushNakedGenesisEvents(ev);
        App::scenes->pushEvent();

        ((CScriptObject*)this)->nakedScript->pushNakedGenesisEvents();

        ev = App::scenes->createSceneObjectChangedEvent(_objectHandle, false, prop(PropScriptObject::script).name, false);
        ev->appendKeyHandle(prop(PropScriptObject::script).name, ((CScriptObject*)this)->nakedScript->getObjectHandle());
        if (createdHere)
            App::scenes->pushEvent();
    }
}

CSceneObject* CScriptObject::copyYourself()
{
    CScriptObject* newScript = (CScriptObject*)CSceneObject::copyYourself();

    _scriptColor.copyYourselfInto(&newScript->_scriptColor);
    newScript->_scriptSize = _scriptSize;
    newScript->_resetAfterSimError = _resetAfterSimError;

    newScript->nakedScript = nakedScript->copyYourself();
    newScript->nakedScript->_sceneObjectScript = true;

    return (newScript);
}

void CScriptObject::announceCollectionWillBeErased(int groupID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollectionWillBeErased(groupID, copyBuffer);
}

void CScriptObject::announceCollisionWillBeErased(int collisionID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollisionWillBeErased(collisionID, copyBuffer);
}

void CScriptObject::announceDistanceWillBeErased(int distanceID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceDistanceWillBeErased(distanceID, copyBuffer);
}

void CScriptObject::performIkLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performIkLoadingMapping(map, opType);
}

void CScriptObject::performCollectionLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performCollectionLoadingMapping(map, opType);
}

void CScriptObject::performCollisionLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performCollisionLoadingMapping(map, opType);
}

void CScriptObject::performDistanceLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performDistanceLoadingMapping(map, opType);
}

void CScriptObject::performTextureObjectLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performTextureObjectLoadingMapping(map, opType);
}

void CScriptObject::performDynMaterialObjectLoadingMapping(const std::map<int, int>* map)
{
    CSceneObject::performDynMaterialObjectLoadingMapping(map);
}

void CScriptObject::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
    nakedScript->initializeInitialValues(simulationAlreadyRunning);
}

void CScriptObject::simulationAboutToStart()
{
    initializeInitialValues(false);
    CSceneObject::simulationAboutToStart();
    nakedScript->simulationAboutToStart();
}

void CScriptObject::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it
    // ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized)
    {
        if ((getCumulativeModelProperty() & sim_modelproperty_not_reset) == 0)
        {
        }
    }
    nakedScript->simulationEnded();
    CSceneObject::simulationEnded();
}

void CScriptObject::serialize(CSer& ar)
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
            nakedScript->serialize(ar);
            if (ar.setWritingMode())
                nakedScript->serialize(ar);

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
                        nakedScript->serialize(ar);
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

            nakedScript->serialize(ar);
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

            nakedScript->serialize(ar);

            computeBoundingBox();
        }
    }
}

void CScriptObject::performObjectLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performObjectLoadingMapping(map, opType);
}

void CScriptObject::announceSceneObjectWillBeErased(const CSceneObject* object, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceSceneObjectWillBeErased(object, copyBuffer);
}

void CScriptObject::announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceIkObjectWillBeErased(ikGroupID, copyBuffer);
}

double CScriptObject::getScriptSize() const
{
    return (_scriptSize);
}

void CScriptObject::reinitAfterSimulationIfNeeded()
{
    if (nakedScript != nullptr)
    {
        if (nakedScript->getScriptType() == sim_scripttype_customization)
        {
            if ((nakedScript->getScriptState() & sim_scriptstate_error) && _resetAfterSimError)
                nakedScript->initScript();
        }
    }
}

bool CScriptObject::getResetAfterSimError() const
{
    return _resetAfterSimError;
}

CColorObject* CScriptObject::getScriptColor()
{
    return (&_scriptColor);
}

void CScriptObject::setScriptSize(double s)
{
    bool diff = (_scriptSize != s);
    if (diff)
    {
        _scriptSize = s;
        computeBoundingBox();
        if (_isInScene && App::scenes->getEventsEnabled())
        {
            const char* cmd = prop(PropScriptObject::size).name;
            CCbor* ev = App::scenes->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDouble(cmd, _scriptSize);
            App::scenes->pushEvent();
        }
    }
}

void CScriptObject::resetAfterSimError(bool r)
{
    bool diff = (_resetAfterSimError != r);
    if (diff)
    {
        _resetAfterSimError = r;
        if (_isInScene && App::scenes->getEventsEnabled())
        {
            const char* cmd = prop(PropScriptObject::resetAfterSimError).name;
            CCbor* ev = App::scenes->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _resetAfterSimError);
            App::scenes->pushEvent();
        }
    }
}

int CScriptObject::getNakedScriptHandle() const
{
    int retVal = -1;
    if (nakedScript != nullptr)
        retVal = nakedScript->getObjectHandle();
    return retVal;
}

#ifdef SIM_WITH_GUI
void CScriptObject::display(CViewableBase* renderingObject, int displayAttrib)
{
    displayScript(this, renderingObject, displayAttrib);
}
#endif

int CScriptObject::setBoolProperty(const char* ppName, bool pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setBoolProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (_pName == prop(PropScriptObject::resetAfterSimError).name)
        {
            resetAfterSimError(pState);
            retVal = sim_propertyret_ok;
        }
    }

    // for backw. compatibility
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (strcmp(prop(PropScriptObject::DEPRECATED_scriptDisabled).name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            nakedScript->setScriptIsDisabled(pState);
        }
        else if (strcmp(prop(PropScriptObject::DEPRECATED_restartOnError).name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            nakedScript->setAutoRestartOnError(pState);
        }
    }

    return retVal;
}

int CScriptObject::getBoolProperty(const char* ppName, bool& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getBoolProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (_pName == prop(PropScriptObject::resetAfterSimError).name)
        {
            pState = _resetAfterSimError;
            retVal = sim_propertyret_ok;
        }
    }

    // for backw. compatibility
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (strcmp(prop(PropScriptObject::DEPRECATED_scriptDisabled).name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            pState = nakedScript->getScriptIsDisabled();
        }
        else if (strcmp(prop(PropScriptObject::DEPRECATED_restartOnError).name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            pState = nakedScript->getAutoRestartOnError();
        }
    }

    return retVal;
}

int CScriptObject::setIntProperty(const char* ppName, int pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setIntProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }

    // for backw. compatibility
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (strcmp(prop(PropScriptObject::DEPRECATED_execPriority).name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            nakedScript->setScriptExecPriority(pState);
        }
    }

    return retVal;
}

int CScriptObject::getIntProperty(const char* ppName, int& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getIntProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }

    // for backw. compatibility
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (strcmp(prop(PropScriptObject::DEPRECATED_execPriority).name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            pState = nakedScript->getScriptExecPriority();
        }
        else if (strcmp(prop(PropScriptObject::DEPRECATED_scriptType).name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            pState = nakedScript->getScriptType();
        }
        else if (strcmp(prop(PropScriptObject::DEPRECATED_executionDepth).name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            pState = nakedScript->getExecutionDepth();
        }
        else if (strcmp(prop(PropScriptObject::DEPRECATED_scriptState).name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            pState = nakedScript->getScriptState();
        }
    }

    return retVal;
}

int CScriptObject::setLongProperty(const char* ppName, int64_t pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setLongProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }

    return retVal;
}

int CScriptObject::getLongProperty(const char* ppName, int64_t& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getLongProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }

    return retVal;
}

int CScriptObject::getHandleProperty(const char* ppName, int64_t& pState) const
{
    int retVal = CSceneObject::getHandleProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (strcmp(prop(PropScriptObject::script).name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            pState = -1;
            if (nakedScript != nullptr)
                pState = nakedScript->getObjectHandle();
        }
     }

    return retVal;
}

int CScriptObject::setFloatProperty(const char* ppName, double pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setFloatProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (_pName == prop(PropScriptObject::size).name)
        {
            setScriptSize(pState);
            retVal = sim_propertyret_ok;
        }
    }

    return retVal;
}

int CScriptObject::getFloatProperty(const char* ppName, double& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getFloatProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _scriptColor.getFloatProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (_pName == prop(PropScriptObject::size).name)
        {
            pState = _scriptSize;
            retVal = sim_propertyret_ok;
        }
    }

    return retVal;
}

int CScriptObject::setStringProperty(const char* ppName, const std::string& pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setStringProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
    }

    // for backw. compatibility
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (strcmp(prop(PropScriptObject::DEPRECATED_code).name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            nakedScript->setScriptText(pState.c_str());
        }
    }

    return retVal;
}

int CScriptObject::getStringProperty(const char* ppName, std::string& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getStringProperty(ppName, pState);

    // for backw. compatibility
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (strcmp(prop(PropScriptObject::type).name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            auto enum_value = magic_enum::enum_cast<scriptType>(nakedScript->getScriptType());
            if (enum_value.has_value())
                pState = magic_enum::enum_name(enum_value.value()).data();
            else
                retVal = sim_propertyret_invalidvalue;
        }
        else if (strcmp(prop(PropScriptObject::DEPRECATED_code).name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
#ifdef SIM_WITH_GUI
            if (GuiApp::mainWindow != nullptr)
                GuiApp::mainWindow->codeEditorContainer->saveOrCopyOperationAboutToHappen();
#endif
            pState = nakedScript->getScriptText();
        }
        else if (strcmp(prop(PropScriptObject::DEPRECATED_language).name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            pState = nakedScript->getLang();
        }
        else if (strcmp(prop(PropScriptObject::DEPRECATED_scriptName).name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            pState = nakedScript->getScriptName();
        }
        else if (strcmp(prop(PropScriptObject::DEPRECATED_addOnPath).name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            pState = nakedScript->getAddOnPath();
        }
        else if (strcmp(prop(PropScriptObject::DEPRECATED_addOnMenuPath).name, ppName) == 0)
        {
            retVal = sim_propertyret_ok;
            pState = nakedScript->getAddOnMenuPath();;
        }
    }

    return retVal;
}

int CScriptObject::setColorProperty(const char* ppName, const float* pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setColorProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _scriptColor.setColorProperty(ppName, pState);
    return retVal;
}

int CScriptObject::getColorProperty(const char* ppName, float* pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getColorProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _scriptColor.getColorProperty(ppName, pState);
    return retVal;
}

int CScriptObject::getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const
{
    int retVal = CSceneObject::getPropertyName(index, pName, appartenance, excludeFlags);
    if (_isInScene && (retVal == sim_propertyret_unknownproperty))
    {
        appartenance = _originalObjectTypeStr;
        retVal = _scriptColor.getPropertyName(index, pName, excludeFlags);
        if (retVal == sim_propertyret_unknownproperty)
        {
            for (size_t i = 0; i < allProps_scriptObject.size(); i++)
            {
                if ((pName.size() == 0) || utils::startsWith(allProps_scriptObject[i].name, pName.c_str()))
                {
                    if ((allProps_scriptObject[i].flags & excludeFlags) == 0)
                    {
                        index--;
                        if (index == -1)
                        {
                            pName = allProps_scriptObject[i].name;
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

int CScriptObject::getPropertyInfo(const char* ppName, int& info, std::string& infoTxt) const
{
    int retVal = CSceneObject::getPropertyInfo(ppName, info, infoTxt);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _scriptColor.getPropertyInfo(ppName, info, infoTxt);
    if (retVal == sim_propertyret_unknownproperty)
    {
        for (size_t i = 0; i < allProps_scriptObject.size(); i++)
        {
            if (strcmp(allProps_scriptObject[i].name, ppName) == 0)
            {
                retVal = allProps_scriptObject[i].type;
                info = allProps_scriptObject[i].flags;
                if (infoTxt == "j")
                    infoTxt = allProps_scriptObject[i].info.json;
                else
                {
                    auto w = allProps_scriptObject[i].info.map;
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
