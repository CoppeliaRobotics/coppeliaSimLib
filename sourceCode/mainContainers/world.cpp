#include "world.h"
#include "pluginContainer.h"
#include "mesh.h"
#include "ttUtil.h"
#include "tt.h"
#include "app.h"

std::vector<SLoadOperationIssue> CWorld::_loadOperationIssues;

CWorld::CWorld()
{
    commTubeContainer=nullptr;
    signalContainer=nullptr;
    dynamicsContainer=nullptr;
    undoBufferContainer=nullptr;
    outsideCommandQueue=nullptr;
    buttonBlockContainer=nullptr;
    environment=nullptr;
    pageContainer=nullptr;
    mainSettings=nullptr;
    pathPlanning=nullptr;
    luaScriptContainer=nullptr;
    textureContainer=nullptr;
    simulation=nullptr;
    customSceneData=nullptr;
    customSceneData_tempData=nullptr;
    cacheData=nullptr;
    drawingCont=nullptr;
    pointCloudCont=nullptr;
    ghostObjectCont=nullptr;
    bannerCont=nullptr;
}

CWorld::~CWorld()
{
}

void CWorld::removeRemoteWorlds()
{
    // Remote worlds:
    sendVoid(sim_syncobj_world_empty);

    // IK plugin world:
    CPluginContainer::ikPlugin_emptyEnvironment();

    // Local references to remote worlds:
    sceneObjects->removeSynchronizationObjects(true);
    ikGroups->removeSynchronizationObjects(true);
    collections->removeSynchronizationObjects(true);
    collisions->removeSynchronizationObjects(true);
    distances->removeSynchronizationObjects(true);
}

void CWorld::initializeWorld()
{
    undoBufferContainer=new CUndoBufferCont();
    outsideCommandQueue=new COutsideCommandQueue();
    buttonBlockContainer=new CButtonBlockContainer(true);
    simulation=new CSimulation();
    textureContainer=new CTextureContainer();
    luaScriptContainer=new CLuaScriptContainer();

    _CWorld_::initializeWorld();

    pathPlanning=new CRegisteredPathPlanningTasks();
    environment=new CEnvironment();
    pageContainer=new CPageContainer();
    mainSettings=new CMainSettings();
    customSceneData=new CCustomData();
    customSceneData_tempData=new CCustomData();
    cacheData=new CCacheCont();
    drawingCont=new CDrawingContainer();
    pointCloudCont=new CPointCloudContainer_old();
    ghostObjectCont=new CGhostObjectContainer();
    bannerCont=new CBannerContainer();
    dynamicsContainer=new CDynamicsContainer();
    signalContainer=new CSignalContainer();
    commTubeContainer=new CCommTubeContainer();
}

void CWorld::clearScene(bool notCalledFromUndoFunction)
{
#ifdef SIM_WITH_GUI
    if (App::mainWindow!=nullptr)
        App::mainWindow->codeEditorContainer->sceneClosed(environment->getSceneUniqueID());
#endif

    if (notCalledFromUndoFunction)
        undoBufferContainer->emptySceneProcedure();
    environment->setSceneIsClosingFlag(true); // so that attached scripts can react to it
    // Important to empty objects first (since objCont->announce....willBeErase
    // might be called for already destroyed objects!)
    sceneObjects->removeSceneDependencies();
    collections->removeAllCollections();
    collections->setUpDefaultValues();
    ikGroups->removeAllIkGroups();
    distances->removeAllDistanceObjects();
    distances->setUpDefaultValues();
    collisions->removeAllCollisionObjects();
    collisions->setUpDefaultValues();
    pathPlanning->removeAllTasks();
    luaScriptContainer->removeAllScripts();

    simulation->setUpDefaultValues();
    if (buttonBlockContainer!=nullptr)
        buttonBlockContainer->removeAllBlocks(false);
    environment->setUpDefaultValues();
    pageContainer->emptySceneProcedure();


    sceneObjects->removeAllObjects(true); //false);
    simulation->setUpDefaultValues();
    pageContainer->emptySceneProcedure();

    customSceneData->removeAllData();
    customSceneData_tempData->removeAllData();
    if (notCalledFromUndoFunction)
        mainSettings->setUpDefaultValues();
    cacheData->clearCache();
    environment->setSceneIsClosingFlag(false);
}

void CWorld::deleteWorld()
{
    simulation->setOnlineMode(false); // disable online mode before deleting
    delete undoBufferContainer;
    undoBufferContainer=nullptr;
    delete luaScriptContainer;
    luaScriptContainer=nullptr;
    delete dynamicsContainer;
    dynamicsContainer=nullptr;
    delete mainSettings;
    mainSettings=nullptr;
    delete pageContainer;
    pageContainer=nullptr;
    delete environment;
    environment=nullptr;
    delete pathPlanning;
    pathPlanning=nullptr;

    _CWorld_::deleteWorld();

    delete textureContainer;
    textureContainer=nullptr;
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
}

void CWorld::rebuildRemoteWorlds()
{
    // Build remote world objects:
    sceneObjects->buildUpdateAndPopulateSynchronizationObjects();
    sceneObjects->connectSynchronizationObjects();

    ikGroups->buildUpdateAndPopulateSynchronizationObjects();
    ikGroups->connectSynchronizationObjects();

    collections->buildUpdateAndPopulateSynchronizationObjects();
    collections->connectSynchronizationObjects();

    collisions->buildUpdateAndPopulateSynchronizationObjects();
    collisions->connectSynchronizationObjects();

    distances->buildUpdateAndPopulateSynchronizationObjects();
    distances->connectSynchronizationObjects();

    // selection state:
    std::vector<int> sel(sceneObjects->getSelectedObjectHandlesPtr()[0]);
    sceneObjects->deselectObjects();
    sceneObjects->setSelectedObjectHandles(&sel);
}

bool CWorld::loadScene(CSer& ar,bool forUndoRedoOperation)
{
    bool retVal=false;
    sceneObjects->removeAllObjects(true);
    if (ar.getFileType()==CSer::filetype_csim_xml_simplescene_file)
    {
        retVal=_loadSimpleXmlSceneOrModel(ar);
        sceneObjects->deselectObjects();
    }
    else
    {
        retVal=_loadModelOrScene(ar,false,true,false,false,nullptr,nullptr,nullptr);
        if (!forUndoRedoOperation)
        {
            void* returnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_sceneloaded,nullptr,nullptr,nullptr);
            delete[] (char*)returnVal;
            App::worldContainer->setModificationFlag(8); // scene loaded
            outsideCommandQueue->addCommand(sim_message_scene_loaded,0,0,0,0,nullptr,0);
        }
    }
    return(retVal);
}

void CWorld::saveScene(CSer& ar)
{
    if (ar.getFileType()==CSer::filetype_csim_xml_simplescene_file)
    {
        _saveSimpleXmlScene(ar);
        return;
    }
    // **** Following needed to save existing calculation structures:
    environment->setSaveExistingCalculationStructuresTemp(false);
    if (!undoBufferContainer->isUndoSavingOrRestoringUnderWay())
    { // real saving!
        if (environment->getSaveExistingCalculationStructures())
            environment->setSaveExistingCalculationStructuresTemp(true);
    }
    // ************************************************************

    //***************************************************
    if (ar.isBinary())
    {
        ar.storeDataName(SER_MODEL_THUMBNAIL);
        ar.setCountingMode();
        environment->modelThumbnail_notSerializedHere.serialize(ar,false);
        if (ar.setWritingMode())
           environment->modelThumbnail_notSerializedHere.serialize(ar,false);
    }
    else
    {
        ar.xmlPushNewNode(SERX_MODEL_THUMBNAIL);
        environment->modelThumbnail_notSerializedHere.serialize(ar,false);
        ar.xmlPopNode();
    }
    //****************************************************

    // Textures:
    int textCnt=0;
    while (textureContainer->getObjectAtIndex(textCnt)!=nullptr)
    {
        CTextureObject* it=textureContainer->getObjectAtIndex(textCnt);
        if (ar.isBinary())
            textureContainer->storeTextureObject(ar,it);
        else
        {
            ar.xmlPushNewNode(SERX_TEXTURE);
            textureContainer->storeTextureObject(ar,it);
            ar.xmlPopNode();
        }
        textCnt++;
    }

    // DynMaterial objects:
    // We only save this for backward compatibility, but not needed for CoppeliaSim's from 3.4.0 on:
    //------------------------------------------------------------
    if (ar.isBinary())
    {
        int dynObjId=SIM_IDSTART_DYNMATERIAL_OLD;
        for (size_t i=0;i<sceneObjects->getShapeCount();i++)
        {
            CShape* it=sceneObjects->getShapeFromIndex(i);
            CDynMaterialObject* mat=it->getDynMaterial();
            it->getMeshWrapper()->setDynMaterialId_OLD(dynObjId);
            mat->setObjectID(dynObjId++);
            ar.storeDataName(SER_DYNMATERIAL);
            ar.setCountingMode();
            mat->serialize(ar);
            if (ar.setWritingMode())
                mat->serialize(ar);
        }
    }
    //------------------------------------------------------------

    // Handle the heavy data here so we don't have duplicates (vertices, indices, normals and edges):
    //------------------------------------------------------------
    if (ar.isBinary())
    {
        CMesh::clearTempVerticesIndicesNormalsAndEdges();
        for (size_t i=0;i<sceneObjects->getShapeCount();i++)
        {
            CShape* it=sceneObjects->getShapeFromIndex(i);
            it->prepareVerticesIndicesNormalsAndEdgesForSerialization();
        }
        ar.storeDataName(SER_VERTICESINDICESNORMALSEDGES);
        ar.setCountingMode();
        CMesh::serializeTempVerticesIndicesNormalsAndEdges(ar);
        if (ar.setWritingMode())
            CMesh::serializeTempVerticesIndicesNormalsAndEdges(ar);
        CMesh::clearTempVerticesIndicesNormalsAndEdges();
    }
    //------------------------------------------------------------

    for (size_t i=0;i<sceneObjects->getObjectCount();i++)
    {
        CSceneObject* it=sceneObjects->getObjectFromIndex(i);
        if (ar.isBinary())
            sceneObjects->writeSceneObject(ar,it);
        else
        {
            ar.xmlPushNewNode(SERX_SCENEOBJECT);
            sceneObjects->writeSceneObject(ar,it);
            ar.xmlPopNode();
        }
    }

    if (ar.isBinary())
    {
        ar.storeDataName(SER_GHOSTS);
        ar.setCountingMode();
        ghostObjectCont->serialize(ar);
        if (ar.setWritingMode())
            ghostObjectCont->serialize(ar);
    }
    else
    {
        ar.xmlPushNewNode(SERX_GHOSTS);
        ghostObjectCont->serialize(ar);
        ar.xmlPopNode();
    }

    if (ar.isBinary())
    {
        ar.storeDataName(SER_ENVIRONMENT);
        ar.setCountingMode();
        environment->serialize(ar);
        if (ar.setWritingMode())
            environment->serialize(ar);
    }
    else
    {
        ar.xmlPushNewNode(SERX_ENVIRONMENT);
        environment->serialize(ar);
        ar.xmlPopNode();
    }


    for (size_t i=0;i<collisions->getObjectCount();i++)
    {
        CCollisionObject* collObj=collisions->getObjectFromIndex(i);
        if (ar.isBinary())
        {
            ar.storeDataName(SER_COLLISION);
            ar.setCountingMode();
            collObj->serialize(ar);
            if (ar.setWritingMode())
                collObj->serialize(ar);
        }
        else
        {
            ar.xmlPushNewNode(SERX_COLLISION);
            collObj->serialize(ar);
            ar.xmlPopNode();
        }
    }
    for (size_t i=0;i<distances->getObjectCount();i++)
    {
        CDistanceObject* distObj=distances->getObjectFromIndex(i);
        if (ar.isBinary())
        {
            ar.storeDataName(SER_DISTANCE);
            ar.setCountingMode();
            distObj->serialize(ar);
            if (ar.setWritingMode())
                distObj->serialize(ar);
        }
        else
        {
            ar.xmlPushNewNode(SERX_DISTANCE);
            distObj->serialize(ar);
            ar.xmlPopNode();
        }
    }
    for (size_t i=0;i<ikGroups->getObjectCount();i++)
    {
        CIkGroup* ikg=ikGroups->getObjectFromIndex(i);
        if (ar.isBinary())
        {
            ar.storeDataName(SER_IK);
            ar.setCountingMode();
            ikg->serialize(ar);
            if (ar.setWritingMode())
                ikg->serialize(ar);
        }
        else
        {
            ar.xmlPushNewNode(SERX_IK);
            ikg->serialize(ar);
            ar.xmlPopNode();
        }
    }
    if (ar.isBinary())
    { // deprecated functionality
        for (size_t i=0;i<pathPlanning->allObjects.size();i++)
        {
            ar.storeDataName(SER_PATH_PLANNING);
            ar.setCountingMode();
            pathPlanning->allObjects[i]->serialize(ar);
            if (ar.setWritingMode())
                pathPlanning->allObjects[i]->serialize(ar);
        }
    }

    if (ar.isBinary())
    {
        ar.storeDataName(SER_SETTINGS);
        ar.setCountingMode();
        mainSettings->serialize(ar);
        if (ar.setWritingMode())
            mainSettings->serialize(ar);
    }
    else
    {
        ar.xmlPushNewNode(SERX_SETTINGS);
        mainSettings->serialize(ar);
        ar.xmlPopNode();
    }

    if (ar.isBinary())
    {
        ar.storeDataName(SER_DYNAMICS);
        ar.setCountingMode();
        dynamicsContainer->serialize(ar);
        if (ar.setWritingMode())
            dynamicsContainer->serialize(ar);
    }
    else
    {
        ar.xmlPushNewNode(SERX_DYNAMICS);
        dynamicsContainer->serialize(ar);
        ar.xmlPopNode();
    }

    if (ar.isBinary())
    {
        ar.storeDataName(SER_SIMULATION);
        ar.setCountingMode();
        simulation->serialize(ar);
        if (ar.setWritingMode())
            simulation->serialize(ar);
    }
    else
    {
        ar.xmlPushNewNode(SERX_SIMULATION);
        simulation->serialize(ar);
        ar.xmlPopNode();
    }
    if (ar.isBinary())
    {
        ar.storeDataName(SER_SCENE_CUSTOM_DATA);
        ar.setCountingMode();
        customSceneData->serializeData(ar,nullptr,-1);
        if (ar.setWritingMode())
            customSceneData->serializeData(ar,nullptr,-1);
    }
    else
    {
        ar.xmlPushNewNode(SERX_SCENE_CUSTOM_DATA);
        customSceneData->serializeData(ar,nullptr,-1);
        ar.xmlPopNode();
    }

    if (ar.isBinary())
    {
        ar.storeDataName(SER_VIEWS);
        ar.setCountingMode();
        pageContainer->serialize(ar);
        if (ar.setWritingMode())
            pageContainer->serialize(ar);
    }
    else
    {
        ar.xmlPushNewNode(SERX_VIEWS);
        pageContainer->serialize(ar);
        ar.xmlPopNode();
    }

    // We serialize all collections:
    for (size_t i=0;i<collections->getObjectCount();i++)
    {
        if (ar.isBinary())
        {
            ar.storeDataName(SER_COLLECTION);
            ar.setCountingMode();
            collections->getObjectFromIndex(i)->serialize(ar);
            if (ar.setWritingMode())
                collections->getObjectFromIndex(i)->serialize(ar);
        }
        else
        {
            ar.xmlPushNewNode(SERX_COLLECTION);
            collections->getObjectFromIndex(i)->serialize(ar);
            ar.xmlPopNode();
        }
    }

    if ( ar.isBinary()&&(!App::userSettings->disableOpenGlBasedCustomUi) )
    { // deprecated functionality
        // We serialize the buttonBlocks (non-system):
        for (size_t i=0;i<buttonBlockContainer->allBlocks.size();i++)
        {
            CButtonBlock* bblk=buttonBlockContainer->allBlocks[i];
            if ((bblk->getAttributes()&sim_ui_property_systemblock)==0)
            {
                ar.storeDataName(SER_BUTTON_BLOCK_OLD);
                ar.setCountingMode();
                bblk->serialize(ar);
                if (ar.setWritingMode())
                    bblk->serialize(ar);
            }
        }
    }

    // We serialize the lua script objects (not the add-on scripts nor the sandbox script):
    for (size_t i=0;i<luaScriptContainer->allScripts.size();i++)
    {
        CLuaScriptObject* it=luaScriptContainer->allScripts[i];
        if (it->isSceneScript())
        {
            if (ar.isBinary())
            {
                ar.storeDataName(SER_LUA_SCRIPT);
                ar.setCountingMode();
                it->serialize(ar);
                if (ar.setWritingMode())
                    it->serialize(ar);
            }
            else
            {
                ar.xmlPushNewNode(SERX_LUA_SCRIPT);
                it->serialize(ar);
                ar.xmlPopNode();
            }
        }
    }

    if (ar.isBinary())
        ar.storeDataName(SER_END_OF_FILE);
    CMesh::clearTempVerticesIndicesNormalsAndEdges();
}

bool CWorld::loadModel(CSer& ar,bool justLoadThumbnail,bool forceModelAsCopy,C7Vector* optionalModelTr,C3Vector* optionalModelBoundingBoxSize,float* optionalModelNonDefaultTranslationStepSize)
{
    bool retVal;
    if (ar.getFileType()==CSer::filetype_csim_xml_simplemodel_file)
        retVal=_loadSimpleXmlSceneOrModel(ar);
    else
        retVal=_loadModelOrScene(ar,true,false,justLoadThumbnail,forceModelAsCopy,optionalModelTr,optionalModelBoundingBoxSize,optionalModelNonDefaultTranslationStepSize);
    if (!justLoadThumbnail)
    {
        void* returnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_modelloaded,nullptr,nullptr,nullptr);
        delete[] (char*)returnVal;
        returnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_model_loaded,nullptr,nullptr,nullptr); // for backward compatibility
        delete[] (char*)returnVal;

        outsideCommandQueue->addCommand(sim_message_model_loaded,0,0,0,0,nullptr,0); // only for Lua
        App::worldContainer->setModificationFlag(4); // model loaded
    }
    return(retVal);
}

void CWorld::simulationAboutToStart()
{
    TRACE_INTERNAL;

#ifdef SIM_WITH_GUI
    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId=SIMULATION_ABOUT_TO_START_UITHREADCMD;
    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
#endif

    luaScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_beforesimulation,nullptr,nullptr,nullptr);
    App::worldContainer->addOnScriptContainer->handleAddOnScriptExecution(sim_syscb_beforesimulation,nullptr,nullptr);
    if (App::worldContainer->sandboxScript!=nullptr)
        App::worldContainer->sandboxScript->runSandboxScript(sim_syscb_beforesimulation,nullptr,nullptr);

    _initialObjectUniqueIdentifiersForRemovingNewObjects.clear();
    for (size_t i=0;i<sceneObjects->getObjectCount();i++)
    {
        CSceneObject* it=sceneObjects->getObjectFromIndex(i);
        _initialObjectUniqueIdentifiersForRemovingNewObjects.push_back(it->getUniqueID());
    }
    POST_SCENE_CHANGED_ANNOUNCEMENT("");

    _savedMouseMode=App::getMouseMode();

#ifdef SIM_WITH_GUI
    if (App::mainWindow!=nullptr)
        App::mainWindow->codeEditorContainer->simulationAboutToStart();
#endif

    if (!CPluginContainer::isGeomPluginAvailable())
    {
#ifdef SIM_WITH_GUI
        simDisplayDialog_internal("ERROR","The 'Geometric' plugin could not be initialized. Collision detection, distance calculation,\n and proximity sensor simulation will not work.",sim_dlgstyle_ok,"",nullptr,nullptr,nullptr);
#endif
        App::logMsg(sim_verbosity_errors,"the 'MeshCalc' plugin could not be initialized. Collision detection,\n       distance calculation, and proximity sensor simulation will not work.");
    }

    _simulationAboutToStart();
    App::worldContainer->calcInfo->simulationAboutToStart();

    void* retVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_simulationabouttostart,nullptr,nullptr,nullptr);
    delete[] (char*)retVal;

    App::worldContainer->setModificationFlag(2048); // simulation started

    SSimulationThreadCommand cmd;
    cmd.cmdId=DISPLAY_VARIOUS_WARNING_MESSAGES_DURING_SIMULATION_CMD;
    App::appendSimulationThreadCommand(cmd,1000);

    App::setToolbarRefreshFlag();
    App::setFullDialogRefreshFlag();

#ifdef SIM_WITH_GUI
    if (App::mainWindow!=nullptr)
        App::mainWindow->simulationRecorder->startRecording(false);
#endif
}

void CWorld::simulationPaused()
{
    _simulationPaused();
    App::setToolbarRefreshFlag();
    App::setFullDialogRefreshFlag();
}

void CWorld::simulationAboutToResume()
{
    _simulationAboutToResume();
    App::setToolbarRefreshFlag();
    App::setFullDialogRefreshFlag();
}

void CWorld::simulationAboutToStep()
{
    _simulationAboutToStep();
}

void CWorld::simulationAboutToEnd()
{
    TRACE_INTERNAL;

    void* retVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_simulationabouttoend,nullptr,nullptr,nullptr);
    delete[] (char*)retVal;

    _simulationAboutToEnd();

#ifdef SIM_WITH_GUI
    if (App::mainWindow!=nullptr)
        App::mainWindow->codeEditorContainer->simulationAboutToEnd();
#endif
}

void CWorld::simulationEnded(bool removeNewObjects)
{
    TRACE_INTERNAL;
    POST_SCENE_CHANGED_ANNOUNCEMENT(""); // keeps this (this has the objects in their last position, including additional objects)

    void* retVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_simulationended,nullptr,nullptr,nullptr);
    delete[] (char*)retVal;
    App::worldContainer->setModificationFlag(4096); // simulation ended

    _simulationEnded();
    App::worldContainer->calcInfo->simulationEnded();

#ifdef SIM_WITH_SERIAL
    App::worldContainer->serialPortContainer->simulationEnded();
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
        const std::vector<int>* savedSelection=sceneObjects->getSelectedObjectHandlesPtr();
        //sceneObjects->deselectObjects();
        std::vector<CSceneObject*> toRemove;
        for (size_t i=0;i<sceneObjects->getObjectCount();i++)
        {
            CSceneObject* it=sceneObjects->getObjectFromIndex(i);
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
                toRemove.push_back(it);
        }
        sceneObjects->eraseSeveralObjects(toRemove,true);
        sceneObjects->setSelectedObjectHandles(savedSelection);
    }
    _initialObjectUniqueIdentifiersForRemovingNewObjects.clear();
    POST_SCENE_CHANGED_ANNOUNCEMENT(""); // keeps this (additional objects were removed, and object positions were reset)

    luaScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_aftersimulation,nullptr,nullptr,nullptr);
    App::worldContainer->addOnScriptContainer->handleAddOnScriptExecution(sim_syscb_aftersimulation,nullptr,nullptr);
    if (App::worldContainer->sandboxScript!=nullptr)
        App::worldContainer->sandboxScript->runSandboxScript(sim_syscb_aftersimulation,nullptr,nullptr);
}

void CWorld::setEnableRemoteWorldsSync(bool enabled)
{
    CSyncObject::setOverallSyncEnabled(enabled);
}

void CWorld::addGeneralObjectsToWorldAndPerformMappings(std::vector<CSceneObject*>* loadedObjectList,
                                                    std::vector<CCollection*>* loadedCollectionList,
                                                    std::vector<CCollisionObject*>* loadedCollisionList,
                                                    std::vector<CDistanceObject*>* loadedDistanceList,
                                                    std::vector<CIkGroup*>* loadedIkGroupList,
                                                    std::vector<CPathPlanningTask*>* loadedPathPlanningTaskList,
                                                    std::vector<CButtonBlock*>* loadedButtonBlockList,
                                                    std::vector<CLuaScriptObject*>* loadedLuaScriptList,
                                                    std::vector<CTextureObject*>& loadedTextureObjectList,
                                                    std::vector<CDynMaterialObject*>& loadedDynMaterialObjectList,
                                                    bool model,int fileSimVersion,bool forceModelAsCopy)
{
    TRACE_INTERNAL;
    // We check what suffix offset is needed for this model (in case of a scene, the offset is ignored since we won't introduce the objects as copies!):
    int suffixOffset=_getSuffixOffsetForGeneralObjectToAdd(false,loadedObjectList,loadedCollectionList,loadedCollisionList,loadedDistanceList,loadedIkGroupList,loadedPathPlanningTaskList,loadedButtonBlockList,loadedLuaScriptList);

    // We have 3 cases:
    // 1. We are loading a scene, 2. We are loading a model, 3. We are pasting objects
    // We add objects to the scene as copies only if we also add at least one associated script and we don't have a scene. Otherwise objects are added
    // and no '#' (or no modified suffix) will appear in their names.
    // Following line summarizes this:
    bool objectIsACopy=(((loadedLuaScriptList->size()!=0)||forceModelAsCopy)&&model); // scenes are not treated like copies!

    // Texture data:
    std::vector<int> textureMapping;
    for (size_t i=0;i<loadedTextureObjectList.size();i++)
    {
        textureMapping.push_back(loadedTextureObjectList[i]->getObjectID());
        CTextureObject* handler=textureContainer->getObject(textureContainer->addObjectWithSuffixOffset(loadedTextureObjectList[i],objectIsACopy,suffixOffset)); // if a same object is found, the object is destroyed in addObject!
        if (handler!=loadedTextureObjectList[i])
            loadedTextureObjectList[i]=handler; // this happens when a similar object is already present
        textureMapping.push_back(handler->getObjectID());
    }
    _prepareFastLoadingMapping(textureMapping);


    setEnableRemoteWorldsSync(false); // do not trigger object creation in plugins, etc. when adding objects to world

    // We add all sceneObjects:
    sceneObjects->enableObjectActualization(false);
    std::vector<int> objectMapping;
    for (size_t i=0;i<loadedObjectList->size();i++)
    {
        objectMapping.push_back(loadedObjectList->at(i)->getObjectHandle()); // Old ID
        sceneObjects->addObjectToSceneWithSuffixOffset(loadedObjectList->at(i),objectIsACopy,suffixOffset,false);
        objectMapping.push_back(loadedObjectList->at(i)->getObjectHandle()); // New ID

        if (loadedObjectList->at(i)->getObjectType()==sim_object_shape_type)
        {
            CShape* shape=(CShape*)loadedObjectList->at(i);
            int matId=shape->getMeshWrapper()->getDynMaterialId_OLD();
            if ((fileSimVersion<30303)&&(matId>=0))
            { // for backward compatibility(29/10/2016), when the dyn material was stored separaterly and shared among shapes
                for (size_t j=0;j<loadedDynMaterialObjectList.size();j++)
                {
                    if (loadedDynMaterialObjectList[j]->getObjectID()==matId)
                    {
                        CDynMaterialObject* mat=loadedDynMaterialObjectList[j]->copyYourself();
                        shape->setDynMaterial(mat);
                        break;
                    }
                }
            }
            if (fileSimVersion<30301)
            { // Following for backward compatibility (09/03/2016)
                CDynMaterialObject* mat=shape->getDynMaterial();
                if (mat->getEngineBoolParam(sim_bullet_body_sticky,nullptr))
                { // Formely sticky contact objects need to be adjusted for the new Bullet:
                    if (shape->getShapeIsDynamicallyStatic())
                        mat->setEngineFloatParam(sim_bullet_body_friction,mat->getEngineFloatParam(sim_bullet_body_oldfriction,nullptr)); // the new Bullet friction
                    else
                        mat->setEngineFloatParam(sim_bullet_body_friction,0.25f); // the new Bullet friction
                }
            }
            shape->getMeshWrapper()->setDynMaterialId_OLD(-1);
        }
    }
    _prepareFastLoadingMapping(objectMapping);
    sceneObjects->enableObjectActualization(true);
    sceneObjects->actualizeObjectInformation();

    // Remove any material that was loaded from a previous file version, where materials were still shared (until V3.3.2)
    for (size_t i=0;i<loadedDynMaterialObjectList.size();i++)
        delete loadedDynMaterialObjectList[i];

    // We add all the collections:
    std::vector<int> collectionMapping;
    for (size_t i=0;i<loadedCollectionList->size();i++)
    {
        collectionMapping.push_back(loadedCollectionList->at(i)->getCollectionHandle()); // Old ID
        collections->addCollectionWithSuffixOffset(loadedCollectionList->at(i),objectIsACopy,suffixOffset);
        collectionMapping.push_back(loadedCollectionList->at(i)->getCollectionHandle()); // New ID
    }
    _prepareFastLoadingMapping(collectionMapping);

    // We add all the collisions:
    std::vector<int> collisionMapping;
    for (size_t i=0;i<loadedCollisionList->size();i++)
    {
        collisionMapping.push_back(loadedCollisionList->at(i)->getObjectHandle()); // Old ID
        collisions->addObjectWithSuffixOffset(loadedCollisionList->at(i),objectIsACopy,suffixOffset);
        collisionMapping.push_back(loadedCollisionList->at(i)->getObjectHandle()); // New ID
    }
    _prepareFastLoadingMapping(collisionMapping);

    // We add all the distances:
    std::vector<int> distanceMapping;
    for (size_t i=0;i<loadedDistanceList->size();i++)
    {
        distanceMapping.push_back(loadedDistanceList->at(i)->getObjectHandle()); // Old ID
        distances->addObjectWithSuffixOffset(loadedDistanceList->at(i),objectIsACopy,suffixOffset);
        distanceMapping.push_back(loadedDistanceList->at(i)->getObjectHandle()); // New ID
    }
    _prepareFastLoadingMapping(distanceMapping);

    // We add all the ik groups:
    std::vector<int> ikGroupMapping;
    for (size_t i=0;i<loadedIkGroupList->size();i++)
    {
        ikGroupMapping.push_back(loadedIkGroupList->at(i)->getObjectHandle()); // Old ID
        ikGroups->addIkGroupWithSuffixOffset(loadedIkGroupList->at(i),objectIsACopy,suffixOffset);
        ikGroupMapping.push_back(loadedIkGroupList->at(i)->getObjectHandle()); // New ID
    }
    _prepareFastLoadingMapping(ikGroupMapping);

    // We add all the path planning tasks:
    std::vector<int> pathPlanningTaskMapping;
    for (size_t i=0;i<loadedPathPlanningTaskList->size();i++)
    {
        pathPlanningTaskMapping.push_back(loadedPathPlanningTaskList->at(i)->getObjectID()); // Old ID
        pathPlanning->addObjectWithSuffixOffset(loadedPathPlanningTaskList->at(i),objectIsACopy,suffixOffset);
        pathPlanningTaskMapping.push_back(loadedPathPlanningTaskList->at(i)->getObjectID()); // New ID
    }
    _prepareFastLoadingMapping(pathPlanningTaskMapping);

    // We add all the button blocks:
    std::vector<int> buttonBlockMapping;
    for (size_t i=0;i<loadedButtonBlockList->size();i++)
    {
        buttonBlockMapping.push_back(loadedButtonBlockList->at(i)->getBlockID()); // Old ID
        buttonBlockContainer->insertBlockWithSuffixOffset(loadedButtonBlockList->at(i),objectIsACopy,suffixOffset);
        buttonBlockMapping.push_back(loadedButtonBlockList->at(i)->getBlockID()); // New ID
    }
    _prepareFastLoadingMapping(buttonBlockMapping);

    // We add all the scripts:
    std::vector<int> luaScriptMapping;
    for (size_t i=0;i<loadedLuaScriptList->size();i++)
    {
        luaScriptMapping.push_back(loadedLuaScriptList->at(i)->getScriptID()); // Old ID
        luaScriptContainer->insertScript(loadedLuaScriptList->at(i));
        luaScriptMapping.push_back(loadedLuaScriptList->at(i)->getScriptID()); // New ID
    }
    _prepareFastLoadingMapping(luaScriptMapping);


    sceneObjects->enableObjectActualization(false);

    // We do the mapping for the sceneObjects:
    for (size_t i=0;i<loadedObjectList->size();i++)
    {
        CSceneObject* it=loadedObjectList->at(i);
        it->performObjectLoadingMapping(&objectMapping,model);
        it->performCollectionLoadingMapping(&collectionMapping,model);
        it->performCollisionLoadingMapping(&collisionMapping,model);
        it->performDistanceLoadingMapping(&distanceMapping,model);
        it->performIkLoadingMapping(&ikGroupMapping,model);
        it->performTextureObjectLoadingMapping(&textureMapping);
    }
    // We do the mapping for the collections:
    for (size_t i=0;i<loadedCollectionList->size();i++)
    {
        CCollection* it=loadedCollectionList->at(i);
        it->performObjectLoadingMapping(&objectMapping);
    }
    // We do the mapping for the collisions:
    for (size_t i=0;i<loadedCollisionList->size();i++)
    {
        CCollisionObject* it=loadedCollisionList->at(i);
        it->performObjectLoadingMapping(&objectMapping);
        it->performCollectionLoadingMapping(&collectionMapping);
    }
    // We do the mapping for the distances:
    for (size_t i=0;i<loadedDistanceList->size();i++)
    {
        CDistanceObject* it=loadedDistanceList->at(i);
        it->performObjectLoadingMapping(&objectMapping);
        it->performCollectionLoadingMapping(&collectionMapping);
    }
    // We do the mapping for the ik groups:
    for (size_t i=0;i<loadedIkGroupList->size();i++)
    {
        CIkGroup* it=loadedIkGroupList->at(i);
        it->performObjectLoadingMapping(&objectMapping);
        it->performIkGroupLoadingMapping(&ikGroupMapping);
    }
    // We do the mapping for the path planning tasks:
    for (size_t i=0;i<loadedPathPlanningTaskList->size();i++)
    {
        CPathPlanningTask* it=loadedPathPlanningTaskList->at(i);
        it->performObjectLoadingMapping(&objectMapping);
        it->performCollectionLoadingMapping(&collectionMapping);
    }
    // We do the mapping for the 2D Elements:
    for (size_t i=0;i<loadedButtonBlockList->size();i++)
    {
        CButtonBlock* it=loadedButtonBlockList->at(i);
        it->performSceneObjectLoadingMapping(&objectMapping);
        it->performTextureObjectLoadingMapping(&textureMapping);
    }
    // We do the mapping for the Lua scripts:
    for (size_t i=0;i<loadedLuaScriptList->size();i++)
    {
        CLuaScriptObject* it=loadedLuaScriptList->at(i);
        it->performSceneObjectLoadingMapping(&objectMapping);
    }

    // We do the mapping for the ghost objects:
    if (!model)
        ghostObjectCont->performObjectLoadingMapping(&objectMapping);

    // We set ALL texture object dependencies (not just for loaded objects):
    // We cannot use textureCont->updateAllDependencies, since the shape list is not yet actualized!
    textureContainer->clearAllDependencies();
    buttonBlockContainer->setTextureDependencies();
    sceneObjects->setTextureDependencies();

    sceneObjects->enableObjectActualization(true);
    sceneObjects->actualizeObjectInformation();

    if (!model)
        pageContainer->performObjectLoadingMapping(&objectMapping);

    //sceneObjects->actualizeObjectInformation();

    // Now clean-up suffixes equal or above those added, but only for models or objects copied into the scene (global suffix clean-up can be done in the environment dialog):
    if (model) // condition was added on 29/9/2014
        cleanupHashNames_allObjects(suffixOffset-1);

/* Until 4/10/2013. Global suffix name clean-up. This was confusing!
    if (App::wc->simulation->isSimulationStopped()) // added on 2010/02/20 (otherwise objects can get automatically renamed during simulation!!)
        cleanupHashNames(-1);
*/

//************ We need to initialize all object types (also because an object copied during simulation hasn't the simulationEnded routine called!)
    bool simulationRunning=!simulation->isSimulationStopped();
    for (size_t i=0;i<loadedObjectList->size();i++)
        loadedObjectList->at(i)->initializeInitialValues(simulationRunning);
    for (size_t i=0;i<loadedButtonBlockList->size();i++)
        loadedButtonBlockList->at(i)->initializeInitialValues(simulationRunning);
    for (size_t i=0;i<loadedCollisionList->size();i++)
        loadedCollisionList->at(i)->initializeInitialValues(simulationRunning);
    for (size_t i=0;i<loadedDistanceList->size();i++)
        loadedDistanceList->at(i)->initializeInitialValues(simulationRunning);
    for (size_t i=0;i<loadedCollectionList->size();i++)
        loadedCollectionList->at(i)->initializeInitialValues(simulationRunning);
    for (size_t i=0;i<loadedIkGroupList->size();i++)
        loadedIkGroupList->at(i)->initializeInitialValues(simulationRunning);
    for (size_t i=0;i<loadedPathPlanningTaskList->size();i++)
        loadedPathPlanningTaskList->at(i)->initializeInitialValues(simulationRunning);
    for (size_t i=0;i<loadedLuaScriptList->size();i++)
        loadedLuaScriptList->at(i)->initializeInitialValues(simulationRunning);

    // Here we call the initializeInitialValues for all pages & views
    for (size_t i=0;i<loadedObjectList->size();i++)
        pageContainer->initializeInitialValues(simulationRunning,loadedObjectList->at(i)->getObjectHandle());
//**************************************************************************************

    // Here make sure that referenced objects still exists (when keeping original references):
    // ------------------------------------------------
    if (model)
    {
        std::map<std::string,int> uniquePersistentIds;
        for (size_t i=0;i<sceneObjects->getObjectCount();i++)
        {
            CSceneObject* obj=sceneObjects->getObjectFromIndex(i);
            uniquePersistentIds[obj->getUniquePersistentIdString()]=obj->getObjectHandle();
        }
        for (size_t i=0;i<collisions->getObjectCount();i++)
            uniquePersistentIds[collisions->getObjectFromIndex(i)->getUniquePersistentIdString()]=collisions->getObjectFromIndex(i)->getObjectHandle();
        for (size_t i=0;i<distances->getObjectCount();i++)
            uniquePersistentIds[distances->getObjectFromIndex(i)->getUniquePersistentIdString()]=distances->getObjectFromIndex(i)->getObjectHandle();
        for (size_t i=0;i<ikGroups->getObjectCount();i++)
            uniquePersistentIds[ikGroups->getObjectFromIndex(i)->getUniquePersistentIdString()]=ikGroups->getObjectFromIndex(i)->getObjectHandle();
        for (size_t i=0;i<collections->getObjectCount();i++)
            uniquePersistentIds[collections->getObjectFromIndex(i)->getUniquePersistentIdString()]=collections->getObjectFromIndex(i)->getCollectionHandle();
        for (size_t i=0;i<loadedObjectList->size();i++)
            loadedObjectList->at(i)->checkReferencesToOriginal(uniquePersistentIds);
    }
    // ------------------------------------------------

    // Now display the load operation issues:
    for (size_t i=0;i<_loadOperationIssues.size();i++)
    {
        int handle=_loadOperationIssues[i].objectHandle;
        std::string newTxt("NAME_NOT_FOUND");
        int handle2=getLoadingMapping(&luaScriptMapping,handle);
        CLuaScriptObject* script=luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(handle2);
        if (script!=nullptr)
            newTxt=script->getShortDescriptiveName();
        std::string msg(_loadOperationIssues[i].message);
        CTTUtil::replaceSubstring(msg,"@@REPLACE@@",newTxt.c_str());
        App::logMsg(_loadOperationIssues[i].verbosity,msg.c_str());
    }
    appendLoadOperationIssue(-1,nullptr,-1); // clears it

    setEnableRemoteWorldsSync(true);

    for (size_t i=0;i<loadedObjectList->size();i++)
        loadedObjectList->at(i)->buildUpdateAndPopulateSynchronizationObject(nullptr);
    for (size_t i=0;i<loadedObjectList->size();i++)
        loadedObjectList->at(i)->connectSynchronizationObject();

    for (size_t i=0;i<loadedIkGroupList->size();i++)
        loadedIkGroupList->at(i)->buildUpdateAndPopulateSynchronizationObject(nullptr);
    for (size_t i=0;i<loadedIkGroupList->size();i++)
        loadedIkGroupList->at(i)->connectSynchronizationObject();

    for (size_t i=0;i<loadedCollectionList->size();i++)
        loadedCollectionList->at(i)->buildUpdateAndPopulateSynchronizationObject(nullptr);
    for (size_t i=0;i<loadedCollectionList->size();i++)
        loadedCollectionList->at(i)->connectSynchronizationObject();

    for (size_t i=0;i<loadedCollisionList->size();i++)
        loadedCollisionList->at(i)->buildUpdateAndPopulateSynchronizationObject(nullptr);
    for (size_t i=0;i<loadedCollisionList->size();i++)
        loadedCollisionList->at(i)->connectSynchronizationObject();

    for (size_t i=0;i<loadedDistanceList->size();i++)
        loadedDistanceList->at(i)->buildUpdateAndPopulateSynchronizationObject(nullptr);
    for (size_t i=0;i<loadedDistanceList->size();i++)
        loadedDistanceList->at(i)->connectSynchronizationObject();

    // We select what was loaded if we have a model loaded through the GUI:
    sceneObjects->deselectObjects();
    if (model)
    {
        for (size_t i=0;i<loadedObjectList->size();i++)
            sceneObjects->addObjectToSelection(loadedObjectList->at(i)->getObjectHandle());
    }
}

void CWorld::cleanupHashNames_allObjects(int suffix)
{ // This function will try to use the lowest hash naming possible (e.g. model#59 --> model and model#67 --> model#0 if possible)
    // if suffix is -1, then all suffixes are cleaned, otherwise only those equal or above 'suffix'

    // 1. we get all object's smallest and biggest suffix:
    int smallestSuffix,biggestSuffix;
    _getMinAndMaxNameSuffixes(smallestSuffix,biggestSuffix);
    if (suffix<=0)
        suffix=0;
    for (int i=suffix;i<=biggestSuffix;i++)
    {
        for (int j=-1;j<i;j++)
        {
            if (_canSuffix1BeSetToSuffix2(i,j))
            {
                _setSuffix1ToSuffix2(i,j);
                break;
            }
        }
    }
}

void CWorld::renderYourGeneralObject3DStuff_beforeRegularObjects(CViewableBase* renderingObject,int displayAttrib,int windowSize[2],float verticalViewSizeOrAngle,bool perspective)
{
    distances->renderYour3DStuff(renderingObject,displayAttrib);
    drawingCont->renderYour3DStuff_nonTransparent(renderingObject,displayAttrib);
    pointCloudCont->renderYour3DStuff_nonTransparent(renderingObject,displayAttrib);
    ghostObjectCont->renderYour3DStuff_nonTransparent(renderingObject,displayAttrib);
    bannerCont->renderYour3DStuff_nonTransparent(renderingObject,displayAttrib,windowSize,verticalViewSizeOrAngle,perspective);
    dynamicsContainer->renderYour3DStuff(renderingObject,displayAttrib);
}

void CWorld::renderYourGeneralObject3DStuff_afterRegularObjects(CViewableBase* renderingObject,int displayAttrib,int windowSize[2],float verticalViewSizeOrAngle,bool perspective)
{
    drawingCont->renderYour3DStuff_transparent(renderingObject,displayAttrib);
    pointCloudCont->renderYour3DStuff_transparent(renderingObject,displayAttrib);
    ghostObjectCont->renderYour3DStuff_transparent(renderingObject,displayAttrib);
    bannerCont->renderYour3DStuff_transparent(renderingObject,displayAttrib,windowSize,verticalViewSizeOrAngle,perspective);
}

void CWorld::renderYourGeneralObject3DStuff_onTopOfRegularObjects(CViewableBase* renderingObject,int displayAttrib,int windowSize[2],float verticalViewSizeOrAngle,bool perspective)
{
    drawingCont->renderYour3DStuff_overlay(renderingObject,displayAttrib);
    pointCloudCont->renderYour3DStuff_overlay(renderingObject,displayAttrib);
    ghostObjectCont->renderYour3DStuff_overlay(renderingObject,displayAttrib);
    bannerCont->renderYour3DStuff_overlay(renderingObject,displayAttrib,windowSize,verticalViewSizeOrAngle,perspective);
    collisions->renderYour3DStuff(renderingObject,displayAttrib);
    dynamicsContainer->renderYour3DStuff_overlay(renderingObject,displayAttrib);
}

void CWorld::announceObjectWillBeErased(int objectHandle)
{
    luaScriptContainer->announceObjectWillBeErased(objectHandle);
    sceneObjects->announceObjectWillBeErased(objectHandle);
    pageContainer->announceObjectWillBeErased(objectHandle); // might trigger a view destruction!
    buttonBlockContainer->announceObjectWillBeErased(objectHandle);
    pathPlanning->announceObjectWillBeErased(objectHandle);
    collisions->announceObjectWillBeErased(objectHandle);
    distances->announceObjectWillBeErased(objectHandle);
    drawingCont->announceObjectWillBeErased(objectHandle);
    pointCloudCont->announceObjectWillBeErased(objectHandle);
    ghostObjectCont->announceObjectWillBeErased(objectHandle);
    bannerCont->announceObjectWillBeErased(objectHandle);
    textureContainer->announceGeneralObjectWillBeErased(objectHandle,-1);
    collections->announceObjectWillBeErased(objectHandle); // can trigger distance, collision
    ikGroups->announceObjectWillBeErased(objectHandle);
}

void CWorld::announceIkGroupWillBeErased(int ikGroupHandle)
{
    sceneObjects->announceIkGroupWillBeErased(ikGroupHandle);
    ikGroups->announceIkGroupWillBeErased(ikGroupHandle);
}

void CWorld::announceCollectionWillBeErased(int collectionHandle)
{
    sceneObjects->announceCollectionWillBeErased(collectionHandle);
    collisions->announceCollectionWillBeErased(collectionHandle); // This can trigger a collision destruction!
    distances->announceCollectionWillBeErased(collectionHandle); // This can trigger a distance destruction!
    pathPlanning->announceCollectionWillBeErased(collectionHandle); // This can trigger a path planning object destruction!
}

void CWorld::announceCollisionWillBeErased(int collisionHandle)
{
    sceneObjects->announceCollisionWillBeErased(collisionHandle);
}

void CWorld::announceDistanceWillBeErased(int distanceHandle)
{
    sceneObjects->announceDistanceWillBeErased(distanceHandle);
}

void CWorld::announce2DElementWillBeErased(int elementID)
{
    if (textureContainer!=nullptr)
        textureContainer->announceGeneralObjectWillBeErased(elementID,-1);
}

void CWorld::announce2DElementButtonWillBeErased(int elementID,int buttonID)
{
    if (textureContainer!=nullptr)
        textureContainer->announceGeneralObjectWillBeErased(elementID,buttonID);
}

void CWorld::exportIkContent(CExtIkSer& ar)
{
    ar.writeInt(0); // this is the ext IK serialization version. Not forward nor backward compatible!
    sceneObjects->exportIkContent(ar);
    ikGroups->exportIkContent(ar);
}

bool CWorld::_loadModelOrScene(CSer& ar,bool selectLoaded,bool isScene,bool justLoadThumbnail,bool forceModelAsCopy,C7Vector* optionalModelTr,C3Vector* optionalModelBoundingBoxSize,float* optionalModelNonDefaultTranslationStepSize)
{
    appendLoadOperationIssue(-1,nullptr,-1); // clear

    CMesh::clearTempVerticesIndicesNormalsAndEdges();
    sceneObjects->deselectObjects();

    std::vector<CSceneObject*> loadedObjectList;
    std::vector<CTextureObject*> loadedTextureList;
    std::vector<CDynMaterialObject*> loadedDynMaterialList;
    std::vector<CCollection*> loadedCollectionList;
    std::vector<CCollisionObject*> loadedCollisionList;
    std::vector<CDistanceObject*> loadedDistanceList;
    std::vector<CIkGroup*> loadedIkGroupList;
    std::vector<CPathPlanningTask*> pathPlanningTaskList;
    std::vector<CButtonBlock*> loadedButtonBlockList;
    std::vector<CLuaScriptObject*> loadedLuaScriptList;

    bool hasThumbnail=false;
    if (ar.isBinary())
    {
        int byteQuantity;
        std::string theName="";
        while (theName.compare(SER_END_OF_FILE)!=0)
        {
            theName=ar.readDataName();
            if (theName.compare(SER_END_OF_FILE)!=0)
            {
                bool noHit=true;
                if (theName.compare(SER_MODEL_THUMBNAIL_INFO)==0)
                {
                    ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                    C7Vector tr;
                    C3Vector bbs;
                    float ndss;
                    environment->modelThumbnail_notSerializedHere.serializeAdditionalModelInfos(ar,tr,bbs,ndss);
                    if (optionalModelTr!=nullptr)
                        optionalModelTr[0]=tr;
                    if (optionalModelBoundingBoxSize!=nullptr)
                        optionalModelBoundingBoxSize[0]=bbs;
                    if (optionalModelNonDefaultTranslationStepSize!=nullptr)
                        optionalModelNonDefaultTranslationStepSize[0]=ndss;
                    noHit=false;
                }

                if (theName.compare(SER_MODEL_THUMBNAIL)==0)
                {
                    ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                    environment->modelThumbnail_notSerializedHere.serialize(ar);
                    noHit=false;
                    if (justLoadThumbnail)
                        return(true);
                    hasThumbnail=true;
                }

                // Handle the heavy data here so we don't have duplicates (vertices, indices, normals and edges):
                //------------------------------------------------------------
                if (theName.compare(SER_VERTICESINDICESNORMALSEDGES)==0)
                {
                    CMesh::clearTempVerticesIndicesNormalsAndEdges();
                    ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                    CMesh::serializeTempVerticesIndicesNormalsAndEdges(ar);
                    noHit=false;
                }
                //------------------------------------------------------------

                CSceneObject* it=sceneObjects->readSceneObject(ar,theName.c_str(),noHit);
                if (it!=nullptr)
                {
                    loadedObjectList.push_back(it);
                    noHit=false;
                }

                CTextureObject* theTextureData=textureContainer->loadTextureObject(ar,theName,noHit);
                if (theTextureData!=nullptr)
                {
                    loadedTextureList.push_back(theTextureData);
                    noHit=false;
                }
                if (theName.compare(SER_DYNMATERIAL)==0)
                { // Following for backward compatibility (i.e. files written prior CoppeliaSim 3.4.0) (30/10/2016)
                    ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                    CDynMaterialObject* myNewObject=new CDynMaterialObject();
                    myNewObject->serialize(ar);
                    loadedDynMaterialList.push_back(myNewObject);
                    noHit=false;
                }
                if (theName.compare(SER_GHOSTS)==0)
                {
                    ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                    ghostObjectCont->serialize(ar);
                    noHit=false;
                }
                if (theName.compare(SER_ENVIRONMENT)==0)
                {
                    ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                    environment->serialize(ar);
                    noHit=false;
                }
                if (theName.compare(SER_SETTINGS)==0)
                {
                    ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                    mainSettings->serialize(ar);
                    noHit=false;
                }
                if (theName.compare(SER_DYNAMICS)==0)
                {
                    ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                    dynamicsContainer->serialize(ar);
                    noHit=false;
                }
                if (theName.compare(SER_SIMULATION)==0)
                {
                    ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                    simulation->serialize(ar);
                    noHit=false;

                    // For backward compatibility (3/1/2012):
                    //************************************************
                    if (mainSettings->forBackwardCompatibility_03_01_2012_stillUsingStepSizeDividers)
                    { // This needs to be done AFTER simulation settings are loaded!
                        float step=float(simulation->getSimulationTimeStep_speedModified_us())/1000000.0f;
                        float bulletStepSize=step/float(mainSettings->dynamicsBULLETStepSizeDivider_forBackwardCompatibility_03_01_2012);
                        float odeStepSize=step/float(mainSettings->dynamicsODEStepSizeDivider_forBackwardCompatibility_03_01_2012);
                        if (fabs(step-0.05f)>0.002f)
                            dynamicsContainer->setUseDynamicDefaultCalculationParameters(4); // use custom settings
                        // Following has an effect only when using custom parameters (custom parameters might already be enabled before above line!):

                        dynamicsContainer->setEngineFloatParam(sim_bullet_global_stepsize,bulletStepSize,false);
                        dynamicsContainer->setEngineFloatParam(sim_ode_global_stepsize,odeStepSize,false);
                    }
                    //************************************************
                }
                if (theName.compare(SER_VIEWS)==0)
                {
                    ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                    pageContainer->serialize(ar);
                    noHit=false;
                }
                if (theName.compare(SER_COLLECTION)==0)
                {
                    if (App::userSettings->xrTest==123456789)
                        App::logMsg(sim_verbosity_errors,"Contains collections...");
                    ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                    CCollection* it=new CCollection();
                    it->serialize(ar);
                    loadedCollectionList.push_back(it);
                    noHit=false;
                }
                if (theName.compare(SER_BUTTON_BLOCK_OLD)==0)
                {
                    if (App::userSettings->xrTest==123456789)
                        App::logMsg(sim_verbosity_errors,"Contains old custom UIs...");
                    if (!App::userSettings->disableOpenGlBasedCustomUi)
                    {
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        CButtonBlock* it=new CButtonBlock(1,1,10,10,0);
                        it->serialize(ar);
                        loadedButtonBlockList.push_back(it);
                        noHit=false;
                    }
                }
                if (theName.compare(SER_LUA_SCRIPT)==0)
                {
                    ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                    CLuaScriptObject* it=new CLuaScriptObject(-1);
                    it->serialize(ar);
                    if ( (it->getScriptType()==sim_scripttype_jointctrlcallback_old)||(it->getScriptType()==sim_scripttype_generalcallback_old)||(it->getScriptType()==sim_scripttype_contactcallback_old) )
                    { // joint callback, contact callback and general callback scripts are not supported anymore since V3.6.1.rev2
                        std::string ml(it->getScriptText());
                        if (it->getScriptType()==sim_scripttype_jointctrlcallback_old)
                            ml="the file contains a joint control callback script, which is a script type that is not supported anymore (since CoppeliaSim V3.6.1 rev2).\nUse a joint callback function instead. Following the script content:\n"+ml;
                        if (it->getScriptType()==sim_scripttype_generalcallback_old)
                            ml="the file contains a general callback script, which is a script type that is not supported anymore (since CoppeliaSim V3.6.1 rev2):\n"+ml;
                        if (it->getScriptType()==sim_scripttype_contactcallback_old)
                            ml="the file contains a contact callback script, which is a script type that is not supported anymore (since CoppeliaSim V3.6.1 rev2).\nUse a contact callback functions instead. Following the script content:\n"+ml;
                        App::logMsg(sim_verbosity_errors,ml.c_str());
                        delete it;
                    }
                    else
                        loadedLuaScriptList.push_back(it);
                    noHit=false;
                }
                if (theName.compare(SER_SCENE_CUSTOM_DATA)==0)
                {
                    ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                    customSceneData->serializeData(ar,nullptr,-1);
                    noHit=false;
                }
                if (theName.compare(SER_COLLISION)==0)
                {
                    if (App::userSettings->xrTest==123456789)
                        App::logMsg(sim_verbosity_errors,"Contains collision objects...");
                    ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                    CCollisionObject* it=new CCollisionObject();
                    it->serialize(ar);
                    loadedCollisionList.push_back(it);
                    noHit=false;
                }
                if (theName.compare(SER_DISTANCE)==0)
                {
                    if (App::userSettings->xrTest==123456789)
                        App::logMsg(sim_verbosity_errors,"Contains distance objects...");
                    ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                    CDistanceObject* it=new CDistanceObject();
                    it->serialize(ar);
                    loadedDistanceList.push_back(it);
                    noHit=false;
                }
                if (theName.compare(SER_IK)==0)
                {
                    if (App::userSettings->xrTest==123456789)
                        App::logMsg(sim_verbosity_errors,"Contains IK objects...");
                    ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                    CIkGroup* it=new CIkGroup();
                    it->serialize(ar);
                    loadedIkGroupList.push_back(it);
                    noHit=false;
                }
                if (theName==SER_PATH_PLANNING)
                {
                    if (App::userSettings->xrTest==123456789)
                        App::logMsg(sim_verbosity_errors,"Contains path planning objects...");
                    ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                    CPathPlanningTask* it=new CPathPlanningTask();
                    it->serialize(ar);
                    pathPlanningTaskList.push_back(it);
                    noHit=false;
                }
                if (noHit)
                    ar.loadUnknownData();
            }
        }
    }
    else
    {
        std::string dummy1;
        bool dummy2;
        if (ar.xmlPushChildNode(SERX_MODEL_THUMBNAIL))
        {
            environment->modelThumbnail_notSerializedHere.serialize(ar);
            ar.xmlPopNode();
            hasThumbnail=true;
        }
        if (ar.xmlPushChildNode(SERX_SCENEOBJECT,false))
        {
            while (true)
            {
                bool dummy2;
                CSceneObject* it=sceneObjects->readSceneObject(ar,"",dummy2);
                if (it!=nullptr)
                    loadedObjectList.push_back(it);
                if (!ar.xmlPushSiblingNode(SERX_SCENEOBJECT,false))
                    break;
            }
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_TEXTURE,false))
        {
            while (true)
            {
                CTextureObject* theTextureData=textureContainer->loadTextureObject(ar,dummy1,dummy2);
                if (theTextureData!=nullptr)
                    loadedTextureList.push_back(theTextureData);
                if (!ar.xmlPushSiblingNode(SERX_TEXTURE,false))
                    break;
            }
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_GHOSTS,isScene))
        {
            ghostObjectCont->serialize(ar);
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_ENVIRONMENT,isScene))
        {
            environment->serialize(ar);
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_SETTINGS,isScene))
        {
            mainSettings->serialize(ar);
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_DYNAMICS,isScene))
        {
            dynamicsContainer->serialize(ar);
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_SIMULATION,isScene))
        {
            simulation->serialize(ar);
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_SCENE_CUSTOM_DATA,isScene))
        {
            customSceneData->serializeData(ar,nullptr,-1);
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_VIEWS,isScene))
        {
            pageContainer->serialize(ar);
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_COLLISION,false))
        {
            while (true)
            {
                CCollisionObject* it=new CCollisionObject();
                it->serialize(ar);
                loadedCollisionList.push_back(it);
                if (!ar.xmlPushSiblingNode(SERX_COLLISION,false))
                    break;
            }
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_DISTANCE,false))
        {
            while (true)
            {
                CDistanceObject* it=new CDistanceObject();
                it->serialize(ar);
                loadedDistanceList.push_back(it);
                if (!ar.xmlPushSiblingNode(SERX_DISTANCE,false))
                    break;
            }
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_COLLECTION,false))
        {
            while (true)
            {
                CCollection* it=new CCollection();
                it->serialize(ar);
                loadedCollectionList.push_back(it);
                if (!ar.xmlPushSiblingNode(SERX_COLLECTION,false))
                    break;
            }
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_IK,false))
        {
            while (true)
            {
                CIkGroup* it=new CIkGroup();
                it->serialize(ar);
                loadedIkGroupList.push_back(it);
                if (!ar.xmlPushSiblingNode(SERX_IK,false))
                    break;
            }
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_LUA_SCRIPT,false))
        {
            while (true)
            {
                CLuaScriptObject* it=new CLuaScriptObject(-1);
                it->serialize(ar);
                loadedLuaScriptList.push_back(it);
                if (!ar.xmlPushSiblingNode(SERX_LUA_SCRIPT,false))
                    break;
            }
            ar.xmlPopNode();
        }
    }

    CMesh::clearTempVerticesIndicesNormalsAndEdges();

    int fileSimVersion=ar.getCoppeliaSimVersionThatWroteThisFile();

    // All object have been loaded and are in:
    // loadedObjectList
    // loadedCollectionList
    // ...

    addGeneralObjectsToWorldAndPerformMappings(&loadedObjectList,
                                        &loadedCollectionList,
                                        &loadedCollisionList,
                                        &loadedDistanceList,
                                        &loadedIkGroupList,
                                        &pathPlanningTaskList,
                                        &loadedButtonBlockList,
                                        &loadedLuaScriptList,
                                        loadedTextureList,
                                        loadedDynMaterialList,
                                        !isScene,fileSimVersion,forceModelAsCopy);

    CMesh::clearTempVerticesIndicesNormalsAndEdges();

    appendLoadOperationIssue(-1,nullptr,-1); // clear

    if (!isScene)
    {
        CInterfaceStack stack;
        stack.pushTableOntoStack();
        stack.pushStringOntoStack("objectHandles",0);
        stack.pushTableOntoStack();
        for (size_t i=0;i<loadedObjectList.size();i++)
        {
            stack.pushNumberOntoStack(double(i+1)); // key or index
            stack.pushNumberOntoStack(loadedObjectList[i]->getObjectHandle());
            stack.insertDataIntoStackTable();
        }
        stack.insertDataIntoStackTable();
        luaScriptContainer->callChildMainCustomizationAddonSandboxScriptWithData(sim_syscb_aftercreate,&stack);
    }

    // Following for backward compatibility for vision sensor filters:
    for (size_t i=0;i<sceneObjects->getVisionSensorCount();i++)
    {
        CVisionSensor* it=sceneObjects->getVisionSensorFromIndex(i);
        CComposedFilter* cf=it->getComposedFilter();
        std::string txt(cf->scriptEquivalent);
        if (txt.size()>0)
        {
            cf->scriptEquivalent.clear();
            CLuaScriptObject* script=luaScriptContainer->getScriptFromObjectAttachedTo_customization(it->getObjectHandle());
            if (script==nullptr)
            {
                txt=std::string("function sysCall_init()\nend\n\n")+txt;
                script=new CLuaScriptObject(sim_scripttype_customizationscript);
                luaScriptContainer->insertScript(script);
                script->setObjectIDThatScriptIsAttachedTo(it->getObjectHandle());
            }
            std::string t(script->getScriptText());
            t=txt+t;
            script->setScriptText(t.c_str());
        }
    }
    // Following for backward compatibility for force/torque sensor filters:
    for (size_t i=0;i<sceneObjects->getForceSensorCount();i++)
    {
        CForceSensor* it=sceneObjects->getForceSensorFromIndex(i);
        if (it->getStillAutomaticallyBreaking())
        {
            CLuaScriptObject* script=luaScriptContainer->getScriptFromObjectAttachedTo_customization(it->getObjectHandle());
            std::string txt("function sysCall_trigger(inData)\n    -- callback function automatically added for backward compatibility\n    sim.breakForceSensor(inData.handle)\nend\n\n");
            if (script==nullptr)
            {
                txt=std::string("function sysCall_init()\nend\n\n")+txt;
                script=new CLuaScriptObject(sim_scripttype_customizationscript);
                luaScriptContainer->insertScript(script);
                script->setObjectIDThatScriptIsAttachedTo(it->getObjectHandle());
            }
            std::string t(script->getScriptText());
            t=txt+t;
            script->setScriptText(t.c_str());
        }
    }

    // Following for backward compatibility (Lua script parameters are now attached to objects, and not scripts anymore):
    for (size_t i=0;i<loadedLuaScriptList.size();i++)
    {
        CLuaScriptObject* script=luaScriptContainer->getScriptFromID_noAddOnsNorSandbox(loadedLuaScriptList[i]->getScriptID());
        if (script!=nullptr)
        {
            CUserParameters* params=script->getScriptParametersObject_backCompatibility();
            int obj=script->getObjectIDThatScriptIsAttachedTo_child();
            CSceneObject* theObj=sceneObjects->getObjectFromHandle(obj);
            if ( (theObj!=nullptr)&&(params!=nullptr) )
            {
                if (params->userParamEntries.size()>0)
                {
                    theObj->setUserScriptParameterObject(params->copyYourself());
                    params->userParamEntries.clear();
                }
            }
        }
    }
    return(true);
}

bool CWorld::_loadSimpleXmlSceneOrModel(CSer& ar)
{
    bool retVal=true;
    bool isScene=(ar.getFileType()==CSer::filetype_csim_xml_simplescene_file);
    removeRemoteWorlds();
    setEnableRemoteWorldsSync(false);
    if ( isScene&&ar.xmlPushChildNode(SERX_ENVIRONMENT,false) )
    {
        environment->serialize(ar);
        ar.xmlPopNode();
    }

    if ( isScene&&ar.xmlPushChildNode(SERX_SETTINGS,false) )
    {
        mainSettings->serialize(ar);
        ar.xmlPopNode();
    }

    if ( isScene&&ar.xmlPushChildNode(SERX_DYNAMICS,false) )
    {
        dynamicsContainer->serialize(ar);
        ar.xmlPopNode();
    }

    if ( isScene&&ar.xmlPushChildNode(SERX_SIMULATION,false) )
    {
        simulation->serialize(ar);
        ar.xmlPopNode();
    }

    std::vector<CCollection*> allLoadedCollections;
    std::map<std::string,CCollection*> _collectionLoadNamesMap;
    if (ar.xmlPushChildNode(SERX_COLLECTION,false))
    {
        while (true)
        {
            CCollection* it=new CCollection();
            it->serialize(ar);
            allLoadedCollections.push_back(it);
            _collectionLoadNamesMap[it->getCollectionLoadName()]=it;
            if (!ar.xmlPushSiblingNode(SERX_COLLECTION,false))
                break;
        }
        ar.xmlPopNode();
    }

    std::vector<CIkGroup*> allLoadedIkGroups;
    if (ar.xmlPushChildNode(SERX_IK,false))
    {
        while (true)
        {
            CIkGroup* it=new CIkGroup();
            it->serialize(ar);
            allLoadedIkGroups.push_back(it);
            if (!ar.xmlPushSiblingNode(SERX_IK,false))
                break;
        }
        ar.xmlPopNode();
    }

    if ( isScene&&(luaScriptContainer->getMainScript()==nullptr) )
        luaScriptContainer->insertDefaultScript_mainAndChildScriptsOnly(sim_scripttype_mainscript,false,false);

    CCamera* mainCam=nullptr;

    C7Vector ident;
    ident.setIdentity();
    std::vector<SSimpleXmlSceneObject> simpleXmlObjects;
    sceneObjects->readAndAddToSceneSimpleXmlSceneObjects(ar,nullptr,ident,simpleXmlObjects);
    std::map<CSceneObject*,bool> addedObjects;
    std::vector<CSceneObject*> allLoadedObjects;
    bool hasAScriptAttached=false;
    while (simpleXmlObjects.size()>0)
    {
        for (size_t i=0;i<simpleXmlObjects.size();i++)
        {
            CSceneObject* it=simpleXmlObjects[i].object;
            if (it->getObjectType()==sim_object_camera_type)
            {
                if ( (mainCam==nullptr)||((CCamera*)it)->getIsMainCamera() )
                    mainCam=(CCamera*)it;
            }
            CSceneObject* pit=simpleXmlObjects[i].parentObject;
            CLuaScriptObject* childScript=simpleXmlObjects[i].childScript;
            CLuaScriptObject* customizationScript=simpleXmlObjects[i].customizationScript;
            std::map<CSceneObject*,bool>::iterator s=addedObjects.find(pit);
            if ( (s!=addedObjects.end())||(pit==nullptr) )
            {
                allLoadedObjects.push_back(it);
                it->setParent(pit,false);
                addedObjects[it]=true;

                if (childScript!=nullptr)
                {
                    hasAScriptAttached=true;
                    luaScriptContainer->insertScript(childScript);
                    childScript->setObjectIDThatScriptIsAttachedTo(it->getObjectHandle());
                }
                if (customizationScript!=nullptr)
                {
                    hasAScriptAttached=true;
                    luaScriptContainer->insertScript(customizationScript);
                    customizationScript->setObjectIDThatScriptIsAttachedTo(it->getObjectHandle());
                }
                simpleXmlObjects.erase(simpleXmlObjects.begin()+i);
            }
        }
    }
    if ( (mainCam!=nullptr)&&isScene )
    {
        pageContainer->setUpDefaultPages(true);
#ifdef SIM_WITH_GUI
        CSPage* page=pageContainer->getPage(pageContainer->getActivePageIndex());
        CSView* view=page->getView(0);
        if (view!=nullptr)
            view->setLinkedObjectID(mainCam->getObjectHandle(),false);
#endif
    }

    // Now adjust the names:
    int suffixOffset=_getSuffixOffsetForGeneralObjectToAdd(true,&allLoadedObjects,&allLoadedCollections,nullptr,nullptr,&allLoadedIkGroups,nullptr,nullptr,nullptr);
    // We add objects to the scene as copies only if we also add at least one associated script and we don't have a scene. Otherwise objects are added
    // and no '#' (or no modified suffix) will appear in their names.
    // Following line summarizes this:
    bool objectIsACopy=(hasAScriptAttached&&(ar.getFileType()==CSer::filetype_csim_xml_simplemodel_file)); // scenes are not treated like copies!
    std::map<std::string,CSceneObject*> _objectTempNamesMap;
    for (size_t i=0;i<allLoadedObjects.size();i++)
    {
        CSceneObject* it=allLoadedObjects[i];
        _objectTempNamesMap[it->getObjectTempName()]=it;
        std::string newObjName=it->getObjectTempName();
        if (objectIsACopy)
            newObjName=tt::generateNewName_hash(newObjName.c_str(),suffixOffset);
        else
        {
            if (sceneObjects->getObjectFromName(newObjName.c_str())!=nullptr)
            {
                // Following faster with many objects:
                std::string baseName(tt::getNameWithoutSuffixNumber(newObjName.c_str(),false));
                int initialSuffix=tt::getNameSuffixNumber(newObjName.c_str(),false);
                std::vector<int> suffixes;
                std::vector<int> dummyValues;
                for (size_t i=0;i<sceneObjects->getObjectCount();i++)
                { //
                    CSceneObject* itt=sceneObjects->getObjectFromIndex(i);
                    std::string baseNameIt(tt::getNameWithoutSuffixNumber(itt->getObjectName().c_str(),false));
                    if (baseName.compare(baseNameIt)==0)
                    {
                        suffixes.push_back(tt::getNameSuffixNumber(itt->getObjectName().c_str(),false));
                        dummyValues.push_back(0);
                    }
                }
                tt::orderAscending(suffixes,dummyValues);
                int lastS=-1;
                for (size_t i=0;i<suffixes.size();i++)
                {
                    if ( (suffixes[i]>initialSuffix)&&(suffixes[i]>lastS+1) )
                        break;
                    lastS=suffixes[i];
                }
                newObjName=tt::generateNewName_noHash(baseName.c_str(),lastS+1+1);
            }
            // Following was too slow with many objects:
            //      while (getObject(newObjName)!=nullptr)
            //          newObjName=tt::generateNewName_noHash(newObjName);
        }
        it->setObjectName(newObjName.c_str(),true);
        //sceneObjects->renameObject(it->getObjectHandle(),newObjName.c_str());

        // Now a similar procedure, but with the alt object names:
        std::string newObjAltName=it->getObjectTempAltName();
        if (sceneObjects->getObjectFromAltName(newObjAltName.c_str())!=nullptr)
        {
            // Following faster with many objects:
            std::string baseAltName(tt::getNameWithoutSuffixNumber(newObjAltName.c_str(),false));
            int initialSuffix=tt::getNameSuffixNumber(newObjAltName.c_str(),false);
            std::vector<int> suffixes;
            std::vector<int> dummyValues;
            for (size_t i=0;i<sceneObjects->getObjectCount();i++)
            {
                CSceneObject* itt=sceneObjects->getObjectFromIndex(i);
                std::string baseAltNameIt(tt::getNameWithoutSuffixNumber(itt->getObjectAltName().c_str(),false));
                if (baseAltName.compare(baseAltNameIt)==0)
                {
                    suffixes.push_back(tt::getNameSuffixNumber(itt->getObjectAltName().c_str(),false));
                    dummyValues.push_back(0);
                }
            }
            tt::orderAscending(suffixes,dummyValues);
            int lastS=-1;
            for (size_t i=0;i<suffixes.size();i++)
            {
                if ( (suffixes[i]>initialSuffix)&&(suffixes[i]>lastS+1) )
                    break;
                lastS=suffixes[i];
            }
            newObjAltName=tt::generateNewName_noHash(baseAltName.c_str(),lastS+1+1);
        }
        // Following was too slow with many objects:
        //      while (getObjectFromAltName(newObjAltName)!=nullptr)
        //          newObjAltName=tt::generateNewName_noHash(newObjAltName);
        it->setObjectAltName(newObjAltName.c_str(),true);
        //sceneObjects->altRenameObject(it->getObjectHandle(),newObjAltName.c_str());
    }

    // Add collections and perform mapping:
    for (size_t i=0;i<allLoadedCollections.size();i++)
    {
        CCollection* it=allLoadedCollections[i];
        for (size_t j=0;j<it->getElementCount();j++)
        {
            CCollectionElement* el=it->getElementFromIndex(j);
            std::map<std::string,CSceneObject*>::const_iterator elIt=_objectTempNamesMap.find(el->getMainObjectTempName());
            if ( (el->getMainObjectTempName().size()>0)&&(elIt!=_objectTempNamesMap.end()) )
                el->setMainObject(elIt->second->getObjectHandle());
            else
            {
                if (el->getElementType()!=sim_collectionelement_all)
                {
                    it->removeCollectionElementFromHandle(el->getElementHandle());
                    j--; // reprocess this position
                }
            }
        }
        if (it->getElementCount()>0)
        {
            collections->addCollectionWithSuffixOffset(it,objectIsACopy,suffixOffset);
            it->actualizeCollection(true);
        }
        else
        {
            delete it;
            allLoadedCollections.erase(allLoadedCollections.begin()+i);
            i--; // reprocess this position
        }
    }

    // Add ik groups and perform mapping:
    for (size_t i=0;i<allLoadedIkGroups.size();i++)
    {
        CIkGroup* it=allLoadedIkGroups[i];
        for (size_t j=0;j<it->getIkElementCount();j++)
        {
            CIkElement* el=it->getIkElementFromIndex(j);
            std::map<std::string,CSceneObject*>::const_iterator elIt=_objectTempNamesMap.find(el->getTipLoadName());
            if ( (el->getTipLoadName().size()>0)&&(elIt!=_objectTempNamesMap.end()) )
            {
                el->setTipHandle(elIt->second->getObjectHandle());
                elIt=_objectTempNamesMap.find(el->getBaseLoadName());
                if ( (el->getBaseLoadName().size()>0)&&(elIt!=_objectTempNamesMap.end()) )
                    el->setBase(elIt->second->getObjectHandle());
                elIt=_objectTempNamesMap.find(el->getAltBaseLoadName());
                if ( (el->getAltBaseLoadName().size()>0)&&(elIt!=_objectTempNamesMap.end()) )
                    el->setAlternativeBaseForConstraints(elIt->second->getObjectHandle());
            }
            else
            {
                it->removeIkElement(el->getObjectHandle());
                j=-1; // start the loop over again
            }
        }
        if (it->getIkElementCount()>0)
            ikGroups->addIkGroupWithSuffixOffset(it,objectIsACopy,suffixOffset);
        else
        {
            delete it;
            allLoadedIkGroups.erase(allLoadedIkGroups.begin()+i);
            i--; // reprocess this position
        }
    }

    for (size_t i=0;i<allLoadedObjects.size();i++)
    {
        CSceneObject* obj=allLoadedObjects[i];
        // Handle dummy-dummy linking:
        if (obj->getObjectType()==sim_object_dummy_type)
        {
            CDummy* dummy=(CDummy*)obj;
            if (dummy->getLinkedDummyLoadName().size()>0)
            {
                std::map<std::string,CSceneObject*>::const_iterator it=_objectTempNamesMap.find(dummy->getLinkedDummyLoadName());
                if (it!=_objectTempNamesMap.end())
                    dummy->setLinkedDummyHandle(it->second->getObjectHandle(),true);
            }
        }
        // Handle joint-joint linking:
        if (obj->getObjectType()==sim_object_joint_type)
        {
            CJoint* joint=(CJoint*)obj;
            if (joint->getDependencyJointLoadName().size()>0)
            {
                std::map<std::string,CSceneObject*>::const_iterator it=_objectTempNamesMap.find(joint->getDependencyJointLoadName());
                if (it!=_objectTempNamesMap.end())
                    joint->setDependencyJointHandle(it->second->getObjectHandle());
            }
        }
        // Handle camera tracking:
        if (obj->getObjectType()==sim_object_camera_type)
        {
            CCamera* camera=(CCamera*)obj;
            if (camera->getTrackedObjectLoadName().size()>0)
            {
                std::map<std::string,CSceneObject*>::const_iterator it=_objectTempNamesMap.find(camera->getTrackedObjectLoadName());
                if (it!=_objectTempNamesMap.end())
                    camera->setTrackedObjectID(it->second->getObjectHandle());
            }
        }
        // Handle proximitySensor sensable entity linking:
        if (obj->getObjectType()==sim_object_proximitysensor_type)
        {
            CProxSensor* proxSensor=(CProxSensor*)obj;
            if (proxSensor->getSensableObjectLoadName().size()>0)
            {
                std::map<std::string,CSceneObject*>::const_iterator it=_objectTempNamesMap.find(proxSensor->getSensableObjectLoadName());
                if (it!=_objectTempNamesMap.end())
                    proxSensor->setSensableObject(it->second->getObjectHandle());
                else
                {
                    std::map<std::string,CCollection*>::const_iterator itColl=_collectionLoadNamesMap.find(proxSensor->getSensableObjectLoadName());
                    if (itColl!=_collectionLoadNamesMap.end())
                        proxSensor->setSensableObject(itColl->second->getCollectionHandle());
                }
            }
        }
        // Handle visionSensor renderable entity linking:
        if (obj->getObjectType()==sim_object_visionsensor_type)
        {
            CVisionSensor* visionSensor=(CVisionSensor*)obj;
            if (visionSensor->getDdetectableEntityLoadName().size()>0)
            {
                std::map<std::string,CSceneObject*>::const_iterator it=_objectTempNamesMap.find(visionSensor->getDdetectableEntityLoadName());
                if (it!=_objectTempNamesMap.end())
                    visionSensor->setDetectableEntityHandle(it->second->getObjectHandle());
                else
                {
                    std::map<std::string,CCollection*>::const_iterator itColl=_collectionLoadNamesMap.find(visionSensor->getDdetectableEntityLoadName());
                    if (itColl!=_collectionLoadNamesMap.end())
                        visionSensor->setDetectableEntityHandle(itColl->second->getCollectionHandle());
                }
            }
        }
    }
    setEnableRemoteWorldsSync(true);
    rebuildRemoteWorlds();

    return(retVal);
}

bool CWorld::_saveSimpleXmlScene(CSer& ar)
{
    bool retVal=true;
    bool isScene=(ar.getFileType()==CSer::filetype_csim_xml_simplescene_file);

    ar.xmlAddNode_comment(" 'environment' tag: has no effect when loading a model ",false);
    ar.xmlPushNewNode(SERX_ENVIRONMENT);
    environment->serialize(ar);
    ar.xmlPopNode();

    ar.xmlAddNode_comment(" 'settings' tag: has no effect when loading a model ",false);
    ar.xmlPushNewNode(SERX_SETTINGS);
    mainSettings->serialize(ar);
    ar.xmlPopNode();

    ar.xmlAddNode_comment(" 'dynamics' tag: has no effect when loading a model ",false);
    ar.xmlPushNewNode(SERX_DYNAMICS);
    dynamicsContainer->serialize(ar);
    ar.xmlPopNode();

    ar.xmlAddNode_comment(" 'simulation' tag: has no effect when loading a model ",false);
    ar.xmlPushNewNode(SERX_SIMULATION);
    simulation->serialize(ar);
    ar.xmlPopNode();

    for (size_t i=0;i<collections->getObjectCount();i++)
    {
        ar.xmlPushNewNode(SERX_COLLECTION);
        collections->getObjectFromIndex(i)->serialize(ar);
        ar.xmlPopNode();
    }

    for (size_t i=0;i<ikGroups->getObjectCount();i++)
    {
        ar.xmlPushNewNode(SERX_IK);
        ikGroups->getObjectFromIndex(i)->serialize(ar);
        ar.xmlPopNode();
    }

    for (size_t i=0;i<sceneObjects->getOrphanCount();i++)
        sceneObjects->writeSimpleXmlSceneObjectTree(ar,sceneObjects->getOrphanFromIndex(i));

    return(retVal);
}

void CWorld::_simulationAboutToStart()
{
    buttonBlockContainer->simulationAboutToStart();
    dynamicsContainer->simulationAboutToStart();
    luaScriptContainer->simulationAboutToStart();
    sceneObjects->simulationAboutToStart();
    pageContainer->simulationAboutToStart();
    collisions->simulationAboutToStart();
    distances->simulationAboutToStart();
    collections->simulationAboutToStart();
    ikGroups->simulationAboutToStart();
    pathPlanning->simulationAboutToStart();
    simulation->simulationAboutToStart();
}

void CWorld::_simulationPaused()
{
    CLuaScriptObject* mainScript=luaScriptContainer->getMainScript();
    if (mainScript!=nullptr)
        mainScript->runMainScript(sim_syscb_suspend,nullptr,nullptr,nullptr);
}

void CWorld::_simulationAboutToResume()
{
    CLuaScriptObject* mainScript=luaScriptContainer->getMainScript();
    if (mainScript!=nullptr)
        mainScript->runMainScript(sim_syscb_resume,nullptr,nullptr,nullptr);
}

void CWorld::_simulationAboutToStep()
{
    ikGroups->resetCalculationResults();
}

void CWorld::_simulationAboutToEnd()
{
    luaScriptContainer->simulationAboutToEnd(); // will call a last time the main and all non-threaded child scripts, then reset them
}

void CWorld::_simulationEnded()
{
    drawingCont->simulationEnded();
    pointCloudCont->simulationEnded();
    bannerCont->simulationEnded();
    buttonBlockContainer->simulationEnded();
    dynamicsContainer->simulationEnded();
    signalContainer->simulationEnded();
    luaScriptContainer->simulationEnded();
    sceneObjects->simulationEnded();
    pageContainer->simulationEnded();
    collisions->simulationEnded();
    distances->simulationEnded();
    collections->simulationEnded();
    ikGroups->simulationEnded();
    pathPlanning->simulationEnded();
    simulation->simulationEnded();
    commTubeContainer->simulationEnded();
}

void CWorld::_getMinAndMaxNameSuffixes(int& smallestSuffix,int& biggestSuffix) const
{
    smallestSuffix=SIM_MAX_INT;
    biggestSuffix=-1;
    int minS,maxS;
    buttonBlockContainer->getMinAndMaxNameSuffixes(minS,maxS);
    if (minS<smallestSuffix)
        smallestSuffix=minS;
    if (maxS>biggestSuffix)
        biggestSuffix=maxS;
    sceneObjects->getMinAndMaxNameSuffixes(minS,maxS);
    if (minS<smallestSuffix)
        smallestSuffix=minS;
    if (maxS>biggestSuffix)
        biggestSuffix=maxS;
    collisions->getMinAndMaxNameSuffixes(minS,maxS);
    if (minS<smallestSuffix)
        smallestSuffix=minS;
    if (maxS>biggestSuffix)
        biggestSuffix=maxS;
    distances->getMinAndMaxNameSuffixes(minS,maxS);
    if (minS<smallestSuffix)
        smallestSuffix=minS;
    if (maxS>biggestSuffix)
        biggestSuffix=maxS;
    collections->getMinAndMaxNameSuffixes(minS,maxS);
    if (minS<smallestSuffix)
        smallestSuffix=minS;
    if (maxS>biggestSuffix)
        biggestSuffix=maxS;
    ikGroups->getMinAndMaxNameSuffixes(minS,maxS);
    if (minS<smallestSuffix)
        smallestSuffix=minS;
    if (maxS>biggestSuffix)
        biggestSuffix=maxS;
    pathPlanning->getMinAndMaxNameSuffixes(minS,maxS);
    if (minS<smallestSuffix)
        smallestSuffix=minS;
    if (maxS>biggestSuffix)
        biggestSuffix=maxS;
}

int CWorld::_getSuffixOffsetForGeneralObjectToAdd(bool tempNames,std::vector<CSceneObject*>* loadedObjectList,
                                                 std::vector<CCollection*>* loadedCollectionList,
                                                 std::vector<CCollisionObject*>* loadedCollisionList,
                                                 std::vector<CDistanceObject*>* loadedDistanceList,
                                                 std::vector<CIkGroup*>* loadedIkGroupList,
                                                 std::vector<CPathPlanningTask*>* loadedPathPlanningTaskList,
                                                 std::vector<CButtonBlock*>* loadedButtonBlockList,
                                                 std::vector<CLuaScriptObject*>* loadedLuaScriptList) const
{
    // 1. We find out about the smallest suffix to paste:
    int smallestSuffix=SIM_MAX_INT;
    // sceneObjects:
    if (loadedObjectList!=nullptr)
    {
        for (size_t i=0;i<loadedObjectList->size();i++)
        {
            std::string str(loadedObjectList->at(i)->getObjectName());
            if (tempNames)
                str=loadedObjectList->at(i)->getObjectTempName();
            int s=tt::getNameSuffixNumber(str.c_str(),true);
            if (i==0)
                smallestSuffix=s;
            else
            {
                if (s<smallestSuffix)
                    smallestSuffix=s;
            }
        }
    }
    // Collections:
    if (loadedCollectionList!=nullptr)
    {
        for (size_t i=0;i<loadedCollectionList->size();i++)
        {
            int s=tt::getNameSuffixNumber(loadedCollectionList->at(i)->getCollectionName().c_str(),true);
            if (s<smallestSuffix)
                smallestSuffix=s;
        }
    }
    // Collisions:
    if (loadedCollisionList!=nullptr)
    {
        for (size_t i=0;i<loadedCollisionList->size();i++)
        {
            int s=tt::getNameSuffixNumber(loadedCollisionList->at(i)->getObjectName().c_str(),true);
            if (s<smallestSuffix)
                smallestSuffix=s;
        }
    }
    // Distances:
    if (loadedDistanceList!=nullptr)
    {
        for (size_t i=0;i<loadedDistanceList->size();i++)
        {
            int s=tt::getNameSuffixNumber(loadedDistanceList->at(i)->getObjectName().c_str(),true);
            if (s<smallestSuffix)
                smallestSuffix=s;
        }
    }
    // IK Groups:
    if (loadedIkGroupList!=nullptr)
    {
        for (size_t i=0;i<loadedIkGroupList->size();i++)
        {
            int s=tt::getNameSuffixNumber(loadedIkGroupList->at(i)->getObjectName().c_str(),true);
            if (s<smallestSuffix)
                smallestSuffix=s;
        }
    }
    // Path planning tasks:
    if (loadedPathPlanningTaskList!=nullptr)
    {
        for (size_t i=0;i<loadedPathPlanningTaskList->size();i++)
        {
            int s=tt::getNameSuffixNumber(loadedPathPlanningTaskList->at(i)->getObjectName().c_str(),true);
            if (s<smallestSuffix)
                smallestSuffix=s;
        }
    }
    // 2D Elements:
    if (loadedButtonBlockList!=nullptr)
    {
        for (size_t i=0;i<loadedButtonBlockList->size();i++)
        {
            int s=tt::getNameSuffixNumber(loadedButtonBlockList->at(i)->getBlockName().c_str(),true);
            if (s<smallestSuffix)
                smallestSuffix=s;
        }
    }

    // 2. Now we find out about the highest suffix among existing objects (already in the scene):
    int biggestSuffix,smallestSuffixDummy;

    _getMinAndMaxNameSuffixes(smallestSuffixDummy,biggestSuffix);
    return(biggestSuffix-smallestSuffix+1);
}

bool CWorld::_canSuffix1BeSetToSuffix2(int suffix1,int suffix2) const
{
    if (!sceneObjects->canSuffix1BeSetToSuffix2(suffix1,suffix2))
        return(false);
    if (!buttonBlockContainer->canSuffix1BeSetToSuffix2(suffix1,suffix2))
        return(false);
    if (!collisions->canSuffix1BeSetToSuffix2(suffix1,suffix2))
        return(false);
    if (!distances->canSuffix1BeSetToSuffix2(suffix1,suffix2))
        return(false);
    if (!collections->canSuffix1BeSetToSuffix2(suffix1,suffix2))
        return(false);
    if (!ikGroups->canSuffix1BeSetToSuffix2(suffix1,suffix2))
        return(false);
    if (!pathPlanning->canSuffix1BeSetToSuffix2(suffix1,suffix2))
        return(false);
    return(true);
}

void CWorld::_setSuffix1ToSuffix2(int suffix1,int suffix2)
{
    sceneObjects->setSuffix1ToSuffix2(suffix1,suffix2);
    buttonBlockContainer->setSuffix1ToSuffix2(suffix1,suffix2);
    collisions->setSuffix1ToSuffix2(suffix1,suffix2);
    distances->setSuffix1ToSuffix2(suffix1,suffix2);
    collections->setSuffix1ToSuffix2(suffix1,suffix2);
    ikGroups->setSuffix1ToSuffix2(suffix1,suffix2);
    pathPlanning->setSuffix1ToSuffix2(suffix1,suffix2);
}

void CWorld::_prepareFastLoadingMapping(std::vector<int>& map)
{
    std::vector<int> mapC(map);
    map.clear();
    int minVal=0;
    int maxVal=0;
    for (size_t i=0;i<mapC.size()/2;i++)
    {
        int v=mapC[2*i+0];
        if ( (v<minVal)||(i==0) )
            minVal=v;
        if ( (v>maxVal)||(i==0) )
            maxVal=v;
    }
    map.push_back(minVal);
    if (mapC.size()!=0)
    {
        for (int i=0;i<maxVal-minVal+1;i++)
            map.push_back(-1);
        for (size_t i=0;i<mapC.size()/2;i++)
            map[1+mapC[2*i+0]-minVal]=mapC[2*i+1];
    }
}

void CWorld::appendLoadOperationIssue(int verbosity,const char* text,int objectId)
{
    if (text==nullptr)
        _loadOperationIssues.clear();
    else
    {
        SLoadOperationIssue issue;
        issue.verbosity=verbosity;
        issue.message=text;
        issue.objectHandle=objectId;
        _loadOperationIssues.push_back(issue);
    }
}

int CWorld::getLoadingMapping(const std::vector<int>* map,int oldVal)
{
    if ( (oldVal<0)||((oldVal-map->at(0))>int(map->size())-2) )
        return(-1);
    return(map->at(oldVal+1-map->at(0)));
}
