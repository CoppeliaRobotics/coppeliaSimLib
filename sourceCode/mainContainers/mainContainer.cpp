
#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "v_rep_internal.h"
#include "tt.h"
#include "app.h"
#include "pluginContainer.h"
#include "vVarious.h"
#include "v_repStrings.h"
#include "rendering.h"

CMainContainer::CMainContainer()
{
    FUNCTION_DEBUG;
    copyBuffer=nullptr;
    sandboxScript=nullptr;
    addOnScriptContainer=nullptr;
    persistentDataContainer=nullptr;
    interfaceStackContainer=nullptr;
    luaCustomFuncAndVarContainer=nullptr;
    customAppData=nullptr;
    simulatorMessageQueue=nullptr;
    commTubeContainer=nullptr;
    signalContainer=nullptr;
    dynamicsContainer=nullptr;
    undoBufferContainer=nullptr;
#ifdef SIM_WITH_GUI
    globalGuiTextureCont=nullptr;
#endif
#ifdef SIM_WITH_SERIAL
    serialPortContainer=nullptr;
#endif
    outsideCommandQueue=nullptr;
    buttonBlockContainer=nullptr;
    collections=nullptr;
    distances=nullptr;
    collisions=nullptr;
    environment=nullptr;
    pageContainer=nullptr;
    mainSettings=nullptr;
    pathPlanning=nullptr;
    motionPlanning=nullptr;
    luaScriptContainer=nullptr;
    textureCont=nullptr;
    confContainer=nullptr;
    simulation=nullptr;
    customSceneData=nullptr;
    customSceneData_tempData=nullptr;
    cacheData=nullptr;
    constraintSolver=nullptr;
    drawingCont=nullptr;
    pointCloudCont=nullptr;
    ghostObjectCont=nullptr;
    bannerCont=nullptr;
    ikGroups=nullptr;
    objCont=nullptr;

    currentInstanceIndex=-1;
}

CMainContainer::~CMainContainer()
{
}

bool CMainContainer::setInstanceIndex(int index)
{ // SIM THREAD only
    FUNCTION_DEBUG;
    if (getCurrentInstanceIndex()==index)
        return(true); // we already have this instance!
    if (!isInstanceSwitchingLocked())
        return(makeInstanceCurrentFromIndex(index,false));
    return(false);
}

void CMainContainer::simulationAboutToStart()
{
    FUNCTION_DEBUG;

#ifdef SIM_WITH_GUI
    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId=SIMULATION_ABOUT_TO_START_UITHREADCMD;
    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
#endif

    luaScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_beforesimulation,nullptr,nullptr,nullptr);
    addOnScriptContainer->handleAddOnScriptExecution(sim_syscb_beforesimulation,nullptr,nullptr);
    if (sandboxScript!=nullptr)
        sandboxScript->runSandboxScript(sim_syscb_beforesimulation,nullptr,nullptr);

    _initialObjectUniqueIdentifiersForRemovingNewObjects.clear();
    for (int i=0;i<int(objCont->objectList.size());i++)
    {
        C3DObject* it=objCont->getObjectFromHandle(objCont->objectList[i]);
        _initialObjectUniqueIdentifiersForRemovingNewObjects.push_back(it->getUniqueID());
    }
    POST_SCENE_CHANGED_ANNOUNCEMENT("");

    _savedMouseMode=App::getMouseMode();

#ifdef SIM_WITH_GUI
    if (App::mainWindow!=nullptr)
        App::mainWindow->codeEditorContainer->simulationAboutToStart();
#endif

    if (App::ct->dynamicsContainer->getDynamicEngineType(nullptr)==sim_physics_newton)
    {
        App::addStatusbarMessage("Warning: the Newton Dynamics plugin is still in a BETA stage, and you might experience strange/inaccurate behaviour, and crashes!");
        printf("Warning: the Newton Dynamics plugin is still in a BETA stage, and you might experience strange/inaccurate behaviour, and crashes!\n");
    }


    if (!CPluginContainer::isMeshPluginAvailable())
    {
#ifdef SIM_WITH_GUI
        simDisplayDialog_internal("ERROR","The 'MeshCalc' plugin could not be initialized. Collision detection, distance calculation,\nproximity sensor simulation and cutting simulation will not work.",sim_dlgstyle_ok,"",nullptr,nullptr,nullptr);
#endif
        printf("ERROR: The 'MeshCalc' plugin could not be initialized. Collision detection,\n       distance calculation, proximity sensor simulation and cutting\n       simulation will not work.\n");
    }

    drawingCont->simulationAboutToStart();
    pointCloudCont->simulationAboutToStart();
    ghostObjectCont->simulationAboutToStart();
    bannerCont->simulationAboutToStart();
    buttonBlockContainer->simulationAboutToStart();
    constraintSolver->simulationAboutToStart();
    customSceneData->simulationAboutToStart();
    customSceneData_tempData->simulationAboutToStart();
    dynamicsContainer->simulationAboutToStart();
    signalContainer->simulationAboutToStart();
    environment->simulationAboutToStart();
    calcInfo->simulationAboutToStart();
    luaScriptContainer->simulationAboutToStart();
    mainSettings->simulationAboutToStart();
    confContainer->simulationAboutToStart();
    objCont->simulationAboutToStart();
    pageContainer->simulationAboutToStart();
    collisions->simulationAboutToStart();
    distances->simulationAboutToStart();
    collections->simulationAboutToStart();
    ikGroups->simulationAboutToStart();
    pathPlanning->simulationAboutToStart();
    motionPlanning->simulationAboutToStart();
    simulation->simulationAboutToStart();
    textureCont->simulationAboutToStart();
    undoBufferContainer->simulationAboutToStart();
    commTubeContainer->simulationAboutToStart();

    void* retVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_simulationabouttostart,nullptr,nullptr,nullptr);
    delete[] (char*)retVal;

    setModificationFlag(2048); // simulation started

    SSimulationThreadCommand cmd;
    cmd.cmdId=DISPLAY_VARIOUS_WARNING_MESSAGES_DURING_SIMULATION_CMD;
    App::appendSimulationThreadCommand(cmd,1000);

    App::setToolbarRefreshFlag();
    App::setFullDialogRefreshFlag();
    if (VREP_PROGRAM_VERSION_NB==30500)
        App::addStatusbarMessage("Warning: 'print()' now prints to the status bar, instead of the console (use 'print=printToConsole' to revert).");

#ifdef SIM_WITH_GUI
    if (App::mainWindow!=nullptr)
        App::mainWindow->simulationRecorder->startRecording(false);
#endif
}

void CMainContainer::simulationPaused()
{
    CLuaScriptObject* mainScript=luaScriptContainer->getMainScript();
    if (mainScript!=nullptr)
        mainScript->runMainScript(sim_syscb_suspend,nullptr,nullptr,nullptr);
    App::setToolbarRefreshFlag();
    App::setFullDialogRefreshFlag();
}

void CMainContainer::simulationAboutToResume()
{
    CLuaScriptObject* mainScript=luaScriptContainer->getMainScript();
    if (mainScript!=nullptr)
        mainScript->runMainScript(sim_syscb_resume,nullptr,nullptr,nullptr);
    App::setToolbarRefreshFlag();
    App::setFullDialogRefreshFlag();
}

void CMainContainer::simulationAboutToStep()
{
    calcInfo->formatInfo();
    calcInfo->resetInfo();
    ikGroups->resetCalculationResults();
}

void CMainContainer::simulationAboutToEnd()
{
    FUNCTION_DEBUG;

    void* retVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_simulationabouttoend,nullptr,nullptr,nullptr);
    delete[] (char*)retVal;

    luaScriptContainer->simulationAboutToEnd(); // will call a last time the main and all non-threaded child scripts, then reset them

#ifdef SIM_WITH_GUI
    if (App::mainWindow!=nullptr)
        App::mainWindow->codeEditorContainer->simulationAboutToEnd();
#endif
}

void CMainContainer::simulationEnded(bool removeNewObjects)
{
    FUNCTION_DEBUG;
    POST_SCENE_CHANGED_ANNOUNCEMENT(""); // keeps this (this has the objects in their last position, including additional objects)
    simResetMilling_internal(sim_handle_all); // important to reset all shapes and volumes!!! (the instruction in the main script might have been removed!)

    void* retVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_simulationended,nullptr,nullptr,nullptr);
    delete[] (char*)retVal;

    setModificationFlag(4096); // simulation ended
    drawingCont->simulationEnded();
    pointCloudCont->simulationEnded();
    ghostObjectCont->simulationEnded();
    bannerCont->simulationEnded();
    buttonBlockContainer->simulationEnded();
    constraintSolver->simulationEnded();
    customSceneData->simulationEnded();
    customSceneData_tempData->simulationEnded();
    dynamicsContainer->simulationEnded();
    signalContainer->simulationEnded();
    environment->simulationEnded();
    calcInfo->simulationEnded();
    luaScriptContainer->simulationEnded();
    mainSettings->simulationEnded();
    confContainer->simulationEnded();
    objCont->simulationEnded();
    pageContainer->simulationEnded();
    collisions->simulationEnded();
    distances->simulationEnded();
    collections->simulationEnded();
    ikGroups->simulationEnded();
    pathPlanning->simulationEnded();
    motionPlanning->simulationEnded();
    simulation->simulationEnded();
    textureCont->simulationEnded();
    undoBufferContainer->simulationEnded();
    commTubeContainer->simulationEnded();
#ifdef SIM_WITH_SERIAL
    serialPortContainer->simulationEnded();
#endif

#ifdef SIM_WITH_GUI
    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId=SIMULATION_JUST_ENDED_UITHREADCMD;
    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
#endif

    App::setMouseMode(_savedMouseMode);
    App::setToolbarRefreshFlag();
    App::setFullDialogRefreshFlag();

    if (removeNewObjects)
    {
        std::vector<int> savedSelection;
        objCont->getSelectedObjects(savedSelection);
        objCont->deselectObjects();
        std::vector<int> toRemove;
        for (size_t i=0;i<objCont->objectList.size();i++)
        {
            C3DObject* it=objCont->getObjectFromHandle(objCont->objectList[i]);
            bool found=false;
            for (size_t j=0;j<_initialObjectUniqueIdentifiersForRemovingNewObjects.size();j++)
            {
                if (it->getUniqueID()==_initialObjectUniqueIdentifiersForRemovingNewObjects[j])
                {
                    found=true;
                    break;
                }
            }
            if (!found)
                toRemove.push_back(it->getObjectHandle());
        }
        objCont->eraseSeveralObjects(toRemove,true);
        for (size_t i=0;i<savedSelection.size();i++)
            objCont->addObjectToSelection(savedSelection[i]);
    }
    _initialObjectUniqueIdentifiersForRemovingNewObjects.clear();
    POST_SCENE_CHANGED_ANNOUNCEMENT(""); // keeps this (additional objects were removed, and object positions were reset)
    luaScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_aftersimulation,nullptr,nullptr,nullptr);
    addOnScriptContainer->handleAddOnScriptExecution(sim_syscb_aftersimulation,nullptr,nullptr);
    if (sandboxScript!=nullptr)
        sandboxScript->runSandboxScript(sim_syscb_aftersimulation,nullptr,nullptr);
}

void CMainContainer::setModificationFlag(int bitMask)
{
    _modificationFlags|=bitMask;
}

int CMainContainer::getModificationFlags(bool clearTheFlagsAfter)
{
    if (App::getEditModeType()!=NO_EDIT_MODE)
        _modificationFlags|=128;
    std::vector<int> currentUniqueIdsOfSel;
    for (int i=0;i<int(objCont->getSelSize());i++)
    {
        C3DObject* it=objCont->getObjectFromHandle(objCont->getSelID(i));
        if (it!=nullptr)
            currentUniqueIdsOfSel.push_back(it->getUniqueID());
    }
    if (currentUniqueIdsOfSel.size()==_uniqueIdsOfSelectionSinceLastTimeGetAndClearModificationFlagsWasCalled.size())
    {
        for (int i=0;i<int(currentUniqueIdsOfSel.size());i++)
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

    int retVal=_modificationFlags;
    if (clearTheFlagsAfter)
        _modificationFlags=0;
    return(retVal);
}


int CMainContainer::createNewInstance()
{
    FUNCTION_DEBUG;

    if (luaScriptContainer!=nullptr)
        luaScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_beforeinstanceswitch,nullptr,nullptr,nullptr);
    addOnScriptContainer->handleAddOnScriptExecution(sim_syscb_beforeinstanceswitch,nullptr,nullptr);
    if (sandboxScript!=nullptr)
        sandboxScript->runSandboxScript(sim_syscb_beforeinstanceswitch,nullptr,nullptr);

    if (simulation!=nullptr)
        simulation->setOnlineMode(false); // disable online mode before switching

    // Added following 3 on 25/5/2017:
    int pluginData[4]={currentInstanceIndex,CEnvironment::getNextSceneUniqueId(),0,0};
    void* pluginReturnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceabouttoswitch,pluginData,nullptr,nullptr);
    delete[] (char*)pluginReturnVal;
//    printf("Instance about to switch: next scene unique ID: %i\n",pluginData[1]);

#ifdef SIM_WITH_GUI
    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId=INSTANCE_ABOUT_TO_BE_CREATED_UITHREADCMD;
    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
#endif

    _undoBufferContainerList.push_back(nullptr);
    _outsideCommandQueueList.push_back(nullptr);
    _simulationList.push_back(nullptr);
    _confContainerList.push_back(nullptr);
    _textureContList.push_back(nullptr);
    _buttonBlockContainerList.push_back(nullptr);
    _collectionList.push_back(nullptr);
    _luaScriptContainerList.push_back(nullptr);
    _distanceList.push_back(nullptr);
    _collisionList.push_back(nullptr);
    _environmentList.push_back(nullptr);
    _pageContainerList.push_back(nullptr);
    _mainSettingsList.push_back(nullptr);
    _pathPlanningList.push_back(nullptr);
    _motionPlanningList.push_back(nullptr);
    _customSceneDataList.push_back(nullptr);
    _customSceneData_tempDataList.push_back(nullptr);
    _cacheDataList.push_back(nullptr);
    _constraintSolverList.push_back(nullptr);
    _drawingContainerList.push_back(nullptr);
    _pointCloudContainerList.push_back(nullptr);
    _ghostObjectContainerList.push_back(nullptr);
    _bannerContainerList.push_back(nullptr);
    _dynamicsContainerList.push_back(nullptr);
    _signalContainerList.push_back(nullptr);
    _commTubeContainerList.push_back(nullptr);
    _ikGroupList.push_back(nullptr);
    _objContList.push_back(nullptr);

    currentInstanceIndex=int(_objContList.size())-1;

    undoBufferContainer=nullptr;
    outsideCommandQueue=nullptr;
    buttonBlockContainer=nullptr;
    simulation=nullptr;
    confContainer=nullptr;
    textureCont=nullptr;
    luaScriptContainer=nullptr;
    collections=nullptr;
    distances=nullptr;
    collisions=nullptr;
    pathPlanning=nullptr;
    motionPlanning=nullptr;
    environment=nullptr;
    pageContainer=nullptr;
    mainSettings=nullptr;
    customSceneData=nullptr;
    customSceneData_tempData=nullptr;
    cacheData=nullptr;
    constraintSolver=nullptr;
    drawingCont=nullptr;
    pointCloudCont=nullptr;
    ghostObjectCont=nullptr;
    bannerCont=nullptr;
    dynamicsContainer=nullptr;
    signalContainer=nullptr;
    commTubeContainer=nullptr;
    ikGroups=nullptr;
    objCont=nullptr;

    undoBufferContainer=new CUndoBufferCont();
    outsideCommandQueue=new COutsideCommandQueue();
    buttonBlockContainer=new CButtonBlockContainer(true);
    simulation=new CSimulation();
    confContainer=new CMemorizedConfContainer();
    textureCont=new CTextureContainer();
    luaScriptContainer=new CLuaScriptContainer();
    collections=new CRegisteredCollections();
    ikGroups=new CRegisterediks();
    distances=new CRegisteredDistances();
    collisions=new CRegisteredCollisions();
    pathPlanning=new CRegisteredPathPlanningTasks();
    motionPlanning=new CRegisteredMotionPlanningTasks();
    environment=new CEnvironment();
    pageContainer=new CPageContainer();
    objCont=new CObjCont();
    mainSettings=new CMainSettings();
    customSceneData=new CCustomData();
    customSceneData_tempData=new CCustomData();
    cacheData=new CCacheCont();
    constraintSolver=new CConstraintSolverContainer();
    drawingCont=new CDrawingContainer();
    pointCloudCont=new CPointCloudContainer_old();
    ghostObjectCont=new CGhostObjectContainer();
    bannerCont=new CBannerContainer();
    dynamicsContainer=new CDynamicsContainer();
    signalContainer=new CSignalContainer();
    commTubeContainer=new CCommTubeContainer();

    _undoBufferContainerList[currentInstanceIndex]=undoBufferContainer;
    _outsideCommandQueueList[currentInstanceIndex]=outsideCommandQueue;
    _buttonBlockContainerList[currentInstanceIndex]=buttonBlockContainer;
    _simulationList[currentInstanceIndex]=simulation;
    _confContainerList[currentInstanceIndex]=confContainer;
    _textureContList[currentInstanceIndex]=textureCont;
    _collectionList[currentInstanceIndex]=collections;
    _luaScriptContainerList[currentInstanceIndex]=luaScriptContainer;
    _distanceList[currentInstanceIndex]=distances;
    _collisionList[currentInstanceIndex]=collisions;
    _environmentList[currentInstanceIndex]=environment;
    _pageContainerList[currentInstanceIndex]=pageContainer;
    _mainSettingsList[currentInstanceIndex]=mainSettings;
    _pathPlanningList[currentInstanceIndex]=pathPlanning;
    _motionPlanningList[currentInstanceIndex]=motionPlanning;
    _customSceneDataList[currentInstanceIndex]=customSceneData;
    _customSceneData_tempDataList[currentInstanceIndex]=customSceneData_tempData;
    _cacheDataList[currentInstanceIndex]=cacheData;
    _constraintSolverList[currentInstanceIndex]=constraintSolver;
    _drawingContainerList[currentInstanceIndex]=drawingCont;
    _pointCloudContainerList[currentInstanceIndex]=pointCloudCont;
    _ghostObjectContainerList[currentInstanceIndex]=ghostObjectCont;
    _bannerContainerList[currentInstanceIndex]=bannerCont;
    _dynamicsContainerList[currentInstanceIndex]=dynamicsContainer;
    _signalContainerList[currentInstanceIndex]=signalContainer;
    _commTubeContainerList[currentInstanceIndex]=commTubeContainer;
    _ikGroupList[currentInstanceIndex]=ikGroups;
    _objContList[currentInstanceIndex]=objCont;

    addOnScriptContainer->handleAddOnScriptExecution(sim_syscb_afterinstanceswitch,nullptr,nullptr);
    if (sandboxScript!=nullptr)
        sandboxScript->runSandboxScript(sim_syscb_afterinstanceswitch,nullptr,nullptr);

    int data[4]={getCurrentInstanceIndex(),_environmentList[currentInstanceIndex]->getSceneUniqueID(),0,0};
    void* returnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceswitch,data,nullptr,nullptr);
    delete[] (char*)returnVal;
    setModificationFlag(64); // instance switched
//    printf("Instance SWITCHED to scene unique ID: %i\n",data[1]);

#ifdef SIM_WITH_GUI
    cmdIn.cmdId=INSTANCE_WAS_JUST_CREATED_UITHREADCMD;
    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);

#endif

    return(currentInstanceIndex);
}

int CMainContainer::getCurrentInstanceIndex()
{
    return (currentInstanceIndex);
}

int CMainContainer::destroyCurrentInstance()
{
    FUNCTION_DEBUG;
    if (currentInstanceIndex==-1)
        return(-1);

    if (_objContList.size()>1)
    { // only send the switch instance message if there is another instance to switch to:
        addOnScriptContainer->handleAddOnScriptExecution(sim_syscb_beforeinstanceswitch,nullptr,nullptr);
        if (sandboxScript!=nullptr)
            sandboxScript->runSandboxScript(sim_syscb_beforeinstanceswitch,nullptr,nullptr);
        int pluginData[4]={-1,_environmentList[int(_objContList.size())-2]->getSceneUniqueID(),0,0};
        void* pluginReturnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceabouttoswitch,pluginData,nullptr,nullptr);
        delete[] (char*)pluginReturnVal;
    }

#ifdef SIM_WITH_GUI
    if (App::mainWindow!=nullptr)
        App::mainWindow->codeEditorContainer->sceneClosed(environment->getSceneUniqueID());

    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId=INSTANCE_ABOUT_TO_BE_DESTROYED_UITHREADCMD;
    cmdIn.intParams.push_back(currentInstanceIndex);
    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
#endif
    environment->setSceneIsClosingFlag(true); // so that attached scripts can react to it
    // Important to empty objects first (since objCont->announce....willBeErase
    // might be called for already destroyed objects!)
    objCont->removeSceneDependencies();
    confContainer->removeMemorized();
    collections->removeAllCollections();
    ikGroups->removeAllIkGroups();
    distances->removeAllDistanceObjects();
    collisions->removeAllCollisionObjects();
    pathPlanning->removeAllTasks();
    motionPlanning->removeAllTasks();
    luaScriptContainer->removeAllScripts();

    objCont->removeAllObjects(false);

    simulation->setUpDefaultValues();
    pageContainer->emptySceneProcedure();

    if (buttonBlockContainer!=nullptr)
        buttonBlockContainer->removeAllBlocks(true);

    delete undoBufferContainer;
    undoBufferContainer=nullptr;
    delete confContainer;
    confContainer=nullptr;
    delete luaScriptContainer;
    luaScriptContainer=nullptr;
    delete dynamicsContainer;
    dynamicsContainer=nullptr;
    delete mainSettings;
    mainSettings=nullptr;
    delete objCont;
    objCont=nullptr;
    delete pageContainer;
    pageContainer=nullptr;
    delete environment;
    environment=nullptr;
    delete pathPlanning;
    pathPlanning=nullptr;
    delete motionPlanning;
    motionPlanning=nullptr;
    delete collisions;
    collisions=nullptr;
    delete distances;
    distances=nullptr;
    delete ikGroups;
    ikGroups=nullptr;
    delete collections;
    collections=nullptr;
    delete textureCont;
    textureCont=nullptr;
    delete simulation;
    simulation=nullptr;
    delete buttonBlockContainer;
    buttonBlockContainer=nullptr;
    delete outsideCommandQueue;
    outsideCommandQueue=nullptr;
    delete customSceneData;
    customSceneData=nullptr;
    delete customSceneData_tempData;
    customSceneData_tempData=nullptr;
    delete cacheData;
    cacheData=nullptr;
    delete constraintSolver;
    constraintSolver=nullptr;
    delete drawingCont;
    drawingCont=nullptr;
    delete pointCloudCont;
    pointCloudCont=nullptr;
    delete ghostObjectCont;
    ghostObjectCont=nullptr;
    delete bannerCont;
    bannerCont=nullptr;
    delete signalContainer;
    signalContainer=nullptr;
    delete commTubeContainer;
    commTubeContainer=nullptr;

    _undoBufferContainerList.erase(_undoBufferContainerList.begin()+currentInstanceIndex);
    _confContainerList.erase(_confContainerList.begin()+currentInstanceIndex);
    _luaScriptContainerList.erase(_luaScriptContainerList.begin()+currentInstanceIndex);
    _mainSettingsList.erase(_mainSettingsList.begin()+currentInstanceIndex);
    _pageContainerList.erase(_pageContainerList.begin()+currentInstanceIndex);
    _environmentList.erase(_environmentList.begin()+currentInstanceIndex);
    _pathPlanningList.erase(_pathPlanningList.begin()+currentInstanceIndex);
    _motionPlanningList.erase(_motionPlanningList.begin()+currentInstanceIndex);
    _collisionList.erase(_collisionList.begin()+currentInstanceIndex);
    _distanceList.erase(_distanceList.begin()+currentInstanceIndex);
    _collectionList.erase(_collectionList.begin()+currentInstanceIndex);
    _textureContList.erase(_textureContList.begin()+currentInstanceIndex);
    _simulationList.erase(_simulationList.begin()+currentInstanceIndex);
    _buttonBlockContainerList.erase(_buttonBlockContainerList.begin()+currentInstanceIndex);
    _outsideCommandQueueList.erase(_outsideCommandQueueList.begin()+currentInstanceIndex);
    _customSceneDataList.erase(_customSceneDataList.begin()+currentInstanceIndex);
    _customSceneData_tempDataList.erase(_customSceneData_tempDataList.begin()+currentInstanceIndex);
    _cacheDataList.erase(_cacheDataList.begin()+currentInstanceIndex);
    _constraintSolverList.erase(_constraintSolverList.begin()+currentInstanceIndex);
    _drawingContainerList.erase(_drawingContainerList.begin()+currentInstanceIndex);
    _pointCloudContainerList.erase(_pointCloudContainerList.begin()+currentInstanceIndex);
    _ghostObjectContainerList.erase(_ghostObjectContainerList.begin()+currentInstanceIndex);
    _bannerContainerList.erase(_bannerContainerList.begin()+currentInstanceIndex);
    _dynamicsContainerList.erase(_dynamicsContainerList.begin()+currentInstanceIndex);
    _signalContainerList.erase(_signalContainerList.begin()+currentInstanceIndex);
    _commTubeContainerList.erase(_commTubeContainerList.begin()+currentInstanceIndex);
    _objContList.erase(_objContList.begin()+currentInstanceIndex);
    _ikGroupList.erase(_ikGroupList.begin()+currentInstanceIndex);

    if (_objContList.size()!=0)
    {
        currentInstanceIndex=-1; // We set this so that next instruction succeeds (otherwise we might already be in instance 0 and the function will not process). Added on 5/3/2012
        makeInstanceCurrentFromIndex(int(_objContList.size())-1,true);
    }
    else
        currentInstanceIndex=-1;
    return(currentInstanceIndex);
}

int CMainContainer::getInstanceCount()
{
    return(int(_objContList.size()));
}

void CMainContainer::initialize()
{
    FUNCTION_DEBUG;

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
//    sandboxScript=new CLuaScriptObject(sim_scripttype_sandboxscript);
    initializeRendering();
    createNewInstance();
}

void CMainContainer::deinitialize()
{
    FUNCTION_DEBUG;
    copyBuffer->clearBuffer();

    bool prevInstanceWasDestroyed=false;
    while (_objContList.size()!=0)
    {
        makeInstanceCurrentFromIndex(int(_objContList.size())-1,prevInstanceWasDestroyed);
        destroyCurrentInstance();
        prevInstanceWasDestroyed=true;
    }
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

void CMainContainer::emptyScene(bool notCalledFromUndoFunction)
{
    FUNCTION_DEBUG;
    if (notCalledFromUndoFunction)
        undoBufferContainer->emptySceneProcedure();

#ifdef SIM_WITH_GUI
    if (App::mainWindow!=nullptr)
        App::mainWindow->codeEditorContainer->sceneClosed(environment->getSceneUniqueID());
#endif
    environment->setSceneIsClosingFlag(true); // so that attached scripts can react to it
    // Following is saved (what does it mean??)
    objCont->removeSceneDependencies();
    confContainer->removeMemorized();
    collections->removeAllCollections();
    ikGroups->removeAllIkGroups();
    distances->removeAllDistanceObjects();
    collisions->removeAllCollisionObjects();
    pathPlanning->removeAllTasks();
    motionPlanning->removeAllTasks();
    luaScriptContainer->removeAllScripts(); // Important to have this in first position (e.g. clean-up procedure of scripts might erase objects, etc.)

    simulation->setUpDefaultValues();
    if (buttonBlockContainer!=nullptr)
        buttonBlockContainer->emptySceneProcedure(true);
    collections->setUpDefaultValues();
    ikGroups->removeAllIkGroups();
    distances->setUpDefaultValues();
    collisions->setUpDefaultValues();
    environment->setUpDefaultValues();
    pageContainer->emptySceneProcedure();
    pathPlanning->removeAllTasks();
    motionPlanning->removeAllTasks();
    objCont->removeAllObjects(true);
    customSceneData->removeAllData();
    customSceneData_tempData->removeAllData();
    constraintSolver->removeAllObjects();
    confContainer->removeMemorized();
    if (notCalledFromUndoFunction)
        mainSettings->setUpDefaultValues();
    cacheData->clearCache();
    drawingCont->emptySceneProcedure();
    pointCloudCont->emptySceneProcedure();
    ghostObjectCont->emptySceneProcedure();
    bannerCont->emptySceneProcedure();
    signalContainer->emptySceneProcedure();
    commTubeContainer->emptySceneProcedure();
    environment->setSceneIsClosingFlag(false);
}

bool CMainContainer::makeInstanceCurrentFromIndex(int instanceIndex,bool previousInstanceWasDestroyed)
{ 
    // Careful!! We cannot access the ct items directly, if previousInstanceWasDestroyed is true

    FUNCTION_DEBUG;
    if ( (instanceIndex<0)||(instanceIndex>=int(_objContList.size())))
        return(false);
    if (currentInstanceIndex==instanceIndex)
        return(true);
    if (isInstanceSwitchingLocked())
        return(false);

    if (simulation!=nullptr)
        simulation->setOnlineMode(false); // disable online mode before switching

    if (!previousInstanceWasDestroyed)
    {
        luaScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_beforeinstanceswitch,nullptr,nullptr,nullptr);
        addOnScriptContainer->handleAddOnScriptExecution(sim_syscb_beforeinstanceswitch,nullptr,nullptr);
        if (sandboxScript!=nullptr)
            sandboxScript->runSandboxScript(sim_syscb_beforeinstanceswitch,nullptr,nullptr);
        int pluginData[4]={currentInstanceIndex,_environmentList[instanceIndex]->getSceneUniqueID(),0,0};
        void* pluginReturnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceabouttoswitch,pluginData,nullptr,nullptr);
        delete[] (char*)pluginReturnVal;
    }




#ifdef SIM_WITH_GUI
    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId=INSTANCE_ABOUT_TO_CHANGE_UITHREADCMD;
    cmdIn.intParams.push_back(instanceIndex);
    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);

    if (pageContainer!=nullptr)
        pageContainer->clearAllLastMouseDownViewIndex();
#endif

    currentInstanceIndex=instanceIndex;

    undoBufferContainer=_undoBufferContainerList[currentInstanceIndex];
    outsideCommandQueue=_outsideCommandQueueList[currentInstanceIndex];
    simulation=_simulationList[currentInstanceIndex];
    confContainer=_confContainerList[currentInstanceIndex];
    textureCont=_textureContList[currentInstanceIndex];
    buttonBlockContainer=_buttonBlockContainerList[currentInstanceIndex];
    collections=_collectionList[currentInstanceIndex];
    luaScriptContainer=_luaScriptContainerList[currentInstanceIndex];
    distances=_distanceList[currentInstanceIndex];
    collisions=_collisionList[currentInstanceIndex];
    environment=_environmentList[currentInstanceIndex];
    pageContainer=_pageContainerList[currentInstanceIndex];
    mainSettings=_mainSettingsList[currentInstanceIndex];
    pathPlanning=_pathPlanningList[currentInstanceIndex];
    motionPlanning=_motionPlanningList[currentInstanceIndex];
    customSceneData=_customSceneDataList[currentInstanceIndex];
    customSceneData_tempData=_customSceneData_tempDataList[currentInstanceIndex];
    cacheData=_cacheDataList[currentInstanceIndex];
    constraintSolver=_constraintSolverList[currentInstanceIndex];
    drawingCont=_drawingContainerList[currentInstanceIndex];
    pointCloudCont=_pointCloudContainerList[currentInstanceIndex];
    ghostObjectCont=_ghostObjectContainerList[currentInstanceIndex];
    bannerCont=_bannerContainerList[currentInstanceIndex];
    dynamicsContainer=_dynamicsContainerList[currentInstanceIndex];
    signalContainer=_signalContainerList[currentInstanceIndex];
    commTubeContainer=_commTubeContainerList[currentInstanceIndex];
    ikGroups=_ikGroupList[currentInstanceIndex];
    objCont=_objContList[currentInstanceIndex];

    if (luaScriptContainer!=nullptr)
        luaScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_afterinstanceswitch,nullptr,nullptr,nullptr);
    addOnScriptContainer->handleAddOnScriptExecution(sim_syscb_afterinstanceswitch,nullptr,nullptr);
    if (sandboxScript!=nullptr)
        sandboxScript->runSandboxScript(sim_syscb_afterinstanceswitch,nullptr,nullptr);

    int pluginData[4]={currentInstanceIndex,environment->getSceneUniqueID(),0,0};
    void* pluginReturnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instanceswitch,pluginData,nullptr,nullptr);
    delete[] (char*)pluginReturnVal;
    setModificationFlag(64); // instance switched
//    printf("Instance SWITCHED to scene unique ID: %i\n",pluginData[1]);

#ifdef SIM_WITH_GUI
    if (App::mainWindow!=nullptr)
    {
        App::mainWindow->editModeContainer->announceSceneInstanceChanged();
        if (VThread::isCurrentThreadTheUiThread())
        { // We are in the UI thread. We execute the command now:
            App::mainWindow->setOpenGlDisplayEnabled(true);
        }
        else
        { // We are not in the UI thread. Execute the command via the UI thread:
            SUIThreadCommand cmdIn;
            SUIThreadCommand cmdOut;
            cmdIn.cmdId=VISUALIZATION_ON_UITHREADCMD;
            App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
        }
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=INSTANCE_HAS_CHANGE_UITHREADCMD;
        cmdIn.intParams.push_back(instanceIndex);
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }
#endif

    C3DObject::incrementModelPropertyValidityNumber();

    return(true);
}

bool CMainContainer::isInstanceSwitchingLocked()
{
    if (simulation==nullptr)
        return(false);
    if (!simulation->isSimulationStopped())
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

void CMainContainer::getAllSceneNames(std::vector<std::string>& l)
{
    l.clear();
    for (size_t i=0;i<_mainSettingsList.size();i++)
        l.push_back(VVarious::splitPath_fileBase(_mainSettingsList[i]->getScenePathAndName()));
}

void CMainContainer::renderYourGeneralObject3DStuff_beforeRegularObjects(CViewableBase* renderingObject,int displayAttrib,int windowSize[2],float verticalViewSizeOrAngle,bool perspective)
{ // Render here things that are not transparent and not overlay
    distances->renderYour3DStuff(renderingObject,displayAttrib);
    drawingCont->renderYour3DStuff_nonTransparent(renderingObject,displayAttrib);
    pointCloudCont->renderYour3DStuff_nonTransparent(renderingObject,displayAttrib);
    ghostObjectCont->renderYour3DStuff_nonTransparent(renderingObject,displayAttrib);
    bannerCont->renderYour3DStuff_nonTransparent(renderingObject,displayAttrib,windowSize,verticalViewSizeOrAngle,perspective);

    dynamicsContainer->renderYour3DStuff(renderingObject,displayAttrib);
    buttonBlockContainer->renderYour3DStuff(renderingObject,displayAttrib);
    constraintSolver->renderYour3DStuff(renderingObject,displayAttrib);
    customSceneData->renderYour3DStuff(renderingObject,displayAttrib);
    customSceneData_tempData->renderYour3DStuff(renderingObject,displayAttrib);
    signalContainer->renderYour3DStuff(renderingObject,displayAttrib);
    environment->renderYour3DStuff(renderingObject,displayAttrib);
    luaScriptContainer->renderYour3DStuff(renderingObject,displayAttrib);
    mainSettings->renderYour3DStuff(renderingObject,displayAttrib);
    confContainer->renderYour3DStuff(renderingObject,displayAttrib);
    objCont->renderYour3DStuff(renderingObject,displayAttrib);
    pageContainer->renderYour3DStuff(renderingObject,displayAttrib);
    collections->renderYour3DStuff(renderingObject,displayAttrib);
    ikGroups->renderYour3DStuff(renderingObject,displayAttrib);
    pathPlanning->renderYour3DStuff(renderingObject,displayAttrib);
    motionPlanning->renderYour3DStuff(renderingObject,displayAttrib);
    simulation->renderYour3DStuff(renderingObject,displayAttrib);
    textureCont->renderYour3DStuff(renderingObject,displayAttrib);
    undoBufferContainer->renderYour3DStuff(renderingObject,displayAttrib);
    commTubeContainer->renderYour3DStuff(renderingObject,displayAttrib);
}

void CMainContainer::renderYourGeneralObject3DStuff_afterRegularObjects(CViewableBase* renderingObject,int displayAttrib,int windowSize[2],float verticalViewSizeOrAngle,bool perspective)
{ // Render here things supposed to be transparent
    drawingCont->renderYour3DStuff_transparent(renderingObject,displayAttrib);
    pointCloudCont->renderYour3DStuff_transparent(renderingObject,displayAttrib);
    ghostObjectCont->renderYour3DStuff_transparent(renderingObject,displayAttrib);
    bannerCont->renderYour3DStuff_transparent(renderingObject,displayAttrib,windowSize,verticalViewSizeOrAngle,perspective);
}

void CMainContainer::renderYourGeneralObject3DStuff_onTopOfRegularObjects(CViewableBase* renderingObject,int displayAttrib,int windowSize[2],float verticalViewSizeOrAngle,bool perspective)
{ // Render here things supposed to be overlay
    drawingCont->renderYour3DStuff_overlay(renderingObject,displayAttrib);
    pointCloudCont->renderYour3DStuff_overlay(renderingObject,displayAttrib);
    ghostObjectCont->renderYour3DStuff_overlay(renderingObject,displayAttrib);
    bannerCont->renderYour3DStuff_overlay(renderingObject,displayAttrib,windowSize,verticalViewSizeOrAngle,perspective);
    collisions->renderYour3DStuff(renderingObject,displayAttrib);
    dynamicsContainer->renderYour3DStuff_overlay(renderingObject,displayAttrib);
}

#ifdef SIM_WITH_GUI
void CMainContainer::addMenu(VMenu* menu)
{ // GUI THREAD only
    FUNCTION_DEBUG;
    bool enabled=(!isInstanceSwitchingLocked())&&simulation->isSimulationStopped()&&(!App::mainWindow->oglSurface->isPageSelectionActive())&&(!App::mainWindow->oglSurface->isViewSelectionActive())&&(App::getEditModeType()==NO_EDIT_MODE);

    for (size_t i=0;i<_collisionList.size();i++)
    {
        std::string txt=_mainSettingsList[i]->getSceneName();
        if (txt=="")
            txt="new scene";
        txt+=tt::decorateString(" (scene ",tt::FNb(int(i)+1),")");
        menu->appendMenuItem(enabled,currentInstanceIndex==int(i),SWITCH_TOINSTANCEWITHTHUMBNAILSAVEINDEX0_GUIGUICMD+int(i),txt,true);
    }
}

void CMainContainer::keyPress(int key)
{
    FUNCTION_DEBUG;
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

int CMainContainer::getInstanceIndexOfASceneNotYetSaved(bool doNotIncludeCurrentScene)
{
    for (int i=0;i<getInstanceCount();i++)
    {
        if ( (!doNotIncludeCurrentScene)||(getCurrentInstanceIndex()!=i) )
        {
            if (_undoBufferContainerList[i]->isSceneSaveMaybeNeededFlagSet())
                return(i);
        }
    }
    return(-1);
}

void CMainContainer::setInstanceIndexWithThumbnails(int index)
{ // GUI THREAD only
    FUNCTION_DEBUG;
    if ( (App::mainWindow!=nullptr)&&(!App::userSettings->doNotShowSceneSelectionThumbnails) )
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=SWITCH_TOINSTANCEINDEX_GUITRIGGEREDCMD;
        cmd.intParams.push_back(index);
        App::mainWindow->prepareSceneThumbnail(cmd);
    }
    else
        App::appendSimulationThreadCommand(SWITCH_TOINSTANCEINDEX_GUITRIGGEREDCMD,index);
}

bool CMainContainer::processGuiCommand(int commandID)
{ // GUI THREAD only. Return value is true if the command belonged to object edition menu and was executed
    FUNCTION_DEBUG;

    if ( (commandID>=SWITCH_TOINSTANCEWITHTHUMBNAILSAVEINDEX0_GUIGUICMD)&&(commandID<=SWITCH_TOINSTANCEWITHTHUMBNAILSAVEINDEX39_GUIGUICMD) )
    {
        setInstanceIndexWithThumbnails(commandID-SWITCH_TOINSTANCEWITHTHUMBNAILSAVEINDEX0_GUIGUICMD);
        return(true);
    }
    return(false);
}

#endif
