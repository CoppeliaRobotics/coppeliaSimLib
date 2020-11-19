#include "worldContainer.h"
#include "app.h"
#include "pluginContainer.h"
#include "rendering.h"
#include "tt.h"

CWorldContainer::CWorldContainer()
{
    TRACE_INTERNAL;
    copyBuffer=nullptr;
    sandboxScript=nullptr;
    addOnScriptContainer=nullptr;
    persistentDataContainer=nullptr;
    interfaceStackContainer=nullptr;
    luaCustomFuncAndVarContainer=nullptr;
    simulatorMessageQueue=nullptr;
    calcInfo=nullptr;
    customAppData=nullptr;
#ifdef SIM_WITH_GUI
    globalGuiTextureCont=nullptr;
#endif
#ifdef SIM_WITH_SERIAL
    serialPortContainer=nullptr;
#endif
    _currentWorldIndex=-1;
    App::currentWorld=nullptr;
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
    if (App::getEditModeType()!=NO_EDIT_MODE)
        _modificationFlags|=128;
    std::vector<int> currentUniqueIdsOfSel;
    for (size_t i=0;i<currentWorld->sceneObjects->getSelectionCount();i++)
    {
        CSceneObject* it=currentWorld->sceneObjects->getObjectFromHandle(currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
        if (it!=nullptr)
            currentUniqueIdsOfSel.push_back(it->getUniqueID());
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
    {
        currentWorld->luaScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_beforeinstanceswitch,nullptr,nullptr,nullptr);
        addOnScriptContainer->handleAddOnScriptExecution(sim_syscb_beforeinstanceswitch,nullptr,nullptr);
        if (sandboxScript!=nullptr)
            sandboxScript->runSandboxScript(sim_syscb_beforeinstanceswitch,nullptr,nullptr);
    }

    // Inform plugins about future switch to new world (only if there is already at least one world):
    if (currentWorld!=nullptr)
    {
        int pluginData[4]={_currentWorldIndex,CEnvironment::getNextSceneUniqueId(),0,0};
        void* pluginReturnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceabouttoswitch,pluginData,nullptr,nullptr);
        delete[] (char*)pluginReturnVal;
    }

    // Disable online mode before switching
    if (currentWorld!=nullptr)
        currentWorld->simulation->setOnlineMode(false);

#ifdef SIM_WITH_GUI
    // Inform UI about new world creation:
    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId=INSTANCE_ABOUT_TO_BE_CREATED_UITHREADCMD;
    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
#endif

    // Empty remote worlds:
    if (currentWorld!=nullptr)
        currentWorld->removeRemoteWorlds();

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
    currentWorld->luaScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_afterinstanceswitch,nullptr,nullptr,nullptr);
    addOnScriptContainer->handleAddOnScriptExecution(sim_syscb_afterinstanceswitch,nullptr,nullptr);
    if (sandboxScript!=nullptr)
        sandboxScript->runSandboxScript(sim_syscb_afterinstanceswitch,nullptr,nullptr);

    // Inform plugins about performed switch to new world:
    int data[4]={getCurrentWorldIndex(),currentWorld->environment->getSceneUniqueID(),0,0};
    void* returnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceswitch,data,nullptr,nullptr);
    delete[] (char*)returnVal;
    setModificationFlag(64); // instance switched

#ifdef SIM_WITH_GUI
    // Inform UI about performed switch to new world:
    cmdIn.cmdId=INSTANCE_WAS_JUST_CREATED_UITHREADCMD;
    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
#endif

    CSceneObject::incrementModelPropertyValidityNumber();

    currentWorld->rebuildRemoteWorlds();

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
        currentWorld->luaScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_beforeinstanceswitch,nullptr,nullptr,nullptr);
        addOnScriptContainer->handleAddOnScriptExecution(sim_syscb_beforeinstanceswitch,nullptr,nullptr);
        if (sandboxScript!=nullptr)
            sandboxScript->runSandboxScript(sim_syscb_beforeinstanceswitch,nullptr,nullptr);

        // Inform plugins about future world switch:
        int pluginData[4]={-1,_worlds[nextWorldIndex]->environment->getSceneUniqueID(),0,0};
        void* pluginReturnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceabouttoswitch,pluginData,nullptr,nullptr);
        delete[] (char*)pluginReturnVal;
    }

    // Empty current scene:
    CWorld* w=currentWorld;
    w->clearScene(true);

    // Disable online mode before destroying:
    w->simulation->setOnlineMode(false);

#ifdef SIM_WITH_GUI
    // Inform UI about future world destruction:
    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId=INSTANCE_ABOUT_TO_BE_DESTROYED_UITHREADCMD;
    cmdIn.intParams.push_back(_currentWorldIndex);
    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
#endif

    // Empty remote worlds:
    currentWorld->removeRemoteWorlds();

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
        currentWorld->luaScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_afterinstanceswitch,nullptr,nullptr,nullptr);
        addOnScriptContainer->handleAddOnScriptExecution(sim_syscb_afterinstanceswitch,nullptr,nullptr);
        if (sandboxScript!=nullptr)
            sandboxScript->runSandboxScript(sim_syscb_afterinstanceswitch,nullptr,nullptr);

        // Inform plugins about performed world switch:
        int pluginData[4]={_currentWorldIndex,currentWorld->environment->getSceneUniqueID(),0,0};
        void* pluginReturnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceswitch,pluginData,nullptr,nullptr);
        delete[] (char*)pluginReturnVal;
        setModificationFlag(64); // instance switched

#ifdef SIM_WITH_GUI
        // Inform UI about performed world switch:
        cmdIn.cmdId=INSTANCE_HAS_CHANGE_UITHREADCMD;
        cmdIn.intParams.clear();
        cmdIn.intParams.push_back(_currentWorldIndex);
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
#endif

        CSceneObject::incrementModelPropertyValidityNumber();

        currentWorld->rebuildRemoteWorlds();
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
#ifdef SIM_WITH_GUI
    globalGuiTextureCont=new CGlobalGuiTextureContainer();
#endif
#ifdef SIM_WITH_SERIAL
    serialPortContainer=new CSerialPortContainer();
#endif

    persistentDataContainer=new CPersistentDataContainer();
    interfaceStackContainer=new CInterfaceStackContainer();
    luaCustomFuncAndVarContainer=new CLuaCustomFuncAndVarContainer();
    customAppData=new CCustomData();
    calcInfo=new CCalculationInfo();
    addOnScriptContainer=new CAddOnScriptContainer();
    initializeRendering();
    createNewWorld();
}

void CWorldContainer::deinitialize()
{
    TRACE_INTERNAL;
    copyBuffer->clearBuffer();
    while (_worlds.size()!=0)
        destroyCurrentWorld();

//    delete sandboxScript;
    delete addOnScriptContainer;
    delete customAppData;
    delete luaCustomFuncAndVarContainer;
    delete interfaceStackContainer;
    delete persistentDataContainer;
#ifdef SIM_WITH_GUI
    delete globalGuiTextureCont;
#endif
    delete copyBuffer;
#ifdef SIM_WITH_SERIAL
    delete serialPortContainer;
#endif
    delete simulatorMessageQueue;
    delete calcInfo;
    deinitializeRendering();
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
    currentWorld->luaScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_beforeinstanceswitch,nullptr,nullptr,nullptr);
    addOnScriptContainer->handleAddOnScriptExecution(sim_syscb_beforeinstanceswitch,nullptr,nullptr);
    if (sandboxScript!=nullptr)
        sandboxScript->runSandboxScript(sim_syscb_beforeinstanceswitch,nullptr,nullptr);

    // Inform plugins about future world switch:
    int pluginData[4]={_currentWorldIndex,_worlds[newWorldIndex]->environment->getSceneUniqueID(),0,0};
    void* pluginReturnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceabouttoswitch,pluginData,nullptr,nullptr);
    delete[] (char*)pluginReturnVal;

    // Disable online mode before switching:
    currentWorld->simulation->setOnlineMode(false);


#ifdef SIM_WITH_GUI
    // Inform UI about future world switch:
    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId=INSTANCE_ABOUT_TO_CHANGE_UITHREADCMD;
    cmdIn.intParams.push_back(newWorldIndex);
    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);

    currentWorld->pageContainer->clearAllLastMouseDownViewIndex();
#endif


    // Empty remote worlds:
    currentWorld->removeRemoteWorlds();

    // Switch worlds:
    _currentWorldIndex=newWorldIndex;
    currentWorld=_worlds[_currentWorldIndex];
    App::currentWorld=currentWorld;

    // Inform scripts about performed world switch:
    currentWorld->luaScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_afterinstanceswitch,nullptr,nullptr,nullptr);
    addOnScriptContainer->handleAddOnScriptExecution(sim_syscb_afterinstanceswitch,nullptr,nullptr);
    if (sandboxScript!=nullptr)
        sandboxScript->runSandboxScript(sim_syscb_afterinstanceswitch,nullptr,nullptr);

    // Inform plugins about performed world switch:
    pluginData[0]=_currentWorldIndex;
    pluginData[1]=currentWorld->environment->getSceneUniqueID();
    pluginReturnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceswitch,pluginData,nullptr,nullptr);
    delete[] (char*)pluginReturnVal;
    setModificationFlag(64); // instance switched


#ifdef SIM_WITH_GUI
    // Inform UI about performed world switch:
    cmdIn.cmdId=INSTANCE_HAS_CHANGE_UITHREADCMD;
    cmdIn.intParams.clear();
    cmdIn.intParams.push_back(_currentWorldIndex);
    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
#endif

    CSceneObject::incrementModelPropertyValidityNumber();

    currentWorld->rebuildRemoteWorlds();

    return(true);
}

bool CWorldContainer::isWorldSwitchingLocked() const
{
    if (currentWorld!=nullptr)
        return(false);
    if (!currentWorld->simulation->isSimulationStopped())
        return(true);
    if (App::getEditModeType()!=NO_EDIT_MODE)
        return(true);
#ifdef SIM_WITH_GUI
    if (App::mainWindow!=nullptr)
    {
        if (App::mainWindow->oglSurface->isViewSelectionActive()||App::mainWindow->oglSurface->isPageSelectionActive())
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

#ifdef SIM_WITH_GUI
void CWorldContainer::addMenu(VMenu* menu)
{ // GUI THREAD only
    TRACE_INTERNAL;
    bool enabled=(!isWorldSwitchingLocked())&&currentWorld->simulation->isSimulationStopped()&&(!App::mainWindow->oglSurface->isPageSelectionActive())&&(!App::mainWindow->oglSurface->isViewSelectionActive())&&(App::getEditModeType()==NO_EDIT_MODE);

    for (size_t i=0;i<_worlds.size();i++)
    {
        std::string txt=_worlds[i]->mainSettings->getSceneName();
        if (txt=="")
            txt="new scene";
        txt+=tt::decorateString(" (scene ",tt::FNb(int(i)+1),")");
        menu->appendMenuItem(enabled,_currentWorldIndex==int(i),SWITCH_TOINSTANCEWITHTHUMBNAILSAVEINDEX0_GUIGUICMD+int(i),txt.c_str(),true);
    }
}

void CWorldContainer::keyPress(int key)
{
    TRACE_INTERNAL;
    if ( (App::mainWindow!=nullptr)&&(key==CTRL_E_KEY) )
    {
        if ((App::getMouseMode()&0x00ff)==sim_navigation_camerashift)
            App::setMouseMode((App::getMouseMode()&0xff00)|sim_navigation_objectshift);
        else
        {
            if ((App::getMouseMode()&0x00ff)==sim_navigation_objectshift)
                App::setMouseMode((App::getMouseMode()&0xff00)|sim_navigation_objectrotate);
            else
                App::setMouseMode((App::getMouseMode()&0xff00)|sim_navigation_camerashift);
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

void CWorldContainer::announceScriptStateWillBeErased(int scriptHandle)
{
    currentWorld->announceScriptStateWillBeErased(scriptHandle);
}


