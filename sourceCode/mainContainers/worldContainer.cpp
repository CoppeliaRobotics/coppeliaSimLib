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
    scriptCustomFuncAndVarContainer=nullptr;
    simulatorMessageQueue=nullptr;
    calcInfo=nullptr;
    customAppData=nullptr;
    moduleMenuItemContainer=nullptr;
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
            currentUniqueIdsOfSel.push_back(it->getObjectUniqueId());
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
        App::worldContainer->sendEvents();
        currentWorld->embeddedScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_beforeinstanceswitch,nullptr,nullptr,nullptr);
        addOnScriptContainer->callScripts(sim_syscb_beforeinstanceswitch,nullptr,nullptr);
        if (sandboxScript!=nullptr)
            sandboxScript->systemCallScript(sim_syscb_beforeinstanceswitch,nullptr,nullptr);
    }

    // Inform plugins about future switch to new world (only if there is already at least one world):
    if (currentWorld!=nullptr)
    {
        int pluginData[4]={_currentWorldIndex,CEnvironment::getNextSceneUniqueId(),0,0};
        void* pluginReturnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceabouttoswitch,pluginData,nullptr,nullptr);
        delete[] (char*)pluginReturnVal;
    }

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
    App::worldContainer->sendEvents();
    currentWorld->embeddedScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_afterinstanceswitch,nullptr,nullptr,nullptr);
    addOnScriptContainer->callScripts(sim_syscb_afterinstanceswitch,nullptr,nullptr);
    if (sandboxScript!=nullptr)
        sandboxScript->systemCallScript(sim_syscb_afterinstanceswitch,nullptr,nullptr);

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

    //CSceneObject::incrementModelPropertyValidityNumber();

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
        App::worldContainer->sendEvents();
        currentWorld->embeddedScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_beforeinstanceswitch,nullptr,nullptr,nullptr);
        addOnScriptContainer->callScripts(sim_syscb_beforeinstanceswitch,nullptr,nullptr);
        if (sandboxScript!=nullptr)
            sandboxScript->systemCallScript(sim_syscb_beforeinstanceswitch,nullptr,nullptr);

        // Inform plugins about future world switch:
        int pluginData[4]={-1,_worlds[nextWorldIndex]->environment->getSceneUniqueID(),0,0};
        void* pluginReturnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceabouttoswitch,pluginData,nullptr,nullptr);
        delete[] (char*)pluginReturnVal;
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
        App::worldContainer->sendEvents();
        currentWorld->embeddedScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_afterinstanceswitch,nullptr,nullptr,nullptr);
        addOnScriptContainer->callScripts(sim_syscb_afterinstanceswitch,nullptr,nullptr);
        if (sandboxScript!=nullptr)
            sandboxScript->systemCallScript(sim_syscb_afterinstanceswitch,nullptr,nullptr);

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

        //CSceneObject::incrementModelPropertyValidityNumber();

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
    moduleMenuItemContainer=new CModuleMenuItemContainer();
#ifdef SIM_WITH_GUI
    globalGuiTextureCont=new CGlobalGuiTextureContainer();
#endif
#ifdef SIM_WITH_SERIAL
    serialPortContainer=new CSerialPortContainer();
#endif

    persistentDataContainer=new CPersistentDataContainer();
    interfaceStackContainer=new CInterfaceStackContainer();
    scriptCustomFuncAndVarContainer=new CScriptCustomFuncAndVarContainer();
    customAppData=new CCustomData();
    calcInfo=new CCalculationInfo();
    addOnScriptContainer=new CAddOnScriptContainer();

    _event=new CInterfaceStackTable();
    _bufferedEvents=interfaceStackContainer->createStack();
    _bufferedEvents->pushTableOntoStack();
    _cborEvents=false;
    _mergeEvents=false;
    _enableEvents=true;

    initializeRendering();
    createNewWorld();
}

void CWorldContainer::deinitialize()
{
    TRACE_INTERNAL;
    delete _event;
    interfaceStackContainer->destroyStack(_bufferedEvents);

    copyBuffer->clearBuffer();
    while (_worlds.size()!=0)
        destroyCurrentWorld();

//    delete sandboxScript;
    delete addOnScriptContainer;
    delete customAppData;
    delete scriptCustomFuncAndVarContainer;
    delete interfaceStackContainer;
    delete persistentDataContainer;
#ifdef SIM_WITH_GUI
    delete globalGuiTextureCont;
#endif
    delete moduleMenuItemContainer;
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
    App::worldContainer->sendEvents();
    currentWorld->embeddedScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_beforeinstanceswitch,nullptr,nullptr,nullptr);
    addOnScriptContainer->callScripts(sim_syscb_beforeinstanceswitch,nullptr,nullptr);
    if (sandboxScript!=nullptr)
        sandboxScript->systemCallScript(sim_syscb_beforeinstanceswitch,nullptr,nullptr);

    // Inform plugins about future world switch:
    int pluginData[4]={_currentWorldIndex,_worlds[newWorldIndex]->environment->getSceneUniqueID(),0,0};
    void* pluginReturnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceabouttoswitch,pluginData,nullptr,nullptr);
    delete[] (char*)pluginReturnVal;

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
    App::worldContainer->sendEvents();
    currentWorld->embeddedScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_afterinstanceswitch,nullptr,nullptr,nullptr);
    addOnScriptContainer->callScripts(sim_syscb_afterinstanceswitch,nullptr,nullptr);
    if (sandboxScript!=nullptr)
        sandboxScript->systemCallScript(sim_syscb_afterinstanceswitch,nullptr,nullptr);

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

    //CSceneObject::incrementModelPropertyValidityNumber();

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

void CWorldContainer::callScripts(int callType,CInterfaceStack* inStack)
{
    TRACE_INTERNAL;
    currentWorld->embeddedScriptContainer->callScripts(callType,inStack);
    addOnScriptContainer->callScripts(callType,inStack,nullptr);
    if (sandboxScript!=nullptr)
        sandboxScript->systemCallScript(callType,inStack,nullptr);
}

CInterfaceStackTable* CWorldContainer::createEvent(const char* event,const char* change,const _CSceneObject_* object,bool isCommonObjectData)
{
    std::string sub;
    if (!isCommonObjectData)
    {
        switch(object->getObjectType())
        {
            case sim_object_shape_type : sub="shape";
                break;
            case sim_object_joint_type : sub="joint";
                break;
            case sim_object_graph_type : sub="graph";
                break;
            case sim_object_camera_type : sub="camera";
                break;
            case sim_object_dummy_type : sub="dummy";
                break;
            case sim_object_proximitysensor_type : sub="proxSensor";
                break;
            case sim_object_path_type : sub="path";
                break;
            case sim_object_visionsensor_type : sub="visionSensor";
                break;
            case sim_object_mill_type : sub="mill";
                break;
            case sim_object_forcesensor_type : sub="forceSensor";
                break;
            case sim_object_light_type : sub="light";
                break;
            case sim_object_mirror_type : sub="mirror";
                break;
            case sim_object_octree_type : sub="octree";
                break;
            case sim_object_pointcloud_type : sub="pointCloud";
                break;
        }

    }

    _lastEvent_event=event;
    _lastEvent_change.clear();
    if (change!=nullptr)
        _lastEvent_change=change;
    _lastEvent_sub=sub;
    _lastEvent_uid=std::to_string(object->getObjectUniqueId());

    _event->appendMapObject_stringString("event",event,0);
    _event->appendMapObject_stringInt32("handle",object->getObjectHandle());
    _event->appendMapObject_stringInt32("uid",object->getObjectUniqueId());
    static long long int seq=0;
    _event->appendMapObject_stringInt64("seq",seq++);
    /*
    std::string tp;
    switch(object->getObjectType())
    {
        case sim_object_shape_type : tp="shape";
            break;
        case sim_object_joint_type : tp="joint";
            break;
        case sim_object_graph_type : tp="graph";
            break;
        case sim_object_camera_type : tp="camera";
            break;
        case sim_object_dummy_type : tp="dummy";
            break;
        case sim_object_proximitysensor_type : tp="proxSensor";
            break;
        case sim_object_path_type : tp="path";
            break;
        case sim_object_visionsensor_type : tp="visionSensor";
            break;
        case sim_object_mill_type : tp="mill";
            break;
        case sim_object_forcesensor_type : tp="forceSensor";
            break;
        case sim_object_light_type : tp="light";
            break;
        case sim_object_mirror_type : tp="mirror";
            break;
        case sim_object_octree_type : tp="octree";
            break;
        case sim_object_pointcloud_type : tp="pointCloud";
            break;
    }
    _event->appendMapObject_stringString("type",tp.c_str(),0);
*/
    CInterfaceStackTable* data=new CInterfaceStackTable();
    _event->appendMapObject_stringObject("data",data);
    if (sub.size()>0)
    {
        CInterfaceStackTable* subC=new CInterfaceStackTable();
        data->appendMapObject_stringObject(sub.c_str(),subC);
        data=subC;
    }
    return(data);
}

void CWorldContainer::pushEvent()
{
    if (!_event->isEmpty())
    {
        CInterfaceStackTable* buff=(CInterfaceStackTable*)_bufferedEvents->getStackObjectFromIndex(0);
        buff->appendArrayObject(_event);

        _eventSumm.push_back(_lastEvent_event);
        _eventSumm.push_back(_lastEvent_change);
        _eventSumm.push_back(_lastEvent_sub);
        _eventSumm.push_back(_lastEvent_uid);

        _event=new CInterfaceStackTable();
    }
}

bool CWorldContainer::getCborEvents() const
{
    return(_cborEvents);
}

void CWorldContainer::setCborEvents(bool b)
{
    _cborEvents=b;
}

void CWorldContainer::setMergeEvents(bool b)
{
    _mergeEvents=b;
}

bool CWorldContainer::getEnableEvents() const
{
    return(_enableEvents);
}

void CWorldContainer::setEnableEvents(bool b)
{
    _enableEvents=b;
}

CInterfaceStack* CWorldContainer::getBufferedEventsStack() const
{
    return(_bufferedEvents);
}

void CWorldContainer::setBufferedEventsStack(CInterfaceStack* stack)
{
    _bufferedEvents=stack;
}

void CWorldContainer::sendEvents()
{
    CInterfaceStackTable* buff=(CInterfaceStackTable*)_bufferedEvents->getStackObjectFromIndex(0);
    if ( (!VThread::isCurrentThreadTheUiThread())&&(!buff->isEmpty()) )
    { // sim thread, send it!
        CInterfaceStack* toSend=interfaceStackContainer->createStack();
        toSend->pushTableOntoStack();
        CInterfaceStackTable* toSendTable=(CInterfaceStackTable*)toSend->getStackObjectFromIndex(0);
        std::map<std::string,bool> map;
        for (int i=int(buff->getArraySize())-1;i>=0;i--)
        {
            std::string c(_eventSumm[4*i+0]+_eventSumm[4*i+1]+_eventSumm[4*i+2]+_eventSumm[4*i+3]);
            std::map<std::string,bool>::iterator it=map.find(c);
            if (it==map.end())
            {
                map[c]=true;
                toSendTable->insertArrayObject(buff->getArrayItemAtIndex(i)->copyYourself(),0);
            }
        }

        if (_mergeEvents)
        {
            for (int i=int(toSendTable->getArraySize())-1;i>=0;i--)
            {
                if (_eventSumm[4*i+0].compare(EVENTTYPE_OBJECTCHANGED)==0)
                {
                    CInterfaceStackTable* data=(CInterfaceStackTable*)((CInterfaceStackTable*)toSendTable->getArrayItemAtIndex(i))->getMapObject("data");
                    CInterfaceStackTable* sub=nullptr;
                    if (_eventSumm[4*i+2].size()>0)
                        sub=(CInterfaceStackTable*)data->getMapObject(_eventSumm[4*i+2].c_str());
                    for (int j=i-1;j>=0;j--)
                    {
                        if ( (_eventSumm[4*i+0]+_eventSumm[4*i+3]).compare(_eventSumm[4*j+0]+_eventSumm[4*j+3])==0 )
                        {
                            CInterfaceStackTable* data2=(CInterfaceStackTable*)((CInterfaceStackTable*)toSendTable->getArrayItemAtIndex(j))->getMapObject("data");
                            CInterfaceStackTable* sub2=nullptr;
                            if (_eventSumm[4*j+2].size()>0)
                                sub2=(CInterfaceStackTable*)data2->getMapObject(_eventSumm[4*j+2].c_str());

                            if ( (sub!=nullptr)&&(sub2!=nullptr) )
                            {
                                std::vector<CInterfaceStackObject*> allObjs;
                                sub2->getAllObjectsAndClearTable(allObjs);
                                for (size_t k=0;k<allObjs.size()/2;k++)
                                    sub->appendArrayOrMapObject(allObjs[2*k+0],allObjs[2*k+1]);
                                data2->removeFromKey(_eventSumm[4*j+2].c_str());
                            }

                            std::vector<CInterfaceStackObject*> allObjs;
                            data2->getAllObjectsAndClearTable(allObjs);
                            for (size_t k=0;k<allObjs.size()/2;k++)
                                data->appendArrayOrMapObject(allObjs[2*k+0],allObjs[2*k+1]);
                            toSendTable->removeArrayItemAtIndex(j);
                            _eventSumm.erase(_eventSumm.begin()+4*j,_eventSumm.begin()+4*j+4);
                            i--;

                            if ( (sub==nullptr)&&(sub2!=nullptr) )
                                sub=sub2;
                        }
                    }
                }
            }
        }


        if (_cborEvents)
        {
            std::string cbor=toSend->getCborEncodedBufferFromTable(0);
            toSend->clear();
            toSend->pushStringOntoStack(cbor.c_str(),cbor.size());
        }
        callScripts(sim_syscb_event,toSend);
        interfaceStackContainer->destroyStack(toSend);
        _bufferedEvents->clear();
        _bufferedEvents->pushTableOntoStack();
        _eventSumm.clear();
    }
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

void CWorldContainer::announceScriptWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript)
{
    // Inform plugins about this event:
    int pluginData[4]={scriptHandle,0,0,0};
    void* pluginReturnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_scriptdestroyed,pluginData,nullptr,nullptr);
    delete[] (char*)pluginReturnVal;

    currentWorld->announceScriptWillBeErased(scriptHandle,simulationScript,sceneSwitchPersistentScript);
}

void CWorldContainer::announceScriptStateWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript)
{
    // Inform plugins about this event:
    int pluginData[4]={scriptHandle,0,0,0};
    void* pluginReturnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_scriptstatedestroyed,pluginData,nullptr,nullptr);
    delete[] (char*)pluginReturnVal;

    moduleMenuItemContainer->announceScriptStateWillBeErased(scriptHandle);
    currentWorld->announceScriptStateWillBeErased(scriptHandle,simulationScript,sceneSwitchPersistentScript);
}


