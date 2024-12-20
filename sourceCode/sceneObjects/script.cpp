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

CScript::CScript()
{
    _commonInit(sim_scripttype_simulation, "", 0, nullptr);
}

CScript::CScript(int scriptType, const char* text, int options, const char* lang)
{
    _commonInit(scriptType, text, options, lang);
}

CScript::CScript(CScriptObject* scrObj)
{
    _commonInit(sim_scripttype_simulation, "", 0, nullptr);
    delete scriptObject;
    scriptObject = scrObj;
    scriptObject->_sceneObjectScript = true;
}

void CScript::_commonInit(int scriptType, const char* text, int options, const char* lang)
{
    scriptObject = new CScriptObject(scriptType);
    scriptObject->_scriptText = text;
    scriptObject->_sceneObjectScript = true;
    if ((scriptType != sim_scripttype_simulation) && (scriptType != sim_scripttype_customization))
        options |= 1;
    scriptObject->setScriptIsDisabled(options & 1);
    scriptObject->setLang(lang);
    _objectType = sim_sceneobject_script;
    _localObjectSpecialProperty = 0;
    _objectProperty |= sim_objectproperty_dontshowasinsidemodel;
    _scriptSize = 0.01;
    _resetAfterSimError = false;

    _visibilityLayer = SCRIPT_LAYER;
    _objectAlias = getObjectTypeInfo();
    _objectName_old = getObjectTypeInfo();
    _objectAltName_old = tt::getObjectAltNameFromObjectName(_objectName_old.c_str());

    _scriptColor.setDefaultValues();
    _scriptColor.setColor(1.0f, 1.0f, 1.0f, sim_colorcomponent_ambient_diffuse);

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
    scriptObject->_scriptHandle = newObjectHandle;
    scriptObject->_sceneObjectHandle = newObjectHandle;
}

bool CScript::canDestroyNow()
{ // overridden from CSceneObject
    bool retVal = CSceneObject::canDestroyNow();
    if (scriptObject != nullptr)
    {
#ifdef SIM_WITH_GUI
        if (GuiApp::mainWindow != nullptr)
            GuiApp::mainWindow->codeEditorContainer->closeFromScriptUid(scriptObject->getScriptUid(), scriptObject->_previousEditionWindowPosAndSize, true);
#endif
        if (scriptObject->getExecutionDepth() != 0)
            retVal = false;
        if (retVal)
        {
            if (scriptObject->_scriptState == CScriptObject::scriptState_initialized)
                scriptObject->systemCallScript(sim_syscb_cleanup, nullptr, nullptr);
            scriptObject->_scriptState = CScriptObject::scriptState_ended; // just in case
            scriptObject->resetScript();
            // Announcements need to happen immediately after calling cleanup!
            App::worldContainer->announceScriptStateWillBeErased(scriptObject->getScriptHandle(), scriptObject->getScriptUid(), scriptObject->isSimulationOrMainScript(), scriptObject->isSceneSwitchPersistentScript());
            App::worldContainer->announceScriptWillBeErased(scriptObject->getScriptHandle(), scriptObject->getScriptUid(), scriptObject->isSimulationOrMainScript(), scriptObject->isSceneSwitchPersistentScript());
            App::worldContainer->setModificationFlag(16384);
            CScriptObject::destroy(scriptObject, true, true);
            scriptObject = nullptr;
        }
    }
    return retVal;
}

std::string CScript::getObjectTypeInfo() const
{
    return "script";
}

std::string CScript::getObjectTypeInfoExtended() const
{
    return getObjectTypeInfo();
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

void CScript::addSpecializedObjectEventData(CCbor* ev)
{
#if SIM_EVENT_PROTOCOL_VERSION == 2
    ev->openKeyMap(getObjectTypeInfo().c_str());
    ev->openKeyArray("colors");
    float c[9];
    _scriptColor.getColor(c, sim_colorcomponent_ambient_diffuse);
    _scriptColor.getColor(c + 3, sim_colorcomponent_specular);
    _scriptColor.getColor(c + 6, sim_colorcomponent_emission);
    ev->appendFloatArray(c, 9);
    ev->closeArrayOrMap(); // colors
#else
    _scriptColor.addGenesisEventData(ev);
#endif
    ev->appendKeyDouble(propScript_size.name, _scriptSize);
    ev->appendKeyBool(propScript_resetAfterSimError.name, _resetAfterSimError);
    scriptObject->addSpecializedObjectEventData(ev);
#if SIM_EVENT_PROTOCOL_VERSION == 2
    ev->closeArrayOrMap(); // script
#endif
}

CSceneObject* CScript::copyYourself()
{
    CScript* newScript = (CScript*)CSceneObject::copyYourself();

    _scriptColor.copyYourselfInto(&newScript->_scriptColor);
    newScript->_scriptSize = _scriptSize;
    newScript->_resetAfterSimError = _resetAfterSimError;

    newScript->scriptObject = scriptObject->copyYourself();
    newScript->scriptObject->_sceneObjectScript = true;

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

void CScript::performIkLoadingMapping(const std::map<int, int>* map, bool loadingAmodel)
{
    CSceneObject::performIkLoadingMapping(map, loadingAmodel);
}

void CScript::performCollectionLoadingMapping(const std::map<int, int>* map, bool loadingAmodel)
{
    CSceneObject::performCollectionLoadingMapping(map, loadingAmodel);
}

void CScript::performCollisionLoadingMapping(const std::map<int, int>* map, bool loadingAmodel)
{
    CSceneObject::performCollisionLoadingMapping(map, loadingAmodel);
}

void CScript::performDistanceLoadingMapping(const std::map<int, int>* map, bool loadingAmodel)
{
    CSceneObject::performDistanceLoadingMapping(map, loadingAmodel);
}

void CScript::performTextureObjectLoadingMapping(const std::map<int, int>* map)
{
    CSceneObject::performTextureObjectLoadingMapping(map);
}

void CScript::performDynMaterialObjectLoadingMapping(const std::map<int, int>* map)
{
    CSceneObject::performDynMaterialObjectLoadingMapping(map);
}

void CScript::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
    scriptObject->initializeInitialValues(simulationAlreadyRunning);
}

void CScript::simulationAboutToStart()
{
    initializeInitialValues(false);
    CSceneObject::simulationAboutToStart();
    scriptObject->simulationAboutToStart();
}

void CScript::simulationEnded()
{   // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it
    // ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized)
    {
        if (App::currentWorld->simulation->getResetSceneAtSimulationEnd() &&
            ((getCumulativeModelProperty() & sim_modelproperty_not_reset) == 0))
        {
        }
    }
    scriptObject->simulationEnded();
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
            scriptObject->serialize(ar);
            if (ar.setWritingMode())
                scriptObject->serialize(ar);

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
                        scriptObject->serialize(ar);
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

            scriptObject->serialize(ar);
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
                                              sim_colorcomponent_ambient_diffuse);
                }
                ar.xmlPopNode();
            }

            scriptObject->serialize(ar);

            computeBoundingBox();
        }
    }
}

void CScript::performObjectLoadingMapping(const std::map<int, int>* map, bool loadingAmodel)
{
    CSceneObject::performObjectLoadingMapping(map, loadingAmodel);
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
    if (scriptObject != nullptr)
    {
        if (scriptObject->getScriptType() == sim_scripttype_customization)
        {
            if ((scriptObject->getScriptState() & CScriptObject::scriptState_error) && _resetAfterSimError)
            {
                scriptObject->resetScript();
                scriptObject->initScript();
            }
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
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propScript_size.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDouble(cmd, _scriptSize);
            App::worldContainer->pushEvent();
        }
    }
}

void CScript::resetAfterSimError(bool r)
{
    bool diff = (_resetAfterSimError != r);
    if (diff)
    {
        _resetAfterSimError = r;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propScript_resetAfterSimError.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _resetAfterSimError);
            App::worldContainer->pushEvent();
        }
    }
}

#ifdef SIM_WITH_GUI
void CScript::display(CViewableBase* renderingObject, int displayAttrib)
{
    displayScript(this, renderingObject, displayAttrib);
}
#endif

int CScript::setBoolProperty(const char* ppName, bool pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "script."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::setBoolProperty(pName, pState);
    if (retVal == -1)
        retVal = scriptObject->setBoolProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propScript_resetAfterSimError.name)
        {
            resetAfterSimError(pState);
            retVal = 1;
        }
    }

    return retVal;
}

int CScript::getBoolProperty(const char* ppName, bool& pState) const
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "script."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getBoolProperty(pName, pState);
    if (retVal == -1)
        retVal = scriptObject->getBoolProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propScript_resetAfterSimError.name)
        {
            pState = _resetAfterSimError;
            retVal = 1;
        }
    }

    return retVal;
}

int CScript::setIntProperty(const char* ppName, int pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "script."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::setIntProperty(pName, pState);
    if (retVal == -1)
        retVal = scriptObject->setIntProperty(pName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CScript::getIntProperty(const char* ppName, int& pState) const
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "script."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getIntProperty(pName, pState);
    if (retVal == -1)
        retVal = scriptObject->getIntProperty(pName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CScript::setLongProperty(const char* ppName, long long int pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "script."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::setLongProperty(pName, pState);
    if (retVal == -1)
        retVal = scriptObject->setLongProperty(pName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CScript::getLongProperty(const char* ppName, long long int& pState) const
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "script."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getLongProperty(pName, pState);
    if (retVal == -1)
        retVal = scriptObject->getLongProperty(pName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CScript::setFloatProperty(const char* ppName, double pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "script."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::setFloatProperty(pName, pState);
    if (retVal == -1)
        retVal = _scriptColor.setFloatProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propScript_size.name)
        {
            setScriptSize(pState);
            retVal = 1;
        }
    }

    return retVal;
}

int CScript::getFloatProperty(const char* ppName, double& pState) const
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "script."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getFloatProperty(pName, pState);
    if (retVal == -1)
        retVal = _scriptColor.getFloatProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propScript_size.name)
        {
            pState = _scriptSize;
            retVal = 1;
        }
    }

    return retVal;
}

int CScript::setStringProperty(const char* ppName, const char* pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "script."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::setStringProperty(pName, pState);
    if (retVal == -1)
        retVal = scriptObject->setStringProperty(pName, pState);
    if (retVal == -1)
    {
    }
    return retVal;
}

int CScript::getStringProperty(const char* ppName, std::string& pState) const
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "script."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getStringProperty(pName, pState);
    if (retVal == -1)
        retVal = scriptObject->getStringProperty(pName, pState);
    if (retVal == -1)
    {
    }
    return retVal;
}

int CScript::setColorProperty(const char* ppName, const float* pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "script."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::setColorProperty(pName, pState);
    if (retVal == -1)
        retVal = _scriptColor.setColorProperty(pName, pState);
    if (retVal != -1)
    {
    }
    return retVal;
}

int CScript::getColorProperty(const char* ppName, float* pState) const
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "script."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getColorProperty(pName, pState);
    if (retVal == -1)
        retVal = _scriptColor.getColorProperty(pName, pState);
    if (retVal != -1)
    {
    }
    return retVal;
}

int CScript::getPropertyName(int& index, std::string& pName, std::string& appartenance)
{
    int retVal = CSceneObject::getPropertyName(index, pName, appartenance);
    if (retVal == -1)
    {
        appartenance += ".script";
        retVal = _scriptColor.getPropertyName(index, pName);
    }
    if (retVal == -1)
        retVal = scriptObject->getPropertyName(index, pName, nullptr);
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_script.size(); i++)
        {
            if ((pName.size() == 0) || utils::startsWith(allProps_script[i].name, pName.c_str()))
            {
                index--;
                if (index == -1)
                {
                    pName = allProps_script[i].name;
                    retVal = 1;
                    break;
                }
            }
        }
    }
    return retVal;
}

int CScript::getPropertyName_static(int& index, std::string& pName, std::string& appartenance)
{
    int retVal = CSceneObject::getPropertyName_bstatic(index, pName, appartenance);
    if (retVal == -1)
    {
        appartenance += ".script";
        retVal = CColorObject::getPropertyName_static(index, pName, 1 + 4 + 8, "");
    }
    if (retVal == -1)
        retVal = CScriptObject::getPropertyName_static(index, pName, nullptr);
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_script.size(); i++)
        {
            if ((pName.size() == 0) || utils::startsWith(allProps_script[i].name, pName.c_str()))
            {
                index--;
                if (index == -1)
                {
                    pName = allProps_script[i].name;
                    retVal = 1;
                    break;
                }
            }
        }
    }
    return retVal;
}

int CScript::getPropertyInfo(const char* ppName, int& info, std::string& infoTxt)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "script."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getPropertyInfo(pName, info, infoTxt);
    if (retVal == -1)
        retVal = _scriptColor.getPropertyInfo(pName, info, infoTxt);
    if (retVal == -1)
        retVal = scriptObject->getPropertyInfo(pName, info, infoTxt, false);
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_script.size(); i++)
        {
            if (strcmp(allProps_script[i].name, pName) == 0)
            {
                retVal = allProps_script[i].type;
                info = allProps_script[i].flags;
                if ((infoTxt == "") && (strcmp(allProps_script[i].infoTxt, "") != 0))
                    infoTxt = allProps_script[i].infoTxt;
                else
                    infoTxt = allProps_script[i].shortInfoTxt;
                break;
            }
        }
    }
    return retVal;
}

int CScript::getPropertyInfo_static(const char* ppName, int& info, std::string& infoTxt)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "script."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getPropertyInfo_bstatic(pName, info, infoTxt);
    if (retVal == -1)
        retVal = CColorObject::getPropertyInfo_static(pName, info, infoTxt, 1 + 4 + 8, "");
    if (retVal == -1)
        retVal = CScriptObject::getPropertyInfo_static(pName, info, infoTxt, false);
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_script.size(); i++)
        {
            if (strcmp(allProps_script[i].name, pName) == 0)
            {
                retVal = allProps_script[i].type;
                info = allProps_script[i].flags;
                if ((infoTxt == "") && (strcmp(allProps_script[i].infoTxt, "") != 0))
                    infoTxt = allProps_script[i].infoTxt;
                else
                    infoTxt = allProps_script[i].shortInfoTxt;
                break;
            }
        }
    }
    return retVal;
}
