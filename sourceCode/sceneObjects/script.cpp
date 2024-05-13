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
    _commonInit(sim_scripttype_childscript, "", 0);
}

CScript::CScript(int scriptType, const char* text, int options)
{
    _commonInit(scriptType, text, options);
}

CScript::CScript(CScriptObject* scrObj)
{
    _commonInit(sim_scripttype_childscript, "", 0);
    delete scriptObject;
    scriptObject = scrObj;
    scriptObject->_sceneObjectScript = true;
}

void CScript::_commonInit(int scriptType, const char* text, int options)
{
    scriptObject = new CScriptObject(scriptType);
    scriptObject->_scriptText = text;
    scriptObject->_sceneObjectScript = true;
    _objectType = sim_object_script_type;
    _localObjectSpecialProperty = 0;
    _objectProperty |= sim_objectproperty_dontshowasinsidemodel;
    _scriptSize = 0.01;

    _visibilityLayer = SCRIPT_LAYER;
    _objectAlias = IDSOGL_SCRIPT;
    _objectName_old = IDSOGL_SCRIPT;
    _objectAltName_old = tt::getObjectAltNameFromObjectName(_objectName_old.c_str());

    _scriptColor.setDefaultValues();
    _scriptColor.setColor(1.0f, 1.0f, 1.0f, sim_colorcomponent_ambient_diffuse);

    computeBoundingBox();
}

CScript::~CScript()
{
    if (scriptObject != nullptr)
    {
        scriptObject->resetScript();
        CScriptObject::destroy(scriptObject, true, false);
        App::worldContainer->setModificationFlag(16384);
    }
}

void CScript::setObjectHandle(int newObjectHandle)
{
    CSceneObject::setObjectHandle(newObjectHandle);
    scriptObject->_scriptHandle = newObjectHandle;
    scriptObject->_objectHandleAttachedTo = newObjectHandle;
}

bool CScript::canDestroyNow(bool inSafePlace)
{ // overridden from CSceneObject
    bool retVal = false;
#ifdef SIM_WITH_GUI
    if (GuiApp::mainWindow != nullptr)
        GuiApp::mainWindow->codeEditorContainer->closeFromScriptUid(scriptObject->getScriptUid(), scriptObject->_previousEditionWindowPosAndSize, true);
#endif
    if ( (inSafePlace) && (scriptObject->_executionDepth == 0) )
    {
        if (scriptObject->_scriptState == CScriptObject::scriptState_initialized)
            scriptObject->systemCallScript(sim_syscb_cleanup, nullptr, nullptr);
        scriptObject->_scriptState = CScriptObject::scriptState_ended; // just in case
        retVal = true;
    }
    return retVal;
}

std::string CScript::getObjectTypeInfo() const
{
    return (IDSOGL_SCRIPT);
}

std::string CScript::getObjectTypeInfoExtended() const
{
    return (IDSOGL_SCRIPT);
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

void CScript::addSpecializedObjectEventData(CCbor *ev) const
{
    ev->openKeyMap("script");
    ev->appendKeyDouble("size", _scriptSize);
    ev->openKeyArray("colors");
    float c[9];
    _scriptColor.getColor(c, sim_colorcomponent_ambient_diffuse);
    _scriptColor.getColor(c + 3, sim_colorcomponent_specular);
    _scriptColor.getColor(c + 6, sim_colorcomponent_emission);
    ev->appendFloatArray(c, 9);
    ev->closeArrayOrMap(); // colors
    ev->closeArrayOrMap(); // script
}

CSceneObject *CScript::copyYourself()
{
    CScript *newScript = (CScript *)CSceneObject::copyYourself();

    _scriptColor.copyYourselfInto(&newScript->_scriptColor);
    newScript->_scriptSize = _scriptSize;

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

void CScript::performIkLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performIkLoadingMapping(map, loadingAmodel);
}

void CScript::performCollectionLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performCollectionLoadingMapping(map, loadingAmodel);
}

void CScript::performCollisionLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performCollisionLoadingMapping(map, loadingAmodel);
}

void CScript::performDistanceLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performDistanceLoadingMapping(map, loadingAmodel);
}

void CScript::performTextureObjectLoadingMapping(const std::map<int, int> *map)
{
    CSceneObject::performTextureObjectLoadingMapping(map);
}

void CScript::performDynMaterialObjectLoadingMapping(const std::map<int, int> *map)
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
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it
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

void CScript::serialize(CSer &ar)
{
    CSceneObject::serialize(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            ar.storeDataName("Sos");
            ar << _scriptSize;
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

void CScript::performObjectLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performObjectLoadingMapping(map, loadingAmodel);
}

void CScript::announceObjectWillBeErased(const CSceneObject *object, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    if ( (scriptObject != nullptr) && (object == this) )
    {
        App::worldContainer->announceScriptStateWillBeErased(_objectHandle, scriptObject->getScriptUid(), scriptObject->isSimulationScript(), scriptObject->isSceneSwitchPersistentScript());
        App::worldContainer->announceScriptWillBeErased(_objectHandle, scriptObject->getScriptUid(), scriptObject->isSimulationScript(), scriptObject->isSceneSwitchPersistentScript());
    }
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

CColorObject *CScript::getScriptColor()
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
            const char *cmd = "size";
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDouble(cmd, _scriptSize);
            App::worldContainer->pushEvent();
        }
    }
}

#ifdef SIM_WITH_GUI
void CScript::display(CViewableBase *renderingObject, int displayAttrib)
{
    displayScript(this, renderingObject, displayAttrib);
}
#endif
