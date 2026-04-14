#include <sceneContainer.h>
#include <app.h>
#include <tt.h>
#include <utils.h>
#include <interfaceStackString.h>
#include <interfaceStackInteger.h>
#include <vDateTime.h>
#include <simFlavor.h>
#ifdef SIM_WITH_GUI
#include <rendering.h>
#include <guiApp.h>
#endif

long long int CSceneContainer::_eventSeq = 0;

CSceneContainer::CSceneContainer()
{
    TRACE_INTERNAL;
    customAppData_volatile.setItemsAreVolatile();
    scene = nullptr;
    _sessionId = utils::generateUniqueAlphaNumericString();
    pluginContainer = nullptr;
    codeEditorInfos = nullptr;
    customObjects = nullptr;
    copyBuffer = nullptr;
    sandboxScript = nullptr;
    addOnScriptContainer = nullptr;
    persistentDataContainer_old = nullptr;
    interfaceStackContainer = nullptr;
    scriptCustomFuncAndVarContainer = nullptr;
    simulatorMessageQueue = nullptr;
    calcInfo = nullptr;
    customAppData_old = nullptr;
    moduleMenuItemContainer = nullptr;
#ifdef SIM_WITH_GUI
    globalGuiTextureCont = nullptr;
    serialPortContainer = nullptr;
#endif
    _currentSceneIndex = -1;
    App::scene = nullptr;
    _eventsEnabled = true;
    _eventSeq = 0;
    //_eventMutex.setName("eventMutex");
}

CSceneContainer::~CSceneContainer()
{
}

bool CSceneContainer::switchToScene(int sceneIndex)
{ // SIM THREAD only
    TRACE_INTERNAL;
    if (getCurrentSceneIndex() == sceneIndex)
        return (true); // we already have this instance!
    if (!isSceneSwitchingLocked())
        return (_switchToScene(sceneIndex));
    return (false);
}

void CSceneContainer::setModificationFlag(int bitMask)
{
    _modificationFlags |= bitMask;
}

int CSceneContainer::getModificationFlags(bool clearTheFlagsAfter)
{
#ifdef SIM_WITH_GUI
    if (GuiApp::getEditModeType() != NO_EDIT_MODE)
        _modificationFlags |= 128;
#endif
    std::vector<long long int> currentUniqueIdsOfSel;
    for (size_t i = 0; i < scene->sceneObjects->getSelectionCount(); i++)
    {
        CSceneObject* it = scene->sceneObjects->getObjectFromHandle(
            scene->sceneObjects->getObjectHandleFromSelectionIndex(i));
        if (it != nullptr)
            currentUniqueIdsOfSel.push_back(it->getObjectUid());
    }
    if (currentUniqueIdsOfSel.size() == _uniqueIdsOfSelectionSinceLastTimeGetAndClearModificationFlagsWasCalled.size())
    {
        for (size_t i = 0; i < currentUniqueIdsOfSel.size(); i++)
        {
            if (currentUniqueIdsOfSel[i] != _uniqueIdsOfSelectionSinceLastTimeGetAndClearModificationFlagsWasCalled[i])
            {
                _modificationFlags |= 512; // selection state changed
                break;
            }
        }
    }
    else
        _modificationFlags |= 512; // selection state changed

    _uniqueIdsOfSelectionSinceLastTimeGetAndClearModificationFlagsWasCalled.assign(currentUniqueIdsOfSel.begin(),
                                                                                   currentUniqueIdsOfSel.end());

    int retVal = _modificationFlags;
    if (clearTheFlagsAfter)
        _modificationFlags = 0;
    return (retVal);
}

int CSceneContainer::createNewScene()
{
    TRACE_INTERNAL;

    // Inform scripts about future switch to new scene (only if there is already at least one scene):
    if (scene != nullptr)
        callScripts(sim_syscb_beforeinstanceswitch, nullptr, nullptr);

    int oldSceneUiqueId = -1;

    // Inform plugins about future switch to new scene (only if there is already at least one scene):
    if (scene != nullptr)
    {
        oldSceneUiqueId = scene->environment->getSceneUniqueID();
        int pluginData[4] = {_currentSceneIndex, CEnvironment::getNextSceneUniqueId(), oldSceneUiqueId, 0};
        pluginContainer->sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceabouttoswitch, pluginData);
    }

#ifdef SIM_WITH_GUI
    // Inform UI about new scene creation:
    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId = INSTANCE_ABOUT_TO_BE_CREATED_UITHREADCMD;
    GuiApp::uiThread->executeCommandViaUiThread(&cmdIn, &cmdOut);
#endif

    if (scene != nullptr)
        scene->removeScene_oldIk();

    // Create new scene and switch to it:
    CScene* w = new CScene();
    _currentSceneIndex = int(_scenes.size());
    _scenes.push_back(w);
    scene = w;
    App::scene = w;
    scene->initializeScene();

    // Inform scripts about performed switch to new scene:
    App::pushGenesisEvents();

    callScripts(sim_syscb_afterinstanceswitch, nullptr, nullptr);

    // Inform plugins about performed switch to new scene:
    int dat[4] = {getCurrentSceneIndex(), scene->environment->getSceneUniqueID(), oldSceneUiqueId, 0};
    pluginContainer->sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceswitch, dat);
    setModificationFlag(64); // instance switched

#ifdef SIM_WITH_GUI
        // Inform UI about performed switch to new scene:
    cmdIn.cmdId = INSTANCE_WAS_JUST_CREATED_UITHREADCMD;
    GuiApp::uiThread->executeCommandViaUiThread(&cmdIn, &cmdOut);
#endif
    scene->rebuildScene_oldIk();

    return (_currentSceneIndex);
}

int CSceneContainer::getCurrentSceneIndex() const
{
    return (_currentSceneIndex);
}

int CSceneContainer::destroyCurrentScene()
{
    TRACE_INTERNAL;

    if ((_currentSceneIndex == -1) || (scene == nullptr))
        return (-1);

    int nextSceneIndex = -1;

    int oldSceneUiqueId = -1;
    if (_scenes.size() > 1)
    {
        nextSceneIndex = _currentSceneIndex;
        if (nextSceneIndex >= int(_scenes.size()) - 1)
            nextSceneIndex = int(_scenes.size()) - 2;

        // Inform scripts about future scene switch:
        callScripts(sim_syscb_beforeinstanceswitch, nullptr, nullptr);

        oldSceneUiqueId = scene->environment->getSceneUniqueID();
        // Inform plugins about future scene switch:
        int pluginData[4] = {-1, _scenes[nextSceneIndex]->environment->getSceneUniqueID(), oldSceneUiqueId, 0};
        pluginContainer->sendEventCallbackMessageToAllPlugins(
            sim_message_eventcallback_instanceabouttoswitch, pluginData);
    }

    // Empty current scene:
    CScene* w = scene;
    w->clearScene(true);

#ifdef SIM_WITH_GUI
    // Inform UI about future scene destruction:
    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId = INSTANCE_ABOUT_TO_BE_DESTROYED_UITHREADCMD;
    cmdIn.intParams.push_back(_currentSceneIndex);
    GuiApp::uiThread->executeCommandViaUiThread(&cmdIn, &cmdOut);
#endif

    scene->removeScene_oldIk();

    // Destroy current scene:
    scene = nullptr;
    App::scene = nullptr;
    w->deleteScene();
    delete w;
    _scenes.erase(_scenes.begin() + _currentSceneIndex);
    _currentSceneIndex = -1;

    if (nextSceneIndex != -1)
    {
        // switch to another scene:
        _currentSceneIndex = nextSceneIndex;
        scene = _scenes[_currentSceneIndex];
        App::scene = scene;

        // Inform scripts about performed scene switch:
        App::pushGenesisEvents();

        callScripts(sim_syscb_afterinstanceswitch, nullptr, nullptr);

        // Inform plugins about performed scene switch:
        int pluginData[4] = {_currentSceneIndex, scene->environment->getSceneUniqueID(), oldSceneUiqueId, 0};
        pluginContainer->sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceswitch, pluginData);
        setModificationFlag(64); // instance switched

#ifdef SIM_WITH_GUI
        // Inform UI about performed scene switch:
        cmdIn.cmdId = INSTANCE_HAS_CHANGE_UITHREADCMD;
        cmdIn.intParams.clear();
        cmdIn.intParams.push_back(_currentSceneIndex);
        GuiApp::uiThread->executeCommandViaUiThread(&cmdIn, &cmdOut);
#endif
        scene->rebuildScene_oldIk();
    }

    return (_currentSceneIndex);
}

int CSceneContainer::getSceneCount() const
{
    return (int(_scenes.size()));
}

void CSceneContainer::initialize()
{
    TRACE_INTERNAL;

    simulatorMessageQueue = new CSimulatorMessageQueue();
    customObjects = new CustomObjectContainer(sim_handle_app);
    copyBuffer = new CCopyBuffer();
    moduleMenuItemContainer = new CModuleMenuItemContainer();
#ifdef SIM_WITH_GUI
    globalGuiTextureCont = new CGlobalGuiTextureContainer();
    serialPortContainer = new CSerialPortContainer();
#endif

    persistentDataContainer_old = new CPersistentDataContainer("persistentData.dat"); // old
    interfaceStackContainer = new CInterfaceStackContainer();
    scriptCustomFuncAndVarContainer = new CScriptCustomFuncAndVarContainer();
    customAppData_old = new CCustomData_old();
    calcInfo = new CCalculationInfo();
    codeEditorInfos = new CCodeEditorInfos();
    pluginContainer = new CPluginContainer();
    addOnScriptContainer = new CAddOnScriptContainer();

    _events = new CCbor();

    createNewScene();
}

void CSceneContainer::deinitialize()
{
    TRACE_INTERNAL;
    delete _events;

    copyBuffer->clearBuffer();
    while (_scenes.size() != 0)
        destroyCurrentScene();

    //    delete sandboxScript; // done elsewhere!
    delete addOnScriptContainer;
    delete pluginContainer;
    delete codeEditorInfos;
    delete customAppData_old;
    delete scriptCustomFuncAndVarContainer;
    delete interfaceStackContainer;
    delete persistentDataContainer_old;
#ifdef SIM_WITH_GUI
    delete globalGuiTextureCont;
    delete serialPortContainer;
#endif
    delete moduleMenuItemContainer;
    delete copyBuffer;
    delete customObjects;
    delete simulatorMessageQueue;
    delete calcInfo;
}

bool CSceneContainer::_switchToScene(int newSceneIndex)
{
    TRACE_INTERNAL;
    if ((newSceneIndex < 0) || (newSceneIndex >= int(_scenes.size())))
        return (false);
    if (_currentSceneIndex == newSceneIndex)
        return (true);
    if (isSceneSwitchingLocked())
        return (false);

    // Inform scripts about future scene switch:
    callScripts(sim_syscb_beforeinstanceswitch, nullptr, nullptr);

    // Inform plugins about future scene switch:
    int oldSceneUiqueId = scene->environment->getSceneUniqueID();
    int pluginData[4] = {_currentSceneIndex, _scenes[newSceneIndex]->environment->getSceneUniqueID(), oldSceneUiqueId, 0};
    pluginContainer->sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceabouttoswitch, pluginData);

#ifdef SIM_WITH_GUI
    // Inform UI about future scene switch:
    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId = INSTANCE_ABOUT_TO_CHANGE_UITHREADCMD;
    cmdIn.intParams.push_back(newSceneIndex);
    GuiApp::uiThread->executeCommandViaUiThread(&cmdIn, &cmdOut);

    scene->pageContainer->clearAllLastMouseDownViewIndex();
#endif

    scene->removeScene_oldIk();

    // Switch scenes:
    _currentSceneIndex = newSceneIndex;
    scene = _scenes[_currentSceneIndex];
    App::scene = scene;

    // Inform scripts about performed scene switch:
    App::pushGenesisEvents();

    callScripts(sim_syscb_afterinstanceswitch, nullptr, nullptr);

    // Inform plugins about performed scene switch:
    pluginData[0] = _currentSceneIndex;
    pluginData[1] = scene->environment->getSceneUniqueID();
    pluginData[2] = oldSceneUiqueId;
    pluginContainer->sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceswitch, pluginData);
    setModificationFlag(64); // instance switched

#ifdef SIM_WITH_GUI
    // Inform UI about performed scene switch:
    cmdIn.cmdId = INSTANCE_HAS_CHANGE_UITHREADCMD;
    cmdIn.intParams.clear();
    cmdIn.intParams.push_back(_currentSceneIndex);
    GuiApp::uiThread->executeCommandViaUiThread(&cmdIn, &cmdOut);
#endif

    scene->rebuildScene_oldIk();

    return (true);
}

bool CSceneContainer::isSceneSwitchingLocked() const
{
    if (scene != nullptr)
        return (false);
    if (!scene->simulation->isSimulationStopped())
        return (true);
#ifdef SIM_WITH_GUI
    if (GuiApp::getEditModeType() != NO_EDIT_MODE)
        return (true);
    if (GuiApp::mainWindow != nullptr)
    {
        if (GuiApp::mainWindow->oglSurface->isViewSelectionActive() ||
            GuiApp::mainWindow->oglSurface->isPageSelectionActive())
            return (true);
    }
#endif
    return (false);
}

void CSceneContainer::getAllSceneNames(std::vector<std::string>& l) const
{
    l.clear();
    for (size_t i = 0; i < _scenes.size(); i++)
        l.push_back(VVarious::splitPath_fileBase(_scenes[i]->environment->getScenePathAndName().c_str()));
}

CDetachedScript* CSceneContainer::getDetachedScriptFromHandle(int scriptHandle) const
{
    CDetachedScript* retVal = nullptr;
    if (scene != nullptr)
        retVal = scene->getDetachedScriptFromHandle(scriptHandle);
    if ((retVal == nullptr) && (addOnScriptContainer != nullptr))
        retVal = addOnScriptContainer->getAddOnFromHandle(scriptHandle);
    if ((retVal == nullptr) && (sandboxScript != nullptr) && (sandboxScript->getScriptHandle() == scriptHandle))
        retVal = sandboxScript;
    return (retVal);
}

CDetachedScript* CSceneContainer::getDetachedScriptFromUid(int uid) const
{
    CDetachedScript* retVal = nullptr;
    if (scene != nullptr)
        retVal = scene->getDetachedScriptFromUid(uid);
    if ((retVal == nullptr) && (addOnScriptContainer != nullptr))
        retVal = addOnScriptContainer->getAddOnFromUid(uid);
    if ((retVal == nullptr) && (sandboxScript != nullptr) && (sandboxScript->getScriptUid() == uid))
        retVal = sandboxScript;
    return (retVal);
}

int CSceneContainer::getSysFuncAndHookCnt(int sysCall) const
{
    int retVal = scene->sceneObjects->getSysFuncAndHookCnt(sysCall);
    retVal += addOnScriptContainer->getSysFuncAndHookCnt(sysCall);
    if (sandboxScript != nullptr)
    {
        for (size_t i = 0; i < 3; i++)
            retVal += sandboxScript->getFuncAndHookCnt(sysCall, i);
    }
    return (retVal);
}

void CSceneContainer::getActiveScripts(std::vector<CDetachedScript*>& scripts, bool reverse /*= false*/, bool alsoLegacyScripts /*= false*/) const
{
    TRACE_INTERNAL;
    if (reverse)
    {
        if ((sandboxScript != nullptr) && (sandboxScript->getScriptState() == CDetachedScript::scriptState_initialized))
            scripts.push_back(sandboxScript);
        addOnScriptContainer->getActiveScripts(scripts);
        if (scene != nullptr)
            scene->getActiveScripts(scripts, reverse, alsoLegacyScripts);
    }
    else
    {
        if (scene != nullptr)
            scene->getActiveScripts(scripts, reverse, alsoLegacyScripts);
        addOnScriptContainer->getActiveScripts(scripts);
        if ((sandboxScript != nullptr) && (sandboxScript->getScriptState() == CDetachedScript::scriptState_initialized))
            scripts.push_back(sandboxScript);
    }
}

void CSceneContainer::callScripts(int callType, CInterfaceStack* inStack, CInterfaceStack* outStack, CSceneObject* objectBranch /*=nullptr*/, int scriptToExclude /*=-1*/)
{
    TRACE_INTERNAL;
    bool doNotInterrupt = !CDetachedScript::isSystemCallbackInterruptible(callType);
    if (CDetachedScript::isSystemCallbackInReverseOrder(callType))
    { // reverse order
        if ((sandboxScript != nullptr) && sandboxScript->hasSystemFunctionOrHook(callType))
        {
            if (scriptToExclude != sandboxScript->getScriptHandle())
                sandboxScript->systemCallScript(callType, inStack, outStack);
        }
        if (doNotInterrupt || (outStack == nullptr) || (outStack->getStackSize() == 0))
            addOnScriptContainer->callScripts(callType, inStack, outStack, scriptToExclude);
        if (scene != nullptr)
        {
            if (doNotInterrupt || (outStack == nullptr) || (outStack->getStackSize() == 0))
                scene->callScripts(callType, inStack, outStack, objectBranch, scriptToExclude);
        }
    }
    else
    { // regular order, from unimportant, to most important
        if (scene != nullptr)
            scene->callScripts(callType, inStack, outStack, objectBranch, scriptToExclude);
        if (doNotInterrupt || (outStack == nullptr) || (outStack->getStackSize() == 0))
            addOnScriptContainer->callScripts(callType, inStack, outStack, scriptToExclude);
        if (doNotInterrupt || (outStack == nullptr) || (outStack->getStackSize() == 0))
        {
            if ((sandboxScript != nullptr) && sandboxScript->hasSystemFunctionOrHook(callType))
            {
                if (scriptToExclude != sandboxScript->getScriptHandle())
                    sandboxScript->systemCallScript(callType, inStack, outStack);
            }
        }
    }
}

void CSceneContainer::broadcastMsg(CInterfaceStack* inStack, int emittingScriptHandle, int options)
{
    TRACE_INTERNAL;
    callScripts(sim_syscb_msg, inStack, nullptr, nullptr, emittingScriptHandle);
}

bool CSceneContainer::shouldTemporarilySuspendMainScript()
{
    TRACE_INTERNAL;
    bool retVal = false;

    // Old plugins:
    int data[4] = {0, 0, 0, 0};
    int rtVal[4] = {-1, -1, -1, -1};
    pluginContainer->sendEventCallbackMessageToAllPlugins_old(
        sim_message_eventcallback_mainscriptabouttobecalled, data, nullptr, rtVal);
    if (rtVal[0] != -1)
        retVal = true;

    // New plugins:
    int dat = -1;
    pluginContainer->sendEventCallbackMessageToAllPlugins(
        sim_message_eventcallback_mainscriptabouttobecalled, &dat, nullptr, true);
    if (dat != -1)
        retVal = true;

    // simulation scripts & customization scripts:
    if (scene->sceneObjects->shouldTemporarilySuspendMainScript())
        retVal = true;

    // Add-on scripts:
    if (addOnScriptContainer->shouldTemporarilySuspendMainScript())
        retVal = true;

    // Sandbox script:
    if ((sandboxScript != nullptr) && sandboxScript->shouldTemporarilySuspendMainScript())
        retVal = true;

    return (retVal);
}

void CSceneContainer::pushSceneObjectRemoveEvent(const CSceneObject* object)
{
    if (getEventsEnabled())
    {
        _createGeneralEvent(EVENTTYPE_OBJECTREMOVED, object->getObjectHandle(), object->getObjectUid(), nullptr, nullptr, false);
        pushEvent();
    }
}

CCbor* CSceneContainer::createSceneObjectAddEvent(const CSceneObject* object)
{
    return (_createGeneralEvent(EVENTTYPE_OBJECTADDED, object->getObjectHandle(), object->getObjectUid(), nullptr,
                                nullptr, false));
}

CCbor* CSceneContainer::createSceneObjectChangedEvent(long long int sceneObjectHandle, bool isCommonObjectData, const char* fieldName, bool mergeable)
{
    CSceneObject* object = scene->sceneObjects->getObjectFromHandle(sceneObjectHandle);
    return (createSceneObjectChangedEvent(object, isCommonObjectData, fieldName, mergeable));
}

CCbor* CSceneContainer::createObjectChangedEvent(long long int objectHandle, const char* fieldName, bool mergeable)
{
    return _createGeneralEvent(EVENTTYPE_OBJECTCHANGED, objectHandle, objectHandle, nullptr, fieldName, mergeable);
}

CCbor* CSceneContainer::createSceneObjectChangedEvent(const CSceneObject* object, bool isCommonObjectData,
                                                      const char* fieldName, bool mergeable)
{
    const char* ot = nullptr;
    std::string objType;
    if (!isCommonObjectData)
    {
        switch (object->getObjectType())
        {
        case sim_sceneobject_shape:
            objType = "shape";
            break;
        case sim_sceneobject_joint:
            objType = "joint";
            break;
        case sim_sceneobject_graph:
            objType = "graph";
            break;
        case sim_sceneobject_camera:
            objType = "camera";
            break;
        case sim_sceneobject_dummy:
            objType = "dummy";
            break;
        case sim_sceneobject_proximitysensor:
            objType = "proxSensor";
            break;
        case sim_sceneobject_path:
            objType = "path";
            break;
        case sim_sceneobject_visionsensor:
            objType = "visionSensor";
            break;
        case sim_sceneobject_mill:
            objType = "mill";
            break;
        case sim_sceneobject_forcesensor:
            objType = "forceSensor";
            break;
        case sim_sceneobject_light:
            objType = "light";
            break;
        case sim_sceneobject_mirror:
            objType = "mirror";
            break;
        case sim_sceneobject_octree:
            objType = "octree";
            break;
        case sim_sceneobject_pointcloud:
            objType = "pointCloud";
            break;
        }
        ot = objType.c_str();
    }
    return (_createGeneralEvent(EVENTTYPE_OBJECTCHANGED, object->getObjectHandle(), object->getObjectUid(), ot, fieldName, mergeable));
}

CCbor* CSceneContainer::createNakedEvent(const char* event, long long int handle, long long int uid, bool mergeable)
{ // has no 'data' field
    return (_createGeneralEvent(event, handle, uid, nullptr, nullptr, mergeable, false));
}

CCbor* CSceneContainer::createEvent(const char* event, long long int handle, long long int uid, const char* fieldName, bool mergeable)
{
    return (_createGeneralEvent(event, handle, uid, nullptr, fieldName, mergeable));
}

void CSceneContainer::pushEvent()
{
    _events->pushEvent();
    _eventMutex.unlock();
}

CCbor* CSceneContainer::_createGeneralEvent(const char* event, long long int objectHandle, long long int uid, const char* objType, const char* fieldName, bool mergeable, bool openDataField /*=true*/)
{
    CCbor* retVal = nullptr;
    if (getEventsEnabled())
    {
        _eventMutex.lock("CSceneContainer::_createGeneralEvent");
        _events->createEvent(event, fieldName, objType, objectHandle, uid, mergeable, openDataField);
        retVal = _events;
    }
    return (retVal);
}

bool CSceneContainer::getEventsEnabled() const
{
    return _eventsEnabled;
}

std::string CSceneContainer::getSessionId() const
{
    return _sessionId;
}

void CSceneContainer::getGenesisEvents(std::vector<unsigned char>* genesisEvents, CInterfaceStack* stack)
{
    _eventMutex.lock("CSceneContainer::getGenesisEvents");
    dispatchEvents(); // Dispatch events in the pipeline
    App::pushGenesisEvents();
    _events->finalizeEvents(_eventSeq, false);
    _events->swapWithEmptyBuffer(genesisEvents);
    _eventMutex.unlock();
}

void CSceneContainer::dispatchEvents()
{
    if (VThread::isSimThread())
    {
        // Push the last changes that are not immediate:
        scene->drawingCont->pushAppendNewPointEvents();
        _eventMutex.lock("CSceneContainer::dispatchEvents");
        if (_events->getEventCnt() > 0)
        {
            _eventMutex.unlock();
            CCbor* ev_ = _createGeneralEvent(EVENTTYPE_MSGDISPATCHTIME, -1, -1, nullptr, nullptr, false);
            ev_->appendKeyInt64("time", VDateTime::getUnixTimeInMs());
            pushEvent();
            _eventMutex.lock("CSceneContainer::dispatchEvents");

            std::vector<SEventInf> _eventInfos;
            _eventSeq = _events->finalizeEvents(_eventSeq, true, &_eventInfos);
            std::vector<unsigned char> ev;
            _events->swapWithEmptyBuffer(&ev);
            int evCnt = int(_events->getEventCnt());
            _eventMutex.unlock(); // below might lead to a deadlock if _eventMutex still locked

            int auxData[2];
            auxData[0] = evCnt;
            auxData[1] = int(ev.size());
            pluginContainer->sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_events, auxData, ev.data());
            if (getSysFuncAndHookCnt(sim_syscb_event) > 0)
            {
                CInterfaceStack* fullEventsStack = nullptr;

                std::vector<CDetachedScript*> scripts;
                getActiveScripts(scripts, false, true);
                for (size_t sc = 0; sc < scripts.size(); sc++)
                {
                    CDetachedScript* script = scripts[sc];
                    if (script->hasSystemFunctionOrHook(sim_syscb_event))
                    {
                        std::vector<unsigned char> ew;
                        if (script->prepareFilteredEventsBuffer(ev, _eventInfos, ew))
                        { // events are filtered
                            if (ew.size() > 2)
                            { // make sure the event array is not empty
                                CInterfaceStack* stack = interfaceStackContainer->createStack();
                                stack->pushBufferOntoStack((char*)ew.data(), ew.size());
                                script->systemCallScript(sim_syscb_event, stack, nullptr);
                                interfaceStackContainer->destroyStack(stack);
                            }
                        }
                        else
                        { // events are not filtered
                            if (fullEventsStack == nullptr)
                            {
                                fullEventsStack = interfaceStackContainer->createStack();
                                fullEventsStack->pushBufferOntoStack((char*)ev.data(), ev.size());
                            }
                            script->systemCallScript(sim_syscb_event, fullEventsStack, nullptr);
                        }
                    }
                }

                if (fullEventsStack != nullptr)
                    interfaceStackContainer->destroyStack(fullEventsStack);
            }
        }
        else
            _eventMutex.unlock();
    }
}

#ifdef SIM_WITH_GUI
void CSceneContainer::addMenu(VMenu* menu)
{ // GUI THREAD only
    TRACE_INTERNAL;
    bool enabled = (!isSceneSwitchingLocked()) && scene->simulation->isSimulationStopped() &&
                   (!GuiApp::mainWindow->oglSurface->isPageSelectionActive()) &&
                   (!GuiApp::mainWindow->oglSurface->isViewSelectionActive()) &&
                   (GuiApp::getEditModeType() == NO_EDIT_MODE);

    for (size_t i = 0; i < _scenes.size(); i++)
    {
        std::string txt = _scenes[i]->environment->getSceneName();
        if (txt == "")
            txt = "new scene";
        txt += tt::decorateString(" (scene ", utils::getIntString(false, int(i) + 1), ")");
        menu->appendMenuItem(enabled, _currentSceneIndex == int(i),
                             SWITCH_TOINSTANCEWITHTHUMBNAILSAVEINDEX0_GUIGUICMD + int(i), txt.c_str(), true);
    }
}

void CSceneContainer::keyPress(int key)
{
    TRACE_INTERNAL;
    if ((GuiApp::mainWindow != nullptr) && (key == CTRL_E_KEY))
    {
        if ((GuiApp::getMouseMode() & 0x00ff) == sim_navigation_camerashift)
            GuiApp::setMouseMode((GuiApp::getMouseMode() & 0xff00) | sim_navigation_objectshift);
        else
        {
            if ((GuiApp::getMouseMode() & 0x00ff) == sim_navigation_objectshift)
                GuiApp::setMouseMode((GuiApp::getMouseMode() & 0xff00) | sim_navigation_objectrotate);
            else
                GuiApp::setMouseMode((GuiApp::getMouseMode() & 0xff00) | sim_navigation_camerashift);
        }
    }
}

int CSceneContainer::getInstanceIndexOfASceneNotYetSaved(bool doNotIncludeCurrentScene)
{
    for (int i = 0; i < getSceneCount(); i++)
    {
        if ((!doNotIncludeCurrentScene) || (getCurrentSceneIndex() != i))
        {
            if (_scenes[i]->undoBufferContainer->isSceneSaveMaybeNeededFlagSet())
                return (i);
        }
    }
    return (-1);
}

void CSceneContainer::setInstanceIndexWithThumbnails(int index)
{ // GUI THREAD only
    TRACE_INTERNAL;
    App::appendSimulationThreadCommand(SWITCH_TOINSTANCEINDEX_GUITRIGGEREDCMD, index);
}

bool CSceneContainer::processGuiCommand(int commandID)
{ // GUI THREAD only. Return value is true if the command belonged to object edition menu and was executed
    TRACE_INTERNAL;

    if ((commandID >= SWITCH_TOINSTANCEWITHTHUMBNAILSAVEINDEX0_GUIGUICMD) &&
        (commandID <= SWITCH_TOINSTANCEWITHTHUMBNAILSAVEINDEX39_GUIGUICMD))
    {
        setInstanceIndexWithThumbnails(commandID - SWITCH_TOINSTANCEWITHTHUMBNAILSAVEINDEX0_GUIGUICMD);
        return (true);
    }
    return (false);
}

#endif

void CSceneContainer::instancePass()
{
    scene->instancePass();
    int auxData[4] = {getModificationFlags(true), 0, 0, 0};
    pluginContainer->sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instancepass, auxData);
}

void CSceneContainer::simulationAboutToStart()
{
    calcInfo->simulationAboutToStart();
    scene->simulationAboutToStart();
}

void CSceneContainer::simulationPaused()
{
    scene->simulationPaused();
}

void CSceneContainer::simulationAboutToResume()
{
    scene->simulationAboutToResume();
}

void CSceneContainer::simulationAboutToStep()
{
    calcInfo->simulationAboutToStep();
    scene->simulationAboutToStep();
}

void CSceneContainer::simulationAboutToEnd()
{
    scene->simulationAboutToEnd();
}

void CSceneContainer::simulationEnded(bool removeNewObjects)
{
    scene->simulationEnded(removeNewObjects);
    calcInfo->simulationEnded();
}

void CSceneContainer::announceObjectWillBeErased(const CSceneObject* object)
{
    scene->announceObjectWillBeErased(object);
}

void CSceneContainer::announceScriptWillBeErased(int scriptHandle, long long int scriptUid, bool simulationScript, bool sceneSwitchPersistentScript)
{
    // Inform plugins about this event:
    int pluginData[4] = {scriptHandle, int(scriptUid & 0xffffffff), int((scriptUid >> 32) & 0xffffffff), 0};
    pluginContainer->sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_scriptabouttobedestroyed, pluginData);

    scene->announceScriptWillBeErased(scriptHandle, simulationScript, sceneSwitchPersistentScript);
}

void CSceneContainer::announceScriptStateWillBeErased(int scriptHandle, long long int scriptUid, bool simulationScript, bool sceneSwitchPersistentScript)
{
    pluginContainer->announceScriptStateWillBeErased(scriptHandle, scriptUid);
    moduleMenuItemContainer->announceScriptStateWillBeErased(scriptHandle);
    scene->announceScriptStateWillBeErased(scriptHandle, simulationScript, sceneSwitchPersistentScript);
#ifdef SIM_WITH_GUI
    if (GuiApp::mainWindow != nullptr)
        GuiApp::mainWindow->announceScriptStateWillBeErased(scriptHandle, scriptUid);
#endif
    customObjects->announceScriptStateWillBeErased(scriptHandle);
}
