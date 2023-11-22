#include <worldContainer.h>
#include <app.h>
#include <tt.h>
#include <utils.h>
#include <interfaceStackString.h>
#include <interfaceStackInteger.h>
#ifdef SIM_WITH_GUI
    #include <rendering.h>
    #include <guiApp.h>
#endif

long long int CWorldContainer::_eventSeq=0;

CWorldContainer::CWorldContainer()
{
    TRACE_INTERNAL;
    currentWorld=nullptr;
    _sessionId=utils::generateUniqueAlphaNumericString();
    pluginContainer=nullptr;
    codeEditorInfos=nullptr;
    copyBuffer=nullptr;
    sandboxScript=nullptr;
    addOnScriptContainer=nullptr;
    persistentDataContainer=nullptr;
    interfaceStackContainer=nullptr;
    scriptCustomFuncAndVarContainer=nullptr;
    simulatorMessageQueue=nullptr;
    calcInfo=nullptr;
    customAppData_old=nullptr;
    moduleMenuItemContainer=nullptr;
#ifdef SIM_WITH_GUI
    globalGuiTextureCont=nullptr;
    serialPortContainer=nullptr;
#endif
    _currentWorldIndex=-1;
    App::currentWorld=nullptr;
    _eventsEnabled=true;
    _eventSeq=0;
    //_eventMutex.setName("eventMutex");
}

CWorldContainer::~CWorldContainer()
{
}

bool CWorldContainer::switchToWorld(int worldIndex)
{ // SIM THREAD only
    TRACE_INTERNAL;
    if (getCurrentWorldIndex()==worldIndex)
        return(true); // we already have this instance!
    if (!isWorldSwitchingLocked())
        return(_switchToWorld(worldIndex));
    return(false);
}

void CWorldContainer::setModificationFlag(int bitMask)
{
    _modificationFlags|=bitMask;
}

int CWorldContainer::getModificationFlags(bool clearTheFlagsAfter)
{
    #ifdef SIM_WITH_GUI
        if (GuiApp::getEditModeType()!=NO_EDIT_MODE)
            _modificationFlags|=128;
    #endif
    std::vector<long long int> currentUniqueIdsOfSel;
    for (size_t i=0;i<currentWorld->sceneObjects->getSelectionCount();i++)
    {
        CSceneObject* it=currentWorld->sceneObjects->getObjectFromHandle(currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
        if (it!=nullptr)
            currentUniqueIdsOfSel.push_back(it->getObjectUid());
    }
    if (currentUniqueIdsOfSel.size()==_uniqueIdsOfSelectionSinceLastTimeGetAndClearModificationFlagsWasCalled.size())
    {
        for (size_t i=0;i<currentUniqueIdsOfSel.size();i++)
        {
            if (currentUniqueIdsOfSel[i]!=_uniqueIdsOfSelectionSinceLastTimeGetAndClearModificationFlagsWasCalled[i])
            {
                _modificationFlags|=512; // selection state changed
                break;
            }
        }
    }
    else
        _modificationFlags|=512; // selection state changed

    _uniqueIdsOfSelectionSinceLastTimeGetAndClearModificationFlagsWasCalled.assign(currentUniqueIdsOfSel.begin(),currentUniqueIdsOfSel.end());

    int retVal=_modificationFlags;
    if (clearTheFlagsAfter)
        _modificationFlags=0;
    return(retVal);
}

int CWorldContainer::createNewWorld()
{
    TRACE_INTERNAL;

    // Inform scripts about future switch to new world (only if there is already at least one world):
    if (currentWorld!=nullptr)
        callScripts(sim_syscb_beforeinstanceswitch,nullptr,nullptr);

    // Inform plugins about future switch to new world (only if there is already at least one world):
    if (currentWorld!=nullptr)
    {
        int pluginData[4]={_currentWorldIndex,CEnvironment::getNextSceneUniqueId(),0,0};
        App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceabouttoswitch,pluginData);
    }

    #ifdef SIM_WITH_GUI
        // Inform UI about new world creation:
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=INSTANCE_ABOUT_TO_BE_CREATED_UITHREADCMD;
        GuiApp::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    #endif

    if (currentWorld!=nullptr)
        currentWorld->removeWorld_oldIk();

    // Create new world and switch to it:
    CWorld* w=new CWorld();
    _currentWorldIndex=int(_worlds.size());
    static int nextWorldHandle=0;
    w->setWorldHandle(nextWorldHandle++);
    _worlds.push_back(w);
    currentWorld=w;
    App::currentWorld=w;
    currentWorld->initializeWorld();

    // Inform scripts about performed switch to new world:
    pushGenesisEvents();

    callScripts(sim_syscb_afterinstanceswitch,nullptr,nullptr);

    // Inform plugins about performed switch to new world:
    int dat[4]={getCurrentWorldIndex(),currentWorld->environment->getSceneUniqueID(),0,0};
    App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceswitch,dat);
    setModificationFlag(64); // instance switched

    #ifdef SIM_WITH_GUI
        // Inform UI about performed switch to new world:
        cmdIn.cmdId=INSTANCE_WAS_JUST_CREATED_UITHREADCMD;
        GuiApp::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    #endif
    currentWorld->rebuildWorld_oldIk();

    return(_currentWorldIndex);
}

int CWorldContainer::getCurrentWorldIndex() const
{
    return(_currentWorldIndex);
}

int CWorldContainer::destroyCurrentWorld()
{
    TRACE_INTERNAL;

    if ( (_currentWorldIndex==-1)||(currentWorld==nullptr) )
        return(-1);

    int nextWorldIndex=-1;

    if (_worlds.size()>1)
    {
        nextWorldIndex=_currentWorldIndex;
        if (nextWorldIndex>=int(_worlds.size())-1)
            nextWorldIndex=int(_worlds.size())-2;

        // Inform scripts about future world switch:
        callScripts(sim_syscb_beforeinstanceswitch,nullptr,nullptr);

        // Inform plugins about future world switch:
        int pluginData[4]={-1,_worlds[nextWorldIndex]->environment->getSceneUniqueID(),0,0};
        App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceabouttoswitch,pluginData);
    }

    // Empty current scene:
    CWorld* w=currentWorld;
    w->clearScene(true);

#ifdef SIM_WITH_GUI
    // Inform UI about future world destruction:
    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId=INSTANCE_ABOUT_TO_BE_DESTROYED_UITHREADCMD;
    cmdIn.intParams.push_back(_currentWorldIndex);
    GuiApp::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
#endif

    currentWorld->removeWorld_oldIk();

    // Destroy current world:
    currentWorld=nullptr;
    App::currentWorld=nullptr;
    w->deleteWorld();
    delete w;
    _worlds.erase(_worlds.begin()+_currentWorldIndex);
    _currentWorldIndex=-1;

    if (nextWorldIndex!=-1)
    {
        // switch to another world:
        _currentWorldIndex=nextWorldIndex;
        currentWorld=_worlds[_currentWorldIndex];
        App::currentWorld=currentWorld;

        // Inform scripts about performed world switch:
        pushGenesisEvents();

        callScripts(sim_syscb_afterinstanceswitch,nullptr,nullptr);

        // Inform plugins about performed world switch:
        int pluginData[4]={_currentWorldIndex,currentWorld->environment->getSceneUniqueID(),0,0};
        App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceswitch,pluginData);
        setModificationFlag(64); // instance switched

#ifdef SIM_WITH_GUI
        // Inform UI about performed world switch:
        cmdIn.cmdId=INSTANCE_HAS_CHANGE_UITHREADCMD;
        cmdIn.intParams.clear();
        cmdIn.intParams.push_back(_currentWorldIndex);
        GuiApp::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
#endif
        currentWorld->rebuildWorld_oldIk();
    }

    return(_currentWorldIndex);
}

int CWorldContainer::getWorldCount() const
{
    return(int(_worlds.size()));
}

void CWorldContainer::initialize()
{
    TRACE_INTERNAL;

    simulatorMessageQueue=new CSimulatorMessageQueue();
    copyBuffer=new CCopyBuffer();
    moduleMenuItemContainer=new CModuleMenuItemContainer();
#ifdef SIM_WITH_GUI
    globalGuiTextureCont=new CGlobalGuiTextureContainer();
    serialPortContainer=new CSerialPortContainer();
#endif

    persistentDataContainer=new CPersistentDataContainer("persistentData.dat");
    interfaceStackContainer=new CInterfaceStackContainer();
    scriptCustomFuncAndVarContainer=new CScriptCustomFuncAndVarContainer();
    customAppData_old=new CCustomData_old();
    calcInfo=new CCalculationInfo();
    codeEditorInfos=new CCodeEditorInfos();
    pluginContainer=new CPluginContainer();
    addOnScriptContainer=new CAddOnScriptContainer();

    _events=new CCbor();

    createNewWorld();
}

void CWorldContainer::deinitialize()
{
    TRACE_INTERNAL;
    delete _events;

    copyBuffer->clearBuffer();
    while (_worlds.size()!=0)
        destroyCurrentWorld();

//    delete sandboxScript; // done elsewhere!
    delete addOnScriptContainer;
    delete pluginContainer;
    delete codeEditorInfos;
    delete customAppData_old;
    delete scriptCustomFuncAndVarContainer;
    delete interfaceStackContainer;
    delete persistentDataContainer;
#ifdef SIM_WITH_GUI
    delete globalGuiTextureCont;
    delete serialPortContainer;
#endif
    delete moduleMenuItemContainer;
    delete copyBuffer;
    delete simulatorMessageQueue;
    delete calcInfo;
}

bool CWorldContainer::_switchToWorld(int newWorldIndex)
{ 
    TRACE_INTERNAL;
    if ( (newWorldIndex<0)||(newWorldIndex>=int(_worlds.size())))
        return(false);
    if (_currentWorldIndex==newWorldIndex)
        return(true);
    if (isWorldSwitchingLocked())
        return(false);

    // Inform scripts about future world switch:
    callScripts(sim_syscb_beforeinstanceswitch,nullptr,nullptr);

    // Inform plugins about future world switch:
    int pluginData[4]={_currentWorldIndex,_worlds[newWorldIndex]->environment->getSceneUniqueID(),0,0};
    App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceabouttoswitch,pluginData);

#ifdef SIM_WITH_GUI
    // Inform UI about future world switch:
    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId=INSTANCE_ABOUT_TO_CHANGE_UITHREADCMD;
    cmdIn.intParams.push_back(newWorldIndex);
    GuiApp::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);

    currentWorld->pageContainer->clearAllLastMouseDownViewIndex();
#endif

    currentWorld->removeWorld_oldIk();

    // Switch worlds:
    _currentWorldIndex=newWorldIndex;
    currentWorld=_worlds[_currentWorldIndex];
    App::currentWorld=currentWorld;

    // Inform scripts about performed world switch:
    pushGenesisEvents();

    callScripts(sim_syscb_afterinstanceswitch,nullptr,nullptr);

    // Inform plugins about performed world switch:
    pluginData[0]=_currentWorldIndex;
    pluginData[1]=currentWorld->environment->getSceneUniqueID();
    App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceswitch,pluginData);
    setModificationFlag(64); // instance switched


#ifdef SIM_WITH_GUI
    // Inform UI about performed world switch:
    cmdIn.cmdId=INSTANCE_HAS_CHANGE_UITHREADCMD;
    cmdIn.intParams.clear();
    cmdIn.intParams.push_back(_currentWorldIndex);
    GuiApp::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
#endif

    currentWorld->rebuildWorld_oldIk();

    return(true);
}

bool CWorldContainer::isWorldSwitchingLocked() const
{
    if (currentWorld!=nullptr)
        return(false);
    if (!currentWorld->simulation->isSimulationStopped())
        return(true);
#ifdef SIM_WITH_GUI
    if (GuiApp::getEditModeType()!=NO_EDIT_MODE)
        return(true);
    if (GuiApp::mainWindow!=nullptr)
    {
        if (GuiApp::mainWindow->oglSurface->isViewSelectionActive()||GuiApp::mainWindow->oglSurface->isPageSelectionActive())
            return(true);
    }
#endif
    return(false);
}

void CWorldContainer::getAllSceneNames(std::vector<std::string>& l) const
{
    l.clear();
    for (size_t i=0;i<_worlds.size();i++)
        l.push_back(VVarious::splitPath_fileBase(_worlds[i]->mainSettings->getScenePathAndName().c_str()));
}

CScriptObject* CWorldContainer::getScriptFromHandle(int scriptHandle) const
{
    CScriptObject* retVal=currentWorld->embeddedScriptContainer->getScriptFromHandle(scriptHandle);
    if (retVal==nullptr)
    {
        retVal=addOnScriptContainer->getAddOnFromID(scriptHandle);
        if ( (retVal==nullptr)&&(sandboxScript!=nullptr)&&(sandboxScript->getScriptHandle()==scriptHandle) )
            retVal=sandboxScript;
    }
    return(retVal);
}

int CWorldContainer::getSysFuncAndHookCnt(int sysCall) const
{
    int retVal=currentWorld->embeddedScriptContainer->getSysFuncAndHookCnt(sysCall);
    retVal+=addOnScriptContainer->getSysFuncAndHookCnt(sysCall);
    if (sandboxScript!=nullptr)
    {
        for (size_t i=0;i<3;i++)
            retVal+=sandboxScript->getFuncAndHookCnt(sysCall,i);
    }
    return(retVal);
}

void CWorldContainer::callScripts(int callType,CInterfaceStack* inStack,CInterfaceStack* outStack,CSceneObject* objectBranch/*=nullptr*/,int scriptToExclude/*=-1*/)
{
    TRACE_INTERNAL;
    bool doNotInterrupt=!CScriptObject::isSystemCallbackInterruptible(callType);
    if (CScriptObject::isSystemCallbackInReverseOrder(callType))
    { // reverse order
        if ( (sandboxScript!=nullptr)&&(sandboxScript->hasSystemFunctionOrHook(callType)||sandboxScript->getOldCallMode()) )
        {
            if (scriptToExclude!=sandboxScript->getScriptHandle())
                sandboxScript->systemCallScript(callType,inStack,outStack);
        }
        if ( doNotInterrupt||(outStack==nullptr)||(outStack->getStackSize()==0) )
            addOnScriptContainer->callScripts(callType,inStack,outStack,scriptToExclude);
        if ( doNotInterrupt||(outStack==nullptr)||(outStack->getStackSize()==0) )
            currentWorld->embeddedScriptContainer->callScripts(callType,inStack,outStack,objectBranch,scriptToExclude);

    }
    else
    { // regular order, from unimportant, to most important
        currentWorld->embeddedScriptContainer->callScripts(callType,inStack,outStack,objectBranch,scriptToExclude);
        if ( doNotInterrupt||(outStack==nullptr)||(outStack->getStackSize()==0) )
            addOnScriptContainer->callScripts(callType,inStack,outStack,scriptToExclude);
        if ( doNotInterrupt||(outStack==nullptr)||(outStack->getStackSize()==0) )
        {
            if ( (sandboxScript!=nullptr)&&(sandboxScript->hasSystemFunctionOrHook(callType)||sandboxScript->getOldCallMode()) )
            {
                if (scriptToExclude!=sandboxScript->getScriptHandle())
                    sandboxScript->systemCallScript(callType,inStack,outStack);
            }
        }
    }
}

void CWorldContainer::broadcastMsg(CInterfaceStack* inStack,int emittingScriptHandle,int options)
{
    TRACE_INTERNAL;
    callScripts(sim_syscb_msg,inStack,nullptr,nullptr,emittingScriptHandle);
}

bool CWorldContainer::shouldTemporarilySuspendMainScript()
{
    TRACE_INTERNAL;
    bool retVal=false;

    // Old plugins:
    int data[4]={0,0,0,0};
    int rtVal[4]={-1,-1,-1,-1};
    App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins_old(sim_message_eventcallback_mainscriptabouttobecalled,data,nullptr,rtVal);
    if (rtVal[0]!=-1)
        retVal=true;

    // New plugins:
    int dat=-1;
    App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_mainscriptabouttobecalled,&dat,nullptr,true);
    if (dat!=-1)
        retVal=true;

    // Child scripts & customization scripts:
    if (currentWorld->embeddedScriptContainer->shouldTemporarilySuspendMainScript())
        retVal=true;

    // Add-on scripts:
    if (addOnScriptContainer->shouldTemporarilySuspendMainScript())
        retVal=true;

    // Sandbox script:
    if ( (sandboxScript!=nullptr)&&sandboxScript->shouldTemporarilySuspendMainScript() )
        retVal=true;

    return(retVal);
}

void CWorldContainer::pushSceneObjectRemoveEvent(const CSceneObject* object)
{
    if (getEventsEnabled())
    {
        _createGeneralEvent(EVENTTYPE_OBJECTREMOVED,object->getObjectHandle(),object->getObjectUid(),nullptr,nullptr,false);
        pushEvent();
    }
}

CCbor* CWorldContainer::createSceneObjectAddEvent(const CSceneObject* object)
{
    return(_createGeneralEvent(EVENTTYPE_OBJECTADDED,object->getObjectHandle(),object->getObjectUid(),nullptr,nullptr,false));
}

CCbor* CWorldContainer::createSceneObjectChangedEvent(int sceneObjectHandle,bool isCommonObjectData,const char* fieldName,bool mergeable)
{
    CSceneObject* object=currentWorld->sceneObjects->getObjectFromHandle(sceneObjectHandle);
    return(createSceneObjectChangedEvent(object,isCommonObjectData,fieldName,mergeable));
}

CCbor* CWorldContainer::createSceneObjectChangedEvent(const CSceneObject* object,bool isCommonObjectData,const char* fieldName,bool mergeable)
{
    const char* ot=nullptr;
    std::string objType;
    if (!isCommonObjectData)
    {
        switch(object->getObjectType())
        {
            case sim_object_shape_type : objType="shape";
                break;
            case sim_object_joint_type : objType="joint";
                break;
            case sim_object_graph_type : objType="graph";
                break;
            case sim_object_camera_type : objType="camera";
                break;
            case sim_object_dummy_type : objType="dummy";
                break;
            case sim_object_proximitysensor_type : objType="proxSensor";
                break;
            case sim_object_path_type : objType="path";
                break;
            case sim_object_visionsensor_type : objType="visionSensor";
                break;
            case sim_object_mill_type : objType="mill";
                break;
            case sim_object_forcesensor_type : objType="forceSensor";
                break;
            case sim_object_light_type : objType="light";
                break;
            case sim_object_mirror_type : objType="mirror";
                break;
            case sim_object_octree_type : objType="octree";
                break;
            case sim_object_pointcloud_type : objType="pointCloud";
                break;
        }
        ot=objType.c_str();
    }
    return(_createGeneralEvent(EVENTTYPE_OBJECTCHANGED,object->getObjectHandle(),object->getObjectUid(),ot,fieldName,mergeable));
}

CCbor* CWorldContainer::createNakedEvent(const char* event,int handle,long long int uid,bool mergeable)
{
    return(_createGeneralEvent(event,handle,uid,nullptr,nullptr,mergeable,false));
}

CCbor* CWorldContainer::createEvent(const char* event,long long int uid,const char* fieldName,bool mergeable)
{
    return(_createGeneralEvent(event,-1,uid,nullptr,fieldName,mergeable));
}

void CWorldContainer::pushEvent()
{
    _events->pushEvent();
    _eventMutex.unlock();
}

CCbor* CWorldContainer::_createGeneralEvent(const char* event,int objectHandle,long long int uid,const char* objType,const char* fieldName,bool mergeable,bool openDataField/*=true*/)
{
    CCbor* retVal=nullptr;
    if (getEventsEnabled())
    {
        _eventMutex.lock("CWorldContainer::_createGeneralEvent");
        _events->createEvent(event,fieldName,objType,uid,objectHandle,mergeable,openDataField);
        retVal=_events;
    }
    return(retVal);
}

bool CWorldContainer::getEventsEnabled() const
{
    return(_eventsEnabled);
}

void CWorldContainer::getGenesisEvents(std::vector<unsigned char>* genesisEvents,CInterfaceStack* stack)
{
    _eventMutex.lock("CWorldContainer::getGenesisEvents");
    dispatchEvents(); // Dispatch events in the pipeline
    pushGenesisEvents();
    _events->finalizeEvents(_eventSeq,false);
    _events->swapWithEmptyBuffer(genesisEvents);
    _eventMutex.unlock();
}

void CWorldContainer::pushGenesisEvents()
{
    if (getEventsEnabled())
    {
        CCbor* ev=_createGeneralEvent(EVENTTYPE_GENESISBEGIN,-1,-1,nullptr,nullptr,false);
        pushEvent();

        ev=_createGeneralEvent(EVENTTYPE_APPSESSION,-1,-1,nullptr,nullptr,false);
        ev->appendKeyString("sessionId",_sessionId.c_str());
        ev->appendKeyInt("protocolVersion",2);
        std::string prod(SIM_PROGRAM_VERSION);
        prod+=".";
        prod+=std::to_string(SIM_PROGRAM_REVISION_NB);
        ev->appendKeyString("productVersion",prod.c_str());
        pushEvent();

        ev=_createGeneralEvent(EVENTTYPE_APPSETTINGSCHANGED,-1,-1,nullptr,nullptr,false);
        ev->appendKeyDouble("defaultTranslationStepSize",App::userSettings->getTranslationStepSize());
        ev->appendKeyDouble("defaultRotationStepSize",App::userSettings->getRotationStepSize());
        pushEvent();

        currentWorld->pushGenesisEvents();

        ev=_createGeneralEvent(EVENTTYPE_GENESISEND,-1,-1,nullptr,nullptr,false);
        pushEvent();
    }
}

void CWorldContainer::dispatchEvents()
{
    if (VThread::isSimThread())
    {
        // Push the last changes that are not immediate:
        currentWorld->drawingCont->pushAppendNewPointEvents();
        _eventMutex.lock("CWorldContainer::dispatchEvents");
        int evCnt=_events->getEventCnt();
        if (evCnt>0)
        {
            _eventSeq=_events->finalizeEvents(_eventSeq,true);
            std::vector<unsigned char> ev;
            _events->swapWithEmptyBuffer(&ev);
            CInterfaceStack* stack=interfaceStackContainer->createStack();
            stack->pushStringOntoStack((char*)ev.data(),ev.size());
            _eventMutex.unlock(); // below might lead to a deadlock if _eventMutex still locked
            int auxData[2];
            auxData[0]=evCnt;
            auxData[1]=int(ev.size());
            pluginContainer->sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_events,auxData,ev.data());
            if (getSysFuncAndHookCnt(sim_syscb_event)>0)
                callScripts(sim_syscb_event,stack,nullptr);
            interfaceStackContainer->destroyStack(stack);
        }
        else
            _eventMutex.unlock();
    }
}

#ifdef SIM_WITH_GUI
void CWorldContainer::addMenu(VMenu* menu)
{ // GUI THREAD only
    TRACE_INTERNAL;
    bool enabled=(!isWorldSwitchingLocked())&&currentWorld->simulation->isSimulationStopped()&&(!GuiApp::mainWindow->oglSurface->isPageSelectionActive())&&(!GuiApp::mainWindow->oglSurface->isViewSelectionActive())&&(GuiApp::getEditModeType()==NO_EDIT_MODE);

    for (size_t i=0;i<_worlds.size();i++)
    {
        std::string txt=_worlds[i]->mainSettings->getSceneName();
        if (txt=="")
            txt="new scene";
        txt+=tt::decorateString(" (scene ",utils::getIntString(false,int(i)+1),")");
        menu->appendMenuItem(enabled,_currentWorldIndex==int(i),SWITCH_TOINSTANCEWITHTHUMBNAILSAVEINDEX0_GUIGUICMD+int(i),txt.c_str(),true);
    }
}

void CWorldContainer::keyPress(int key)
{
    TRACE_INTERNAL;
    if ( (GuiApp::mainWindow!=nullptr)&&(key==CTRL_E_KEY) )
    {
        if ((GuiApp::getMouseMode()&0x00ff)==sim_navigation_camerashift)
            GuiApp::setMouseMode((GuiApp::getMouseMode()&0xff00)|sim_navigation_objectshift);
        else
        {
            if ((GuiApp::getMouseMode()&0x00ff)==sim_navigation_objectshift)
                GuiApp::setMouseMode((GuiApp::getMouseMode()&0xff00)|sim_navigation_objectrotate);
            else
                GuiApp::setMouseMode((GuiApp::getMouseMode()&0xff00)|sim_navigation_camerashift);
        }
    }
}

int CWorldContainer::getInstanceIndexOfASceneNotYetSaved(bool doNotIncludeCurrentScene)
{
    for (int i=0;i<getWorldCount();i++)
    {
        if ( (!doNotIncludeCurrentScene)||(getCurrentWorldIndex()!=i) )
        {
            if (_worlds[i]->undoBufferContainer->isSceneSaveMaybeNeededFlagSet())
                return(i);
        }
    }
    return(-1);
}

void CWorldContainer::setInstanceIndexWithThumbnails(int index)
{ // GUI THREAD only
    TRACE_INTERNAL;
    App::appendSimulationThreadCommand(SWITCH_TOINSTANCEINDEX_GUITRIGGEREDCMD,index);
}

bool CWorldContainer::processGuiCommand(int commandID)
{ // GUI THREAD only. Return value is true if the command belonged to object edition menu and was executed
    TRACE_INTERNAL;

    if ( (commandID>=SWITCH_TOINSTANCEWITHTHUMBNAILSAVEINDEX0_GUIGUICMD)&&(commandID<=SWITCH_TOINSTANCEWITHTHUMBNAILSAVEINDEX39_GUIGUICMD) )
    {
        setInstanceIndexWithThumbnails(commandID-SWITCH_TOINSTANCEWITHTHUMBNAILSAVEINDEX0_GUIGUICMD);
        return(true);
    }
    return(false);
}

#endif

void CWorldContainer::simulationAboutToStart()
{
    calcInfo->simulationAboutToStart();
    currentWorld->simulationAboutToStart();
}

void CWorldContainer::simulationPaused()
{
    currentWorld->simulationPaused();
}

void CWorldContainer::simulationAboutToResume()
{
    currentWorld->simulationAboutToResume();
}

void CWorldContainer::simulationAboutToStep()
{
    calcInfo->simulationAboutToStep();
    currentWorld->simulationAboutToStep();
}

void CWorldContainer::simulationAboutToEnd()
{
    currentWorld->simulationAboutToEnd();
}

void CWorldContainer::simulationEnded(bool removeNewObjects)
{
    currentWorld->simulationEnded(removeNewObjects);
    calcInfo->simulationEnded();
}

void CWorldContainer::announceScriptWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript)
{
    // Inform plugins about this event:
    int pluginData[4]={scriptHandle,0,0,0};
    App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_scriptdestroyed,pluginData);

    currentWorld->announceScriptWillBeErased(scriptHandle,simulationScript,sceneSwitchPersistentScript);
}

void CWorldContainer::announceScriptStateWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript)
{
    pluginContainer->announceScriptStateWillBeErased(scriptHandle);
    moduleMenuItemContainer->announceScriptStateWillBeErased(scriptHandle);
    currentWorld->announceScriptStateWillBeErased(scriptHandle,simulationScript,sceneSwitchPersistentScript);
#ifdef SIM_WITH_GUI
    if (GuiApp::mainWindow!=nullptr)
        GuiApp::mainWindow->announceScriptStateWillBeErased(scriptHandle);
#endif

}


