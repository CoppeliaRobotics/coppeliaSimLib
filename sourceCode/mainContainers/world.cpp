#include <world.h>
#include <mesh.h>
#include <utils.h>
#include <tt.h>
#include <app.h>
#include <simFlavor.h>
#ifdef SIM_WITH_GUI
#include <guiApp.h>
#endif

std::vector<SLoadOperationIssue> CWorld::_loadOperationIssues;

CWorld::CWorld()
{
    collections = nullptr;
    distances_old = nullptr;
    collisions_old = nullptr;
    ikGroups_old = nullptr;
    sceneObjects = nullptr;
    commTubeContainer_old = nullptr;
    dynamicsContainer = nullptr;
    undoBufferContainer = nullptr;
    outsideCommandQueue_old = nullptr;
    buttonBlockContainer_old = nullptr;
    environment = nullptr;
    pageContainer = nullptr;
    mainSettings_old = nullptr;
    pathPlanning_old = nullptr;
    textureContainer = nullptr;
    simulation = nullptr;
    customSceneData_old = nullptr;
    cacheData = nullptr;
    drawingCont = nullptr;
    pointCloudCont_old = nullptr;
    ghostObjectCont_old = nullptr;
    bannerCont_old = nullptr;
    _worldHandle = -1;
}

CWorld::~CWorld()
{
}

void CWorld::removeWorld_oldIk()
{
    App::worldContainer->pluginContainer->oldIkPlugin_emptyEnvironment();
    sceneObjects->remove_oldIk();
    ikGroups_old->remove_oldIk();
}

void CWorld::initializeWorld()
{
    customSceneData_volatile.setItemsAreVolatile();
    undoBufferContainer = new CUndoBufferCont();
    outsideCommandQueue_old = new COutsideCommandQueue();
    buttonBlockContainer_old = new CButtonBlockContainer(true);
    simulation = new CSimulation();
    textureContainer = new CTextureContainer();
    ikGroups_old = new CIkGroupContainer();
    collections = new CCollectionContainer();
    distances_old = new CDistanceObjectContainer_old();
    collisions_old = new CCollisionObjectContainer_old();
    sceneObjects = new CSceneObjectContainer();
    pathPlanning_old = new CRegisteredPathPlanningTasks();
    environment = new CEnvironment();
    pageContainer = new CPageContainer();
    mainSettings_old = new CMainSettings();
    customSceneData_old = new CCustomData_old();
    cacheData = new CCacheCont();
    drawingCont = new CDrawingContainer();
    pointCloudCont_old = new CPointCloudContainer_old();
    ghostObjectCont_old = new CGhostObjectContainer();
    bannerCont_old = new CBannerContainer();
    dynamicsContainer = new CDynamicsContainer();
    commTubeContainer_old = new CCommTubeContainer();
}

void CWorld::clearScene(bool notCalledFromUndoFunction)
{
#ifdef SIM_WITH_GUI
    if (GuiApp::mainWindow != nullptr)
        GuiApp::mainWindow->codeEditorContainer->sceneClosed(environment->getSceneUniqueID());
#endif

    if (notCalledFromUndoFunction)
        undoBufferContainer->emptySceneProcedure();
    environment->setSceneIsClosingFlag(true);
    sceneObjects->eraseAllObjects(true);
    collections->removeAllCollections();
    collections->setUpDefaultValues();
    ikGroups_old->removeAllIkGroups();
    distances_old->removeAllDistanceObjects();
    distances_old->setUpDefaultValues();
    collisions_old->removeAllCollisionObjects();
    collisions_old->setUpDefaultValues();
    pathPlanning_old->removeAllTasks();

    simulation->setUpDefaultValues();
    if (buttonBlockContainer_old != nullptr)
        buttonBlockContainer_old->removeAllBlocks(false);
    environment->setUpDefaultValues();
    pageContainer->emptySceneProcedure();

    simulation->setUpDefaultValues();
    pageContainer->emptySceneProcedure();

    customSceneData.setData(nullptr, nullptr, 0, true);
    customSceneData_volatile.setData(nullptr, nullptr, 0, true);
    customSceneData_old->removeAllData();
    if (notCalledFromUndoFunction)
        mainSettings_old->setUpDefaultValues();
    cacheData->clearCache();
    environment->setSceneIsClosingFlag(false);
}

void CWorld::deleteWorld()
{
    delete undoBufferContainer;
    undoBufferContainer = nullptr;
    delete dynamicsContainer;
    dynamicsContainer = nullptr;
    delete mainSettings_old;
    mainSettings_old = nullptr;
    delete pageContainer;
    pageContainer = nullptr;
    delete environment;
    environment = nullptr;
    delete pathPlanning_old;
    pathPlanning_old = nullptr;
    delete sceneObjects;
    sceneObjects = nullptr;
    delete collisions_old;
    collisions_old = nullptr;
    delete distances_old;
    distances_old = nullptr;
    delete collections;
    collections = nullptr;
    delete ikGroups_old;
    ikGroups_old = nullptr;
    delete textureContainer;
    textureContainer = nullptr;
    delete simulation;
    simulation = nullptr;
    delete buttonBlockContainer_old;
    buttonBlockContainer_old = nullptr;
    delete outsideCommandQueue_old;
    outsideCommandQueue_old = nullptr;
    delete customSceneData_old;
    customSceneData_old = nullptr;
    delete cacheData;
    cacheData = nullptr;
    delete drawingCont;
    drawingCont = nullptr;
    delete pointCloudCont_old;
    pointCloudCont_old = nullptr;
    delete ghostObjectCont_old;
    ghostObjectCont_old = nullptr;
    delete bannerCont_old;
    bannerCont_old = nullptr;
    delete commTubeContainer_old;
    commTubeContainer_old = nullptr;
}

void CWorld::rebuildWorld_oldIk()
{
    sceneObjects->buildOrUpdate_oldIk();
    sceneObjects->connect_oldIk();
    ikGroups_old->buildOrUpdate_oldIk();
    ikGroups_old->connect_oldIk();
}

bool CWorld::loadScene(CSer &ar, bool forUndoRedoOperation)
{
    bool retVal = false;
    sceneObjects->eraseAllObjects(true);
    if (ar.getFileType() == CSer::filetype_csim_xml_simplescene_file)
    {
        retVal = _loadSimpleXmlSceneOrModel(ar);
        sceneObjects->deselectObjects();
    }
    else
    {
        retVal = _loadModelOrScene(ar, false, true, false, false, nullptr, nullptr, nullptr);
        if (!forUndoRedoOperation)
        {
            App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(
                sim_message_eventcallback_sceneloaded);
            App::worldContainer->setModificationFlag(8); // scene loaded
            outsideCommandQueue_old->addCommand(sim_message_scene_loaded, 0, 0, 0, 0, nullptr, 0);
        }
    }
    return (retVal);
}

void CWorld::saveScene(CSer &ar)
{
    if (ar.getFileType() == CSer::filetype_csim_xml_simplescene_file)
    {
        App::worldContainer->callScripts(sim_syscb_beforesave, nullptr, nullptr);
        _saveSimpleXmlScene(ar);
        App::worldContainer->callScripts(sim_syscb_aftersave, nullptr, nullptr);
        return;
    }

    App::worldContainer->callScripts(sim_syscb_beforesave, nullptr, nullptr);

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
        environment->modelThumbnail_notSerializedHere.serialize(ar, false);
        if (ar.setWritingMode())
            environment->modelThumbnail_notSerializedHere.serialize(ar, false);
    }
    else
    {
        ar.xmlPushNewNode(SERX_MODEL_THUMBNAIL);
        environment->modelThumbnail_notSerializedHere.serialize(ar, false);
        ar.xmlPopNode();
    }
    //****************************************************

    // Textures:
    int textCnt = 0;
    while (textureContainer->getObjectAtIndex(textCnt) != nullptr)
    {
        CTextureObject *it = textureContainer->getObjectAtIndex(textCnt);
        if (ar.isBinary())
            textureContainer->storeTextureObject(ar, it);
        else
        {
            ar.xmlPushNewNode(SERX_TEXTURE);
            textureContainer->storeTextureObject(ar, it);
            ar.xmlPopNode();
        }
        textCnt++;
    }

    // DynMaterial objects:
    // We only save this for backward compatibility, but not needed for CoppeliaSim's from 3.4.0 on:
    //------------------------------------------------------------
    if (ar.isBinary())
    {
        int dynObjId = SIM_IDSTART_DYNMATERIAL_old;
        for (size_t i = 0; i < sceneObjects->getObjectCount(sim_sceneobject_shape); i++)
        {
            CShape *it = sceneObjects->getShapeFromIndex(i);
            CDynMaterialObject *mat = it->getDynMaterial();
            it->getMesh()->setDynMaterialId_old(dynObjId);
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
        for (size_t i = 0; i < sceneObjects->getObjectCount(sim_sceneobject_shape); i++)
        {
            CShape *it = sceneObjects->getShapeFromIndex(i);
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

    // Save objects in hierarchial order!
    std::vector<CSceneObject *> allObjects;
    sceneObjects->getObjects_hierarchyOrder(allObjects);
    for (size_t i = 0; i < allObjects.size(); i++)
    {
        CSceneObject *it = allObjects[i];
        if (ar.isBinary())
            sceneObjects->writeSceneObject(ar, it);
        else
        {
            ar.xmlPushNewNode(SERX_SCENEOBJECT);
            sceneObjects->writeSceneObject(ar, it);
            ar.xmlPopNode();
        }
    }

    // Old:
    // --------------------------
    if (ar.isBinary())
    {
        ar.storeDataName(SER_GHOSTS);
        ar.setCountingMode();
        ghostObjectCont_old->serialize(ar);
        if (ar.setWritingMode())
            ghostObjectCont_old->serialize(ar);
    }
    else
    {
        ar.xmlPushNewNode(SERX_GHOSTS);
        ghostObjectCont_old->serialize(ar);
        ar.xmlPopNode();
    }
    // --------------------------

    if (ar.isBinary())
    {
        ar.storeDataName(SER_SETTINGS);
        ar.setCountingMode();
        mainSettings_old->serialize(ar);
        if (ar.setWritingMode())
            mainSettings_old->serialize(ar);
    }
    else
    {
        ar.xmlPushNewNode(SERX_SETTINGS);
        mainSettings_old->serialize(ar);
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

    // Old:
    // --------------------------
    for (size_t i = 0; i < collisions_old->getObjectCount(); i++)
    {
        CCollisionObject_old *collObj = collisions_old->getObjectFromIndex(i);
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
    for (size_t i = 0; i < distances_old->getObjectCount(); i++)
    {
        CDistanceObject_old *distObj = distances_old->getObjectFromIndex(i);
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
    for (size_t i = 0; i < ikGroups_old->getObjectCount(); i++)
    {
        CIkGroup_old *ikg = ikGroups_old->getObjectFromIndex(i);
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
    {
        for (size_t i = 0; i < pathPlanning_old->allObjects.size(); i++)
        {
            ar.storeDataName(SER_PATH_PLANNING);
            ar.setCountingMode();
            pathPlanning_old->allObjects[i]->serialize(ar);
            if (ar.setWritingMode())
                pathPlanning_old->allObjects[i]->serialize(ar);
        }
    }
    // --------------------------

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
        customSceneData.serializeData(ar, nullptr);
        if (ar.setWritingMode())
            customSceneData.serializeData(ar, nullptr);
    }
    else
    {
        ar.xmlPushNewNode(SERX_SCENE_CUSTOM_DATA);
        customSceneData.serializeData(ar, nullptr);
        ar.xmlPopNode();
    }
    // Keep a while for backward compatibility (e.g. until V4.4.0)
    //------------------------------------------------------------
    if (ar.isBinary())
    {
        ar.storeDataName(SER_SCENE_CUSTOM_DATA_OLD);
        ar.setCountingMode();
        customSceneData_old->serializeData(ar, nullptr, -1);
        if (ar.setWritingMode())
            customSceneData_old->serializeData(ar, nullptr, -1);
    }
    else
    {
        ar.xmlPushNewNode(SERX_SCENE_CUSTOM_DATA_OLD);
        customSceneData_old->serializeData(ar, nullptr, -1);
        ar.xmlPopNode();
    }
    //------------------------------------------------------------

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

    // Old:
    // --------------------------
    for (size_t i = 0; i < collections->getObjectCount(); i++)
    {
        CCollection *coll = collections->getObjectFromIndex(i);
        if (coll->getCreatorHandle() == -2)
        {
            if (ar.isBinary())
            {
                ar.storeDataName(SER_COLLECTION);
                ar.setCountingMode();
                coll->serialize(ar);
                if (ar.setWritingMode())
                    collections->getObjectFromIndex(i)->serialize(ar);
            }
            else
            {
                ar.xmlPushNewNode(SERX_COLLECTION);
                coll->serialize(ar);
                ar.xmlPopNode();
            }
        }
    }
    if (ar.isBinary() && (!App::userSettings->disableOpenGlBasedCustomUi))
    {
        for (size_t i = 0; i < buttonBlockContainer_old->allBlocks.size(); i++)
        {
            CButtonBlock *bblk = buttonBlockContainer_old->allBlocks[i];
            if ((bblk->getAttributes() & sim_ui_property_systemblock) == 0)
            {
                ar.storeDataName(SER_BUTTON_BLOCK_old);
                ar.setCountingMode();
                bblk->serialize(ar);
                if (ar.setWritingMode())
                    bblk->serialize(ar);
            }
        }
    }
    // --------------------------

    // We serialize the old scripts (not the add-on scripts nor the sandbox script):
    for (size_t i = 0; i < sceneObjects->embeddedScriptContainer->allScripts.size(); i++)
    {
        CScriptObject *it = sceneObjects->embeddedScriptContainer->allScripts[i];
        if (it->isSimulatonCustomizationOrMainScript())
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

    App::worldContainer->callScripts(sim_syscb_aftersave, nullptr, nullptr);
}

bool CWorld::loadModel(CSer &ar, bool justLoadThumbnail, bool forceModelAsCopy, C7Vector *optionalModelTr,
                       C3Vector *optionalModelBoundingBoxSize, double *optionalModelNonDefaultTranslationStepSize)
{
    bool retVal;
    if (ar.getFileType() == CSer::filetype_csim_xml_simplemodel_file)
        retVal = _loadSimpleXmlSceneOrModel(ar);
    else
        retVal = _loadModelOrScene(ar, true, false, justLoadThumbnail, forceModelAsCopy, optionalModelTr,
                                   optionalModelBoundingBoxSize, optionalModelNonDefaultTranslationStepSize);
    if (!justLoadThumbnail)
    {
        App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(
            sim_message_eventcallback_modelloaded);
        App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(
            sim_message_model_loaded); // for backward compatibility

        outsideCommandQueue_old->addCommand(sim_message_model_loaded, 0, 0, 0, 0, nullptr, 0); // only for Lua
        App::worldContainer->setModificationFlag(4);                                       // model loaded
    }
    return (retVal);
}

void CWorld::simulationAboutToStart()
{
    TRACE_INTERNAL;

#ifdef SIM_WITH_GUI
    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId = SIMULATION_ABOUT_TO_START_UITHREADCMD;
    GuiApp::uiThread->executeCommandViaUiThread(&cmdIn, &cmdOut);
#endif

    App::worldContainer->callScripts(sim_syscb_beforesimulation, nullptr, nullptr);

    _initialObjectUniqueIdentifiersForRemovingNewObjects.clear();
    for (size_t i = 0; i < sceneObjects->getObjectCount(); i++)
    {
        CSceneObject *it = sceneObjects->getObjectFromIndex(i);
        _initialObjectUniqueIdentifiersForRemovingNewObjects.push_back(it->getObjectUid());
    }
    App::undoRedo_sceneChanged("");

#ifdef SIM_WITH_GUI
    _savedMouseMode = GuiApp::getMouseMode();
    if (GuiApp::mainWindow != nullptr)
        GuiApp::mainWindow->codeEditorContainer->simulationAboutToStart();
#endif

    if (!App::worldContainer->pluginContainer->isGeomPluginAvailable())
    {
#ifdef SIM_WITH_GUI
        if (GuiApp::canShowDialogs())
            GuiApp::uiThread->messageBox_warning(
                GuiApp::mainWindow, "Warning",
                "The 'Geometric' plugin could not be initialized. Collision detection, distance calculation, and "
                "proximity sensor simulation will not work.",
                VMESSAGEBOX_OKELI, VMESSAGEBOX_REPLY_OK);
#endif
        App::logMsg(sim_verbosity_errors, "the 'Geometric' plugin could not be initialized. Collision detection,\n     "
                                          "  distance calculation, and proximity sensor simulation will not work.");
    }

    buttonBlockContainer_old->simulationAboutToStart(); // old
    sceneObjects->simulationAboutToStart();
    dynamicsContainer->simulationAboutToStart();
    pageContainer->simulationAboutToStart();
    collisions_old->simulationAboutToStart(); // old
    distances_old->simulationAboutToStart();  // old
    collections->simulationAboutToStart();
    ikGroups_old->simulationAboutToStart();     // old
    pathPlanning_old->simulationAboutToStart(); // old
    simulation->simulationAboutToStart();

    App::worldContainer->calcInfo->simulationAboutToStart();

    App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(
        sim_message_eventcallback_simulationabouttostart);

    App::worldContainer->setModificationFlag(2048); // simulation started

#ifdef SIM_WITH_GUI
    SSimulationThreadCommand cmd;
    cmd.cmdId = DISPLAY_VARIOUS_WARNING_MESSAGES_DURING_SIMULATION_CMD;
    App::appendSimulationThreadCommand(cmd, 1.0);
#endif

#ifdef SIM_WITH_GUI
    GuiApp::setToolbarRefreshFlag();
    GuiApp::setFullDialogRefreshFlag();
    if (GuiApp::canShowDialogs())
        GuiApp::mainWindow->simulationRecorder->startRecording(false);
#endif
}

void CWorld::simulationPaused()
{
    CScriptObject *mainScript = sceneObjects->embeddedScriptContainer->getMainScript();
    if (mainScript != nullptr)
        mainScript->systemCallMainScript(sim_syscb_suspend, nullptr, nullptr);

#ifdef SIM_WITH_GUI
    GuiApp::setToolbarRefreshFlag();
    GuiApp::setFullDialogRefreshFlag();
#endif
}

void CWorld::simulationAboutToResume()
{
    CScriptObject *mainScript = sceneObjects->embeddedScriptContainer->getMainScript();
    if (mainScript != nullptr)
        mainScript->systemCallMainScript(sim_syscb_resume, nullptr, nullptr);

#ifdef SIM_WITH_GUI
    GuiApp::setToolbarRefreshFlag();
    GuiApp::setFullDialogRefreshFlag();
#endif
}

void CWorld::simulationAboutToStep()
{
    ikGroups_old->resetCalculationResults();
}

void CWorld::simulationAboutToEnd()
{
    TRACE_INTERNAL;

    App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_simulationabouttoend);

    sceneObjects->simulationAboutToEnd();

#ifdef SIM_WITH_GUI
    if (GuiApp::mainWindow != nullptr)
        GuiApp::mainWindow->codeEditorContainer->simulationAboutToEnd();
#endif
}

void CWorld::simulationEnded(bool removeNewObjects)
{
    TRACE_INTERNAL;
    App::undoRedo_sceneChanged(
        ""); // keeps this (this has the objects in their last position, including additional objects)

    App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(
        sim_message_eventcallback_simulationended);
    App::worldContainer->setModificationFlag(4096); // simulation ended

    if (removeNewObjects)
    {
        std::vector<int> savedSelection(sceneObjects->getSelectedObjectHandlesPtr()[0]);
        std::vector<int> toRemove;
        for (size_t i = 0; i < sceneObjects->getObjectCount(); i++)
        {
            CSceneObject *it = sceneObjects->getObjectFromIndex(i);
            bool found = false;
            for (size_t j = 0; j < _initialObjectUniqueIdentifiersForRemovingNewObjects.size(); j++)
            {
                if (it->getObjectUid() == _initialObjectUniqueIdentifiersForRemovingNewObjects[j])
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                toRemove.push_back(it->getObjectHandle());
        }
        sceneObjects->eraseObjects(&toRemove, true);
        sceneObjects->setSelectedObjectHandles(savedSelection.data(), savedSelection.size());
    }
    _initialObjectUniqueIdentifiersForRemovingNewObjects.clear();

    drawingCont->simulationEnded();
    pointCloudCont_old->simulationEnded();
    bannerCont_old->simulationEnded();
    buttonBlockContainer_old->simulationEnded();
    dynamicsContainer->simulationEnded();
    sceneObjects->simulationEnded();
    pageContainer->simulationEnded();
    collisions_old->simulationEnded();
    distances_old->simulationEnded();
    collections->simulationEnded();
    ikGroups_old->simulationEnded();
    pathPlanning_old->simulationEnded();
    simulation->simulationEnded();
    commTubeContainer_old->simulationEnded();
    App::worldContainer->calcInfo->simulationEnded();
#ifdef SIM_WITH_GUI
    App::worldContainer->serialPortContainer->simulationEnded();
    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId = SIMULATION_JUST_ENDED_UITHREADCMD;
    GuiApp::uiThread->executeCommandViaUiThread(&cmdIn, &cmdOut);
    GuiApp::setMouseMode(_savedMouseMode);
    GuiApp::setToolbarRefreshFlag();
    GuiApp::setFullDialogRefreshFlag();
#endif

    App::undoRedo_sceneChanged(""); // keeps this (additional objects were removed, and object positions were reset)

    App::worldContainer->callScripts(sim_syscb_aftersimulation, nullptr, nullptr);
}

void CWorld::addGeneralObjectsToWorldAndPerformMappings(
    std::vector<CSceneObject *> *loadedObjectList, std::vector<CCollection *> *loadedCollectionList,
    std::vector<CCollisionObject_old *> *loadedCollisionList, std::vector<CDistanceObject_old *> *loadedDistanceList,
    std::vector<CIkGroup_old *> *loadedIkGroupList, std::vector<CPathPlanningTask *> *loadedPathPlanningTaskList,
    std::vector<CButtonBlock *> *loadedButtonBlockList, std::vector<CScriptObject *> *loadedLuaScriptList,
    std::vector<CTextureObject *> &loadedTextureObjectList,
    std::vector<CDynMaterialObject *> &loadedDynMaterialObjectList, bool model, int fileSimVersion,
    bool forceModelAsCopy)
{
    TRACE_INTERNAL;
    // We check what suffix offset is needed for this model (in case of a scene, the offset is ignored since we won't
    // introduce the objects as copies!):
    int suffixOffset = _getSuffixOffsetForGeneralObjectToAdd(
        false, loadedObjectList, loadedCollectionList, loadedCollisionList, loadedDistanceList, loadedIkGroupList,
        loadedPathPlanningTaskList, loadedButtonBlockList, loadedLuaScriptList);
    // We have 3 cases:
    // 1. We are loading a scene, 2. We are loading a model, 3. We are pasting objects
    // We add objects to the scene as copies only if we also add at least one associated script and we don't have a
    // scene. Otherwise objects are added and no '#' (or no modified suffix) will appear in their names. Following line
    // summarizes this:
    bool objectIsACopy =
        (((loadedLuaScriptList->size() != 0) || forceModelAsCopy) && model); // scenes are not treated like copies!

    // Texture data:
    std::map<int, int> textureMapping;
    for (size_t i = 0; i < loadedTextureObjectList.size(); i++)
    {
        int oldHandle = loadedTextureObjectList[i]->getObjectID();
        CTextureObject *handler = textureContainer->getObject(textureContainer->addObjectWithSuffixOffset(
            loadedTextureObjectList[i], objectIsACopy,
            suffixOffset)); // if a same object is found, the object is destroyed in addObject!
        if (handler != loadedTextureObjectList[i])
            loadedTextureObjectList[i] = handler; // this happens when a similar object is already present
        textureMapping[oldHandle] = handler->getObjectID();
    }

    // We add all sceneObjects:
    sceneObjects->enableObjectActualization(false);
    std::map<int, int> objectMapping;
    for (size_t i = 0; i < loadedObjectList->size(); i++)
    {
        loadedObjectList->at(i)->performTextureObjectLoadingMapping(&textureMapping);
        int oldHandle = loadedObjectList->at(i)->getObjectHandle();
        sceneObjects->addObjectToSceneWithSuffixOffset(loadedObjectList->at(i), objectIsACopy, suffixOffset, false);
        objectMapping[oldHandle] = loadedObjectList->at(i)->getObjectHandle();
        if (loadedObjectList->at(i)->getObjectType() == sim_sceneobject_shape)
        {
            CShape *shape = (CShape *)loadedObjectList->at(i);
            int matId = shape->getMesh()->getDynMaterialId_old();
            if ((fileSimVersion < 30303) && (matId >= 0))
            { // for backward compatibility(29/10/2016), when the dyn material was stored separaterly and shared among
              // shapes
                for (size_t j = 0; j < loadedDynMaterialObjectList.size(); j++)
                {
                    if (loadedDynMaterialObjectList[j]->getObjectID() == matId)
                    {
                        CDynMaterialObject *mat = loadedDynMaterialObjectList[j]->copyYourself();
                        shape->setDynMaterial(mat);
                        break;
                    }
                }
            }
            if (fileSimVersion < 30301)
            { // Following for backward compatibility (09/03/2016)
                CDynMaterialObject *mat = shape->getDynMaterial();
                if (mat->getEngineBoolParam_old(sim_bullet_body_sticky, nullptr))
                { // Formely sticky contact objects need to be adjusted for the new Bullet:
                    if (shape->getStatic())
                        mat->setFloatProperty(propMaterial_bulletFriction.name, mat->getFloatPropertyValue(propMaterial_bulletFriction0.name)); // the new Bullet friction
                    else
                        mat->setFloatProperty(propMaterial_bulletFriction.name, 0.25); // the new Bullet friction
                }
            }
            shape->getMesh()->setDynMaterialId_old(-1);
        }
    }
    sceneObjects->enableObjectActualization(true);
    sceneObjects->actualizeObjectInformation();

    // Remove any material that was loaded from a previous file version, where materials were still shared (until
    // V3.3.2)
    for (size_t i = 0; i < loadedDynMaterialObjectList.size(); i++)
        delete loadedDynMaterialObjectList[i];

    // Old:
    // -----------------
    // We add all the collections:
    std::map<int, int> collectionMapping;
    for (size_t i = 0; i < loadedCollectionList->size(); i++)
    {
        int oldHandle = loadedCollectionList->at(i)->getCollectionHandle();
        collections->addCollectionWithSuffixOffset(loadedCollectionList->at(i), objectIsACopy, suffixOffset);
        collectionMapping[oldHandle] = loadedCollectionList->at(i)->getCollectionHandle();
    }
    // We add all the collisions:
    std::map<int, int> collisionMapping;
    for (size_t i = 0; i < loadedCollisionList->size(); i++)
    {
        int oldHandle = loadedCollisionList->at(i)->getObjectHandle();
        collisions_old->addObjectWithSuffixOffset(loadedCollisionList->at(i), objectIsACopy, suffixOffset);
        collisionMapping[oldHandle] = loadedCollisionList->at(i)->getObjectHandle();
    }
    // We add all the distances:
    std::map<int, int> distanceMapping;
    for (size_t i = 0; i < loadedDistanceList->size(); i++)
    {
        int oldHandle = loadedDistanceList->at(i)->getObjectHandle();
        distances_old->addObjectWithSuffixOffset(loadedDistanceList->at(i), objectIsACopy, suffixOffset);
        distanceMapping[oldHandle] = loadedDistanceList->at(i)->getObjectHandle();
    }
    // We add all the ik groups:
    std::map<int, int> ikGroupMapping;
    for (size_t i = 0; i < loadedIkGroupList->size(); i++)
    {
        int oldHandle = loadedIkGroupList->at(i)->getObjectHandle();
        ikGroups_old->addIkGroupWithSuffixOffset(loadedIkGroupList->at(i), objectIsACopy, suffixOffset);
        ikGroupMapping[oldHandle] = loadedIkGroupList->at(i)->getObjectHandle();
    }
    // We add all the path planning tasks:
    std::map<int, int> pathPlanningTaskMapping;
    for (size_t i = 0; i < loadedPathPlanningTaskList->size(); i++)
    {
        int oldHandle = loadedPathPlanningTaskList->at(i)->getObjectID();
        pathPlanning_old->addObjectWithSuffixOffset(loadedPathPlanningTaskList->at(i), objectIsACopy, suffixOffset);
        pathPlanningTaskMapping[oldHandle] = loadedPathPlanningTaskList->at(i)->getObjectID();
    }
    // We add all the button blocks:
    std::map<int, int> buttonBlockMapping;
    for (size_t i = 0; i < loadedButtonBlockList->size(); i++)
    {
        int oldHandle = loadedButtonBlockList->at(i)->getBlockID();
        buttonBlockContainer_old->insertBlockWithSuffixOffset(loadedButtonBlockList->at(i), objectIsACopy, suffixOffset);
        buttonBlockMapping[oldHandle] = loadedButtonBlockList->at(i)->getBlockID();
    }
    // -----------------

    // We add all the old scripts:
    std::map<int, int> luaScriptMapping;
    for (size_t i = 0; i < loadedLuaScriptList->size(); i++)
    {
        int oldHandle = loadedLuaScriptList->at(i)->getScriptHandle();
        sceneObjects->embeddedScriptContainer->insertScript(loadedLuaScriptList->at(i));
        luaScriptMapping[oldHandle] = loadedLuaScriptList->at(i)->getScriptHandle();
    }

    sceneObjects->enableObjectActualization(false);

    // We do the mapping for the sceneObjects:
    for (size_t i = 0; i < loadedObjectList->size(); i++)
    {
        CSceneObject *it = loadedObjectList->at(i);
        it->performObjectLoadingMapping(&objectMapping, model);
        it->performScriptLoadingMapping(&luaScriptMapping);
        it->performCollectionLoadingMapping(&collectionMapping, model);
        it->performCollisionLoadingMapping(&collisionMapping, model);
        it->performDistanceLoadingMapping(&distanceMapping, model);
        it->performIkLoadingMapping(&ikGroupMapping, model);
    }
    // We do the mapping for the collections (OLD):
    for (size_t i = 0; i < loadedCollectionList->size(); i++)
    {
        CCollection *it = loadedCollectionList->at(i);
        it->performObjectLoadingMapping(&objectMapping);
    }
    // We do the mapping for the collisions (OLD):
    for (size_t i = 0; i < loadedCollisionList->size(); i++)
    {
        CCollisionObject_old *it = loadedCollisionList->at(i);
        it->performObjectLoadingMapping(&objectMapping);
        it->performCollectionLoadingMapping(&collectionMapping);
    }
    // We do the mapping for the distances (OLD):
    for (size_t i = 0; i < loadedDistanceList->size(); i++)
    {
        CDistanceObject_old *it = loadedDistanceList->at(i);
        it->performObjectLoadingMapping(&objectMapping);
        it->performCollectionLoadingMapping(&collectionMapping);
    }
    // We do the mapping for the ik groups (OLD):
    for (size_t i = 0; i < loadedIkGroupList->size(); i++)
    {
        CIkGroup_old *it = loadedIkGroupList->at(i);
        it->performObjectLoadingMapping(&objectMapping);
        it->performIkGroupLoadingMapping(&ikGroupMapping);
    }
    // We do the mapping for the path planning tasks (OLD):
    for (size_t i = 0; i < loadedPathPlanningTaskList->size(); i++)
    {
        CPathPlanningTask *it = loadedPathPlanningTaskList->at(i);
        it->performObjectLoadingMapping(&objectMapping);
        it->performCollectionLoadingMapping(&collectionMapping);
    }
    // We do the mapping for the 2D Elements (OLD):
    for (size_t i = 0; i < loadedButtonBlockList->size(); i++)
    {
        CButtonBlock *it = loadedButtonBlockList->at(i);
        it->performSceneObjectLoadingMapping(&objectMapping);
        it->performTextureObjectLoadingMapping(&textureMapping);
    }
    // We do the mapping for the Lua scripts:
    for (size_t i = 0; i < loadedLuaScriptList->size(); i++)
    {
        CScriptObject *it = loadedLuaScriptList->at(i);
        it->performSceneObjectLoadingMapping(&objectMapping);
    }

    // We do the mapping for the ghost objects (OLD):
    if (!model)
        ghostObjectCont_old->performObjectLoadingMapping(&objectMapping);

    // We set ALL texture object dependencies (not just for loaded objects):
    // We cannot use textureCont->updateAllDependencies, since the shape list is not yet actualized!
    textureContainer->clearAllDependencies();
    buttonBlockContainer_old->setTextureDependencies();
    sceneObjects->setTextureDependencies();

    sceneObjects->enableObjectActualization(true);
    sceneObjects->actualizeObjectInformation();

    if (!model)
        pageContainer->performObjectLoadingMapping(&objectMapping);

    // sceneObjects->actualizeObjectInformation();

    // Now clean-up suffixes equal or above those added, but only for models or objects copied into the scene (global
    // suffix clean-up can be done in the environment dialog):
    if (model) // condition was added on 29/9/2014
        cleanupHashNames_allObjects(suffixOffset - 1);

    /* Until 4/10/2013. Global suffix name clean-up. This was confusing!
        if (App::wc->simulation->isSimulationStopped()) // added on 2010/02/20 (otherwise objects can get automatically
       renamed during simulation!!) cleanupHashNames(-1);
    */

    //************ We need to initialize all object types (also because an object copied during simulation hasn't the
    // simulationEnded routine called!)
    bool simulationAlreadyRunning = !simulation->isSimulationStopped();
    if (simulationAlreadyRunning)
    {
        for (size_t i = 0; i < loadedObjectList->size(); i++)
            loadedObjectList->at(i)->initializeInitialValues(simulationAlreadyRunning);
        for (size_t i = 0; i < loadedButtonBlockList->size(); i++)
            loadedButtonBlockList->at(i)->initializeInitialValues(simulationAlreadyRunning);
        for (size_t i = 0; i < loadedCollisionList->size(); i++)
            loadedCollisionList->at(i)->initializeInitialValues(simulationAlreadyRunning);
        for (size_t i = 0; i < loadedDistanceList->size(); i++)
            loadedDistanceList->at(i)->initializeInitialValues(simulationAlreadyRunning);
        for (size_t i = 0; i < loadedCollectionList->size(); i++)
            loadedCollectionList->at(i)->initializeInitialValues(simulationAlreadyRunning);
        for (size_t i = 0; i < loadedIkGroupList->size(); i++)
            loadedIkGroupList->at(i)->initializeInitialValues(simulationAlreadyRunning);
        for (size_t i = 0; i < loadedPathPlanningTaskList->size(); i++)
            loadedPathPlanningTaskList->at(i)->initializeInitialValues(simulationAlreadyRunning);
        for (size_t i = 0; i < loadedLuaScriptList->size(); i++)
            loadedLuaScriptList->at(i)->initializeInitialValues(simulationAlreadyRunning);

        // Here we call the initializeInitialValues for all pages & views
        for (size_t i = 0; i < loadedObjectList->size(); i++)
            pageContainer->initializeInitialValues(simulationAlreadyRunning,
                                                   loadedObjectList->at(i)->getObjectHandle());
    }
    //**************************************************************************************

    // Here make sure that referenced objects still exists (when keeping original references):
    // ------------------------------------------------
    if (model)
    {
        std::map<std::string, int> uniquePersistentIds;
        for (size_t i = 0; i < sceneObjects->getObjectCount(); i++)
        {
            CSceneObject *obj = sceneObjects->getObjectFromIndex(i);
            uniquePersistentIds[obj->getUniquePersistentIdString()] = obj->getObjectHandle();
        }
        for (size_t i = 0; i < collisions_old->getObjectCount(); i++)
            uniquePersistentIds[collisions_old->getObjectFromIndex(i)->getUniquePersistentIdString()] =
                collisions_old->getObjectFromIndex(i)->getObjectHandle();
        for (size_t i = 0; i < distances_old->getObjectCount(); i++)
            uniquePersistentIds[distances_old->getObjectFromIndex(i)->getUniquePersistentIdString()] =
                distances_old->getObjectFromIndex(i)->getObjectHandle();
        for (size_t i = 0; i < ikGroups_old->getObjectCount(); i++)
            uniquePersistentIds[ikGroups_old->getObjectFromIndex(i)->getUniquePersistentIdString()] =
                ikGroups_old->getObjectFromIndex(i)->getObjectHandle();
        for (size_t i = 0; i < collections->getObjectCount(); i++)
            uniquePersistentIds[collections->getObjectFromIndex(i)->getUniquePersistentIdString()] =
                collections->getObjectFromIndex(i)->getCollectionHandle();
        for (size_t i = 0; i < loadedObjectList->size(); i++)
            loadedObjectList->at(i)->checkReferencesToOriginal(uniquePersistentIds);
    }
    // ------------------------------------------------

    // Now display the load operation issues:
    for (size_t i = 0; i < _loadOperationIssues.size(); i++)
    {
        int handle = _loadOperationIssues[i].objectHandle;
        std::string newTxt("NAME_NOT_FOUND");
        int handle2 = getLoadingMapping(&luaScriptMapping, handle);
        CScriptObject *script = sceneObjects->embeddedScriptContainer->getScriptObjectFromHandle(handle2);
        if (script != nullptr)
            newTxt = script->getShortDescriptiveName();
        std::string msg(_loadOperationIssues[i].message);
        utils::replaceSubstring(msg, "@@REPLACE@@", newTxt.c_str());
        App::logMsg(_loadOperationIssues[i].verbosity, msg.c_str());
    }
    appendLoadOperationIssue(-1, nullptr, -1); // clears it

    if (loadedIkGroupList->size() > 0)
    { // OLD
        for (size_t i = 0; i < loadedObjectList->size(); i++)
            loadedObjectList->at(i)->buildOrUpdate_oldIk();
        for (size_t i = 0; i < loadedObjectList->size(); i++)
            loadedObjectList->at(i)->connect_oldIk();
        for (size_t i = 0; i < loadedIkGroupList->size(); i++)
            loadedIkGroupList->at(i)->buildOrUpdate_oldIk();
        for (size_t i = 0; i < loadedIkGroupList->size(); i++)
            loadedIkGroupList->at(i)->connect_oldIk();
    }

    // We select what was loaded if we have a model loaded through the GUI:
    sceneObjects->deselectObjects();
    if (model)
    {
        for (size_t i = 0; i < loadedObjectList->size(); i++)
            sceneObjects->addObjectToSelection(loadedObjectList->at(i)->getObjectHandle());
    }
}

void CWorld::cleanupHashNames_allObjects(int suffix)
{ // This function will try to use the lowest hash naming possible (e.g. model#59 --> model and model#67 --> model#0 if
  // possible)
    // if suffix is -1, then all suffixes are cleaned, otherwise only those equal or above 'suffix'

    // 1. we get all object's smallest and biggest suffix:
    int smallestSuffix, biggestSuffix;
    _getMinAndMaxNameSuffixes(smallestSuffix, biggestSuffix);
    if (suffix <= 0)
        suffix = 0;
    for (int i = suffix; i <= biggestSuffix; i++)
    {
        for (int j = -1; j < i; j++)
        {
            if (_canSuffix1BeSetToSuffix2(i, j))
            {
                _setSuffix1ToSuffix2(i, j);
                break;
            }
        }
    }
}

void CWorld::announceObjectWillBeErased(const CSceneObject *object)
{
    sceneObjects->announceObjectWillBeErased(object);
    drawingCont->announceObjectWillBeErased(object);
    textureContainer->announceGeneralObjectWillBeErased(object->getObjectHandle(), -1);
    pageContainer->announceObjectWillBeErased(object->getObjectHandle()); // might trigger a view destruction!

    // Old:
    buttonBlockContainer_old->announceObjectWillBeErased(object->getObjectHandle());
    pathPlanning_old->announceObjectWillBeErased(object->getObjectHandle());
    collisions_old->announceObjectWillBeErased(object->getObjectHandle());
    distances_old->announceObjectWillBeErased(object->getObjectHandle());
    pointCloudCont_old->announceObjectWillBeErased(object->getObjectHandle());
    ghostObjectCont_old->announceObjectWillBeErased(object->getObjectHandle());
    bannerCont_old->announceObjectWillBeErased(object->getObjectHandle());
    collections->announceObjectWillBeErased(object->getObjectHandle()); // can trigger distance, collision
    ikGroups_old->announceObjectWillBeErased(object->getObjectHandle());
}

void CWorld::announceScriptWillBeErased(int scriptHandle, bool simulationScript, bool sceneSwitchPersistentScript)
{
    sceneObjects->announceScriptWillBeErased(scriptHandle, simulationScript, sceneSwitchPersistentScript);
}

void CWorld::announceScriptStateWillBeErased(int scriptHandle, bool simulationScript, bool sceneSwitchPersistentScript)
{
    collections->announceScriptStateWillBeErased(scriptHandle, simulationScript, sceneSwitchPersistentScript);
    drawingCont->announceScriptStateWillBeErased(scriptHandle, simulationScript, sceneSwitchPersistentScript);
}

CScriptObject *CWorld::getScriptObjectFromHandle(int scriptHandle) const
{
    CScriptObject *retVal = nullptr;
    if (sceneObjects != nullptr)
        retVal = sceneObjects->getScriptObjectFromHandle(scriptHandle);
    return (retVal);
}

CScriptObject *CWorld::getScriptObjectFromUid(int uid) const
{
    CScriptObject *retVal = nullptr;
    if (sceneObjects != nullptr)
        retVal = sceneObjects->getScriptObjectFromUid(uid);
    return (retVal);
}

void CWorld::getActiveScripts(std::vector<CScriptObject*>& scripts, bool reverse /*= false*/, bool alsoLegacyScripts /*= false*/) const
{
    TRACE_INTERNAL;
    sceneObjects->getActiveScripts(scripts, reverse, alsoLegacyScripts);
}

void CWorld::callScripts(int callType, CInterfaceStack *inStack, CInterfaceStack *outStack, CSceneObject *objectBranch /*=nullptr*/, int scriptToExclude /*=-1*/)
{
    TRACE_INTERNAL;
    sceneObjects->callScripts(callType, inStack, outStack, objectBranch, scriptToExclude);
}

void CWorld::pushGenesisEvents()
{
    CCbor *ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, nullptr, false);
    simulation->appendGenesisData(ev);
    environment->appendGenesisData(ev);
    customSceneData.appendEventData(nullptr, ev);
    customSceneData_volatile.appendEventData(nullptr, ev);
    dynamicsContainer->appendGenesisData(ev);
    sceneObjects->appendNonObjectGenesisData(ev);
    App::worldContainer->pushEvent();

    sceneObjects->pushObjectGenesisEvents();

    drawingCont->pushGenesisEvents();
    pointCloudCont_old->pushGenesisEvents();
}

// Old:
// -----------
void CWorld::announceIkGroupWillBeErased(int ikGroupHandle)
{
    sceneObjects->announceIkGroupWillBeErased(ikGroupHandle);
    ikGroups_old->announceIkGroupWillBeErased(ikGroupHandle);
}

void CWorld::announceCollectionWillBeErased(int collectionHandle)
{
    sceneObjects->announceCollectionWillBeErased(collectionHandle);
    collisions_old->announceCollectionWillBeErased(collectionHandle); // This can trigger a collision destruction!
    distances_old->announceCollectionWillBeErased(collectionHandle);  // This can trigger a distance destruction!
    pathPlanning_old->announceCollectionWillBeErased(
        collectionHandle); // This can trigger a path planning object destruction!
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
    if (textureContainer != nullptr)
        textureContainer->announceGeneralObjectWillBeErased(elementID, -1);
}

void CWorld::announce2DElementButtonWillBeErased(int elementID, int buttonID)
{
    if (textureContainer != nullptr)
        textureContainer->announceGeneralObjectWillBeErased(elementID, buttonID);
}
// -----------

bool CWorld::_loadModelOrScene(CSer &ar, bool selectLoaded, bool isScene, bool justLoadThumbnail, bool forceModelAsCopy,
                               C7Vector *optionalModelTr, C3Vector *optionalModelBoundingBoxSize,
                               double *optionalModelNonDefaultTranslationStepSize)
{
    appendLoadOperationIssue(-1, nullptr, -1); // clear

    CMesh::clearTempVerticesIndicesNormalsAndEdges();
    sceneObjects->deselectObjects();

    std::vector<CSceneObject *> loadedObjectList;
    std::vector<CTextureObject *> loadedTextureList;
    std::vector<CDynMaterialObject *> loadedDynMaterialList;
    std::vector<CCollection *> loadedCollectionList;
    std::vector<CCollisionObject_old *> loadedCollisionList;
    std::vector<CDistanceObject_old *> loadedDistanceList;
    std::vector<CIkGroup_old *> loadedIkGroupList;
    std::vector<CPathPlanningTask *> pathPlanningTaskList;
    std::vector<CButtonBlock *> loadedButtonBlockList;
    std::vector<CScriptObject *> loadedLuaScriptList;

    bool hasThumbnail = false;
    if (ar.isBinary())
    {
        int byteQuantity;
        std::string theName = "";
        while (theName.compare(SER_END_OF_FILE) != 0)
        {
            theName = ar.readDataName();
            if (theName.compare(SER_END_OF_FILE) != 0)
            {
                bool noHit = true;
                if (theName.compare(SER_MODEL_THUMBNAIL_INFO) == 0)
                {
                    ar >> byteQuantity;
                    C7Vector tr;
                    C3Vector bbs;
                    double ndss;
                    environment->modelThumbnail_notSerializedHere.serializeAdditionalModelInfos(ar, tr, bbs, ndss);
                    if (optionalModelTr != nullptr)
                        optionalModelTr[0] = tr;
                    if (optionalModelBoundingBoxSize != nullptr)
                        optionalModelBoundingBoxSize[0] = bbs;
                    if (optionalModelNonDefaultTranslationStepSize != nullptr)
                        optionalModelNonDefaultTranslationStepSize[0] = ndss;
                    noHit = false;
                }

                if (theName.compare(SER_MODEL_THUMBNAIL) == 0)
                {
                    ar >> byteQuantity;
                    environment->modelThumbnail_notSerializedHere.serialize(ar);
                    noHit = false;
                    if (justLoadThumbnail)
                        return (true);
                    hasThumbnail = true;
                }

                // Handle the heavy data here so we don't have duplicates (vertices, indices, normals and edges):
                //------------------------------------------------------------
                if (theName.compare(SER_VERTICESINDICESNORMALSEDGES) == 0)
                {
                    CMesh::clearTempVerticesIndicesNormalsAndEdges();
                    ar >> byteQuantity;
                    CMesh::serializeTempVerticesIndicesNormalsAndEdges(ar);
                    noHit = false;
                }
                //------------------------------------------------------------

                CSceneObject *it = sceneObjects->readSceneObject(ar, theName.c_str(), noHit);
                if (it != nullptr)
                {
                    loadedObjectList.push_back(it);
                    noHit = false;
                }

                CTextureObject *theTextureData = textureContainer->loadTextureObject(ar, theName, noHit);
                if (theTextureData != nullptr)
                {
                    loadedTextureList.push_back(theTextureData);
                    noHit = false;
                }
                if (theName.compare(SER_DYNMATERIAL) == 0)
                { // Following for backward compatibility (i.e. files written prior CoppeliaSim 3.4.0) (30/10/2016)
                    ar >> byteQuantity;
                    CDynMaterialObject *myNewObject = new CDynMaterialObject();
                    myNewObject->serialize(ar);
                    loadedDynMaterialList.push_back(myNewObject);
                    noHit = false;
                }
                if (theName.compare(SER_GHOSTS) == 0)
                {
                    ar >> byteQuantity;
                    ghostObjectCont_old->serialize(ar);
                    noHit = false;
                }
                if (theName.compare(SER_SETTINGS) == 0)
                {
                    ar >> byteQuantity;
                    mainSettings_old->serialize(ar);
                    noHit = false;
                }
                if (theName.compare(SER_ENVIRONMENT) == 0)
                {
                    ar >> byteQuantity;
                    environment->serialize(ar);
                    noHit = false;
                }
                if (theName.compare(SER_DYNAMICS) == 0)
                {
                    ar >> byteQuantity;
                    dynamicsContainer->serialize(ar);
                    noHit = false;
                }
                if (theName.compare(SER_SIMULATION) == 0)
                {
                    ar >> byteQuantity;
                    simulation->serialize(ar);
                    noHit = false;

                    // For backward compatibility (3/1/2012):
                    //************************************************
                    if (mainSettings_old->forBackwardCompatibility_03_01_2012_stillUsingStepSizeDividers)
                    { // This needs to be done AFTER simulation settings are loaded!
                        double bulletStepSize =
                            simulation->getTimeStep() /
                            double(mainSettings_old->dynamicsBULLETStepSizeDivider_forBackwardCompatibility_03_01_2012);
                        dynamicsContainer->setDesiredStepSize(bulletStepSize);
                    }
                    //************************************************
                }
                if (theName.compare(SER_VIEWS) == 0)
                {
                    ar >> byteQuantity;
                    pageContainer->serialize(ar);
                    noHit = false;
                }
                if (theName.compare(SER_COLLECTION) == 0)
                { // for backward compatibility 18.11.2020
                    if (CSimFlavor::getBoolVal(18))
                        App::logMsg(sim_verbosity_errors, "Contains collections...");
                    ar >> byteQuantity;
                    CCollection *it = new CCollection(-2);
                    it->serialize(ar);
                    loadedCollectionList.push_back(it);
                    noHit = false;
                }
                if (theName.compare(SER_BUTTON_BLOCK_old) == 0)
                {
                    if (CSimFlavor::getBoolVal(18))
                        App::logMsg(sim_verbosity_errors, "Contains old custom UIs...");
                    if (!App::userSettings->disableOpenGlBasedCustomUi)
                    {
                        ar >> byteQuantity;
                        CButtonBlock *it = new CButtonBlock(1, 1, 10, 10, 0);
                        it->serialize(ar);
                        loadedButtonBlockList.push_back(it);
                        noHit = false;
                    }
                }
                if (theName.compare(SER_LUA_SCRIPT) == 0)
                {
                    ar >> byteQuantity;
                    CScriptObject *it = new CScriptObject(-1);
                    it->serialize(ar);
                    if ((it->getScriptType() == sim_scripttype_jointctrlcallback_old) ||
                        (it->getScriptType() == sim_scripttype_generalcallback_old) ||
                        (it->getScriptType() == sim_scripttype_contactcallback_old))
                    { // joint callback, contact callback and general callback scripts are not supported anymore since
                      // V3.6.1.rev2
                        std::string ml(it->getScriptText());
                        if (it->getScriptType() == sim_scripttype_jointctrlcallback_old)
                            ml = "the file contains a joint control callback script, which is a script type that is "
                                 "not supported anymore (since CoppeliaSim V3.6.1 rev2).\nUse a joint callback "
                                 "function instead. Following the script content:\n" +
                                 ml;
                        if (it->getScriptType() == sim_scripttype_generalcallback_old)
                            ml = "the file contains a general callback script, which is a script type that is not "
                                 "supported anymore (since CoppeliaSim V3.6.1 rev2):\n" +
                                 ml;
                        if (it->getScriptType() == sim_scripttype_contactcallback_old)
                            ml = "the file contains a contact callback script, which is a script type that is not "
                                 "supported anymore (since CoppeliaSim V3.6.1 rev2).\nUse a contact callback functions "
                                 "instead. Following the script content:\n" +
                                 ml;
                        App::logMsg(sim_verbosity_errors, ml.c_str());
                        CScriptObject::destroy(it, false);
                    }
                    else
                        loadedLuaScriptList.push_back(it);
                    noHit = false;
                }
                if (theName.compare(SER_SCENE_CUSTOM_DATA) == 0)
                {
                    ar >> byteQuantity;
                    customSceneData.serializeData(ar, nullptr);
                    noHit = false;
                }
                if (theName.compare(SER_SCENE_CUSTOM_DATA_OLD) == 0)
                { // for backward compatibility
                    ar >> byteQuantity;
                    customSceneData_old->serializeData(ar, nullptr, -1);
                    noHit = false;
                    if (customSceneData.getDataCount() == 0)
                        customSceneData_old->initNewFormat(customSceneData, false);
                }
                if (theName.compare(SER_COLLISION) == 0)
                {
                    if (CSimFlavor::getBoolVal(18))
                        App::logMsg(sim_verbosity_errors, "Contains collision objects...");
                    ar >> byteQuantity;
                    CCollisionObject_old *it = new CCollisionObject_old();
                    it->serialize(ar);
                    loadedCollisionList.push_back(it);
                    noHit = false;
                }
                if (theName.compare(SER_DISTANCE) == 0)
                {
                    if (CSimFlavor::getBoolVal(18))
                        App::logMsg(sim_verbosity_errors, "Contains distance objects...");
                    ar >> byteQuantity;
                    CDistanceObject_old *it = new CDistanceObject_old();
                    it->serialize(ar);
                    loadedDistanceList.push_back(it);
                    noHit = false;
                }
                if (theName.compare(SER_IK) == 0)
                {
                    if (CSimFlavor::getBoolVal(18))
                        App::logMsg(sim_verbosity_errors, "Contains IK objects...");
                    ar >> byteQuantity;
                    CIkGroup_old *it = new CIkGroup_old();
                    it->serialize(ar);
                    loadedIkGroupList.push_back(it);
                    noHit = false;
                }
                if (theName == SER_PATH_PLANNING)
                {
                    if (CSimFlavor::getBoolVal(18))
                        App::logMsg(sim_verbosity_errors, "Contains path planning objects...");
                    ar >> byteQuantity;
                    CPathPlanningTask *it = new CPathPlanningTask();
                    it->serialize(ar);
                    pathPlanningTaskList.push_back(it);
                    noHit = false;
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
            hasThumbnail = true;
        }
        if (ar.xmlPushChildNode(SERX_SCENEOBJECT, false))
        {
            while (true)
            {
                bool dummy2;
                CSceneObject *it = sceneObjects->readSceneObject(ar, "", dummy2);
                if (it != nullptr)
                    loadedObjectList.push_back(it);
                if (!ar.xmlPushSiblingNode(SERX_SCENEOBJECT, false))
                    break;
            }
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_TEXTURE, false))
        {
            while (true)
            {
                CTextureObject *theTextureData = textureContainer->loadTextureObject(ar, dummy1, dummy2);
                if (theTextureData != nullptr)
                    loadedTextureList.push_back(theTextureData);
                if (!ar.xmlPushSiblingNode(SERX_TEXTURE, false))
                    break;
            }
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_GHOSTS, isScene))
        {
            ghostObjectCont_old->serialize(ar);
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_SETTINGS, isScene))
        {
            mainSettings_old->serialize(ar);
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_ENVIRONMENT, isScene))
        {
            environment->serialize(ar);
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_DYNAMICS, isScene))
        {
            dynamicsContainer->serialize(ar);
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_SIMULATION, isScene))
        {
            simulation->serialize(ar);
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_SCENE_CUSTOM_DATA, false))
        {
            customSceneData.serializeData(ar, nullptr);
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_SCENE_CUSTOM_DATA_OLD, false))
        { // for backward compatibility
            customSceneData_old->serializeData(ar, nullptr, -1);
            ar.xmlPopNode();
            if (customSceneData.getDataCount() == 0)
                customSceneData_old->initNewFormat(customSceneData, false);
        }
        if (ar.xmlPushChildNode(SERX_VIEWS, isScene))
        {
            pageContainer->serialize(ar);
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_COLLISION, false))
        {
            while (true)
            {
                CCollisionObject_old *it = new CCollisionObject_old();
                it->serialize(ar);
                loadedCollisionList.push_back(it);
                if (!ar.xmlPushSiblingNode(SERX_COLLISION, false))
                    break;
            }
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_DISTANCE, false))
        {
            while (true)
            {
                CDistanceObject_old *it = new CDistanceObject_old();
                it->serialize(ar);
                loadedDistanceList.push_back(it);
                if (!ar.xmlPushSiblingNode(SERX_DISTANCE, false))
                    break;
            }
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_COLLECTION, false))
        { // for backward compatibility 18.11.2020
            while (true)
            {
                CCollection *it = new CCollection(-2);
                it->serialize(ar);
                loadedCollectionList.push_back(it);
                if (!ar.xmlPushSiblingNode(SERX_COLLECTION, false))
                    break;
            }
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_IK, false))
        {
            while (true)
            {
                CIkGroup_old *it = new CIkGroup_old();
                it->serialize(ar);
                loadedIkGroupList.push_back(it);
                if (!ar.xmlPushSiblingNode(SERX_IK, false))
                    break;
            }
            ar.xmlPopNode();
        }
        if (ar.xmlPushChildNode(SERX_LUA_SCRIPT, false))
        {
            while (true)
            {
                CScriptObject *it = new CScriptObject(-1);
                it->serialize(ar);
                loadedLuaScriptList.push_back(it);
                if (!ar.xmlPushSiblingNode(SERX_LUA_SCRIPT, false))
                    break;
            }
            ar.xmlPopNode();
        }
    }

    CMesh::clearTempVerticesIndicesNormalsAndEdges();

    int fileSimVersion = ar.getCoppeliaSimVersionThatWroteThisFile();

    // All object have been loaded and are in:
    // loadedObjectList
    // loadedCollectionList
    // ...
    addGeneralObjectsToWorldAndPerformMappings(&loadedObjectList, &loadedCollectionList, &loadedCollisionList,
                                               &loadedDistanceList, &loadedIkGroupList, &pathPlanningTaskList,
                                               &loadedButtonBlockList, &loadedLuaScriptList, loadedTextureList,
                                               loadedDynMaterialList, !isScene, fileSimVersion, forceModelAsCopy);

    CMesh::clearTempVerticesIndicesNormalsAndEdges();

    appendLoadOperationIssue(-1, nullptr, -1); // clear

    if (!isScene)
    {
        CInterfaceStack *stack = App::worldContainer->interfaceStackContainer->createStack();
        stack->pushTableOntoStack();

        std::vector<int> hand;
        for (size_t i = 0; i < loadedObjectList.size(); i++)
            hand.push_back(loadedObjectList[i]->getObjectHandle());
        stack->pushTextOntoStack("objects");
        stack->pushInt32ArrayOntoStack(hand.data(), hand.size());
        stack->insertDataIntoStackTable();

        // Following for backward compatibility:
        stack->pushTextOntoStack("objectHandles");
        stack->pushTableOntoStack();
        for (size_t i = 0; i < loadedObjectList.size(); i++)
        {
            stack->pushInt32OntoStack(int(i + 1)); // key or index
            stack->pushInt32OntoStack(loadedObjectList[i]->getObjectHandle());
            stack->insertDataIntoStackTable();
        }
        stack->insertDataIntoStackTable();
        // --------------------------------------

        App::worldContainer->callScripts(sim_syscb_aftercreate, stack, nullptr);
        App::worldContainer->interfaceStackContainer->destroyStack(stack);
    }

    // Following for backward compatibility for vision sensor filters:
    for (size_t i = 0; i < sceneObjects->getObjectCount(sim_sceneobject_visionsensor); i++)
    {
        CVisionSensor *it = sceneObjects->getVisionSensorFromIndex(i);
        CComposedFilter *cf = it->getComposedFilter();
        std::string txt(cf->scriptEquivalent);
        if (txt.size() > 0)
        {
            cf->scriptEquivalent.clear();
            CScriptObject *script = sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customization, it->getObjectHandle());
            if (script == nullptr)
            {
                txt = std::string("function sysCall_init()\nend\n\n") + txt;
                script = new CScriptObject(sim_scripttype_customization);
                script->setLang("lua");
                sceneObjects->embeddedScriptContainer->insertScript(script);
                script->setObjectHandleThatScriptIsAttachedTo(it->getObjectHandle());
            }
            std::string t(script->getScriptText());
            t = txt + t;
            script->setScriptText(t.c_str());
        }
    }

    // Following for backward compatibility (Lua script parameters are now attached to objects, and not scripts anymore):
    for (size_t i = 0; i < loadedLuaScriptList.size(); i++)
    {
        CScriptObject *script = sceneObjects->embeddedScriptContainer->getScriptObjectFromHandle(loadedLuaScriptList[i]->getScriptHandle());
        if (script != nullptr)
        {
            CUserParameters *params = script->getScriptParametersObject_backCompatibility();
            int obj = script->getObjectHandleThatScriptIsAttachedTo(sim_scripttype_simulation);
            CSceneObject *theObj = sceneObjects->getObjectFromHandle(obj);
            if ((theObj != nullptr) && (params != nullptr))
            {
                if (params->userParamEntries.size() > 0)
                {
                    theObj->setUserScriptParameterObject(params->copyYourself());
                    params->userParamEntries.clear();
                }
            }
        }
    }

    // Following for backward compatibility for script exec priorities:
    if (ar.getCoppeliaSimVersionThatWroteThisFile() < 40400)
    {
        for (size_t i = 0; i < loadedObjectList.size(); i++)
        {
            int p = sceneObjects->embeddedScriptContainer->getEquivalentScriptExecPriority_old(loadedObjectList[i]->getObjectHandle());
            if (p >= sim_scriptexecorder_first)
            {
                CSceneObject *it = sceneObjects->getObjectFromHandle(loadedObjectList[i]->getObjectHandle());
                it->setScriptExecPriority_raw(p);
            }
        }
    }

    // Convert old embedded scripts to script-type objects (or vice-versa):
    if (App::userSettings->scriptConversion == 1)
    { // convert from old to new (except for very old threaded scripts (blue icon)):
        for (size_t i = 0; i < loadedObjectList.size(); i++)
        {
            CScriptObject* scriptObject = sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customization, loadedObjectList[i]->getObjectHandle());
            if ( (scriptObject != nullptr) && (!scriptObject->getThreadedExecution_oldThreads()) )
            {
                sceneObjects->embeddedScriptContainer->extractScript(scriptObject->getScriptHandle());
                CScript* script = new CScript(scriptObject);
                scriptObject->setParentIsProxy(true);
                script->setVisibilityLayer(0);
                sceneObjects->addObjectToScene(script, false, false);
                sceneObjects->setObjectParent(script, loadedObjectList[i], false);
                if (loadedObjectList[i]->getChildCount() == 1)
                {
                    loadedObjectList[i]->setModelBase(true);
                    loadedObjectList[i]->setObjectProperty(loadedObjectList[i]->getObjectProperty() | sim_objectproperty_collapsed);
                }
                sceneObjects->setObjectSequence(script, 0);
                sceneObjects->setObjectAlias(script, "autoConvertedScript", false);
                std::string nn("autoConvertedSimulationScript_");
                nn += loadedObjectList[i]->getObjectName_old();
                sceneObjects->setObjectName_old(script, nn.c_str(), false);
            }
            scriptObject = sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_simulation, loadedObjectList[i]->getObjectHandle());
            if ( (scriptObject != nullptr) && (!scriptObject->getThreadedExecution_oldThreads()) )
            {
                sceneObjects->embeddedScriptContainer->extractScript(scriptObject->getScriptHandle());
                CScript* script = new CScript(scriptObject);
                scriptObject->setParentIsProxy(true);
                script->setVisibilityLayer(0);
                sceneObjects->addObjectToScene(script, false, false);
                sceneObjects->setObjectParent(script, loadedObjectList[i], false);
                if (loadedObjectList[i]->getChildCount() == 1)
                {
                    loadedObjectList[i]->setModelBase(true);
                    loadedObjectList[i]->setObjectProperty(loadedObjectList[i]->getObjectProperty() | sim_objectproperty_collapsed);
                }
                sceneObjects->setObjectSequence(script, 0);
                sceneObjects->setObjectAlias(script, "autoConvertedScript", false);
                std::string nn("autoConvertedCustomizationScript_");
                nn += loadedObjectList[i]->getObjectName_old();
                sceneObjects->setObjectName_old(script, nn.c_str(), false);

            }
        }
    }

    if (App::userSettings->scriptConversion == -1)
    { // convert from new to old:
        std::vector<int> objectsToRemove;
        for (size_t i = 0; i < loadedObjectList.size(); i++)
        {
            CSceneObject* it = loadedObjectList[i];
            if (it->getObjectType() != sim_sceneobject_script)
            {
                int itemDone = 0; // bit 0 simulation script, bit 1 custom. script
                for (size_t j = 0; j < it->getChildCount(); j++)
                {
                    CSceneObject* c = it->getChildFromIndex(j);
                    if (c->getObjectType() == sim_sceneobject_script)
                    {
                        CScript* script = (CScript*)c;
                        int id = itemDone;
                        if ( ((itemDone & 1) == 0) && (script->scriptObject->getScriptType() == sim_scripttype_simulation) )
                            itemDone |= 1;
                        if ( ((itemDone & 2) == 0) && (script->scriptObject->getScriptType() == sim_scripttype_customization) )
                            itemDone |= 2;
                        if (itemDone != id)
                        {
                            script->scriptObject->setHandle();
                            sceneObjects->embeddedScriptContainer->insertScript(script->scriptObject);
                            script->scriptObject->setIsSceneObjectScript(false);
                            script->scriptObject->setObjectHandleThatScriptIsAttachedTo(it->getObjectHandle());
                            script->scriptObject = nullptr;
                        }
                        objectsToRemove.push_back(script->getObjectHandle());
                    }
                }
            }
        }
        sceneObjects->eraseObjects(&objectsToRemove, false);
    }

    // Enable scripts (we previously didn't want to have them react to object add event, etc., during the load operation)
    sceneObjects->setScriptsTemporarilySuspended(false);

    return (true);
}

bool CWorld::_loadSimpleXmlSceneOrModel(CSer &ar)
{
    bool retVal = true;
    bool isScene = (ar.getFileType() == CSer::filetype_csim_xml_simplescene_file);
    removeWorld_oldIk();
    if (isScene && ar.xmlPushChildNode(SERX_ENVIRONMENT, false))
    {
        environment->serialize(ar);
        ar.xmlPopNode();
    }

    if (isScene && ar.xmlPushChildNode(SERX_SETTINGS, false))
    {
        mainSettings_old->serialize(ar);
        ar.xmlPopNode();
    }

    if (isScene && ar.xmlPushChildNode(SERX_DYNAMICS, false))
    {
        dynamicsContainer->serialize(ar);
        ar.xmlPopNode();
    }

    if (isScene && ar.xmlPushChildNode(SERX_SIMULATION, false))
    {
        simulation->serialize(ar);
        ar.xmlPopNode();
    }

    std::vector<CCollection *> allLoadedCollections;
    std::map<std::string, CCollection *> _collectionLoadNamesMap;
    if (ar.xmlPushChildNode(SERX_COLLECTION, false))
    { // for backward compatibility 18.11.2020
        while (true)
        {
            CCollection *it = new CCollection(-2);
            it->serialize(ar);
            allLoadedCollections.push_back(it);
            _collectionLoadNamesMap[it->getCollectionLoadName()] = it;
            if (!ar.xmlPushSiblingNode(SERX_COLLECTION, false))
                break;
        }
        ar.xmlPopNode();
    }

    std::vector<CIkGroup_old *> allLoadedIkGroups;
    if (ar.xmlPushChildNode(SERX_IK, false))
    {
        while (true)
        {
            CIkGroup_old *it = new CIkGroup_old();
            it->serialize(ar);
            allLoadedIkGroups.push_back(it);
            if (!ar.xmlPushSiblingNode(SERX_IK, false))
                break;
        }
        ar.xmlPopNode();
    }

    if (isScene && (sceneObjects->embeddedScriptContainer->getMainScript() == nullptr))
        sceneObjects->embeddedScriptContainer->insertDefaultScript(sim_scripttype_main, false, true);

    CCamera *mainCam = nullptr;

    C7Vector ident;
    ident.setIdentity();
    std::vector<SSimpleXmlSceneObject> simpleXmlObjects;
    sceneObjects->readAndAddToSceneSimpleXmlSceneObjects(ar, nullptr, ident, simpleXmlObjects);

    bool hasAScriptAttached = false;
    std::vector<CSceneObject *> allLoadedObjects;
    for (size_t i = 0; i < simpleXmlObjects.size(); i++)
    {
        CSceneObject *it = simpleXmlObjects[i].object;
        CSceneObject *pit = simpleXmlObjects[i].parentObject;
        CScriptObject *childScript = simpleXmlObjects[i].childScript;
        CScriptObject *customizationScript = simpleXmlObjects[i].customizationScript;
        allLoadedObjects.push_back(it);
        if (it->getObjectType() == sim_sceneobject_camera)
        {
            if ((mainCam == nullptr) || ((CCamera *)it)->getIsMainCamera())
                mainCam = (CCamera *)it;
        }
        sceneObjects->setObjectParent(it, pit, false);
        if (childScript != nullptr)
        { // old scripts
            hasAScriptAttached = true;
            sceneObjects->embeddedScriptContainer->insertScript(childScript);
            childScript->setObjectHandleThatScriptIsAttachedTo(it->getObjectHandle());
        }
        if (customizationScript != nullptr)
        { // old scripts
            hasAScriptAttached = true;
            sceneObjects->embeddedScriptContainer->insertScript(customizationScript);
            customizationScript->setObjectHandleThatScriptIsAttachedTo(it->getObjectHandle());
        }
    }
    if ((mainCam != nullptr) && isScene)
    {
        pageContainer->setUpDefaultPages(true);
#ifdef SIM_WITH_GUI
        CSPage *page = pageContainer->getPage(pageContainer->getActivePageIndex());
        CSView *view = page->getView(0);
        if (view != nullptr)
            view->setLinkedObjectID(mainCam->getObjectHandle(), false);
#endif
    }

    // Now adjust the names:
    int suffixOffset = _getSuffixOffsetForGeneralObjectToAdd(true, &allLoadedObjects, &allLoadedCollections, nullptr,
                                                             nullptr, &allLoadedIkGroups, nullptr, nullptr, nullptr);
    // We add objects to the scene as copies only if we also add at least one associated script and we don't have a
    // scene. Otherwise objects are added and no '#' (or no modified suffix) will appear in their names. Following line
    // summarizes this:
    bool objectIsACopy =
        (hasAScriptAttached &&
         (ar.getFileType() == CSer::filetype_csim_xml_simplemodel_file)); // scenes are not treated like copies!
    std::map<std::string, CSceneObject *> _objectAliasesMap;
    std::map<std::string, CSceneObject *> _objectTempNamesMap;
    for (size_t i = 0; i < allLoadedObjects.size(); i++)
    {
        CSceneObject *it = allLoadedObjects[i];
        _objectAliasesMap[it->getObjectTempAlias()] = it;

        // Old, for backward compatibility:
        // ----------------------------
        _objectTempNamesMap[it->getObjectTempName_old()] = it;
        std::string newObjName = it->getObjectTempName_old();
        if (objectIsACopy)
            newObjName = tt::generateNewName_hash(newObjName.c_str(), suffixOffset);
        else
        {
            if (sceneObjects->getObjectFromName_old(newObjName.c_str()) != nullptr)
            {
                // Following faster with many objects:
                std::string baseName(tt::getNameWithoutSuffixNumber(newObjName.c_str(), false));
                int initialSuffix = tt::getNameSuffixNumber(newObjName.c_str(), false);
                std::vector<int> suffixes;
                std::vector<int> dummyValues;
                for (size_t i = 0; i < sceneObjects->getObjectCount(); i++)
                { //
                    CSceneObject *itt = sceneObjects->getObjectFromIndex(i);
                    std::string baseNameIt(tt::getNameWithoutSuffixNumber(itt->getObjectName_old().c_str(), false));
                    if (baseName.compare(baseNameIt) == 0)
                    {
                        suffixes.push_back(tt::getNameSuffixNumber(itt->getObjectName_old().c_str(), false));
                        dummyValues.push_back(0);
                    }
                }
                tt::orderAscending(suffixes, dummyValues);
                int lastS = -1;
                for (size_t i = 0; i < suffixes.size(); i++)
                {
                    if ((suffixes[i] > initialSuffix) && (suffixes[i] > lastS + 1))
                        break;
                    lastS = suffixes[i];
                }
                newObjName = tt::generateNewName_noHash(baseName.c_str(), lastS + 1 + 1);
            }
        }
        sceneObjects->setObjectName_old(it, newObjName.c_str(), true);

        // Now a similar procedure, but with the alt object names:
        std::string newObjAltName = it->getObjectTempAltName_old();
        if (sceneObjects->getObjectFromAltName_old(newObjAltName.c_str()) != nullptr)
        {
            // Following faster with many objects:
            std::string baseAltName(tt::getNameWithoutSuffixNumber(newObjAltName.c_str(), false));
            int initialSuffix = tt::getNameSuffixNumber(newObjAltName.c_str(), false);
            std::vector<int> suffixes;
            std::vector<int> dummyValues;
            for (size_t i = 0; i < sceneObjects->getObjectCount(); i++)
            {
                CSceneObject *itt = sceneObjects->getObjectFromIndex(i);
                std::string baseAltNameIt(tt::getNameWithoutSuffixNumber(itt->getObjectAltName_old().c_str(), false));
                if (baseAltName.compare(baseAltNameIt) == 0)
                {
                    suffixes.push_back(tt::getNameSuffixNumber(itt->getObjectAltName_old().c_str(), false));
                    dummyValues.push_back(0);
                }
            }
            tt::orderAscending(suffixes, dummyValues);
            int lastS = -1;
            for (size_t i = 0; i < suffixes.size(); i++)
            {
                if ((suffixes[i] > initialSuffix) && (suffixes[i] > lastS + 1))
                    break;
                lastS = suffixes[i];
            }
            newObjAltName = tt::generateNewName_noHash(baseAltName.c_str(), lastS + 1 + 1);
        }
        sceneObjects->setObjectAltName_old(it, newObjAltName.c_str(), true);
        // ----------------------------
    }

    // Old, for backward compatibility when persistent collections & Ik groups are present:
    // -----------------------
    for (size_t i = 0; i < allLoadedCollections.size(); i++)
    {
        CCollection *it = allLoadedCollections[i];
        for (size_t j = 0; j < it->getElementCount(); j++)
        {
            CCollectionElement *el = it->getElementFromIndex(j);
            std::map<std::string, CSceneObject *>::const_iterator elIt =
                _objectTempNamesMap.find(el->getMainObjectTempName());
            if ((el->getMainObjectTempName().size() > 0) && (elIt != _objectTempNamesMap.end()))
                el->setMainObject(elIt->second->getObjectHandle());
            else
            {
                if (el->getElementType() != sim_collectionelement_all)
                {
                    it->removeCollectionElementFromHandle(el->getElementHandle());
                    j--; // reprocess this position
                }
            }
        }
        if (it->getElementCount() > 0)
        {
            collections->addCollectionWithSuffixOffset(it, objectIsACopy, suffixOffset);
            it->actualizeCollection();
        }
        else
        {
            delete it;
            allLoadedCollections.erase(allLoadedCollections.begin() + i);
            i--; // reprocess this position
        }
    }
    for (size_t i = 0; i < allLoadedIkGroups.size(); i++)
    {
        CIkGroup_old *it = allLoadedIkGroups[i];
        for (size_t j = 0; j < it->getIkElementCount(); j++)
        {
            CIkElement_old *el = it->getIkElementFromIndex(j);
            std::map<std::string, CSceneObject *>::const_iterator elIt = _objectTempNamesMap.find(el->getTipLoadName());
            if ((el->getTipLoadName().size() > 0) && (elIt != _objectTempNamesMap.end()))
            {
                el->setTipHandle(elIt->second->getObjectHandle());
                elIt = _objectTempNamesMap.find(el->getBaseLoadName());
                if ((el->getBaseLoadName().size() > 0) && (elIt != _objectTempNamesMap.end()))
                    el->setBase(elIt->second->getObjectHandle());
                elIt = _objectTempNamesMap.find(el->getAltBaseLoadName());
                if ((el->getAltBaseLoadName().size() > 0) && (elIt != _objectTempNamesMap.end()))
                    el->setAlternativeBaseForConstraints(elIt->second->getObjectHandle());
            }
            else
            {
                it->removeIkElement(el->getObjectHandle());
                j = -1; // start the loop over again
            }
        }
        if (it->getIkElementCount() > 0)
            ikGroups_old->addIkGroupWithSuffixOffset(it, objectIsACopy, suffixOffset);
        else
        {
            delete it;
            allLoadedIkGroups.erase(allLoadedIkGroups.begin() + i);
            i--; // reprocess this position
        }
    }
    // -----------------------

    for (size_t i = 0; i < allLoadedObjects.size(); i++)
    {
        CSceneObject *obj = allLoadedObjects[i];
        // Handle dummy-dummy linking:
        if (obj->getObjectType() == sim_sceneobject_dummy)
        {
            CDummy *dummy = (CDummy *)obj;
            std::map<std::string, CSceneObject *>::const_iterator it =
                _objectAliasesMap.find(dummy->getLinkedDummyLoadAlias());
            if ((dummy->getLinkedDummyLoadAlias().size() > 0) && (it != _objectAliasesMap.end()))
                dummy->setLinkedDummyHandle(it->second->getObjectHandle(), true);
            else
            { // for backward compatibility
                if (dummy->getLinkedDummyLoadName_old().size() > 0)
                {
                    it = _objectTempNamesMap.find(dummy->getLinkedDummyLoadName_old());
                    if (it != _objectTempNamesMap.end())
                        dummy->setLinkedDummyHandle(it->second->getObjectHandle(), true);
                }
            }
        }
        // Handle joint-joint linking:
        if (obj->getObjectType() == sim_sceneobject_joint)
        {
            CJoint *joint = (CJoint *)obj;
            std::map<std::string, CSceneObject *>::const_iterator it =
                _objectAliasesMap.find(joint->getDependencyJointLoadAlias());
            if ((joint->getDependencyJointLoadAlias().size() > 0) && (it != _objectAliasesMap.end()))
                joint->setDependencyMasterJointHandle(it->second->getObjectHandle());
            else
            { // for backward compatibility
                if (joint->getDependencyJointLoadName_old().size() > 0)
                {
                    it = _objectTempNamesMap.find(joint->getDependencyJointLoadName_old());
                    if (it != _objectTempNamesMap.end())
                        joint->setDependencyMasterJointHandle(it->second->getObjectHandle());
                }
            }
        }
        // Handle camera tracking:
        if (obj->getObjectType() == sim_sceneobject_camera)
        {
            CCamera *camera = (CCamera *)obj;
            std::map<std::string, CSceneObject *>::const_iterator it =
                _objectAliasesMap.find(camera->getTrackedObjectLoadAlias());
            if ((camera->getTrackedObjectLoadAlias().size() > 0) && (it != _objectAliasesMap.end()))
                camera->setTrackedObjectHandle(it->second->getObjectHandle());
            else
            { // for backward compatibility
                if (camera->getTrackedObjectLoadName_old().size() > 0)
                {
                    it = _objectTempNamesMap.find(camera->getTrackedObjectLoadName_old());
                    if (it != _objectTempNamesMap.end())
                        camera->setTrackedObjectHandle(it->second->getObjectHandle());
                }
            }
        }
        // Handle proximitySensor sensable entity linking:
        if (obj->getObjectType() == sim_sceneobject_proximitysensor)
        {
            CProxSensor *proxSensor = (CProxSensor *)obj;
            std::map<std::string, CSceneObject *>::const_iterator it =
                _objectAliasesMap.find(proxSensor->getSensableObjectLoadAlias());
            if ((proxSensor->getSensableObjectLoadAlias().size() > 0) && (it != _objectAliasesMap.end()))
                proxSensor->setSensableObject(it->second->getObjectHandle());
            else
            { // for backward compatibility
                if (proxSensor->getSensableObjectLoadName_old().size() > 0)
                {
                    it = _objectTempNamesMap.find(proxSensor->getSensableObjectLoadName_old());
                    if (it != _objectTempNamesMap.end())
                        proxSensor->setSensableObject(it->second->getObjectHandle());
                    else
                    {
                        std::map<std::string, CCollection *>::const_iterator itColl =
                            _collectionLoadNamesMap.find(proxSensor->getSensableObjectLoadName_old());
                        if (itColl != _collectionLoadNamesMap.end())
                            proxSensor->setSensableObject(itColl->second->getCollectionHandle());
                    }
                }
            }
        }
        // Handle visionSensor renderable entity linking:
        if (obj->getObjectType() == sim_sceneobject_visionsensor)
        {
            CVisionSensor *visionSensor = (CVisionSensor *)obj;
            std::map<std::string, CSceneObject *>::const_iterator it =
                _objectAliasesMap.find(visionSensor->getDetectableEntityLoadAlias());
            if ((visionSensor->getDetectableEntityLoadAlias().size() > 0) && (it != _objectAliasesMap.end()))
                visionSensor->setDetectableEntityHandle(it->second->getObjectHandle());
            else
            { // for backward compatibility
                if (visionSensor->getDetectableEntityLoadName_old().size() > 0)
                {
                    it = _objectTempNamesMap.find(visionSensor->getDetectableEntityLoadName_old());
                    if (it != _objectTempNamesMap.end())
                        visionSensor->setDetectableEntityHandle(it->second->getObjectHandle());
                    else
                    {
                        std::map<std::string, CCollection *>::const_iterator itColl =
                            _collectionLoadNamesMap.find(visionSensor->getDetectableEntityLoadName_old());
                        if (itColl != _collectionLoadNamesMap.end())
                            visionSensor->setDetectableEntityHandle(itColl->second->getCollectionHandle());
                    }
                }
            }
        }
    }

    rebuildWorld_oldIk();

    // Enable scripts (we previously didn't want to have them react to object add event, etc., during the load operation)
    sceneObjects->setScriptsTemporarilySuspended(false);

    return (retVal);
}

bool CWorld::_saveSimpleXmlScene(CSer &ar)
{
    bool retVal = true;
    bool isScene = (ar.getFileType() == CSer::filetype_csim_xml_simplescene_file);

    ar.xmlAddNode_comment(" 'environment' tag: has no effect when loading a model ", false);
    ar.xmlPushNewNode(SERX_ENVIRONMENT);
    environment->serialize(ar);
    ar.xmlPopNode();

    ar.xmlAddNode_comment(" 'settings' tag: has no effect when loading a model ", false);
    ar.xmlPushNewNode(SERX_SETTINGS);
    mainSettings_old->serialize(ar);
    ar.xmlPopNode();

    ar.xmlAddNode_comment(" 'dynamics' tag: has no effect when loading a model ", false);
    ar.xmlPushNewNode(SERX_DYNAMICS);
    dynamicsContainer->serialize(ar);
    ar.xmlPopNode();

    ar.xmlAddNode_comment(" 'simulation' tag: has no effect when loading a model ", false);
    ar.xmlPushNewNode(SERX_SIMULATION);
    simulation->serialize(ar);
    ar.xmlPopNode();

    for (size_t i = 0; i < collections->getObjectCount(); i++)
    { // Old:
        ar.xmlPushNewNode(SERX_COLLECTION);
        collections->getObjectFromIndex(i)->serialize(ar);
        ar.xmlPopNode();
    }

    for (size_t i = 0; i < ikGroups_old->getObjectCount(); i++)
    { // Old:
        ar.xmlPushNewNode(SERX_IK);
        ikGroups_old->getObjectFromIndex(i)->serialize(ar);
        ar.xmlPopNode();
    }

    for (size_t i = 0; i < sceneObjects->getOrphanCount(); i++)
        sceneObjects->writeSimpleXmlSceneObjectTree(ar, sceneObjects->getOrphanFromIndex(i));

    return (retVal);
}

void CWorld::_getMinAndMaxNameSuffixes(int &smallestSuffix, int &biggestSuffix) const
{
    smallestSuffix = SIM_MAX_INT;
    biggestSuffix = -1;
    int minS, maxS;
    buttonBlockContainer_old->getMinAndMaxNameSuffixes(minS, maxS);
    if (minS < smallestSuffix)
        smallestSuffix = minS;
    if (maxS > biggestSuffix)
        biggestSuffix = maxS;
    sceneObjects->getMinAndMaxNameSuffixes(minS, maxS);
    if (minS < smallestSuffix)
        smallestSuffix = minS;
    if (maxS > biggestSuffix)
        biggestSuffix = maxS;
    collisions_old->getMinAndMaxNameSuffixes(minS, maxS);
    if (minS < smallestSuffix)
        smallestSuffix = minS;
    if (maxS > biggestSuffix)
        biggestSuffix = maxS;
    distances_old->getMinAndMaxNameSuffixes(minS, maxS);
    if (minS < smallestSuffix)
        smallestSuffix = minS;
    if (maxS > biggestSuffix)
        biggestSuffix = maxS;
    collections->getMinAndMaxNameSuffixes(minS, maxS);
    if (minS < smallestSuffix)
        smallestSuffix = minS;
    if (maxS > biggestSuffix)
        biggestSuffix = maxS;
    ikGroups_old->getMinAndMaxNameSuffixes(minS, maxS);
    if (minS < smallestSuffix)
        smallestSuffix = minS;
    if (maxS > biggestSuffix)
        biggestSuffix = maxS;
    pathPlanning_old->getMinAndMaxNameSuffixes(minS, maxS);
    if (minS < smallestSuffix)
        smallestSuffix = minS;
    if (maxS > biggestSuffix)
        biggestSuffix = maxS;
}

int CWorld::_getSuffixOffsetForGeneralObjectToAdd(
    bool tempNames, std::vector<CSceneObject *> *loadedObjectList, std::vector<CCollection *> *loadedCollectionList,
    std::vector<CCollisionObject_old *> *loadedCollisionList, std::vector<CDistanceObject_old *> *loadedDistanceList,
    std::vector<CIkGroup_old *> *loadedIkGroupList, std::vector<CPathPlanningTask *> *loadedPathPlanningTaskList,
    std::vector<CButtonBlock *> *loadedButtonBlockList, std::vector<CScriptObject *> *loadedLuaScriptList) const
{
    // 1. We find out about the smallest suffix to paste:
    int smallestSuffix = SIM_MAX_INT;
    // sceneObjects:
    if (loadedObjectList != nullptr)
    {
        for (size_t i = 0; i < loadedObjectList->size(); i++)
        {
            std::string str(loadedObjectList->at(i)->getObjectName_old());
            if (tempNames)
                str = loadedObjectList->at(i)->getObjectTempName_old();
            int s = tt::getNameSuffixNumber(str.c_str(), true);
            if (i == 0)
                smallestSuffix = s;
            else
            {
                if (s < smallestSuffix)
                    smallestSuffix = s;
            }
        }
    }
    // Collections:
    if (loadedCollectionList != nullptr)
    {
        for (size_t i = 0; i < loadedCollectionList->size(); i++)
        {
            int s = tt::getNameSuffixNumber(loadedCollectionList->at(i)->getCollectionName().c_str(), true);
            if (s < smallestSuffix)
                smallestSuffix = s;
        }
    }
    // Collisions:
    if (loadedCollisionList != nullptr)
    {
        for (size_t i = 0; i < loadedCollisionList->size(); i++)
        {
            int s = tt::getNameSuffixNumber(loadedCollisionList->at(i)->getObjectName().c_str(), true);
            if (s < smallestSuffix)
                smallestSuffix = s;
        }
    }
    // Distances:
    if (loadedDistanceList != nullptr)
    {
        for (size_t i = 0; i < loadedDistanceList->size(); i++)
        {
            int s = tt::getNameSuffixNumber(loadedDistanceList->at(i)->getObjectName().c_str(), true);
            if (s < smallestSuffix)
                smallestSuffix = s;
        }
    }
    // IK Groups:
    if (loadedIkGroupList != nullptr)
    {
        for (size_t i = 0; i < loadedIkGroupList->size(); i++)
        {
            int s = tt::getNameSuffixNumber(loadedIkGroupList->at(i)->getObjectName().c_str(), true);
            if (s < smallestSuffix)
                smallestSuffix = s;
        }
    }
    // Path planning tasks:
    if (loadedPathPlanningTaskList != nullptr)
    {
        for (size_t i = 0; i < loadedPathPlanningTaskList->size(); i++)
        {
            int s = tt::getNameSuffixNumber(loadedPathPlanningTaskList->at(i)->getObjectName().c_str(), true);
            if (s < smallestSuffix)
                smallestSuffix = s;
        }
    }
    // 2D Elements:
    if (loadedButtonBlockList != nullptr)
    {
        for (size_t i = 0; i < loadedButtonBlockList->size(); i++)
        {
            int s = tt::getNameSuffixNumber(loadedButtonBlockList->at(i)->getBlockName().c_str(), true);
            if (s < smallestSuffix)
                smallestSuffix = s;
        }
    }

    // 2. Now we find out about the highest suffix among existing objects (already in the scene):
    int biggestSuffix, smallestSuffixDummy;

    _getMinAndMaxNameSuffixes(smallestSuffixDummy, biggestSuffix);
    return (biggestSuffix - smallestSuffix + 1);
}

bool CWorld::_canSuffix1BeSetToSuffix2(int suffix1, int suffix2) const
{
    if (!sceneObjects->canSuffix1BeSetToSuffix2(suffix1, suffix2))
        return (false);
    if (!buttonBlockContainer_old->canSuffix1BeSetToSuffix2(suffix1, suffix2))
        return (false);
    if (!collisions_old->canSuffix1BeSetToSuffix2(suffix1, suffix2))
        return (false);
    if (!distances_old->canSuffix1BeSetToSuffix2(suffix1, suffix2))
        return (false);
    if (!collections->canSuffix1BeSetToSuffix2(suffix1, suffix2))
        return (false);
    if (!ikGroups_old->canSuffix1BeSetToSuffix2(suffix1, suffix2))
        return (false);
    if (!pathPlanning_old->canSuffix1BeSetToSuffix2(suffix1, suffix2))
        return (false);
    return (true);
}

void CWorld::_setSuffix1ToSuffix2(int suffix1, int suffix2)
{
    sceneObjects->setSuffix1ToSuffix2(suffix1, suffix2);
    buttonBlockContainer_old->setSuffix1ToSuffix2(suffix1, suffix2);
    collisions_old->setSuffix1ToSuffix2(suffix1, suffix2);
    distances_old->setSuffix1ToSuffix2(suffix1, suffix2);
    collections->setSuffix1ToSuffix2(suffix1, suffix2);
    ikGroups_old->setSuffix1ToSuffix2(suffix1, suffix2);
    pathPlanning_old->setSuffix1ToSuffix2(suffix1, suffix2);
}

void CWorld::appendLoadOperationIssue(int verbosity, const char *text, int objectId)
{
    if (text == nullptr)
        _loadOperationIssues.clear();
    else
    {
        SLoadOperationIssue issue;
        issue.verbosity = verbosity;
        issue.message = text;
        issue.objectHandle = objectId;
        _loadOperationIssues.push_back(issue);
    }
}

int CWorld::getLoadingMapping(const std::map<int, int> *map, int oldVal)
{
    int retVal = -1;
    auto it = map->find(oldVal);
    if (it != map->end())
        retVal = it->second;
    return (retVal);
}

void CWorld::setWorldHandle(int handle)
{
    _worldHandle = handle;
}

int CWorld::getWorldHandle() const
{
    return (_worldHandle);
}

int CWorld::setBoolProperty(long long int target, const char* ppName, bool pState)
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        if (dynamicsContainer != nullptr)
            retVal = dynamicsContainer->setBoolProperty(pName, pState);
        if ( (retVal == -1) && (simulation != nullptr) )
            retVal = simulation->setBoolProperty(pName, pState);
        if ( (retVal == -1) && (environment != nullptr) )
            retVal = environment->setBoolProperty(pName, pState);
        if ( (retVal == -1) && (sceneObjects != nullptr) )
            retVal = sceneObjects->setBoolProperty(-1, pName, pState); // for the container itself
        if (retVal == -1)
        {
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->setBoolProperty(target, pName, pState);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
            retVal = script->setBoolProperty(pName, pState);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::getBoolProperty(long long int target, const char* ppName, bool& pState) const
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        if (dynamicsContainer != nullptr)
            retVal = dynamicsContainer->getBoolProperty(pName, pState);
        if ( (retVal == -1) && (simulation != nullptr) )
            retVal = simulation->getBoolProperty(pName, pState);
        if ( (retVal == -1) && (environment != nullptr) )
            retVal = environment->getBoolProperty(pName, pState);
        if ( (retVal == -1) && (sceneObjects != nullptr) )
            retVal = sceneObjects->getBoolProperty(-1, pName, pState); // for the container itself
        if (retVal == -1)
        {
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->getBoolProperty(target, pName, pState);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
            retVal = script->getBoolProperty(pName, pState);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::setIntProperty(long long int target, const char* ppName, int pState)
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        if (dynamicsContainer != nullptr)
            retVal = dynamicsContainer->setIntProperty(pName, pState);
        if ( (retVal == -1) && (simulation != nullptr) )
            retVal = simulation->setIntProperty(pName, pState);
        if ( (retVal == -1) && (environment != nullptr) )
            retVal = environment->setIntProperty(pName, pState);
        if ( (retVal == -1) && (sceneObjects != nullptr) )
            retVal = sceneObjects->setIntProperty(-1, pName, pState); // for the container itself
        if (retVal == -1)
        {
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->setIntProperty(target, pName, pState);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
            retVal = script->setIntProperty(pName, pState);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::getIntProperty(long long int target, const char* ppName, int& pState) const
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        if (dynamicsContainer != nullptr)
            retVal = dynamicsContainer->getIntProperty(pName, pState);
        if ( (retVal == -1) && (simulation != nullptr) )
            retVal = simulation->getIntProperty(pName, pState);
        if ( (retVal == -1) && (environment != nullptr) )
            retVal = environment->getIntProperty(pName, pState);
        if ( (retVal == -1) && (sceneObjects != nullptr) )
            retVal = sceneObjects->getIntProperty(-1, pName, pState); // for the container itself
        if (retVal == -1)
        {
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->getIntProperty(target, pName, pState);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
            retVal = script->getIntProperty(pName, pState);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::setLongProperty(long long int target, const char* ppName, long long int pState)
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        /*
        if (dynamicsContainer != nullptr)
            retVal = dynamicsContainer->setLongProperty(pName, pState);
        if ( (retVal == -1) && (simulation != nullptr) )
            retVal = simulation->setLongProperty(pName, pState);
        if ( (retVal == -1) && (environment != nullptr) )
            retVal = environment->setLongProperty(pName, pState);
        if ( (retVal == -1) && (sceneObjects != nullptr) )
            retVal = sceneObjects->setLongProperty(-1, pName, pState); // for the container itself
        if (retVal == -1)
        {
        }
        */
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->setLongProperty(target, pName, pState);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
            retVal = script->setLongProperty(pName, pState);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::getLongProperty(long long int target, const char* ppName, long long int& pState) const
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        /*
        if (dynamicsContainer != nullptr)
            retVal = dynamicsContainer->setLongProperty(pName, pState);
        if ( (retVal == -1) && (simulation != nullptr) )
            retVal = simulation->setLongProperty(pName, pState);
        if ( (retVal == -1) && (environment != nullptr) )
            retVal = environment->setLongProperty(pName, pState);
        if ( (retVal == -1) && (sceneObjects != nullptr) )
            retVal = sceneObjects->setLongProperty(-1, pName, pState); // for the container itself
        if (retVal == -1)
        {
        }
        */
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->getLongProperty(target, pName, pState);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
            retVal = script->getLongProperty(pName, pState);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::setFloatProperty(long long int target, const char* ppName, double pState)
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        if (dynamicsContainer != nullptr)
            retVal = dynamicsContainer->setFloatProperty(pName, pState);
        if ( (retVal == -1) && (simulation != nullptr) )
            retVal = simulation->setFloatProperty(pName, pState);
        if ( (retVal == -1) && (sceneObjects != nullptr) )
            retVal = sceneObjects->setFloatProperty(-1, pName, pState); // for the container itself
        if (retVal == -1)
        {
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->setFloatProperty(target, pName, pState);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
         //   retVal = script->setFloatProperty(pName, pState);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::getFloatProperty(long long int target, const char* ppName, double& pState) const
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        if (dynamicsContainer != nullptr)
            retVal = dynamicsContainer->getFloatProperty(pName, pState);
        if ( (retVal == -1) && (simulation != nullptr) )
            retVal = simulation->getFloatProperty(pName, pState);
        if ( (retVal == -1) && (sceneObjects != nullptr) )
            retVal = sceneObjects->getFloatProperty(-1, pName, pState); // for the container itself
        if (retVal == -1)
        {
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->getFloatProperty(target, pName, pState);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
        //    retVal = script->getFloatProperty(pName, pState);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::setStringProperty(long long int target, const char* ppName, const char* pState)
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        if (dynamicsContainer != nullptr)
            retVal = dynamicsContainer->setStringProperty(pName, pState);
        if ( (retVal == -1) && (environment != nullptr) )
            retVal = environment->setStringProperty(pName, pState);
        if ( (retVal == -1) && (sceneObjects != nullptr) )
            retVal = sceneObjects->setStringProperty(-1, pName, pState); // for the container itself
        if (retVal == -1)
        {
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->setStringProperty(target, pName, pState);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
            retVal = script->setStringProperty(pName, pState);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::getStringProperty(long long int target, const char* ppName, std::string& pState) const
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        if (dynamicsContainer != nullptr)
            retVal = dynamicsContainer->getStringProperty(pName, pState);
        if ( (retVal == -1) && (environment != nullptr) )
            retVal = environment->getStringProperty(pName, pState);
        if ( (retVal == -1) && (sceneObjects != nullptr) )
            retVal = sceneObjects->getStringProperty(-1, pName, pState); // for the container itself
        if (retVal == -1)
        {
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->getStringProperty(target, pName, pState);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
            retVal = script->getStringProperty(pName, pState);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::setBufferProperty(long long int target, const char* ppName, const char* buffer, int bufferL)
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (buffer == nullptr)
        bufferL = 0;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        std::string pN(pName);
        if (utils::replaceSubstringStart(pN, CUSTOMDATAPREFIX, ""))
        {
            if (pN.size() > 0)
            {
                bool diff = customSceneData.setData(pN.c_str(), buffer, bufferL, true);
                if (diff && App::worldContainer->getEventsEnabled())
                {
                    CCbor *ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, nullptr, false);
                    customSceneData.appendEventData(pN.c_str(), ev);
                    App::worldContainer->pushEvent();
                }
                retVal = 1;
            }
        }
        else if (utils::replaceSubstringStart(pN, SIGNALPREFIX, ""))
        {
            if (pN.size() > 0)
            {
                bool diff = customSceneData_volatile.setData(pN.c_str(), buffer, bufferL, true);
                if (diff && App::worldContainer->getEventsEnabled())
                {
                    CCbor *ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, nullptr, false);
                    customSceneData_volatile.appendEventData(pN.c_str(), ev);
                    App::worldContainer->pushEvent();
                }
                retVal = 1;
            }
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->setBufferProperty(target, pName, buffer, bufferL);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
        //    retVal = script->setBufferProperty(pName, buffer, bufferL);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::getBufferProperty(long long int target, const char* ppName, std::string& pState) const
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        std::string pN(pName);
        if (utils::replaceSubstringStart(pN, CUSTOMDATAPREFIX, ""))
        {
            if (pN.size() > 0)
            {
                if (customSceneData.hasData(pN.c_str(), false) >= 0)
                {
                    pState = customSceneData.getData(pN.c_str());
                    retVal = 1;
                }
            }
        }
        if (utils::replaceSubstringStart(pN, SIGNALPREFIX, ""))
        {
            if (pN.size() > 0)
            {
                if (customSceneData_volatile.hasData(pN.c_str(), false) >= 0)
                {
                    pState = customSceneData_volatile.getData(pN.c_str());
                    retVal = 1;
                }
            }
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->getBufferProperty(target, pName, pState);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
        //    retVal = script->getBufferProperty(pName, pState);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::setIntArray2Property(long long int target, const char* ppName, const int* pState)
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        if (dynamicsContainer != nullptr)
            retVal = dynamicsContainer->setIntArray2Property(pName, pState);
        if ( (retVal == -1) && (sceneObjects != nullptr) )
            retVal = sceneObjects->setIntArray2Property(-1, pName, pState); // for the container itself
        if (retVal == -1)
        {
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->setIntArray2Property(target, pName, pState);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
        //    retVal = script->setIntArray2Property(pName, pState);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::getIntArray2Property(long long int target, const char* ppName, int* pState) const
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        if (dynamicsContainer != nullptr)
            retVal = dynamicsContainer->getIntArray2Property(pName, pState);
        if ( (retVal == -1) && (sceneObjects != nullptr) )
            retVal = sceneObjects->getIntArray2Property(-1, pName, pState); // for the container itself
        if (retVal == -1)
        {
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->getIntArray2Property(target, pName, pState);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
        //    retVal = script->getIntArray2Property(pName, pState);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::setVector2Property(long long int target, const char* ppName, const double* pState)
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        if (dynamicsContainer != nullptr)
            retVal = dynamicsContainer->setVector2Property(pName, pState);
        if ( (retVal == -1) && (sceneObjects != nullptr) )
            retVal = sceneObjects->setVector2Property(-1, pName, pState); // for the container itself
        if (retVal == -1)
        {
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->setVector2Property(target, pName, pState);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
        //    retVal = script->setVector2Property(pName, pState);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::getVector2Property(long long int target, const char* ppName, double* pState) const
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        if (dynamicsContainer != nullptr)
            retVal = dynamicsContainer->getVector2Property(pName, pState);
        if ( (retVal == -1) && (sceneObjects != nullptr) )
            retVal = sceneObjects->getVector2Property(-1, pName, pState); // for the container itself
        if (retVal == -1)
        {
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->getVector2Property(target, pName, pState);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
        //    retVal = script->getVector2Property(pName, pState);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::setVector3Property(long long int target, const char* ppName, const C3Vector& pState)
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        if (dynamicsContainer != nullptr)
            retVal = dynamicsContainer->setVector3Property(pName, &pState);
        if ( (retVal == -1) && (sceneObjects != nullptr) )
            retVal = sceneObjects->setVector3Property(-1, pName, pState); // for the container itself
        if (retVal == -1)
        {
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->setVector3Property(target, pName, pState);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
        //    retVal = script->setVector3Property(pName, pState);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::getVector3Property(long long int target, const char* ppName, C3Vector& pState) const
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        if (dynamicsContainer != nullptr)
            retVal = dynamicsContainer->getVector3Property(pName, pState);
        if ( (retVal == -1) && (sceneObjects != nullptr) )
            retVal = sceneObjects->getVector3Property(-1, pName, pState); // for the container itself
        if (retVal == -1)
        {
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->getVector3Property(target, pName, pState);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
        //    retVal = script->getVector3Property(pName, pState);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::setQuaternionProperty(long long int target, const char* ppName, const C4Vector& pState)
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        if (sceneObjects != nullptr)
            retVal = sceneObjects->setQuaternionProperty(-1, pName, pState); // for the container itself
        if (retVal == -1)
        {
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->setQuaternionProperty(target, pName, pState);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
        //    retVal = script->setQuaternionProperty(pName, pState);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::getQuaternionProperty(long long int target, const char* ppName, C4Vector& pState) const
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        if (sceneObjects != nullptr)
            retVal = sceneObjects->getQuaternionProperty(-1, pName, pState); // for the container itself
        if (retVal == -1)
        {
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->getQuaternionProperty(target, pName, pState);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
        //    retVal = script->getQuaternionProperty(pName, pState);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::setPoseProperty(long long int target, const char* ppName, const C7Vector& pState)
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        if (sceneObjects != nullptr)
            retVal = sceneObjects->setPoseProperty(-1, pName, pState); // for the container itself
        if (retVal == -1)
        {
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->setPoseProperty(target, pName, pState);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
        //    retVal = script->setPoseProperty(pName, pState);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::getPoseProperty(long long int target, const char* ppName, C7Vector& pState) const
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        if (sceneObjects != nullptr)
            retVal = sceneObjects->getPoseProperty(-1, pName, pState); // for the container itself
        if (retVal == -1)
        {
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->getPoseProperty(target, pName, pState);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
        //    retVal = script->getPoseProperty(pName, pState);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::setColorProperty(long long int target, const char* ppName, const float* pState)
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        if (environment != nullptr)
            retVal = environment->setColorProperty(pName, pState);
        if ( (retVal == -1) && (sceneObjects != nullptr) )
            retVal = sceneObjects->setColorProperty(-1, pName, pState); // for the container itself
        if (retVal == -1)
        {
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->setColorProperty(target, pName, pState);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
        //    retVal = script->setColorProperty(pName, pState);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::getColorProperty(long long int target, const char* ppName, float* pState) const
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        if (environment != nullptr)
            retVal = environment->getColorProperty(pName, pState);
        if ( (retVal == -1) && (sceneObjects != nullptr) )
            retVal = sceneObjects->getColorProperty(-1, pName, pState); // for the container itself
        if (retVal == -1)
        {
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->getColorProperty(target, pName, pState);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
        //    retVal = script->getColorProperty(pName, pState);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::setFloatArrayProperty(long long int target, const char* ppName, const double* v, int vL)
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        if (dynamicsContainer != nullptr)
            retVal = dynamicsContainer->setFloatArrayProperty(pName, v, vL);
        if ( (retVal == -1) && (sceneObjects != nullptr) )
            retVal = sceneObjects->setFloatArrayProperty(-1, pName, v, vL); // for the container itself
        if (retVal == -1)
        {
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->setFloatArrayProperty(target, pName, v, vL);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
        //    retVal = script->setFloatArrayProperty(pName, v, vL);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::getFloatArrayProperty(long long int target, const char* ppName, std::vector<double>& pState) const
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    pState.clear();
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        if (dynamicsContainer != nullptr)
            retVal = dynamicsContainer->getFloatArrayProperty(pName, pState);
        if ( (retVal == -1) && (sceneObjects != nullptr) )
            retVal = sceneObjects->getFloatArrayProperty(-1, pName, pState); // for the container itself
        if (retVal == -1)
        {
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->getFloatArrayProperty(target, pName, pState);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
        //    retVal = script->getFloatArrayProperty(pName, pState);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::setIntArrayProperty(long long int target, const char* ppName, const int* v, int vL)
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        if (dynamicsContainer != nullptr)
            retVal = dynamicsContainer->setIntArrayProperty(pName, v, vL);
        if ( (retVal == -1) && (sceneObjects != nullptr) )
            retVal = sceneObjects->setIntArrayProperty(-1, pName, v, vL); // for the container itself
        if (retVal == -1)
        {
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->setIntArrayProperty(target, pName, v, vL);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
        //    retVal = script->setIntArrayProperty(pName, v, vL);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::getIntArrayProperty(long long int target, const char* ppName, std::vector<int>& pState) const
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    pState.clear();
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        if (dynamicsContainer != nullptr)
            retVal = dynamicsContainer->getIntArrayProperty(pName, pState);
        if ( (retVal == -1) && (sceneObjects != nullptr) )
            retVal = sceneObjects->getIntArrayProperty(-1, pName, pState); // for the container itself
        if (retVal == -1)
        {
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->getIntArrayProperty(target, pName, pState);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
        //    retVal = script->getIntArrayProperty(pName, pState);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::removeProperty(long long int target, const char* ppName)
{
    if (target == sim_handle_mainscript)
    {
        CScriptObject* it = sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        std::string pN(pName);
        if (utils::replaceSubstringStart(pN, CUSTOMDATAPREFIX, ""))
        {
            if (pN.size() > 0)
            {
                int tp = customSceneData.hasData(pN.c_str(), true);
                if (tp >= 0)
                {
                    bool diff = customSceneData.clearData((propertyStrings[tp] + pN).c_str());
                    if (diff && App::worldContainer->getEventsEnabled())
                    {
                        CCbor *ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, nullptr, false);
                        customSceneData.appendEventData(pN.c_str(), ev, true);
                        App::worldContainer->pushEvent();
                    }
                    retVal = 1;
                }
            }
        }
        else if (utils::replaceSubstringStart(pN, SIGNALPREFIX, ""))
        {
            if (pN.size() > 0)
            {
                int tp = customSceneData_volatile.hasData(pN.c_str(), true);
                if (tp >= 0)
                {
                    bool diff = customSceneData_volatile.clearData((propertyStrings[tp] + pN).c_str());
                    if (diff && App::worldContainer->getEventsEnabled())
                    {
                        CCbor *ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, nullptr, false);
                        customSceneData_volatile.appendEventData(pN.c_str(), ev, true);
                        App::worldContainer->pushEvent();
                    }
                    retVal = 1;
                }
            }
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        retVal = sceneObjects->removeProperty(target, pName);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
        //    retVal = script->removeProperty(pName);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::getPropertyName(long long int target, int& index, std::string& pName, std::string& appartenance, CWorld* targetObject)
{
    if ( (target == sim_handle_mainscript) && (targetObject != nullptr) )
    {
        CScriptObject* it = targetObject->sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        appartenance += ".scene";
        if (App::currentWorld->dynamicsContainer != nullptr)
            retVal = App::currentWorld->dynamicsContainer->getPropertyName(index, pName);
        if ( (retVal == -1) && (App::currentWorld->simulation != nullptr) )
            retVal = App::currentWorld->simulation->getPropertyName(index, pName);
        if ( (retVal == -1) && (App::currentWorld->environment != nullptr) )
            retVal = App::currentWorld->environment->getPropertyName(index, pName);
        if (retVal == -1)
        {
            CSceneObjectContainer* soc = nullptr;
            if (targetObject != nullptr)
                soc = targetObject->sceneObjects;
            retVal = CSceneObjectContainer::getPropertyName(-1, index, pName, appartenance, soc); // for the container itself
        }
        if (retVal == -1)
        {
            if (targetObject != nullptr)
            {
                if (targetObject->customSceneData.getPropertyName(index, pName))
                {
                    pName = CUSTOMDATAPREFIX + pName;
                    retVal = 1;
                }
                else if (targetObject->customSceneData_volatile.getPropertyName(index, pName))
                {
                    pName = SIGNALPREFIX + pName;
                    retVal = 1;
                }
            }
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        appartenance += ".scene";
        CSceneObjectContainer* soc = nullptr;
        if (targetObject != nullptr)
            soc = targetObject->sceneObjects;
        retVal = CSceneObjectContainer::getPropertyName(target, index, pName, appartenance, soc);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) && (targetObject != nullptr) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                appartenance += ".scene";
            retVal = script->getPropertyName(index, pName, &appartenance);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

int CWorld::getPropertyInfo(long long int target, const char* ppName, int& info, std::string& infoTxt, CWorld* targetObject)
{
    if ( (target == sim_handle_mainscript) && (targetObject != nullptr) )
    {
        CScriptObject* it = targetObject->sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
            target = it->getScriptHandle();
    }

    int retVal = -1;
    if (target == sim_handle_scene)
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        if (App::currentWorld->dynamicsContainer != nullptr)
            retVal = App::currentWorld->dynamicsContainer->getPropertyInfo(pName, info, infoTxt);
        if ( (retVal == -1) && (App::currentWorld->simulation != nullptr) )
            retVal = App::currentWorld->simulation->getPropertyInfo(pName, info, infoTxt);
        if ( (retVal == -1) && (App::currentWorld->environment != nullptr) )
            retVal = App::currentWorld->environment->getPropertyInfo(pName, info, infoTxt);
        if (retVal == -1)
        {
            CSceneObjectContainer* soc = nullptr;
            if (targetObject != nullptr)
                soc = targetObject->sceneObjects;
            retVal = CSceneObjectContainer::getPropertyInfo(-1, pName, info, infoTxt, soc); // for the container itself
        }
        if (retVal == -1)
        {
            std::string pN(pName);
            if (utils::replaceSubstringStart(pN, CUSTOMDATAPREFIX, ""))
            {
                if (targetObject != nullptr)
                {
                    if (pN.size() > 0)
                    {
                        int s;
                        retVal = targetObject->customSceneData.hasData(pN.c_str(), true, &s);
                        if (retVal >= 0)
                        {
                            info = 4; // removable
                            if (s > LARGE_PROPERTY_SIZE)
                                info = info | 0x100;
                        }
                    }
                }
            }
            else if (utils::replaceSubstringStart(pN, SIGNALPREFIX, ""))
            {
                if (targetObject != nullptr)
                {
                    if (pN.size() > 0)
                    {
                        int s;
                        retVal = targetObject->customSceneData_volatile.hasData(pN.c_str(), true, &s);
                        if (retVal >= 0)
                        {
                            info = 4; // removable
                            if (s > LARGE_PROPERTY_SIZE)
                                info = info | 0x100;
                        }
                    }
                }
            }
        }
    }
    else if ( ( (target >= 0) && (target <= SIM_IDEND_SCENEOBJECT) ) || (target >= SIM_UIDSTART) )
    {
        std::string _pName(utils::getWithoutPrefix(ppName, "scene."));
        const char* pName = _pName.c_str();
        CSceneObjectContainer* soc = nullptr;
        if (targetObject != nullptr)
            soc = targetObject->sceneObjects;
        retVal = CSceneObjectContainer::getPropertyInfo(target, pName, info, infoTxt, soc);
    }
    else if ( (target >= SIM_IDSTART_LUASCRIPT) && (target <= SIM_IDEND_LUASCRIPT) && (targetObject != nullptr) )
    { // sandbox, main, add-ons, or old associated scripts:
        CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
                _pName = utils::getWithoutPrefix(ppName, "scene.");
            const char* pName = _pName.c_str();
            retVal = script->getPropertyInfo(pName, info, infoTxt, true);
        }
    }
    else
        retVal = -2; // target does not exist
    return retVal;
}

#ifdef SIM_WITH_GUI
void CWorld::renderYourGeneralObject3DStuff_beforeRegularObjects(CViewableBase *renderingObject, int displayAttrib,
                                                                 int windowSize[2], double verticalViewSizeOrAngle,
                                                                 bool perspective)
{
    distances_old->renderYour3DStuff(renderingObject, displayAttrib);
    drawingCont->renderYour3DStuff_nonTransparent(renderingObject, displayAttrib);
    pointCloudCont_old->renderYour3DStuff_nonTransparent(renderingObject, displayAttrib);
    ghostObjectCont_old->renderYour3DStuff_nonTransparent(renderingObject, displayAttrib);
    bannerCont_old->renderYour3DStuff_nonTransparent(renderingObject, displayAttrib, windowSize, verticalViewSizeOrAngle,
                                                 perspective);
    dynamicsContainer->renderYour3DStuff(renderingObject, displayAttrib);
}

void CWorld::renderYourGeneralObject3DStuff_afterRegularObjects(CViewableBase *renderingObject, int displayAttrib,
                                                                int windowSize[2], double verticalViewSizeOrAngle,
                                                                bool perspective)
{
    drawingCont->renderYour3DStuff_transparent(renderingObject, displayAttrib);
    pointCloudCont_old->renderYour3DStuff_transparent(renderingObject, displayAttrib);
    ghostObjectCont_old->renderYour3DStuff_transparent(renderingObject, displayAttrib);
    bannerCont_old->renderYour3DStuff_transparent(renderingObject, displayAttrib, windowSize, verticalViewSizeOrAngle,
                                              perspective);
}

void CWorld::renderYourGeneralObject3DStuff_onTopOfRegularObjects(CViewableBase *renderingObject, int displayAttrib,
                                                                  int windowSize[2], double verticalViewSizeOrAngle,
                                                                  bool perspective)
{
    drawingCont->renderYour3DStuff_overlay(renderingObject, displayAttrib);
    pointCloudCont_old->renderYour3DStuff_overlay(renderingObject, displayAttrib);
    ghostObjectCont_old->renderYour3DStuff_overlay(renderingObject, displayAttrib);
    bannerCont_old->renderYour3DStuff_overlay(renderingObject, displayAttrib, windowSize, verticalViewSizeOrAngle,
                                          perspective);
    collisions_old->renderYour3DStuff(renderingObject, displayAttrib);
    dynamicsContainer->renderYour3DStuff_overlay(renderingObject, displayAttrib);
}
#endif
