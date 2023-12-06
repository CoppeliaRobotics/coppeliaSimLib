#include <simInternal.h>
#include <fileOperations.h>
#include <simulation.h>
#include <tt.h>
#include <persistentDataContainer.h>
#include <sceneObjectOperations.h>
#include <algos.h>
#include <app.h>
#include <meshManip.h>
#include <mesh.h>
#include <simStrings.h>
#include <boost/lexical_cast.hpp>
#include <imgLoaderSaver.h>
#include <vVarious.h>
#include <vDateTime.h>
#include <utils.h>
#include <simFlavor.h>
#include <boost/algorithm/string/predicate.hpp>
#ifdef SIM_WITH_GUI
#include <vFileDialog.h>
#include <vMessageBox.h>
#include <guiApp.h>
#endif
#include <vFileFinder.h>

void CFileOperations::createNewScene(bool keepCurrentScene)
{
    TRACE_INTERNAL;
    CSimFlavor::run(2);
    if (keepCurrentScene)
        App::worldContainer->createNewWorld();
    else
        App::currentWorld->simulation->stopSimulation();
    App::currentWorld->clearScene(true);
    std::string fullPathAndFilename = App::folders->getSystemPath() + "/";
    fullPathAndFilename += CSimFlavor::getStringVal(16);
    loadScene(fullPathAndFilename.c_str(), false);
    App::currentWorld->mainSettings->setScenePathAndName("");
    App::currentWorld->environment->generateNewUniquePersistentIdString();
    App::currentWorld->undoBufferContainer->memorizeState(); // so that we can come back to the initial state!
    App::currentWorld->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
}

void CFileOperations::closeScene()
{
    App::currentWorld->simulation->stopSimulation();
    App::currentWorld->clearScene(true);
    if (App::worldContainer->getWorldCount() > 1)
        App::worldContainer->destroyCurrentWorld();
    else
    { // simply set-up an empty (default) scene
        std::string savedLoc = App::currentWorld->mainSettings->getScenePathAndName();
        std::string fullPathAndFilename = App::folders->getSystemPath() + "/";
        fullPathAndFilename += "dfltscn.";
        fullPathAndFilename += SIM_SCENE_EXTENSION;
        loadScene(fullPathAndFilename.c_str(), false);
        App::currentWorld->mainSettings->setScenePathAndName(""); // savedLoc.c_str());
        App::currentWorld->environment->generateNewUniquePersistentIdString();
        App::currentWorld->undoBufferContainer->memorizeState(); // so that we can come back to the initial state!
        App::currentWorld->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
    }
#ifdef SIM_WITH_GUI
    if (GuiApp::mainWindow != nullptr)
        GuiApp::mainWindow->refreshDimensions(); // this is important so that the new pages and views are set to the
                                                 // correct dimensions
    GuiApp::setRebuildHierarchyFlag();
#endif
}

bool CFileOperations::loadScene(const char *pathAndFilename, bool setCurrentDir,
                                std::vector<char> *loadBuffer /*=nullptr*/, std::string *infoStr /*=nullptr*/,
                                std::string *errorStr /*=nullptr*/)
{ // empty pathAndFilename to create the default scene
    bool retVal = false;
    TRACE_INTERNAL;
    if ((pathAndFilename == nullptr) || (strlen(pathAndFilename) != 0))
    {
        int result = -3;
        CSimFlavor::run(2);
        App::currentWorld->sceneObjects->deselectObjects();
        App::currentWorld->simulation->stopSimulation(); // should be anyway stopped!
        if ((pathAndFilename == nullptr) || VFile::doesFileExist(pathAndFilename))
        {
            App::currentWorld->clearScene(true);
            if (pathAndFilename != nullptr)
                App::currentWorld->mainSettings->setScenePathAndName(pathAndFilename);
            if (setCurrentDir)
                App::folders->setScenesPath(App::currentWorld->mainSettings->getScenePath().c_str());
            if (CSimFlavor::getBoolVal_str(1, App::currentWorld->mainSettings->getScenePathAndName().c_str()))
                App::currentWorld->mainSettings->setScenePathAndName("");

            if (pathAndFilename != nullptr)
            { // loading from file...
                CSer *serObj;
                if ((CSer::getFileTypeFromName(pathAndFilename) == CSer::filetype_csim_xml_simplescene_file))
                {
                    serObj = new CSer(pathAndFilename, CSer::getFileTypeFromName(pathAndFilename));
                    result = serObj->readOpenXml(0, false, infoStr, errorStr);
                    if (serObj->getFileType() ==
                        CSer::filetype_csim_xml_simplescene_file) // final file type is set in readOpenXml (whether
                                                                  // exhaustive or simple scene)
                        App::currentWorld->mainSettings->setScenePathAndName(""); // since lossy format
                }
                else
                {
                    serObj = new CSer(pathAndFilename, CSer::getFileTypeFromName(pathAndFilename));
                    result = serObj->readOpenBinary(0, false, infoStr, errorStr);
                }

                if (result == 1)
                {
                    App::currentWorld->loadScene(serObj[0], false);
                    serObj->readClose();
                    if (infoStr != nullptr)
                    {
                        std::string acknowledgement(App::currentWorld->environment->getAcknowledgement());
                        tt::removeSpacesAtBeginningAndEnd(acknowledgement);
                        if (acknowledgement.length() != 0)
                        {
                            acknowledgement = "Scene infos:\n" + acknowledgement;
                            infoStr[0] += "\n";
                            infoStr[0] += acknowledgement;
                        }
                    }
                }
                delete serObj;
            }
            else
            { // loading from buffer
                CSer serObj(loadBuffer[0], CSer::filetype_csim_bin_scene_buff);
                result = serObj.readOpenBinary(0, false, infoStr, errorStr);
                if (result == 1)
                {
                    App::currentWorld->loadScene(serObj, false);
                    serObj.readClose();
                }
            }
            App::currentWorld->undoBufferContainer->memorizeState(); // so that we can come back to the initial state!
        }
        else
        {
            if (errorStr != nullptr)
                errorStr[0] = "file does not exist.";
        }
        retVal = (result == 1);
    }
    else
    {
        createNewScene(true);
        retVal = true;
    }
#ifdef SIM_WITH_GUI
    if (GuiApp::mainWindow != nullptr)
        GuiApp::mainWindow->refreshDimensions(); // this is important so that the new pages and views are set to the
                                                 // correct dimensions
    GuiApp::setRebuildHierarchyFlag();
#endif
    return retVal;
}

bool CFileOperations::loadModel(const char *pathAndFilename, bool setCurrentDir, bool doUndoThingInHere,
                                std::vector<char> *loadBuffer, bool onlyThumbnail, bool forceModelAsCopy,
                                std::string *infoStr /*=nullptr*/, std::string *errorStr /*=nullptr*/)
{
    TRACE_INTERNAL;
    int result = -3;
    CSimFlavor::run(2);
    if ((pathAndFilename == nullptr) || VFile::doesFileExist(pathAndFilename))
    {
        App::currentWorld->sceneObjects->deselectObjects();

        if (setCurrentDir && (pathAndFilename != nullptr))
            App::folders->setModelsPath(App::folders->getPathFromFull(pathAndFilename).c_str());

        if (pathAndFilename != nullptr)
        { // loading from file...
            CSer *serObj;

            if (CSer::getFileTypeFromName(pathAndFilename) == CSer::filetype_csim_xml_simplemodel_file)
            {
                serObj = new CSer(pathAndFilename, CSer::getFileTypeFromName(pathAndFilename));
                result = serObj->readOpenXml(1, false, infoStr, errorStr);
            }
            else
            {
                serObj = new CSer(pathAndFilename, CSer::getFileTypeFromName(pathAndFilename));
                result = serObj->readOpenBinary(1, false, infoStr, errorStr);
            }

            if (result == 1)
            {
                App::currentWorld->loadModel(serObj[0], onlyThumbnail, forceModelAsCopy, nullptr, nullptr, nullptr);
                serObj->readClose();
                if ((!onlyThumbnail) && (infoStr != nullptr))
                {
                    std::vector<CSceneObject *> loadedObjects;
                    App::currentWorld->sceneObjects->getSelectedObjects(loadedObjects);
                    for (size_t obba = 0; obba < loadedObjects.size(); obba++)
                    {
                        if (loadedObjects[obba]->getParent() == nullptr)
                        {
                            std::string acknowledgement(loadedObjects[obba]->getModelAcknowledgement());
                            tt::removeSpacesAtBeginningAndEnd(acknowledgement);
                            if (acknowledgement.size() > 0)
                            {
                                acknowledgement = "Model infos:\n" + acknowledgement;
                                infoStr[0] += "\n";
                                infoStr[0] += acknowledgement;
                            }
                            break;
                        }
                    }
                }
            }
            delete serObj;
        }
        else
        { // loading from buffer...
            CSer serObj(loadBuffer[0], CSer::filetype_csim_bin_model_buff);
            result = serObj.readOpenBinary(1, false, infoStr, errorStr);
            if (result == 1)
            {
                App::currentWorld->loadModel(serObj, onlyThumbnail, forceModelAsCopy, nullptr, nullptr, nullptr);
                serObj.readClose();
            }
        }

        App::currentWorld->sceneObjects->removeFromSelectionAllExceptModelBase(false);
        if (doUndoThingInHere)
            App::undoRedo_sceneChanged("");
    }
    else
    {
        if (errorStr != nullptr)
            errorStr[0] = "file does not exist.";
    }
    return (result == 1);
}

bool CFileOperations::saveScene(const char *pathAndFilename, bool setCurrentDir, bool changeSceneUniqueId,
                                std::vector<char> *saveBuffer /*=nullptr*/, std::string *infoStr /*=nullptr*/,
                                std::string *errorStr /*=nullptr*/)
{
    bool retVal = false;
    if (CSimFlavor::getBoolVal(16))
    {
#ifdef SIM_WITH_GUI
        if (GuiApp::mainWindow != nullptr)
            GuiApp::mainWindow->codeEditorContainer->saveOrCopyOperationAboutToHappen();
#endif
        App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_scenesave);
        if (pathAndFilename != nullptr)
        { // saving to file...
            std::string _pathAndFilename(pathAndFilename);
            size_t simpleXmlPos = _pathAndFilename.find("@simpleXml");
            bool simpleXml = (simpleXmlPos != std::string::npos);
            if (simpleXml)
                _pathAndFilename.erase(_pathAndFilename.begin() + simpleXmlPos, _pathAndFilename.end());
            CSer *serObj;
            if ((CSer::getFileTypeFromName(_pathAndFilename.c_str()) == CSer::filetype_csim_xml_simplescene_file))
            {
                VFile::eraseFilesWithPrefix(VVarious::splitPath_path(_pathAndFilename.c_str()).c_str(),
                                            (VVarious::splitPath_fileBase(_pathAndFilename.c_str()) + "_").c_str());
                if (!simpleXml)
                    serObj = new CSer(_pathAndFilename.c_str(), CSer::filetype_csim_xml_xscene_file);
                else
                    serObj = new CSer(_pathAndFilename.c_str(), CSer::filetype_csim_xml_simplescene_file);
                retVal = serObj->writeOpenXml(App::userSettings->xmlExportSplitSize,
                                              App::userSettings->xmlExportKnownFormats);
            }
            else
            {
                serObj = new CSer(_pathAndFilename.c_str(), CSer::getFileTypeFromName(_pathAndFilename.c_str()));
                retVal = serObj->writeOpenBinary(App::userSettings->compressFiles);
            }

            if (infoStr != nullptr)
            {
                infoStr[0] = "Scene: ";
                infoStr[0] += _pathAndFilename;
            }

            if (retVal)
            {
                if (!simpleXml) // because lossy
                    App::currentWorld->mainSettings->setScenePathAndName(_pathAndFilename.c_str());

                App::currentWorld->embeddedScriptContainer->sceneOrModelAboutToBeSaved_old(-1);

                if (changeSceneUniqueId)
                    App::currentWorld->environment->generateNewUniquePersistentIdString();

                if (setCurrentDir)
                    App::folders->setScenesPath(App::currentWorld->mainSettings->getScenePath().c_str());

                if (infoStr != nullptr)
                {
                    infoStr[0] += "\n";
                    infoStr[0] += IDSNS_SERIALIZATION_VERSION_IS;
                    infoStr[0] += " ";
                    if ((serObj->getFileType() == CSer::filetype_csim_xml_xscene_file) ||
                        (serObj->getFileType() == CSer::filetype_csim_xml_simplescene_file))
                        infoStr[0] += boost::lexical_cast<std::string>(CSer::XML_XSERIALIZATION_VERSION) + ".";
                    else
                        infoStr[0] += boost::lexical_cast<std::string>(CSer::SER_SERIALIZATION_VERSION) + ".";
                }
                App::currentWorld->saveScene(serObj[0]);
                serObj->writeClose();
            }
            delete serObj;
        }
        else
        { // saving to buffer...
            CSer serObj(saveBuffer[0], CSer::filetype_csim_bin_scene_buff);
            retVal = serObj.writeOpenBinary(App::userSettings->compressFiles);
            App::currentWorld->embeddedScriptContainer->sceneOrModelAboutToBeSaved_old(-1);
            App::currentWorld->saveScene(serObj);
            serObj.writeClose();
        }
    }
    if ((!retVal) && (errorStr != nullptr))
        errorStr[0] = "Failed to save scene.";
    return (retVal);
}

bool CFileOperations::saveModel(int modelBaseDummyID, const char *pathAndFilename, bool setCurrentDir,
                                std::vector<char> *saveBuffer /*=nullptr*/, std::string *infoStr /*=nullptr*/,
                                std::string *errorStr /*=nullptr*/)
{
    bool retVal = false;
    if (CSimFlavor::getBoolVal(16) || (saveBuffer != nullptr))
    {
        App::currentWorld->embeddedScriptContainer->sceneOrModelAboutToBeSaved_old(modelBaseDummyID);
        std::vector<int> sel;
        sel.push_back(modelBaseDummyID);

        CSceneObject *modelBaseObject = App::currentWorld->sceneObjects->getObjectFromHandle(modelBaseDummyID);
        C3Vector minV(C3Vector::inf);
        C3Vector maxV(C3Vector::ninf);
        C7Vector modelTr(modelBaseObject->getCumulativeTransformation() * modelBaseObject->getBB(nullptr));
        C3Vector modelBBSize;
        double modelNonDefaultTranslationStepSize = modelBaseObject->getObjectMovementStepSize(0);

        if (modelBaseObject->getModelBB(modelTr.getInverse(), minV, maxV, true))
        {
            modelBBSize = maxV - minV;
            modelTr.X += modelTr.Q * ((minV + maxV) * 0.5);
        }
        else
            modelBBSize = C3Vector::zeroVector;

#ifdef SIM_WITH_GUI
        if (GuiApp::mainWindow != nullptr)
            GuiApp::mainWindow->codeEditorContainer->saveOrCopyOperationAboutToHappen();
#endif

        App::currentWorld->sceneObjects->addModelObjects(sel);
        App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_modelsave);

        std::string infoPrintOut(IDSNS_SAVING_MODEL);
        if (pathAndFilename != nullptr)
        {
            if (setCurrentDir)
                App::folders->setModelsPath(App::folders->getPathFromFull(pathAndFilename).c_str());
            infoPrintOut += " (";
            infoPrintOut += std::string(pathAndFilename) + "). ";
        }

        if (pathAndFilename != nullptr)
        { // saving to file...
            if (infoStr != nullptr)
            {
                infoStr[0] = "Model: ";
                infoStr[0] += pathAndFilename;
                infoStr[0] += "\n";
                infoStr[0] += IDSNS_SERIALIZATION_VERSION_IS;
                infoStr[0] += " ";
                infoStr[0] += boost::lexical_cast<std::string>(CSer::SER_SERIALIZATION_VERSION) + ".";
            }

            CSer *serObj;
            if (CSer::getFileTypeFromName(pathAndFilename) == CSer::filetype_csim_xml_simplemodel_file)
            {
                VFile::eraseFilesWithPrefix(VVarious::splitPath_path(pathAndFilename).c_str(),
                                            (VVarious::splitPath_fileBase(pathAndFilename) + "_").c_str());
                serObj = new CSer(pathAndFilename,
                                  CSer::filetype_csim_xml_xmodel_file); // we can only save exhaustive models
                serObj->writeOpenXml(App::userSettings->xmlExportSplitSize, App::userSettings->xmlExportKnownFormats);
            }
            else
            {
                serObj = new CSer(pathAndFilename, CSer::getFileTypeFromName(pathAndFilename));
                serObj->writeOpenBinary(App::userSettings->compressFiles);
            }
            App::worldContainer->copyBuffer->serializeCurrentSelection(serObj[0], &sel, modelTr, modelBBSize,
                                                                       modelNonDefaultTranslationStepSize);
            serObj->writeClose();
            delete serObj;
        }
        else
        { // saving to buffer...
            CSer serObj(saveBuffer[0], CSer::filetype_csim_bin_model_buff);

            serObj.writeOpenBinary(App::userSettings->compressFiles);
            App::worldContainer->copyBuffer->serializeCurrentSelection(serObj, &sel, modelTr, modelBBSize,
                                                                       modelNonDefaultTranslationStepSize);
            serObj.writeClose();
        }
        retVal = true;
    }
    else
    {
        if (errorStr != nullptr)
            errorStr[0] = "Model could not be saved.";
    }
    return (retVal);
}

int CFileOperations::createHeightfield(int xSize, double pointSpacing,
                                       const std::vector<std::vector<double> *> &readData, double shadingAngle,
                                       int options)
{ // options bits:
    // 0 set --> backfaces are culled
    // 1 set --> edges are visible
    // 2 set --> a normal shape is created instead
    // 4 set --> non respondable
    int ySize = int(readData.size());
    std::vector<double> allHeights;
    double maxHeight = -99999999.0;
    double minHeight = +99999999.0;
    for (int i = ySize - 1; i >= 0; i--)
    {
        for (int j = 0; j < xSize; j++)
        {
            allHeights.push_back(readData[i]->at(j));
            if (readData[i]->at(j) > maxHeight)
                maxHeight = readData[i]->at(j);
            if (readData[i]->at(j) < minHeight)
                minHeight = readData[i]->at(j);
        }
    }
    CShape *shape = new CShape(allHeights, xSize, ySize, pointSpacing, maxHeight - minHeight);

    if (options & 4)
        shape->getSingleMesh()->setPurePrimitiveType(sim_primitiveshape_none, 1.0, 1.0, 1.0);

    shape->setCulling((options & 1) != 0);
    shape->setVisibleEdges((options & 2) != 0);
    shape->getSingleMesh()->setShadingAngle(shadingAngle);
    shape->getSingleMesh()->setEdgeThresholdAngle(shadingAngle);
    shape->setColor(nullptr, sim_colorcomponent_ambient_diffuse, 0.68f, 0.56f, 0.36f);
    shape->setColor(nullptr, sim_colorcomponent_specular, 0.25f, 0.25f, 0.25f);
    App::currentWorld->sceneObjects->addObjectToScene(shape, false, true);
    App::currentWorld->sceneObjects->setObjectAlias(shape, "heightfield", true);
    App::currentWorld->sceneObjects->setObjectName_old(shape, "heightfield", true);
    App::currentWorld->sceneObjects->setObjectAltName_old(shape, "heightfield", true);

    int propToRemove = sim_objectspecialproperty_collidable | sim_objectspecialproperty_measurable;
    shape->setLocalObjectSpecialProperty((shape->getLocalObjectSpecialProperty() | propToRemove) - propToRemove);
    shape->setRespondable((options & 8) == 0);
    shape->setStatic(true);

    return (shape->getObjectHandle());
}

#ifdef SIM_WITH_GUI
void CFileOperations::_addToRecentlyOpenedScenes(std::string filenameAndPath)
{
    CPersistentDataContainer cont;
    std::string recentScenes[10];
    int sameIndex = -1;
    for (int i = 0; i < 10; i++)
    {
        std::string tmp("SIMSETTINGS_RECENTSCENE0");
        tmp[23] = 48 + i;
        cont.readData(tmp.c_str(), recentScenes[i]);
        if (recentScenes[i].compare(filenameAndPath) == 0)
            sameIndex = i;
    }
    if (sameIndex == -1)
    {
        for (int i = 8; i >= 0; i--)
            recentScenes[i + 1] = recentScenes[i];
    }
    else
    {
        for (int i = sameIndex; i > 0; i--)
            recentScenes[i] = recentScenes[i - 1];
    }
    recentScenes[0] = filenameAndPath;
    int cnt = 0;
    for (int i = 0; i < 10; i++)
    {
        if (recentScenes[i].length() > 3)
        {
            std::string tmp("SIMSETTINGS_RECENTSCENE0");
            tmp[23] = 48 + cnt;
            cont.writeData(tmp.c_str(), recentScenes[i], !App::userSettings->doNotWritePersistentData);
            cnt++;
        }
    }
}

void CFileOperations::_removeFromRecentlyOpenedScenes(std::string filenameAndPath)
{
    CPersistentDataContainer cont;
    std::string recentScenes[10];
    int sameIndex = -1;
    for (int i = 0; i < 10; i++)
    {
        std::string tmp("SIMSETTINGS_RECENTSCENE0");
        tmp[23] = 48 + i;
        cont.readData(tmp.c_str(), recentScenes[i]);
        if (recentScenes[i].compare(filenameAndPath) == 0)
            sameIndex = i;
    }
    if (sameIndex != -1)
    {
        for (int i = sameIndex; i < 9; i++)
            recentScenes[i] = recentScenes[i + 1];
        recentScenes[9] = "";
        int cnt = 0;
        for (int i = 0; i < 10; i++)
        {
            if (recentScenes[i].length() > 3)
            {
                std::string tmp("SIMSETTINGS_RECENTSCENE0");
                tmp[23] = 48 + cnt;
                cont.writeData(tmp.c_str(), recentScenes[i], !App::userSettings->doNotWritePersistentData);
                cnt++;
            }
        }
        for (int i = cnt; i < 10; i++)
        {
            std::string tmp("SIMSETTINGS_RECENTSCENE0");
            tmp[23] = 48 + i;
            cont.writeData(tmp.c_str(), "", !App::userSettings->doNotWritePersistentData);
        }
    }
}

void CFileOperations::keyPress(int key)
{
    if (key == CTRL_S_KEY)
        processCommand(FILE_OPERATION_SAVE_SCENE_FOCMD);
    if (key == CTRL_O_KEY)
        processCommand(FILE_OPERATION_OPEN_SCENE_FOCMD);
    if (key == CTRL_W_KEY)
        processCommand(FILE_OPERATION_CLOSE_SCENE_FOCMD);
    if (key == CTRL_Q_KEY)
        processCommand(FILE_OPERATION_EXIT_SIMULATOR_FOCMD);
    if (key == CTRL_N_KEY)
        processCommand(FILE_OPERATION_NEW_SCENE_FOCMD);
}

void CFileOperations::addMenu(VMenu *menu)
{
    bool fileOpOk =
        (App::currentWorld->simulation->isSimulationStopped()) && (GuiApp::getEditModeType() == NO_EDIT_MODE);
    bool simStoppedOrPausedNoEditMode =
        App::currentWorld->simulation->isSimulationStopped() || App::currentWorld->simulation->isSimulationPaused();
    bool fileOpOkAlsoDuringSimulation = (GuiApp::getEditModeType() == NO_EDIT_MODE);
    size_t selItems = App::currentWorld->sceneObjects->getSelectionCount();
    bool justModelSelected = false;
    if (selItems == 1)
    {
        CSceneObject *obj = App::currentWorld->sceneObjects->getObjectFromHandle(
            App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(0));
        justModelSelected = (obj != nullptr) && (obj->getModelBase());
    }
    std::vector<int> sel;
    App::currentWorld->sceneObjects->getSelectedObjectHandles(sel, sim_object_shape_type, true, true);
    size_t shapeNumber = sel.size();
    App::currentWorld->sceneObjects->getSelectedObjectHandles(sel, sim_object_graph_type, true);
    size_t graphNumber = sel.size();

    menu->appendMenuItem(fileOpOk, false, FILE_OPERATION_NEW_SCENE_FOCMD, IDS_NEW_SCENE_MENU_ITEM);

    menu->appendMenuItem(fileOpOk, false, FILE_OPERATION_OPEN_SCENE_FOCMD, IDS_OPEN_SCENE___MENU_ITEM);

    // recent scene files:
    CPersistentDataContainer cont;
    std::string recentScenes[10];
    int recentScenesCnt = 0;
    for (int i = 0; i < 10; i++)
    {
        std::string tmp("SIMSETTINGS_RECENTSCENE0");
        tmp[23] = 48 + i;
        cont.readData(tmp.c_str(), recentScenes[i]);
        if (recentScenes[i].length() > 3)
            recentScenesCnt++;
    }
    VMenu *recentSceneMenu = new VMenu();
    for (int i = 0; i < 10; i++)
    {
        if (recentScenes[i].length() > 3)
            recentSceneMenu->appendMenuItem(fileOpOk, false, FILE_OPERATION_OPEN_RECENT_SCENE0_FOCMD + i,
                                            VVarious::splitPath_fileBaseAndExtension(recentScenes[i].c_str()).c_str());
    }
    menu->appendMenuAndDetach(recentSceneMenu, (recentScenesCnt > 0) && fileOpOk, IDS_OPEN_RECENT_SCENE_MENU_ITEM);

    menu->appendMenuItem(fileOpOkAlsoDuringSimulation, false, FILE_OPERATION_LOAD_MODEL_FOCMD,
                         IDS_LOAD_MODEL___MENU_ITEM);

    menu->appendMenuSeparator();

    menu->appendMenuItem(fileOpOk, false, FILE_OPERATION_CLOSE_SCENE_FOCMD, IDS_CLOSE_SCENE_MENU_ITEM);

    if (CSimFlavor::getIntVal(2) != 0)
    {
        menu->appendMenuSeparator();
        bool r = ( CSimFlavor::getBoolVal(16) || (CSimFlavor::getIntVal(2) == -1) );
        menu->appendMenuItem(fileOpOk, false,
                             r ? FILE_OPERATION_SAVE_SCENE_FOCMD : FILE_OPERATION_RG,
                             IDS_SAVE_SCENE_MENU_ITEM);
        VMenu *saveSceneMenu = new VMenu();
        saveSceneMenu->appendMenuItem(fileOpOk, false,
                                      r ? FILE_OPERATION_SAVE_SCENE_AS_CSIM_FOCMD : FILE_OPERATION_RG,
                                      IDS_SCENE_AS_CSIM___MENU_ITEM);
        VMenu *saveSceneAsXmlMenu = new VMenu();
        saveSceneAsXmlMenu->appendMenuItem(fileOpOk, false,
                                           r ? FILE_OPERATION_SAVE_SCENE_AS_EXXML_FOCMD : FILE_OPERATION_RG,
                                           IDS_SCENE_AS_XML___MENU_ITEM);
        saveSceneAsXmlMenu->appendMenuItem(fileOpOk, false,
                                           r ? FILE_OPERATION_SAVE_SCENE_AS_SIMPLEXML_FOCMD : FILE_OPERATION_RG,
                                           IDS_SCENE_AS_SIMPLEXML___MENU_ITEM);
        saveSceneMenu->appendMenuAndDetach(saveSceneAsXmlMenu, fileOpOk, IDS_SAVE_SCENE_AS_XML_MENU_ITEM);
        menu->appendMenuAndDetach(saveSceneMenu, fileOpOk, IDS_SAVE_SCENE_AS_MENU_ITEM);

        VMenu *saveModelMenu = new VMenu();
        saveModelMenu->appendMenuItem(fileOpOk && justModelSelected, false,
                                      r ? FILE_OPERATION_SAVE_MODEL_AS_CSIM_FOCMD : FILE_OPERATION_RG,
                                      IDS_MODEL_AS_CSIM___MENU_ITEM);
        saveModelMenu->appendMenuItem(fileOpOk && justModelSelected, false,
                                      r ? FILE_OPERATION_SAVE_MODEL_AS_EXXML_FOCMD : FILE_OPERATION_RG,
                                      IDS_MODEL_AS_XML___MENU_ITEM);
        menu->appendMenuAndDetach(saveModelMenu, fileOpOk && justModelSelected, IDS_SAVE_MODEL_AS_MENU_ITEM);
    }

    if ((CSimFlavor::getIntVal(2) == -1) || (CSimFlavor::getIntVal(2) == 1) || (CSimFlavor::getIntVal(2) == 2))
    {
        menu->appendMenuSeparator();
        VMenu *impMenu = new VMenu();
        impMenu->appendMenuItem(fileOpOk, false, FILE_OPERATION_IMPORT_MESH_FOCMD, IDS_IMPORT_MESH___MENU_ITEM);
        impMenu->appendMenuItem(fileOpOk, false, FILE_OPERATION_IMPORT_HEIGHTFIELD_FOCMD,
                                (std::string(IDSN_IMPORT_HEIGHTFIELD) + "...").c_str());
        menu->appendMenuAndDetach(impMenu, true, IDSN_IMPORT_MENU_ITEM);

        VMenu *expMenu = new VMenu();
        expMenu->appendMenuItem(simStoppedOrPausedNoEditMode && (shapeNumber > 0), false,
                                FILE_OPERATION_EXPORT_SHAPE_FOCMD, IDS_EXPORT_SELECTION_SHAPES_MENU_ITEM);
        expMenu->appendMenuItem(fileOpOk && (graphNumber != 0), false, FILE_OPERATION_EXPORT_GRAPHS_FOCMD,
                                IDS_EXPORT_SELECTION_GRAPHS_MENU_ITEM);
        bool canExportDynamicContent = App::worldContainer->pluginContainer->dyn_isDynamicContentAvailable() != 0;
        expMenu->appendMenuItem(canExportDynamicContent, false, FILE_OPERATION_EXPORT_DYNAMIC_CONTENT_FOCMD,
                                IDSN_EXPORT_DYNAMIC_CONTENT);
        menu->appendMenuAndDetach(expMenu, true, IDSN_EXPORT_MENU_ITEM);
    }

    menu->appendMenuSeparator();
    menu->appendMenuItem(true, false, FILE_OPERATION_EXIT_SIMULATOR_FOCMD, IDS_EXIT_MENU_ITEM);
}

bool CFileOperations::_saveSceneWithDialogAndEverything()
{ // SHOULD ONLY BE CALLED BY THE MAIN SIMULATION THREAD!
    bool retVal = false;
    if (!App::currentWorld->environment->getSceneLocked())
    {
        if (App::currentWorld->mainSettings->getScenePathAndName() == "")
            retVal = _saveSceneAsWithDialogAndEverything(CSimFlavor::getIntVal(1));
        else
        {
            if ((!App::currentWorld->environment->getRequestFinalSave()) ||
                (VMESSAGEBOX_REPLY_YES ==
                 GuiApp::uiThread->messageBox_warning(GuiApp::mainWindow, IDSN_SAVE, IDS_FINAL_SCENE_SAVE_WARNING,
                                                      VMESSAGEBOX_YES_NO, VMESSAGEBOX_REPLY_YES)))
            {
                if (App::currentWorld->environment->getRequestFinalSave())
                    App::currentWorld->environment->setSceneLocked();
                std::string infoPrintOut(IDSN_SAVING_SCENE);
                infoPrintOut += "...";
                App::logMsg(sim_verbosity_msgs, infoPrintOut.c_str());
                std::string infoStr;
                std::string errorStr;
                if (saveScene(App::currentWorld->mainSettings->getScenePathAndName().c_str(), true, false, nullptr,
                              &infoStr, &errorStr))
                {
                    if (infoStr.size() > 0)
                        App::logMsg(sim_verbosity_msgs, infoStr.c_str());
                    App::logMsg(sim_verbosity_msgs, IDSNS_SCENE_WAS_SAVED);
                    GuiApp::setRebuildHierarchyFlag(); // we might have saved under a different name, we need to reflect
                                                       // it
                    _addToRecentlyOpenedScenes(App::currentWorld->mainSettings->getScenePathAndName());
                    App::currentWorld->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
                }
                else
                {
                    if (infoStr.size() > 0)
                        App::logMsg(sim_verbosity_msgs, infoStr.c_str());
                    App::logMsg(sim_verbosity_errors, errorStr.c_str());
                }
            }
            retVal = true;
        }
    }
    else
        GuiApp::uiThread->messageBox_warning(GuiApp::mainWindow, IDSN_SCENE, IDS_SCENE_IS_LOCKED_WARNING,
                                             VMESSAGEBOX_OKELI, VMESSAGEBOX_REPLY_OK);
    return (retVal);
}

bool CFileOperations::_saveSceneAsWithDialogAndEverything(int filetype)
{
    bool retVal = false;
    if (!App::currentWorld->environment->getSceneLocked())
    {
        if ((!App::currentWorld->environment->getRequestFinalSave()) ||
            (VMESSAGEBOX_REPLY_YES == GuiApp::uiThread->messageBox_warning(GuiApp::mainWindow, IDSN_SAVE,
                                                                           IDS_FINAL_SCENE_SAVE_WARNING,
                                                                           VMESSAGEBOX_YES_NO, VMESSAGEBOX_REPLY_YES)))
        {
            if (App::currentWorld->environment->getRequestFinalSave())
                App::currentWorld->environment->setSceneLocked();

            std::string infoPrintOut(IDSN_SAVING_SCENE);
            infoPrintOut += "...";
            App::logMsg(sim_verbosity_msgs, infoPrintOut.c_str());
            std::string initPath;
            if (App::currentWorld->mainSettings->getScenePathAndName().size() == 0)
                initPath = App::folders->getScenesPath();
            else
                initPath = App::currentWorld->mainSettings->getScenePath();
            std::string filenameAndPath;
            std::string sceneName(App::currentWorld->mainSettings->getScenePathAndName());
            sceneName = VVarious::splitPath_fileBaseAndExtension(sceneName.c_str());
            std::string ext = VVarious::splitPath_fileExtension(sceneName.c_str());
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if ((filetype == CSer::filetype_csim_bin_scene_file) && (ext.compare(SIM_SCENE_EXTENSION) != 0))
                sceneName = "";
            if ((filetype == CSer::filetype_csim_xml_xscene_file) && (ext.compare(SIM_XML_SCENE_EXTENSION) != 0))
                sceneName = "";
            if ((filetype == CSer::filetype_csim_xml_simplescene_file) && (ext.compare(SIM_XML_SCENE_EXTENSION) != 0))
                sceneName = "";

            if (filetype == CSer::filetype_csim_bin_scene_file)
                filenameAndPath = GuiApp::uiThread->getSaveFileName(
                    GuiApp::mainWindow, 0, tt::decorateString("", IDSN_SAVING_SCENE, "...").c_str(), initPath.c_str(),
                    sceneName.c_str(), false, "CoppeliaSim Scene", SIM_SCENE_EXTENSION);
            if (filetype == CSer::filetype_csim_xml_xscene_file)
                filenameAndPath = GuiApp::uiThread->getSaveFileName(
                    GuiApp::mainWindow, 0, tt::decorateString("", IDSN_SAVING_SCENE, "...").c_str(), initPath.c_str(),
                    sceneName.c_str(), false, "CoppeliaSim XML Scene (exhaustive)", SIM_XML_SCENE_EXTENSION);
            if (filetype == CSer::filetype_csim_xml_simplescene_file)
                filenameAndPath = GuiApp::uiThread->getSaveFileName(
                    GuiApp::mainWindow, 0, tt::decorateString("", IDSN_SAVING_SCENE, "...").c_str(), initPath.c_str(),
                    sceneName.c_str(), false, "CoppeliaSim XML Scene (simple)", SIM_XML_SCENE_EXTENSION);

            if (filenameAndPath.length() != 0)
            {
                bool abort = false;
                if (!App::userSettings->suppressXmlOverwriteMsg)
                {
                    if ((filetype == CSer::filetype_csim_xml_xscene_file) &&
                        (App::userSettings->xmlExportSplitSize != 0))
                    {
                        std::string prefix(VVarious::splitPath_fileBase(filenameAndPath.c_str()) + "_");
                        int cnt0 = VFileFinder::countFiles(VVarious::splitPath_path(filenameAndPath.c_str()).c_str());
                        int cnt1 = VFileFinder::countFolders(VVarious::splitPath_path(filenameAndPath.c_str()).c_str());
                        int cnt2 = VFileFinder::countFilesWithPrefix(
                            VVarious::splitPath_path(filenameAndPath.c_str()).c_str(), prefix.c_str());
                        int cnt3 = VFileFinder::countFilesWithPrefix(
                            VVarious::splitPath_path(filenameAndPath.c_str()).c_str(),
                            (VVarious::splitPath_fileBase(filenameAndPath.c_str()) + ".").c_str());
                        if ((cnt2 + cnt3 != cnt0) || (cnt1 != 0))
                        {
                            std::string msg(
                                "The scene/model will possibly be saved as several separate files, all with the '");
                            msg += prefix;
                            msg += "' prefix. Existing files with the same prefix will be erased or overwritten. To "
                                   "avoid this, it is recommended to either save XML scenes/models in individual "
                                   "folders, or to set the 'xmlExportSplitSize' variable in ";
                            msg += App::folders->getUserSettingsPath() +
                                   "/usrset.txt to 0 to generate a single file.\n(this warning can be disabled via the "
                                   "'suppressXmlOverwriteMsg' variable in usrset.txt)\n\nProceed anyway?";
                            abort = (VMESSAGEBOX_REPLY_NO ==
                                     GuiApp::uiThread->messageBox_warning(GuiApp::mainWindow, IDSN_SAVE, msg.c_str(),
                                                                          VMESSAGEBOX_YES_NO, VMESSAGEBOX_REPLY_YES));
                        }
                    }
                    if (filetype == CSer::filetype_csim_xml_simplescene_file)
                    {
                        std::string prefix(VVarious::splitPath_fileBase(filenameAndPath.c_str()) + "_");
                        int cnt0 = VFileFinder::countFiles(VVarious::splitPath_path(filenameAndPath.c_str()).c_str());
                        int cnt1 = VFileFinder::countFolders(VVarious::splitPath_path(filenameAndPath.c_str()).c_str());
                        int cnt2 = VFileFinder::countFilesWithPrefix(
                            VVarious::splitPath_path(filenameAndPath.c_str()).c_str(), prefix.c_str());
                        int cnt3 = VFileFinder::countFilesWithPrefix(
                            VVarious::splitPath_path(filenameAndPath.c_str()).c_str(),
                            (VVarious::splitPath_fileBase(filenameAndPath.c_str()) + ".").c_str());
                        if ((cnt2 + cnt3 != cnt0) || (cnt1 != 0))
                        {
                            std::string msg(
                                "The scene/model will possibly be saved as several separate files, all with the '");
                            msg += prefix;
                            msg +=
                                "' prefix. Existing files with the same prefix will be erased or overwritten. To avoid "
                                "this, it is recommended to save XML scenes/models in individual folders.\n(this "
                                "warning can be disabled via the 'suppressXmlOverwriteMsg' variable in ";
                            msg += App::folders->getUserSettingsPath() + "/usrset.txt)\n\nProceed anyway?";
                            abort = (VMESSAGEBOX_REPLY_NO ==
                                     GuiApp::uiThread->messageBox_warning(GuiApp::mainWindow, IDSN_SAVE, msg.c_str(),
                                                                          VMESSAGEBOX_YES_NO, VMESSAGEBOX_REPLY_YES));
                        }
                    }
                }
                if (!abort)
                {
                    if (filetype == CSer::filetype_csim_xml_simplescene_file)
                        filenameAndPath += "@simpleXml";
                    std::string infoStr;
                    std::string errorStr;
                    if (saveScene(filenameAndPath.c_str(), true, true, nullptr, &infoStr, &errorStr))
                    {
                        if (infoStr.size() > 0)
                            App::logMsg(sim_verbosity_msgs, infoStr.c_str());
                        App::logMsg(sim_verbosity_msgs, IDSNS_SCENE_WAS_SAVED);
                        GuiApp::setRebuildHierarchyFlag(); // we might have saved under a different name, we need to
                                                           // reflect it
                        _addToRecentlyOpenedScenes(App::currentWorld->mainSettings->getScenePathAndName());
                        App::currentWorld->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
                        retVal = true;
                    }
                    else
                    {
                        if (infoStr.size() > 0)
                            App::logMsg(sim_verbosity_msgs, infoStr.c_str());
                        App::logMsg(sim_verbosity_errors, errorStr.c_str());
                    }
                }
                else
                    App::logMsg(sim_verbosity_msgs, "Aborted.");
            }
            else
                App::logMsg(sim_verbosity_msgs, "Aborted.");
        }
    }
    else
        GuiApp::uiThread->messageBox_warning(GuiApp::mainWindow, IDSN_SCENE, IDS_SCENE_IS_LOCKED_WARNING,
                                             VMESSAGEBOX_OKELI, VMESSAGEBOX_REPLY_OK);
    return (retVal);
}

bool CFileOperations::processCommand(int commandID)
{ // Return value is true if the command belonged to file menu and was executed
    if ((commandID > FILE_OPERATION_START_FOCMD) && (commandID < FILE_OPERATION_END_FOCMD))
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId = commandID;
        App::appendSimulationThreadCommand(cmd);
        return (true);
    }
    return (false);
}

bool CFileOperations::processCommand(const SSimulationThreadCommand &cmd)
{
    //-----------
    if (cmd.cmdId == FILE_OPERATION_NEW_SCENE_FOCMD)
    { // Cannot undo this command
        if (App::currentWorld->simulation->isSimulationStopped() && (GuiApp::getEditModeType() == NO_EDIT_MODE))
        { // execute the command only when simulation is not running and not in an edit mode
            if (!VThread::isUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                GuiApp::setDefaultMouseMode();
                createNewScene(true);
                App::logMsg(sim_verbosity_msgs, IDSNS_DEFAULT_SCENE_WAS_SET_UP);
            }
            else
                App::appendSimulationThreadCommand(
                    cmd); // We are in the UI thread. Execute the command via the main thread
        }
        return (true);
    }
    //-----------

    if (cmd.cmdId == FILE_OPERATION_CLOSE_SCENE_FOCMD)
    { // Cannot undo this command
        if (App::currentWorld->simulation->isSimulationStopped() && (GuiApp::getEditModeType() == NO_EDIT_MODE))
        { // execute the command only when simulation is not running and not in an edit mode
            if (!VThread::isUiThread())
            {
                unsigned short action = VMESSAGEBOX_REPLY_NO;
                if (!App::currentWorld->environment->getSceneCanBeDiscardedWhenNewSceneOpened())
                {
                    if (CSimFlavor::getBoolVal(16) && (!App::currentWorld->environment->getSceneLocked()))
                    {
                        if (App::currentWorld->undoBufferContainer->isSceneSaveMaybeNeededFlagSet())
                        {
                            action = GuiApp::uiThread->messageBox_warning(
                                GuiApp::mainWindow, IDSN_SAVE, IDS_WANNA_SAVE_THE_SCENE_WARNING,
                                VMESSAGEBOX_YES_NO_CANCEL, VMESSAGEBOX_REPLY_NO);
                            if (action == VMESSAGEBOX_REPLY_YES)
                            {
                                if (_saveSceneWithDialogAndEverything()) // will call save as if needed!
                                    action = VMESSAGEBOX_REPLY_NO;
                            }
                        }
                    }
                }
                if (action == VMESSAGEBOX_REPLY_NO)
                {
                    App::currentWorld->simulation->stopSimulation();
                    GuiApp::setDefaultMouseMode();
                    if (GuiApp::mainWindow != nullptr)
                        GuiApp::mainWindow->editModeContainer->processCommand(
                            ANY_EDIT_MODE_FINISH_AND_CANCEL_CHANGES_EMCMD, nullptr);
                    closeScene();
                    App::logMsg(sim_verbosity_msgs, "Scene closed.");
                }
            }
            else
                App::appendSimulationThreadCommand(
                    cmd); // We are in the UI thread. Execute the command via the main thread
        }
        return (true);
    }

    //-----------
#ifdef SIM_WITH_GUI
    if (cmd.cmdId == FILE_OPERATION_OPEN_SCENE_FOCMD)
    {
        if (App::currentWorld->simulation->isSimulationStopped() && (GuiApp::getEditModeType() == NO_EDIT_MODE))
        { // execute the command only when simulation is not running and not in an edit mode
            if (!VThread::isUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                App::logMsg(sim_verbosity_msgs, tt::decorateString("", IDSNS_LOADING_SCENE, "...").c_str());
                CSimFlavor::run(2);
                std::string ext[4];
                for (int i = 0; i < 4; i++)
                    ext[i] = CSimFlavor::getStringVal_int(1, i);
                std::string filenameAndPath = GuiApp::uiThread->getOpenFileName(
                    GuiApp::mainWindow, 0, IDSN_LOADING_SCENE, App::folders->getScenesPath().c_str(), "", false,
                    "Scenes", ext[0].c_str(), ext[1].c_str(), ext[2].c_str(), ext[3].c_str());

                if (filenameAndPath.length() != 0)
                {
                    GuiApp::setRebuildHierarchyFlag();
                    GuiApp::setDefaultMouseMode();
                    // App::worldContainer->createNewWorld();
                    createNewScene(true);
                    std::string infoStr;
                    std::string errorStr;
                    if (loadScene(filenameAndPath.c_str(), true, nullptr, &infoStr, &errorStr))
                    {
                        if (infoStr.size() != 0)
                            App::logMsg(sim_verbosity_msgs, infoStr.c_str());
                        App::logMsg(sim_verbosity_msgs, IDSNS_SCENE_OPENED);
                        _addToRecentlyOpenedScenes(filenameAndPath);
                    }
                    else
                    {
                        if (infoStr.size() != 0)
                            App::logMsg(sim_verbosity_msgs, infoStr.c_str());
                        App::logMsg(sim_verbosity_errors, IDSNS_SCENE_COULD_NOT_BE_OPENED);
                        _removeFromRecentlyOpenedScenes(filenameAndPath);
                    }
                }
                else
                    App::logMsg(sim_verbosity_msgs, "Aborted.");
                App::currentWorld->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
            }
            else
                App::appendSimulationThreadCommand(
                    cmd); // We are in the UI thread. Execute the command via the main thread
        }
        return (true);
    }
    //-----------

    //-----------
    if ((cmd.cmdId >= FILE_OPERATION_OPEN_RECENT_SCENE0_FOCMD) &&
        (cmd.cmdId <= FILE_OPERATION_OPEN_RECENT_SCENE9_FOCMD))
    {
        SSimulationThreadCommand cmd2;
        cmd2.cmdId =
            cmd.cmdId - FILE_OPERATION_OPEN_RECENT_SCENE0_FOCMD + FILE_OPERATION_OPEN_RECENT_SCENE0_PHASE2_FOCMD;
        App::appendSimulationThreadCommand(cmd2);
        return (true);
    }
    if ((cmd.cmdId >= FILE_OPERATION_OPEN_RECENT_SCENE0_PHASE2_FOCMD) &&
        (cmd.cmdId <= FILE_OPERATION_OPEN_RECENT_SCENE9_PHASE2_FOCMD))
    {
        if (App::currentWorld->simulation->isSimulationStopped() && (GuiApp::getEditModeType() == NO_EDIT_MODE))
        { // execute the command only when simulation is not running and not in an edit mode
            if (!VThread::isUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                CSimFlavor::run(2);
                CPersistentDataContainer cont;
                std::string filenameAndPath;
                int recentScenesCnt = 0;
                for (int i = 0; i < 10; i++)
                {
                    std::string tmp("SIMSETTINGS_RECENTSCENE0");
                    tmp[23] = 48 + i;
                    cont.readData(tmp.c_str(), filenameAndPath);
                    if (filenameAndPath.length() > 3)
                        recentScenesCnt++;
                    if (recentScenesCnt == cmd.cmdId - FILE_OPERATION_OPEN_RECENT_SCENE0_PHASE2_FOCMD + 1)
                        break;
                }

                if (VFile::doesFileExist(filenameAndPath.c_str()))
                {
                    GuiApp::setDefaultMouseMode();
                    // App::worldContainer->createNewWorld();
                    createNewScene(true);
                    std::string infoStr;
                    std::string errorStr;
                    if (loadScene(filenameAndPath.c_str(), true, nullptr, &infoStr, &errorStr))
                    {
                        if (infoStr.size() != 0)
                            App::logMsg(sim_verbosity_msgs, infoStr.c_str());
                        App::logMsg(sim_verbosity_msgs, IDSNS_SCENE_OPENED);
                        _addToRecentlyOpenedScenes(filenameAndPath);
                    }
                    else
                    {
                        if (infoStr.size() != 0)
                            App::logMsg(sim_verbosity_msgs, infoStr.c_str());
                        App::logMsg(sim_verbosity_errors, IDSNS_SCENE_COULD_NOT_BE_OPENED);
                        _removeFromRecentlyOpenedScenes(filenameAndPath);
                    }
                    App::currentWorld->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
                }
                else
                { // file does not exist anymore
                    GuiApp::uiThread->messageBox_information(GuiApp::mainWindow, "Open Recent Scene",
                                                             "File does not exist anymore.", VMESSAGEBOX_OKELI,
                                                             VMESSAGEBOX_REPLY_OK);
                    _removeFromRecentlyOpenedScenes(filenameAndPath);
                }
            }
            else
                App::appendSimulationThreadCommand(
                    cmd); // We are in the UI thread. Execute the command via the main thread
        }
        return (true);
    }
    //-----------

    if (cmd.cmdId == FILE_OPERATION_LOAD_MODEL_FOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            CSimFlavor::run(2);
            std::string ext[4];
            for (int i = 0; i < 4; i++)
                ext[i] = CSimFlavor::getStringVal_int(2, i);
            std::string filenameAndPath = GuiApp::uiThread->getOpenFileName(
                GuiApp::mainWindow, 0, IDSN_LOADING_MODEL, App::folders->getModelsPath().c_str(), "", false, "Models",
                ext[0].c_str(), ext[1].c_str(), ext[2].c_str(), ext[3].c_str());

            if (filenameAndPath.length() != 0)
            {
                std::string infoStr;
                std::string errorStr;
                if (loadModel(filenameAndPath.c_str(), true, true, nullptr, false, false, &infoStr,
                              &errorStr)) // Undo things is in here.
                {
                    GuiApp::setRebuildHierarchyFlag();
                    if (infoStr.size() > 0)
                        App::logMsg(sim_verbosity_msgs, infoStr.c_str());
                    App::logMsg(sim_verbosity_msgs, IDSNS_MODEL_LOADED);
                }
                else
                {
                    if (infoStr.size() > 0)
                        App::logMsg(sim_verbosity_msgs, infoStr.c_str());
                    App::logMsg(sim_verbosity_errors, errorStr.c_str());
                }
            }
            else
                App::logMsg(sim_verbosity_msgs, "Aborted.");
        }
        else
            App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        return (true);
    }
    if (cmd.cmdId == FILE_OPERATION_SAVE_SCENE_FOCMD)
    {
        if (App::currentWorld->simulation->isSimulationStopped() && (GuiApp::getEditModeType() == NO_EDIT_MODE))
        { // execute the command only when simulation is not running and not in an edit mode
            if (!VThread::isUiThread())
            {                                        // we are NOT in the UI thread. We execute the command now:
                _saveSceneWithDialogAndEverything(); // will call save as if needed!
            }
            else
                App::appendSimulationThreadCommand(
                    cmd); // We are in the UI thread. Execute the command via the main thread
        }
        return (true);
    }
    if ((cmd.cmdId == FILE_OPERATION_SAVE_SCENE_AS_CSIM_FOCMD) ||
        (cmd.cmdId == FILE_OPERATION_SAVE_SCENE_AS_EXXML_FOCMD) ||
        (cmd.cmdId == FILE_OPERATION_SAVE_SCENE_AS_SIMPLEXML_FOCMD))
    {
        if (App::currentWorld->simulation->isSimulationStopped() && (GuiApp::getEditModeType() == NO_EDIT_MODE))
        { // execute the command only when simulation is not running and not in an edit mode
            if (!VThread::isUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                int filetype = CSer::filetype_unspecified_file;
                if (cmd.cmdId == FILE_OPERATION_SAVE_SCENE_AS_CSIM_FOCMD)
                    filetype = CSer::filetype_csim_bin_scene_file;
                if (cmd.cmdId == FILE_OPERATION_SAVE_SCENE_AS_EXXML_FOCMD)
                    filetype = CSer::filetype_csim_xml_xscene_file;
                if (cmd.cmdId == FILE_OPERATION_SAVE_SCENE_AS_SIMPLEXML_FOCMD)
                    filetype = CSer::filetype_csim_xml_simplescene_file;
                _saveSceneAsWithDialogAndEverything(filetype);
            }
            else
                App::appendSimulationThreadCommand(
                    cmd); // We are in the UI thread. Execute the command via the main thread
        }
        return (true);
    }
    if ((cmd.cmdId == FILE_OPERATION_SAVE_MODEL_AS_CSIM_FOCMD) ||
        (cmd.cmdId == FILE_OPERATION_SAVE_MODEL_AS_EXXML_FOCMD))
    {
        if (App::currentWorld->simulation->isSimulationStopped() && (GuiApp::getEditModeType() == NO_EDIT_MODE))
        { // execute the command only when simulation is not running and not in an edit mode
            if (!VThread::isUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                std::vector<int> sel;
                for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount(); i++)
                    sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
                if (!App::currentWorld->environment->getSceneLocked())
                {
                    std::string infoM(IDSNS_SAVING_MODEL);
                    infoM += "...";
                    App::logMsg(sim_verbosity_msgs, infoM.c_str());
                    if (sel.size() != 0)
                    {
                        int modelBase = App::currentWorld->sceneObjects->getLastSelectionHandle();

                        // Display a warning if needed
                        CPersistentDataContainer cont;
                        std::string val;
                        cont.readData("SIMSETTINGS_MODEL_SAVE_OFFSET_WARNING", val);
                        int intVal = 0;
                        tt::getValidInt(val.c_str(), intVal);
                        if (intVal < 1)
                        {
                            if (GuiApp::uiThread->messageBox_checkbox(GuiApp::mainWindow, IDSN_MODEL,
                                                                      IDSN_MODEL_SAVE_POSITION_OFFSET_INFO,
                                                                      IDSN_DO_NOT_SHOW_THIS_MESSAGE_AGAIN, false))
                            {
                                intVal++;
                                val = utils::getIntString(false, intVal);
                                cont.writeData("SIMSETTINGS_MODEL_SAVE_OFFSET_WARNING", val,
                                               !App::userSettings->doNotWritePersistentData);
                            }
                        }

                        bool keepCurrentThumbnail = false;
                        bool operationCancelled = false;
                        int ft = -1;
                        if (cmd.cmdId == FILE_OPERATION_SAVE_MODEL_AS_CSIM_FOCMD)
                            ft = 0;
                        if (cmd.cmdId == FILE_OPERATION_SAVE_MODEL_AS_EXXML_FOCMD)
                            ft = 2;
                        while (true)
                        {
                            if (App::currentWorld->environment->modelThumbnail_notSerializedHere.hasImage())
                            { // we already have a thumbnail!
                                SUIThreadCommand cmdIn;
                                SUIThreadCommand cmdOut;
                                cmdIn.cmdId = KEEP_THUMBNAIL_QUESTION_DLG_UITHREADCMD;
                                {
                                    SIM_THREAD_INDICATE_UI_THREAD_CAN_DO_ANYTHING;
                                    GuiApp::uiThread->executeCommandViaUiThread(&cmdIn, &cmdOut);
                                }
                                if (cmdOut.boolParams.size() > 0)
                                    keepCurrentThumbnail = cmdOut.boolParams[0];
                            }
                            if (!keepCurrentThumbnail)
                            {
                                SUIThreadCommand cmdIn;
                                SUIThreadCommand cmdOut;
                                cmdIn.intParams.push_back(modelBase);
                                cmdIn.cmdId = SELECT_THUMBNAIL_DLG_UITHREADCMD;
                                {
                                    SIM_THREAD_INDICATE_UI_THREAD_CAN_DO_ANYTHING;
                                    GuiApp::uiThread->executeCommandViaUiThread(&cmdIn, &cmdOut);
                                }
                                if (cmdOut.boolParams.size() > 0)
                                {
                                    if (!cmdOut.boolParams[0])
                                    {
                                        if (cmdOut.boolParams[1])
                                            break;
                                    }
                                    else
                                    {
                                        operationCancelled = true;
                                        break;
                                    }
                                }
                                else
                                {
                                    operationCancelled = true;
                                    break;
                                }
                            }
                            else
                                break;
                        }
                        if (!operationCancelled)
                        {
                            std::string filenameAndPath;
                            if (ft == 0)
                                filenameAndPath = GuiApp::uiThread->getSaveFileName(
                                    GuiApp::mainWindow, 0, IDS_SAVING_MODEL___, App::folders->getModelsPath().c_str(),
                                    "", false, "CoppeliaSim Model", SIM_MODEL_EXTENSION);
                            if (ft == 2)
                                filenameAndPath = GuiApp::uiThread->getSaveFileName(
                                    GuiApp::mainWindow, 0, IDS_SAVING_MODEL___, App::folders->getModelsPath().c_str(),
                                    "", false, "CoppeliaSim XML Model (exhaustive)", SIM_XML_MODEL_EXTENSION);
                            if (ft == 3)
                                filenameAndPath = GuiApp::uiThread->getSaveFileName(
                                    GuiApp::mainWindow, 0, IDS_SAVING_MODEL___, App::folders->getModelsPath().c_str(),
                                    "", false, "CoppeliaSim XML Model (simple)", SIM_XML_MODEL_EXTENSION);

                            if (filenameAndPath.length() != 0)
                            {

                                bool abort = false;
                                if (!App::userSettings->suppressXmlOverwriteMsg)
                                {
                                    if ((CSer::getFileTypeFromName(filenameAndPath.c_str()) ==
                                         CSer::filetype_csim_xml_xmodel_file) &&
                                        (App::userSettings->xmlExportSplitSize != 0))
                                    {
                                        std::string prefix(VVarious::splitPath_fileBase(filenameAndPath.c_str()) + "_");
                                        int cnt0 = VFileFinder::countFiles(
                                            VVarious::splitPath_path(filenameAndPath.c_str()).c_str());
                                        int cnt1 = VFileFinder::countFolders(
                                            VVarious::splitPath_path(filenameAndPath.c_str()).c_str());
                                        int cnt2 = VFileFinder::countFilesWithPrefix(
                                            VVarious::splitPath_path(filenameAndPath.c_str()).c_str(), prefix.c_str());
                                        int cnt3 = VFileFinder::countFilesWithPrefix(
                                            VVarious::splitPath_path(filenameAndPath.c_str()).c_str(),
                                            (VVarious::splitPath_fileBase(filenameAndPath.c_str()) + ".").c_str());
                                        if ((cnt2 + cnt3 != cnt0) || (cnt1 != 0))
                                        {
                                            std::string msg("The scene/model will possibly be saved as several "
                                                            "separate files, all with the '");
                                            msg += prefix;
                                            msg += "' prefix. Existing files with the same prefix will be erased or "
                                                   "overwritten. To avoid this, it is recommended to either save XML "
                                                   "scenes/models in individual folders, or to set the "
                                                   "'xmlExportSplitSize' variable in ";
                                            msg += App::folders->getUserSettingsPath() +
                                                   "/usrset.txt to 0 to generate a single file.\n(this warning can be "
                                                   "disabled via the 'suppressXmlOverwriteMsg' variable in "
                                                   "usrset.txt)\n\nProceed anyway?";
                                            abort = (VMESSAGEBOX_REPLY_NO == GuiApp::uiThread->messageBox_warning(
                                                                                 GuiApp::mainWindow, IDSN_SAVE,
                                                                                 msg.c_str(), VMESSAGEBOX_YES_NO,
                                                                                 VMESSAGEBOX_REPLY_YES));
                                        }
                                    }
                                    if (CSer::getFileTypeFromName(filenameAndPath.c_str()) ==
                                        CSer::filetype_csim_xml_simplemodel_file)
                                    {
                                        std::string prefix(VVarious::splitPath_fileBase(filenameAndPath.c_str()) + "_");
                                        int cnt0 = VFileFinder::countFiles(
                                            VVarious::splitPath_path(filenameAndPath.c_str()).c_str());
                                        int cnt1 = VFileFinder::countFolders(
                                            VVarious::splitPath_path(filenameAndPath.c_str()).c_str());
                                        int cnt2 = VFileFinder::countFilesWithPrefix(
                                            VVarious::splitPath_path(filenameAndPath.c_str()).c_str(), prefix.c_str());
                                        int cnt3 = VFileFinder::countFilesWithPrefix(
                                            VVarious::splitPath_path(filenameAndPath.c_str()).c_str(),
                                            (VVarious::splitPath_fileBase(filenameAndPath.c_str()) + ".").c_str());
                                        if ((cnt2 + cnt3 != cnt0) || (cnt1 != 0))
                                        {
                                            std::string msg("The scene/model will possibly be saved as several "
                                                            "separate files, all with the '");
                                            msg += prefix;
                                            msg += "' prefix. Existing files with the same prefix will be erased or "
                                                   "overwritten. To avoid this, it is recommended to save XML "
                                                   "scenes/models in individual folders.\n(this warning can be "
                                                   "disabled via the 'suppressXmlOverwriteMsg' variable in ";
                                            msg +=
                                                App::folders->getUserSettingsPath() + "/usrset.txt)\n\nProceed anyway?";
                                            abort = (VMESSAGEBOX_REPLY_NO == GuiApp::uiThread->messageBox_warning(
                                                                                 GuiApp::mainWindow, IDSN_SAVE,
                                                                                 msg.c_str(), VMESSAGEBOX_YES_NO,
                                                                                 VMESSAGEBOX_REPLY_YES));
                                        }
                                    }
                                }
                                if (!abort)
                                {
                                    std::string infoStr;
                                    std::string errorStr;
                                    if (saveModel(modelBase, filenameAndPath.c_str(), true, nullptr, &infoStr,
                                                  &errorStr))
                                    {
                                        if (infoStr.size() > 0)
                                            App::logMsg(sim_verbosity_msgs, infoStr.c_str());
                                        App::logMsg(sim_verbosity_msgs, IDSNS_MODEL_WAS_SAVED);
                                    }
                                    else
                                        App::logMsg(sim_verbosity_errors, errorStr.c_str());
                                    App::currentWorld->sceneObjects->deselectObjects();
                                }
                                else
                                    App::logMsg(sim_verbosity_msgs, "Aborted.");
                            }
                            else
                                App::logMsg(sim_verbosity_msgs, "Aborted.");
                        }
                        else
                            App::logMsg(sim_verbosity_msgs, "Aborted.");
                    }
                    else
                        App::logMsg(sim_verbosity_errors, "Cannot proceed, selection is empty.");
                }
                else
                    GuiApp::uiThread->messageBox_warning(GuiApp::mainWindow, IDSN_MODEL, IDS_SCENE_IS_LOCKED_WARNING,
                                                         VMESSAGEBOX_OKELI, VMESSAGEBOX_REPLY_OK);
            }
            else
                App::appendSimulationThreadCommand(
                    cmd); // We are in the UI thread. Execute the command via the main thread
        }
        return (true);
    }
    if (cmd.cmdId == FILE_OPERATION_IMPORT_MESH_FOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (App::worldContainer->pluginContainer->isAssimpPluginAvailable())
            {
                App::logMsg(sim_verbosity_msgs, "Importing mesh...");
                App::currentWorld->sceneObjects->deselectObjects();
                std::string tst(App::folders->getImportExportPath());

                std::vector<std::string> filenamesAndPaths;
                if (GuiApp::uiThread->getOpenFileNames(filenamesAndPaths, GuiApp::mainWindow, 0, "Importing mesh...",
                                                       tst.c_str(), "", false, "Mesh files", "obj", "dxf", "ply", "stl",
                                                       "dae"))
                {
                    std::string files;
                    for (size_t i = 0; i < filenamesAndPaths.size(); i++)
                    {
                        if (i != 0)
                            files += ";";
                        else
                            App::folders->setImportExportPath(
                                App::folders->getPathFromFull(filenamesAndPaths[i].c_str()).c_str());
                        files += filenamesAndPaths[i];
                    }
                    CInterfaceStack *stack = App::worldContainer->interfaceStackContainer->createStack();
                    stack->pushStringOntoStack(files.c_str(), 0);
                    App::worldContainer->sandboxScript->executeScriptString("simAssimp=require('simAssimp')", nullptr);
                    App::worldContainer->sandboxScript->callCustomScriptFunction("simAssimp.importShapesDlg", stack);
                    App::worldContainer->interfaceStackContainer->destroyStack(stack);
                }
                else
                    App::logMsg(sim_verbosity_msgs, "Aborted.");
            }
            else
                GuiApp::uiThread->messageBox_critical(GuiApp::mainWindow, IDSN_EXPORT,
                                                      "Assimp plugin was not found, cannot import", VMESSAGEBOX_OKELI,
                                                      VMESSAGEBOX_REPLY_OK);
        }
        else
            App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        return (true);
    }

    if (cmd.cmdId == FILE_OPERATION_IMPORT_HEIGHTFIELD_FOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs, IDSNS_IMPORTING_HEIGHTFIELD_SHAPE);
            App::currentWorld->sceneObjects->deselectObjects();
            std::string tst(App::folders->getImportExportPath());
            std::string filenameAndPath = GuiApp::uiThread->getOpenFileName(
                GuiApp::mainWindow, 0, IDS_IMPORTING_HEIGHTFIELD___, tst.c_str(), "", true, "Image, CSV and TXT files",
                "tga", "jpg", "jpeg", "png", "gif", "bmp", "tiff", "csv", "txt");

            if (filenameAndPath.length() != 0)
            {
                App::folders->setImportExportPath(App::folders->getPathFromFull(filenameAndPath.c_str()).c_str());
                if (VFile::doesFileExist(filenameAndPath.c_str()))
                {
                    if (_heightfieldImportRoutine(filenameAndPath.c_str()))
                        App::logMsg(sim_verbosity_msgs, "done.");
                    else
                        App::logMsg(sim_verbosity_errors, "An error occurred during the import operation.");
                    App::undoRedo_sceneChanged("");
                }
                else
                    App::logMsg(sim_verbosity_errors, "Aborted (file does not exist).");
            }
            else
                App::logMsg(sim_verbosity_msgs, "Aborted.");
        }
        else
            App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        return (true);
    }

    if (cmd.cmdId == FILE_OPERATION_EXPORT_SHAPE_FOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (App::worldContainer->pluginContainer->isAssimpPluginAvailable())
            {
                std::vector<int> sel;
                App::currentWorld->sceneObjects->getSelectedObjectHandles(sel, sim_object_shape_type, true, true);
                if (!App::currentWorld->environment->getSceneLocked())
                {
                    App::logMsg(sim_verbosity_msgs, IDSNS_EXPORTING_SHAPES);
                    if (sel.size() == 0)
                        return (true); // Selection contains nothing that can be exported!
                    std::string tst(App::folders->getImportExportPath());
                    std::string filenameAndPath =
                        GuiApp::uiThread->getSaveFileName(GuiApp::mainWindow, 0, IDSNS_EXPORTING_SHAPES, tst.c_str(),
                                                          "", false, "Mesh files", "obj", "ply", "stl", "dae");
                    if (filenameAndPath.length() != 0)
                    {
                        App::folders->setImportExportPath(
                            App::folders->getPathFromFull(filenameAndPath.c_str()).c_str());

                        CInterfaceStack *stack = App::worldContainer->interfaceStackContainer->createStack();
                        stack->pushStringOntoStack(filenameAndPath.c_str(), 0);
                        stack->pushTableOntoStack();
                        int cnt = 1;
                        for (size_t i = 0; i < sel.size(); i++)
                        {
                            CShape *shape = App::currentWorld->sceneObjects->getShapeFromHandle(sel[i]);
                            if (shape != nullptr)
                            {
                                stack->pushInt32OntoStack(cnt++); // key or index
                                stack->pushInt32OntoStack(sel[i]);
                                stack->insertDataIntoStackTable();
                            }
                        }
                        App::worldContainer->sandboxScript->executeScriptString("simAssimp=require('simAssimp')",
                                                                                nullptr);
                        App::worldContainer->sandboxScript->callCustomScriptFunction("simAssimp.exportShapesDlg",
                                                                                     stack);
                        App::worldContainer->interfaceStackContainer->destroyStack(stack);
                    }
                    else
                        App::logMsg(sim_verbosity_msgs, "Aborted.");
                }
                else
                    GuiApp::uiThread->messageBox_warning(GuiApp::mainWindow, IDSN_EXPORT, IDS_SCENE_IS_LOCKED_WARNING,
                                                         VMESSAGEBOX_OKELI, VMESSAGEBOX_REPLY_OK);
            }
            else
                GuiApp::uiThread->messageBox_critical(GuiApp::mainWindow, IDSN_EXPORT,
                                                      "Assimp plugin was not found, cannot export", VMESSAGEBOX_OKELI,
                                                      VMESSAGEBOX_REPLY_OK);
        }
        else
            App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        return (true);
    }
    if (cmd.cmdId == FILE_OPERATION_EXPORT_GRAPHS_FOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            App::currentWorld->sceneObjects->getSelectedObjectHandles(sel, sim_object_graph_type, true, false);
            App::logMsg(sim_verbosity_msgs, IDSNS_EXPORTING_GRAPH_DATA);
            App::currentWorld->simulation->stopSimulation();
            if (sel.size() != 0)
            {
                std::string tst(App::folders->getOtherFilesPath());
                std::string filenameAndPath = GuiApp::uiThread->getSaveFileName(
                    GuiApp::mainWindow, 0, IDS_SAVING_GRAPHS___, tst.c_str(), "", false, "CSV Files", "csv");
                if (filenameAndPath.length() != 0)
                {
                    VFile myFile(filenameAndPath.c_str(), VFile::CREATE_WRITE | VFile::SHARE_EXCLUSIVE);
                    VArchive ar(&myFile, VArchive::STORE);
                    App::folders->setOtherFilesPath(App::folders->getPathFromFull(filenameAndPath.c_str()).c_str());
                    for (size_t i = 0; i < sel.size(); i++)
                    {
                        CGraph *it = App::currentWorld->sceneObjects->getGraphFromHandle(sel[i]);
                        if (it != nullptr)
                            it->exportGraphData(ar);
                    }
                    ar.close();
                    myFile.close();
                    App::logMsg(sim_verbosity_msgs, "done.");
                }
                else
                    App::logMsg(sim_verbosity_msgs, "Aborted.");
            }
        }
        else
            App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        return (true);
    }

    if (cmd.cmdId == FILE_OPERATION_EXPORT_DYNAMIC_CONTENT_FOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (!App::currentWorld->environment->getSceneLocked())
            {
                App::logMsg(sim_verbosity_msgs, IDSNS_EXPORTING_DYNAMIC_CONTENT);
                if (App::worldContainer->pluginContainer->dyn_isDynamicContentAvailable() != 0)
                {
                    int eng = App::currentWorld->dynamicsContainer->getDynamicEngineType(nullptr);
                    if (eng == sim_physics_ode)
                    {
                        std::string tst(App::folders->getOtherFilesPath());
                        std::string filenameAndPath = GuiApp::uiThread->getSaveFileName(
                            GuiApp::mainWindow, 0, IDS_EXPORTING_DYNAMIC_CONTENT___, tst.c_str(), "", false,
                            "ODE Dynamics World Files", "ode");
                        if (filenameAndPath.length() != 0)
                        {
                            App::folders->setOtherFilesPath(
                                App::folders->getPathFromFull(filenameAndPath.c_str()).c_str());
                            App::worldContainer->pluginContainer->dyn_serializeDynamicContent(filenameAndPath.c_str(),
                                                                                              0);
                            App::logMsg(sim_verbosity_msgs, "done.");
                        }
                        else
                            App::logMsg(sim_verbosity_msgs, "Aborted.");
                    }
                    if (eng == sim_physics_bullet)
                    {
                        std::string tst(App::folders->getOtherFilesPath());
                        std::string filenameAndPath = GuiApp::uiThread->getSaveFileName(
                            GuiApp::mainWindow, 0, IDS_EXPORTING_DYNAMIC_CONTENT___, tst.c_str(), "", false,
                            "Bullet Dynamics World Files", "bullet");
                        if (filenameAndPath.length() != 0)
                        {
                            App::folders->setOtherFilesPath(
                                App::folders->getPathFromFull(filenameAndPath.c_str()).c_str());
                            App::worldContainer->pluginContainer->dyn_serializeDynamicContent(filenameAndPath.c_str(),
                                                                                              10000000);
                            App::logMsg(sim_verbosity_msgs, "done.");
                        }
                        else
                            App::logMsg(sim_verbosity_msgs, "Aborted.");
                    }
                    if (eng == sim_physics_vortex)
                    {
                        std::string tst(App::folders->getOtherFilesPath());
                        std::string filenameAndPath = GuiApp::uiThread->getSaveFileName(
                            GuiApp::mainWindow, 0, IDS_EXPORTING_DYNAMIC_CONTENT___, tst.c_str(), "", false,
                            "Vortex Dynamics World Files", "vortex");
                        if (filenameAndPath.length() != 0)
                        {
                            App::folders->setOtherFilesPath(
                                App::folders->getPathFromFull(filenameAndPath.c_str()).c_str());
                            App::worldContainer->pluginContainer->dyn_serializeDynamicContent(filenameAndPath.c_str(),
                                                                                              10000000);
                            App::logMsg(sim_verbosity_msgs, "done.");
                        }
                        else
                            App::logMsg(sim_verbosity_msgs, "Aborted.");
                    }
                    if (eng == sim_physics_newton)
                    {
                        std::string tst(App::folders->getOtherFilesPath());
                        std::string filenameAndPath = GuiApp::uiThread->getSaveFileName(
                            GuiApp::mainWindow, 0, IDS_EXPORTING_DYNAMIC_CONTENT___, tst.c_str(), "", false,
                            "Newton Dynamics World Files", "newton");
                        if (filenameAndPath.length() != 0)
                        {
                            App::folders->setOtherFilesPath(
                                App::folders->getPathFromFull(filenameAndPath.c_str()).c_str());
                            App::worldContainer->pluginContainer->dyn_serializeDynamicContent(filenameAndPath.c_str(),
                                                                                              10000000);
                            App::logMsg(sim_verbosity_msgs, "done.");
                        }
                        else
                            App::logMsg(sim_verbosity_msgs, "Aborted.");
                    }
                }
                else
                    App::logMsg(sim_verbosity_errors, "Cannot proceed, no dynamic content available!");
            }
            else
                GuiApp::uiThread->messageBox_warning(GuiApp::mainWindow, IDSN_EXPORT, IDS_SCENE_IS_LOCKED_WARNING,
                                                     VMESSAGEBOX_OKELI, VMESSAGEBOX_REPLY_OK);
        }
        else
            App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        return (true);
    }

    if (cmd.cmdId == FILE_OPERATION_RG)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread
            SUIThreadCommand cmdIn;
            SUIThreadCommand cmdOut;
            cmdIn.cmdId = RG_UITHREADCMD;
            GuiApp::uiThread->executeCommandViaUiThread(&cmdIn, &cmdOut);
        }
        return (true);
    }
#endif

    if (cmd.cmdId == FILE_OPERATION_EXIT_SIMULATOR_FOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            bool displayed = false;
#ifdef SIM_WITH_GUI
            int ci = -1;
            int si = -1;
            int ei = -1;
            if (GuiApp::mainWindow != nullptr)
                GuiApp::mainWindow->simulationRecorder->stopRecording(false);

            ci = App::worldContainer->getInstanceIndexOfASceneNotYetSaved(
                App::currentWorld->environment->getSceneLocked());
            if (!App::currentWorld->simulation->isSimulationStopped())
                si = App::worldContainer->getCurrentWorldIndex();
            if (GuiApp::getEditModeType() != NO_EDIT_MODE)
                ei = App::worldContainer->getCurrentWorldIndex();

            if (!App::currentWorld->environment->getSceneLocked())
            {
                if (App::currentWorld->undoBufferContainer->isSceneSaveMaybeNeededFlagSet())
                    ci = App::worldContainer->getCurrentWorldIndex();
            }
            if ((ei == App::worldContainer->getCurrentWorldIndex()) && (!displayed))
            {
                if (VMESSAGEBOX_REPLY_OK == GuiApp::uiThread->messageBox_warning(
                                                GuiApp::mainWindow, IDSN_EXIT, IDS_INSTANCE_STILL_IN_EDIT_MODE_MESSAGE,
                                                VMESSAGEBOX_OK_CANCEL, VMESSAGEBOX_REPLY_OK))
                {
                    if (GuiApp::mainWindow != nullptr)
                        GuiApp::mainWindow->editModeContainer->processCommand(
                            ANY_EDIT_MODE_FINISH_AND_CANCEL_CHANGES_EMCMD, nullptr);
                    ei = -1;
                }
                else
                    displayed = true;
            }
            if ((si == App::worldContainer->getCurrentWorldIndex()) && (!displayed))
            {
                if (VMESSAGEBOX_REPLY_OK == GuiApp::uiThread->messageBox_warning(
                                                GuiApp::mainWindow, IDSN_EXIT, IDS_SIMULATION_STILL_RUNNING_MESSAGE,
                                                VMESSAGEBOX_OK_CANCEL, VMESSAGEBOX_REPLY_OK))
                    App::worldContainer->simulatorMessageQueue->addCommand(sim_message_simulation_stop_request, 0, 0, 0,
                                                                           0, nullptr, 0);
                displayed = true;
            }
            if ((ci == App::worldContainer->getCurrentWorldIndex()) && (!displayed))
            {
                unsigned short action = VMESSAGEBOX_REPLY_NO;
                if (CSimFlavor::getBoolVal(16))
                    action = GuiApp::uiThread->messageBox_warning(GuiApp::mainWindow, IDSN_SAVE,
                                                                  IDS_WANNA_SAVE_THE_SCENE_WARNING,
                                                                  VMESSAGEBOX_YES_NO_CANCEL, VMESSAGEBOX_REPLY_NO);
                if (action == VMESSAGEBOX_REPLY_YES)
                {
                    if (_saveSceneWithDialogAndEverything()) // will call save as if needed!
                        action = VMESSAGEBOX_REPLY_NO;
                }
                if (action == VMESSAGEBOX_REPLY_NO)
                {
                    App::currentWorld->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
                    ci = App::worldContainer->getInstanceIndexOfASceneNotYetSaved(false);
                }
                else
                    displayed = true;
            }
            if ((ei != -1) && (!displayed))
            {
                GuiApp::uiThread->messageBox_warning(GuiApp::mainWindow, IDSN_EXIT,
                                                     IDS_ANOTHER_INSTANCE_STILL_IN_EDIT_MODE_MESSAGE, VMESSAGEBOX_OKELI,
                                                     VMESSAGEBOX_REPLY_OK);
                App::worldContainer->switchToWorld(ei);
                displayed = true;
            }
            if ((si != -1) && (!displayed))
            {
                GuiApp::uiThread->messageBox_warning(GuiApp::mainWindow, IDSN_EXIT,
                                                     IDS_ANOTHER_SIMULATION_STILL_RUNNING_MESSAGE, VMESSAGEBOX_OKELI,
                                                     VMESSAGEBOX_REPLY_OK);
                App::worldContainer->switchToWorld(si);
                displayed = true;
            }
            if ((ci != -1) && (!displayed))
            {
                if (CSimFlavor::getBoolVal(16))
                {
                    if (VMESSAGEBOX_REPLY_CANCEL == GuiApp::uiThread->messageBox_warning(
                                                        GuiApp::mainWindow, IDSN_SAVE,
                                                        IDS_ANOTHER_INSTANCE_STILL_NOT_SAVED_WANNA_LEAVE_ANYWAY_MESSAGE,
                                                        VMESSAGEBOX_OK_CANCEL, VMESSAGEBOX_REPLY_OK))
                    {
                        App::worldContainer->switchToWorld(ci);
                        displayed = true;
                    }
                }
            }
#endif
            if (!displayed)
            {
                SSimulationThreadCommand cmd;
                cmd.cmdId = EXIT_REQUEST_CMD;
                App::appendSimulationThreadCommand(cmd);
            }
        }
        else
            App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread

        return (true);
    }

    return (false);
}

bool CFileOperations::_heightfieldImportRoutine(const char *pathName)
{
    if (VFile::doesFileExist(pathName))
    {
        try
        {
            std::vector<std::vector<double> *> readData;
            // We read each line at a time, which gives rows:
            int minRow = -1;

            std::string ext(utils::getLowerCaseString(VVarious::splitPath_fileExtension(pathName).c_str()));
            if ((ext.compare("csv") != 0) && (ext.compare("txt") != 0))
            { // from image file
                int resX, resY, n;
                unsigned char *data = CImageLoaderSaver::load(pathName, &resX, &resY, &n, 3);
                if (data != nullptr)
                {
                    if ((resX > 1) && (resY > 1))
                    {
                        int bytesPerPixel = 0;
                        if (n == 3)
                            bytesPerPixel = 3;
                        if (n == 4)
                            bytesPerPixel = 4;
                        if (bytesPerPixel != 0)
                        {
                            for (int i = 0; i < resY; i++)
                            {
                                std::vector<double> *lineVect = new std::vector<double>;
                                for (int j = 0; j < resX; j++)
                                    lineVect->push_back(double(data[bytesPerPixel * (i * resX + j) + 0] +
                                                               data[bytesPerPixel * (i * resX + j) + 1] +
                                                               data[bytesPerPixel * (i * resX + j) + 2]) /
                                                        768.0);
                                readData.push_back(lineVect);
                            }
                            minRow = resX;
                        }
                    }
                    delete[] data;
                }
                else
                    GuiApp::uiThread->messageBox_critical(GuiApp::mainWindow, IDSN_IMPORT,
                                                          IDS_TEXTURE_FILE_COULD_NOT_BE_LOADED, VMESSAGEBOX_OKELI,
                                                          VMESSAGEBOX_REPLY_OK);
            }
            else
            { // from csv or txt file:
                VFile file(pathName, VFile::READ | VFile::SHARE_DENY_NONE);
                VArchive archive(&file, VArchive::LOAD);
                unsigned int currentPos = 0;
                std::string line;
                while (archive.readSingleLine(currentPos, line, false))
                {
                    std::string word;
                    std::vector<double> *lineVect = new std::vector<double>;
                    while (tt::extractCommaSeparatedWord(line, word))
                    {
                        tt::removeSpacesAtBeginningAndEnd(word);
                        double val;
                        if (tt::getValidFloat(word.c_str(), val))
                            lineVect->push_back(val);
                        else
                            break;
                    }
                    if (lineVect->size() != 0)
                    {
                        if ((minRow == -1) || (int(lineVect->size()) < minRow))
                            minRow = int(lineVect->size());
                        readData.push_back(lineVect);
                    }
                    else
                    {
                        delete lineVect;
                        break;
                    }
                }
                archive.close();
                file.close();
            }
            if ((readData.size() > 1) && (minRow > 1))
            {
                int xSize = minRow;
                int ySize = int(readData.size());
                double pointSpacing = 1.0;
                // Display the heightfield scaling dialog:
                SUIThreadCommand cmdIn;
                SUIThreadCommand cmdOut;
                cmdIn.cmdId = HEIGHTFIELD_DIMENSION_DLG_UITHREADCMD;
                cmdIn.floatParams.push_back(10.0);
                cmdIn.floatParams.push_back(double(ySize - 1) / double(xSize - 1));
                {
                    SIM_THREAD_INDICATE_UI_THREAD_CAN_DO_ANYTHING;
                    GuiApp::uiThread->executeCommandViaUiThread(&cmdIn, &cmdOut);
                }
                if (cmdOut.floatParams.size() == 0)
                {
                    cmdOut.floatParams.push_back(1.0);
                    cmdOut.floatParams.push_back(1.0);
                }
                pointSpacing = cmdOut.floatParams[0] / double(xSize - 1);
                for (int i = 0; i < int(readData.size()); i++)
                {
                    for (int j = 0; j < int(readData[i]->size()); j++)
                    {
                        readData[i]->at(j) *= cmdOut.floatParams[1];
                    }
                }
                int shapeHandle = createHeightfield(xSize, pointSpacing, readData, 0.0, 2);
                App::currentWorld->sceneObjects->deselectObjects();
                App::currentWorld->sceneObjects->addObjectToSelection(shapeHandle);
            }
            for (int i = 0; i < int(readData.size()); i++)
                delete readData[i];
            return (readData.size() != 0);
        }
        catch (VFILE_EXCEPTION_TYPE e)
        {
            VFile::reportAndHandleFileExceptionError(e);
            return (false);
        }
    }
    return (false);
}
#endif
