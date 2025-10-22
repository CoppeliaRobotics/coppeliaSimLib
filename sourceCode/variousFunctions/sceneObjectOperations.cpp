#include <simInternal.h>
#include <sceneObjectOperations.h>
#include <simulation.h>
#include <mesh.h>
#include <app.h>
#include <meshManip.h>
#include <tt.h>
#include <simStrings.h>
#include <meshRoutines.h>
#include <simFlavor.h>
#include <boost/lexical_cast.hpp>
#ifdef SIM_WITH_GUI
#include <vMessageBox.h>
#include <guiApp.h>
#endif

#ifdef SIM_WITH_GUI
void CSceneObjectOperations::keyPress(int key)
{
    if (key == CTRL_V_KEY)
        processCommand(SCENE_OBJECT_OPERATION_PASTE_OBJECTS_SOOCMD);
    if ((key == DELETE_KEY) || (key == BACKSPACE_KEY))
        processCommand(SCENE_OBJECT_OPERATION_DELETE_OBJECTS_SOOCMD);
    if (key == CTRL_X_KEY)
        processCommand(SCENE_OBJECT_OPERATION_OBJECT_FULL_CUT_SOOCMD);
    if (key == CTRL_C_KEY)
        processCommand(SCENE_OBJECT_OPERATION_OBJECT_FULL_COPY_SOOCMD);
    if (key == ESC_KEY)
        processCommand(SCENE_OBJECT_OPERATION_DESELECT_OBJECTS_SOOCMD);
    if (key == CTRL_Y_KEY)
        processCommand(SCENE_OBJECT_OPERATION_REDO_SOOCMD);
    if (key == CTRL_Z_KEY)
        processCommand(SCENE_OBJECT_OPERATION_UNDO_SOOCMD);
    if (key == CTRL_A_KEY)
        processCommand(SCENE_OBJECT_OPERATION_SELECT_ALL_OBJECTS_SOOCMD);
}

bool CSceneObjectOperations::processCommand(int commandID)
{ // Return value is true if the command belonged to object edition menu and was executed
    // Can be called by the UI and SIM thread!

    if (commandID == SCENE_OBJECT_OPERATION_ASSEMBLE_SOOCMD)
    {
        // There is another such routine!! XXBFVGA
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            size_t selS = App::currentWorld->sceneObjects->getSelectionCount();
            if (selS == 1)
            {
                if (App::disassemble(App::currentWorld->sceneObjects->getLastSelectionHandle(), false, true))
                    App::currentWorld->sceneObjects->deselectObjects();
            }
            else if (selS == 2)
            {
                int lastSel = App::currentWorld->sceneObjects->getLastSelectionHandle();
                if (App::assemble(lastSel, App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(0), false,
                                  true))
                {
                    App::currentWorld->sceneObjects->deselectObjects();
                    App::currentWorld->sceneObjects->selectObject(lastSel);
                }
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }

    if (commandID == SCENE_OBJECT_OPERATION_TRANSFER_DNA_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            size_t selS = App::currentWorld->sceneObjects->getSelectionCount();
            CSceneObject* it = App::currentWorld->sceneObjects->getLastSelectionObject();
            if ((selS == 1) && it->getModelBase())
            {
                std::vector<CSceneObject*> clones;
                std::vector<CSceneObject*> toExplore;
                for (size_t i = 0; i < App::currentWorld->sceneObjects->getOrphanCount(); i++)
                    toExplore.push_back(App::currentWorld->sceneObjects->getOrphanFromIndex(i));
                while (toExplore.size() > 0)
                {
                    CSceneObject* obj = toExplore[0];
                    toExplore.erase(toExplore.begin());
                    if (obj != it)
                    {
                        if (obj->getModelBase() && (obj->getDnaString().compare(it->getDnaString()) == 0))
                            clones.push_back(obj);
                        else
                            toExplore.insert(toExplore.end(), obj->getChildren()->begin(), obj->getChildren()->end());
                    }
                }

                std::vector<int> newSelection;
                if (clones.size() > 0)
                {
                    App::logMsg(sim_verbosity_msgs, IDSN_TRANSFERRING_DNA_TO_CLONES);
                    App::worldContainer->copyBuffer->memorizeBuffer();

                    std::vector<int> sel;
                    sel.push_back(it->getObjectHandle());
                    App::currentWorld->sceneObjects->addModelObjects(sel);
                    std::string masterName(it->getObjectName_old());

                    App::worldContainer->copyBuffer->copyCurrentSelection(sel, App::currentWorld->environment->getSceneLocked(), 0);
                    App::currentWorld->sceneObjects->deselectObjects();
                    for (size_t i = 0; i < clones.size(); i++)
                    {
                        std::string name(clones[i]->getObjectName_old());
                        std::string altName(clones[i]->getObjectAltName_old());
                        std::vector<int> objs;
                        objs.push_back(clones[i]->getObjectHandle());
                        App::currentWorld->sceneObjects->addModelObjects(objs);
                        C7Vector tr(clones[i]->getLocalTransformation());
                        CSceneObject* parent(clones[i]->getParent());
                        int order = App::currentWorld->sceneObjects->getObjectSequence(clones[i]);
                        App::currentWorld->sceneObjects->eraseObjects(&objs, true);
                        App::worldContainer->copyBuffer->pasteBuffer(App::currentWorld->environment->getSceneLocked(),
                                                                     2);
                        CSceneObject* newObj = App::currentWorld->sceneObjects->getLastSelectionObject();
                        App::currentWorld->sceneObjects->deselectObjects();
                        newSelection.push_back(newObj->getObjectHandle());
                        App::currentWorld->sceneObjects->setObjectParent(newObj, parent, true);
                        App::currentWorld->sceneObjects->setObjectSequence(newObj, order);
                        newObj->setLocalTransformation(tr);

                        std::string autoName(newObj->getObjectName_old());
                        int suffixNb = tt::getNameSuffixNumber(autoName.c_str(), true);
                        name = tt::getNameWithoutSuffixNumber(name.c_str(), true);
                        if (suffixNb >= 0)
                            name += "#" + std::to_string(suffixNb);
                        App::currentWorld->sceneObjects->setObjectName_old(newObj, name.c_str(), true);
                        App::currentWorld->sceneObjects->setObjectAltName_old(newObj, altName.c_str(), true);
                    }
                    App::worldContainer->copyBuffer->restoreBuffer();
                    App::worldContainer->copyBuffer->clearMemorizedBuffer();
                    App::logMsg(sim_verbosity_msgs, "done.");
                    std::string txt;
                    txt += boost::lexical_cast<std::string>(clones.size()) + IDSN_X_CLONES_WERE_UPDATED;
                    App::logMsg(sim_verbosity_msgs, txt.c_str());

                    for (size_t i = 0; i < newSelection.size(); i++)
                        App::currentWorld->sceneObjects->addObjectToSelection(newSelection[i]);

                    App::undoRedo_sceneChanged("");
                }
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }

    if ((commandID == SCENE_OBJECT_OPERATION_MAKE_PARENT_SOOCMD) ||
        (commandID == SCENE_OBJECT_OPERATION_MAKE_PARENT_AND_MOVE_SOOCMD))
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount(); i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            if (sel.size() > 1)
            {
                CSceneObject* last = App::currentWorld->sceneObjects->getObjectFromHandle(sel[sel.size() - 1]);
                for (size_t i = 0; i < sel.size() - 1; i++)
                {
                    CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(sel[i]);
                    App::currentWorld->sceneObjects->setObjectParent(
                        it, last, commandID == SCENE_OBJECT_OPERATION_MAKE_PARENT_SOOCMD);
                }
                App::currentWorld->sceneObjects->selectObject(last->getObjectHandle()); // We select the parent

                App::undoRedo_sceneChanged("");
                std::string txt("Setting object '");
                txt += last->getObjectAlias_printPath() + "' parent...";
                App::logMsg(sim_verbosity_msgs, txt.c_str());
                App::logMsg(sim_verbosity_msgs, "done.");
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }

    if (commandID == SCENE_OBJECT_OPERATION_MAKE_ORPHANS_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount(); i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            App::logMsg(sim_verbosity_msgs, "Setting object(s) parent-less...");
            for (size_t i = 0; i < sel.size(); i++)
            {
                CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(sel[i]);
                App::currentWorld->sceneObjects->setObjectParent(it, nullptr, true);
            }
            App::undoRedo_sceneChanged("");
            App::currentWorld->sceneObjects->deselectObjects(); // We clear selection
            App::logMsg(sim_verbosity_msgs, "done.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }

    if ( (commandID == SCENE_OBJECT_OPERATION_COMPUTE_INERTIA_SOOCMD) || (commandID == SCENE_OBJECT_OPERATION_COMPUTE_INERTIA_MODEL_SOOCMD) )
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<CSceneObject*> sel;
            if (commandID == SCENE_OBJECT_OPERATION_COMPUTE_INERTIA_SOOCMD)
                App::currentWorld->sceneObjects->getSelectedObjects(sel, sim_sceneobject_shape, false);
            else
                App::currentWorld->sceneObjects->getSelectedModels(sel, sim_sceneobject_shape, true);
            App::currentWorld->sceneObjects->deselectObjects();
            bool ok;
            double density = 0.0;
            ok = GuiApp::uiThread->dialogInputGetFloat(GuiApp::mainWindow, "Body density", "Uniform density", 1000.05,
                                                       0.1, 30000, 1, &density);
            if (ok)
            {
                App::logMsg(sim_verbosity_msgs, "Computing mass and inertia...");
                GuiApp::uiThread->showOrHideProgressBar(true, -1, "Computing mass and inertia...");
                std::vector<int> toSelect;
                for (size_t i = 0; i < sel.size(); i++)
                {
                    CShape* it = (CShape*)sel[i];
                    if (!it->getStatic())
                    {
                        toSelect.push_back(it->getObjectHandle());
                        it->computeMassAndInertia(density);
                    }
                }
                App::currentWorld->sceneObjects->setSelectedObjectHandles(toSelect.data(), toSelect.size());
                App::logMsg(sim_verbosity_msgs, "done.");
                GuiApp::uiThread->showOrHideProgressBar(false);
                App::undoRedo_sceneChanged("");
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }

    if ( (commandID == SCENE_OBJECT_OPERATION_SCALE_MASS_SOOCMD) || (commandID == SCENE_OBJECT_OPERATION_SCALE_MASS_MODEL_SOOCMD) )
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<CSceneObject*> sel;
            if (commandID == SCENE_OBJECT_OPERATION_SCALE_MASS_SOOCMD)
                App::currentWorld->sceneObjects->getSelectedObjects(sel, sim_sceneobject_shape, false);
            else
                App::currentWorld->sceneObjects->getSelectedModels(sel, sim_sceneobject_shape, true);
            App::currentWorld->sceneObjects->deselectObjects();
            bool ok;
            double fact = 0.0;
            ok = GuiApp::uiThread->dialogInputGetFloat(GuiApp::mainWindow, "Mass scaling", "Scaling factor", 2.0, 0.1,
                                                       10.0, 2, &fact);
            if (ok)
            {
                App::logMsg(sim_verbosity_msgs, "Scaling mass...");
                std::vector<int> toSelect;
                for (size_t i = 0; i < sel.size(); i++)
                {
                    CShape* it = (CShape*)sel[i];
                    if (!it->getStatic())
                    {
                        toSelect.push_back(it->getObjectHandle());
                        it->getMesh()->setMass(it->getMesh()->getMass() * fact);
                    }
                }
                App::currentWorld->sceneObjects->setSelectedObjectHandles(toSelect.data(), toSelect.size());
                App::logMsg(sim_verbosity_msgs, "done.");
                App::undoRedo_sceneChanged("");
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }

    if ( (commandID == SCENE_OBJECT_OPERATION_SCALE_INERTIA_SOOCMD) || (commandID == SCENE_OBJECT_OPERATION_SCALE_INERTIA_MODEL_SOOCMD) )
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<CSceneObject*> sel;
            if (commandID == SCENE_OBJECT_OPERATION_SCALE_INERTIA_SOOCMD)
                App::currentWorld->sceneObjects->getSelectedObjects(sel, sim_sceneobject_shape, false);
            else
                App::currentWorld->sceneObjects->getSelectedModels(sel, sim_sceneobject_shape, true);
            App::currentWorld->sceneObjects->deselectObjects();
            bool ok;
            double fact = 0.0;
            ok = GuiApp::uiThread->dialogInputGetFloat(GuiApp::mainWindow, "Inertia scaling", "Scaling factor", 2.0, 0.1, 10.0, 2, &fact);
            if (ok)
            {
                App::logMsg(sim_verbosity_msgs, "Scaling inertia...");
                std::vector<int> toSelect;
                for (size_t i = 0; i < sel.size(); i++)
                {
                    CShape* it = (CShape*)sel[i];
                    if (!it->getStatic())
                    {
                        toSelect.push_back(it->getObjectHandle());
                        it->getMesh()->setInertia(it->getMesh()->getInertia() * fact);
                    }
                }
                App::currentWorld->sceneObjects->setSelectedObjectHandles(toSelect.data(), toSelect.size());
                App::logMsg(sim_verbosity_msgs, "done.");
                App::undoRedo_sceneChanged("");
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }

    if (commandID == SCENE_OBJECT_OPERATION_SELECT_ALL_OBJECTS_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs, IDSNS_SELECTING_ALL_OBJECTS);
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(); i++)
                App::currentWorld->sceneObjects->addObjectToSelection(
                    App::currentWorld->sceneObjects->getObjectFromIndex(i)->getObjectHandle());
            App::logMsg(sim_verbosity_msgs, "done.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }

    if (commandID == SCENE_OBJECT_OPERATION_REMOVE_ASSOCIATED_CHILD_SCRIPT_SOOCMD)
    { // Old scripts
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            int id = App::currentWorld->sceneObjects->getLastSelectionHandle();
            CScriptObject* script = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(
                sim_scripttype_simulation, id);
            if (script != nullptr)
            {
                if (GuiApp::mainWindow != nullptr)
                    GuiApp::mainWindow->codeEditorContainer->closeFromScriptUid(script->getScriptUid(), nullptr, true);
                App::currentWorld->sceneObjects->embeddedScriptContainer->removeScript(script->getScriptHandle());
                App::undoRedo_sceneChanged("");
                GuiApp::setFullDialogRefreshFlag();
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }

    if (commandID == SCENE_OBJECT_OPERATION_REMOVE_ASSOCIATED_CUSTOMIZATION_SCRIPT_SOOCMD)
    { // Old scripts
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            int id = App::currentWorld->sceneObjects->getLastSelectionHandle();
            CScriptObject* script = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(
                sim_scripttype_customization, id);
            if (script != nullptr)
            {
                if (GuiApp::mainWindow != nullptr)
                    GuiApp::mainWindow->codeEditorContainer->closeFromScriptUid(script->getScriptUid(), nullptr, true);
                App::currentWorld->sceneObjects->embeddedScriptContainer->removeScript(script->getScriptHandle());
                App::undoRedo_sceneChanged("");
                GuiApp::setFullDialogRefreshFlag();
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }

    if (commandID == SCENE_OBJECT_OPERATION_DESELECT_OBJECTS_SOOCMD)
    {
        App::currentWorld->sceneObjects->deselectObjects();
    }

    if (commandID == SCENE_OBJECT_OPERATION_OBJECT_FULL_COPY_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount(); i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            App::logMsg(sim_verbosity_msgs, IDSNS_COPYING_SELECTION);
            GuiApp::uiThread->showOrHideProgressBar(true, -1.0, "Copying objects...");
            _copyObjects(&sel);
            GuiApp::uiThread->showOrHideProgressBar(false);
            App::logMsg(sim_verbosity_msgs, "done.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    if (commandID == SCENE_OBJECT_OPERATION_OBJECT_FULL_CUT_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount(); i++)
            {
                CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(
                    App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
                if ((it->getObjectProperty() & sim_objectproperty_cannotdelete) == 0)
                {
                    if (((it->getObjectProperty() & sim_objectproperty_cannotdeleteduringsim) == 0) ||
                        App::currentWorld->simulation->isSimulationStopped())
                        sel.push_back(it->getObjectHandle());
                }
            }
            if (sel.size() > 0)
            {
                App::logMsg(sim_verbosity_msgs, IDSNS_CUTTING_SELECTION);
                GuiApp::uiThread->showOrHideProgressBar(true, -1.0, "Cutting objects...");
                App::currentWorld->sceneObjects->addModelObjects(sel);
                _copyObjects(&sel);
                _deleteObjects(&sel);
                App::currentWorld->sceneObjects->deselectObjects(); // We clear selection
                GuiApp::uiThread->showOrHideProgressBar(false);
                App::undoRedo_sceneChanged("");
                App::logMsg(sim_verbosity_msgs, "done.");
            }
            App::currentWorld->sceneObjects->deselectObjects();
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    if (commandID == SCENE_OBJECT_OPERATION_PASTE_OBJECTS_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount(); i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            App::logMsg(sim_verbosity_msgs, IDSNS_PASTING_BUFFER);

            GuiApp::uiThread->showOrHideProgressBar(true, -1.0, "Pasting objects...");
            bool failed = (App::worldContainer->copyBuffer->pasteBuffer(
                               App::currentWorld->environment->getSceneLocked(), 3) == -1);
            GuiApp::uiThread->showOrHideProgressBar(false);
            if (failed) // Error: trying to copy locked buffer into unlocked scene!
                GuiApp::uiThread->messageBox_warning(GuiApp::mainWindow, "Paste",
                                                     IDS_SCENE_IS_LOCKED_CANNOT_PASTE_WARNING, VMESSAGEBOX_OKELI,
                                                     VMESSAGEBOX_REPLY_OK);
            else
                App::logMsg(sim_verbosity_msgs, "done.");
            App::undoRedo_sceneChanged("");
        }
        else
        { // We are in the UI thread. We execute the command in a delayed manner:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }

    if (commandID == SCENE_OBJECT_OPERATION_DELETE_OBJECTS_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount(); i++)
            {
                CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(
                    App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
                if ((it->getObjectProperty() & sim_objectproperty_cannotdelete) == 0)
                {
                    if (((it->getObjectProperty() & sim_objectproperty_cannotdeleteduringsim) == 0) ||
                        App::currentWorld->simulation->isSimulationStopped())
                        sel.push_back(it->getObjectHandle());
                }
            }
            if (sel.size() > 0)
            {
                App::logMsg(sim_verbosity_msgs, IDSNS_DELETING_SELECTION);
                GuiApp::uiThread->showOrHideProgressBar(true, -1.0, "Deleting objects...");
                _deleteObjects(&sel);
                GuiApp::uiThread->showOrHideProgressBar(false);
                App::undoRedo_sceneChanged("");
                App::logMsg(sim_verbosity_msgs, "done.");
            }
            App::currentWorld->sceneObjects->deselectObjects();
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    if ((commandID == SCENE_OBJECT_OPERATION_RELOCATE_FRAME_TO_ORIGIN_SOOCMD) ||
        (commandID == SCENE_OBJECT_OPERATION_RELOCATE_FRAME_TO_PARENT_SOOCMD) ||
        (commandID == SCENE_OBJECT_OPERATION_RELOCATE_FRAME_TO_CENTER_SOOCMD))
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<CSceneObject*> sel;
            App::currentWorld->sceneObjects->getSelectedObjects(sel, sim_sceneobject_shape, false, false);
            if (sel.size() > 0)
            {
                if (commandID == SCENE_OBJECT_OPERATION_RELOCATE_FRAME_TO_ORIGIN_SOOCMD)
                    App::logMsg(sim_verbosity_msgs, "Relocating reference frame to world origin...");
                if (commandID == SCENE_OBJECT_OPERATION_RELOCATE_FRAME_TO_PARENT_SOOCMD)
                    App::logMsg(sim_verbosity_msgs, "Relocating reference frame to parent origin...");
                if (commandID == SCENE_OBJECT_OPERATION_RELOCATE_FRAME_TO_CENTER_SOOCMD)
                    App::logMsg(sim_verbosity_msgs, "relocating reference frame to mesh center...");
                bool success = true;
                std::vector<int> toSelect;
                for (size_t i = 0; i < sel.size(); i++)
                {
                    CShape* theShape = (CShape*)sel[i];
                    bool r = false;
                    if (commandID == SCENE_OBJECT_OPERATION_RELOCATE_FRAME_TO_ORIGIN_SOOCMD)
                        r = theShape->relocateFrame("world");
                    if (commandID == SCENE_OBJECT_OPERATION_RELOCATE_FRAME_TO_PARENT_SOOCMD)
                        r = theShape->relocateFrame("parent");
                    if (commandID == SCENE_OBJECT_OPERATION_RELOCATE_FRAME_TO_CENTER_SOOCMD)
                        r = theShape->relocateFrame("mesh");
                    if (r)
                        toSelect.push_back(theShape->getObjectHandle());
                    success = r && success;
                }
                App::currentWorld->sceneObjects->setSelectedObjectHandles(toSelect.data(), toSelect.size());
                App::undoRedo_sceneChanged("");
                if (success)
                    App::logMsg(sim_verbosity_msgs, "done.");
                else
                    App::logMsg(sim_verbosity_warnings, "One or more reference frames could not be relocated.");
            }
            else
                App::logMsg(sim_verbosity_msgs, "invalid selection. Operation aborted.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }

    if ((commandID == SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_MESH_SOOCMD) ||
        (commandID == SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_WORLD_SOOCMD))
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<CSceneObject*> sel;
            App::currentWorld->sceneObjects->getSelectedObjects(sel, sim_sceneobject_shape, false, false);
            if (sel.size() > 0)
            {
                if (commandID == SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_MESH_SOOCMD)
                    App::logMsg(sim_verbosity_msgs, "aligning bounding box with mesh...");
                if (commandID == SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_WORLD_SOOCMD)
                    App::logMsg(sim_verbosity_msgs, "aligning bounding box with world...");
                bool success = true;
                std::vector<int> toSelect;
                for (size_t i = 0; i < sel.size(); i++)
                {
                    CShape* theShape = (CShape*)sel[i];
                    bool r = false;
                    if (commandID == SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_MESH_SOOCMD)
                        r = theShape->alignBB("mesh");
                    if (commandID == SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_WORLD_SOOCMD)
                        r = theShape->alignBB("world");
                    if (r)
                        toSelect.push_back(theShape->getObjectHandle());
                    success = r && success;
                }
                App::currentWorld->sceneObjects->setSelectedObjectHandles(toSelect.data(), toSelect.size());
                App::undoRedo_sceneChanged("");
                if (success)
                    App::logMsg(sim_verbosity_msgs, "done.");
                else
                    App::logMsg(sim_verbosity_warnings, "One or more bounding boxes could not be reoriented.");
            }
            else
                App::logMsg(sim_verbosity_msgs, "invalid selection. Operation aborted.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }

    if (commandID == SCENE_OBJECT_OPERATION_GROUP_SHAPES_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            App::currentWorld->sceneObjects->getSelectedObjectHandles(sel, sim_sceneobject_shape, false, false);
            if (sel.size() > 1)
            {
                App::logMsg(sim_verbosity_msgs, "Grouping shapes...");
                groupSelection(&sel);
                App::undoRedo_sceneChanged("");
                App::logMsg(sim_verbosity_msgs, "done.");
            }
            else
                App::logMsg(sim_verbosity_msgs, "invalid selection. Operation aborted.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }

    if (commandID == SCENE_OBJECT_OPERATION_UNGROUP_SHAPES_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            App::currentWorld->sceneObjects->getSelectedObjectHandles(sel, sim_sceneobject_shape, false, false);
            bool hasCompound = false;
            for (size_t i = 0; i < sel.size(); i++)
            {
                CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(sel[i]);
                if ( (it != nullptr) && it->isCompound() )
                {
                    hasCompound = true;
                    break;
                }
            }
            if (hasCompound)
            {
                App::logMsg(sim_verbosity_msgs, "Ungrouping shapes...");
                ungroupSelection(&sel);
                App::undoRedo_sceneChanged("");
                App::logMsg(sim_verbosity_msgs, "done.");
            }
            else
                App::logMsg(sim_verbosity_msgs, "invalid selection. Operation aborted.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }

    if (commandID == SCENE_OBJECT_OPERATION_MERGE_SHAPES_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            App::currentWorld->sceneObjects->getSelectedObjectHandles(sel, sim_sceneobject_shape, false, false);
            if (sel.size() > 1)
            {
                App::logMsg(sim_verbosity_msgs, "Merging shapes...");
                mergeSelection(&sel);
                App::undoRedo_sceneChanged("");
                App::logMsg(sim_verbosity_msgs, "done.");
            }
            else
                App::logMsg(sim_verbosity_msgs, "invalid selection. Operation aborted.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    if (commandID == SCENE_OBJECT_OPERATION_DIVIDE_SHAPES_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            App::currentWorld->sceneObjects->getSelectedObjectHandles(sel, sim_sceneobject_shape, false, false);
            if (sel.size() > 0)
            {
                App::logMsg(sim_verbosity_msgs, "Dividing shapes...");
                divideSelection(&sel);
                App::undoRedo_sceneChanged("");
                App::logMsg(sim_verbosity_msgs, "done.");
            }
            else
                App::logMsg(sim_verbosity_msgs, "invalid selection. Operation aborted.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }

    if (commandID == SCENE_OBJECT_OPERATION_UNDO_SOOCMD)
    {
        if (GuiApp::getEditModeType() == NO_EDIT_MODE)
        {
            if (!VThread::isUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                App::logMsg(sim_verbosity_msgs, IDSNS_EXECUTING_UNDO);
                App::currentWorld->undoBufferContainer->undo();
                App::logMsg(sim_verbosity_msgs, "done.");
            }
            else
            { // We are in the UI thread. Execute the command via the main thread:
                SSimulationThreadCommand cmd;
                cmd.cmdId = commandID;
                App::appendSimulationThreadCommand(cmd);
            }
        }
        return (true);
    }

    if (commandID == SCENE_OBJECT_OPERATION_REDO_SOOCMD)
    {
        if (GuiApp::getEditModeType() == NO_EDIT_MODE)
        {
            if (!VThread::isUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                App::logMsg(sim_verbosity_msgs, IDSNS_EXECUTING_REDO);
                App::currentWorld->undoBufferContainer->redo();
                App::logMsg(sim_verbosity_msgs, "done.");
            }
            else
            { // We are in the UI thread. Execute the command via the main thread:
                SSimulationThreadCommand cmd;
                cmd.cmdId = commandID;
                App::appendSimulationThreadCommand(cmd);
            }
        }
        return (true);
    }
    return (false);
}
#endif
void CSceneObjectOperations::_copyObjects(std::vector<int>* selection)
{
    // We first copy the selection:
    std::vector<int> sel(*selection);
    App::currentWorld->sceneObjects->addModelObjects(sel);
    App::worldContainer->copyBuffer->copyCurrentSelection(sel, App::currentWorld->environment->getSceneLocked(), 0);
    App::currentWorld->sceneObjects->deselectObjects(); // We clear selection
}

void CSceneObjectOperations::_deleteObjects(std::vector<int>* selection)
{ // There are a few other spots where objects get deleted (e.g. the C-interface)
    TRACE_INTERNAL;
    App::currentWorld->sceneObjects->addModelObjects(selection[0]);
    App::currentWorld->sceneObjects->eraseObjects(selection, true);
    App::currentWorld->sceneObjects->deselectObjects();
}

int CSceneObjectOperations::groupSelection(std::vector<int>* selection)
{
    if (selection->size() < 2)
        return (-1);

    std::vector<CShape*> shapesToGroup;
    for (size_t i = 0; i < selection->size(); i++)
    {
        CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(selection->at(i));
        shapesToGroup.push_back(it);
    }

    App::currentWorld->sceneObjects->deselectObjects();

    CShape* compoundShape = _groupShapes(shapesToGroup);

    App::currentWorld->sceneObjects->selectObject(compoundShape->getObjectHandle());

    return (compoundShape->getObjectHandle());
}

CShape* CSceneObjectOperations::_groupShapes(const std::vector<CShape*>& shapesToGroup)
{                         // returned shape is replacing the last shape in the selection. All involved shapes are destroyed
    size_t pureCount = 0; // except for heightfields
    for (size_t i = 0; i < shapesToGroup.size(); i++)
    {
        CShape* it = shapesToGroup[i];
        if (it->getMesh()->isPure())
        {
            CMesh* m = it->getSingleMesh();
            if (m == nullptr)
                pureCount++; // pure compound
            else
            {
                if (m->getPurePrimitiveType() != sim_primitiveshape_heightfield)
                    pureCount++; // not a heightfield
            }
        }
    }
    bool allToNonPure = (pureCount < shapesToGroup.size());
    std::vector<CMeshWrapper*> allMeshes;
    for (size_t i = 0; i < shapesToGroup.size(); i++)
    {
        CShape* it = shapesToGroup[i];
        if (allToNonPure)
            it->getMesh()->setPurePrimitiveType(sim_primitiveshape_none, 1.0, 1.0, 1.0); // this will be propagated to all geometrics!
        allMeshes.push_back(it->getMesh()->copyYourself());
    }

    CShape* lastSel = shapesToGroup[shapesToGroup.size() - 1];
    CShape* newShape = (CShape*)lastSel->copyYourself();

    CMeshWrapper* newWrapper = new CMeshWrapper();
    for (size_t i = 0; i < allMeshes.size(); i++)
    {
        CMeshWrapper* mesh = allMeshes[i];
        mesh->setName(shapesToGroup[i]->getObjectAlias().c_str());
        mesh->setIFrame(lastSel->getCumulativeTransformation().getInverse() * shapesToGroup[i]->getCumulativeTransformation() * mesh->getIFrame());
        newWrapper->addItem(mesh);
    }

    newShape->replaceMesh(newWrapper, false);
    App::currentWorld->sceneObjects->addObjectToScene(newShape, false, true);
    App::currentWorld->sceneObjects->setObjectParent(newShape, lastSel->getParent(), false);
    int order = App::currentWorld->sceneObjects->getObjectSequence(lastSel, nullptr);
    App::currentWorld->sceneObjects->setObjectSequence(newShape, order);
    App::currentWorld->sceneObjects->setObjectParent(lastSel, newShape, true);

    std::vector<int> shapesToErase;
    for (size_t i = 0; i < shapesToGroup.size(); i++)
        shapesToErase.push_back(shapesToGroup[i]->getObjectHandle());
    App::currentWorld->sceneObjects->eraseObjects(&shapesToErase, true);
    return (newShape);
}

void CSceneObjectOperations::ungroupSelection(std::vector<int>* selection, bool fullUngroup /*= false*/)
{
    App::currentWorld->sceneObjects->deselectObjects();
    std::vector<int> finalSel;
    for (size_t i = 0; i < selection->size(); i++)
    {
        CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(selection->at(i));
        if ((it != nullptr) && it->isCompound())
        {
            std::vector<CShape*> newShapes;
            if (fullUngroup)
                _fullUngroupShape(it, newShapes);
            else
                _ungroupShape(it, newShapes);
            for (size_t j = 0; j < newShapes.size(); j++)
                finalSel.push_back(newShapes[j]->getObjectHandle());
        }
    }

    selection->clear();
    for (size_t i = 0; i < finalSel.size(); i++)
    {
        App::currentWorld->sceneObjects->addObjectToSelection(finalSel[i]);
        selection->push_back(finalSel[i]);
    }
}

void CSceneObjectOperations::_fullUngroupShape(CShape* it, std::vector<CShape*>& newShapes)
{
    if (it->isCompound())
    {
        std::vector<CShape*> ns;
        _ungroupShape(it, ns);
        for (size_t i = 0; i < ns.size(); i++)
            _fullUngroupShape(ns[i], newShapes);
    }
    else
        newShapes.push_back(it);
}

void CSceneObjectOperations::CSceneObjectOperations::_ungroupShape(CShape* it, std::vector<CShape*>& newShapes)
{ // the input shape is destroyed!
    if (!it->getMesh()->isPure())
        it->getMesh()->setPurePrimitiveType(sim_primitiveshape_none, 1.0, 1.0, 1.0);

    CMeshWrapper* wrapper = it->getMesh();
    C7Vector oldTransf(it->getCumulativeTransformation());
    C7Vector oldParentTransf(it->getFullParentCumulativeTransformation());
    std::vector<CMeshWrapper*> meshes;
    for (size_t i = 0; i < wrapper->childList.size(); i++)
    {
        CMeshWrapper* mesh = wrapper->childList[i]->copyYourself();
        C7Vector newTransf(oldTransf * mesh->getIFrame());
        mesh->setIFrame(C7Vector::identityTransformation);
        CShape* shape = nullptr;
        if (i == wrapper->childList.size() - 1)
        {
            shape = (CShape*)it->copyYourself();
            shape->replaceMesh(mesh, false);
            shape->setLocalTransformation(oldParentTransf.getInverse() * newTransf);
            App::currentWorld->sceneObjects->addObjectToScene(shape, false, true);
            App::currentWorld->sceneObjects->setObjectAlias(shape, it->getObjectAlias().c_str(), true);
            App::currentWorld->sceneObjects->setObjectParent(shape, it->getParent(), false);
            int order = App::currentWorld->sceneObjects->getObjectSequence(it, nullptr);
            App::currentWorld->sceneObjects->setObjectSequence(shape, order);
            App::currentWorld->sceneObjects->setObjectParent(it, shape, true);
        }
        else
        {
            shape = new CShape();
            it->copyAttributesTo(shape);
            shape->replaceMesh(mesh, false);
            shape->setLocalTransformation(newTransf);
            App::currentWorld->sceneObjects->addObjectToScene(shape, false, true);
            App::currentWorld->sceneObjects->setObjectParent(shape, it->getParent(), true);
            App::currentWorld->sceneObjects->setObjectAlias(shape, mesh->getName().c_str(), true);
        }
        newShapes.push_back(shape);
    }
    App::currentWorld->sceneObjects->eraseObject(it, true);
}

int CSceneObjectOperations::mergeSelection(std::vector<int>* selection)
{
    int retVal = -1;
    std::vector<CShape*> shapesToMerge;
    for (size_t i = 0; i < selection->size(); i++)
    {
        CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(selection->at(i));
        if (it != nullptr)
            shapesToMerge.push_back(it);
    }
    App::currentWorld->sceneObjects->deselectObjects();
    if (shapesToMerge.size() >= 2)
    {
        CShape* mergedShape = _mergeShapes(shapesToMerge);
        retVal = mergedShape->getObjectHandle();
        App::currentWorld->sceneObjects->selectObject(retVal);
    }
    return (retVal);
}

CShape* CSceneObjectOperations::_mergeShapes(const std::vector<CShape*>& allShapes)
{ // returned shape is replacing the last shape in the selection. All involved shapes are destroyed
    for (size_t i = 0; i < allShapes.size(); i++)
    {
        CShape* it = allShapes[i];
        if (it->getMesh()->getTextureCount() != 0)
        {
            App::currentWorld->textureContainer->announceGeneralObjectWillBeErased(it->getObjectHandle(), -1);
            it->getMesh()->removeAllTextures();
        }
    }

    CShape* lastSel = allShapes[allShapes.size() - 1];
    CShape* newShape = (CShape*)lastSel->copyYourself();
    newShape->setLocalTransformation(lastSel->getCumulativeTransformation());

    std::vector<CMeshWrapper*> allMeshes;
    for (size_t i = 0; i < allShapes.size(); i++)
        allMeshes.push_back(allShapes[i]->getMesh());

    std::vector<double> vertices;
    std::vector<int> indices;
    std::vector<double> normals;
    for (size_t i = 0; i < allMeshes.size(); i++)
    {
        CMeshWrapper* mesh = allMeshes[i];
        mesh->getCumulativeMeshes(allShapes[i]->getCumulativeTransformation(), vertices, &indices, &normals);
    }
    CMesh* newMesh = new CMesh(newShape->getLocalTransformation(), vertices, indices, &normals, nullptr, 0);
    newShape->replaceMesh(newMesh, true);

    std::vector<int> shapesToErase;
    App::currentWorld->sceneObjects->addObjectToScene(newShape, false, true);
    App::currentWorld->sceneObjects->setObjectParent(newShape, lastSel->getParent(), true);
    int order = App::currentWorld->sceneObjects->getObjectSequence(lastSel, nullptr);
    App::currentWorld->sceneObjects->setObjectSequence(newShape, order);
    App::currentWorld->sceneObjects->setObjectParent(lastSel, newShape, true);

    for (size_t i = 0; i < allShapes.size(); i++)
        shapesToErase.push_back(allShapes[i]->getObjectHandle());
    App::currentWorld->sceneObjects->eraseObjects(&shapesToErase, true);
    return newShape;
}

void CSceneObjectOperations::divideSelection(std::vector<int>* selection)
{
    std::vector<CShape*> shapesToDivide;
    for (size_t i = 0; i < selection->size(); i++)
    {
        CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(selection->at(i));
        if ((it != nullptr) && (!it->getMesh()->isPure()))
            shapesToDivide.push_back(it);
    }

    App::currentWorld->sceneObjects->deselectObjects();
    selection->clear();

    for (size_t i = 0; i < shapesToDivide.size(); i++)
    {
        std::vector<CShape*> ns;
        if (_divideShape(shapesToDivide[i], ns))
        {
            for (size_t j = 0; j < ns.size(); j++)
                selection->push_back(ns[j]->getObjectHandle());
        }
    }

    App::currentWorld->sceneObjects->setSelectedObjectHandles(selection->data(), selection->size());
}

bool CSceneObjectOperations::_divideShape(CShape* it, std::vector<CShape*>& newShapes)
{ // the input shape is destroyed!
    std::vector<double> vertices;
    std::vector<int> indices;
    it->getMesh()->getCumulativeMeshes(it->getFullCumulativeTransformation(), vertices, &indices, nullptr);
    int extractedCount = 0;
    while (true)
    {
        std::vector<double> subvert;
        std::vector<int> subind;
        CShape* shape = nullptr;
        if (CMeshManip::extractOneShape(&vertices, &indices, &subvert, &subind))
        { // Something was extracted
            extractedCount++;
            CMesh* mesh = new CMesh(it->getFullCumulativeTransformation(), subvert, subind, nullptr, nullptr, 0);
            shape = new CShape();
            it->copyAttributesTo(shape);
            shape->replaceMesh(mesh, false);
            if (it->getMesh()->isMesh())
                ((CMesh*)it->getMesh())->copyVisualAttributesTo(mesh);
            shape->setLocalTransformation(it->getCumulativeTransformation());
            App::currentWorld->sceneObjects->addObjectToScene(shape, false, true);
            App::currentWorld->sceneObjects->setObjectParent(shape, it->getParent(), true);
            App::currentWorld->sceneObjects->setObjectAlias(shape, it->getObjectAlias().c_str(), true);
            newShapes.push_back(shape);
        }
        else
        { // nothing was extracted
            if (extractedCount == 0)
                break; // we couldn't extract anything!
            shape = (CShape*)it->copyYourself();
            shape->setLocalTransformation(it->getCumulativeTransformation());
            CMesh* mesh = new CMesh(shape->getLocalTransformation(), vertices, indices, nullptr, nullptr, 0);
            shape->replaceMesh(mesh, true);
            App::currentWorld->sceneObjects->addObjectToScene(shape, false, true);
            App::currentWorld->sceneObjects->setObjectParent(shape, it->getParent(), true);
            int order = App::currentWorld->sceneObjects->getObjectSequence(it, nullptr);
            App::currentWorld->sceneObjects->setObjectSequence(shape, order);
            App::currentWorld->sceneObjects->setObjectParent(it, shape, true);
            App::currentWorld->sceneObjects->eraseObject(it, true);
            newShapes.push_back(shape);
            break;
        }
    }
    return (newShapes.size() > 0);
}

void CSceneObjectOperations::scaleObjects(const std::vector<int>& selection, double scalingFactor,
                                          bool scalePositionsToo)
{
    std::vector<int> sel(selection);
    App::currentWorld->sceneObjects->addModelObjects(sel);
    for (size_t i = 0; i < sel.size(); i++)
    {
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(sel[i]);
        if (scalePositionsToo)
            it->scalePosition(scalingFactor);
        else
        { // If one parent is a root object (model base) and in this selection, then we also scale the position here!!
            // (2009/06/10)
            CSceneObject* itp = it->getParent();
            while (itp != nullptr)
            {
                if (itp->getModelBase())
                { // We found a parent that is a root! Is it in the selection?
                    bool f = false;
                    for (int j = 0; j < int(sel.size()); j++)
                    {
                        if (sel[j] == itp->getObjectHandle())
                        { // YEs!
                            f = true;
                            break;
                        }
                    }
                    if (f)
                    { // We also scale the pos here!!
                        it->scalePosition(scalingFactor);
                        break;
                    }
                }
                itp = itp->getParent();
            }
        }
        it->scaleObject(scalingFactor);
    }

    // OLD IK:
    for (size_t i = 0; i < App::currentWorld->ikGroups_old->getObjectCount(); i++)
    {
        CIkGroup_old* ikGroup = App::currentWorld->ikGroups_old->getObjectFromIndex(i);
        // Go through all ikElement lists:
        for (size_t j = 0; j < ikGroup->getIkElementCount(); j++)
        {
            CIkElement_old* ikEl = ikGroup->getIkElementFromIndex(j);
            CDummy* tip = App::currentWorld->sceneObjects->getDummyFromHandle(ikEl->getTipHandle());
            bool scaleElement = false;
            if (tip != nullptr)
            { // was this tip scaled?
                bool tipFound = false;
                for (int k = 0; k < int(sel.size()); k++)
                {
                    if (sel[k] == tip->getObjectHandle())
                    {
                        tipFound = true;
                        break;
                    }
                }
                if (tipFound)
                { // yes, tip was found!
                    scaleElement = true;
                }
            }
            if (scaleElement)
                ikEl->setMinLinearPrecision(ikEl->getMinLinearPrecision() * scalingFactor); // we scale that ikElement!
        }
    }
}

CShape* CSceneObjectOperations::_morphToConvexDecomposed(
    CShape* it, size_t nClusters, double maxConcavity, bool addExtraDistPoints, bool addFacesPoints,
    double maxConnectDist, size_t maxTrianglesInDecimatedMesh, size_t maxHullVertices, double smallClusterThreshold,
    bool useHACD, int resolution_VHACD, int depth_VHACD_old, double concavity_VHACD, int planeDownsampling_VHACD,
    int convexHullDownsampling_VHACD, double alpha_VHACD, double beta_VHACD, double gamma_VHACD_old, bool pca_VHACD,
    bool voxelBased_VHACD, int maxVerticesPerCH_VHACD, double minVolumePerCH_VHACD)
{
    CShape* morphedShape = nullptr;
    C7Vector obbTr(it->getCumulativeTransformation() * it->getBB(nullptr));
    if (it->isCompound())
    { // ungroup, then group again
        C7Vector tr(it->getCumulativeTransformation());
        double mass = it->getMesh()->getMass();
        C3Vector com(it->getMesh()->getCOM());
        C3X3Matrix inertia(it->getMesh()->getInertia());
        std::vector<CShape*> newShapes;
        _fullUngroupShape(it, newShapes);
        newShapes.push_back(it);
        std::vector<CShape*> newShapes2;
        for (size_t j = 0; j < newShapes.size(); j++)
        {
            CShape* it2 = newShapes[j];
            std::vector<double> vert;
            std::vector<int> ind;
            it2->getMesh()->getCumulativeMeshes(C7Vector::identityTransformation, vert, &ind, nullptr);
            std::vector<std::vector<double>*> outputVert;
            std::vector<std::vector<int>*> outputInd;
            CMeshRoutines::convexDecompose(&vert[0], (int)vert.size(), &ind[0], (int)ind.size(), outputVert, outputInd,
                                           nClusters, maxConcavity, addExtraDistPoints, addFacesPoints, maxConnectDist,
                                           maxTrianglesInDecimatedMesh, maxHullVertices, smallClusterThreshold, useHACD,
                                           resolution_VHACD, depth_VHACD_old, concavity_VHACD, planeDownsampling_VHACD,
                                           convexHullDownsampling_VHACD, alpha_VHACD, beta_VHACD, gamma_VHACD_old,
                                           pca_VHACD, voxelBased_VHACD, maxVerticesPerCH_VHACD, minVolumePerCH_VHACD);
            std::vector<CMesh*> allMeshes;
            for (size_t i = 0; i < outputVert.size(); i++)
            {
                bool addMesh = true;
                if (CMeshRoutines::getConvexType(outputVert[i][0], outputInd[i][0], 0.015) != 0)
                    addMesh = CMeshRoutines::getConvexHull(outputVert[i][0], outputVert[i][0], outputInd[i][0]);
                if (addMesh)
                {
                    CMesh* mesh = new CMesh(C7Vector::identityTransformation, outputVert[i][0], outputInd[i][0],
                                            nullptr, nullptr, 0);
                    allMeshes.push_back(mesh);
                }
                delete outputVert[i];
                delete outputInd[i];
            }
            if (allMeshes.size() > 0)
            {
                CMeshWrapper* wrap = new CMeshWrapper();
                for (size_t i = 0; i < allMeshes.size(); i++)
                    wrap->addItem(allMeshes[i]);
                it2->replaceMesh(wrap, true);
                newShapes2.push_back(it2);
            }
            else
                App::currentWorld->sceneObjects->eraseObject(it2, true);
        }
        if (newShapes2.size() > 0)
        {
            morphedShape = newShapes2[newShapes2.size() - 1];
            if (newShapes2.size() > 1)
                _groupShapes(newShapes2);
            morphedShape->relocateFrame("custom", &tr);
            morphedShape->getMesh()->setMass(mass);
            morphedShape->getMesh()->setCOM(com);
            morphedShape->getMesh()->setInertia(inertia);
        }
    }
    else
    {
        std::vector<double> vert;
        std::vector<int> ind;
        it->getMesh()->getCumulativeMeshes(C7Vector::identityTransformation, vert, &ind, nullptr);
        std::vector<std::vector<double>*> outputVert;
        std::vector<std::vector<int>*> outputInd;
        CMeshRoutines::convexDecompose(&vert[0], (int)vert.size(), &ind[0], (int)ind.size(), outputVert, outputInd,
                                       nClusters, maxConcavity, addExtraDistPoints, addFacesPoints, maxConnectDist,
                                       maxTrianglesInDecimatedMesh, maxHullVertices, smallClusterThreshold, useHACD,
                                       resolution_VHACD, depth_VHACD_old, concavity_VHACD, planeDownsampling_VHACD,
                                       convexHullDownsampling_VHACD, alpha_VHACD, beta_VHACD, gamma_VHACD_old,
                                       pca_VHACD, voxelBased_VHACD, maxVerticesPerCH_VHACD, minVolumePerCH_VHACD);
        std::vector<CMesh*> allMeshes;
        for (size_t i = 0; i < outputVert.size(); i++)
        {
            bool addMesh = true;
            if (CMeshRoutines::getConvexType(outputVert[i][0], outputInd[i][0], 0.015) != 0)
                addMesh = CMeshRoutines::getConvexHull(outputVert[i][0], outputVert[i][0], outputInd[i][0]);
            if (addMesh)
            {
                CMesh* mesh =
                    new CMesh(C7Vector::identityTransformation, outputVert[i][0], outputInd[i][0], nullptr, nullptr, 0);
                allMeshes.push_back(mesh);
            }
            delete outputVert[i];
            delete outputInd[i];
        }
        if (allMeshes.size() > 0)
        {
            morphedShape = it;
            CMeshWrapper* wrap = new CMeshWrapper();
            for (size_t i = 0; i < allMeshes.size(); i++)
                wrap->addItem(allMeshes[i]);
            it->replaceMesh(wrap, true);
        }
    }
    if (morphedShape)
        morphedShape->alignBB("custom", &obbTr);
    return (morphedShape);
}

int CSceneObjectOperations::convexDecompose(int shapeHandle, int options, const int* intParams,
                                            const double* floatParams)
{
    TRACE_INTERNAL;
    int retVal = -1;

    static bool addExtraDistPoints = true;
    static bool addFacesPoints = true;
    static int nClusters = 1;
    static int maxHullVertices = 200; // from 100 to 200 on 5/2/2014
    static double maxConcavity = 100.0;
    static double smallClusterThreshold = 0.25;
    static int maxTrianglesInDecimatedMesh = 500;
    static double maxConnectDist = 30.0;
    static bool useHACD = false; // i.e. use V-HACD
    static int resolution = 100000;
    static double concavity = 0.0025;
    static int planeDownsampling = 4;
    static int convexHullDownsampling = 4;
    static double alpha = 0.05;
    static double beta = 0.05;
    static bool pca = false;
    static bool voxelBasedMode = true;
    static int maxVerticesPerCH = 64;
    static double minVolumePerCH = 0.0001;

    if ((options & 4) == 0)
    {
        addExtraDistPoints = (options & 8) != 0;
        addFacesPoints = (options & 16) != 0;
        nClusters = intParams[0];
        maxHullVertices = intParams[2];
        maxConcavity = floatParams[0];
        smallClusterThreshold = floatParams[2];
        maxTrianglesInDecimatedMesh = intParams[1];
        maxConnectDist = floatParams[1];
        useHACD = true; // forgotten, fixed thanks to Patrick Gruener
        if (options & 128)
        { // we have more parameters than usual (i.e. the V-HACD parameters):
            useHACD = false;
            resolution = intParams[5];
            concavity = floatParams[5];
            planeDownsampling = intParams[7];
            convexHullDownsampling = intParams[8];
            alpha = floatParams[6];
            beta = floatParams[7];
            pca = (options & 256);
            voxelBasedMode = !(options & 512);
            maxVerticesPerCH = intParams[9];
            minVolumePerCH = floatParams[9];
        }
    }
    CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
    if ((options & 1) == 0)
    { // We want to create a new shape from it:
        CShape* it2 = new CShape();
        it2->replaceMesh(it->getMesh()->copyYourself(), false);
        it2->setLocalTransformation(it->getCumulativeTransformation());
        App::currentWorld->sceneObjects->addObjectToScene(it2, false, true);
        it = it2;
    }
    it = CSceneObjectOperations::_morphToConvexDecomposed(
        it, nClusters, maxConcavity, addExtraDistPoints, addFacesPoints, maxConnectDist, maxTrianglesInDecimatedMesh,
        maxHullVertices, smallClusterThreshold, useHACD, resolution, 20, concavity, planeDownsampling,
        convexHullDownsampling, alpha, beta, 0.00125, pca, voxelBasedMode, maxVerticesPerCH, minVolumePerCH);
    if (it != nullptr)
        retVal = it->getObjectHandle();
    return (retVal);
}

#ifdef SIM_WITH_GUI
void CSceneObjectOperations::addMenu(VMenu* menu)
{
    std::vector<CSceneObject*> objectSel;
    App::currentWorld->sceneObjects->getSelectedObjects(objectSel, -1, false);
    int objectSel_shapeCnt = 0;
    int objectSel_compoundCnt = 0;
    int objectSel_dynShapeCnt = 0;
    for (size_t i = 0; i < objectSel.size(); i++)
    {
        int t = objectSel[i]->getObjectType();
        if (t == sim_sceneobject_shape)
        {
            CShape* it = (CShape*)objectSel[i];
            objectSel_shapeCnt++;
            if (it->isCompound())
                objectSel_compoundCnt++;
            if (!it->getStatic())
                objectSel_dynShapeCnt++;
        }
    }

    std::vector<CSceneObject*> modelSel;
    App::currentWorld->sceneObjects->getSelectedModels(modelSel, -1, true);
    int modelSel_dynShapeCnt = 0;
    for (size_t i = 0; i < modelSel.size(); i++)
    {
        printf("Alias: %s\n", modelSel[i]->getObjectAlias().c_str());
        int t = modelSel[i]->getObjectType();
        if (t == sim_sceneobject_shape)
        {
            CShape* it = (CShape*)modelSel[i];
            if (!it->getStatic())
                modelSel_dynShapeCnt++;
        }
    }

    size_t selItems = App::currentWorld->sceneObjects->getSelectionCount();

    bool noSim = App::currentWorld->simulation->isSimulationStopped();

    if (GuiApp::getEditModeType() == NO_EDIT_MODE)
    {
        menu->appendMenuItem(App::currentWorld->undoBufferContainer->canUndo(), false, SCENE_OBJECT_OPERATION_UNDO_SOOCMD, IDSN_UNDO);
        menu->appendMenuItem(App::currentWorld->undoBufferContainer->canRedo(), false, SCENE_OBJECT_OPERATION_REDO_SOOCMD, IDSN_REDO);
        menu->appendMenuSeparator();
        menu->appendMenuItem(objectSel.size() > 1, false, SCENE_OBJECT_OPERATION_MAKE_PARENT_SOOCMD, "Set parent, keep pose(s)");
        menu->appendMenuItem(objectSel.size() > 1, false, SCENE_OBJECT_OPERATION_MAKE_PARENT_AND_MOVE_SOOCMD, "Set parent");
        menu->appendMenuItem(objectSel.size() > 0, false, SCENE_OBJECT_OPERATION_MAKE_ORPHANS_SOOCMD, "Set parent-less");
        menu->appendMenuSeparator();
        menu->appendMenuItem(objectSel.size() > 0, false, SCENE_OBJECT_OPERATION_OBJECT_FULL_COPY_SOOCMD, "Copy object(s)/model(s)");
        menu->appendMenuItem(!App::worldContainer->copyBuffer->isBufferEmpty(), false, SCENE_OBJECT_OPERATION_PASTE_OBJECTS_SOOCMD, "Paste buffer");
        menu->appendMenuItem(objectSel.size() > 0, false, SCENE_OBJECT_OPERATION_DELETE_OBJECTS_SOOCMD, "Delete object(s)/model(s)");
        menu->appendMenuItem(objectSel.size() > 0, false, SCENE_OBJECT_OPERATION_OBJECT_FULL_CUT_SOOCMD, "Cut object(s)/model(s)");
        menu->appendMenuSeparator();
        menu->appendMenuItem(true, false, SCENE_OBJECT_OPERATION_SELECT_ALL_OBJECTS_SOOCMD, "Select all");
        menu->appendMenuSeparator();

        if (CSimFlavor::getBoolVal(12))
        {
            // To remove all scripts:
            // ------------------------
            bool hasChildScriptAttached = false;
            bool hasCustomizationScriptAttached = false;
            if (selItems == 1)
            {
                hasChildScriptAttached = (App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_simulation, App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(0)) != nullptr);
                hasCustomizationScriptAttached = (App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo( sim_scripttype_customization, App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(0)) != nullptr);
            }

            if (hasChildScriptAttached || hasCustomizationScriptAttached)
            {
                VMenu* removing = new VMenu();
                removing->appendMenuItem(hasChildScriptAttached && noSim, false, SCENE_OBJECT_OPERATION_REMOVE_ASSOCIATED_CHILD_SCRIPT_SOOCMD, "Associated simulation script");
                removing->appendMenuItem(hasCustomizationScriptAttached && noSim, false, SCENE_OBJECT_OPERATION_REMOVE_ASSOCIATED_CUSTOMIZATION_SCRIPT_SOOCMD, "Associated customization script");
                menu->appendMenuAndDetach(removing, noSim, "Remove");
                menu->appendMenuSeparator();
            }
            // ------------------------

            VMenu* grouping = new VMenu();
            grouping->appendMenuItem((objectSel_shapeCnt > 1) && noSim, false, SCENE_OBJECT_OPERATION_GROUP_SHAPES_SOOCMD, "group");
            grouping->appendMenuItem((objectSel_compoundCnt > 0) && noSim, false, SCENE_OBJECT_OPERATION_UNGROUP_SHAPES_SOOCMD, "ungroup");
            grouping->appendMenuSeparator();
            grouping->appendMenuItem((objectSel_shapeCnt > 1) && noSim, false, SCENE_OBJECT_OPERATION_MERGE_SHAPES_SOOCMD, "merge");
            grouping->appendMenuItem((objectSel_shapeCnt > 0) && noSim, false, SCENE_OBJECT_OPERATION_DIVIDE_SHAPES_SOOCMD, "divide");
            menu->appendMenuAndDetach(grouping, true, "Shape grouping / merging");

            VMenu* relocate = new VMenu();
            relocate->appendMenuItem((objectSel_shapeCnt > 0) && noSim, false, SCENE_OBJECT_OPERATION_RELOCATE_FRAME_TO_ORIGIN_SOOCMD, "relocate to world origin");
            relocate->appendMenuItem((objectSel_shapeCnt > 0) && noSim, false, SCENE_OBJECT_OPERATION_RELOCATE_FRAME_TO_PARENT_SOOCMD, "relocate to parent origin");
            relocate->appendMenuItem((objectSel_shapeCnt > 0) && noSim, false, SCENE_OBJECT_OPERATION_RELOCATE_FRAME_TO_CENTER_SOOCMD, "relocate to mesh center");
            menu->appendMenuAndDetach(relocate, true, "Shape reference frame");

            VMenu* align = new VMenu();
            align->appendMenuItem((objectSel_shapeCnt > 0) && noSim, false, SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_WORLD_SOOCMD, "align with world");
            align->appendMenuItem((objectSel_shapeCnt > 0) && noSim, false, SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_MESH_SOOCMD, "align with mesh");
            menu->appendMenuAndDetach(align, true, "Shape bounding box");

            VMenu* minertia = new VMenu();
            minertia->appendMenuItem((objectSel_dynShapeCnt > 0) && noSim, false, SCENE_OBJECT_OPERATION_COMPUTE_INERTIA_SOOCMD, "compute from uniform density...");
            minertia->appendMenuItem((objectSel_dynShapeCnt > 0) && noSim, false, SCENE_OBJECT_OPERATION_SCALE_MASS_SOOCMD, "scale mass...");
            minertia->appendMenuItem((objectSel_dynShapeCnt > 0) && noSim, false, SCENE_OBJECT_OPERATION_SCALE_INERTIA_SOOCMD, "scale inertia...");
            menu->appendMenuAndDetach(minertia, true, "Shape mass and inertia");

            if (modelSel_dynShapeCnt > 0)
            {
                menu->appendMenuSeparator();
                VMenu* minertia = new VMenu();
                minertia->appendMenuItem(noSim, false, SCENE_OBJECT_OPERATION_COMPUTE_INERTIA_MODEL_SOOCMD, "compute from uniform density...");
                minertia->appendMenuItem(noSim, false, SCENE_OBJECT_OPERATION_SCALE_MASS_MODEL_SOOCMD, "scale mass...");
                minertia->appendMenuItem(noSim, false, SCENE_OBJECT_OPERATION_SCALE_INERTIA_MODEL_SOOCMD, "scale inertia...");
                VMenu* modelOperations = new VMenu();
                modelOperations->appendMenuAndDetach(minertia, true, "Shape mass and inertia");
                menu->appendMenuAndDetach(modelOperations, true, "Model operations");
            }
        }
    }
}
#endif
