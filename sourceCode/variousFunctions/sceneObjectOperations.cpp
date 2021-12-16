#include "simInternal.h"
#include "sceneObjectOperations.h"
#include "simulation.h"
#include "mesh.h"
#include "app.h"
#include "meshManip.h"
#include "tt.h"
#include "simStrings.h"
#include "meshRoutines.h"
#include "simFlavor.h"
#include <boost/lexical_cast.hpp>
#ifdef SIM_WITH_GUI
    #include "vMessageBox.h"
    #include "qdlgconvexdecomposition.h"
#endif

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
        if (!VThread::isCurrentThreadTheUiThread())
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
                App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
                App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
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
        if (!VThread::isCurrentThreadTheUiThread())
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
                    CSceneObjectOperations::addRootObjectChildrenToSelection(sel);
                    std::string masterName(it->getObjectName_old());

                    App::worldContainer->copyBuffer->copyCurrentSelection(&sel,App::currentWorld->environment->getSceneLocked(),0);
                    App::currentWorld->sceneObjects->deselectObjects();
                    for (size_t i=0;i<clones.size();i++)
                    {
                        std::string name(clones[i]->getObjectName_old());
                        std::string altName(clones[i]->getObjectAltName_old());
                        std::vector<int> objs;
                        objs.push_back(clones[i]->getObjectHandle());
                        CSceneObjectOperations::addRootObjectChildrenToSelection(objs);
                        C7Vector tr(clones[i]->getLocalTransformation());
                        CSceneObject* parent(clones[i]->getParent());
                        int order=App::currentWorld->sceneObjects->getObjectSequence(clones[i]);
                        App::currentWorld->sceneObjects->eraseSeveralObjects(objs,true);
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
                    App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
                    std::string txt;
                    txt+=boost::lexical_cast<std::string>(clones.size())+IDSN_X_CLONES_WERE_UPDATED;
                    App::logMsg(sim_verbosity_msgs,txt.c_str());

                    for (size_t i=0;i<newSelection.size();i++)
                        App::currentWorld->sceneObjects->addObjectToSelection(newSelection[i]);

                    App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
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

    if (commandID==SCENE_OBJECT_OPERATION_MAKE_PARENT_SOOCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            if (sel.size()>1)
            {
                CSceneObject* last=App::currentWorld->sceneObjects->getObjectFromHandle(sel[sel.size()-1]);
                for (int i=0;i<int(sel.size())-1;i++)
                {
                    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(sel[i]);
                    App::currentWorld->sceneObjects->setObjectParent(it,last,true);
                }
                App::currentWorld->sceneObjects->selectObject(last->getObjectHandle()); // We select the parent

                App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
                std::string txt(IDSNS_ATTACHING_OBJECTS_TO);
                txt+=last->getObjectAlias_printPath()+"'...";
                App::logMsg(sim_verbosity_msgs,txt.c_str());
                App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
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
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            App::logMsg(sim_verbosity_msgs,IDSNS_MAKING_ORPHANS);
            for (size_t i=0;i<sel.size();i++)
            {
                CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(sel[i]);
                App::currentWorld->sceneObjects->setObjectParent(it,nullptr,true);
            }
            App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
            App::currentWorld->sceneObjects->deselectObjects(); // We clear selection
            App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
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
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
//          CSceneObjectOperations::addRootObjectChildrenToSelection(sel);
            App::uiThread->showOrHideProgressBar(true,-1,"Morphing into convex shape(s)...");
            App::logMsg(sim_verbosity_msgs,IDSNS_MORPHING_INTO_CONVEX_SHAPES);
            bool printQHullFail=false;
            for (int obji=0;obji<int(sel.size());obji++)
            {
                CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(sel[obji]);
                if (it!=nullptr)
                {
                    if ( (!it->getMeshWrapper()->isConvex())||it->isCompound() )
                    {
                        int newShapeHandle=generateConvexHull(sel[obji]);
                        if (newShapeHandle!=-1)
                        {
                            // Get the mass and inertia info from the old shape:
                            C7Vector absCOM(it->getFullCumulativeTransformation());
                            absCOM=absCOM*it->getMeshWrapper()->getLocalInertiaFrame();
                            float mass=it->getMeshWrapper()->getMass();
                            C7Vector absCOMNoShift(absCOM);
                            absCOMNoShift.X.clear(); // we just wanna get the orientation of the inertia matrix, no shift info!
                            C3X3Matrix tensor(CMeshWrapper::getNewTensor(it->getMeshWrapper()->getPrincipalMomentsOfInertia(),absCOMNoShift));

                            // Set-up the new shape:
                            CShape* newShape=App::currentWorld->sceneObjects->getShapeFromHandle(newShapeHandle);
                            C7Vector newLocal(it->getFullParentCumulativeTransformation().getInverse()*newShape->getFullCumulativeTransformation());
                            C7Vector oldLocal(it->getFullLocalTransformation());
                            newShape->getSingleMesh()->setConvexVisualAttributes();
                            it->setNewMesh(newShape->getMeshWrapper());
                            newShape->disconnectMesh();
                            it->setLocalTransformation(newLocal); // The shape's frame was changed!
                            App::currentWorld->sceneObjects->eraseObject(newShape,true);

                            // Transfer the mass and inertia info to the new shape:
                            it->getMeshWrapper()->setMass(mass);
                            C4Vector rot;
                            C3Vector pmoi;
                            CMeshWrapper::findPrincipalMomentOfInertia(tensor,rot,pmoi);
                            it->getMeshWrapper()->setPrincipalMomentsOfInertia(pmoi);
                            absCOM.Q=rot;
                            C7Vector relCOM(it->getFullCumulativeTransformation().getInverse()*absCOM);
                            it->getMeshWrapper()->setLocalInertiaFrame(relCOM);

                            it->setColor(nullptr,sim_colorcomponent_ambient_diffuse,1.0f,0.7f,0.7f);
                            it->getSingleMesh()->setEdgeThresholdAngle(0.0f);
                            it->getSingleMesh()->setShadingAngle(0.0f);
                            it->getSingleMesh()->setVisibleEdges(false);

                            // We need to correct all its children for this change of frame:
                            for (size_t i=0;i<it->getChildCount();i++)
                            {
                                CSceneObject* child=it->getChildFromIndex(i);
                                child->setLocalTransformation(newLocal.getInverse()*oldLocal*child->getLocalTransformation());
                                child->incrementMemorizedConfigurationValidCounter(); // so if we are running in a simulation, the shape doesn't get reset at its initial config
                            }
                        }
                        else
                            printQHullFail=true;
                    }
                    else
                    { // that shape is not a compound and already convex. We just change its visual attributes:
                        it->getSingleMesh()->setConvexVisualAttributes();
                    }
                }
            }

            App::uiThread->showOrHideProgressBar(false);

            App::currentWorld->sceneObjects->deselectObjects();
            if (printQHullFail)
                App::logMsg(sim_verbosity_errors,IDSNS_FAILED_IS_THE_QHULL_PLUGIN_LOADED);
            else
            {
                App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
                App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
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

    if (commandID==SCENE_OBJECT_OPERATION_DECIMATE_SHAPE_SOOCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            CShape* sh=nullptr;
            if ( (App::currentWorld->sceneObjects->getSelectionCount()==1)&&(App::currentWorld->sceneObjects->getLastSelectionObject()->getObjectType()==sim_object_shape_type) )
            {
                sh=App::currentWorld->sceneObjects->getLastSelectionShape();
                if (sh->getMeshWrapper()->isPure())
                    sh=nullptr;
            }
            if (sh!=nullptr)
            {
                std::vector<float> vert;
                std::vector<int> ind;
                sh->getMeshWrapper()->getCumulativeMeshes(vert,&ind,nullptr);
                C7Vector tr(sh->getFullCumulativeTransformation());
                for (size_t i=0;i<vert.size()/3;i++)
                {
                    C3Vector v(&vert[3*i+0]);
                    v*=tr;
                    vert[3*i+0]=v(0);
                    vert[3*i+1]=v(1);
                    vert[3*i+2]=v(2);
                }
                SUIThreadCommand cmdIn;
                SUIThreadCommand cmdOut;
                cmdIn.cmdId=DISPLAY_MESH_DECIMATION_DIALOG_UITHREADCMD;
                cmdIn.intParams.push_back((int)ind.size()/3);
                cmdIn.floatParams.push_back(0.2f);
                App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
                if ((cmdOut.boolParams.size()>0)&&cmdOut.boolParams[0])
                {
                    float decimationPercentage=cmdOut.floatParams[0];
                    std::vector<float> vertOut;
                    std::vector<int> indOut;

                    App::uiThread->showOrHideProgressBar(true,-1,"Computing decimated shape...");

                    if (CMeshRoutines::getDecimatedMesh(vert,ind,decimationPercentage,vertOut,indOut))
                    { // decimation algo was successful:
                        App::logMsg(sim_verbosity_msgs,IDSNS_DECIMATING_MESH);

                        // Create the new shape:
                        CShape* newShape=new CShape(nullptr,vertOut,indOut,nullptr,nullptr);
                        newShape->getSingleMesh()->setConvexVisualAttributes();
                        newShape->getMeshWrapper()->setLocalInertiaFrame(C7Vector::identityTransformation);
                        App::currentWorld->sceneObjects->addObjectToScene(newShape,false,true);

                        // Get the mass and inertia info from the old shape:
                        C7Vector absCOM(sh->getFullCumulativeTransformation());
                        absCOM=absCOM*sh->getMeshWrapper()->getLocalInertiaFrame();
                        float mass=sh->getMeshWrapper()->getMass();
                        C7Vector absCOMNoShift(absCOM);
                        absCOMNoShift.X.clear(); // we just wanna get the orientation of the inertia matrix, no shift info!
                        C3X3Matrix tensor(CMeshWrapper::getNewTensor(sh->getMeshWrapper()->getPrincipalMomentsOfInertia(),absCOMNoShift));

                        // Set-up the new shape:
                        C7Vector newLocal(sh->getFullParentCumulativeTransformation().getInverse()*newShape->getFullCumulativeTransformation());
                        C7Vector oldLocal(sh->getFullLocalTransformation());
                        sh->setNewMesh(newShape->getMeshWrapper());
                        newShape->disconnectMesh();
                        sh->setLocalTransformation(newLocal); // The shape's frame was changed!
                        App::currentWorld->sceneObjects->eraseObject(newShape,true);

                        // Transfer the mass and inertia info to the new shape:
                        sh->getMeshWrapper()->setMass(mass);
                        C4Vector rot;
                        C3Vector pmoi;
                        CMeshWrapper::findPrincipalMomentOfInertia(tensor,rot,pmoi);
                        sh->getMeshWrapper()->setPrincipalMomentsOfInertia(pmoi);
                        absCOM.Q=rot;
                        C7Vector relCOM(sh->getFullCumulativeTransformation().getInverse()*absCOM);
                        sh->getMeshWrapper()->setLocalInertiaFrame(relCOM);

                        // Set some visual parameters:
                        sh->setColor(nullptr,sim_colorcomponent_ambient_diffuse,0.7f,0.7f,1.0f);
                        sh->getSingleMesh()->setEdgeThresholdAngle(0.0f);
                        sh->getSingleMesh()->setShadingAngle(0.0f);
                        sh->getSingleMesh()->setVisibleEdges(false);

                        // We need to correct all its children for this change of frame:
                        for (size_t i=0;i<sh->getChildCount();i++)
                        {
                            CSceneObject* child=sh->getChildFromIndex(i);
                            child->setLocalTransformation(newLocal.getInverse()*oldLocal*child->getLocalTransformation());
                            child->incrementMemorizedConfigurationValidCounter(); // so if we are running in a simulation, the shape doesn't get reset at its initial config
                        }

                        App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
                        App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
                    }

                    App::uiThread->showOrHideProgressBar(false);

                    App::currentWorld->sceneObjects->deselectObjects();
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

    if (commandID==SCENE_OBJECT_OPERATION_MORPH_INTO_CONVEX_DECOMPOSITION_SOOCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            // CSceneObjectOperations::addRootObjectChildrenToSelection(sel);

            SUIThreadCommand cmdIn; // leave empty for default parameters
            SUIThreadCommand cmdOut;
            cmdIn.cmdId=DISPLAY_CONVEX_DECOMPOSITION_DIALOG_UITHREADCMD;
            App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
            bool addExtraDistPoints=cmdOut.boolParams[0];
            bool addFacesPoints=cmdOut.boolParams[1];
            int nClusters=cmdOut.intParams[0];
            int maxHullVertices=cmdOut.intParams[1];
            float maxConcavity=cmdOut.floatParams[0];
            float smallClusterThreshold=cmdOut.floatParams[1];
            int maxTrianglesInDecimatedMesh=cmdOut.intParams[2];
            float maxConnectDist=cmdOut.floatParams[2];
            bool individuallyConsiderMultishapeComponents=cmdOut.boolParams[2];
            int maxIterations=cmdOut.intParams[3];
            bool cancel=cmdOut.boolParams[4];
            bool useHACD=cmdOut.boolParams[5];
            bool pca=cmdOut.boolParams[6];
            bool voxelBased=cmdOut.boolParams[7];
            int resolution=cmdOut.intParams[4];
            int depth=cmdOut.intParams[5];
            int planeDownsampling=cmdOut.intParams[6];
            int convexHullDownsampling=cmdOut.intParams[7];
            int maxNumVerticesPerCH=cmdOut.intParams[8];
            float concavity=cmdOut.floatParams[3];
            float alpha=cmdOut.floatParams[4];
            float beta=cmdOut.floatParams[5];
            float gamma=cmdOut.floatParams[6];
            float minVolumePerCH=cmdOut.floatParams[7];
            if (!cancel)
            {
                App::logMsg(sim_verbosity_msgs,IDSNS_MORPHING_INTO_CONVEX_DECOMPOSITION);
                App::uiThread->showOrHideProgressBar(true,-1,"Morphing into convex decomposed shape(s)...");

                for (int obji=0;obji<int(sel.size());obji++)
                {
                    CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(sel[obji]);
                    if (it!=nullptr)
                    {
                        int newShapeHandle=generateConvexDecomposed(sel[obji],nClusters,maxConcavity,addExtraDistPoints,addFacesPoints,
                                                                    maxConnectDist,maxTrianglesInDecimatedMesh,maxHullVertices,
                                                                    smallClusterThreshold,individuallyConsiderMultishapeComponents,
                                                                    maxIterations,useHACD,resolution,depth,concavity,planeDownsampling,
                                                                    convexHullDownsampling,alpha,beta,gamma,pca,voxelBased,
                                                                    maxNumVerticesPerCH,minVolumePerCH);
                        if (newShapeHandle!=-1)
                        {
                            // Get the mass and inertia info from the old shape:
                            C7Vector absCOM(it->getFullCumulativeTransformation());
                            absCOM=absCOM*it->getMeshWrapper()->getLocalInertiaFrame();
                            float mass=it->getMeshWrapper()->getMass();
                            C7Vector absCOMNoShift(absCOM);
                            absCOMNoShift.X.clear(); // we just wanna get the orientation of the inertia matrix, no shift info!
                            C3X3Matrix tensor(CMeshWrapper::getNewTensor(it->getMeshWrapper()->getPrincipalMomentsOfInertia(),absCOMNoShift));

                            // Set-up the new shape:
                            CShape* newShape=App::currentWorld->sceneObjects->getShapeFromHandle(newShapeHandle);
                            C7Vector newLocal(it->getFullParentCumulativeTransformation().getInverse()*newShape->getFullCumulativeTransformation());
                            C7Vector oldLocal(it->getFullLocalTransformation());
                            it->setNewMesh(newShape->getMeshWrapper());
                            newShape->disconnectMesh();
                            it->setLocalTransformation(newLocal); // The shape's frame was changed!
                            App::currentWorld->sceneObjects->eraseObject(newShape,true);

                            // Transfer the mass and inertia info to the new shape:
                            it->getMeshWrapper()->setMass(mass);
                            C4Vector rot;
                            C3Vector pmoi;
                            CMeshWrapper::findPrincipalMomentOfInertia(tensor,rot,pmoi);
                            it->getMeshWrapper()->setPrincipalMomentsOfInertia(pmoi);
                            absCOM.Q=rot;
                            C7Vector relCOM(it->getFullCumulativeTransformation().getInverse()*absCOM);
                            it->getMeshWrapper()->setLocalInertiaFrame(relCOM);

                            // We need to correct all its children for this change of frame:
                            for (size_t i=0;i<it->getChildCount();i++)
                            {
                                CSceneObject* child=it->getChildFromIndex(i);
                                child->setLocalTransformation(newLocal.getInverse()*oldLocal*child->getLocalTransformation());
                                child->incrementMemorizedConfigurationValidCounter(); // so if we are running in a simulation, the shape doesn't get reset at its initial config
                            }
                        }
                    }
                }
                App::uiThread->showOrHideProgressBar(false);
                App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
            }
            App::currentWorld->sceneObjects->deselectObjects();
            App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if ((commandID==SCENE_OBJECT_OPERATION_UNLINK_DUMMIES_SOOCMD)||(commandID==SCENE_OBJECT_OPERATION_LINK_DUMMIES_IK_TIP_TARGET_SOOCMD)||(commandID==SCENE_OBJECT_OPERATION_LINK_DUMMIES_GCS_LOOP_CLOSURE_SOOCMD)||
        (commandID==SCENE_OBJECT_OPERATION_LINK_DUMMIES_GCS_TIP_SOOCMD)||(commandID==SCENE_OBJECT_OPERATION_LINK_DUMMIES_GCS_TARGET_SOOCMD)||(commandID==SCENE_OBJECT_OPERATION_LINK_DUMMIES_DYNAMICS_LOOP_CLOSURE_SOOCMD))
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            if (sel.size()==2)
            {
                CDummy* a=App::currentWorld->sceneObjects->getDummyFromHandle(sel[0]);
                CDummy* b=App::currentWorld->sceneObjects->getDummyFromHandle(sel[1]);
                if ((a!=nullptr)&&(b!=nullptr))
                {
                    if (commandID==SCENE_OBJECT_OPERATION_UNLINK_DUMMIES_SOOCMD)
                    {
                        a->setLinkedDummyHandle(-1,true);
                        App::logMsg(sim_verbosity_msgs,"Unlinking selected dummies... Done.");
                    }
                    else
                    {
                        a->setLinkedDummyHandle(b->getObjectHandle(),true);
                        if (commandID==SCENE_OBJECT_OPERATION_LINK_DUMMIES_IK_TIP_TARGET_SOOCMD)
                            b->setLinkType(sim_dummy_linktype_ik_tip_target,true);
                        if (commandID==SCENE_OBJECT_OPERATION_LINK_DUMMIES_GCS_LOOP_CLOSURE_SOOCMD)
                            b->setLinkType(sim_dummy_linktype_gcs_loop_closure,true);
                        if (commandID==SCENE_OBJECT_OPERATION_LINK_DUMMIES_GCS_TIP_SOOCMD)
                            b->setLinkType(sim_dummy_linktype_gcs_tip,true);
                        if (commandID==SCENE_OBJECT_OPERATION_LINK_DUMMIES_GCS_TARGET_SOOCMD)
                            b->setLinkType(sim_dummy_linktype_gcs_target,true);
                        if (commandID==SCENE_OBJECT_OPERATION_LINK_DUMMIES_DYNAMICS_LOOP_CLOSURE_SOOCMD)
                            b->setLinkType(sim_dummy_linktype_dynamics_loop_closure,true);
                        App::logMsg(sim_verbosity_msgs,"Linking selected dummies... Done.");
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
    if (commandID==SCENE_OBJECT_OPERATION_SELECT_ALL_OBJECTS_SOOCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs,IDSNS_SELECTING_ALL_OBJECTS);
            for (size_t i=0;i<App::currentWorld->sceneObjects->getObjectCount();i++)
                App::currentWorld->sceneObjects->addObjectToSelection(App::currentWorld->sceneObjects->getObjectFromIndex(i)->getObjectHandle());
            App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
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
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            int id=App::currentWorld->sceneObjects->getLastSelectionHandle();
            CScriptObject* script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo_child(id);
            if (script!=nullptr)
            {
#ifdef SIM_WITH_GUI
                if (App::mainWindow!=nullptr)
                    App::mainWindow->codeEditorContainer->closeFromScriptHandle(script->getScriptHandle(),nullptr,true);
#endif
                App::currentWorld->embeddedScriptContainer->removeScript(script->getScriptHandle());
                App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
                App::setFullDialogRefreshFlag();
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
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            int id=App::currentWorld->sceneObjects->getLastSelectionHandle();
            CScriptObject* script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo_customization(id);
            if (script!=nullptr)
            {
#ifdef SIM_WITH_GUI
                if (App::mainWindow!=nullptr)
                    App::mainWindow->codeEditorContainer->closeFromScriptHandle(script->getScriptHandle(),nullptr,true);
#endif
                App::currentWorld->embeddedScriptContainer->removeScript(script->getScriptHandle());
                App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
                App::setFullDialogRefreshFlag();
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
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            App::logMsg(sim_verbosity_msgs,IDSNS_COPYING_SELECTION);
            copyObjects(&sel,true);
            App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
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
        if (!VThread::isCurrentThreadTheUiThread())
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
                cutObjects(&sel,true);
                App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
                App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
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
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            App::logMsg(sim_verbosity_msgs,IDSNS_PASTING_BUFFER);
            pasteCopyBuffer(true);
            App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
            App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
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
        if (!VThread::isCurrentThreadTheUiThread())
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
                deleteObjects(&sel,true);
                App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
                App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
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
    if ((commandID==SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_MAIN_AXIS_SOOCMD)||(commandID==SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_WORLD_SOOCMD)||(commandID==SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_TUBE_MAIN_AXIS_SOOCMD)||(commandID==SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_CUBOID_MAIN_AXIS_SOOCMD))
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            bool tubeFail=false;
            bool cuboidFail=false;
            if (commandID==SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_MAIN_AXIS_SOOCMD)
                App::logMsg(sim_verbosity_msgs,IDSNS_ALIGNING_BOUNDING_BOXES_WITH_MAIN_AXIS);
            if (commandID==SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_WORLD_SOOCMD)
                App::logMsg(sim_verbosity_msgs,IDSNS_ALIGNING_BOUNDING_BOXES_WITH_WORLD);
            if (commandID==SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_TUBE_MAIN_AXIS_SOOCMD)
                App::logMsg(sim_verbosity_msgs,IDSNS_ALIGNING_BOUNDING_BOXES_WITH_TUBES);
            if (commandID==SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_CUBOID_MAIN_AXIS_SOOCMD)
                App::logMsg(sim_verbosity_msgs,IDSNS_ALIGNING_BOUNDING_BOXES_WITH_CUBOIDS);
            if (App::currentWorld->sceneObjects->getShapeCountInSelection(&sel)==int(sel.size()))
            {
                std::vector<void*> processedGeoms;
                processedGeoms.reserve(sel.size());
                bool informThatPurePrimitivesWereNotChanged=false;
                for (int i=0;i<int(sel.size());i++)
                {
                    CShape* theShape=App::currentWorld->sceneObjects->getShapeFromHandle(sel[i]);
                    if (theShape!=nullptr)
                    {
                        // Did we already process this geometric resource?
                        bool found=false;
                        for (size_t j=0;j<processedGeoms.size();j++)
                        {
                            if (processedGeoms[j]==(void*)theShape)
                                found=true;
                        }
                        if (!found)
                        {
                            processedGeoms.push_back(theShape);
                            if ( (!theShape->getMeshWrapper()->isPure())||(theShape->isCompound()) )
                            { // We can reorient all shapes, except for pure simple shapes (i.e. pure non-compound shapes)
                                if (commandID==SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_WORLD_SOOCMD)
                                    theShape->alignBoundingBoxWithWorld();
                                if (commandID==SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_MAIN_AXIS_SOOCMD)
                                    theShape->alignBoundingBoxWithMainAxis();
                                if (commandID==SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_TUBE_MAIN_AXIS_SOOCMD)
                                {
                                    if (!theShape->alignTubeBoundingBoxWithMainAxis())
                                        tubeFail=true;
                                }
                                if (commandID==SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_CUBOID_MAIN_AXIS_SOOCMD)
                                {
                                    if (!theShape->alignCuboidBoundingBoxWithMainAxis())
                                        cuboidFail=true;
                                }
                                App::undoRedo_sceneChangeStart(""); // ************************** UNDO thingy **************************
                            }
                            else
                                informThatPurePrimitivesWereNotChanged=true;
                        }
                    }
                }
                App::undoRedo_sceneChangeEnd(); // ************************** UNDO thingy **************************
#ifdef SIM_WITH_GUI
                if (informThatPurePrimitivesWereNotChanged)
                    App::uiThread->messageBox_warning(App::mainWindow,"Alignment",IDS_INFORM_PURE_PRIMITIVES_COULD_NOT_BE_REORIENTED,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
                if (tubeFail)
                    App::uiThread->messageBox_warning(App::mainWindow,"Alignment",IDSN_INFORM_SHAPE_COULD_NOT_BE_REORIENTED_ALONG_TUBE,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
                if (cuboidFail)
                    App::uiThread->messageBox_warning(App::mainWindow,"Alignment",IDSN_INFORM_SHAPE_COULD_NOT_BE_REORIENTED_ALONG_CUBOID,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
#endif
            }
            App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
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
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            App::logMsg(sim_verbosity_msgs,IDSNS_GROUPING_SELECTED_SHAPES);
            if (groupSelection(&sel,true)!=-1)
            {
                App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
                App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
            }
            else
                App::logMsg(sim_verbosity_msgs,IDSNS_ABORTED);
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
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            App::logMsg(sim_verbosity_msgs,IDSNS_UNGROUPING_SELECTED_SHAPES);
            ungroupSelection(&sel,true);
            App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
            App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
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
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            App::logMsg(sim_verbosity_msgs,IDSNS_MERGING_SELECTED_SHAPES);
            if (mergeSelection(&sel,true)>=0)
            {
                App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
                App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
            }
            else
                App::logMsg(sim_verbosity_msgs,IDSNS_ABORTED);
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
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            App::logMsg(sim_verbosity_msgs,IDSNS_DIVIDING_SELECTED_SHAPES);
            divideSelection(&sel,true);
            App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
            App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

#ifdef SIM_WITH_GUI
    if (commandID==SCENE_OBJECT_OPERATION_UNDO_SOOCMD)
    {
        if (App::getEditModeType()==NO_EDIT_MODE)
        {
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                App::logMsg(sim_verbosity_msgs,IDSNS_EXECUTING_UNDO);
                App::currentWorld->undoBufferContainer->undo();
                App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
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
        if (App::getEditModeType()==NO_EDIT_MODE)
        {
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                App::logMsg(sim_verbosity_msgs,IDSNS_EXECUTING_REDO);
                App::currentWorld->undoBufferContainer->redo();
                App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
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
#endif

    return(false);
}

void CSceneObjectOperations::addRootObjectChildrenToSelection(std::vector<int>& selection)
{
    for (int i=0;i<int(selection.size());i++)
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(selection[i]);
        if ( (it!=nullptr)&&it->getModelBase() )
        {
            std::vector<CSceneObject*> newObjs;
            it->getAllObjectsRecursive(&newObjs,false,true);
            for (int j=0;j<int(newObjs.size());j++)
            {
                if (!App::currentWorld->sceneObjects->isObjectInSelection(newObjs[j]->getObjectHandle(),&selection))
                    selection.push_back(newObjs[j]->getObjectHandle());
            }
        }
    }
}

void CSceneObjectOperations::copyObjects(std::vector<int>* selection,bool displayMessages)
{
    if (displayMessages)
        App::uiThread->showOrHideProgressBar(true,-1.0f,"Copying objects...");

    // We first copy the selection:
    std::vector<int> sel(*selection);
    addRootObjectChildrenToSelection(sel);
    App::worldContainer->copyBuffer->copyCurrentSelection(&sel,App::currentWorld->environment->getSceneLocked(),0);
    App::currentWorld->sceneObjects->deselectObjects(); // We clear selection

    if (displayMessages)
        App::uiThread->showOrHideProgressBar(false);
}

void CSceneObjectOperations::pasteCopyBuffer(bool displayMessages)
{
    TRACE_INTERNAL;
#ifdef SIM_WITH_GUI
    if (displayMessages)
        App::uiThread->showOrHideProgressBar(true,-1.0f,"Pasting objects...");
#endif

    bool failed=(App::worldContainer->copyBuffer->pasteBuffer(App::currentWorld->environment->getSceneLocked(),3)==-1);

#ifdef SIM_WITH_GUI
    if (displayMessages)
        App::uiThread->showOrHideProgressBar(false);

    if (failed) // Error: trying to copy locked buffer into unlocked scene!
    {
        if (displayMessages)
            App::uiThread->messageBox_warning(App::mainWindow,"Paste",IDS_SCENE_IS_LOCKED_CANNOT_PASTE_WARNING,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
    }
#endif
}

void CSceneObjectOperations::cutObjects(std::vector<int>* selection,bool displayMessages)
{
    TRACE_INTERNAL;
    if (displayMessages)
        App::uiThread->showOrHideProgressBar(true,-1.0f,"Cutting objects...");

    addRootObjectChildrenToSelection(*selection);
    copyObjects(selection,false);
    deleteObjects(selection,false);
    App::currentWorld->sceneObjects->deselectObjects(); // We clear selection

    if (displayMessages)
        App::uiThread->showOrHideProgressBar(false);
}

void CSceneObjectOperations::deleteObjects(std::vector<int>* selection,bool displayMessages)
{ // There are a few other spots where objects get deleted (e.g. the C-interface)
    TRACE_INTERNAL;
    if (displayMessages)
        App::uiThread->showOrHideProgressBar(true,-1.0f,"Deleting objects...");

    addRootObjectChildrenToSelection(selection[0]);
    App::currentWorld->sceneObjects->eraseSeveralObjects(selection[0],true);
    App::currentWorld->sceneObjects->deselectObjects();

    if (displayMessages)
        App::uiThread->showOrHideProgressBar(false);
}

int CSceneObjectOperations::groupSelection(std::vector<int>* selection,bool showMessages)
{
    if (App::currentWorld->sceneObjects->getShapeCountInSelection(selection)!=int(selection->size()))
        return(-1);
    if (selection->size()<2)
        return(-1);

    // Check if some shapes are pure primitives, convex, and check if we have a heightfield:
    size_t pureCount=0;
    size_t convexCount=0;
    bool includesHeightfields=false;
    std::vector<CShape*> shapesToGroup;
    for (size_t i=0;i<selection->size();i++)
    {
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(selection->at(i));
        if (it!=nullptr)
        {
            shapesToGroup.push_back(it);
            if (it->getMeshWrapper()->isPure())
            {
                pureCount++;
                if ( (it->getMeshWrapper()->isMesh())&&(it->getSingleMesh()->getPurePrimitiveType()==sim_pure_primitive_heightfield) )
                    includesHeightfields=true;
            }
            if (it->getMeshWrapper()->isConvex())
                convexCount++;
        }
    }

    bool onlyPureShapes=false;
    if ( (pureCount!=0)&&(pureCount!=shapesToGroup.size()) )
    { // we are mixing pure and non-pure shapes. Ask what to do
#ifdef SIM_WITH_GUI
        if (showMessages)
        {
            if (VMESSAGEBOX_REPLY_YES!=App::uiThread->messageBox_warning(App::mainWindow,IDSN_GROUPING,IDS_GROUPING_PURE_AND_NON_PURE_SHAPES_PROCEED_INFO_MESSAGE,VMESSAGEBOX_YES_NO,VMESSAGEBOX_REPLY_YES))
                return(-1); // we abort
        }
#endif
    }
    else
    {
        onlyPureShapes=true;
        if (includesHeightfields)
        {
#ifdef SIM_WITH_GUI
            if (showMessages)
                App::uiThread->messageBox_critical(App::mainWindow,IDS_GROUPING_MERGING_MENU_ITEM,IDS_GROUPING_HEIGHTFIELDS_ERROR_MESSAGE,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
#endif
            return(-1); // we abort, heightfields cannot be grouped
        }
    }

    if (showMessages)
        App::uiThread->showOrHideProgressBar(true,-1.0f,"Grouping shapes...");

    App::currentWorld->sceneObjects->deselectObjects();

    CShape* compoundShape=_groupShapes(shapesToGroup);

    App::currentWorld->sceneObjects->selectObject(compoundShape->getObjectHandle());

    if (showMessages)
        App::uiThread->showOrHideProgressBar(false);

    return(compoundShape->getObjectHandle());

}

CShape* CSceneObjectOperations::_groupShapes(const std::vector<CShape*>& shapesToGroup)
{ // returned shape is the last shape in the selection (it is modified, otherse are destroyed)
    size_t pureCount=0;
    bool includesHeightfields=false;
    bool allConvex=true;
    for (size_t i=0;i<shapesToGroup.size();i++)
    {
        CShape* it=shapesToGroup[i];
        if (it->getMeshWrapper()->isPure())
        {
            pureCount++;
            if ( (it->getMeshWrapper()->isMesh())&&(it->getSingleMesh()->getPurePrimitiveType()==sim_pure_primitive_heightfield) )
                includesHeightfields=true;
        }
        if (!it->getMeshWrapper()->isConvex())
            allConvex=false;
    }
    bool allToNonPure=( (pureCount<shapesToGroup.size())||includesHeightfields );
    for (size_t i=0;i<shapesToGroup.size();i++)
    {
        CShape* it=shapesToGroup[i];
        if (allToNonPure)
            it->getMeshWrapper()->setPurePrimitiveType(sim_pure_primitive_none,1.0f,1.0f,1.0f); // this will be propagated to all geometrics!

        App::currentWorld->drawingCont->announceObjectWillBeErased(it->getObjectHandle());
        App::currentWorld->pointCloudCont->announceObjectWillBeErased(it->getObjectHandle());
        App::currentWorld->bannerCont->announceObjectWillBeErased(it->getObjectHandle());
    }

    CShape* lastSel=shapesToGroup[shapesToGroup.size()-1];

    // Let's first compute the composed mass and center of mass:
    C3Vector newCenterOfMass; // absolute
    newCenterOfMass.clear();
    float cumulMass=0.0f;
    for (size_t i=0;i<shapesToGroup.size();i++)
    {
        CShape* it=shapesToGroup[i];
        newCenterOfMass+=it->getFullCumulativeTransformation()*it->getMeshWrapper()->getLocalInertiaFrame().X*it->getMeshWrapper()->getMass();
        cumulMass+=it->getMeshWrapper()->getMass();
    }
    newCenterOfMass/=cumulMass;
    C3X3Matrix composedInertia; // relative to world
    C7Vector newInertiaFrame(C4Vector::identityRotation,newCenterOfMass);
    composedInertia.clear();

    // Now the recipient and the first item:
    CMeshWrapper* theWrap=new CMeshWrapper();
    theWrap->setConvex(allConvex);
    theWrap->childList.push_back(lastSel->getMeshWrapper());
    lastSel->getMeshWrapper()->setTransformationsSinceGrouping(C7Vector::identityTransformation); // so that we can properly (i.e. like it was before) reorient the shape after ungrouping
    lastSel->getMeshWrapper()->setName(lastSel->getObjectAlias());
    C7Vector tmp(newInertiaFrame.getInverse()*lastSel->getFullCumulativeTransformation()*lastSel->getMeshWrapper()->getLocalInertiaFrame());
    composedInertia+=CMeshWrapper::getNewTensor(lastSel->getMeshWrapper()->getPrincipalMomentsOfInertia(),tmp)*lastSel->getMeshWrapper()->getMass();

    // now the other items:
    std::vector<int> shapesToErase;
    for (size_t i=0;i<shapesToGroup.size()-1;i++)
    {
        CShape* it=shapesToGroup[i];
        it->getMeshWrapper()->setTransformationsSinceGrouping(C7Vector::identityTransformation); // so that we can properly (i.e. like it was before) reorient the shape after ungrouping
        it->getMeshWrapper()->setName(it->getObjectAlias());

        tmp=newInertiaFrame.getInverse()*it->getFullCumulativeTransformation()*it->getMeshWrapper()->getLocalInertiaFrame();
        composedInertia+=CMeshWrapper::getNewTensor(it->getMeshWrapper()->getPrincipalMomentsOfInertia(),tmp)*it->getMeshWrapper()->getMass();

        C7Vector correctionTr=lastSel->getFullCumulativeTransformation().getInverse()*it->getFullCumulativeTransformation();
        it->getMeshWrapper()->preMultiplyAllVerticeLocalFrames(correctionTr);
        theWrap->childList.push_back(it->getMeshWrapper());
        it->disconnectMesh();
        shapesToErase.push_back(it->getObjectHandle()); // erase it later (if we do it now, texture dependencies might get mixed up)
    }

    lastSel->disconnectMesh();
    C7Vector newTr(lastSel->reinitMesh2(lastSel->getFullCumulativeTransformation(),theWrap));
    theWrap->setMass(cumulMass);
    C7Vector oldTrLocal(lastSel->getFullLocalTransformation());
    C7Vector newTrLocal(lastSel->getFullParentCumulativeTransformation().getInverse()*newTr);
    lastSel->setLocalTransformation(newTrLocal);

    // Set the composed inertia:
    composedInertia/=cumulMass; // remember, we only normally work with massless inertias!
    C7Vector newAbsOfComposedInertia;
    newAbsOfComposedInertia.X=newCenterOfMass;
    C3Vector principalMoments;
    CMeshWrapper::findPrincipalMomentOfInertia(composedInertia,newAbsOfComposedInertia.Q,principalMoments);
    lastSel->getMeshWrapper()->setLocalInertiaFrame(lastSel->getFullCumulativeTransformation().getInverse()*newAbsOfComposedInertia);
    lastSel->getMeshWrapper()->setPrincipalMomentsOfInertia(principalMoments);

    // correct the pos/orient. of all children of the 'lastSel' shape:
    for (size_t i=0;i<lastSel->getChildCount();i++)
    {
        CSceneObject* child=lastSel->getChildFromIndex(i);
        child->setLocalTransformation(newTrLocal.getInverse()*oldTrLocal*child->getLocalTransformation());
    }

    lastSel->actualizeContainsTransparentComponent();

    App::currentWorld->textureContainer->updateAllDependencies();

    App::currentWorld->sceneObjects->eraseSeveralObjects(shapesToErase,true);
    lastSel->pushObjectRefreshEvent();

    return(lastSel);
}

void CSceneObjectOperations::ungroupSelection(std::vector<int>* selection,bool showMessages)
{
    if (showMessages)
        App::uiThread->showOrHideProgressBar(true,-1.0f,"Ungrouping shapes...");
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

    if (showMessages)
        App::uiThread->showOrHideProgressBar(false);

    if (newObjectHandles.size()>0)
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->createStack();
        stack->pushTableOntoStack();
        stack->pushStringOntoStack("objectHandles",0);
        stack->pushInt32ArrayOntoStack(&newObjectHandles[0],newObjectHandles.size());
        stack->insertDataIntoStackTable();
        App::worldContainer->callScripts(sim_syscb_aftercreate,stack);
        App::worldContainer->interfaceStackContainer->destroyStack(stack);
    }
}

void CSceneObjectOperations::_fullUngroupShape(CShape* shape,std::vector<CShape*>& newShapes)
{
    std::vector<CShape*> toCheckAndUngroup;
    while (shape->isCompound())
    {
        std::vector<CShape*> ns;
        _ungroupShape(shape,ns);
        toCheckAndUngroup.insert(toCheckAndUngroup.end(),ns.begin(),ns.end());
    }
    for (size_t i=0;i<toCheckAndUngroup.size();i++)
    {
        std::vector<CShape*> ns;
        CShape* it=toCheckAndUngroup[i];
        _fullUngroupShape(it,ns);
        newShapes.push_back(it);
        newShapes.insert(newShapes.end(),ns.begin(),ns.end());
    }
}

void CSceneObjectOperations::CSceneObjectOperations::_ungroupShape(CShape* it,std::vector<CShape*>& newShapes)
{
    // Following 2 lines not needed, but added because a previous bug might have done something wrong! So here we make sure that all elements of the multishape are non-pure!!!
    if (!it->getMeshWrapper()->isPure())
        it->getMeshWrapper()->setPurePrimitiveType(sim_pure_primitive_none,1.0f,1.0f,1.0f);

    // we have to remove all attached drawing objects (we cannot correct for that or it would be very difficult!!)
    App::currentWorld->drawingCont->announceObjectWillBeErased(it->getObjectHandle());
    App::currentWorld->pointCloudCont->announceObjectWillBeErased(it->getObjectHandle());
    App::currentWorld->bannerCont->announceObjectWillBeErased(it->getObjectHandle());

    CMeshWrapper* oldGeomInfo=it->getMeshWrapper();
    it->disconnectMesh();
    C7Vector itCumulTransf(it->getFullCumulativeTransformation());
    for (int i=int(oldGeomInfo->childList.size())-1;i>=0;i--)
    {
        if (i==0)
        { // the first element in the list keeps its original shape
            C7Vector itOldLocal=it->getFullLocalTransformation();

            C7Vector newTr(it->reinitMesh2(itCumulTransf,oldGeomInfo->childList[i]));
            C7Vector itNewLocal(it->getFullParentCumulativeTransformation().getInverse()*newTr);
            it->setLocalTransformation(itNewLocal);

            // Now correct for all attached chil objects:
            for (size_t j=0;j<it->getChildCount();j++)
            {
                CSceneObject* aChild=it->getChildFromIndex(j);
                C7Vector oldChild=aChild->getLocalTransformation();
                aChild->setLocalTransformation(itNewLocal.getInverse()*itOldLocal*oldChild);
            }

            // Correctly reorient the shape to what we had before grouping (important for inertia frames that are relative to the shape's frame):
            C7Vector tr(oldGeomInfo->childList[i]->getTransformationsSinceGrouping());
            C7Vector currentLocal=it->getFullLocalTransformation();
            C7Vector tempLocal=it->getFullParentCumulativeTransformation().getInverse()*tr.getInverse();
            it->setLocalTransformation(tempLocal);
            it->alignBoundingBoxWithWorld();
            it->setLocalTransformation(currentLocal*tempLocal.getInverse()*it->getFullLocalTransformation());
            it->pushObjectRefreshEvent();
        }
        else
        { // the other elements in the list will receive a new shape
            // reinitMesh2
            CShape* newIt=new CShape(itCumulTransf,oldGeomInfo->childList[i]);

            newIt->setObjectAlias_direct(oldGeomInfo->childList[i]->getName().c_str());
            newIt->setObjectName_direct_old(oldGeomInfo->childList[i]->getName().c_str());
            newIt->setObjectAltName_direct_old(tt::getObjectAltNameFromObjectName(newIt->getObjectName_old().c_str()).c_str());
            newIt->setDynMaterial(it->getDynMaterial()->copyYourself());

            App::currentWorld->sceneObjects->addObjectToScene(newIt,false,false);
            newShapes.push_back(newIt);

            // Now a few properties/things we want to be same for the new shape:
            App::currentWorld->sceneObjects->setObjectParent(newIt,it->getParent(),true);
            newIt->setSizeFactor(it->getSizeFactor());
            newIt->setObjectProperty(it->getObjectProperty());
            newIt->setLocalObjectSpecialProperty(it->getLocalObjectSpecialProperty());
            newIt->setVisibilityLayer(it->getVisibilityLayer());
            newIt->setShapeIsDynamicallyStatic(it->getShapeIsDynamicallyStatic());
            newIt->setRespondable(it->getRespondable());
            newIt->setDynamicCollisionMask(it->getDynamicCollisionMask());
            newIt->actualizeContainsTransparentComponent();

            // Correctly reorient the shape to what we had before grouping (important for inertia frames that are relative to the shape's frame):
            C7Vector tr(oldGeomInfo->childList[i]->getTransformationsSinceGrouping());
            C7Vector currentLocal=newIt->getFullLocalTransformation();
            C7Vector tempLocal=newIt->getFullParentCumulativeTransformation().getInverse()*tr.getInverse();

            newIt->setLocalTransformation(tempLocal);
            newIt->alignBoundingBoxWithWorld();
            newIt->setLocalTransformation(currentLocal*tempLocal.getInverse()*newIt->getFullLocalTransformation());
        }
    }
    oldGeomInfo->childList.clear();
    delete oldGeomInfo;

    App::currentWorld->textureContainer->updateAllDependencies();
}

int CSceneObjectOperations::mergeSelection(std::vector<int>* selection,bool showMessages)
{
    if (selection->size()<2)
        return(-1);

    std::vector<CShape*> shapesToMerge;

    // Check if some shapes are pure primitives:
    if (showMessages)
    {
        for (size_t i=0;i<selection->size();i++)
        {
            CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(selection->at(i));
            if (it!=nullptr)
            {
                if (it->getMeshWrapper()->isPure())
                {
#ifdef SIM_WITH_GUI
                    if (VMESSAGEBOX_REPLY_YES==App::uiThread->messageBox_warning(App::mainWindow,IDSN_MERGING,IDS_MERGING_SOME_PURE_SHAPES_PROCEED_INFO_MESSAGE,VMESSAGEBOX_YES_NO,VMESSAGEBOX_REPLY_YES))
                        break;
                    return(-1); // we abort
#else
                    break;
#endif
                }
            }
        }
    }

    // Check if some shapes contain textures:
    bool textureWarningOutput=false;
    for (size_t i=0;i<selection->size();i++)
    {
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(selection->at(i));
        if (it!=nullptr)
        {
            shapesToMerge.push_back(it);
            if (it->getMeshWrapper()->getTextureCount()!=0)
            {
#ifdef SIM_WITH_GUI
                if (showMessages)
                {
                    if ( (!textureWarningOutput)&&(VMESSAGEBOX_REPLY_NO==App::uiThread->messageBox_warning(App::mainWindow,IDSN_MERGING,IDS_MERGING_OR_DIVIDING_REMOVES_TEXTURES_PROCEED_INFO_MESSAGE,VMESSAGEBOX_YES_NO,VMESSAGEBOX_REPLY_YES)) )
                        return(-1); // we abort
                }
#endif
                textureWarningOutput=true;
            }
        }
    }

    if (showMessages)
        App::uiThread->showOrHideProgressBar(true,-1,"Merging shapes...");

    App::currentWorld->sceneObjects->deselectObjects();

    CShape* mergedShape=_mergeShapes(shapesToMerge);

    if (showMessages)
        App::uiThread->showOrHideProgressBar(false);

    App::currentWorld->sceneObjects->selectObject(mergedShape->getObjectHandle());

    return(mergedShape->getObjectHandle());
}

CShape* CSceneObjectOperations::_mergeShapes(const std::vector<CShape*>& allShapesToMerge)
{ // returned shape is the last shape in the selection (it is modified, otherse are destroyed)
    for (size_t i=0;i<allShapesToMerge.size();i++)
    {
        CShape* it=allShapesToMerge[i];
        if (it->getMeshWrapper()->getTextureCount()!=0)
        {
            App::currentWorld->textureContainer->announceGeneralObjectWillBeErased(it->getObjectHandle(),-1);
            it->getMeshWrapper()->removeAllTextures();
        }
    }

    // We have to decompose completely the last shape:
    CShape* lastSel=allShapesToMerge[allShapesToMerge.size()-1];
    std::vector<CShape*> allShapesExceptLast(allShapesToMerge.begin(),allShapesToMerge.end()-1);
    while (lastSel->isCompound())
    {
        std::vector<CShape*> ns;
        _ungroupShape(lastSel,ns);
        allShapesExceptLast.insert(allShapesExceptLast.end(),ns.begin(),ns.end());
    }

    lastSel->getMeshWrapper()->setPurePrimitiveType(sim_pure_primitive_none,1.0f,1.0f,1.0f);

    if (allShapesExceptLast.size()>0)
    {
        // we have to remove all attached drawing objects (we cannot correct for that or it would be very difficult!!)
        App::currentWorld->drawingCont->announceObjectWillBeErased(lastSel->getObjectHandle());
        App::currentWorld->pointCloudCont->announceObjectWillBeErased(lastSel->getObjectHandle());
        App::currentWorld->bannerCont->announceObjectWillBeErased(lastSel->getObjectHandle());

        std::vector<float> wvert;
        std::vector<int> wind;
        lastSel->getMeshWrapper()->getCumulativeMeshes(wvert,&wind,nullptr);
        float cumulMass=lastSel->getMeshWrapper()->getMass();
        C7Vector tr(lastSel->getFullCumulativeTransformation());
        for (size_t i=0;i<wvert.size()/3;i++)
        {
            C3Vector v(&wvert[3*i+0]);
            v*=tr;
            wvert[3*i+0]=v(0);
            wvert[3*i+1]=v(1);
            wvert[3*i+2]=v(2);
        }
        int voff=(int)wvert.size();
        for (size_t j=0;j<allShapesExceptLast.size();j++)
        {
            CShape* aShape=allShapesExceptLast[j];
            aShape->getMeshWrapper()->getCumulativeMeshes(wvert,&wind,nullptr);
            cumulMass+=aShape->getMeshWrapper()->getMass();
            tr=aShape->getFullCumulativeTransformation();
            for (size_t i=voff/3;i<wvert.size()/3;i++)
            {
                C3Vector v(&wvert[3*i+0]);
                v*=tr;
                wvert[3*i+0]=v(0);
                wvert[3*i+1]=v(1);
                wvert[3*i+2]=v(2);
            }
            App::currentWorld->sceneObjects->eraseObject(aShape,true);
            voff=(int)wvert.size();
        }

        // We now have in wvert and wind all the vertices and indices (absolute vertices)
        CMesh* geometricTemp=lastSel->getSingleMesh()->copyYourself();

        C7Vector newTr(lastSel->reinitMesh(nullptr,wvert,wind,nullptr,nullptr));
        C7Vector lastSelPreviousLocal(lastSel->getFullLocalTransformation());
        C7Vector lastSelCurrentLocal(lastSel->getFullParentCumulativeTransformation().getInverse()*newTr);
        lastSel->setLocalTransformation(lastSelCurrentLocal);

        // Copy some CMesh properties (not all):
        geometricTemp->copyVisualAttributesTo(lastSel->getSingleMesh());
        lastSel->getSingleMesh()->actualizeGouraudShadingAndVisibleEdges(); // since 21/3/2014

        // Copy the CMeshWrapper properties (not all):
        lastSel->getMeshWrapper()->checkIfConvex();
        lastSel->getMeshWrapper()->setMass(cumulMass); // we do not copy here!

        // Do not copy following:
        //      lastSel->geomInfo->setPrincipalMomentsOfInertia(lastSel->geomInfo->getPrincipalMomentsOfInertia());
        lastSel->getMeshWrapper()->setLocalInertiaFrame(C7Vector::identityTransformation); // to have the inertia frame centered in the geometric middle of the mesh!
        lastSel->pushObjectRefreshEvent();

        delete geometricTemp;

        // Adjust the transformation of all its children:
        for (size_t i=0;i<lastSel->getChildCount();i++)
        {
            CSceneObject* child=lastSel->getChildFromIndex(i);
            child->setLocalTransformation(lastSelCurrentLocal.getInverse()*lastSelPreviousLocal*child->getFullLocalTransformation());
        }
    }
    return(lastSel);
}

void CSceneObjectOperations::divideSelection(std::vector<int>* selection,bool showMessages)
{
    if (selection->size()<1)
        return;

    // Check if some shapes contain textures:
    bool textureWarningOutput=false;
    std::vector<CShape*> shapesToDivide;
    for (size_t i=0;i<selection->size();i++)
    {
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(selection->at(i));
        if ( (it!=nullptr)&&(!it->getMeshWrapper()->isPure()) )
        {
            shapesToDivide.push_back(it);
            if (it->getMeshWrapper()->getTextureCount()!=0)
                textureWarningOutput=true;
        }
    }
#ifdef SIM_WITH_GUI
    if ( showMessages&&textureWarningOutput&&(VMESSAGEBOX_REPLY_NO==App::uiThread->messageBox_warning(App::mainWindow,"Dividing",IDS_MERGING_OR_DIVIDING_REMOVES_TEXTURES_PROCEED_INFO_MESSAGE,VMESSAGEBOX_YES_NO,VMESSAGEBOX_REPLY_YES)) )
        return; // we abort
#endif

    std::vector<int> newObjectHandles;
    App::currentWorld->sceneObjects->deselectObjects();
    selection->clear();

    if (showMessages)
        App::uiThread->showOrHideProgressBar(true,-1,"Dividing shapes...");

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
        App::worldContainer->callScripts(sim_syscb_aftercreate,stack);
        App::worldContainer->interfaceStackContainer->destroyStack(stack);
    }

    if (showMessages)
        App::uiThread->showOrHideProgressBar(false);
}

bool CSceneObjectOperations::_divideShape(CShape* shape,std::vector<CShape*>& newShapes)
{
    App::currentWorld->textureContainer->announceGeneralObjectWillBeErased(shape->getObjectHandle(),-1);
    shape->getMeshWrapper()->removeAllTextures();

    _fullUngroupShape(shape,newShapes);
    std::vector<CShape*> toDivide;
    toDivide.push_back(shape);
    toDivide.insert(toDivide.end(),newShapes.begin(),newShapes.end());

    for (size_t i=0;i<toDivide.size();i++)
    { // divide all non-compound, non-pure shapes:
        CShape* it=toDivide[i];
        if (!it->getMeshWrapper()->isPure())
        {
            std::vector<float> wvert;
            std::vector<int> wind;
            it->getMeshWrapper()->getCumulativeMeshes(wvert,&wind,nullptr);
            int extractedCount=0;
            while (true)
            {
                std::vector<float> subvert;
                std::vector<int> subind;
                if (CMeshManip::extractOneShape(&wvert,&wind,&subvert,&subind))
                { // Something was extracted
                    extractedCount++;

                    C7Vector tmpTr(it->getFullCumulativeTransformation());
                    CShape* newIt=new CShape(&tmpTr,subvert,subind,nullptr,nullptr);

                    // Now a few properties/things we want to be same for the new shape:
                    newIt->setSizeFactor(it->getSizeFactor());
                    newIt->setObjectProperty(it->getObjectProperty());
                    newIt->setLocalObjectSpecialProperty(it->getLocalObjectSpecialProperty());
                    newIt->setVisibilityLayer(it->getVisibilityLayer());

                    // Copy some CMesh properties:
                    it->getSingleMesh()->copyVisualAttributesTo(newIt->getSingleMesh());
                    newIt->getSingleMesh()->actualizeGouraudShadingAndVisibleEdges();
                    newIt->setDynMaterial(it->getDynMaterial()->copyYourself());
                    newIt->getMeshWrapper()->setLocalInertiaFrame(C7Vector::identityTransformation); // to have the inertia frame centered in the geometric middle of the mesh!
                    newIt->actualizeContainsTransparentComponent();

                    App::currentWorld->sceneObjects->addObjectToScene(newIt,false,false);
                    App::currentWorld->sceneObjects->setObjectParent(newIt,it->getParent(),true);
                    newShapes.push_back(newIt);
                }
                else
                { // nothing was extracted
                    if (extractedCount==0)
                        break; // we couldn't extract anything!

                    // Now adjust the old shape:
                    C7Vector tmpTr(it->getFullCumulativeTransformation());
                    C7Vector itLocalOld(it->getFullLocalTransformation());
                    CMesh* oldGeomCopy=it->getSingleMesh()->copyYourself();
                    C7Vector newTr(it->reinitMesh(&tmpTr,subvert,subind,nullptr,nullptr));
                    it->setLocalTransformation(it->getFullParentCumulativeTransformation().getInverse()*newTr);

                    // Copy the CMesh properties (not all):
                    oldGeomCopy->copyVisualAttributesTo(it->getSingleMesh());
                    it->getSingleMesh()->actualizeGouraudShadingAndVisibleEdges(); // since 21/3/2014

                    // Do not copy following:
                    //      it->geomInfo->setPrincipalMomentsOfInertia(it->geomInfo->getPrincipalMomentsOfInertia());
                    it->getMeshWrapper()->setLocalInertiaFrame(C7Vector::identityTransformation); // to have the inertia frame centered in the geometric middle of the mesh!
                    //      it->geomInfo->setLocalInertiaFrame(bla);

                    delete oldGeomCopy;
                    it->actualizeContainsTransparentComponent();
                    it->pushObjectRefreshEvent();

                    // Now we have to adjust all the children:
                    for (size_t j=0;j<it->getChildCount();j++)
                    {
                        CSceneObject* child=it->getChildFromIndex(j);
                        child->setLocalTransformation(it->getLocalTransformation().getInverse()*itLocalOld*child->getLocalTransformation());
                    }
                    break;
                }
            }
        }
    }
    return(newShapes.size()>0);
}

void CSceneObjectOperations::scaleObjects(const std::vector<int>& selection,float scalingFactor,bool scalePositionsToo)
{
    std::vector<int> sel(selection);
    CSceneObjectOperations::addRootObjectChildrenToSelection(sel);
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

    App::setFullDialogRefreshFlag();
}

int CSceneObjectOperations::generateConvexHull(int shapeHandle)
{
    TRACE_INTERNAL;
    std::vector<float> allHullVertices;
    allHullVertices.reserve(40000*3);

    CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
    if (it!=nullptr)
    {
        C7Vector transf(it->getFullCumulativeTransformation());
        std::vector<float> vert;
        std::vector<float> vertD;
        std::vector<int> ind;
        it->getMeshWrapper()->getCumulativeMeshes(vertD,&ind,nullptr);
        for (int j=0;j<int(vertD.size())/3;j++)
        {
            C3Vector v(&vertD[3*j+0]);
            v=transf*v;
            allHullVertices.push_back(v(0));
            allHullVertices.push_back(v(1));
            allHullVertices.push_back(v(2));
        }
    }

    if (allHullVertices.size()!=0)
    {
        std::vector<float> hull;
        std::vector<int> indices;
        std::vector<float> normals;
        if (CMeshRoutines::getConvexHull(&allHullVertices,&hull,&indices))
        {
            CShape* it=new CShape(nullptr,hull,indices,nullptr,nullptr);
            it->getSingleMesh()->setConvexVisualAttributes();
            it->getMeshWrapper()->setLocalInertiaFrame(C7Vector::identityTransformation);
            App::currentWorld->sceneObjects->addObjectToScene(it,false,true);
            return(it->getObjectHandle());
        }
    }
    return(-1);
}

int CSceneObjectOperations::generateConvexDecomposed(int shapeHandle,size_t nClusters,double maxConcavity,
                                             bool addExtraDistPoints,bool addFacesPoints,double maxConnectDist,
                                             size_t maxTrianglesInDecimatedMesh,size_t maxHullVertices,
                                             double smallClusterThreshold,bool individuallyConsiderMultishapeComponents,
                                             int maxIterations,bool useHACD,int resolution_VHACD,int depth_VHACD_old,float concavity_VHACD,
                                             int planeDownsampling_VHACD,int convexHullDownsampling_VHACD,
                                             float alpha_VHACD,float beta_VHACD,float gamma_VHACD_old,bool pca_VHACD,
                                             bool voxelBased_VHACD,int maxVerticesPerCH_VHACD,float minVolumePerCH_VHACD)
{
    TRACE_INTERNAL;
    std::vector<float> vert;
    std::vector<int> ind;
    CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
    if (it!=nullptr)
    {
        C7Vector tr(it->getFullCumulativeTransformation());
        std::vector<int> generatedShapeHandles;
        if (individuallyConsiderMultishapeComponents&&(!it->getMeshWrapper()->isMesh()))
        {
            std::vector<CMesh*> shapeComponents;
            it->getMeshWrapper()->getAllShapeComponentsCumulative(shapeComponents);
            for (int comp=0;comp<int(shapeComponents.size());comp++)
            {
                CMesh* geom=shapeComponents[comp];
                vert.clear();
                ind.clear();
                geom->getCumulativeMeshes(vert,&ind,nullptr);
                for (int j=0;j<int(vert.size()/3);j++)
                {
                    C3Vector v(&vert[3*j+0]);
                    v=tr*v;
                    vert[3*j+0]=v(0);
                    vert[3*j+1]=v(1);
                    vert[3*j+2]=v(2);
                }
                std::vector<std::vector<float>*> outputVert;
                std::vector<std::vector<int>*> outputInd;
                int addClusters=0;
                for (int tryNumber=0;tryNumber<maxIterations;tryNumber++)
                { // the convex decomposition routine sometimes fails producing good convectivity (i.e. there are slightly non-convex items that CoppeliaSim doesn't want to recognize as convex)
                    // For those situations, we try several times to convex decompose:
                    outputVert.clear();
                    outputInd.clear();
                    std::vector<int> _tempHandles;
                    CMeshRoutines::convexDecompose(&vert[0],(int)vert.size(),&ind[0],(int)ind.size(),outputVert,outputInd,
                            nClusters,maxConcavity,addExtraDistPoints,addFacesPoints,maxConnectDist,
                            maxTrianglesInDecimatedMesh,maxHullVertices,smallClusterThreshold,
                            useHACD,resolution_VHACD,depth_VHACD_old,concavity_VHACD,planeDownsampling_VHACD,
                            convexHullDownsampling_VHACD,alpha_VHACD,beta_VHACD,gamma_VHACD_old,pca_VHACD,
                            voxelBased_VHACD,maxVerticesPerCH_VHACD,minVolumePerCH_VHACD);
                    int convexRecognizedCount=0;
                    for (size_t i=0;i<outputVert.size();i++)
                    {
                        int handle=simCreateMeshShape_internal(2,20.0f*piValue_f/180.0f,&outputVert[i]->at(0),(int)outputVert[i]->size(),&outputInd[i]->at(0),(int)outputInd[i]->size(),nullptr);
                        CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(handle);
                        if (shape!=nullptr)
                        {
                            // Following flag is automatically set upon shape creation. Also, it seems that the convex decomposition algo sometimes failes..
                            if (shape->getSingleMesh()->isConvex())
                                convexRecognizedCount++; // CoppeliaSim convex test is more strict than what the convex decomp. algo does
                            _tempHandles.push_back(handle);
                            shape->getSingleMesh()->setConvexVisualAttributes();
                            // Set some visual parameters:
                            shape->setColor(nullptr,sim_colorcomponent_ambient_diffuse,0.7f,1.0f,0.7f);
                            shape->getSingleMesh()->setEdgeThresholdAngle(0.0f);
                            shape->getSingleMesh()->setShadingAngle(0.0f);
                            shape->getSingleMesh()->setVisibleEdges(false);
                        }
                        delete outputVert[i];
                        delete outputInd[i];
                    }
                    // we check if all shapes are recognized as convex shapes by CoppeliaSim
                    if ( (convexRecognizedCount==int(outputVert.size())) || (tryNumber>=maxIterations-1) )
                    {
                        for (int i=0;i<int(_tempHandles.size());i++)
                            generatedShapeHandles.push_back(_tempHandles[i]);
                        break;
                    }
                    else
                    { // No! Some shapes have a too large non-convexity. We take all generated shapes, and use them to generate new convex shapes:
                        vert.clear();
                        ind.clear();
                        for (int i=0;i<int(_tempHandles.size());i++)
                        {
                            CShape* as=App::currentWorld->sceneObjects->getShapeFromHandle(_tempHandles[i]);
                            if (as!=nullptr)
                            {
                                C7Vector tr2(as->getFullCumulativeTransformation());
                                CMesh* geom=as->getSingleMesh();
                                int offset=(int)vert.size()/3;
                                geom->getCumulativeMeshes(vert,&ind,nullptr);
                                for (int j=offset;j<int(vert.size()/3);j++)
                                {
                                    C3Vector v(&vert[3*j+0]);
                                    v=tr2*v;
                                    vert[3*j+0]=v(0);
                                    vert[3*j+1]=v(1);
                                    vert[3*j+2]=v(2);
                                }
                            }
                            simRemoveObject_internal(_tempHandles[i]);
                        }
                        // We adjust some parameters a bit, in order to obtain a better convexity for all shapes:
                        addClusters+=2;
                        nClusters=addClusters+int(_tempHandles.size());
                    }
                }
            }
        }
        else
        {
            it->getMeshWrapper()->getCumulativeMeshes(vert,&ind,nullptr);
            for (int j=0;j<int(vert.size()/3);j++)
            {
                C3Vector v(&vert[3*j+0]);
                v=tr*v;
                vert[3*j+0]=v(0);
                vert[3*j+1]=v(1);
                vert[3*j+2]=v(2);
            }
            std::vector<std::vector<float>*> outputVert;
            std::vector<std::vector<int>*> outputInd;

            int addClusters=0;
            for (int tryNumber=0;tryNumber<maxIterations;tryNumber++)
            { // the convex decomposition routine sometimes fails producing good convectivity (i.e. there are slightly non-convex items that CoppeliaSim doesn't want to recognize as convex)
                // For those situations, we try several times to convex decompose:
                outputVert.clear();
                outputInd.clear();
                std::vector<int> _tempHandles;
                CMeshRoutines::convexDecompose(&vert[0],(int)vert.size(),&ind[0],(int)ind.size(),outputVert,outputInd,
                        nClusters,maxConcavity,addExtraDistPoints,addFacesPoints,maxConnectDist,
                        maxTrianglesInDecimatedMesh,maxHullVertices,smallClusterThreshold,
                        useHACD,resolution_VHACD,depth_VHACD_old,concavity_VHACD,planeDownsampling_VHACD,
                        convexHullDownsampling_VHACD,alpha_VHACD,beta_VHACD,gamma_VHACD_old,pca_VHACD,
                        voxelBased_VHACD,maxVerticesPerCH_VHACD,minVolumePerCH_VHACD);
                int convexRecognizedCount=0;
                for (int i=0;i<int(outputVert.size());i++)
                {
                    int handle=simCreateMeshShape_internal(2,20.0f*piValue_f/180.0f,&outputVert[i]->at(0),(int)outputVert[i]->size(),&outputInd[i]->at(0),(int)outputInd[i]->size(),nullptr);
                    CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(handle);
                    if (shape!=nullptr)
                    {
                        // Following flag is automatically set upon shape creation. Also, it seems that the convex decomposition algo sometimes failes..
                        if (shape->getSingleMesh()->isConvex())
                            convexRecognizedCount++; // CoppeliaSim convex test is more strict than what the convex decomp. algo does
                        _tempHandles.push_back(handle);

                        shape->getSingleMesh()->setConvexVisualAttributes();
                        // Set some visual parameters:
                        shape->setColor(nullptr,sim_colorcomponent_ambient_diffuse,0.7f,1.0f,0.7f);
                        shape->getSingleMesh()->setEdgeThresholdAngle(0.0f);
                        shape->getSingleMesh()->setShadingAngle(0.0f);
                        shape->getSingleMesh()->setVisibleEdges(false);
                    }
                    delete outputVert[i];
                    delete outputInd[i];
                }
                // we check if all shapes are recognized as convex shapes by CoppeliaSim
                if ( (convexRecognizedCount==int(outputVert.size())) || (tryNumber>=maxIterations-1) )
                {
                    for (int i=0;i<int(_tempHandles.size());i++)
                        generatedShapeHandles.push_back(_tempHandles[i]);
                    break;
                }
                else
                { // No! Some shapes have a too large non-convexity. We take all generated shapes, and use them to generate new convex shapes:
                    vert.clear();
                    ind.clear();
                    for (int i=0;i<int(_tempHandles.size());i++)
                    {
                        CShape* as=App::currentWorld->sceneObjects->getShapeFromHandle(_tempHandles[i]);
                        if (as!=nullptr)
                        {
                            C7Vector tr2(as->getFullCumulativeTransformation());
                            CMesh* geom=as->getSingleMesh();
                            int offset=(int)vert.size()/3;
                            geom->getCumulativeMeshes(vert,&ind,nullptr);
                            for (int j=offset;j<int(vert.size()/3);j++)
                            {
                                C3Vector v(&vert[3*j+0]);
                                v=tr2*v;
                                vert[3*j+0]=v(0);
                                vert[3*j+1]=v(1);
                                vert[3*j+2]=v(2);
                            }
                        }
                        simRemoveObject_internal(_tempHandles[i]);
                    }
                    // We adjust some parameters a bit, in order to obtain a better convexity for all shapes:
                    addClusters+=2;
                    nClusters=addClusters+int(_tempHandles.size());
                }
            }
        }


        int newShapeHandle=-1;
        if (generatedShapeHandles.size()==1)
            newShapeHandle=generatedShapeHandles[0];
        if (generatedShapeHandles.size()>1)
            newShapeHandle=simGroupShapes_internal(&generatedShapeHandles[0],(int)generatedShapeHandles.size()); // we have to group them first
        return(newShapeHandle);
    }
    return(-1);
}

int CSceneObjectOperations::convexDecompose_apiVersion(int shapeHandle,int options,const int* intParams,const float* floatParams)
{
    TRACE_INTERNAL;
    int retVal=-1;

#ifdef SIM_WITH_GUI
    if (App::mainWindow==nullptr)
#endif
        options=(options|2)-2; // we are in headless mode: do not display the dialog!

    static bool addExtraDistPoints=true;
    static bool addFacesPoints=true;
    static int nClusters=1;
    static int maxHullVertices=200; // from 100 to 200 on 5/2/2014
    static float maxConcavity=100.0f;
    static float smallClusterThreshold=0.25f;
    static int maxTrianglesInDecimatedMesh=500;
    static float maxConnectDist=30.0f;
    static bool individuallyConsiderMultishapeComponents=false;
    static int maxIterations=4;
    static bool useHACD=true;
    static int resolution=100000;
// not present in newest VHACD   static int depth=20;
    static float concavity=0.0025f;
    static int planeDownsampling=4;
    static int convexHullDownsampling=4;
    static float alpha=0.05f;
    static float beta=0.05f;
// not present in newest VHACD   static float gamma=0.00125f;
    static bool pca=false;
    static bool voxelBasedMode=true;
    static int maxVerticesPerCH=64;
    static float minVolumePerCH=0.0001f;

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
        individuallyConsiderMultishapeComponents=(options&32)!=0;
        maxIterations=intParams[3];
        if (maxIterations<=0)
            maxIterations=4; // zero asks for default value
        useHACD=true; // forgotten, fixed thanks to Patrick Gruener
        if (options&128)
        { // we have more parameters than usual (i.e. the V-HACD parameters):
            useHACD=false;
            resolution=intParams[5];
            // depth=intParams[6];
            concavity=floatParams[5];
            planeDownsampling=intParams[7];
            convexHullDownsampling=intParams[8];
            alpha=floatParams[6];
            beta=floatParams[7];
            //gamma=floatParams[8];
            pca=(options&256);
            voxelBasedMode=!(options&512);
            maxVerticesPerCH=intParams[9];
            minVolumePerCH=floatParams[9];
        }
    }
    bool abortp=false;
    if ((options&2)!=0)
    {
        // Display the dialog:
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=DISPLAY_CONVEX_DECOMPOSITION_DIALOG_UITHREADCMD;
        cmdIn.boolParams.push_back(addExtraDistPoints);
        cmdIn.boolParams.push_back(addFacesPoints);
        cmdIn.intParams.push_back(nClusters);
        cmdIn.intParams.push_back(maxHullVertices);
        cmdIn.floatParams.push_back(maxConcavity);
        cmdIn.floatParams.push_back(smallClusterThreshold);
        cmdIn.intParams.push_back(maxTrianglesInDecimatedMesh);
        cmdIn.floatParams.push_back(maxConnectDist);
        cmdIn.boolParams.push_back(individuallyConsiderMultishapeComponents);
        cmdIn.boolParams.push_back(false);
        cmdIn.intParams.push_back(maxIterations);
        cmdIn.boolParams.push_back(false);
        cmdIn.boolParams.push_back(useHACD);
        cmdIn.intParams.push_back(resolution);
        cmdIn.intParams.push_back(20); //depth);
        cmdIn.floatParams.push_back(concavity);
        cmdIn.intParams.push_back(planeDownsampling);
        cmdIn.intParams.push_back(convexHullDownsampling);
        cmdIn.floatParams.push_back(alpha);
        cmdIn.floatParams.push_back(beta);
        cmdIn.floatParams.push_back(0.00125f); //gamma);
        cmdIn.boolParams.push_back(pca);
        cmdIn.boolParams.push_back(voxelBasedMode);
        cmdIn.intParams.push_back(maxVerticesPerCH);
        cmdIn.floatParams.push_back(minVolumePerCH);

        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);

        // Retrieve the chosen settings:
        abortp=cmdOut.boolParams[4];
        if (!abortp)
        {
            addExtraDistPoints=cmdOut.boolParams[0];
            addFacesPoints=cmdOut.boolParams[1];
            nClusters=cmdOut.intParams[0];
            maxHullVertices=cmdOut.intParams[1];
            maxConcavity=cmdOut.floatParams[0];
            smallClusterThreshold=cmdOut.floatParams[1];
            maxTrianglesInDecimatedMesh=cmdOut.intParams[2];
            maxConnectDist=cmdOut.floatParams[2];
            individuallyConsiderMultishapeComponents=cmdOut.boolParams[2];
            maxIterations=cmdOut.intParams[3];
            useHACD=cmdOut.boolParams[5];
            resolution=cmdOut.intParams[4];
            // depth=cmdOut.intParams[5];
            concavity=cmdOut.floatParams[3];
            planeDownsampling=cmdOut.intParams[6];
            convexHullDownsampling=cmdOut.intParams[7];
            alpha=cmdOut.floatParams[4];
            beta=cmdOut.floatParams[5];
            // gamma=cmdOut.floatParams[6];
            pca=cmdOut.boolParams[6];
            voxelBasedMode=cmdOut.boolParams[7];
            maxVerticesPerCH=cmdOut.intParams[8];
            minVolumePerCH=cmdOut.floatParams[7];
        }
    }
    if (!abortp)
        retVal=CSceneObjectOperations::generateConvexDecomposed(shapeHandle,nClusters,maxConcavity,addExtraDistPoints,
                                                        addFacesPoints,maxConnectDist,maxTrianglesInDecimatedMesh,
                                                        maxHullVertices,smallClusterThreshold,individuallyConsiderMultishapeComponents,
                                                        maxIterations,useHACD,resolution,20,concavity,planeDownsampling,
                                                        convexHullDownsampling,alpha,beta,0.00125f,pca,voxelBasedMode,
                                                        maxVerticesPerCH,minVolumePerCH);
    else
        retVal=-1;

    if (retVal!=-1)
    { // transfer the inertia and mass:
        // Get the mass and inertia info from the old shape:
        CShape* oldShape=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
        C7Vector absCOM(oldShape->getFullCumulativeTransformation());
        absCOM=absCOM*oldShape->getMeshWrapper()->getLocalInertiaFrame();
        float mass=oldShape->getMeshWrapper()->getMass();
        C7Vector absCOMNoShift(absCOM);
        absCOMNoShift.X.clear(); // we just wanna get the orientation of the inertia matrix, no shift info!
        C3X3Matrix tensor(CMeshWrapper::getNewTensor(oldShape->getMeshWrapper()->getPrincipalMomentsOfInertia(),absCOMNoShift));

        // Transfer the mass and inertia info to the new shape:
        CShape* newShape=App::currentWorld->sceneObjects->getShapeFromHandle(retVal);
        newShape->getMeshWrapper()->setMass(mass);
        C4Vector rot;
        C3Vector pmoi;
        CMeshWrapper::findPrincipalMomentOfInertia(tensor,rot,pmoi);
        newShape->getMeshWrapper()->setPrincipalMomentsOfInertia(pmoi);
        absCOM.Q=rot;
        C7Vector relCOM(newShape->getFullCumulativeTransformation().getInverse()*absCOM);
        newShape->getMeshWrapper()->setLocalInertiaFrame(relCOM);
    }

    if ( (retVal!=-1)&&((options&1)!=0) )
    { // we wanted a morph!!
        CShape* newShape=App::currentWorld->sceneObjects->getShapeFromHandle(retVal);
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
        C7Vector newLocal(it->getFullParentCumulativeTransformation().getInverse()*newShape->getFullCumulativeTransformation());
        C7Vector oldLocal(it->getFullLocalTransformation());
        it->setNewMesh(newShape->getMeshWrapper());
        newShape->disconnectMesh();
        it->setLocalTransformation(newLocal); // The shape's frame was changed!
        App::currentWorld->sceneObjects->eraseObject(newShape,true);
        // We need to correct all its children for this change of frame:
        for (size_t i=0;i<it->getChildCount();i++)
        {
            CSceneObject* child=it->getChildFromIndex(i);
            child->setLocalTransformation(newLocal.getInverse()*oldLocal*child->getLocalTransformation());
            child->incrementMemorizedConfigurationValidCounter(); // so if we are running in a simulation, the shape doesn't get reset at its initial config
        }
        retVal=shapeHandle;
    }
    return(retVal);
}

#ifdef SIM_WITH_GUI
void CSceneObjectOperations::addMenu(VMenu* menu)
{
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
            if (it->getMeshWrapper()->isMesh())
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
        lastSelIsNonPureShape=!sh->getMeshWrapper()->isPure();
        lastSelIsNonGrouping=!sh->isCompound();
    }

    bool lastSelIsPath=App::currentWorld->sceneObjects->isLastSelectionAPath();
    bool hasChildScriptAttached=false;
    bool hasCustomizationScriptAttached=false;
    if (selItems==1)
    {
        hasChildScriptAttached=(App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo_child(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(0))!=nullptr);
        hasCustomizationScriptAttached=(App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo_customization(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(0))!=nullptr);
    }
    std::vector<int> rootSel;
    for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
        rootSel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
    CSceneObjectOperations::addRootObjectChildrenToSelection(rootSel);
    size_t shapesInRootSel=App::currentWorld->sceneObjects->getShapeCountInSelection(&rootSel);
    if (App::getEditModeType()==NO_EDIT_MODE)
    {
        menu->appendMenuItem(App::currentWorld->undoBufferContainer->canUndo(),false,SCENE_OBJECT_OPERATION_UNDO_SOOCMD,IDSN_UNDO);
        menu->appendMenuItem(App::currentWorld->undoBufferContainer->canRedo(),false,SCENE_OBJECT_OPERATION_REDO_SOOCMD,IDSN_REDO);
        menu->appendMenuSeparator();
        if (CSimFlavor::getBoolVal(11))
        {
            menu->appendMenuItem(selItems>1,false,SCENE_OBJECT_OPERATION_MAKE_PARENT_SOOCMD,IDS_MAKE_LAST_SELECTED_OBJECTS_PARENT_MENU_ITEM);
            menu->appendMenuItem(selItems>0,false,SCENE_OBJECT_OPERATION_MAKE_ORPHANS_SOOCMD,IDS_MAKE_SELECTED_OBJECT_S__ORPHAN_MENU_ITEM);
            menu->appendMenuSeparator();
        }
        if (CSimFlavor::getBoolVal(12))
        {
//          menu->appendMenuItem((selItems>1)&&noSim,false,SCENE_OBJECT_OPERATION_REPLACE_OBJECTS_SOOCMD,IDS_REPLACE_OBJECTS_MENU_ITEM);
            menu->appendMenuItem((shapesInRootSel>0)&&noSim,false,SCENE_OBJECT_OPERATION_MORPH_INTO_CONVEX_SHAPES_SOOCMD,IDS_CONVEX_MORPH_MENU_ITEM);
            menu->appendMenuItem((shapesInRootSel>0)&&noSim,false,SCENE_OBJECT_OPERATION_MORPH_INTO_CONVEX_DECOMPOSITION_SOOCMD,IDS_CONVEX_DECOMPOSITION_MORPH_MENU_ITEM);
            menu->appendMenuItem(lastSelIsShape&&(selItems==1)&&noSim&&lastSelIsNonPureShape,false,SCENE_OBJECT_OPERATION_DECIMATE_SHAPE_SOOCMD,IDS_MESH_DECIMATION_MENU_ITEM);
//            menu->appendMenuItem(lastSelIsShape&&(selItems==1)&&noSim&&lastSelIsNonPureShape&&lastSelIsNonGrouping,false,SCENE_OBJECT_OPERATION_EXTRACT_SHAPE_INSIDE_SOOCMD,IDS_REMOVE_SHAPE_INSIDE_MENU_ITEM);

            if ((selItems==2)&&(selDummies==2))
            { // we have 2 selected dummies we might want to link/unlink:
                CDummy* dumA=App::currentWorld->sceneObjects->getDummyFromHandle(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(0));
                CDummy* dumB=App::currentWorld->sceneObjects->getDummyFromHandle(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(1));
                if ((dumA!=nullptr)&&(dumB!=nullptr))
                {
                    if (dumA->getLinkedDummyHandle()==dumB->getObjectHandle())
                        menu->appendMenuItem(true,false,SCENE_OBJECT_OPERATION_UNLINK_DUMMIES_SOOCMD,IDS_UNLINK_SELECTED_DUMMIES_MENU_ITEM);
                    else
                    {
                        VMenu* dummyLinking=new VMenu();
                        dummyLinking->appendMenuItem(true,false,SCENE_OBJECT_OPERATION_LINK_DUMMIES_IK_TIP_TARGET_SOOCMD,IDS_DUMMY_LINK_TYPE_IK_TIP_TARGET);
                        dummyLinking->appendMenuItem(true,false,SCENE_OBJECT_OPERATION_LINK_DUMMIES_GCS_LOOP_CLOSURE_SOOCMD,IDS_DUMMY_LINK_TYPE_GCS_LOOP_CLOSURE);
                        dummyLinking->appendMenuItem(true,false,SCENE_OBJECT_OPERATION_LINK_DUMMIES_GCS_TIP_SOOCMD,IDS_DUMMY_LINK_TYPE_GCS_TIP);
                        dummyLinking->appendMenuItem(true,false,SCENE_OBJECT_OPERATION_LINK_DUMMIES_GCS_TARGET_SOOCMD,IDS_DUMMY_LINK_TYPE_GCS_TARGET);
                        dummyLinking->appendMenuItem(true,false,SCENE_OBJECT_OPERATION_LINK_DUMMIES_DYNAMICS_LOOP_CLOSURE_SOOCMD,IDS_DUMMY_LINK_TYPE_DYNAMICS_LOOP_CLOSURE);
                        menu->appendMenuAndDetach(dummyLinking,true,IDS_LINK_SELECTED_DUMMIES_LINK_TYPE_MENU_ITEM);
                    }
                }
            }
        }
        menu->appendMenuSeparator();
        menu->appendMenuItem(selItems>0,false,SCENE_OBJECT_OPERATION_OBJECT_FULL_COPY_SOOCMD,IDS_COPY_SELECTED_OBJECTS_MENU_ITEM);
        menu->appendMenuItem(!App::worldContainer->copyBuffer->isBufferEmpty(),false,SCENE_OBJECT_OPERATION_PASTE_OBJECTS_SOOCMD,IDS_PASTE_BUFFER_MENU_ITEM);
        menu->appendMenuItem(selItems>0,false,SCENE_OBJECT_OPERATION_DELETE_OBJECTS_SOOCMD,IDS_DELETE_SELECTED_OBJECTS_MENU_ITEM);
        menu->appendMenuItem(selItems>0,false,SCENE_OBJECT_OPERATION_OBJECT_FULL_CUT_SOOCMD,IDS_CUT_SELECTED_OBJECTS_MENU_ITEM);
        menu->appendMenuSeparator();
        if (CSimFlavor::getBoolVal(11))
        {
            menu->appendMenuItem(true,false,SCENE_OBJECT_OPERATION_SELECT_ALL_OBJECTS_SOOCMD,IDSN_SELECT_ALL_MENU_ITEM);
            menu->appendMenuSeparator();
        }

        if (CSimFlavor::getBoolVal(12))
        {
            VMenu* removing=new VMenu();
            removing->appendMenuItem(hasChildScriptAttached&&noSim,false,SCENE_OBJECT_OPERATION_REMOVE_ASSOCIATED_CHILD_SCRIPT_SOOCMD,"Associated child script");
            removing->appendMenuItem(hasCustomizationScriptAttached&&noSim,false,SCENE_OBJECT_OPERATION_REMOVE_ASSOCIATED_CUSTOMIZATION_SCRIPT_SOOCMD,"Associated customization script");
            menu->appendMenuAndDetach(removing,(hasChildScriptAttached||hasCustomizationScriptAttached)&&noSim,IDSN_REMOVE_MENU_ITEM);
            menu->appendMenuSeparator();

            VMenu* grouping=new VMenu();
            grouping->appendMenuItem((shapeNumber==selItems)&&(selItems>1)&&noSim,false,SCENE_OBJECT_OPERATION_GROUP_SHAPES_SOOCMD,IDS_GROUP_SELECTED_SHAPES_MENU_ITEM);
            grouping->appendMenuItem((shapeNumber==selItems)&&(selItems>0)&&noSim,false,SCENE_OBJECT_OPERATION_UNGROUP_SHAPES_SOOCMD,IDS_UNGROUP_SELECTED_SHAPES_MENU_ITEM);
            grouping->appendMenuSeparator();
            grouping->appendMenuItem((shapeNumber==selItems)&&(selItems>1)&&noSim,false,SCENE_OBJECT_OPERATION_MERGE_SHAPES_SOOCMD,IDS_MERGE_SELECTED_SHAPES_MENU_ITEM);
            grouping->appendMenuItem((shapeNumber==selItems)&&(selItems>0)&&noSim,false,SCENE_OBJECT_OPERATION_DIVIDE_SHAPES_SOOCMD,IDS_DIVIDE_SELECTED_SHAPES_MENU_ITEM);
            menu->appendMenuAndDetach(grouping,true,IDS_GROUPING_MERGING_MENU_ITEM);

            VMenu* align=new VMenu();
            align->appendMenuItem((shapeNumber==selItems)&&(selItems!=0)&&noSim,false,SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_WORLD_SOOCMD,IDS_ALIGN_SELECTED_SHAPE_WORLD_MENU_ITEM);
            align->appendMenuItem((shapeNumber==selItems)&&(selItems!=0)&&noSim,false,SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_MAIN_AXIS_SOOCMD,IDS_ALIGN_SELECTED_SHAPE_MAIN_AXIS_MENU_ITEM);
            align->appendMenuItem((shapeNumber==selItems)&&(selItems!=0)&&noSim,false,SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_TUBE_MAIN_AXIS_SOOCMD,IDSN_ALIGN_BB_WITH_TUBE);
            align->appendMenuItem((shapeNumber==selItems)&&(selItems!=0)&&noSim,false,SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_CUBOID_MAIN_AXIS_SOOCMD,IDSN_ALIGN_BB_WITH_CUBOID);
            menu->appendMenuAndDetach(align,true,IDS_BOUNDING_BOX_ALIGNMENT_MENU_ITEM);
        }
    }
}
#endif
