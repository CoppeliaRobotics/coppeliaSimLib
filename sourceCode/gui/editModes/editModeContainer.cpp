#include <editModeContainer.h>
#include <app.h>
#include <simStringTable.h>
#include <vMessageBox.h>

CEditModeContainer::CEditModeContainer()
{
    TRACE_INTERNAL;
    _shapeEditMode=nullptr;
    _multishapeEditMode=nullptr;
    _pathEditMode=nullptr;
    _editModeObject=-1;
    _simulationStopped=true;
    pathPointManipulation=new CPathPointManipulation_old();
}

CEditModeContainer::~CEditModeContainer()
{
    TRACE_INTERNAL;
    // Following should already be erased:
    delete _shapeEditMode;
    delete _multishapeEditMode;
    delete _pathEditMode;

    delete pathPointManipulation;
}

bool CEditModeContainer::enterEditMode(int objID,int modeType)
{
    TRACE_INTERNAL;
    App::currentWorld->sceneObjects->deselectObjects();
    if (getEditModeType()!=NO_EDIT_MODE)
        return(false);
    if (!App::currentWorld->simulation->isSimulationStopped())
        return(false);

    if (modeType&SHAPE_EDIT_MODE)
    {
        CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(objID);
        if (shape==nullptr)
            return(false);
        if (shape->isCompound())
            return(false);
    }
    if (modeType&PATH_EDIT_MODE_OLD)
    {
        CPath_old* path=App::currentWorld->sceneObjects->getPathFromHandle(objID);
        if (path==nullptr)
            return(false);
    }
    if (modeType&MULTISHAPE_EDIT_MODE)
    {
        CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(objID);
        if (shape==nullptr)
            return(false);
        if (!shape->isCompound())
            return(false);
    }

    // UI and Multishape edit modes don't support item shift/rotate:
    App::setMouseMode((App::getMouseMode()&0xfff0)|sim_navigation_camerashift);

    _editModeObject=objID;

    _editMode_hierarchyWasEnabledBeforeEditMode=App::mainWindow->oglSurface->isHierarchyEnabled();
    if (!_editMode_hierarchyWasEnabledBeforeEditMode)
    {
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=OPEN_HIERARCHY_UITHREADCMD;
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }

    if (modeType&SHAPE_EDIT_MODE)
    {
        _shapeEditMode=new CShapeEditMode(App::currentWorld->sceneObjects->getShapeFromHandle(objID),modeType,App::currentWorld->sceneObjects,App::currentWorld->textureContainer,App::uiThread,App::userSettings->identicalVerticesCheck,App::userSettings->identicalTrianglesCheck,App::userSettings->identicalVerticesTolerance);

        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=OPEN_OR_CLOSE_UITHREADCMD;
        cmdIn.intParams.push_back(OPEN_SHAPE_EDITION_DLG_CMD);
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }
    else if (modeType&PATH_EDIT_MODE_OLD)
    {
        _pathEditMode=new CPathEditMode_old(App::currentWorld->sceneObjects->getPathFromHandle(objID),App::currentWorld->sceneObjects);

        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=OPEN_OR_CLOSE_UITHREADCMD;
        cmdIn.intParams.push_back(OPEN_PATH_EDITION_DLG_CMD);
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }
    else if (modeType&MULTISHAPE_EDIT_MODE)
    {
        _multishapeEditMode=new CMultishapeEditMode(App::currentWorld->sceneObjects->getShapeFromHandle(objID));

        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=OPEN_OR_CLOSE_UITHREADCMD;
        cmdIn.intParams.push_back(OPEN_MULTISHAPE_EDITION_DLG_CMD);
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }

    App::setFullDialogRefreshFlag();

    App::mainWindow->closeTemporarilyNonEditModeDialogs();

    App::setRebuildHierarchyFlag();
    App::setResetHierarchyViewFlag();
    App::setToolbarRefreshFlag();

    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId=CREATE_DEFAULT_MENU_BAR_UITHREADCMD;
    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);

    cmdIn.cmdId=EDIT_MODE_ABOUT_TO_START_UITHREADCMD;
    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    return(true);
}

void CEditModeContainer::endEditMode(bool cancelChanges)
{
    TRACE_INTERNAL;
    if (getEditModeType()==NO_EDIT_MODE)
        return;

    int objectIDToSelect=_editModeObject;

    if (_shapeEditMode!=nullptr)
    {
        deselectEditModeBuffer();
        if (!_shapeEditMode->endEditMode(cancelChanges))
            objectIDToSelect=-1; // the shape is now empty --> was erased
        delete _shapeEditMode;
        _shapeEditMode=nullptr;

        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=OPEN_OR_CLOSE_UITHREADCMD;
        cmdIn.intParams.push_back(CLOSE_SHAPE_EDITION_DLG_CMD);
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }
    if (_pathEditMode!=nullptr)
    {
        deselectEditModeBuffer();
        _pathEditMode->endEditMode(cancelChanges);
        delete _pathEditMode;
        _pathEditMode=nullptr;

        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=OPEN_OR_CLOSE_UITHREADCMD;
        cmdIn.intParams.push_back(CLOSE_PATH_EDITION_DLG_CMD);
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }
    if (_multishapeEditMode!=nullptr)
    {
        delete _multishapeEditMode;
        _multishapeEditMode=nullptr;

        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=OPEN_OR_CLOSE_UITHREADCMD;
        cmdIn.intParams.push_back(CLOSE_MULTISHAPE_EDITION_DLG_CMD);
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }

    _editModeObject=-1;

    App::currentWorld->sceneObjects->selectObject(objectIDToSelect);

    if (!_editMode_hierarchyWasEnabledBeforeEditMode)
    {
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=CLOSE_HIERARCHY_UITHREADCMD;
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }

    App::mainWindow->reopenTemporarilyClosedNonEditModeDialogs();

    App::setRebuildHierarchyFlag();
    App::setResetHierarchyViewFlag();
    App::setToolbarRefreshFlag();
    App::setFullDialogRefreshFlag();

    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId=CREATE_DEFAULT_MENU_BAR_UITHREADCMD;
    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);

    cmdIn.cmdId=EDIT_MODE_JUST_ENDED_UITHREADCMD;
    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
}

int CEditModeContainer::getEditModeObjectID()
{
    TRACE_INTERNAL;
    return(_editModeObject);
}

int CEditModeContainer::getEditModeType()
{
    TRACE_INTERNAL;
    if (_shapeEditMode!=nullptr)
        return(_shapeEditMode->getEditModeType());
    if (_multishapeEditMode!=nullptr)
        return(MULTISHAPE_EDIT_MODE);
    if (_pathEditMode!=nullptr)
        return(PATH_EDIT_MODE_OLD);
    return(NO_EDIT_MODE);
}

void CEditModeContainer::swapShapeEditModeType(int theType)
{
    TRACE_INTERNAL;
    if (_shapeEditMode!=nullptr)
        _shapeEditMode->swapShapeEditModeType(theType);
}

int CEditModeContainer::getEditModeBufferSize()
{
    TRACE_INTERNAL;
    if (_shapeEditMode!=nullptr)
        return(_shapeEditMode->getEditModeBufferSize());
    if (_pathEditMode!=nullptr)
        return(_pathEditMode->getEditModeBufferSize());
    return(0);
}

int CEditModeContainer::getLastEditModeBufferValue()
{
    TRACE_INTERNAL;
    if (_shapeEditMode!=nullptr)
        return(_shapeEditMode->getLastEditModeBufferValue());
    if (_pathEditMode!=nullptr)
        return(_pathEditMode->getLastEditModeBufferValue());
    return(-1);
}

int CEditModeContainer::getEditModeBufferValue(int index)
{
    TRACE_INTERNAL;
    if (_shapeEditMode!=nullptr)
        return(_shapeEditMode->getEditModeBufferValue(index));
    if (_pathEditMode!=nullptr)
        return(_pathEditMode->getEditModeBufferValue(index));
    return(-1);
}

std::vector<int>* CEditModeContainer::getEditModeBuffer()
{
    TRACE_INTERNAL;
    if (_shapeEditMode!=nullptr)
        return(_shapeEditMode->getEditModeBuffer());
    if (_pathEditMode!=nullptr)
        return(_pathEditMode->getEditModeBuffer());
    return(nullptr);
}

void CEditModeContainer::deselectEditModeBuffer()
{
    TRACE_INTERNAL;
    if (_shapeEditMode!=nullptr)
        _shapeEditMode->deselectEditModeBuffer();
    if (_pathEditMode!=nullptr)
        _pathEditMode->deselectEditModeBuffer();
    if (_multishapeEditMode!=nullptr)
        _multishapeEditMode->setMultishapeGeometricComponentIndex(-1);
    pathPointManipulation->clearPathPointIndices_nonEditMode();
    App::setLightDialogRefreshFlag();
}

void CEditModeContainer::removeItemFromEditModeBuffer(int item)
{
    TRACE_INTERNAL;
    if (_shapeEditMode!=nullptr)
        _shapeEditMode->removeItemFromEditModeBuffer(item);
    if (_pathEditMode!=nullptr)
        _pathEditMode->removeItemFromEditModeBuffer(item);
    App::setLightDialogRefreshFlag();
}

void CEditModeContainer::xorAddItemToEditModeBuffer(int item,bool disableEdgeFollowing)
{
    TRACE_INTERNAL;
    if (_shapeEditMode!=nullptr)
        _shapeEditMode->xorAddItemToEditModeBuffer(item,disableEdgeFollowing);
    if (_pathEditMode!=nullptr)
        _pathEditMode->xorAddItemToEditModeBuffer(item);
    App::setLightDialogRefreshFlag();
}

void CEditModeContainer::addItemToEditModeBuffer(int item,bool disableEdgeFollowing)
{
    TRACE_INTERNAL;
    if (_shapeEditMode!=nullptr)
        _shapeEditMode->addItemToEditModeBuffer(item,disableEdgeFollowing);
    if (_pathEditMode!=nullptr)
        _pathEditMode->addItemToEditModeBuffer(item);
    App::setLightDialogRefreshFlag();
}

CShapeEditMode* CEditModeContainer::getShapeEditMode()
{
    TRACE_INTERNAL;
    return(_shapeEditMode);
}

CMultishapeEditMode* CEditModeContainer::getMultishapeEditMode()
{
    TRACE_INTERNAL;
    return(_multishapeEditMode);
}

CPathEditMode_old* CEditModeContainer::getPathEditMode()
{
    TRACE_INTERNAL;
    return(_pathEditMode);
}

CShape* CEditModeContainer::getEditModeShape()
{
    TRACE_INTERNAL;
    if (_shapeEditMode!=nullptr)
        return(_shapeEditMode->getEditModeShape());
    if (_multishapeEditMode!=nullptr)
        return(_multishapeEditMode->getEditModeMultishape());
    return(nullptr);
}

CPath_old* CEditModeContainer::getEditModePath_old()
{
    TRACE_INTERNAL;
    if (_pathEditMode!=nullptr)
        return(_pathEditMode->getEditModePath_old());
    return(nullptr);
}

CSceneObject* CEditModeContainer::getEditModeObject()
{
    TRACE_INTERNAL;
    if (_shapeEditMode!=nullptr)
        return(_shapeEditMode->getEditModeShape());
    if (_multishapeEditMode!=nullptr)
        return(_multishapeEditMode->getEditModeMultishape());
    if (_pathEditMode!=nullptr)
        return(_pathEditMode->getEditModePath_old());
    return(nullptr);
}

CPathCont_old* CEditModeContainer::getEditModePathContainer_old()
{
    TRACE_INTERNAL;
    if (_pathEditMode!=nullptr)
        return(_pathEditMode->getEditModePathContainer_old());
    return(nullptr);
}

bool CEditModeContainer::keyPress(int key)
{ // return true means the key press was ment for this object

    if (getEditModeType()==NO_EDIT_MODE)
    {
        if (pathPointManipulation->getSelectedPathPointIndicesSize_nonEditMode()==0)
            return(false);
        // The key press can only be meant for the path point manipulation mode:
        if ( (key==CTRL_V_KEY)||(key==CTRL_X_KEY)||(key==CTRL_C_KEY) )
            App::logMsg(sim_verbosity_msgs,IDSNS_OPERATION_DISABLED_WITH_INDIVIDUAL_PATH_POINTS_SELECTED);
        if (key==ESC_KEY)
            processCommand(ANY_EDIT_MODE_DESELECT_BUFFER_EMCMD,nullptr);
        return(true);
    }

    if (_shapeEditMode!=nullptr)
    { // for the shape edit mode:
        if (key==CTRL_V_KEY)
            processCommand(SHAPE_EDIT_MODE_PASTE_TRIANGLE_VERTEX_EMCMD,nullptr);
        if ((key==DELETE_KEY)||(key==BACKSPACE_KEY))
            processCommand(SHAPE_EDIT_MODE_DELETE_TRIANGLE_VERTEX_EDGE_EMCMD,nullptr);
        if (key==CTRL_X_KEY)
            processCommand(SHAPE_EDIT_MODE_TRIANGLE_VERTEX_CUT_EMCMD,nullptr);
        if (key==CTRL_C_KEY)
            processCommand(SHAPE_EDIT_MODE_TRIANGLE_VERTEX_COPY_EMCMD,nullptr);
        if (key==ESC_KEY)
            processCommand(ANY_EDIT_MODE_DESELECT_BUFFER_EMCMD,nullptr);
        if (key==CTRL_A_KEY)
            processCommand(SHAPE_EDIT_MODE_SELECT_ALL_ITEMS_EMCMD,nullptr);
    }

    if (_pathEditMode!=nullptr)
    { // for the path edit mode:
        if (key==CTRL_V_KEY)
            processCommand(PATH_EDIT_MODE_OLD_PASTE_PATH_POINT_EMCMD,nullptr);
        if ((key==DELETE_KEY)||(key==BACKSPACE_KEY))
            processCommand(PATH_EDIT_MODE_OLD_DELETE_PATH_POINT_EMCMD,nullptr);
        if (key==CTRL_X_KEY)
            processCommand(PATH_EDIT_MODE_OLD_PATH_POINT_CUT_EMCMD,nullptr);
        if (key==CTRL_C_KEY)
            processCommand(PATH_EDIT_MODE_OLD_PATH_POINT_COPY_EMCMD,nullptr);
        if (key==ESC_KEY)
            processCommand(ANY_EDIT_MODE_DESELECT_BUFFER_EMCMD,nullptr);
        if (key==CTRL_A_KEY)
            processCommand(PATH_EDIT_MODE_OLD_SELECT_ALL_PATH_POINTS_EMCMD,nullptr);
    }

    if (_multishapeEditMode!=nullptr)
    { // for the multishape edit mode:
        if (key==ESC_KEY)
            processCommand(ANY_EDIT_MODE_DESELECT_BUFFER_EMCMD,nullptr);
    }

    return(true);
}

void CEditModeContainer::addMenu(VMenu* menu,CSceneObject* viewableObject)
{
    TRACE_INTERNAL;
    if (_shapeEditMode!=nullptr)
        _shapeEditMode->addMenu(menu);
    if (_pathEditMode!=nullptr)
        _pathEditMode->addMenu(menu,viewableObject);
}

bool CEditModeContainer::processCommand(int commandID,CSceneObject* viewableObject)
{
    TRACE_INTERNAL;

    if (commandID==ANY_EDIT_MODE_DESELECT_BUFFER_EMCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            deselectEditModeBuffer();
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if ( (commandID==SHAPE_EDIT_MODE_START_EMCMD)||( (getEditModeType()==NO_EDIT_MODE)&&(commandID==SHAPE_EDIT_MODE_TOGGLE_ON_OFF_EMCMD) ) )
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            CShape* it=nullptr;
            if (App::currentWorld->sceneObjects->getSelectionCount()>=1)
                it=App::currentWorld->sceneObjects->getShapeFromHandle(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(App::currentWorld->sceneObjects->getSelectionCount()-1));
            if (it!=nullptr)
            {
                if (it->isCompound())
                { // Multishape
                    App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
                    if (enterEditMode(it->getObjectHandle(),MULTISHAPE_EDIT_MODE))
                    {
                        App::logMsg(sim_verbosity_msgs,IDSNS_STARTING_COMPOUND_SHAPE_EDIT_MODE);

                        // Frame the shape
                        CSPage* thePage=App::currentWorld->pageContainer->getPage(App::currentWorld->pageContainer->getActivePageIndex());
                        if (thePage!=nullptr)
                        {
                            CSView* theView=thePage->getView(0);
                            if (theView!=nullptr)
                            {
                                CCamera* theCamera=App::currentWorld->sceneObjects->getCameraFromHandle(theView->getLinkedObjectID());
                                if (theCamera!=nullptr)
                                {
                                    int viewSize[2];
                                    theView->getViewSize(viewSize);
                                    theCamera->frameSceneOrSelectedObjects(double(viewSize[0])/double(viewSize[1]),theView->getPerspectiveDisplay(),nullptr,true,true,0.7,nullptr);
                                    theCamera->setFogTimer(4.0);
                                }
                            }
                        }
                        App::logMsg(sim_verbosity_msgs,"done.");
                    }
                }
                else
                { // simple shape
                    bool goOn=(getEditModeType()==NO_EDIT_MODE);
                    if (goOn&&(it->getMeshWrapper()->isPure()))
                        goOn=(VMESSAGEBOX_REPLY_YES==App::uiThread->messageBox_warning(App::mainWindow,IDSN_SHAPE_EDIT_MODE,IDS_SHAPE_IS_PURE_PRIMITIVE_INFO_MESSAGE,VMESSAGEBOX_YES_NO,VMESSAGEBOX_REPLY_YES));
                    if (goOn)
                    {
                        App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
                        if (enterEditMode(it->getObjectHandle(),TRIANGLE_EDIT_MODE))
                        {
                            App::logMsg(sim_verbosity_msgs,IDSNS_STARTING_TRIANGLE_EDIT_MODE);

                            // Frame the shape
                            CSPage* thePage=App::currentWorld->pageContainer->getPage(App::currentWorld->pageContainer->getActivePageIndex());
                            if (thePage!=nullptr)
                            {
                                CSView* theView=thePage->getView(0);
                                if (theView!=nullptr)
                                {
                                    CCamera* theCamera=App::currentWorld->sceneObjects->getCameraFromHandle(theView->getLinkedObjectID());
                                    if (theCamera!=nullptr)
                                    {
                                        int viewSize[2];
                                        theView->getViewSize(viewSize);
                                        theCamera->frameSceneOrSelectedObjects(double(viewSize[0])/double(viewSize[1]),theView->getPerspectiveDisplay(),nullptr,true,true,0.7,nullptr);
                                        theCamera->setFogTimer(4.0);
                                    }
                                }
                            }
                            App::logMsg(sim_verbosity_msgs,"done.");
                        }
                    }
                }
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if ( (commandID==PATH_EDIT_MODE_OLD_START_EMCMD)||( (getEditModeType()==NO_EDIT_MODE)&&(commandID==PATH_EDIT_MODE_OLD_TOGGLE_ON_OFF_EMCMD) ) )
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            CPath_old* it=nullptr;
            if (App::currentWorld->sceneObjects->getSelectionCount()>=1)
                it=App::currentWorld->sceneObjects->getPathFromHandle(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(App::currentWorld->sceneObjects->getSelectionCount()-1));
            if (it!=nullptr)
            {
                App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
                if (enterEditMode(it->getObjectHandle(),PATH_EDIT_MODE_OLD))
                {
                    App::logMsg(sim_verbosity_msgs,IDSNS_STARTING_PATH_EDIT_MODE_OLD);
                    // Frame the path
                    CSPage* thePage=App::currentWorld->pageContainer->getPage(App::currentWorld->pageContainer->getActivePageIndex());
                    if (thePage!=nullptr)
                    {
                        CSView* theView=thePage->getView(0);
                        if (theView!=nullptr)
                        {
                            CCamera* theCamera=App::currentWorld->sceneObjects->getCameraFromHandle(theView->getLinkedObjectID());
                            if (theCamera!=nullptr)
                            {
                                int viewSize[2];
                                theView->getViewSize(viewSize);
                                theCamera->frameSceneOrSelectedObjects(double(viewSize[0])/double(viewSize[1]),theView->getPerspectiveDisplay(),nullptr,true,true,0.7,nullptr);
                                theCamera->setFogTimer(4.0);
                            }
                        }
                    }
                    App::logMsg(sim_verbosity_msgs,"done.");
                }
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (_shapeEditMode!=nullptr)
        return(_processShapeEditModeCommand(commandID));
    if (_multishapeEditMode!=nullptr)
        return(_processMultishapeEditModeCommand(commandID));
    if (_pathEditMode!=nullptr)
        return(_processPathEditModeCommand_old(commandID,viewableObject));

    return(false);
}

bool CEditModeContainer::_processShapeEditModeCommand(int commandID)
{
    TRACE_INTERNAL;
    if (commandID==SHAPE_EDIT_MODE_SWAP_TO_VERTEX_EDIT_MODE_EMCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs,IDSNS_SWAPPING_TO_VERTEX_EDIT_MODE);
            _shapeEditMode->processCommand(commandID);
            SUIThreadCommand cmdIn;
            SUIThreadCommand cmdOut;
            cmdIn.cmdId=OPEN_OR_CLOSE_UITHREADCMD;
            cmdIn.intParams.push_back(OPEN_SHAPE_EDIT_MODE_VERTEX_PART_CMD);
            App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
            App::setFullDialogRefreshFlag();
            App::logMsg(sim_verbosity_msgs,"done.");
        }
        else
        { // We are in the UI thread. We execute the command in a delayed manner:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==SHAPE_EDIT_MODE_SWAP_TO_TRIANGLE_EDIT_MODE_EMCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs,IDSNS_SWAPPING_TO_TRIANGLE_EDIT_MODE);
            _shapeEditMode->processCommand(commandID);
            SUIThreadCommand cmdIn;
            SUIThreadCommand cmdOut;
            cmdIn.cmdId=OPEN_OR_CLOSE_UITHREADCMD;
            cmdIn.intParams.push_back(OPEN_SHAPE_EDIT_MODE_TRIANGLE_PART_CMD);
            App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
            App::setFullDialogRefreshFlag();
            App::logMsg(sim_verbosity_msgs,"done.");
        }
        else
        { // We are in the UI thread. We execute the command in a delayed manner:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==SHAPE_EDIT_MODE_SWAP_TO_EDGE_EDIT_MODE_EMCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs,IDSNS_SWAPPING_TO_EDGE_EDIT_MODE);
            _shapeEditMode->processCommand(commandID);
            SUIThreadCommand cmdIn;
            SUIThreadCommand cmdOut;
            cmdIn.cmdId=OPEN_OR_CLOSE_UITHREADCMD;
            cmdIn.intParams.push_back(OPEN_SHAPE_EDIT_MODE_EDGE_PART_CMD);
            App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
            App::setFullDialogRefreshFlag();
            App::logMsg(sim_verbosity_msgs,"done.");
        }
        else
        { // We are in the UI thread. We execute the command in a delayed manner:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==SHAPE_EDIT_MODE_TRIANGLE_VERTEX_COPY_EMCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (_shapeEditMode->getEditModeType()&EDGE_EDIT_MODE)
                App::logMsg(sim_verbosity_msgs,IDSNS_EDGES_CANNOT_BE_COPIED_CUR_OR_PASTED);
            else
            {
                App::logMsg(sim_verbosity_msgs,IDSNS_COPYING_SELECTION);
                _shapeEditMode->processCommand(commandID);
                App::setLightDialogRefreshFlag();
                App::logMsg(sim_verbosity_msgs,"done.");
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==SHAPE_EDIT_MODE_PASTE_TRIANGLE_VERTEX_EMCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // We are in the main thread. We execute the command here:
            if (_shapeEditMode->getEditModeType()&EDGE_EDIT_MODE)
                App::logMsg(sim_verbosity_msgs,IDSNS_EDGES_CANNOT_BE_COPIED_CUR_OR_PASTED);
            else
            {
                App::logMsg(sim_verbosity_msgs,IDSNS_PASTING_BUFFER);
                _shapeEditMode->processCommand(commandID);
                App::setFullDialogRefreshFlag();
                App::logMsg(sim_verbosity_msgs,"done.");
            }
        }
        else
        { // We are in the UI thread. We execute the command in a delayed manner:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==SHAPE_EDIT_MODE_EMPTY_TRIANGLE_VERTEX_COPY_BUFFER_EMCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs,IDSNS_CLEARING_BUFFER);
            _shapeEditMode->processCommand(commandID);
            App::logMsg(sim_verbosity_msgs,"done.");
        }
        else
        { // We are in the UI thread. We execute the command in a delayed manner:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }


    if (commandID==SHAPE_EDIT_MODE_DELETE_TRIANGLE_VERTEX_EDGE_EMCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs,IDSNS_DELETING_SELECTION);
            _shapeEditMode->processCommand(commandID);
            App::setFullDialogRefreshFlag();
            App::logMsg(sim_verbosity_msgs,"done.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==SHAPE_EDIT_MODE_SELECT_ALL_ITEMS_EMCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (_shapeEditMode->getEditModeType()&TRIANGLE_EDIT_MODE)
                App::logMsg(sim_verbosity_msgs,IDSNS_SELECTING_ALL_TRIANGLES);
            if (_shapeEditMode->getEditModeType()&VERTEX_EDIT_MODE)
                App::logMsg(sim_verbosity_msgs,IDSNS_SELECTING_ALL_VERTICES);
            if (_shapeEditMode->getEditModeType()&EDGE_EDIT_MODE)
                App::logMsg(sim_verbosity_msgs,IDSNS_SELECTING_ALL_EDGES);
            _shapeEditMode->processCommand(commandID);
            App::setFullDialogRefreshFlag();
            App::logMsg(sim_verbosity_msgs,"done.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==SHAPE_EDIT_MODE_TOGGLE_HIDDEN_VERTICE_EDGE_SHOW_EMCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            _shapeEditMode->processCommand(commandID);
            App::setFullDialogRefreshFlag();
            App::logMsg(sim_verbosity_msgs,IDSNS_TOGGLED_HIDDEN_VERTICES_EDGES_SHOW_STATE);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==SHAPE_EDIT_MODE_TOGGLE_AUTOMATIC_EDGE_FOLLOWING_EMCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            _shapeEditMode->processCommand(commandID);
            App::setFullDialogRefreshFlag();
            App::logMsg(sim_verbosity_msgs,IDSNS_TOGGLED_AUTOMATIC_EDGE_FOLLOWING);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==SHAPE_EDIT_MODE_TRIANGLE_VERTEX_CUT_EMCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (_shapeEditMode->getEditModeType()&EDGE_EDIT_MODE)
                App::logMsg(sim_verbosity_msgs,IDSNS_EDGES_CANNOT_BE_COPIED_CUR_OR_PASTED);
            else
            {
                App::logMsg(sim_verbosity_msgs,IDSNS_CUTTING_SELECTION);
                _shapeEditMode->processCommand(commandID);
                App::setFullDialogRefreshFlag();
                App::logMsg(sim_verbosity_msgs,"done.");
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==SHAPE_EDIT_MODE_FLIP_TRIANGLES_SIDE_EMCMD)
    {
        IF_UI_EVENT_CAN_READ_DATA // no write here
        {
            if (_shapeEditMode->getEditModeType()&TRIANGLE_EDIT_MODE)
                _shapeEditMode->flipTriangles();
            deselectEditModeBuffer();
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
        return(true);
    }

    if (commandID==SHAPE_EDIT_MODE_MAKE_SHAPE_WITH_SELECTED_TRIANGLES_EMCMD)
    {
        IF_UI_EVENT_CAN_READ_DATA // no write here
        {
            _shapeEditMode->makeShape();
        }
        return(true);
    }
    if (commandID==SHAPE_EDIT_MODE_MAKE_RECTANGLE_WITH_SELECTED_TRIANGLES_EMCMD)
    {
        IF_UI_EVENT_CAN_READ_DATA // no write here
        {
            _shapeEditMode->makePrimitive(0);
        }
        return(true);
    }
    if (commandID==SHAPE_EDIT_MODE_MAKE_SPHERE_WITH_SELECTED_TRIANGLES_EMCMD)
    {
        IF_UI_EVENT_CAN_READ_DATA // no write here
        {
            _shapeEditMode->makePrimitive(1);
        }
        return(true);
    }
    if (commandID==SHAPE_EDIT_MODE_MAKE_CYLINDER_WITH_SELECTED_TRIANGLES_EMCMD)
    {
        IF_UI_EVENT_CAN_READ_DATA // no write here
        {
            _shapeEditMode->makePrimitive(3);
        }
        return(true);
    }

    if (commandID==SHAPE_EDIT_MODE_MAKE_DUMMIES_WITH_SELECTED_VERTICES_EMCMD)
    {
        IF_UI_EVENT_CAN_READ_DATA // no write here
        {
            _shapeEditMode->makeDummies();
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
        return(true);
    }

    if (commandID==SHAPE_EDIT_MODE_INSERT_TRIANGLES_EMCMD)
    {
        IF_UI_EVENT_CAN_READ_DATA // no write here!!
        {
            _shapeEditMode->insertTriangles();
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
        return(true);
    }

    if (commandID==SHAPE_EDIT_MODE_DIVIDE_LARGEST_TRIANGLES_EMCMD)
    {
        IF_UI_EVENT_CAN_READ_DATA // no write here!!
        {
            _shapeEditMode->subdivideTriangles();
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
        return(true);
    }

    if (commandID==SHAPE_EDIT_MODE_INSERT_TRIANGLE_FAN_EMCMD)
    {
        IF_UI_EVENT_CAN_READ_DATA // no write here!!
        {
            _shapeEditMode->insertTriangleFan();
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
        return(true);
    }

    if ( (commandID==ANY_EDIT_MODE_FINISH_WITH_QUESTION_DLG_EMCMD)||(commandID==ANY_EDIT_MODE_FINISH_AND_CANCEL_CHANGES_EMCMD)||(commandID==ANY_EDIT_MODE_FINISH_AND_APPLY_CHANGES_EMCMD)||(commandID==SHAPE_EDIT_MODE_TOGGLE_ON_OFF_EMCMD) )
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            unsigned short res=0;
            if ( (commandID==ANY_EDIT_MODE_FINISH_WITH_QUESTION_DLG_EMCMD)||(commandID==SHAPE_EDIT_MODE_TOGGLE_ON_OFF_EMCMD) )
                res=App::uiThread->messageBox_information(App::mainWindow,IDSN_SHAPE_EDIT_MODE,IDSN_DO_YOU_WANT_TO_APPLY_THE_CHANGES,VMESSAGEBOX_YES_NO_CANCEL,VMESSAGEBOX_REPLY_YES);
            if (commandID==ANY_EDIT_MODE_FINISH_AND_CANCEL_CHANGES_EMCMD)
                res=VMESSAGEBOX_REPLY_NO;
            if (commandID==ANY_EDIT_MODE_FINISH_AND_APPLY_CHANGES_EMCMD)
                res=VMESSAGEBOX_REPLY_YES;
            if (res!=VMESSAGEBOX_REPLY_CANCEL)
            {
                if (res==VMESSAGEBOX_REPLY_YES)
                    App::logMsg(sim_verbosity_msgs,IDSNS_ENDING_EDIT_MODE_AND_APPLYING_CHANGES);
                else
                    App::logMsg(sim_verbosity_msgs,IDSNS_ENDING_EDIT_MODE_AND_CANCELLING_CHANGES);

                endEditMode(res==VMESSAGEBOX_REPLY_NO);
                if (res==VMESSAGEBOX_REPLY_YES)
                {
                    App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
                }
                App::logMsg(sim_verbosity_msgs,"done.");
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            App::appendSimulationThreadCommand(commandID);
        }
        return(true);
    }

    return(false);
}

bool CEditModeContainer::_processMultishapeEditModeCommand(int commandID)
{
    TRACE_INTERNAL;
    if ( (commandID==ANY_EDIT_MODE_FINISH_AND_APPLY_CHANGES_EMCMD)||(commandID==ANY_EDIT_MODE_FINISH_AND_CANCEL_CHANGES_EMCMD)||(commandID==SHAPE_EDIT_MODE_TOGGLE_ON_OFF_EMCMD) )
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs,IDSNS_ENDING_EDIT_MODE_AND_APPLYING_CHANGES);
            endEditMode(false);
            App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
            App::logMsg(sim_verbosity_msgs,"done.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    return(false);
}

bool CEditModeContainer::_processPathEditModeCommand_old(int commandID,CSceneObject* viewableObject)
{
    TRACE_INTERNAL;
    if (commandID==PATH_EDIT_MODE_OLD_SELECT_ALL_PATH_POINTS_EMCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (_pathEditMode->processCommand(commandID,viewableObject))
            {
                App::logMsg(sim_verbosity_msgs,IDSNS_SELECTING_ALL_PATH_POINTS);
                App::setLightDialogRefreshFlag();
                App::logMsg(sim_verbosity_msgs,"done.");
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==PATH_EDIT_MODE_OLD_PATH_POINT_COPY_EMCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (_pathEditMode->processCommand(commandID,viewableObject))
            {
                App::logMsg(sim_verbosity_msgs,IDSNS_COPYING_SELECTION);
                App::setLightDialogRefreshFlag();
                App::logMsg(sim_verbosity_msgs,"done.");
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==PATH_EDIT_MODE_OLD_PASTE_PATH_POINT_EMCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (_pathEditMode->processCommand(commandID,viewableObject))
            {
                App::logMsg(sim_verbosity_msgs,IDSNS_PASTING_BUFFER);
                App::setFullDialogRefreshFlag();
                App::logMsg(sim_verbosity_msgs,"done.");
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==PATH_EDIT_MODE_OLD_INSERT_NEW_PATH_POINT_EMCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (_pathEditMode->processCommand(commandID,viewableObject))
            {
                App::logMsg(sim_verbosity_msgs,IDSNS_INSERTING_NEW_PATH_POINT);
                App::setFullDialogRefreshFlag();
                App::logMsg(sim_verbosity_msgs,"done.");
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==PATH_EDIT_MODE_OLD_APPEND_NEW_PATH_POINT_FROM_CAMERA_EMCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if ((viewableObject!=nullptr)&&(viewableObject->getObjectType()==sim_object_camera_type))
            {
                if (_pathEditMode->processCommand(commandID,viewableObject))
                {
                    App::logMsg(sim_verbosity_msgs,IDSNS_APPENDING_NEW_PATH_POINT_FROM_CAMERA_CONFIGURATION);
                    App::setFullDialogRefreshFlag();
                    App::logMsg(sim_verbosity_msgs,"done.");
                }
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            cmd.objectParams.push_back(viewableObject);
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==PATH_EDIT_MODE_OLD_DELETE_PATH_POINT_EMCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (_pathEditMode->processCommand(commandID,viewableObject))
            {
                App::logMsg(sim_verbosity_msgs,IDSNS_DELETING_SELECTION);
                App::setFullDialogRefreshFlag();
                App::logMsg(sim_verbosity_msgs,"done.");
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==PATH_EDIT_MODE_OLD_PATH_POINT_CUT_EMCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (_pathEditMode->processCommand(commandID,viewableObject))
            {
                App::logMsg(sim_verbosity_msgs,IDSNS_CUTTING_SELECTION);
                App::setFullDialogRefreshFlag();
                App::logMsg(sim_verbosity_msgs,"done.");
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==PATH_EDIT_MODE_OLD_KEEP_ALL_X_AND_ALIGN_Z_FOR_PATH_POINTS_EMCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (_pathEditMode->processCommand(commandID,viewableObject))
            {
                App::logMsg(sim_verbosity_msgs,IDSNS_KEEPING_CONTROL_POINT_X_AXIS_AND_ALIGNING_CONTROL_POINT_Z_AXIS_WITH_LAST_SELECTION);
                App::setLightDialogRefreshFlag();
                App::logMsg(sim_verbosity_msgs,"done.");
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==PATH_EDIT_MODE_OLD_MAKE_DUMMIES_FROM_PATH_CTRL_POINTS_EMCMD)
    {
        _pathEditMode->makeDummies();
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        return(true);
    }

    if (commandID==PATH_EDIT_MODE_OLD_INVERSE_ORDER_OF_SELECTED_PATH_CTRL_POINTS_EMCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs,IDSNS_INVERTING_CONTROL_POINT_ORDER);
            if (_pathEditMode->processCommand(commandID,viewableObject))
                App::logMsg(sim_verbosity_msgs,"done.");
            else
                App::logMsg(sim_verbosity_msgs,IDSNS_FAILED_SELECTED_CONTROL_POINTS_ARE_NOT_CONSECUTIVE);
            App::setFullDialogRefreshFlag();
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==PATH_EDIT_MODE_OLD_MAKE_PATH_FROM_BEZIER_EMCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            bool proceed=true;
            if (_pathEditMode->getBezierPathPointCount()>50)
                proceed=(VMESSAGEBOX_REPLY_YES==App::uiThread->messageBox_warning(App::mainWindow,IDSN_PATH,IDS_LARGE_QUANTITY_OF_PATH_POINTS_WARNING,VMESSAGEBOX_YES_NO,VMESSAGEBOX_REPLY_YES));
            if (_pathEditMode->getBezierPathPointCount()<2)
                proceed=false;
            if (proceed)
            {
                App::logMsg(sim_verbosity_msgs,IDSNS_GENERATING_NEW_PATH);
                _pathEditMode->processCommand(commandID,viewableObject);
                App::setFullDialogRefreshFlag();
                App::logMsg(sim_verbosity_msgs,"done.");
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if ( (commandID==ANY_EDIT_MODE_FINISH_WITH_QUESTION_DLG_EMCMD)||(commandID==ANY_EDIT_MODE_FINISH_AND_CANCEL_CHANGES_EMCMD)||(commandID==ANY_EDIT_MODE_FINISH_AND_APPLY_CHANGES_EMCMD)||(commandID==PATH_EDIT_MODE_OLD_TOGGLE_ON_OFF_EMCMD) )
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            unsigned short res=0;
            if ( (commandID==ANY_EDIT_MODE_FINISH_WITH_QUESTION_DLG_EMCMD)||(commandID==PATH_EDIT_MODE_OLD_TOGGLE_ON_OFF_EMCMD) )
                res=App::uiThread->messageBox_information(App::mainWindow,IDSN_PATH_EDIT_MODE_OLD,IDSN_DO_YOU_WANT_TO_APPLY_THE_CHANGES,VMESSAGEBOX_YES_NO_CANCEL,VMESSAGEBOX_REPLY_YES);
            if (commandID==ANY_EDIT_MODE_FINISH_AND_CANCEL_CHANGES_EMCMD)
                res=VMESSAGEBOX_REPLY_NO;
            if (commandID==ANY_EDIT_MODE_FINISH_AND_APPLY_CHANGES_EMCMD)
                res=VMESSAGEBOX_REPLY_YES;
            if (res!=VMESSAGEBOX_REPLY_CANCEL)
            {
                if (res==VMESSAGEBOX_REPLY_YES)
                    App::logMsg(sim_verbosity_msgs,IDSNS_ENDING_EDIT_MODE_AND_APPLYING_CHANGES);
                else
                    App::logMsg(sim_verbosity_msgs,IDSNS_ENDING_EDIT_MODE_AND_CANCELLING_CHANGES);
                endEditMode(res==VMESSAGEBOX_REPLY_NO);
                if (res==VMESSAGEBOX_REPLY_YES)
                {
                    App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
                }
                App::logMsg(sim_verbosity_msgs,"done.");
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    return(false);
}

void CEditModeContainer::announceObjectSelectionChanged()
{
    TRACE_INTERNAL;
    pathPointManipulation->announceObjectSelectionChanged();
}

void CEditModeContainer::announceSceneInstanceChanged()
{
    TRACE_INTERNAL;
    pathPointManipulation->announceSceneInstanceChanged();
}

void CEditModeContainer::simulationAboutToStart()
{
    TRACE_INTERNAL;
    pathPointManipulation->simulationAboutToStart();
    _simulationStopped=false;
}

void CEditModeContainer::simulationEnded()
{
    TRACE_INTERNAL;
    pathPointManipulation->simulationEnded();
    _simulationStopped=true;
}
