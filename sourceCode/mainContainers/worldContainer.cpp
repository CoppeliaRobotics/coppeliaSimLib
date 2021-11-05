#include "worldContainer.h"
#include "app.h"
#include "pluginContainer.h"
#include "rendering.h"
#include "tt.h"
#include "interfaceStackString.h"

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
    {
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
    pushAllInitialEvents();
    currentWorld->embeddedScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_afterinstanceswitch,nullptr,nullptr,nullptr);
    addOnScriptContainer->callScripts(sim_syscb_afterinstanceswitch,nullptr,nullptr);
    if (sandboxScript!=nullptr)
        sandboxScript->systemCallScript(sim_syscb_afterinstanceswitch,nullptr,nullptr);

    // Inform plugins about performed switch to new world:
    int dat[4]={getCurrentWorldIndex(),currentWorld->environment->getSceneUniqueID(),0,0};
    void* returnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceswitch,dat,nullptr,nullptr);
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
        pushAllInitialEvents();
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

    _bufferedEvents=new SBufferedEvents;
    _bufferedEvents->eventsStack=interfaceStackContainer->createStack();
    _bufferedEvents->eventsStack->pushTableOntoStack();
    _cborEvents=false;
    _mergeTheEvents=false;
    _enableEvents=true;

    initializeRendering();
    createNewWorld();
}

void CWorldContainer::deinitialize()
{
    TRACE_INTERNAL;
    interfaceStackContainer->destroyStack(_bufferedEvents->eventsStack);
    delete _bufferedEvents;

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
    pushAllInitialEvents();
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

long long int CWorldContainer::_eventSeq=0;

void CWorldContainer::pushSceneObjectRemoveEvent(const _CSceneObject_* object)
{
    if (_enableEvents)
    {
        auto [event,data]=_prepareGeneralEvent(EVENTTYPE_OBJECTREMOVED,object->getObjectHandle(),object->getObjectUid(),nullptr,nullptr,false);
        pushEvent(event);
    }
}

std::tuple<SEventInfo,CInterfaceStackTable*> CWorldContainer::prepareSceneObjectAddEvent(const _CSceneObject_* object)
{
    if (_enableEvents)
    {
        auto [eventInfo,data]=_prepareGeneralEvent(EVENTTYPE_OBJECTADDED,object->getObjectHandle(),object->getObjectUid(),nullptr,nullptr,false);
        return {eventInfo,data};
    }
    SEventInfo d;
    return {d,nullptr};
}

std::tuple<SEventInfo,CInterfaceStackTable*> CWorldContainer::prepareSceneObjectChangedEvent(int sceneObjectHandle,bool isCommonObjectData,const char* fieldName,bool mergeable)
{
    if (_enableEvents)
    {
        CSceneObject* object=currentWorld->sceneObjects->getObjectFromHandle(sceneObjectHandle);
        auto [eventInfo,data]=prepareSceneObjectChangedEvent(object,isCommonObjectData,fieldName,mergeable);
        return {eventInfo,data};
    }
    SEventInfo d;
    return {d,nullptr};
}

std::tuple<SEventInfo,CInterfaceStackTable*> CWorldContainer::prepareSceneObjectChangedEvent(const _CSceneObject_* object,bool isCommonObjectData,const char* fieldName,bool mergeable)
{
    if (_enableEvents)
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
        auto [eventInfo,data]=_prepareGeneralEvent(EVENTTYPE_OBJECTCHANGED,object->getObjectHandle(),object->getObjectUid(),ot,fieldName,mergeable);
        return {eventInfo,data};
    }
    SEventInfo d;
    return {d,nullptr};
}

std::tuple<SEventInfo,CInterfaceStackTable*> CWorldContainer::prepareEvent(const char* event,int uid,const char* fieldName,bool mergeable)
{
    if (_enableEvents)
    {
        auto [eventInfo,data]=_prepareGeneralEvent(event,-1,uid,nullptr,fieldName,mergeable);
        return {eventInfo,data};
    }
    SEventInfo d;
    return {d,nullptr};
}

std::tuple<SEventInfo,CInterfaceStackTable*> CWorldContainer::_prepareGeneralEvent(const char* event,int objectHandle,int uid,const char* objType,const char* fieldName,bool mergeable)
{
    _eventMutex.lock();
    SEventInfo eventInfo;
    eventInfo.event=event;
    if (fieldName!=nullptr)
        eventInfo.subEvent=fieldName;
    if (objType!=nullptr)
        eventInfo.dataSubtype=objType;
    eventInfo.uid=uid;
    eventInfo.mergeable=mergeable;

    eventInfo.eventTable=new CInterfaceStackTable();
    eventInfo.eventTable->appendMapObject_stringString("event",event,0);
    eventInfo.eventTable->appendMapObject_stringInt64("seq",_eventSeq++);
    if (objectHandle!=-1)
        eventInfo.eventTable->appendMapObject_stringInt32("handle",objectHandle);
    if (uid!=-1)
        eventInfo.eventTable->appendMapObject_stringInt32("uid",uid);
    CInterfaceStackTable* data=new CInterfaceStackTable();
    eventInfo.eventTable->appendMapObject_stringObject("data",data);
    if (objType!=nullptr)
    {
        CInterfaceStackTable* subC=new CInterfaceStackTable();
        data->appendMapObject_stringObject(objType,subC);
        data=subC;
    }
    _eventMutex.unlock();
    return {eventInfo,data};
}

void CWorldContainer::pushEvent(SEventInfo& event)
{
    _eventMutex.lock();
    CInterfaceStackTable* buff=(CInterfaceStackTable*)_bufferedEvents->eventsStack->getStackObjectFromIndex(0);
    buff->appendArrayObject(event.eventTable);
    _bufferedEvents->eventDescriptions.push_back(event);
    _eventMutex.unlock();
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
    _mergeTheEvents=b;
}

bool CWorldContainer::getEnableEvents() const
{
    return(_enableEvents);
}

void CWorldContainer::setEnableEvents(bool b)
{
    _enableEvents=b;
}

void CWorldContainer::getAllInitialEvents(CInterfaceStack* stack)
{
    // Swap event buffer:
    SBufferedEvents* tmpEvents=new SBufferedEvents;
    tmpEvents->eventsStack=stack;
    tmpEvents->eventsStack->pushTableOntoStack();
    SBufferedEvents* savedEvents=swapBufferedEvents(tmpEvents);

    // Push all initial events:
    pushAllInitialEvents();

    // Restore event buffer:
    swapBufferedEvents(savedEvents);

    // Condition events
    _prepareEventsForDispatch(tmpEvents);
    delete tmpEvents;
}

void CWorldContainer::pushAllInitialEvents()
{
    if (_enableEvents)
    {
        auto [event,data]=_prepareGeneralEvent(EVENTTYPE_APPSETTINGSCHANGED,-1,-1,nullptr,nullptr,false);
        data->appendMapObject_stringFloat("defaultTranslationStepSize",App::userSettings->getTranslationStepSize());
        data->appendMapObject_stringFloat("defaultRotationStepSize",App::userSettings->getRotationStepSize());
        pushEvent(event);

        currentWorld->pushAllInitialEvents();
    }
}

SBufferedEvents* CWorldContainer::swapBufferedEvents(SBufferedEvents* newBuffer)
{
    _eventMutex.lock();
    SBufferedEvents* retVal=_bufferedEvents;
    _bufferedEvents=newBuffer;
    _eventMutex.unlock();
    return(retVal);
}

void CWorldContainer::dispatchEvents()
{
    if (!VThread::isCurrentThreadTheUiThread())
    {
        // Push the last changes that are not immediate:
        currentWorld->drawingCont->pushAppendNewPointEvents();

        // Swap the event buffer:
        _eventMutex.lock();
        CInterfaceStackTable* buff=(CInterfaceStackTable*)_bufferedEvents->eventsStack->getStackObjectFromIndex(0);
        if (buff->isEmpty())
        {
            _eventMutex.unlock();
            return;
        }
        SBufferedEvents* tmpEvents=_bufferedEvents;
        _bufferedEvents=new SBufferedEvents;
        _bufferedEvents->eventsStack=interfaceStackContainer->createStack();
        _bufferedEvents->eventsStack->pushTableOntoStack();
        _eventMutex.unlock();

        _prepareEventsForDispatch(tmpEvents);

        // Dispatch events:
        callScripts(sim_syscb_event,tmpEvents->eventsStack);

        interfaceStackContainer->destroyStack(tmpEvents->eventsStack);
        delete tmpEvents;
    }
}

void CWorldContainer::_combineDuplicateEvents(SBufferedEvents* events) const
{
    CInterfaceStackTable* buff=(CInterfaceStackTable*)events->eventsStack->getStackObjectFromIndex(0);
    std::vector<SEventInfo>* evSum=&events->eventDescriptions;
    if (!buff->isEmpty())
    {
        std::map<std::string,bool> map;
        for (int i=int(buff->getArraySize())-1;i>=0;i--)
        {
            std::string c(evSum->at(i).event+evSum->at(i).subEvent+evSum->at(i).dataSubtype+std::to_string(evSum->at(i).uid));
            std::map<std::string,bool>::iterator it=map.find(c);
            if (it==map.end()||(!evSum->at(i).mergeable))
                map[c]=true;
            else
            {
                buff->removeArrayItemAtIndex(i);
                evSum->erase(evSum->begin()+i);
            }
        }
    }
}

void CWorldContainer::_mergeEvents(SBufferedEvents* events) const
{
    CInterfaceStackTable* buff=(CInterfaceStackTable*)events->eventsStack->getStackObjectFromIndex(0);
    std::vector<SEventInfo>* evSum=&events->eventDescriptions;
    for (int i=int(buff->getArraySize())-1;i>=0;i--)
    {
        if (evSum->at(i).mergeable)
        {
            CInterfaceStackTable* data=(CInterfaceStackTable*)((CInterfaceStackTable*)buff->getArrayItemAtIndex(i))->getMapObject("data");
            CInterfaceStackTable* sub=nullptr;
            if (evSum->at(i).dataSubtype.size()>0)
                sub=(CInterfaceStackTable*)data->getMapObject(evSum->at(i).dataSubtype.c_str());
            for (int j=i-1;j>=0;j--)
            {
                if ( (evSum->at(j).mergeable)&&(evSum->at(i).uid==evSum->at(j).uid)&&(evSum->at(i).event.compare(evSum->at(j).event)==0) )
                {
                    CInterfaceStackTable* data2=(CInterfaceStackTable*)((CInterfaceStackTable*)buff->getArrayItemAtIndex(j))->getMapObject("data");
                    CInterfaceStackTable* sub2=nullptr;
                    if (evSum->at(j).dataSubtype.size()>0)
                        sub2=(CInterfaceStackTable*)data2->getMapObject(evSum->at(j).dataSubtype.c_str());

                    if ( (sub!=nullptr)&&(sub2!=nullptr) )
                    {
                        std::vector<CInterfaceStackObject*> allObjs;
                        sub2->getAllObjectsAndClearTable(allObjs);
                        for (size_t k=0;k<allObjs.size()/2;k++)
                        {
                            if (sub->containsKey(((CInterfaceStackString*)allObjs[2*k+0])->getValue(nullptr)))
                            {
                                delete allObjs[2*k+0];
                                delete allObjs[2*k+1];
                            }
                            else
                                sub->appendArrayOrMapObject(allObjs[2*k+0],allObjs[2*k+1]);
                        }
//                        data2->removeFromKey(evSum->at(j).dataSubtype.c_str());
                    }

                    std::vector<CInterfaceStackObject*> allObjs;
                    data2->getAllObjectsAndClearTable(allObjs);
                    for (size_t k=0;k<allObjs.size()/2;k++)
                    {
                        if (data->containsKey(((CInterfaceStackString*)allObjs[2*k+0])->getValue(nullptr)))
                        {
                            delete allObjs[2*k+0];
                            delete allObjs[2*k+1];
                        }
                        else
                            data->appendArrayOrMapObject(allObjs[2*k+0],allObjs[2*k+1]);
                    }
                    buff->removeArrayItemAtIndex(j);
                    evSum->erase(evSum->begin()+j);
                    i--;

                    if ( (sub==nullptr)&&(sub2!=nullptr) )
                        sub=sub2;
                }
            }
        }
    }
}

void CWorldContainer::_prepareEventsForDispatch(SBufferedEvents* events) const
{
    if (_mergeTheEvents)
    {
        _combineDuplicateEvents(events);
        _mergeEvents(events);
    }

    if (_cborEvents)
    {
        std::string cbor=events->eventsStack->getCborEncodedBufferFromTable(0);
        events->eventsStack->clear();
        events->eventsStack->pushStringOntoStack(cbor.c_str(),cbor.size());
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


