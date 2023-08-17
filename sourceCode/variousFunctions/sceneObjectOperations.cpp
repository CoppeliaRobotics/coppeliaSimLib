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
    #include <qdlgconvexdecomposition.h>
    #include <guiApp.h>
#endif

#ifdef SIM_WITH_GUI
void CSceneObjectOperations::keyPress(int key)
{
    if (key==CTRL_V_KEY)
        processCommand(SCENE_OBJECT_OPERATION_PASTE_OBJECTS_SOOCMD);
    if ((key==DELETE_KEY)||(key==BACKSPACE_KEY))
        processCommand(SCENE_OBJECT_OPERATION_DELETE_OBJECTS_SOOCMD);
    if (key==CTRL_X_KEY)
        processCommand(SCENE_OBJECT_OPERATION_OBJECT_FULL_CUT_SOOCMD);
    if (key==CTRL_C_KEY)
        processCommand(SCENE_OBJECT_OPERATION_OBJECT_FULL_COPY_SOOCMD);
    if (key==ESC_KEY)
        processCommand(SCENE_OBJECT_OPERATION_DESELECT_OBJECTS_SOOCMD);
    if (key==CTRL_Y_KEY)
        processCommand(SCENE_OBJECT_OPERATION_REDO_SOOCMD);
    if (key==CTRL_Z_KEY)
        processCommand(SCENE_OBJECT_OPERATION_UNDO_SOOCMD);
    if (key==CTRL_A_KEY)
        processCommand(SCENE_OBJECT_OPERATION_SELECT_ALL_OBJECTS_SOOCMD);
}

bool CSceneObjectOperations::processCommand(int commandID)
{ // Return value is true if the command belonged to object edition menu and was executed
 // Can be called by the UI and SIM thread!
    
    if (commandID==SCENE_OBJECT_OPERATION_ASSEMBLE_SOOCMD)
    {
        // There is another such routine!! XXBFVGA
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            bool assembleEnabled=false;
            bool disassembleEnabled=false;
            size_t selS=App::currentWorld->sceneObjects->getSelectionCount();
            if (selS==1)
            { // here we can only have disassembly
                CSceneObject* it=App::currentWorld->sceneObjects->getLastSelectionObject();
                disassembleEnabled=(it->getParent()!=nullptr)&&(it->getAssemblyMatchValues(true).length()!=0);
                if (disassembleEnabled)
                {
                    App::logMsg(sim_verbosity_msgs,IDSN_DISASSEMBLING_OBJECT);
                    App::currentWorld->sceneObjects->setObjectParent(it,nullptr,true);
                }
            }
            else if (selS==2)
            { // here we can have assembly or disassembly
                CSceneObject* it1=App::currentWorld->sceneObjects->getLastSelectionObject();
                CSceneObject* it2=App::currentWorld->sceneObjects->getObjectFromHandle(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(0));
                if ((it1->getParent()==it2)||(it2->getParent()==it1))
                { // disassembly
                    if ( (it1->getParent()==it2) && (it1->getAssemblyMatchValues(true).length()!=0) )
                        disassembleEnabled=true;
                    if ( (it2->getParent()==it1) && (it2->getAssemblyMatchValues(true).length()!=0) )
                        disassembleEnabled=true;
                    if (disassembleEnabled)
                    {
                        App::logMsg(sim_verbosity_msgs,IDSN_DISASSEMBLING_OBJECT);
                        if (it1->getParent()==it2)
                        {
                            App::currentWorld->sceneObjects->setObjectParent(it1,nullptr,true);
                            App::currentWorld->sceneObjects->deselectObjects();
                            App::currentWorld->sceneObjects->addObjectToSelection(it1->getObjectHandle());
                            App::currentWorld->sceneObjects->addObjectToSelection(it2->getObjectHandle());
                        }
                        else
                            App::currentWorld->sceneObjects->setObjectParent(it2,nullptr,true);
                    }
                }
                else
                { // assembly
                    std::vector<CSceneObject*> potParents;
                    it1->getAllChildrenThatMayBecomeAssemblyParent(it2->getChildAssemblyMatchValuesPointer(),potParents);
                    bool directAssembly=it1->doesParentAssemblingMatchValuesMatchWithChild(it2->getChildAssemblyMatchValuesPointer());
                    if ( directAssembly||(potParents.size()==1) )
                    {
                        App::logMsg(sim_verbosity_msgs,IDSN_ASSEMBLING_2_OBJECTS);
                        assembleEnabled=true;
                        if (directAssembly)
                            App::currentWorld->sceneObjects->setObjectParent(it2,it1,true);
                        else
                            App::currentWorld->sceneObjects->setObjectParent(it2,potParents[0],true);
                        if (it2->getAssemblingLocalTransformationIsUsed())
                            it2->setLocalTransformation(it2->getAssemblingLocalTransformation());
                    }
                    else
                    { // here we might have the opposite of what we usually do to assemble (i.e. last selection should always be parent, but not here)
                        // we assemble anyways if the roles are unequivoque:
                        if ( it2->doesParentAssemblingMatchValuesMatchWithChild(it1->getChildAssemblyMatchValuesPointer()) )
                        {
                            App::logMsg(sim_verbosity_msgs,IDSN_ASSEMBLING_2_OBJECTS);
                            assembleEnabled=true;
                            App::currentWorld->sceneObjects->setObjectParent(it1,it2,true);
                            if (it1->getAssemblingLocalTransformationIsUsed())
                                it1->setLocalTransformation(it1->getAssemblingLocalTransformation());
                        }
                    }
                }
            }

            if (assembleEnabled||disassembleEnabled)
            {
                App::undoRedo_sceneChanged("");
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

    if (commandID==SCENE_OBJECT_OPERATION_TRANSFER_DNA_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            size_t selS=App::currentWorld->sceneObjects->getSelectionCount();
            CSceneObject* it=App::currentWorld->sceneObjects->getLastSelectionObject();
            if ( (selS==1)&&it->getModelBase() )
            {
                std::vector<CSceneObject*> clones;
                std::vector<CSceneObject*> toExplore;
                for (size_t i=0;i<App::currentWorld->sceneObjects->getOrphanCount();i++)
                    toExplore.push_back(App::currentWorld->sceneObjects->getOrphanFromIndex(i));
                while (toExplore.size()>0)
                {
                    CSceneObject* obj=toExplore[0];
                    toExplore.erase(toExplore.begin());
                    if (obj!=it)
                    {
                        if ( obj->getModelBase()&&(obj->getDnaString().compare(it->getDnaString())==0) )
                            clones.push_back(obj);
                        else
                            toExplore.insert(toExplore.end(),obj->getChildren()->begin(),obj->getChildren()->end());
                    }
                }

                std::vector<int> newSelection;
                if (clones.size()>0)
                {
                    App::logMsg(sim_verbosity_msgs,IDSN_TRANSFERRING_DNA_TO_CLONES);
                    App::worldContainer->copyBuffer->memorizeBuffer();

                    std::vector<int> sel;
                    sel.push_back(it->getObjectHandle());
                    App::currentWorld->sceneObjects->addModelObjects(sel);
                    std::string masterName(it->getObjectName_old());

                    App::worldContainer->copyBuffer->copyCurrentSelection(&sel,App::currentWorld->environment->getSceneLocked(),0);
                    App::currentWorld->sceneObjects->deselectObjects();
                    for (size_t i=0;i<clones.size();i++)
                    {
                        std::string name(clones[i]->getObjectName_old());
                        std::string altName(clones[i]->getObjectAltName_old());
                        std::vector<int> objs;
                        objs.push_back(clones[i]->getObjectHandle());
                        App::currentWorld->sceneObjects->addModelObjects(objs);
                        C7Vector tr(clones[i]->getLocalTransformation());
                        CSceneObject* parent(clones[i]->getParent());
                        int order=App::currentWorld->sceneObjects->getObjectSequence(clones[i]);
                        App::currentWorld->sceneObjects->eraseObjects(objs,true);
                        App::worldContainer->copyBuffer->pasteBuffer(App::currentWorld->environment->getSceneLocked(),2);
                        CSceneObject* newObj=App::currentWorld->sceneObjects->getLastSelectionObject();
                        App::currentWorld->sceneObjects->deselectObjects();
                        newSelection.push_back(newObj->getObjectHandle());
                        App::currentWorld->sceneObjects->setObjectParent(newObj,parent,true);
                        App::currentWorld->sceneObjects->setObjectSequence(newObj,order);
                        newObj->setLocalTransformation(tr);

                        std::string autoName(newObj->getObjectName_old());
                        int suffixNb=tt::getNameSuffixNumber(autoName.c_str(),true);
                        name=tt::getNameWithoutSuffixNumber(name.c_str(),true);
                        if (suffixNb>=0)
                            name+="#"+std::to_string(suffixNb);
                        App::currentWorld->sceneObjects->setObjectName_old(newObj,name.c_str(),true);
                        App::currentWorld->sceneObjects->setObjectAltName_old(newObj,altName.c_str(),true);
                    }
                    App::worldContainer->copyBuffer->restoreBuffer();
                    App::worldContainer->copyBuffer->clearMemorizedBuffer();
                    App::logMsg(sim_verbosity_msgs,"done.");
                    std::string txt;
                    txt+=boost::lexical_cast<std::string>(clones.size())+IDSN_X_CLONES_WERE_UPDATED;
                    App::logMsg(sim_verbosity_msgs,txt.c_str());

                    for (size_t i=0;i<newSelection.size();i++)
                        App::currentWorld->sceneObjects->addObjectToSelection(newSelection[i]);

                    App::undoRedo_sceneChanged("");
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

    if ( (commandID==SCENE_OBJECT_OPERATION_MAKE_PARENT_SOOCMD)||(commandID==SCENE_OBJECT_OPERATION_MAKE_PARENT_AND_MOVE_SOOCMD) )
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            if (sel.size()>1)
            {
                CSceneObject* last=App::currentWorld->sceneObjects->getObjectFromHandle(sel[sel.size()-1]);
                for (size_t i=0;i<sel.size()-1;i++)
                {
                    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(sel[i]);
                    App::currentWorld->sceneObjects->setObjectParent(it,last,commandID==SCENE_OBJECT_OPERATION_MAKE_PARENT_SOOCMD);
                }
                App::currentWorld->sceneObjects->selectObject(last->getObjectHandle()); // We select the parent

                App::undoRedo_sceneChanged("");
                std::string txt("Setting object '");
                txt+=last->getObjectAlias_printPath()+"' parent...";
                App::logMsg(sim_verbosity_msgs,txt.c_str());
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

    if (commandID==SCENE_OBJECT_OPERATION_MAKE_ORPHANS_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            App::logMsg(sim_verbosity_msgs,"Setting object(s) parent-less...");
            for (size_t i=0;i<sel.size();i++)
            {
                CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(sel[i]);
                App::currentWorld->sceneObjects->setObjectParent(it,nullptr,true);
            }
            App::undoRedo_sceneChanged("");
            App::currentWorld->sceneObjects->deselectObjects(); // We clear selection
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
    if (commandID==SCENE_OBJECT_OPERATION_MORPH_INTO_CONVEX_SHAPES_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<CSceneObject*> sel;
            App::currentWorld->sceneObjects->getSelectedObjects(sel,sim_object_shape_type,true,true);

            App::currentWorld->sceneObjects->deselectObjects();
            std::vector<int> toSelect;
            App::logMsg(sim_verbosity_msgs,"Morphing into convex shape(s)...");
            GuiApp::uiThread->showOrHideProgressBar(true,-1,"Morphing into convex shape(s)...");
            for (size_t i=0;i<sel.size();i++)
            {
                CShape* it=(CShape*)sel[i];
                if (!it->getMesh()->isConvex())
                {
                    C7Vector obbTr(it->getCumulativeTransformation()*it->getBB(nullptr));
                    double mass=it->getMesh()->getMass();
                    C3Vector com(it->getMesh()->getCOM());
                    C3X3Matrix inertia(it->getMesh()->getInertia());
                    if (it->isCompound())
                    { // ungroup, then group again
                        C7Vector tr(it->getCumulativeTransformation());
                        std::vector<CShape*> newShapes;
                        _fullUngroupShape(it,newShapes);
                        newShapes.push_back(it);
                        for (size_t j=0;j<newShapes.size();j++)
                        {
                            CMesh* convexHull=_generateConvexHull(newShapes[j]->getObjectHandle());
                            if (convexHull!=nullptr)
                                newShapes[j]->replaceMesh(convexHull,true);
                        }
                        _groupShapes(newShapes);
                        it->relocateFrame("custom",&tr);
                        it->getMesh()->setMass(mass);
                        it->getMesh()->setCOM(com);
                        it->getMesh()->setInertia(inertia);
                    }
                    else
                    {
                        CMesh* convexHull=_generateConvexHull(it->getObjectHandle());
                        if (convexHull!=nullptr)
                            it->replaceMesh(convexHull,true);
                    }
                    it->alignBB("custom",&obbTr);
                    toSelect.push_back(it->getObjectHandle());
                }
            }
            App::currentWorld->sceneObjects->setSelectedObjectHandles(&toSelect);
            App::undoRedo_sceneChanged("");
            GuiApp::uiThread->showOrHideProgressBar(false);
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

    if (commandID==SCENE_OBJECT_OPERATION_DECIMATE_SHAPE_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<CSceneObject*> sel;
            App::currentWorld->sceneObjects->getSelectedObjects(sel,sim_object_shape_type,true,true);

            int totalTriangles=0;
            for (size_t i=0;i<sel.size();i++)
            {
                CShape* it=(CShape*)sel[i];
                totalTriangles+=it->getMesh()->countTriangles();
            }

            double percentageToKeep=0.0;
            SUIThreadCommand cmdIn;
            SUIThreadCommand cmdOut;
            cmdIn.cmdId=DISPLAY_MESH_DECIMATION_DIALOG_UITHREADCMD;
            cmdIn.intParams.push_back(totalTriangles);
            cmdIn.floatParams.push_back(0.2);
            GuiApp::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
            if ( (cmdOut.boolParams.size()>0)&&cmdOut.boolParams[0] )
                percentageToKeep=cmdOut.floatParams[0];
            App::currentWorld->sceneObjects->deselectObjects();
            if (percentageToKeep>0.0)
            {
                App::logMsg(sim_verbosity_msgs,"Decimating shape(s)...");
                GuiApp::uiThread->showOrHideProgressBar(true,-1,"Decimating shape(s)...");
                std::vector<int> toSelect;
                for (size_t i=0;i<sel.size();i++)
                {
                    CShape* it=(CShape*)sel[i];
                    C7Vector obbTr(it->getCumulativeTransformation()*it->getBB(nullptr));
                    double mass=it->getMesh()->getMass();
                    C3Vector com(it->getMesh()->getCOM());
                    C3X3Matrix inertia(it->getMesh()->getInertia());
                    bool success=false;
                    if (it->isCompound())
                    { // ungroup, then group again
                        C7Vector tr(it->getCumulativeTransformation());
                        std::vector<CShape*> newShapes;
                        _fullUngroupShape(it,newShapes);
                        newShapes.push_back(it);
                        for (size_t j=0;j<newShapes.size();j++)
                        {
                            std::vector<double> vert;
                            std::vector<int> ind;
                            CShape* it2=newShapes[j];
                            C7Vector tr(it2->getCumulativeTransformation());
                            it2->getMesh()->getCumulativeMeshes(tr,vert,&ind,nullptr);
                            std::vector<double> vertOut;
                            std::vector<int> indOut;
                            if (CMeshRoutines::getDecimatedMesh(vert,ind,percentageToKeep,vertOut,indOut,App::userSettings->identicalVertexTolerance))
                            { // decimation algo was successful:
                                CMesh* mesh=new CMesh(tr,vertOut,indOut,nullptr,nullptr,0);
                                it2->replaceMesh(mesh,true);
                            }
                        }
                        _groupShapes(newShapes);
                        it->relocateFrame("custom",&tr);
                        it->getMesh()->setMass(mass);
                        it->getMesh()->setCOM(com);
                        it->getMesh()->setInertia(inertia);
                        success=true;
                    }
                    else
                    {
                        std::vector<double> vert;
                        std::vector<int> ind;
                        C7Vector tr(it->getCumulativeTransformation());
                        it->getMesh()->getCumulativeMeshes(tr,vert,&ind,nullptr);
                        std::vector<double> vertOut;
                        std::vector<int> indOut;
                        if (CMeshRoutines::getDecimatedMesh(vert,ind,percentageToKeep,vertOut,indOut,App::userSettings->identicalVertexTolerance))
                        { // decimation algo was successful:
                            CMesh* mesh=new CMesh(tr,vertOut,indOut,nullptr,nullptr,0);
                            it->replaceMesh(mesh,true);
                            success=true;
                        }
                    }
                    if (success)
                    {
                        it->alignBB("custom",&obbTr);
                        toSelect.push_back(it->getObjectHandle());
                    }
                }
                App::currentWorld->sceneObjects->setSelectedObjectHandles(&toSelect);
                App::undoRedo_sceneChanged("");
                GuiApp::uiThread->showOrHideProgressBar(false);
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

    if (commandID==SCENE_OBJECT_OPERATION_MORPH_INTO_CONVEX_DECOMPOSITION_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<CSceneObject*> sel;
            App::currentWorld->sceneObjects->getSelectedObjects(sel,sim_object_shape_type,true,true);
            SUIThreadCommand cmdIn;
            SUIThreadCommand cmdOut;
            cmdIn.cmdId=DISPLAY_CONVEX_DECOMPOSITION_DIALOG_UITHREADCMD;
            GuiApp::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
            bool addExtraDistPoints=cmdOut.boolParams[0];
            bool addFacesPoints=cmdOut.boolParams[1];
            int nClusters=cmdOut.intParams[0];
            int maxHullVertices=cmdOut.intParams[1];
            double maxConcavity=cmdOut.floatParams[0];
            double smallClusterThreshold=cmdOut.floatParams[1];
            int maxTrianglesInDecimatedMesh=cmdOut.intParams[2];
            double maxConnectDist=cmdOut.floatParams[2];
            bool cancel=cmdOut.boolParams[4];
            bool useHACD=cmdOut.boolParams[5];
            bool pca=cmdOut.boolParams[6];
            bool voxelBased=cmdOut.boolParams[7];
            int resolution=cmdOut.intParams[4];
            int depth=cmdOut.intParams[5];
            int planeDownsampling=cmdOut.intParams[6];
            int convexHullDownsampling=cmdOut.intParams[7];
            int maxNumVerticesPerCH=cmdOut.intParams[8];
            double concavity=cmdOut.floatParams[3];
            double alpha=cmdOut.floatParams[4];
            double beta=cmdOut.floatParams[5];
            double gamma=cmdOut.floatParams[6];
            double minVolumePerCH=cmdOut.floatParams[7];
            App::currentWorld->sceneObjects->deselectObjects();
            if (!cancel)
            {
                App::logMsg(sim_verbosity_msgs,"Morphing into convex decomposed shape(s)...");
                GuiApp::uiThread->showOrHideProgressBar(true,-1,"Morphing into convex decomposed shape(s)...");
                std::vector<int> toSelect;
                for (size_t i=0;i<sel.size();i++)
                {
                    CShape* it=(CShape*)sel[i];
                    if (!it->getMesh()->isConvex())
                    {
                        CShape* morphedShape=_morphToConvexDecomposed(it,nClusters,maxConcavity,addExtraDistPoints,addFacesPoints,
                                                                    maxConnectDist,maxTrianglesInDecimatedMesh,maxHullVertices,
                                                                    smallClusterThreshold,
                                                                    useHACD,resolution,depth,concavity,planeDownsampling,
                                                                    convexHullDownsampling,alpha,beta,gamma,pca,voxelBased,
                                                                    maxNumVerticesPerCH,minVolumePerCH);
                        if (morphedShape!=nullptr)
                            toSelect.push_back(morphedShape->getObjectHandle());
                    }
                }
                App::currentWorld->sceneObjects->setSelectedObjectHandles(&toSelect);
                App::logMsg(sim_verbosity_msgs,"done.");
                GuiApp::uiThread->showOrHideProgressBar(false);
                App::undoRedo_sceneChanged("");
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

    if (commandID==SCENE_OBJECT_OPERATION_COMPUTE_INERTIA_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<CSceneObject*> sel;
            App::currentWorld->sceneObjects->getSelectedObjects(sel,sim_object_shape_type,true,false);
            App::currentWorld->sceneObjects->deselectObjects();
            bool ok;
            double density=0.0;
            ok=GuiApp::uiThread->dialogInputGetFloat(GuiApp::mainWindow,"Body density","Uniform density",1000.05,0.1,30000,1,&density);
            if (ok)
            {
                App::logMsg(sim_verbosity_msgs,"Computing mass and inertia...");
                GuiApp::uiThread->showOrHideProgressBar(true,-1,"Computing mass and inertia...");
                std::vector<int> toSelect;
                for (size_t i=0;i<sel.size();i++)
                {
                    CShape* it=(CShape*)sel[i];
                    if (!it->getStatic())
                    {
                        toSelect.push_back(it->getObjectHandle());
                        it->computeMassAndInertia(density);
                    }
                }
                App::currentWorld->sceneObjects->setSelectedObjectHandles(&toSelect);
                App::logMsg(sim_verbosity_msgs,"done.");
                GuiApp::uiThread->showOrHideProgressBar(false);
                App::undoRedo_sceneChanged("");
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

    if (commandID==SCENE_OBJECT_OPERATION_SCALE_MASS_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<CSceneObject*> sel;
            App::currentWorld->sceneObjects->getSelectedObjects(sel,sim_object_shape_type,true,false);
            App::currentWorld->sceneObjects->deselectObjects();
            bool ok;
            double fact=0.0;
            ok=GuiApp::uiThread->dialogInputGetFloat(GuiApp::mainWindow,"Mass scaling","Scaling factor",2.0,0.1,10.0,2,&fact);
            if (ok)
            {
                App::logMsg(sim_verbosity_msgs,"Scaling mass...");
                std::vector<int> toSelect;
                for (size_t i=0;i<sel.size();i++)
                {
                    CShape* it=(CShape*)sel[i];
                    if (!it->getStatic())
                    {
                        toSelect.push_back(it->getObjectHandle());
                        it->getMesh()->setMass(it->getMesh()->getMass()*fact);
                    }
                }
                App::currentWorld->sceneObjects->setSelectedObjectHandles(&toSelect);
                App::logMsg(sim_verbosity_msgs,"done.");
                App::undoRedo_sceneChanged("");
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

    if (commandID==SCENE_OBJECT_OPERATION_SCALE_INERTIA_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<CSceneObject*> sel;
            App::currentWorld->sceneObjects->getSelectedObjects(sel,sim_object_shape_type,true,false);
            App::currentWorld->sceneObjects->deselectObjects();
            bool ok;
            double fact=0.0;
            ok=GuiApp::uiThread->dialogInputGetFloat(GuiApp::mainWindow,"Inertia scaling","Scaling factor",2.0,0.1,10.0,2,&fact);
            if (ok)
            {
                App::logMsg(sim_verbosity_msgs,"Scaling inertia...");
                std::vector<int> toSelect;
                for (size_t i=0;i<sel.size();i++)
                {
                    CShape* it=(CShape*)sel[i];
                    if (!it->getStatic())
                    {
                        toSelect.push_back(it->getObjectHandle());
                        it->getMesh()->setInertia(it->getMesh()->getInertia()*fact);
                    }
                }
                App::currentWorld->sceneObjects->setSelectedObjectHandles(&toSelect);
                App::logMsg(sim_verbosity_msgs,"done.");
                App::undoRedo_sceneChanged("");
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

    if (commandID==SCENE_OBJECT_OPERATION_SELECT_ALL_OBJECTS_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs,IDSNS_SELECTING_ALL_OBJECTS);
            for (size_t i=0;i<App::currentWorld->sceneObjects->getObjectCount();i++)
                App::currentWorld->sceneObjects->addObjectToSelection(App::currentWorld->sceneObjects->getObjectFromIndex(i)->getObjectHandle());
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

    if (commandID==SCENE_OBJECT_OPERATION_REMOVE_ASSOCIATED_CHILD_SCRIPT_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            int id=App::currentWorld->sceneObjects->getLastSelectionHandle();
            CScriptObject* script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,id);
            if (script!=nullptr)
            {
                if (GuiApp::mainWindow!=nullptr)
                    GuiApp::mainWindow->codeEditorContainer->closeFromScriptHandle(script->getScriptHandle(),nullptr,true);
                App::currentWorld->embeddedScriptContainer->removeScript(script->getScriptHandle());
                App::undoRedo_sceneChanged("");
                GuiApp::setFullDialogRefreshFlag();
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

    if (commandID==SCENE_OBJECT_OPERATION_REMOVE_ASSOCIATED_CUSTOMIZATION_SCRIPT_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            int id=App::currentWorld->sceneObjects->getLastSelectionHandle();
            CScriptObject* script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,id);
            if (script!=nullptr)
            {
                if (GuiApp::mainWindow!=nullptr)
                    GuiApp::mainWindow->codeEditorContainer->closeFromScriptHandle(script->getScriptHandle(),nullptr,true);
                App::currentWorld->embeddedScriptContainer->removeScript(script->getScriptHandle());
                App::undoRedo_sceneChanged("");
                GuiApp::setFullDialogRefreshFlag();
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

    if (commandID==SCENE_OBJECT_OPERATION_DESELECT_OBJECTS_SOOCMD)
    {
        App::currentWorld->sceneObjects->deselectObjects();
    }

    if (commandID==SCENE_OBJECT_OPERATION_OBJECT_FULL_COPY_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            App::logMsg(sim_verbosity_msgs,IDSNS_COPYING_SELECTION);
            GuiApp::uiThread->showOrHideProgressBar(true,-1.0,"Copying objects...");
            _copyObjects(&sel);
            GuiApp::uiThread->showOrHideProgressBar(false);
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
    if (commandID==SCENE_OBJECT_OPERATION_OBJECT_FULL_CUT_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
            {
                CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
                if ((it->getObjectProperty()&sim_objectproperty_cannotdelete)==0)
                {
                    if ( ((it->getObjectProperty()&sim_objectproperty_cannotdeleteduringsim)==0)||App::currentWorld->simulation->isSimulationStopped() )
                        sel.push_back(it->getObjectHandle());
                }
            }
            if (sel.size()>0)
            {
                App::logMsg(sim_verbosity_msgs,IDSNS_CUTTING_SELECTION);
                GuiApp::uiThread->showOrHideProgressBar(true,-1.0,"Cutting objects...");
                App::currentWorld->sceneObjects->addModelObjects(sel);
                _copyObjects(&sel);
                _deleteObjects(&sel);
                App::currentWorld->sceneObjects->deselectObjects(); // We clear selection
                GuiApp::uiThread->showOrHideProgressBar(false);
                App::undoRedo_sceneChanged("");
                App::logMsg(sim_verbosity_msgs,"done.");
            }
            App::currentWorld->sceneObjects->deselectObjects();
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==SCENE_OBJECT_OPERATION_PASTE_OBJECTS_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            App::logMsg(sim_verbosity_msgs,IDSNS_PASTING_BUFFER);

            GuiApp::uiThread->showOrHideProgressBar(true,-1.0,"Pasting objects...");
            bool failed=(App::worldContainer->copyBuffer->pasteBuffer(App::currentWorld->environment->getSceneLocked(),3)==-1);
            GuiApp::uiThread->showOrHideProgressBar(false);
            if (failed) // Error: trying to copy locked buffer into unlocked scene!
                GuiApp::uiThread->messageBox_warning(GuiApp::mainWindow,"Paste",IDS_SCENE_IS_LOCKED_CANNOT_PASTE_WARNING,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
            else
                App::logMsg(sim_verbosity_msgs,"done.");
            App::undoRedo_sceneChanged("");
        }
        else
        { // We are in the UI thread. We execute the command in a delayed manner:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==SCENE_OBJECT_OPERATION_DELETE_OBJECTS_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
            {
                CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
                if ((it->getObjectProperty()&sim_objectproperty_cannotdelete)==0)
                {
                    if ( ((it->getObjectProperty()&sim_objectproperty_cannotdeleteduringsim)==0)||App::currentWorld->simulation->isSimulationStopped() )
                        sel.push_back(it->getObjectHandle());
                }
            }
            if (sel.size()>0)
            {
                App::logMsg(sim_verbosity_msgs,IDSNS_DELETING_SELECTION);
                GuiApp::uiThread->showOrHideProgressBar(true,-1.0,"Deleting objects...");
                _deleteObjects(&sel);
                GuiApp::uiThread->showOrHideProgressBar(false);
                App::undoRedo_sceneChanged("");
                App::logMsg(sim_verbosity_msgs,"done.");
            }
            App::currentWorld->sceneObjects->deselectObjects();
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if ( (commandID==SCENE_OBJECT_OPERATION_RELOCATE_FRAME_TO_ORIGIN_SOOCMD)||(commandID==SCENE_OBJECT_OPERATION_RELOCATE_FRAME_TO_PARENT_SOOCMD)||(commandID==SCENE_OBJECT_OPERATION_RELOCATE_FRAME_TO_CENTER_SOOCMD) )
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<CSceneObject*> sel;
            App::currentWorld->sceneObjects->getSelectedObjects(sel,sim_object_shape_type,true,true);
            if (commandID==SCENE_OBJECT_OPERATION_RELOCATE_FRAME_TO_ORIGIN_SOOCMD)
                App::logMsg(sim_verbosity_msgs,"Relocating reference frame to world origin...");
            if (commandID==SCENE_OBJECT_OPERATION_RELOCATE_FRAME_TO_PARENT_SOOCMD)
                App::logMsg(sim_verbosity_msgs,"Relocating reference frame to parent origin...");
            if (commandID==SCENE_OBJECT_OPERATION_RELOCATE_FRAME_TO_CENTER_SOOCMD)
                App::logMsg(sim_verbosity_msgs,"relocating reference frame to mesh center...");
            bool success=true;
            std::vector<int> toSelect;
            for (size_t i=0;i<sel.size();i++)
            {
                CShape* theShape=(CShape*)sel[i];
                bool r=false;
                if (commandID==SCENE_OBJECT_OPERATION_RELOCATE_FRAME_TO_ORIGIN_SOOCMD)
                    r=theShape->relocateFrame("world");
                if (commandID==SCENE_OBJECT_OPERATION_RELOCATE_FRAME_TO_PARENT_SOOCMD)
                    r=theShape->relocateFrame("parent");
                if (commandID==SCENE_OBJECT_OPERATION_RELOCATE_FRAME_TO_CENTER_SOOCMD)
                    r=theShape->relocateFrame("mesh");
                if (r)
                    toSelect.push_back(theShape->getObjectHandle());
                success=r&&success;
            }
            App::currentWorld->sceneObjects->setSelectedObjectHandles(&toSelect);
            App::undoRedo_sceneChanged("");
            if (success)
                App::logMsg(sim_verbosity_msgs,"done.");
            else
                App::logMsg(sim_verbosity_warnings,"One or more reference frames could not be relocated.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if ((commandID==SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_MESH_SOOCMD)||(commandID==SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_WORLD_SOOCMD) )
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<CSceneObject*> sel;
            App::currentWorld->sceneObjects->getSelectedObjects(sel,sim_object_shape_type,true,true);
            if (commandID==SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_MESH_SOOCMD)
                App::logMsg(sim_verbosity_msgs,"aligning bounding box with mesh...");
            if (commandID==SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_WORLD_SOOCMD)
                App::logMsg(sim_verbosity_msgs,"aligning bounding box with world...");
            bool success=true;
            std::vector<int> toSelect;
            for (size_t i=0;i<sel.size();i++)
            {
                CShape* theShape=(CShape*)sel[i];
                bool r=false;
                if (commandID==SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_MESH_SOOCMD)
                    r=theShape->alignBB("mesh");
                if (commandID==SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_WORLD_SOOCMD)
                    r=theShape->alignBB("world");
                if (r)
                    toSelect.push_back(theShape->getObjectHandle());
                success=r&&success;
            }
            App::currentWorld->sceneObjects->setSelectedObjectHandles(&toSelect);
            App::undoRedo_sceneChanged("");
            if (success)
                App::logMsg(sim_verbosity_msgs,"done.");
            else
                App::logMsg(sim_verbosity_warnings,"One or more bounding boxes could not be reoriented.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==SCENE_OBJECT_OPERATION_GROUP_SHAPES_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            App::currentWorld->sceneObjects->getSelectedObjectHandles(sel,sim_object_shape_type,true,true);
            App::logMsg(sim_verbosity_msgs,"Grouping shapes...");
            groupSelection(&sel);
            App::undoRedo_sceneChanged("");
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

    if (commandID==SCENE_OBJECT_OPERATION_UNGROUP_SHAPES_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            App::currentWorld->sceneObjects->getSelectedObjectHandles(sel,sim_object_shape_type,true,true);
            App::logMsg(sim_verbosity_msgs,"Ungrouping shapes...");
            ungroupSelection(&sel);
            App::undoRedo_sceneChanged("");
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

    if (commandID==SCENE_OBJECT_OPERATION_MERGE_SHAPES_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            App::currentWorld->sceneObjects->getSelectedObjectHandles(sel,sim_object_shape_type,true,true);
            App::logMsg(sim_verbosity_msgs,"Merging shapes...");
            mergeSelection(&sel);
            App::undoRedo_sceneChanged("");
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
    if (commandID==SCENE_OBJECT_OPERATION_DIVIDE_SHAPES_SOOCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            App::currentWorld->sceneObjects->getSelectedObjectHandles(sel,sim_object_shape_type,true,true);
            App::logMsg(sim_verbosity_msgs,"Dividing shapes...");
            divideSelection(&sel);
            App::undoRedo_sceneChanged("");
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

    if (commandID==SCENE_OBJECT_OPERATION_UNDO_SOOCMD)
    {
        if (GuiApp::getEditModeType()==NO_EDIT_MODE)
        {
            if (!VThread::isUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                App::logMsg(sim_verbosity_msgs,IDSNS_EXECUTING_UNDO);
                App::currentWorld->undoBufferContainer->undo();
                App::logMsg(sim_verbosity_msgs,"done.");
            }
            else
            { // We are in the UI thread. Execute the command via the main thread:
                SSimulationThreadCommand cmd;
                cmd.cmdId=commandID;
                App::appendSimulationThreadCommand(cmd);
            }
        }
        return(true);
    }

    if (commandID==SCENE_OBJECT_OPERATION_REDO_SOOCMD)
    {
        if (GuiApp::getEditModeType()==NO_EDIT_MODE)
        {
            if (!VThread::isUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                App::logMsg(sim_verbosity_msgs,IDSNS_EXECUTING_REDO);
                App::currentWorld->undoBufferContainer->redo();
                App::logMsg(sim_verbosity_msgs,"done.");
            }
            else
            { // We are in the UI thread. Execute the command via the main thread:
                SSimulationThreadCommand cmd;
                cmd.cmdId=commandID;
                App::appendSimulationThreadCommand(cmd);
            }
        }
        return(true);
    }
    return(false);
}
#endif
void CSceneObjectOperations::_copyObjects(std::vector<int>* selection)
{
    // We first copy the selection:
    std::vector<int> sel(*selection);
    App::currentWorld->sceneObjects->addModelObjects(sel);
    App::worldContainer->copyBuffer->copyCurrentSelection(&sel,App::currentWorld->environment->getSceneLocked(),0);
    App::currentWorld->sceneObjects->deselectObjects(); // We clear selection
}

void CSceneObjectOperations::_deleteObjects(std::vector<int>* selection)
{ // There are a few other spots where objects get deleted (e.g. the C-interface)
    TRACE_INTERNAL;
    App::currentWorld->sceneObjects->addModelObjects(selection[0]);
    App::currentWorld->sceneObjects->eraseObjects(selection[0],true);
    App::currentWorld->sceneObjects->deselectObjects();
}

int CSceneObjectOperations::groupSelection(std::vector<int>* selection)
{
    if (selection->size()<2)
        return(-1);

    std::vector<CShape*> shapesToGroup;
    for (size_t i=0;i<selection->size();i++)
    {
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(selection->at(i));
        shapesToGroup.push_back(it);
    }

    App::currentWorld->sceneObjects->deselectObjects();

    CShape* compoundShape=_groupShapes(shapesToGroup);

    App::currentWorld->sceneObjects->selectObject(compoundShape->getObjectHandle());

    return(compoundShape->getObjectHandle());

}

CShape* CSceneObjectOperations::_groupShapes(const std::vector<CShape*>& shapesToGroup)
{ // returned shape is the last shape in the selection (it is modified, others are destroyed)
    size_t pureCount=0; // except for heightfields
    for (size_t i=0;i<shapesToGroup.size();i++)
    {
        CShape* it=shapesToGroup[i];
        if (it->getMesh()->isPure())
        {
            CMesh* m=it->getSingleMesh();
            if (m==nullptr)
                pureCount++; // pure compound
            else
            {
                if (m->getPurePrimitiveType()!=sim_primitiveshape_heightfield)
                    pureCount++; // not a heightfield
            }
        }
    }
    bool allToNonPure=(pureCount<shapesToGroup.size());
    std::vector<CMeshWrapper*> allMeshes;
    for (size_t i=0;i<shapesToGroup.size();i++)
    {
        CShape* it=shapesToGroup[i];
        if (allToNonPure)
            it->getMesh()->setPurePrimitiveType(sim_primitiveshape_none,1.0,1.0,1.0); // this will be propagated to all geometrics!
        allMeshes.push_back(it->getMesh());
        it->detachMesh();
        App::currentWorld->drawingCont->announceObjectWillBeErased(it);
        App::currentWorld->pointCloudCont->announceObjectWillBeErased(it->getObjectHandle());
        App::currentWorld->bannerCont->announceObjectWillBeErased(it->getObjectHandle());
    }

    CShape* lastSel=shapesToGroup[shapesToGroup.size()-1];

    CMeshWrapper* newWrapper=new CMeshWrapper();
    for (size_t i=0;i<allMeshes.size();i++)
    {
        CMeshWrapper* mesh=allMeshes[i];
        mesh->setName(shapesToGroup[i]->getObjectAlias().c_str());
        mesh->setIFrame(lastSel->getCumulativeTransformation().getInverse()*shapesToGroup[i]->getCumulativeTransformation()*mesh->getIFrame());
        newWrapper->addItem(mesh);
    }

    lastSel->replaceMesh(newWrapper,false);
    App::currentWorld->textureContainer->updateAllDependencies();

    std::vector<int> shapesToErase;
    for (size_t i=0;i<shapesToGroup.size()-1;i++)
        shapesToErase.push_back(shapesToGroup[i]->getObjectHandle());
    App::currentWorld->sceneObjects->eraseObjects(shapesToErase,true);

    return(lastSel);
}

void CSceneObjectOperations::ungroupSelection(std::vector<int>* selection)
{
    std::vector<int> newObjectHandles;
    App::currentWorld->sceneObjects->deselectObjects();
    std::vector<int> finalSel;
    for (size_t i=0;i<selection->size();i++)
    {
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(selection->at(i));
        if ( (it!=nullptr)&&it->isCompound() )
        {
            std::vector<CShape*> newShapes;
            _ungroupShape(it,newShapes);
            for (size_t j=0;j<newShapes.size();j++)
            {
                newObjectHandles.push_back(newShapes[j]->getObjectHandle());
                finalSel.push_back(newShapes[j]->getObjectHandle());
            }
            finalSel.push_back(it->getObjectHandle());
        }
    }

    selection->clear();
    for (size_t i=0;i<finalSel.size();i++)
    {
        App::currentWorld->sceneObjects->addObjectToSelection(finalSel[i]);
        selection->push_back(finalSel[i]);
    }

    if (newObjectHandles.size()>0)
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->createStack();
        stack->pushTableOntoStack();
        stack->pushStringOntoStack("objectHandles",0);
        stack->pushInt32ArrayOntoStack(&newObjectHandles[0],newObjectHandles.size());
        stack->insertDataIntoStackTable();
        App::worldContainer->callScripts(sim_syscb_aftercreate,stack,nullptr);
        App::worldContainer->interfaceStackContainer->destroyStack(stack);
    }
}

void CSceneObjectOperations::_fullUngroupShape(CShape* it,std::vector<CShape*>& newShapes)
{
    while (it->isCompound())
    {
        _ungroupShape(it,newShapes);
        for (size_t i=0;i<newShapes.size();i++)
        {
            while (newShapes[i]->isCompound())
                _ungroupShape(newShapes[i],newShapes);
        }
    }
}

void CSceneObjectOperations::CSceneObjectOperations::_ungroupShape(CShape* it,std::vector<CShape*>& newShapes)
{
    // added because a previous bug: (2014)
    if (!it->getMesh()->isPure())
        it->getMesh()->setPurePrimitiveType(sim_primitiveshape_none,1.0,1.0,1.0);

    // we have to remove all attached drawing objects
    App::currentWorld->drawingCont->announceObjectWillBeErased(it);
    App::currentWorld->pointCloudCont->announceObjectWillBeErased(it->getObjectHandle());
    App::currentWorld->bannerCont->announceObjectWillBeErased(it->getObjectHandle());

    CMeshWrapper* wrapper=it->getMesh();
    C7Vector oldTransf(it->getCumulativeTransformation());
    C7Vector oldParentTransf(it->getFullParentCumulativeTransformation());
    it->detachMesh();
    std::vector<CMeshWrapper*> meshes;
    for (size_t i=0;i<wrapper->childList.size();i++)
    {
        CMeshWrapper* mesh=wrapper->childList[i];
        C7Vector newTransf(oldTransf*mesh->getIFrame());
        mesh->setIFrame(C7Vector::identityTransformation);
        if (i==wrapper->childList.size()-1)
        {
            it->replaceMesh(mesh,false);
            it->setLocalTransformation(oldParentTransf.getInverse()*newTransf);
            for (size_t j=0;j<it->getChildCount();j++)
            { // Adjust children for the frame change
                CSceneObject* child=it->getChildFromIndex(j);
                child->setLocalTransformation(newTransf.getInverse()*oldTransf*child->getLocalTransformation());
            }
        }
        else
        {
            CShape* shape=new CShape();
            it->copyAttributesTo(shape);
            shape->replaceMesh(mesh,false);
            shape->setLocalTransformation(newTransf);
            App::currentWorld->sceneObjects->addObjectToScene(shape,false,false);
            App::currentWorld->sceneObjects->setObjectParent(shape,it->getParent(),true);
            App::currentWorld->sceneObjects->setObjectAlias(shape,mesh->getName().c_str(),true);
            newShapes.push_back(shape);
        }
    }
    wrapper->detachItems();
    delete wrapper;
    App::currentWorld->textureContainer->updateAllDependencies();
}

int CSceneObjectOperations::mergeSelection(std::vector<int>* selection)
{
    int retVal=-1;
    std::vector<CShape*> shapesToMerge;
    for (size_t i=0;i<selection->size();i++)
    {
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(selection->at(i));
        if (it!=nullptr)
            shapesToMerge.push_back(it);
    }
    App::currentWorld->sceneObjects->deselectObjects();
    if (shapesToMerge.size()>=2)
    {
        CShape* mergedShape=_mergeShapes(shapesToMerge);
        retVal=mergedShape->getObjectHandle();
        App::currentWorld->sceneObjects->selectObject(retVal);
    }
    return(retVal);
}

CShape* CSceneObjectOperations::_mergeShapes(const std::vector<CShape*>& allShapes)
{ // returned shape is the last shape in the selection (it is modified, others are destroyed)
    for (size_t i=0;i<allShapes.size();i++)
    {
        CShape* it=allShapes[i];
        if (it->getMesh()->getTextureCount()!=0)
        {
            App::currentWorld->textureContainer->announceGeneralObjectWillBeErased(it->getObjectHandle(),-1);
            it->getMesh()->removeAllTextures();
        }
    }

    CShape* lastSel=allShapes[allShapes.size()-1];

    std::vector<CMeshWrapper*> allMeshes;
    for (size_t i=0;i<allShapes.size()-1;i++)
    {
        CShape* it=allShapes[i];
        allMeshes.push_back(it->getMesh());
        it->detachMesh();
        App::currentWorld->drawingCont->announceObjectWillBeErased(it);
        App::currentWorld->pointCloudCont->announceObjectWillBeErased(it->getObjectHandle());
        App::currentWorld->bannerCont->announceObjectWillBeErased(it->getObjectHandle());
    }
    allMeshes.push_back(allShapes[allShapes.size()-1]->getMesh());

    lastSel=allShapes[allShapes.size()-1];

    std::vector<double> vertices;
    std::vector<int> indices;
    std::vector<double> normals;
    for (size_t i=0;i<allMeshes.size();i++)
    {
        CMeshWrapper* mesh=allMeshes[i];
        mesh->getCumulativeMeshes(allShapes[i]->getCumulativeTransformation(),vertices,&indices,&normals);
    }
    CMesh* newMesh=new CMesh(lastSel->getCumulativeTransformation(),vertices,indices,&normals,nullptr,0);
    lastSel->replaceMesh(newMesh,true);

    std::vector<int> shapesToErase;
    for (size_t i=0;i<allShapes.size()-1;i++)
        shapesToErase.push_back(allShapes[i]->getObjectHandle());
    App::currentWorld->sceneObjects->eraseObjects(shapesToErase,true);
    App::currentWorld->textureContainer->updateAllDependencies();
    return(lastSel);
}

void CSceneObjectOperations::divideSelection(std::vector<int>* selection)
{
    std::vector<CShape*> shapesToDivide;
    for (size_t i=0;i<selection->size();i++)
    {
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(selection->at(i));
        if ( (it!=nullptr)&&(!it->getMesh()->isPure()) )
            shapesToDivide.push_back(it);
    }

    std::vector<int> newObjectHandles;
    App::currentWorld->sceneObjects->deselectObjects();
    selection->clear();

    for (size_t i=0;i<shapesToDivide.size();i++)
    {
        std::vector<CShape*> ns;
        if (_divideShape(shapesToDivide[i],ns))
        {
            for (size_t j=0;j<ns.size();j++)
            {
                newObjectHandles.push_back(ns[j]->getObjectHandle());
                selection->push_back(ns[j]->getObjectHandle());
            }
            selection->push_back(shapesToDivide[i]->getObjectHandle());
        }
    }

    App::currentWorld->sceneObjects->setSelectedObjectHandles(selection);

    if (newObjectHandles.size()>0)
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->createStack();
        stack->pushTableOntoStack();
        stack->pushStringOntoStack("objectHandles",0);
        stack->pushInt32ArrayOntoStack(&newObjectHandles[0],newObjectHandles.size());
        stack->insertDataIntoStackTable();
        App::worldContainer->callScripts(sim_syscb_aftercreate,stack,nullptr);
        App::worldContainer->interfaceStackContainer->destroyStack(stack);
    }
}

bool CSceneObjectOperations::_divideShape(CShape* it,std::vector<CShape*>& newShapes)
{
    App::currentWorld->textureContainer->announceGeneralObjectWillBeErased(it->getObjectHandle(),-1);
    it->getMesh()->removeAllTextures();

    // we have to remove all attached drawing objects
    App::currentWorld->drawingCont->announceObjectWillBeErased(it);
    App::currentWorld->pointCloudCont->announceObjectWillBeErased(it->getObjectHandle());
    App::currentWorld->bannerCont->announceObjectWillBeErased(it->getObjectHandle());

    std::vector<double> vertices;
    std::vector<int> indices;
    it->getMesh()->getCumulativeMeshes(it->getFullCumulativeTransformation(),vertices,&indices,nullptr);
    int extractedCount=0;
    while (true)
    {
        std::vector<double> subvert;
        std::vector<int> subind;
        if (CMeshManip::extractOneShape(&vertices,&indices,&subvert,&subind))
        { // Something was extracted
            extractedCount++;
            CMesh* mesh=new CMesh(it->getFullCumulativeTransformation(),subvert,subind,nullptr,nullptr,0);
            CShape* shape=new CShape();
            shape->replaceMesh(mesh,false);
            if (it->getMesh()->isMesh())
                ((CMesh*)it->getMesh())->copyVisualAttributesTo(mesh);
            it->copyAttributesTo(shape);
            shape->setLocalTransformation(it->getCumulativeTransformation());
            App::currentWorld->sceneObjects->addObjectToScene(shape,false,false);
            App::currentWorld->sceneObjects->setObjectParent(shape,it->getParent(),true);
            App::currentWorld->sceneObjects->setObjectAlias(shape,it->getObjectAlias().c_str(),true);
            newShapes.push_back(shape);
        }
        else
        { // nothing was extracted
            if (extractedCount==0)
                break; // we couldn't extract anything!
            CMesh* mesh=new CMesh(it->getFullCumulativeTransformation(),vertices,indices,nullptr,nullptr,0);
            it->replaceMesh(mesh,true);
            break;
        }
    }
    return(newShapes.size()>0);
}

void CSceneObjectOperations::scaleObjects(const std::vector<int>& selection,double scalingFactor,bool scalePositionsToo)
{
    std::vector<int> sel(selection);
    App::currentWorld->sceneObjects->addModelObjects(sel);
    for (size_t i=0;i<sel.size();i++)
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(sel[i]);
        if (scalePositionsToo)
            it->scalePosition(scalingFactor);
        else
        { // If one parent is a root object (model base) and in this selection, then we also scale the position here!! (2009/06/10)
            CSceneObject* itp=it->getParent();
            while (itp!=nullptr)
            {
                if (itp->getModelBase())
                { // We found a parent that is a root! Is it in the selection?
                    bool f=false;
                    for (int j=0;j<int(sel.size());j++)
                    {
                        if (sel[j]==itp->getObjectHandle())
                        { // YEs!
                            f=true;
                            break;
                        }
                    }
                    if (f)
                    { // We also scale the pos here!!
                        it->scalePosition(scalingFactor);
                        break;
                    }
                }
                itp=itp->getParent();
            }
        }
        it->scaleObject(scalingFactor);
    }

    // OLD IK:
    for (size_t i=0;i<App::currentWorld->ikGroups->getObjectCount();i++)
    {
        CIkGroup_old* ikGroup=App::currentWorld->ikGroups->getObjectFromIndex(i);
        // Go through all ikElement lists:
        for (size_t j=0;j<ikGroup->getIkElementCount();j++)
        {
            CIkElement_old* ikEl=ikGroup->getIkElementFromIndex(j);
            CDummy* tip=App::currentWorld->sceneObjects->getDummyFromHandle(ikEl->getTipHandle());
            bool scaleElement=false;
            if (tip!=nullptr)
            { // was this tip scaled?
                bool tipFound=false;
                for (int k=0;k<int(sel.size());k++)
                {
                    if (sel[k]==tip->getObjectHandle())
                    {
                        tipFound=true;
                        break;
                    }
                }
                if (tipFound)
                { // yes, tip was found!
                    scaleElement=true;
                }
            }
            if (scaleElement)
                ikEl->setMinLinearPrecision(ikEl->getMinLinearPrecision()*scalingFactor); // we scale that ikElement!
        }
    }
}

CMesh* CSceneObjectOperations::_generateConvexHull(int shapeHandle)
{
    TRACE_INTERNAL;
    CMesh* retVal=nullptr;
    CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
    if (it!=nullptr)
    {
        std::vector<double> allHullVertices;
        C7Vector transf(it->getFullCumulativeTransformation());
        std::vector<double> vert;
        std::vector<int> ind;
        it->getMesh()->getCumulativeMeshes(C7Vector::identityTransformation,vert,&ind,nullptr);
        for (size_t i=0;i<vert.size()/3;i++)
        {
            C3Vector v(vert.data()+3*i);
            v=transf*v;
            allHullVertices.push_back(v(0));
            allHullVertices.push_back(v(1));
            allHullVertices.push_back(v(2));
        }
        if (allHullVertices.size()!=0)
        {
            std::vector<double> hull;
            std::vector<int> indices;
            std::vector<double> normals;
            if (CMeshRoutines::getConvexHull(allHullVertices,hull,indices))
                retVal=new CMesh(transf,hull,indices,nullptr,nullptr,0);
        }
    }
    return(retVal);
}

CShape* CSceneObjectOperations::_morphToConvexDecomposed(CShape* it,size_t nClusters,double maxConcavity,
                                             bool addExtraDistPoints,bool addFacesPoints,double maxConnectDist,
                                             size_t maxTrianglesInDecimatedMesh,size_t maxHullVertices,
                                             double smallClusterThreshold,bool useHACD,int resolution_VHACD,int depth_VHACD_old,double concavity_VHACD,
                                             int planeDownsampling_VHACD,int convexHullDownsampling_VHACD,
                                             double alpha_VHACD,double beta_VHACD,double gamma_VHACD_old,bool pca_VHACD,
                                             bool voxelBased_VHACD,int maxVerticesPerCH_VHACD,double minVolumePerCH_VHACD)
{
    CShape* morphedShape=nullptr;
    C7Vector obbTr(it->getCumulativeTransformation()*it->getBB(nullptr));
    if (it->isCompound())
    { // ungroup, then group again
        C7Vector tr(it->getCumulativeTransformation());
        double mass=it->getMesh()->getMass();
        C3Vector com(it->getMesh()->getCOM());
        C3X3Matrix inertia(it->getMesh()->getInertia());
        std::vector<CShape*> newShapes;
        _fullUngroupShape(it,newShapes);
        newShapes.push_back(it);
        std::vector<CShape*> newShapes2;
        for (size_t j=0;j<newShapes.size();j++)
        {
            CShape* it2=newShapes[j];
            std::vector<double> vert;
            std::vector<int> ind;
            it2->getMesh()->getCumulativeMeshes(C7Vector::identityTransformation,vert,&ind,nullptr);
            std::vector<std::vector<double>*> outputVert;
            std::vector<std::vector<int>*> outputInd;
            CMeshRoutines::convexDecompose(&vert[0],(int)vert.size(),&ind[0],(int)ind.size(),outputVert,outputInd,
                    nClusters,maxConcavity,addExtraDistPoints,addFacesPoints,maxConnectDist,
                    maxTrianglesInDecimatedMesh,maxHullVertices,smallClusterThreshold,
                    useHACD,resolution_VHACD,depth_VHACD_old,concavity_VHACD,planeDownsampling_VHACD,
                    convexHullDownsampling_VHACD,alpha_VHACD,beta_VHACD,gamma_VHACD_old,pca_VHACD,
                    voxelBased_VHACD,maxVerticesPerCH_VHACD,minVolumePerCH_VHACD);
            std::vector<CMesh*> allMeshes;
            for (size_t i=0;i<outputVert.size();i++)
            {
                bool addMesh=true;
                if (CMeshRoutines::getConvexType(outputVert[i][0],outputInd[i][0],0.015)!=0)
                    addMesh=CMeshRoutines::getConvexHull(outputVert[i][0],outputVert[i][0],outputInd[i][0]);
                if (addMesh)
                {
                    CMesh* mesh=new CMesh(C7Vector::identityTransformation,outputVert[i][0],outputInd[i][0],nullptr,nullptr,0);
                    allMeshes.push_back(mesh);
                }
                delete outputVert[i];
                delete outputInd[i];
            }
            if (allMeshes.size()>0)
            {
                CMeshWrapper* wrap=new CMeshWrapper();
                for (size_t i=0;i<allMeshes.size();i++)
                    wrap->addItem(allMeshes[i]);
                it2->replaceMesh(wrap,true);
                newShapes2.push_back(it2);
            }
            else
                App::currentWorld->sceneObjects->eraseObject(it2,true);
        }
        if (newShapes2.size()>0)
        {
            morphedShape=newShapes2[newShapes2.size()-1];
            if (newShapes2.size()>1)
                _groupShapes(newShapes2);
            morphedShape->relocateFrame("custom",&tr);
            morphedShape->getMesh()->setMass(mass);
            morphedShape->getMesh()->setCOM(com);
            morphedShape->getMesh()->setInertia(inertia);
        }
    }
    else
    {
        std::vector<double> vert;
        std::vector<int> ind;
        it->getMesh()->getCumulativeMeshes(C7Vector::identityTransformation,vert,&ind,nullptr);
        std::vector<std::vector<double>*> outputVert;
        std::vector<std::vector<int>*> outputInd;
        CMeshRoutines::convexDecompose(&vert[0],(int)vert.size(),&ind[0],(int)ind.size(),outputVert,outputInd,
                nClusters,maxConcavity,addExtraDistPoints,addFacesPoints,maxConnectDist,
                maxTrianglesInDecimatedMesh,maxHullVertices,smallClusterThreshold,
                useHACD,resolution_VHACD,depth_VHACD_old,concavity_VHACD,planeDownsampling_VHACD,
                convexHullDownsampling_VHACD,alpha_VHACD,beta_VHACD,gamma_VHACD_old,pca_VHACD,
                voxelBased_VHACD,maxVerticesPerCH_VHACD,minVolumePerCH_VHACD);
        std::vector<CMesh*> allMeshes;
        for (size_t i=0;i<outputVert.size();i++)
        {
            bool addMesh=true;
            if (CMeshRoutines::getConvexType(outputVert[i][0],outputInd[i][0],0.015)!=0)
                addMesh=CMeshRoutines::getConvexHull(outputVert[i][0],outputVert[i][0],outputInd[i][0]);
            if (addMesh)
            {
                CMesh* mesh=new CMesh(C7Vector::identityTransformation,outputVert[i][0],outputInd[i][0],nullptr,nullptr,0);
                allMeshes.push_back(mesh);
            }
            delete outputVert[i];
            delete outputInd[i];
        }
        if (allMeshes.size()>0)
        {
            morphedShape=it;
            CMeshWrapper* wrap=new CMeshWrapper();
            for (size_t i=0;i<allMeshes.size();i++)
                wrap->addItem(allMeshes[i]);
            it->replaceMesh(wrap,true);
        }
    }
    if (morphedShape)
        morphedShape->alignBB("custom",&obbTr);
    return(morphedShape);
}

int CSceneObjectOperations::convexDecompose(int shapeHandle,int options,const int* intParams,const double* floatParams)
{
    TRACE_INTERNAL;
    int retVal=-1;

    static bool addExtraDistPoints=true;
    static bool addFacesPoints=true;
    static int nClusters=1;
    static int maxHullVertices=200; // from 100 to 200 on 5/2/2014
    static double maxConcavity=100.0;
    static double smallClusterThreshold=0.25;
    static int maxTrianglesInDecimatedMesh=500;
    static double maxConnectDist=30.0;
    static bool useHACD=false; // i.e. use V-HACD
    static int resolution=100000;
    static double concavity=0.0025;
    static int planeDownsampling=4;
    static int convexHullDownsampling=4;
    static double alpha=0.05;
    static double beta=0.05;
    static bool pca=false;
    static bool voxelBasedMode=true;
    static int maxVerticesPerCH=64;
    static double minVolumePerCH=0.0001;

    if ((options&4)==0)
    {
        addExtraDistPoints=(options&8)!=0;
        addFacesPoints=(options&16)!=0;
        nClusters=intParams[0];
        maxHullVertices=intParams[2];
        maxConcavity=floatParams[0];
        smallClusterThreshold=floatParams[2];
        maxTrianglesInDecimatedMesh=intParams[1];
        maxConnectDist=floatParams[1];
        useHACD=true; // forgotten, fixed thanks to Patrick Gruener
        if (options&128)
        { // we have more parameters than usual (i.e. the V-HACD parameters):
            useHACD=false;
            resolution=intParams[5];
            concavity=floatParams[5];
            planeDownsampling=intParams[7];
            convexHullDownsampling=intParams[8];
            alpha=floatParams[6];
            beta=floatParams[7];
            pca=(options&256);
            voxelBasedMode=!(options&512);
            maxVerticesPerCH=intParams[9];
            minVolumePerCH=floatParams[9];
        }
    }
    CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
    if ((options&1)==0)
    { // We want to create a new shape from it:
        CShape* it2=new CShape();
        it2->replaceMesh(it->getMesh()->copyYourself(),false);
        it2->setLocalTransformation(it->getCumulativeTransformation());
        App::currentWorld->sceneObjects->addObjectToScene(it2,false,true);
        it=it2;
    }
    it=CSceneObjectOperations::_morphToConvexDecomposed(it,nClusters,maxConcavity,addExtraDistPoints,
                                                    addFacesPoints,maxConnectDist,maxTrianglesInDecimatedMesh,
                                                    maxHullVertices,smallClusterThreshold,
                                                    useHACD,resolution,20,concavity,planeDownsampling,
                                                    convexHullDownsampling,alpha,beta,0.00125,pca,voxelBasedMode,
                                                    maxVerticesPerCH,minVolumePerCH);
    if (it!=nullptr)
        retVal=it->getObjectHandle();
    return(retVal);
}

#ifdef SIM_WITH_GUI
void CSceneObjectOperations::addMenu(VMenu* menu)
{
    std::vector<CSceneObject*> sel;
    App::currentWorld->sceneObjects->getSelectedObjects(sel,-1,true,true);
    int shapeCnt=0;
    int compoundCnt=0;
    for (size_t i=0;i<sel.size();i++)
    {
        int t=sel[i]->getObjectType();
        if (t==sim_object_shape_type)
        {
            CShape* it=(CShape*)sel[i];
            shapeCnt++;
            if (it->getSingleMesh()==nullptr)
                compoundCnt++;
        }
    }
    int dynShapeCnt=0;
    sel.clear();
    App::currentWorld->sceneObjects->getSelectedObjects(sel,sim_object_shape_type,true,false);
    for (size_t i=0;i<sel.size();i++)
    {
        CShape* it=(CShape*)sel[i];
        if (!it->getStatic())
            dynShapeCnt++;
    }

    size_t selItems=App::currentWorld->sceneObjects->getSelectionCount();
    size_t selDummies=App::currentWorld->sceneObjects->getDummyCountInSelection();
    size_t shapeNumber=App::currentWorld->sceneObjects->getShapeCountInSelection();
    size_t pathNumber=App::currentWorld->sceneObjects->getPathCountInSelection();
    size_t simpleShapeNumber=0;
    std::vector<CSceneObject*> objects;
    App::currentWorld->sceneObjects->getSelectedObjects(objects);
    for (size_t i=0;i<objects.size();i++)
    {
        if (objects[i]->getObjectType()==sim_object_shape_type)
        {
            CShape* it=(CShape*)objects[i];
            if (it->getMesh()->isMesh())
                simpleShapeNumber++;
        }
    }


    bool noSim=App::currentWorld->simulation->isSimulationStopped();
    bool lastSelIsShape=App::currentWorld->sceneObjects->isLastSelectionAShape();
    bool lastSelIsNonPureShape=false;
    bool lastSelIsNonGrouping=false;
    if (lastSelIsShape)
    {
        CShape* sh=App::currentWorld->sceneObjects->getLastSelectionShape();
        lastSelIsNonPureShape=!sh->getMesh()->isPure();
        lastSelIsNonGrouping=!sh->isCompound();
    }

    bool lastSelIsPath=App::currentWorld->sceneObjects->isLastSelectionAPath();
    bool hasChildScriptAttached=false;
    bool hasCustomizationScriptAttached=false;
    if (selItems==1)
    {
        hasChildScriptAttached=(App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(0))!=nullptr);
        hasCustomizationScriptAttached=(App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(0))!=nullptr);
    }

    if (GuiApp::getEditModeType()==NO_EDIT_MODE)
    {
        menu->appendMenuItem(App::currentWorld->undoBufferContainer->canUndo(),false,SCENE_OBJECT_OPERATION_UNDO_SOOCMD,IDSN_UNDO);
        menu->appendMenuItem(App::currentWorld->undoBufferContainer->canRedo(),false,SCENE_OBJECT_OPERATION_REDO_SOOCMD,IDSN_REDO);
        menu->appendMenuSeparator();
        menu->appendMenuItem(selItems>1,false,SCENE_OBJECT_OPERATION_MAKE_PARENT_SOOCMD,"Set parent, keep pose(s)");
        menu->appendMenuItem(selItems>1,false,SCENE_OBJECT_OPERATION_MAKE_PARENT_AND_MOVE_SOOCMD,"Set parent");
        menu->appendMenuItem(selItems>0,false,SCENE_OBJECT_OPERATION_MAKE_ORPHANS_SOOCMD,"Set parent-less");
        menu->appendMenuSeparator();
        if (CSimFlavor::getBoolVal(12))
        {
            menu->appendMenuItem((shapeCnt>0)&&noSim,false,SCENE_OBJECT_OPERATION_MORPH_INTO_CONVEX_SHAPES_SOOCMD,"Morph into convex shape(s)");
            menu->appendMenuItem((shapeCnt>0)&&noSim,false,SCENE_OBJECT_OPERATION_MORPH_INTO_CONVEX_DECOMPOSITION_SOOCMD,"Morph into convex decomposed shape(s)...");
            menu->appendMenuItem((shapeCnt>0)&&noSim,false,SCENE_OBJECT_OPERATION_DECIMATE_SHAPE_SOOCMD,"Decimate shape(s)...");
        }
        menu->appendMenuSeparator();
        menu->appendMenuItem(selItems>0,false,SCENE_OBJECT_OPERATION_OBJECT_FULL_COPY_SOOCMD,"Copy object(s)");
        menu->appendMenuItem(!App::worldContainer->copyBuffer->isBufferEmpty(),false,SCENE_OBJECT_OPERATION_PASTE_OBJECTS_SOOCMD,"Paste buffer");
        menu->appendMenuItem(selItems>0,false,SCENE_OBJECT_OPERATION_DELETE_OBJECTS_SOOCMD,"Delete object(s)");
        menu->appendMenuItem(selItems>0,false,SCENE_OBJECT_OPERATION_OBJECT_FULL_CUT_SOOCMD,"Cut object(s)");
        menu->appendMenuSeparator();
        menu->appendMenuItem(true,false,SCENE_OBJECT_OPERATION_SELECT_ALL_OBJECTS_SOOCMD,"Select all");
        menu->appendMenuSeparator();

        if (CSimFlavor::getBoolVal(12))
        {
            VMenu* removing=new VMenu();
            removing->appendMenuItem(hasChildScriptAttached&&noSim,false,SCENE_OBJECT_OPERATION_REMOVE_ASSOCIATED_CHILD_SCRIPT_SOOCMD,"Associated child script");
            removing->appendMenuItem(hasCustomizationScriptAttached&&noSim,false,SCENE_OBJECT_OPERATION_REMOVE_ASSOCIATED_CUSTOMIZATION_SCRIPT_SOOCMD,"Associated customization script");
            menu->appendMenuAndDetach(removing,(hasChildScriptAttached||hasCustomizationScriptAttached)&&noSim,IDSN_REMOVE_MENU_ITEM);
            menu->appendMenuSeparator();

            VMenu* grouping=new VMenu();
            grouping->appendMenuItem((shapeCnt>1)&&noSim,false,SCENE_OBJECT_OPERATION_GROUP_SHAPES_SOOCMD,"group");
            grouping->appendMenuItem((compoundCnt>0)&&noSim,false,SCENE_OBJECT_OPERATION_UNGROUP_SHAPES_SOOCMD,"ungroup");
            grouping->appendMenuSeparator();
            grouping->appendMenuItem((shapeCnt>1)&&noSim,false,SCENE_OBJECT_OPERATION_MERGE_SHAPES_SOOCMD,"merge");
            grouping->appendMenuItem((shapeCnt>0)&&noSim,false,SCENE_OBJECT_OPERATION_DIVIDE_SHAPES_SOOCMD,"divide");
            menu->appendMenuAndDetach(grouping,true,"Shape grouping / merging");

            VMenu* relocate=new VMenu();
            relocate->appendMenuItem((shapeCnt>0)&&noSim,false,SCENE_OBJECT_OPERATION_RELOCATE_FRAME_TO_ORIGIN_SOOCMD,"relocate to world origin");
            relocate->appendMenuItem((shapeCnt>0)&&noSim,false,SCENE_OBJECT_OPERATION_RELOCATE_FRAME_TO_PARENT_SOOCMD,"relocate to parent origin");
            relocate->appendMenuItem((shapeCnt>0)&&noSim,false,SCENE_OBJECT_OPERATION_RELOCATE_FRAME_TO_CENTER_SOOCMD,"relocate to mesh center");
            menu->appendMenuAndDetach(relocate,true,"Shape reference frame");

            VMenu* align=new VMenu();
            align->appendMenuItem((shapeCnt>0)&&noSim,false,SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_WORLD_SOOCMD,"align with world");
            align->appendMenuItem((shapeCnt>0)&&noSim,false,SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_MESH_SOOCMD,"align with mesh");
            menu->appendMenuAndDetach(align,true,"Shape bounding box");

            VMenu* minertia=new VMenu();
            minertia->appendMenuItem((dynShapeCnt>0)&&noSim,false,SCENE_OBJECT_OPERATION_COMPUTE_INERTIA_SOOCMD,"compute from uniform density...");
            minertia->appendMenuItem((dynShapeCnt>0)&&noSim,false,SCENE_OBJECT_OPERATION_SCALE_MASS_SOOCMD,"scale mass...");
            minertia->appendMenuItem((dynShapeCnt>0)&&noSim,false,SCENE_OBJECT_OPERATION_SCALE_INERTIA_SOOCMD,"scale inertia...");
            menu->appendMenuAndDetach(minertia,true,"Shape mass and inertia");
        }
    }
}
#endif
