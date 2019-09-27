
#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "v_rep_internal.h"
#include "sceneObjectOperations.h"
#include "simulation.h"
#include "geometric.h"
#include "app.h"
#include "meshManip.h"
#include "tt.h"
#include "v_repStrings.h"
#include "meshRoutines.h"
#include <boost/lexical_cast.hpp>
#ifdef SIM_WITH_GUI
    #include "vMessageBox.h"
    #include "qdlgconvexdecomposition.h"
    #include "sceneObjectOperationsBase.h"
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
    if (commandID==SCENE_OBJECT_OPERATION_EMPTY_PATH_SOOCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
            if (sel.size()>0)
            {
                CPath* path=App::ct->objCont->getPath(sel[sel.size()-1]);
                if (path!=nullptr)
                {
                    path->pathContainer->removeAllSimplePathPoints();
                    POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                    App::addStatusbarMessage(IDSNS_PATH_WAS_EMPTIED);
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
    if ((commandID==SCENE_OBJECT_OPERATION_ROLL_PATH_POINTS_FORWARD_SOOCMD)||(commandID==SCENE_OBJECT_OPERATION_ROLL_PATH_POINTS_BACKWARD_SOOCMD))
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
            if (sel.size()>0)
            {
                CPath* path=App::ct->objCont->getPath(sel[sel.size()-1]);
                if (path!=nullptr)
                {
                    path->pathContainer->rollPathPoints(commandID==SCENE_OBJECT_OPERATION_ROLL_PATH_POINTS_FORWARD_SOOCMD);
                    POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                    if (commandID==SCENE_OBJECT_OPERATION_ROLL_PATH_POINTS_FORWARD_SOOCMD)
                        App::addStatusbarMessage(IDSNS_FORWARD_ROLL_PERFORMED);
                    else
                        App::addStatusbarMessage(IDSNS_BACKWARD_ROLL_PERFORMED);
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
    
    if (commandID==SCENE_OBJECT_OPERATION_ASSEMBLE_SOOCMD)
    {
        // There is another such routine!! XXBFVGA
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            bool assembleEnabled=false;
            bool disassembleEnabled=false;
            int selS=App::ct->objCont->getSelSize();
            if (selS==1)
            { // here we can only have disassembly
                C3DObject* it=App::ct->objCont->getLastSelection_object();
                disassembleEnabled=(it->getParentObject()!=nullptr)&&(it->getAssemblyMatchValues(true).length()!=0);
                if (disassembleEnabled)
                {
                    App::addStatusbarMessage(IDSN_DISASSEMBLING_OBJECT);
                    App::ct->objCont->makeObjectChildOf(it,nullptr);
                }
            }
            else if (selS==2)
            { // here we can have assembly or disassembly
                C3DObject* it1=App::ct->objCont->getLastSelection_object();
                C3DObject* it2=App::ct->objCont->getObjectFromHandle(App::ct->objCont->getSelID(0));
                if ((it1->getParentObject()==it2)||(it2->getParentObject()==it1))
                { // disassembly
                    if ( (it1->getParentObject()==it2) && (it1->getAssemblyMatchValues(true).length()!=0) )
                        disassembleEnabled=true;
                    if ( (it2->getParentObject()==it1) && (it2->getAssemblyMatchValues(true).length()!=0) )
                        disassembleEnabled=true;
                    if (disassembleEnabled)
                    {
                        App::addStatusbarMessage(IDSN_DISASSEMBLING_OBJECT);
                        if (it1->getParentObject()==it2)
                        {
                            App::ct->objCont->makeObjectChildOf(it1,nullptr);
                            App::ct->objCont->deselectObjects();
                            App::ct->objCont->addObjectToSelection(it1->getObjectHandle());
                            App::ct->objCont->addObjectToSelection(it2->getObjectHandle());
                        }
                        else
                            App::ct->objCont->makeObjectChildOf(it2,nullptr);
                    }
                }
                else
                { // assembly
                    std::vector<C3DObject*> potParents;
                    it1->getAllChildrenThatMayBecomeAssemblyParent(it2->getChildAssemblyMatchValuesPointer(),potParents);
                    bool directAssembly=it1->doesParentAssemblingMatchValuesMatchWithChild(it2->getChildAssemblyMatchValuesPointer());
                    if ( directAssembly||(potParents.size()==1) )
                    {
                        App::addStatusbarMessage(IDSN_ASSEMBLING_2_OBJECTS);
                        assembleEnabled=true;
                        if (directAssembly)
                            App::ct->objCont->makeObjectChildOf(it2,it1);
                        else
                            App::ct->objCont->makeObjectChildOf(it2,potParents[0]);
                        if (it2->getAssemblingLocalTransformationIsUsed())
                            it2->setLocalTransformation(it2->getAssemblingLocalTransformation());
                    }
                    else
                    { // here we might have the opposite of what we usually do to assemble (i.e. last selection should always be parent, but not here)
                        // we assemble anyways if the roles are unequivoque:
                        if ( it2->doesParentAssemblingMatchValuesMatchWithChild(it1->getChildAssemblyMatchValuesPointer()) )
                        {
                            App::addStatusbarMessage(IDSN_ASSEMBLING_2_OBJECTS);
                            assembleEnabled=true;
                            App::ct->objCont->makeObjectChildOf(it1,it2);
                            if (it1->getAssemblingLocalTransformationIsUsed())
                                it1->setLocalTransformation(it1->getAssemblingLocalTransformation());
                        }
                    }
                }
            }

            if (assembleEnabled||disassembleEnabled)
            {
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                App::addStatusbarMessage(IDSNS_DONE);
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
            int selS=App::ct->objCont->getSelSize();
            if (selS==1)
            {
                C3DObject* it=App::ct->objCont->getLastSelection_object();
                if (it->getLocalObjectProperty()&sim_objectproperty_canupdatedna)
                {
                    bool model=it->getModelBase();
                    std::vector<C3DObject*> siblings;
                    // Retrieve the sibling in the scene:
                    for (int i=0;i<int(App::ct->objCont->objectList.size());i++)
                    {
                        C3DObject* it2=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
                        if ( (it2!=it)&&(it2->getLocalObjectProperty()&sim_objectproperty_canupdatedna)&&(it2->getDnaString().compare(it->getDnaString())==0) )
                        {
                            if (!model)
                                siblings.push_back(it2);
                            else
                            { // Here we also have to check that the sibling model is not located in the same hierarchy as this one:
                                bool sameHierarchy=false;
                                C3DObject* ite=it2;
                                while (true)
                                {
                                    ite=ite->getParentObject();
                                    if (ite==nullptr)
                                        break;
                                    if (ite==it)
                                    {
                                        sameHierarchy=true;
                                        break;
                                    }
                                }
                                if (!sameHierarchy)
                                { // Here we also need to check if the sibling model is not located in the hierarchy of another sibling:
                                    sameHierarchy=false;
                                    ite=it2;
                                    while (true)
                                    {
                                        ite=ite->getParentObject();
                                        if (ite==nullptr)
                                            break;
                                        if ( (ite->getLocalObjectProperty()&sim_objectproperty_canupdatedna)&&(ite->getDnaString().compare(it->getDnaString())==0) )
                                        {
                                            sameHierarchy=true;
                                            break;
                                        }
                                    }
                                    if (!sameHierarchy)
                                        siblings.push_back(it2);
                                }
                            }
                        }
                    }
                    std::vector<int> newSelection;
                    if (siblings.size()>0)
                    {
                        App::addStatusbarMessage(IDSN_TRANSFERING_DNA_TO_SIBLINGS);
                        App::ct->copyBuffer->memorizeBuffer();

                        std::vector<int> sel;
                        sel.push_back(it->getObjectHandle());
                        if (model)
                            CSceneObjectOperations::addRootObjectChildrenToSelection(sel);
                        std::string masterName(it->getObjectName());
                        App::ct->copyBuffer->copyCurrentSelection(&sel,App::ct->environment->getSceneLocked());
                        App::ct->objCont->deselectObjects();
                        for (int i=0;i<int(siblings.size());i++)
                        {
                            if (!model)
                            {
                                std::string name(siblings[i]->getObjectName());
                                std::string altName(siblings[i]->getObjectAltName());
                                C3DObject* parent(siblings[i]->getParentObject());
                                C7Vector tr(siblings[i]->getLocalTransformationPart1());
                                std::vector<C3DObject*> children;
                                std::vector<C7Vector> childrenTr;
                                for (size_t j=0;j<siblings[i]->childList.size();j++)
                                {
                                    children.push_back(siblings[i]->childList[j]);
                                    childrenTr.push_back(siblings[i]->childList[j]->getLocalTransformationPart1());
                                }
                                App::ct->objCont->eraseObject(siblings[i],true);
                                App::ct->copyBuffer->pasteBuffer(App::ct->environment->getSceneLocked());
                                C3DObject* newObj=App::ct->objCont->getLastSelection_object();
                                App::ct->objCont->deselectObjects();
                                if (newObj!=nullptr)
                                {
                                    newSelection.push_back(newObj->getObjectHandle());
                                    App::ct->objCont->makeObjectChildOf(newObj,parent);
                                    newObj->setLocalTransformation(tr);
                                    for (size_t j=0;j<children.size();j++)
                                    {
                                        children[j]->setParentObject(newObj);
                                        children[j]->setLocalTransformation(childrenTr[j]);
                                    }
                                    if (App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_child(newObj->getObjectHandle())!=nullptr)
                                    { // we just try to keep the name suffix
                                        int oldSuffix=tt::getNameSuffixNumber(name.c_str(),true);
                                        std::string newName(tt::generateNewName_dash(tt::getNameWithoutSuffixNumber(masterName.c_str(),true),oldSuffix));
                                        while (App::ct->objCont->getObjectFromName(newName.c_str())!=nullptr)
                                            newName=tt::generateNewName_dash(newName);
                                        App::ct->objCont->renameObject(newObj->getObjectHandle(),newName.c_str());
                                        newName=tt::getObjectAltNameFromObjectName(altName);
                                        while (App::ct->objCont->getObjectFromAltName(newName.c_str())!=nullptr)
                                            newName=tt::generateNewName_noDash(newName);
                                        App::ct->objCont->altRenameObject(newObj->getObjectHandle(),newName.c_str());
                                    }
                                    else
                                    { // we keep the old names
                                        App::ct->objCont->renameObject(newObj->getObjectHandle(),name.c_str());
                                        App::ct->objCont->altRenameObject(newObj->getObjectHandle(),name.c_str());
                                    }
                                }
                            }
                            else
                            {
                                std::vector<int> objs;
                                objs.push_back(siblings[i]->getObjectHandle());
                                CSceneObjectOperations::addRootObjectChildrenToSelection(objs);
                                C7Vector tr(siblings[i]->getLocalTransformation());
                                C3DObject* parent(siblings[i]->getParentObject());
                                App::ct->objCont->eraseSeveralObjects(objs,true);
                                App::ct->copyBuffer->pasteBuffer(App::ct->environment->getSceneLocked());
                                App::ct->objCont->removeFromSelectionAllExceptModelBase(false);
                                C3DObject* newObj=App::ct->objCont->getLastSelection_object();
                                App::ct->objCont->deselectObjects();
                                if (newObj!=nullptr)
                                {
                                    newSelection.push_back(newObj->getObjectHandle());
                                    App::ct->objCont->makeObjectChildOf(newObj,parent);
                                    newObj->setLocalTransformation(tr);
                                }
                            }
                        }
                        App::ct->copyBuffer->restoreBuffer();
                        App::ct->copyBuffer->clearMemorizedBuffer();
                        App::addStatusbarMessage(IDSNS_DONE);
                        std::string txt;
                        txt+=boost::lexical_cast<std::string>(siblings.size())+IDSN_X_SIBLINGS_WERE_UPDATED;
                        App::addStatusbarMessage(txt.c_str());

                        for (int i=0;i<int(newSelection.size());i++)
                            App::ct->objCont->addObjectToSelection(newSelection[i]);

                        POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
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

    if (commandID==SCENE_OBJECT_OPERATION_MAKE_PARENT_SOOCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
            if (sel.size()>1)
            {
                C3DObject* last=App::ct->objCont->getObjectFromHandle(sel[sel.size()-1]);
                for (int i=0;i<int(sel.size())-1;i++)
                {
                    C3DObject* it=App::ct->objCont->getObjectFromHandle(sel[i]);
                    App::ct->objCont->makeObjectChildOf(it,last);
                }
                App::ct->objCont->selectObject(last->getObjectHandle()); // We select the parent

                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                std::string txt(IDSNS_ATTACHING_OBJECTS_TO);
                txt+=last->getObjectName()+"'...";
                App::addStatusbarMessage(txt.c_str());
                App::addStatusbarMessage(IDSNS_DONE);
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
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
            App::addStatusbarMessage(IDSNS_MAKING_ORPHANS);
            for (int i=0;i<int(sel.size());i++)
            {
                C3DObject* it=App::ct->objCont->getObjectFromHandle(sel[i]);
                App::ct->objCont->makeObjectChildOf(it,nullptr);
            }
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            App::ct->objCont->deselectObjects(); // We clear selection
            App::addStatusbarMessage(IDSNS_DONE);
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
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
//          CSceneObjectOperations::addRootObjectChildrenToSelection(sel);
            App::uiThread->showOrHideProgressBar(true,-1,"Morphing into convex shape(s)...");
            App::addStatusbarMessage(IDSNS_MORPHING_INTO_CONVEX_SHAPES);
            bool printQHullFail=false;
            for (int obji=0;obji<int(sel.size());obji++)
            {
                CShape* it=App::ct->objCont->getShape(sel[obji]);
                if (it!=nullptr)
                {
                    if ( (!it->geomData->geomInfo->isConvex())||it->isCompound() )
                    {
                        int newShapeHandle=generateConvexHull(sel[obji]);
                        if (newShapeHandle!=-1)
                        {
                            // Get the mass and inertia info from the old shape:
                            C7Vector absCOM(it->getCumulativeTransformation());
                            absCOM=absCOM*it->geomData->geomInfo->getLocalInertiaFrame();
                            float mass=it->geomData->geomInfo->getMass();
                            C7Vector absCOMNoShift(absCOM);
                            absCOMNoShift.X.clear(); // we just wanna get the orientation of the inertia matrix, no shift info!
                            C3X3Matrix tensor(CGeomWrap::getNewTensor(it->geomData->geomInfo->getPrincipalMomentsOfInertia(),absCOMNoShift));

                            // Set-up the new shape:
                            CShape* newShape=App::ct->objCont->getShape(newShapeHandle);
                            C7Vector newLocal(it->getParentCumulativeTransformation().getInverse()*newShape->getCumulativeTransformation());
                            C7Vector oldLocal(it->getLocalTransformation());
                            ((CGeometric*)newShape->geomData->geomInfo)->setConvexVisualAttributes();
                            delete it->geomData;
                            it->geomData=newShape->geomData; // we exchange the geomData object
                            it->setLocalTransformation(newLocal); // The shape's frame was changed!
                            it->incrementMemorizedConfigurationValidCounter(); // so if we are running in a simulation, the shape doesn't get reset at its initial config
                            newShape->geomData=nullptr;
                            App::ct->objCont->eraseObject(newShape,true);

                            // Transfer the mass and inertia info to the new shape:
                            it->geomData->geomInfo->setMass(mass);
                            C4Vector rot;
                            C3Vector pmoi;
                            CGeomWrap::findPrincipalMomentOfInertia(tensor,rot,pmoi);
                            it->geomData->geomInfo->setPrincipalMomentsOfInertia(pmoi);
                            absCOM.Q=rot;
                            C7Vector relCOM(it->getCumulativeTransformation().getInverse()*absCOM);
                            it->geomData->geomInfo->setLocalInertiaFrame(relCOM);

                            ((CGeometric*)it->geomData->geomInfo)->color.colors[0]=1.0f;
                            ((CGeometric*)it->geomData->geomInfo)->color.colors[1]=0.7f;
                            ((CGeometric*)it->geomData->geomInfo)->color.colors[2]=0.7f;
                            ((CGeometric*)it->geomData->geomInfo)->setEdgeThresholdAngle(0.0f);
                            ((CGeometric*)it->geomData->geomInfo)->setGouraudShadingAngle(0.0f);
                            ((CGeometric*)it->geomData->geomInfo)->setVisibleEdges(true);

                            // We need to correct all its children for this change of frame:
                            for (int i=0;i<int(it->childList.size());i++)
                            {
                                it->childList[i]->setLocalTransformation(newLocal.getInverse()*oldLocal*it->childList[i]->getLocalTransformationPart1());
                                it->childList[i]->incrementMemorizedConfigurationValidCounter(); // so if we are running in a simulation, the shape doesn't get reset at its initial config
                            }
                        }
                        else
                            printQHullFail=true;
                    }
                    else
                    { // that shape is not a compound and already convex. We just change its visual attributes:
                        ((CGeometric*)it->geomData->geomInfo)->setConvexVisualAttributes();
                    }
                }
            }

            App::uiThread->showOrHideProgressBar(false);

            App::ct->objCont->deselectObjects();
            if (printQHullFail)
                App::addStatusbarMessage(IDSNS_FAILED_IS_THE_QHULL_PLUGIN_LOADED);
            else
            {
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                App::addStatusbarMessage(IDSNS_DONE);
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
            if ( (App::ct->objCont->getSelSize()==1)&&(App::ct->objCont->getLastSelection_object()->getObjectType()==sim_object_shape_type) )
            {
                sh=App::ct->objCont->getLastSelection_shape();
                if (sh->geomData->geomInfo->isPure())
                    sh=nullptr;
            }
            if (sh!=nullptr)
            {
                std::vector<float> vert;
                std::vector<int> ind;
                sh->geomData->geomInfo->getCumulativeMeshes(vert,&ind,nullptr);
                C7Vector tr(sh->getCumulativeTransformation());
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
                        App::addStatusbarMessage(IDSNS_DECIMATING_MESH);

                        // Create the new shape:
                        CGeomProxy* geom=new CGeomProxy(nullptr,vertOut,indOut,nullptr,nullptr);
                        CShape* newShape=new CShape();
                        newShape->setLocalTransformation(geom->getCreationTransformation());
                        geom->setCreationTransformation(C7Vector::identityTransformation);
                        newShape->geomData=geom;
                        ((CGeometric*)geom->geomInfo)->setConvexVisualAttributes();
                        geom->geomInfo->setLocalInertiaFrame(C7Vector::identityTransformation);
                        App::ct->objCont->addObjectToScene(newShape,false,true);

                        // Get the mass and inertia info from the old shape:
                        C7Vector absCOM(sh->getCumulativeTransformation());
                        absCOM=absCOM*sh->geomData->geomInfo->getLocalInertiaFrame();
                        float mass=sh->geomData->geomInfo->getMass();
                        C7Vector absCOMNoShift(absCOM);
                        absCOMNoShift.X.clear(); // we just wanna get the orientation of the inertia matrix, no shift info!
                        C3X3Matrix tensor(CGeomWrap::getNewTensor(sh->geomData->geomInfo->getPrincipalMomentsOfInertia(),absCOMNoShift));

                        // Set-up the new shape:
                        C7Vector newLocal(sh->getParentCumulativeTransformation().getInverse()*newShape->getCumulativeTransformation());
                        C7Vector oldLocal(sh->getLocalTransformation());
                        delete sh->geomData;
                        sh->geomData=newShape->geomData; // we exchange the geomData object
                        sh->setLocalTransformation(newLocal); // The shape's frame was changed!
                        sh->incrementMemorizedConfigurationValidCounter(); // so if we are running in a simulation, the shape doesn't get reset at its initial config
                        newShape->geomData=nullptr;
                        App::ct->objCont->eraseObject(newShape,true);

                        // Transfer the mass and inertia info to the new shape:
                        sh->geomData->geomInfo->setMass(mass);
                        C4Vector rot;
                        C3Vector pmoi;
                        CGeomWrap::findPrincipalMomentOfInertia(tensor,rot,pmoi);
                        sh->geomData->geomInfo->setPrincipalMomentsOfInertia(pmoi);
                        absCOM.Q=rot;
                        C7Vector relCOM(sh->getCumulativeTransformation().getInverse()*absCOM);
                        sh->geomData->geomInfo->setLocalInertiaFrame(relCOM);

                        // Set some visual parameters:
                        ((CGeometric*)sh->geomData->geomInfo)->color.colors[0]=0.7f;
                        ((CGeometric*)sh->geomData->geomInfo)->color.colors[1]=0.7f;
                        ((CGeometric*)sh->geomData->geomInfo)->color.colors[2]=1.0f;
                        ((CGeometric*)sh->geomData->geomInfo)->setEdgeThresholdAngle(0.0f);
                        ((CGeometric*)sh->geomData->geomInfo)->setGouraudShadingAngle(0.0f);
                        ((CGeometric*)sh->geomData->geomInfo)->setVisibleEdges(true);

                        // We need to correct all its children for this change of frame:
                        for (int i=0;i<int(sh->childList.size());i++)
                        {
                            sh->childList[i]->setLocalTransformation(newLocal.getInverse()*oldLocal*sh->childList[i]->getLocalTransformationPart1());
                            sh->childList[i]->incrementMemorizedConfigurationValidCounter(); // so if we are running in a simulation, the shape doesn't get reset at its initial config
                        }

                        POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                        App::addStatusbarMessage(IDSNS_DONE);
                    }

                    App::uiThread->showOrHideProgressBar(false);

                    App::ct->objCont->deselectObjects();
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
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
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
                App::addStatusbarMessage(IDSNS_MORPHING_INTO_CONVEX_DECOMPOSITION);
                App::uiThread->showOrHideProgressBar(true,-1,"Morphing into convex decomposed shape(s)...");

                for (int obji=0;obji<int(sel.size());obji++)
                {
                    CShape* it=App::ct->objCont->getShape(sel[obji]);
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
                            C7Vector absCOM(it->getCumulativeTransformation());
                            absCOM=absCOM*it->geomData->geomInfo->getLocalInertiaFrame();
                            float mass=it->geomData->geomInfo->getMass();
                            C7Vector absCOMNoShift(absCOM);
                            absCOMNoShift.X.clear(); // we just wanna get the orientation of the inertia matrix, no shift info!
                            C3X3Matrix tensor(CGeomWrap::getNewTensor(it->geomData->geomInfo->getPrincipalMomentsOfInertia(),absCOMNoShift));

                            // Set-up the new shape:
                            CShape* newShape=App::ct->objCont->getShape(newShapeHandle);
                            C7Vector newLocal(it->getParentCumulativeTransformation().getInverse()*newShape->getCumulativeTransformation());
                            C7Vector oldLocal(it->getLocalTransformation());
                            delete it->geomData;
                            it->geomData=newShape->geomData; // we exchange the geomData object
                            it->setLocalTransformation(newLocal); // The shape's frame was changed!
                            it->incrementMemorizedConfigurationValidCounter(); // so if we are running in a simulation, the shape doesn't get reset at its initial config
                            newShape->geomData=nullptr;
                            App::ct->objCont->eraseObject(newShape,true);

                            // Transfer the mass and inertia info to the new shape:
                            it->geomData->geomInfo->setMass(mass);
                            C4Vector rot;
                            C3Vector pmoi;
                            CGeomWrap::findPrincipalMomentOfInertia(tensor,rot,pmoi);
                            it->geomData->geomInfo->setPrincipalMomentsOfInertia(pmoi);
                            absCOM.Q=rot;
                            C7Vector relCOM(it->getCumulativeTransformation().getInverse()*absCOM);
                            it->geomData->geomInfo->setLocalInertiaFrame(relCOM);

                            // We need to correct all its children for this change of frame:
                            for (int i=0;i<int(it->childList.size());i++)
                            {
                                it->childList[i]->setLocalTransformation(newLocal.getInverse()*oldLocal*it->childList[i]->getLocalTransformationPart1());
                                it->childList[i]->incrementMemorizedConfigurationValidCounter(); // so if we are running in a simulation, the shape doesn't get reset at its initial config
                            }
                        }
                    }
                }
                App::uiThread->showOrHideProgressBar(false);
                App::addStatusbarMessage(IDSNS_DONE);
            }
            App::ct->objCont->deselectObjects();
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==SCENE_OBJECT_OPERATION_EXTRACT_SHAPE_INSIDE_SOOCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            CShape* sh=nullptr;
            if ( (App::ct->objCont->getSelSize()==1)&&(App::ct->objCont->getLastSelection_object()->getObjectType()==sim_object_shape_type) )
            {
                sh=App::ct->objCont->getLastSelection_shape();
                if (sh->geomData->geomInfo->isPure())
                    sh=nullptr;
            }
            if (sh!=nullptr)
            {
                std::vector<float> vert;
                std::vector<int> ind;
                sh->geomData->geomInfo->getCumulativeMeshes(vert,&ind,nullptr);
                ((CGeometric*)sh->geomData->geomInfo)->setWireframe(false);
                int shapeId=sh->getObjectHandle();
                C7Vector tr(sh->getCumulativeTransformation());
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
                cmdIn.cmdId=DISPLAY_MESH_INSIDE_EXTRACTION_ITERATIONS_DIALOG_UITHREADCMD;
                cmdIn.intParams.push_back(1024);
                cmdIn.intParams.push_back(300);
                App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);

                if ((cmdOut.boolParams.size()>0)&&cmdOut.boolParams[0])
                {
                    int resolution=cmdOut.intParams[0];
                    int iterations=cmdOut.intParams[1];
                    CVisionSensor* visionSensor=new CVisionSensor();
                    visionSensor->setPerspectiveOperation(true);
                    visionSensor->setNearClippingPlane(0.01f);
                    visionSensor->setFarClippingPlane(50.0f);
                    int res[2]={resolution,resolution};
                    visionSensor->setDesiredResolution(res);
                    visionSensor->setViewAngle(60.0f*piValue_f/180.0f);
                    int newAttr=sim_displayattribute_renderpass|sim_displayattribute_forvisionsensor|sim_displayattribute_ignorerenderableflag|sim_displayattribute_colorcoded|sim_displayattribute_colorcodedtriangles;
                    visionSensor->setRenderMode(2);
                    visionSensor->setAttributesForRendering(newAttr);
                    visionSensor->setDetectableEntityID(sh->getObjectHandle());
                    visionSensor->setExplicitHandling(true);
                    App::ct->objCont->addObjectToScene(visionSensor,false,false);
                    int visionId=visionSensor->getObjectHandle();

                    CCamera* camera=new CCamera();
                    camera->setNearClippingPlane(0.01f);
                    camera->setFarClippingPlane(50.0f);
                    camera->setViewAngle(60.0f*piValue_f/180.0f);
                    App::ct->objCont->addObjectToScene(camera,false,false);
                    App::ct->objCont->makeObjectChildOf(visionSensor,camera);
                    visionSensor->setLocalTransformation(C7Vector::identityTransformation);

                    int floatingView=simFloatingViewAdd_internal(0.2f,0.8f,0.4f,0.4f,0);
                    simAdjustView_internal(floatingView,camera->getObjectHandle(),128,"");

                    std::vector<bool> seenTriangles(ind.size()/3,false);
                    sh->setForceAlwaysVisible_tmp(true);
                    for (int l=0;l<iterations;l++)
                    {
                        App::uiThread->showOrHideProgressBar(true,float(l)/float(iterations),"Extracting inside triangles...");

                        C7Vector camTr;
                        camTr.Q.buildRandomOrientation();
                        camTr.X.clear();
                        camera->setLocalTransformation(camTr);
                        simCameraFitToView_internal(floatingView,1,&shapeId,3,1.0f);
                        C3Vector c(camera->getCumulativeTransformation().X);
                        float* auxValues=nullptr;
                        int* auxValuesCount=nullptr;
                        if (simHandleVisionSensor_internal(visionId,&auxValues,&auxValuesCount)>=0)
                        {
                            if (auxValuesCount[0]>0)
                            {
                                for (int i=0;i<auxValuesCount[1];i++)
                                    seenTriangles[int(auxValues[i])]=true;
                            }
                            simReleaseBuffer_internal((char*)auxValues);
                            simReleaseBuffer_internal((char*)auxValuesCount);
                        }
                    }
                    App::uiThread->showOrHideProgressBar(false);

                    sh->setForceAlwaysVisible_tmp(false);

                    App::ct->objCont->eraseObject(visionSensor,false);
                    App::ct->objCont->eraseObject(camera,false);
                    simFloatingViewRemove_internal(floatingView);

                    size_t outsideCnt=0;
                    for (size_t i=0;i<seenTriangles.size();i++)
                    {
                        if (seenTriangles[i])
                            outsideCnt++;
                    }
                    if ( (outsideCnt!=seenTriangles.size())&&(outsideCnt!=0) )
                    {
                        App::addStatusbarMessage(IDSNS_EXTRACTING_INSIDE_PART);
                        std::vector<int> insideTris;
                        std::vector<int> outsideTris;
                        for (size_t i=0;i<seenTriangles.size();i++)
                        {
                            if (seenTriangles[i])
                            {
                                outsideTris.push_back(ind[3*i+0]);
                                outsideTris.push_back(ind[3*i+1]);
                                outsideTris.push_back(ind[3*i+2]);
                            }
                            else
                            {
                                insideTris.push_back(ind[3*i+0]);
                                insideTris.push_back(ind[3*i+1]);
                                insideTris.push_back(ind[3*i+2]);
                            }
                        }
                        // 1. Modify the original shape, keeping only the outside:
                        CGeomProxy* geom=new CGeomProxy(nullptr,vert,outsideTris,nullptr,nullptr);
                        CShape* newShape=new CShape();
                        newShape->setLocalTransformation(geom->getCreationTransformation());
                        geom->setCreationTransformation(C7Vector::identityTransformation);
                        newShape->geomData=geom;
                        ((CGeometric*)geom->geomInfo)->setConvexVisualAttributes();
                        geom->geomInfo->setLocalInertiaFrame(C7Vector::identityTransformation);
                        App::ct->objCont->addObjectToScene(newShape,false,true);
                        ((CGeometric*)sh->geomData->geomInfo)->color.copyYourselfInto(&((CGeometric*)geom->geomInfo)->color);
                        geom->geomInfo->setEdgeThresholdAngle(sh->geomData->geomInfo->getEdgeThresholdAngle());
                        geom->geomInfo->setHideEdgeBorders(sh->geomData->geomInfo->getHideEdgeBorders());
                        geom->geomInfo->setGouraudShadingAngle(sh->geomData->geomInfo->getGouraudShadingAngle());
                        ((CGeometric*)geom->geomInfo)->setVisibleEdges(((CGeometric*)sh->geomData->geomInfo)->getVisibleEdges());
                        SSimulationThreadCommand trCmd;
                        trCmd.cmdId=SET_SHAPE_TRANSPARENCY_CMD;
                        trCmd.intParams.push_back(shapeId);
                        trCmd.boolParams.push_back(((CGeometric*)sh->geomData->geomInfo)->color.translucid);
                        trCmd.floatParams.push_back(((CGeometric*)sh->geomData->geomInfo)->color.transparencyFactor);
                        App::simThread->appendSimulationThreadCommand(trCmd,2000);
                        ((CGeometric*)geom->geomInfo)->color.translucid=true;
                        ((CGeometric*)geom->geomInfo)->color.transparencyFactor=0.25f;

                        // Get the mass and inertia info from the old shape:
                        C7Vector absCOM(sh->getCumulativeTransformation());
                        absCOM=absCOM*sh->geomData->geomInfo->getLocalInertiaFrame();
                        float mass=sh->geomData->geomInfo->getMass();
                        C7Vector absCOMNoShift(absCOM);
                        absCOMNoShift.X.clear(); // we just wanna get the orientation of the inertia matrix, no shift info!
                        C3X3Matrix tensor(CGeomWrap::getNewTensor(sh->geomData->geomInfo->getPrincipalMomentsOfInertia(),absCOMNoShift));

                        // Set-up the new shape:
                        C7Vector newLocal(sh->getParentCumulativeTransformation().getInverse()*newShape->getCumulativeTransformation());
                        C7Vector oldLocal(sh->getLocalTransformation());
                        delete sh->geomData;
                        sh->geomData=newShape->geomData; // we exchange the geomData object
                        sh->setLocalTransformation(newLocal); // The shape's frame was changed!
                        sh->incrementMemorizedConfigurationValidCounter(); // so if we are running in a simulation, the shape doesn't get reset at its initial config
                        newShape->geomData=nullptr;
                        App::ct->objCont->eraseObject(newShape,true);
                        sh->actualizeContainsTransparentComponent();

                        // Transfer the mass and inertia info to the new shape:
                        sh->geomData->geomInfo->setMass(mass);
                        C4Vector rot;
                        C3Vector pmoi;
                        CGeomWrap::findPrincipalMomentOfInertia(tensor,rot,pmoi);
                        sh->geomData->geomInfo->setPrincipalMomentsOfInertia(pmoi);
                        absCOM.Q=rot;
                        C7Vector relCOM(sh->getCumulativeTransformation().getInverse()*absCOM);
                        sh->geomData->geomInfo->setLocalInertiaFrame(relCOM);

                        // We need to correct all its children for this change of frame:
                        for (int i=0;i<int(sh->childList.size());i++)
                        {
                            sh->childList[i]->setLocalTransformation(newLocal.getInverse()*oldLocal*sh->childList[i]->getLocalTransformationPart1());
                            sh->childList[i]->incrementMemorizedConfigurationValidCounter(); // so if we are running in a simulation, the shape doesn't get reset at its initial config
                        }

                        // 2. Create the inside shape
                        geom=new CGeomProxy(nullptr,vert,insideTris,nullptr,nullptr);
                        newShape=new CShape();
                        newShape->setLocalTransformation(geom->getCreationTransformation());
                        geom->setCreationTransformation(C7Vector::identityTransformation);
                        newShape->geomData=geom;
                        ((CGeometric*)geom->geomInfo)->setConvexVisualAttributes();
                        geom->geomInfo->setLocalInertiaFrame(C7Vector::identityTransformation);
                        App::ct->objCont->addObjectToScene(newShape,false,true);

                        // Set some visual parameters:
                        ((CGeometric*)newShape->geomData->geomInfo)->color.colors[0]=1.0f;
                        ((CGeometric*)newShape->geomData->geomInfo)->color.colors[1]=0.2f;
                        ((CGeometric*)newShape->geomData->geomInfo)->color.colors[2]=0.2f;
                        ((CGeometric*)newShape->geomData->geomInfo)->setEdgeThresholdAngle(0.0f);
                        ((CGeometric*)newShape->geomData->geomInfo)->setGouraudShadingAngle(0.0f);
                        ((CGeometric*)newShape->geomData->geomInfo)->setVisibleEdges(true);

                        App::ct->objCont->deselectObjects();
                        App::ct->objCont->selectObject(newShape->getObjectHandle());
                        POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                        App::addStatusbarMessage(IDSNS_DONE);
                    }
                    else
                        App::ct->objCont->deselectObjects();
                }
                else
                    App::ct->objCont->deselectObjects();
            }
            else
                App::ct->objCont->deselectObjects();
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
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
            if (sel.size()==2)
            {
                CDummy* a=App::ct->objCont->getDummy(sel[0]);
                CDummy* b=App::ct->objCont->getDummy(sel[1]);
                if ((a!=nullptr)&&(b!=nullptr))
                {
                    if (commandID==SCENE_OBJECT_OPERATION_UNLINK_DUMMIES_SOOCMD)
                    {
                        a->setLinkedDummyID(-1,false);
                        App::addStatusbarMessage("Unlinking selected dummies... Done.");
                    }
                    else
                    {
                        a->setLinkedDummyID(b->getObjectHandle(),false);
                        if (commandID==SCENE_OBJECT_OPERATION_LINK_DUMMIES_IK_TIP_TARGET_SOOCMD)
                            b->setLinkType(sim_dummy_linktype_ik_tip_target,false);
                        if (commandID==SCENE_OBJECT_OPERATION_LINK_DUMMIES_GCS_LOOP_CLOSURE_SOOCMD)
                            b->setLinkType(sim_dummy_linktype_gcs_loop_closure,false);
                        if (commandID==SCENE_OBJECT_OPERATION_LINK_DUMMIES_GCS_TIP_SOOCMD)
                            b->setLinkType(sim_dummy_linktype_gcs_tip,false);
                        if (commandID==SCENE_OBJECT_OPERATION_LINK_DUMMIES_GCS_TARGET_SOOCMD)
                            b->setLinkType(sim_dummy_linktype_gcs_target,false);
                        if (commandID==SCENE_OBJECT_OPERATION_LINK_DUMMIES_DYNAMICS_LOOP_CLOSURE_SOOCMD)
                            b->setLinkType(sim_dummy_linktype_dynamics_loop_closure,false);
                        App::addStatusbarMessage("Linking selected dummies... Done.");
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
            App::addStatusbarMessage(IDSNS_SELECTING_ALL_OBJECTS);
            for (int i=0;i<int(App::ct->objCont->objectList.size());i++)
                App::ct->objCont->addObjectToSelection(App::ct->objCont->objectList[i]);
            App::addStatusbarMessage(IDSNS_DONE);
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
            int id=App::ct->objCont->getLastSelectionID();
            CLuaScriptObject* script=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_child(id);
            if (script!=nullptr)
            {
                App::ct->luaScriptContainer->removeScript(script->getScriptID());
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
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
            int id=App::ct->objCont->getLastSelectionID();
            CLuaScriptObject* script=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_customization(id);
            if (script!=nullptr)
            {
                App::ct->luaScriptContainer->removeScript(script->getScriptID());
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
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
        App::ct->objCont->deselectObjects();
    }

    if (commandID==SCENE_OBJECT_OPERATION_OBJECT_FULL_COPY_SOOCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
            App::addStatusbarMessage(IDSNS_COPYING_SELECTION);
            copyObjects(&sel,true);
            App::addStatusbarMessage(IDSNS_DONE);
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
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
            {
                C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->getSelID(i));
                if ((it->getLocalObjectProperty()&sim_objectproperty_cannotdelete)==0)
                {
                    if ( ((it->getLocalObjectProperty()&sim_objectproperty_cannotdeleteduringsim)==0)||App::ct->simulation->isSimulationStopped() )
                        sel.push_back(it->getObjectHandle());
                }
            }
            if (sel.size()>0)
            {
                App::addStatusbarMessage(IDSNS_CUTTING_SELECTION);
                cutObjects(&sel,true);
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                App::addStatusbarMessage(IDSNS_DONE);
            }
            App::ct->objCont->deselectObjects();
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
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
            App::addStatusbarMessage(IDSNS_PASTING_BUFFER);
            pasteCopyBuffer(true);
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            App::addStatusbarMessage(IDSNS_DONE);
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
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
            {
                C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->getSelID(i));
                if ((it->getLocalObjectProperty()&sim_objectproperty_cannotdelete)==0)
                {
                    if ( ((it->getLocalObjectProperty()&sim_objectproperty_cannotdeleteduringsim)==0)||App::ct->simulation->isSimulationStopped() )
                        sel.push_back(it->getObjectHandle());
                }
            }
            if (sel.size()>0)
            {
                App::addStatusbarMessage(IDSNS_DELETING_SELECTION);
                deleteObjects(&sel,true);
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                App::addStatusbarMessage(IDSNS_DONE);
            }
            App::ct->objCont->deselectObjects();
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
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
            bool tubeFail=false;
            bool cuboidFail=false;
            if (commandID==SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_MAIN_AXIS_SOOCMD)
                App::addStatusbarMessage(IDSNS_ALIGNING_BOUNDING_BOXES_WITH_MAIN_AXIS);
            if (commandID==SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_WORLD_SOOCMD)
                App::addStatusbarMessage(IDSNS_ALIGNING_BOUNDING_BOXES_WITH_WORLD);
            if (commandID==SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_TUBE_MAIN_AXIS_SOOCMD)
                App::addStatusbarMessage(IDSNS_ALIGNING_BOUNDING_BOXES_WITH_TUBES);
            if (commandID==SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_CUBOID_MAIN_AXIS_SOOCMD)
                App::addStatusbarMessage(IDSNS_ALIGNING_BOUNDING_BOXES_WITH_CUBOIDS);
            if (App::ct->objCont->getShapeNumberInSelection(&sel)==int(sel.size()))
            {
                std::vector<void*> processedGeoms;
                processedGeoms.reserve(sel.size());
                bool informThatPurePrimitivesWereNotChanged=false;
                for (int i=0;i<int(sel.size());i++)
                {
                    CShape* theShape=App::ct->objCont->getShape(sel[i]);
                    if (theShape!=nullptr)
                    {
                        // Did we already process this geometric resource?
                        bool found=false;
                        for (int j=0;j<int(processedGeoms.size());j++)
                        {
                            if (processedGeoms[j]==(void*)theShape->geomData)
                                found=true;
                        }
                        if (!found)
                        {
                            processedGeoms.push_back(theShape->geomData);
                            if ( (!theShape->geomData->geomInfo->isPure())||(theShape->isCompound()) )
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
                                POST_SCENE_CHANGED_START_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                            }
                            else
                                informThatPurePrimitivesWereNotChanged=true;
                        }
                    }
                }
                POST_SCENE_CHANGED_END_ANNOUNCEMENT(); // ************************** UNDO thingy **************************
#ifdef SIM_WITH_GUI
                if (informThatPurePrimitivesWereNotChanged)
                    App::uiThread->messageBox_warning(App::mainWindow,strTranslate("Alignment"),strTranslate(IDS_INFORM_PURE_PRIMITIVES_COULD_NOT_BE_REORIENTED),VMESSAGEBOX_OKELI);
                if (tubeFail)
                    App::uiThread->messageBox_warning(App::mainWindow,strTranslate("Alignment"),strTranslate(IDSN_INFORM_SHAPE_COULD_NOT_BE_REORIENTED_ALONG_TUBE),VMESSAGEBOX_OKELI);
                if (cuboidFail)
                    App::uiThread->messageBox_warning(App::mainWindow,strTranslate("Alignment"),strTranslate(IDSN_INFORM_SHAPE_COULD_NOT_BE_REORIENTED_ALONG_CUBOID),VMESSAGEBOX_OKELI);
#endif
            }
            App::addStatusbarMessage(IDSNS_DONE);
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
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
            App::addStatusbarMessage(IDSNS_GROUPING_SELECTED_SHAPES);
            if (groupSelection(&sel,true)!=-1)
            {
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                App::addStatusbarMessage(IDSNS_DONE);
            }
            else
                App::addStatusbarMessage(IDSNS_ABORTED);
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
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
            App::addStatusbarMessage(IDSNS_UNGROUPING_SELECTED_SHAPES);
            ungroupSelection(&sel,true);
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            App::addStatusbarMessage(IDSNS_DONE);
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
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
            App::addStatusbarMessage(IDSNS_MERGING_SELECTED_SHAPES);
            if (mergeSelection(&sel,true))
            {
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                App::addStatusbarMessage(IDSNS_DONE);
            }
            else
                App::addStatusbarMessage(IDSNS_ABORTED);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==SCENE_OBJECT_OPERATION_MERGE_PATHS_SOOCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
            App::addStatusbarMessage(IDSNS_MERGING_SELECTED_PATHS);
            mergePathSelection(&sel);
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            App::addStatusbarMessage(IDSNS_DONE);
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
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
            App::addStatusbarMessage(IDSNS_DIVIDING_SELECTED_SHAPES);
            divideSelection(&sel,true);
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            App::addStatusbarMessage(IDSNS_DONE);
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
                App::addStatusbarMessage(IDSNS_EXECUTING_UNDO);
                App::ct->undoBufferContainer->undo();
                App::addStatusbarMessage(IDSNS_DONE);
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
                App::addStatusbarMessage(IDSNS_EXECUTING_REDO);
                App::ct->undoBufferContainer->redo();
                App::addStatusbarMessage(IDSNS_DONE);
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
        C3DObject* it=App::ct->objCont->getObjectFromHandle(selection[i]);
        if ( (it!=nullptr)&&it->getModelBase() )
        {
            std::vector<C3DObject*> newObjs;
            it->getAllObjectsRecursive(&newObjs,false,true);
            for (int j=0;j<int(newObjs.size());j++)
            {
                if (!App::ct->objCont->isObjectInSelection(newObjs[j]->getObjectHandle(),&selection))
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
    App::ct->copyBuffer->copyCurrentSelection(&sel,App::ct->environment->getSceneLocked());
    App::ct->objCont->deselectObjects(); // We clear selection

    if (displayMessages)
        App::uiThread->showOrHideProgressBar(false);
}

void CSceneObjectOperations::pasteCopyBuffer(bool displayMessages)
{
    FUNCTION_DEBUG;
    if (displayMessages)
        App::uiThread->showOrHideProgressBar(true,-1.0f,"Pasting objects...");

    bool failed=(App::ct->copyBuffer->pasteBuffer(App::ct->environment->getSceneLocked())==-1);

    if (displayMessages)
        App::uiThread->showOrHideProgressBar(false);

    if (failed) // Error: trying to copy locked buffer into unlocked scene!
    {
#ifdef SIM_WITH_GUI
        if (displayMessages)
            App::uiThread->messageBox_warning(App::mainWindow,strTranslate("Paste"),strTranslate(IDS_SCENE_IS_LOCKED_CANNOT_PASTE_WARNING),VMESSAGEBOX_OKELI);
#endif
    }
    else
        App::ct->objCont->removeFromSelectionAllExceptModelBase(true);
}

void CSceneObjectOperations::cutObjects(std::vector<int>* selection,bool displayMessages)
{
    FUNCTION_DEBUG;
    if (displayMessages)
        App::uiThread->showOrHideProgressBar(true,-1.0f,"Cutting objects...");

    addRootObjectChildrenToSelection(*selection);
    copyObjects(selection,false);
    deleteObjects(selection,false);
    App::ct->objCont->deselectObjects(); // We clear selection

    if (displayMessages)
        App::uiThread->showOrHideProgressBar(false);
}

void CSceneObjectOperations::deleteObjects(std::vector<int>* selection,bool displayMessages)
{ // There are a few other spots where objects get deleted (e.g. the C-interface)
    FUNCTION_DEBUG;
    if (displayMessages)
        App::uiThread->showOrHideProgressBar(true,-1.0f,"Deleting objects...");

    addRootObjectChildrenToSelection(selection[0]);
    App::ct->objCont->eraseSeveralObjects(selection[0],true);
    App::ct->objCont->deselectObjects();

    if (displayMessages)
        App::uiThread->showOrHideProgressBar(false);
}

int CSceneObjectOperations::groupSelection(std::vector<int>* selection,bool showMessages)
{ // CALL ONLY FROM THE MAIN SIMULATION THREAD!
    if (App::ct->objCont->getShapeNumberInSelection(selection)!=int(selection->size()))
        return(-1);
    if (selection->size()<2)
        return(-1);

    // Check if some shapes are pure primitives, convex, and check if we have a heightfield:
    int pureCount=0;
    int convexCount=0;
    bool includesHeightfields=false;
    for (int i=0;i<int(selection->size());i++)
    {
        CShape* it=App::ct->objCont->getShape(selection->at(i));
        if (it!=nullptr)
        {
            if (it->geomData->geomInfo->isPure())
            {
                pureCount++;
                if ( (it->geomData->geomInfo->isGeometric())&&(((CGeometric*)it->geomData->geomInfo)->getPurePrimitiveType()==sim_pure_primitive_heightfield) )
                    includesHeightfields=true;
            }
            if (it->geomData->geomInfo->isConvex())
                convexCount++;
        }
    }

    bool onlyPureShapes=false;
    if ( (pureCount!=0)&&(pureCount!=int(selection->size())) )
    { // we are mixing pure and non-pure shapes. Ask what to do
#ifdef SIM_WITH_GUI
        if (showMessages)
        {
            if (VMESSAGEBOX_REPLY_YES!=App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_GROUPING),strTranslate(IDS_GROUPING_PURE_AND_NON_PURE_SHAPES_PROCEED_INFO_MESSAGE),VMESSAGEBOX_YES_NO))
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
                App::uiThread->messageBox_critical(App::mainWindow,strTranslate(IDS_GROUPING_MERGING_MENU_ITEM),strTranslate(IDS_GROUPING_HEIGHTFIELDS_ERROR_MESSAGE),VMESSAGEBOX_OKELI);
#endif
            return(-1); // we abort, heightfields cannot be grouped
        }
    }

    if (showMessages)
        App::uiThread->showOrHideProgressBar(true,-1.0f,"Grouping shapes...");

    if (!onlyPureShapes)
    { // we have only non-pure shapes, or a mix of pure and non-pure. Make sure we have all non-pure now, and that all shapes have no attached drawing objects:
        for (int i=0;i<int(selection->size());i++)
        {
            CShape* it=App::ct->objCont->getShape(selection->at(i));
            if (it!=nullptr)
            {
                it->geomData->geomInfo->setPurePrimitiveType(sim_pure_primitive_none,1.0f,1.0f,1.0f); // this will be propagated to all geometrics!
                // we have to remove all attached drawing objects (we cannot correct for that or it would be very difficult!!)
                App::ct->drawingCont->announceObjectWillBeErased(it->getObjectHandle());
                App::ct->pointCloudCont->announceObjectWillBeErased(it->getObjectHandle());
                App::ct->bannerCont->announceObjectWillBeErased(it->getObjectHandle());
            }
        }
    }

    bool allConvex=(convexCount==int(selection->size()));

    App::ct->objCont->deselectObjects();

    CShape* lastSel=(CShape*)App::ct->objCont->getObjectFromHandle(selection->at(selection->size()-1));

    // Let's first compute the composed mass and center of mass:
    C3Vector newCenterOfMass; // absolute
    newCenterOfMass.clear();
    float cumulMass=0.0f;
    for (size_t i=0;i<selection->size();i++)
    {
        CShape* it=App::ct->objCont->getShape(selection->at(i));
        if (it!=nullptr)
        {
            newCenterOfMass+=it->getCumulativeTransformation()*it->geomData->geomInfo->getLocalInertiaFrame().X*it->geomData->geomInfo->getMass();
            cumulMass+=it->geomData->geomInfo->getMass();
        }
    }
    newCenterOfMass/=cumulMass;
    C3X3Matrix composedInertia; // relative to world
    C7Vector newInertiaFrame(C4Vector::identityRotation,newCenterOfMass);
    composedInertia.clear();

    // Now the recipient and the first item:
    CGeomWrap* theWrap=new CGeomWrap();
    theWrap->setConvex(allConvex);
    theWrap->childList.push_back(lastSel->geomData->geomInfo);
    lastSel->geomData->geomInfo->setTransformationsSinceGrouping(C7Vector::identityTransformation); // so that we can properly (i.e. like it was before) reorient the shape after ungrouping
    lastSel->geomData->geomInfo->setName(lastSel->getObjectName());
    C7Vector tmp(newInertiaFrame.getInverse()*lastSel->getCumulativeTransformation()*lastSel->geomData->geomInfo->getLocalInertiaFrame());
    composedInertia+=CGeomWrap::getNewTensor(lastSel->geomData->geomInfo->getPrincipalMomentsOfInertia(),tmp)*lastSel->geomData->geomInfo->getMass();

    // now the other items:
    std::vector<int> objectsToErase;
    for (int i=0;i<int(selection->size()-1);i++)
    {
        CShape* it=App::ct->objCont->getShape(selection->at(i));
        if (it!=nullptr)
        {
            it->geomData->geomInfo->setTransformationsSinceGrouping(C7Vector::identityTransformation); // so that we can properly (i.e. like it was before) reorient the shape after ungrouping
            it->geomData->geomInfo->setName(it->getObjectName());

            tmp=newInertiaFrame.getInverse()*it->getCumulativeTransformation()*it->geomData->geomInfo->getLocalInertiaFrame();
            composedInertia+=CGeomWrap::getNewTensor(it->geomData->geomInfo->getPrincipalMomentsOfInertia(),tmp)*it->geomData->geomInfo->getMass();

            C7Vector correctionTr=lastSel->getCumulativeTransformation().getInverse()*it->getCumulativeTransformation();
            it->geomData->geomInfo->preMultiplyAllVerticeLocalFrames(correctionTr);
            theWrap->childList.push_back(it->geomData->geomInfo);
            it->geomData->geomInfo=nullptr;
            objectsToErase.push_back(it->getObjectHandle()); // erase it later (if we do it now, texture dependencies might get mixed up)
        }
    }
    lastSel->geomData->geomInfo=nullptr;
    delete lastSel->geomData;
    CGeomProxy* proxy=new CGeomProxy(lastSel->getCumulativeTransformation(),theWrap);
    theWrap->setMass(cumulMass);
    C7Vector oldTrLocal(lastSel->getLocalTransformation());
    C7Vector newTrLocal(lastSel->getParentCumulativeTransformation().getInverse()*proxy->getCreationTransformation());
    lastSel->setLocalTransformation(newTrLocal);
    proxy->setCreationTransformation(C7Vector::identityTransformation);
    lastSel->geomData=proxy;

    // Set the composed inertia:
    composedInertia/=cumulMass; // remember, we only normally work with massless inertias!
    C7Vector newAbsOfComposedInertia;
    newAbsOfComposedInertia.X=newCenterOfMass;
    C3Vector principalMoments;
    CGeomWrap::findPrincipalMomentOfInertia(composedInertia,newAbsOfComposedInertia.Q,principalMoments);
    lastSel->geomData->geomInfo->setLocalInertiaFrame(lastSel->getCumulativeTransformation().getInverse()*newAbsOfComposedInertia);
    lastSel->geomData->geomInfo->setPrincipalMomentsOfInertia(principalMoments);

    // correct the pos/orient. of all children of the 'lastSel' shape:
    for (size_t i=0;i<lastSel->childList.size();i++)
        lastSel->childList[i]->setLocalTransformation(newTrLocal.getInverse()*oldTrLocal*lastSel->childList[i]->getLocalTransformationPart1());

    lastSel->actualizeContainsTransparentComponent();

    App::ct->textureCont->updateAllDependencies();

    App::ct->objCont->eraseSeveralObjects(objectsToErase,true);

    App::ct->objCont->selectObject(lastSel->getObjectHandle());

    if (showMessages)
        App::uiThread->showOrHideProgressBar(false);

    return(lastSel->getObjectHandle());
}

void CSceneObjectOperations::ungroupSelection(std::vector<int>* selection,bool showMessages)
{ // CALL ONLY FROM THE MAIN SIMULATION THREAD!

    if (showMessages)
        App::uiThread->showOrHideProgressBar(true,-1.0f,"Ungrouping shapes...");
    std::vector<int> newObjectHandles;
    App::ct->objCont->deselectObjects();
    std::vector<int> finalSel;
    for (int i=0;i<int(selection->size());i++)
    {
        CShape* it=App::ct->objCont->getShape(selection->at(i));
        if (it!=nullptr)
        {
            if (it->isCompound())
            {
                // Following 2 lines not needed, but added because a previous bug might have done something wrong! So here we make sure that all elements of the multishape are non-pure!!!
                if (!it->geomData->geomInfo->isPure())
                    it->geomData->geomInfo->setPurePrimitiveType(sim_pure_primitive_none,1.0f,1.0f,1.0f);

                // we have to remove all attached drawing objects (we cannot correct for that or it would be very difficult!!)
                App::ct->drawingCont->announceObjectWillBeErased(it->getObjectHandle());
                App::ct->pointCloudCont->announceObjectWillBeErased(it->getObjectHandle());
                App::ct->bannerCont->announceObjectWillBeErased(it->getObjectHandle());
            
                CGeomWrap* oldGeomInfo=it->geomData->geomInfo;
                it->geomData->geomInfo=nullptr;
                C7Vector itCumulTransf(it->getCumulativeTransformation());
                for (int i=int(oldGeomInfo->childList.size())-1;i>=0;i--)
                {
                    if (i==0)
                    { // the first element in the list keeps its original shape
                        C7Vector itOldLocal=it->getLocalTransformation();
                        CGeomProxy* newGeomProxy=new CGeomProxy(itCumulTransf,oldGeomInfo->childList[i]);
                        delete it->geomData;
                        C7Vector itNewLocal(it->getParentCumulativeTransformation().getInverse()*newGeomProxy->getCreationTransformation());
                        it->setLocalTransformation(itNewLocal);
                        newGeomProxy->setCreationTransformation(C7Vector::identityTransformation);
                        it->geomData=newGeomProxy;
                        finalSel.push_back(it->getObjectHandle());
                        it->actualizeContainsTransparentComponent();
                        // Now correct for all attached chil objects:
                        for (int j=0;j<int(it->childList.size());j++)
                        {
                            C3DObject* aChild=it->childList[j];
                            C7Vector oldChild=aChild->getLocalTransformationPart1();
                            aChild->setLocalTransformation(itNewLocal.getInverse()*itOldLocal*oldChild);
                        }

                        // Correctly reorient the shape to what we had before grouping (important for inertia frames that are relative to the shape's frame):
                        C7Vector tr(oldGeomInfo->childList[i]->getTransformationsSinceGrouping());
                        C7Vector currentLocal=it->getLocalTransformation();
                        C7Vector tempLocal=it->getParentCumulativeTransformation().getInverse()*tr.getInverse();
                        it->setLocalTransformation(tempLocal);
                        it->alignBoundingBoxWithWorld();
                        it->setLocalTransformation(currentLocal*tempLocal.getInverse()*it->getLocalTransformation());
                    }
                    else
                    { // the other elements in the list will receive a new shape
                        CGeomProxy* newGeomProxy=new CGeomProxy(itCumulTransf,oldGeomInfo->childList[i]);
                        CShape* newIt=new CShape();
                        newIt->setLocalTransformation(newGeomProxy->getCreationTransformation());
                        newGeomProxy->setCreationTransformation(C7Vector::identityTransformation);
                        newIt->geomData=newGeomProxy;
                        newIt->setObjectName_objectNotYetInScene(oldGeomInfo->childList[i]->getName());
                        newIt->setObjectAltName_objectNotYetInScene(tt::getObjectAltNameFromObjectName(newIt->getObjectName()));
                        newIt->setDynMaterial(it->getDynMaterial()->copyYourself());
                        App::ct->objCont->addObjectToScene(newIt,false,false);
                        newObjectHandles.push_back(newIt->getObjectHandle());
                        // Now a few properties/things we want to be same for the new shape:
                        App::ct->objCont->makeObjectChildOf(newIt,it->getParentObject());
                        newIt->setSizeFactor(it->getSizeFactor());
                        newIt->setLocalObjectProperty(it->getLocalObjectProperty());
                        newIt->setLocalObjectSpecialProperty(it->getLocalObjectSpecialProperty());
                        newIt->layer=it->layer;
                        newIt->setShapeIsDynamicallyStatic(it->getShapeIsDynamicallyStatic());
                        newIt->setRespondable(it->getRespondable());
                        newIt->setDynamicCollisionMask(it->getDynamicCollisionMask());
                        finalSel.push_back(newIt->getObjectHandle());
                        newIt->actualizeContainsTransparentComponent();

                        // Correctly reorient the shape to what we had before grouping (important for inertia frames that are relative to the shape's frame):
                        C7Vector tr(oldGeomInfo->childList[i]->getTransformationsSinceGrouping());
                        C7Vector currentLocal=newIt->getLocalTransformation();
                        C7Vector tempLocal=newIt->getParentCumulativeTransformation().getInverse()*tr.getInverse();

                        newIt->setLocalTransformation(tempLocal);
                        newIt->alignBoundingBoxWithWorld();
                        newIt->setLocalTransformation(currentLocal*tempLocal.getInverse()*newIt->getLocalTransformation());
                    }
                }
                oldGeomInfo->childList.clear();
                delete oldGeomInfo;
            }
        }
    }

    App::ct->textureCont->updateAllDependencies();

    selection->clear();
    for (size_t i=0;i<finalSel.size();i++)
    {
        App::ct->objCont->addObjectToSelection(finalSel[i]);
        selection->push_back(finalSel[i]);
    }

    if (showMessages)
        App::uiThread->showOrHideProgressBar(false);

    if (newObjectHandles.size()>0)
    {
        CInterfaceStack stack;
        stack.pushTableOntoStack();
        stack.pushStringOntoStack("objectHandles",0);
        stack.pushIntArrayTableOntoStack(&newObjectHandles[0],(int)newObjectHandles.size());
        stack.insertDataIntoStackTable();
        App::ct->luaScriptContainer->callChildMainCustomizationAddonSandboxScriptWithData(sim_syscb_aftercreate,&stack);
    }

}

bool CSceneObjectOperations::mergeSelection(std::vector<int>* selection,bool showMessages)
{ // CALL ONLY FROM THE MAIN SIMULATION THREAD!
    if (selection->size()<2)
        return(false);

    // Check if some shapes are pure primitives:
    if (showMessages)
    {
        for (int i=0;i<int(selection->size());i++)
        {
            CShape* it=App::ct->objCont->getShape(selection->at(i));
            if (it!=nullptr)
            {
                if (it->geomData->geomInfo->isPure())
                {
#ifdef SIM_WITH_GUI
                    if (VMESSAGEBOX_REPLY_YES==App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_MERGING),strTranslate(IDS_MERGING_SOME_PURE_SHAPES_PROCEED_INFO_MESSAGE),VMESSAGEBOX_YES_NO))
                        break;
                    return(false); // we abort
#else
                    break;
#endif
                }
            }
        }
    }

    // Check if some shapes contain textures:
    bool textureWarningOutput=false;
    for (int i=0;i<int(selection->size());i++)
    {
        CShape* it=App::ct->objCont->getShape(selection->at(i));
        if (it!=nullptr)
        {
            if (it->geomData->geomInfo->getTextureCount()!=0)
            {
#ifdef SIM_WITH_GUI
                if (showMessages)
                {
                    if ( (!textureWarningOutput)&&(VMESSAGEBOX_REPLY_NO==App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_MERGING),strTranslate(IDS_MERGING_OR_DIVIDING_REMOVES_TEXTURES_PROCEED_INFO_MESSAGE),VMESSAGEBOX_YES_NO)) )
                        return(false); // we abort
                }
#endif
                textureWarningOutput=true;
                // Now remove the textures:
                App::ct->textureCont->announceGeneralObjectWillBeErased(it->getObjectHandle(),-1);
                it->geomData->geomInfo->removeAllTextures();
            }
        }
    }

    if (showMessages)
        App::uiThread->showOrHideProgressBar(true,-1,"Merging shapes...");

    // We have to decompose completely all groups first
    // First isolate the simple shape that should hold all other shapes:
    CShape* lastSel=App::ct->objCont->getShape(selection->at(selection->size()-1));
    selection->pop_back();
    std::vector<int> augmentedSelection(*selection);
    while (lastSel->isCompound())
    {
        std::vector<int> sel;
        sel.push_back(lastSel->getObjectHandle());
        ungroupSelection(&sel,false);
        for (int j=0;j<int(sel.size()-1);j++)
            augmentedSelection.push_back(sel[j]);
        lastSel=App::ct->objCont->getShape(sel[sel.size()-1]);
    }
    lastSel->geomData->geomInfo->setPurePrimitiveType(sim_pure_primitive_none,1.0f,1.0f,1.0f);

    // Now decompose all other shapes:
    std::vector<CShape*> simpleShapes;
    for (int i=0;i<int(augmentedSelection.size());i++)
    {
        CShape* it=App::ct->objCont->getShape(augmentedSelection[i]);
        if (it->isCompound())
        { // We have to decompose this group:
            std::vector<int> sel;
            sel.push_back(it->getObjectHandle());
            ungroupSelection(&sel,false);
            for (int j=0;j<int(sel.size());j++)
                augmentedSelection.push_back(sel[j]);
        }
        else
        {
            it->geomData->geomInfo->setPurePrimitiveType(sim_pure_primitive_none,1.0f,1.0f,1.0f);
            simpleShapes.push_back(it);
        }
    }
    App::ct->objCont->deselectObjects();

    if (simpleShapes.size()>0)
    {
        // we have to remove all attached drawing objects (we cannot correct for that or it would be very difficult!!)
        App::ct->drawingCont->announceObjectWillBeErased(lastSel->getObjectHandle());
        App::ct->pointCloudCont->announceObjectWillBeErased(lastSel->getObjectHandle());
        App::ct->bannerCont->announceObjectWillBeErased(lastSel->getObjectHandle());

        std::vector<float> wvert;
        std::vector<int> wind;
        lastSel->geomData->geomInfo->getCumulativeMeshes(wvert,&wind,nullptr);
        float cumulMass=lastSel->geomData->geomInfo->getMass();
        C7Vector tr(lastSel->getCumulativeTransformation());
        for (int i=0;i<int(wvert.size())/3;i++)
        {
            C3Vector v(&wvert[3*i+0]);
            v*=tr;
            wvert[3*i+0]=v(0);
            wvert[3*i+1]=v(1);
            wvert[3*i+2]=v(2);
        }
        int voff=(int)wvert.size();
        for (size_t j=0;j<simpleShapes.size();j++)
        {
            CShape* aShape=simpleShapes[j];
            aShape->geomData->geomInfo->getCumulativeMeshes(wvert,&wind,nullptr);
            cumulMass+=aShape->geomData->geomInfo->getMass();
            tr=aShape->getCumulativeTransformation();
            for (size_t i=voff/3;i<wvert.size()/3;i++)
            {
                C3Vector v(&wvert[3*i+0]);
                v*=tr;
                wvert[3*i+0]=v(0);
                wvert[3*i+1]=v(1);
                wvert[3*i+2]=v(2);
            }
            App::ct->objCont->eraseObject(aShape,true);
            voff=(int)wvert.size();
        }
        // We now have in wvert and wind all the vertices and indices (absolute vertices)

        CGeomProxy* proxy=new CGeomProxy(nullptr,wvert,wind,nullptr,nullptr);
        C7Vector lastSelPreviousLocal(lastSel->getLocalTransformation());
        C7Vector lastSelCurrentLocal(lastSel->getParentCumulativeTransformation().getInverse()*proxy->getCreationTransformation());
        lastSel->setLocalTransformation(lastSelCurrentLocal);
        proxy->setCreationTransformation(C7Vector::identityTransformation);
        // Copy the CGeometric properties (not all):

        ((CGeometric*)lastSel->geomData->geomInfo)->copyVisualAttributesTo(((CGeometric*)proxy->geomInfo));
        ((CGeometric*)proxy->geomInfo)->actualizeGouraudShadingAndVisibleEdges(); // since 21/3/2014

        // Copy the CGeomWrap properties (not all):
        proxy->geomInfo->setConvex(false); // not really needed here, already set to false normally
        proxy->geomInfo->setMass(cumulMass); // we do not copy here!

        // Do not copy following:
        //      proxy->geomInfo->setPrincipalMomentsOfInertia(lastSel->geomData->geomInfo->getPrincipalMomentsOfInertia());
        proxy->geomInfo->setLocalInertiaFrame(C7Vector::identityTransformation); // to have the inertia frame centered in the geometric middle of the mesh!

        // Delete the old structure and connect the new one:
        delete lastSel->geomData;
        lastSel->geomData=proxy;

        // Adjust the transformation of all its children:
        for (int i=0;i<int(lastSel->childList.size());i++)
            lastSel->childList[i]->setLocalTransformation(lastSelCurrentLocal.getInverse()*lastSelPreviousLocal*lastSel->childList[i]->getLocalTransformation());

        // Finally select the merged object:
        App::ct->objCont->selectObject(lastSel->getObjectHandle());
    }

    if (showMessages)
        App::uiThread->showOrHideProgressBar(false);

    return(true);
}

void CSceneObjectOperations::divideSelection(std::vector<int>* selection,bool showMessages)
{ // CALL ONLY FROM THE MAIN SIMULATION THREAD!
    if (selection->size()<1)
        return;

    std::vector<int> newObjectHandles;
    // Check if some shapes contain textures:
    bool textureWarningOutput=false;
    for (int i=0;i<int(selection->size());i++)
    {
        CShape* it=App::ct->objCont->getShape(selection->at(i));
        if (it!=nullptr)
        {
            if (it->geomData->geomInfo->getTextureCount()!=0)
            {
#ifdef SIM_WITH_GUI
                if ( showMessages&&(!textureWarningOutput)&&(VMESSAGEBOX_REPLY_NO==App::uiThread->messageBox_warning(App::mainWindow,strTranslate("Dividing"),strTranslate(IDS_MERGING_OR_DIVIDING_REMOVES_TEXTURES_PROCEED_INFO_MESSAGE),VMESSAGEBOX_YES_NO)) )
                    return; // we abort
#endif
                textureWarningOutput=true;
                // Now remove the textures:
                App::ct->textureCont->announceGeneralObjectWillBeErased(it->getObjectHandle(),-1);
                it->geomData->geomInfo->removeAllTextures();
            }
        }
    }

    if (showMessages)
        App::uiThread->showOrHideProgressBar(true,-1,"Dividing shapes...");

    // We have to decompose completely all groups first
    std::vector<int> augmentedSelection(*selection);
    std::vector<CShape*> simpleShapes;
    for (int i=0;i<int(augmentedSelection.size());i++)
    {
        CShape* it=App::ct->objCont->getShape(augmentedSelection[i]);
        if (it->isCompound())
        { // We have to decompose this group:
            std::vector<int> sel;
            sel.push_back(it->getObjectHandle());
            ungroupSelection(&sel,false);
            for (int j=0;j<int(sel.size());j++)
                augmentedSelection.push_back(sel[j]);
        }
        else
        {
            if (!it->geomData->geomInfo->isPure())
                simpleShapes.push_back(it); // pure simple shapes can anyway not be divided!
        }
    }
    App::ct->objCont->deselectObjects();
    selection->clear();

    for (int i=0;i<int(simpleShapes.size());i++)
    {
        CShape* it=simpleShapes[i];

        std::vector<float> wvert;
        std::vector<int> wind;
        it->geomData->geomInfo->getCumulativeMeshes(wvert,&wind,nullptr);
        int extractedCount=0;
        while (true)
        {
            std::vector<float> subvert;
            std::vector<int> subind;
            if (CMeshManip::extractOneShape(&wvert,&wind,&subvert,&subind))
            { // There are more parts to extract
                extractedCount++;
                C7Vector tmpTr(it->getCumulativeTransformation());
                CGeomProxy* newGeomProxy=new CGeomProxy(&tmpTr,subvert,subind,nullptr,nullptr);
                CShape* newIt=new CShape();
                newIt->setLocalTransformation(newGeomProxy->getCreationTransformation());
                newGeomProxy->setCreationTransformation(C7Vector::identityTransformation);
                newIt->geomData=newGeomProxy;

                std::string name(it->getObjectName()+"_sub");
                tt::removeIllegalCharacters(name,false);
                while (App::ct->objCont->getObjectFromName(name.c_str())!=nullptr)
                    name=tt::generateNewName_noDash(name);
                newIt->setObjectName_objectNotYetInScene(name);
                newIt->setObjectAltName_objectNotYetInScene(tt::getObjectAltNameFromObjectName(name));
                App::ct->objCont->addObjectToScene(newIt,false,false);
                newObjectHandles.push_back(newIt->getObjectHandle());
                // Now a few properties/things we want to be same for the new shape:
                App::ct->objCont->makeObjectChildOf(newIt,it->getParentObject());
                newIt->setSizeFactor(it->getSizeFactor());
                newIt->setLocalObjectProperty(it->getLocalObjectProperty());
                newIt->setLocalObjectSpecialProperty(it->getLocalObjectSpecialProperty());
                newIt->layer=it->layer;
                // Do not copy following:
                //              newIt->setShapeIsDynamicallyStatic(it->getShapeIsDynamicallyStatic());
                //              newIt->setRespondable(it->getRespondable());
                //              newIt->setDynamicCollisionMask(it->getDynamicCollisionMask());

                // Copy the CGeometric properties (not all):
                ((CGeometric*)it->geomData->geomInfo)->copyVisualAttributesTo(((CGeometric*)newIt->geomData->geomInfo));
                ((CGeometric*)newIt->geomData->geomInfo)->actualizeGouraudShadingAndVisibleEdges(); // since 21/3/2014

                newIt->setDynMaterial(it->getDynMaterial()->copyYourself());

                // Do not copy following:
                //      newIt->geomData->geomInfo->setPrincipalMomentsOfInertia(it->geomData->geomInfo->getPrincipalMomentsOfInertia());
                newIt->geomData->geomInfo->setLocalInertiaFrame(C7Vector::identityTransformation); // to have the inertia frame centered in the geometric middle of the mesh!

                newIt->actualizeContainsTransparentComponent();
                selection->push_back(newIt->getObjectHandle());
            }
            else
            { // This was the last part
                if (extractedCount==0)
                    break; // we couldn't extract anything!
                C7Vector tmpTr(it->getCumulativeTransformation());
                CGeomProxy* newGeomProxy=new CGeomProxy(&tmpTr,subvert,subind,nullptr,nullptr);
                C7Vector itLocalOld(it->getLocalTransformation());
//  Was         C7Vector itLocalNew(newGeomProxy->getCreationTransformation()); and corrected on 18/4/2013 to:
                C7Vector itLocalNew(it->getParentCumulativeTransformation().getInverse()*newGeomProxy->getCreationTransformation());
                it->setLocalTransformation(itLocalNew);
                newGeomProxy->setCreationTransformation(C7Vector::identityTransformation);

                // Copy the CGeometric properties (not all):
                ((CGeometric*)it->geomData->geomInfo)->copyVisualAttributesTo(((CGeometric*)newGeomProxy->geomInfo));
                ((CGeometric*)newGeomProxy->geomInfo)->actualizeGouraudShadingAndVisibleEdges(); // since 21/3/2014


                // Do not copy following:
                //      newIt->geomData->geomInfo->setPrincipalMomentsOfInertia(it->geomData->geomInfo->getPrincipalMomentsOfInertia());
                newGeomProxy->geomInfo->setLocalInertiaFrame(C7Vector::identityTransformation); // to have the inertia frame centered in the geometric middle of the mesh!
                //      newIt->geomData->geomInfo->setLocalInertiaFrame(bla);

                delete it->geomData;
                it->geomData=newGeomProxy;
                it->actualizeContainsTransparentComponent();
                
                // Now we have to adjust all the children:
                for (int j=0;j<int(it->childList.size());j++)
                    it->childList[j]->setLocalTransformation(itLocalNew.getInverse()*itLocalOld*it->childList[j]->getLocalTransformationPart1());

                selection->push_back(it->getObjectHandle());
                break;
            }
        }
    }

    // We select extracted shapes:
    for (size_t i=0;i<selection->size();i++)
        App::ct->objCont->addObjectToSelection(selection->at(i));

    if (newObjectHandles.size()>0)
    {
        CInterfaceStack stack;
        stack.pushTableOntoStack();
        stack.pushStringOntoStack("objectHandles",0);
        stack.pushIntArrayTableOntoStack(&newObjectHandles[0],(int)newObjectHandles.size());
        stack.insertDataIntoStackTable();
        App::ct->luaScriptContainer->callChildMainCustomizationAddonSandboxScriptWithData(sim_syscb_aftercreate,&stack);
    }

    if (showMessages)
        App::uiThread->showOrHideProgressBar(false);
}

void CSceneObjectOperations::mergePathSelection(std::vector<int>* selection)
{ // CALL ONLY FROM THE MAIN SIMULATION THREAD!
    App::ct->objCont->deselectObjects();
    if (selection->size()<2)
        return;
    C3DObject* lastObj=App::ct->objCont->getLastSelection(selection);
    if (lastObj->getObjectType()==sim_object_path_type)
    {
        CPath* last=(CPath*)lastObj;
        C7Vector lastTrInv(last->getCumulativeTransformation().getInverse());
        last->pathContainer->enableActualization(false);
        for (size_t i=0;i<selection->size()-1;i++)
        {
            C3DObject* obj=App::ct->objCont->getObjectFromHandle((*selection)[i]);
            if (obj->getObjectType()==sim_object_path_type)
            {
                CPath* it=(CPath*)obj;
                C7Vector itTr(it->getCumulativeTransformation());
                for (int j=0;j<it->pathContainer->getSimplePathPointCount();j++)
                {
                    CSimplePathPoint* itCopy(it->pathContainer->getSimplePathPoint(j)->copyYourself());
                    C7Vector confRel(itCopy->getTransformation());
                    last->pathContainer->addSimplePathPoint(itCopy);
                    itCopy->setTransformation(lastTrInv*itTr*confRel,it->pathContainer->getAttributes());
                }
                // Now remove that path:
                App::ct->objCont->eraseObject(obj,true);
            }       
        }
        last->pathContainer->enableActualization(true);
        last->pathContainer->actualizePath();
    }
}


void CSceneObjectOperations::scaleObjects(const std::vector<int>& selection,float scalingFactor,bool scalePositionsToo)
{
    std::vector<int> sel(selection);
    CSceneObjectOperations::addRootObjectChildrenToSelection(sel);
    for (int i=0;i<int(sel.size());i++)
    {
        C3DObject* it=App::ct->objCont->getObjectFromHandle(sel[i]);
        if (scalePositionsToo)
            it->scalePosition(scalingFactor);
        else
        { // If one parent is a root object (model base) and in this selection, then we also scale the position here!! (2009/06/10)
            C3DObject* itp=it->getParentObject();
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
                itp=itp->getParentObject();
            }
        }
        it->scaleObject(scalingFactor);
    }

    // Now we might have to scale a few ikElements/ikGroups:
    for (int i=0;i<int(App::ct->ikGroups->ikGroups.size());i++)
    {
        CikGroup* ikGroup=App::ct->ikGroups->ikGroups[i];
        bool scaleIkGroup=true;
        // Go through all ikElement lists:
        for (int j=0;j<int(ikGroup->ikElements.size());j++)
        {
            CikEl* ikEl=ikGroup->ikElements[j];
            CDummy* tip=App::ct->objCont->getDummy(ikEl->getTooltip());
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
            else
                scaleIkGroup=false; // not all ik elements are scaled --> we do not scale the ik goup!
        }
        if (scaleIkGroup)
        { // we scale that ikGroup!
            ikGroup->setJointTreshholdLinear(ikGroup->getJointTreshholdLinear()*scalingFactor);
            ikGroup->setAvoidanceThreshold(ikGroup->getAvoidanceThreshold()*scalingFactor);
        }
    }

    App::setFullDialogRefreshFlag();
}

int CSceneObjectOperations::generateConvexHull(int shapeHandle)
{
    FUNCTION_DEBUG;
    std::vector<float> allHullVertices;
    allHullVertices.reserve(40000*3);

    CShape* it=App::ct->objCont->getShape(shapeHandle);
    if (it!=nullptr)
    {
        C7Vector transf(it->getCumulativeTransformation());
        std::vector<float> vert;
        std::vector<float> vertD;
        std::vector<int> ind;
        it->geomData->geomInfo->getCumulativeMeshes(vertD,&ind,nullptr);
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
            CGeomProxy* geom=new CGeomProxy(nullptr,hull,indices,nullptr,nullptr);
            CShape* it=new CShape();
            it->setLocalTransformation(geom->getCreationTransformation());
            geom->setCreationTransformation(C7Vector::identityTransformation);
            it->geomData=geom;
            ((CGeometric*)geom->geomInfo)->setConvexVisualAttributes();
            geom->geomInfo->setLocalInertiaFrame(C7Vector::identityTransformation);
            App::ct->objCont->addObjectToScene(it,false,true);
            return(it->getObjectHandle());
        }
    }
    return(-1);
}

int CSceneObjectOperations::generateConvexDecomposed(int shapeHandle,size_t nClusters,double maxConcavity,
                                             bool addExtraDistPoints,bool addFacesPoints,double maxConnectDist,
                                             size_t maxTrianglesInDecimatedMesh,size_t maxHullVertices,
                                             double smallClusterThreshold,bool individuallyConsiderMultishapeComponents,
                                             int maxIterations,bool useHACD,int resolution_VHACD,int depth_VHACD,float concavity_VHACD,
                                             int planeDownsampling_VHACD,int convexHullDownsampling_VHACD,
                                             float alpha_VHACD,float beta_VHACD,float gamma_VHACD,bool pca_VHACD,
                                             bool voxelBased_VHACD,int maxVerticesPerCH_VHACD,float minVolumePerCH_VHACD)
{
    FUNCTION_DEBUG;
    std::vector<float> vert;
    std::vector<int> ind;
    CShape* it=App::ct->objCont->getShape(shapeHandle);
    if (it!=nullptr)
    {
        C7Vector tr(it->getCumulativeTransformation());
        std::vector<int> generatedShapeHandles;
        if (individuallyConsiderMultishapeComponents&&(!it->geomData->geomInfo->isGeometric()))
        {
            std::vector<CGeometric*> shapeComponents;
            it->geomData->geomInfo->getAllShapeComponentsCumulative(shapeComponents);
            for (int comp=0;comp<int(shapeComponents.size());comp++)
            {
                CGeometric* geom=shapeComponents[comp];
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
                { // the convex decomposition routine sometimes fails producing good convectivity (i.e. there are slightly non-convex items that V-REP doesn't want to recognize as convex)
                    // For those situations, we try several times to convex decompose:
                    outputVert.clear();
                    outputInd.clear();
                    std::vector<int> _tempHandles;
                    CMeshRoutines::convexDecompose(&vert[0],(int)vert.size(),&ind[0],(int)ind.size(),outputVert,outputInd,
                            nClusters,maxConcavity,addExtraDistPoints,addFacesPoints,maxConnectDist,
                            maxTrianglesInDecimatedMesh,maxHullVertices,smallClusterThreshold,
                            useHACD,resolution_VHACD,depth_VHACD,concavity_VHACD,planeDownsampling_VHACD,
                            convexHullDownsampling_VHACD,alpha_VHACD,beta_VHACD,gamma_VHACD,pca_VHACD,
                            voxelBased_VHACD,maxVerticesPerCH_VHACD,minVolumePerCH_VHACD);
                    int convexRecognizedCount=0;
                    for (size_t i=0;i<outputVert.size();i++)
                    {
                        int handle=simCreateMeshShape_internal(2,20.0f*piValue_f/180.0f,&outputVert[i]->at(0),(int)outputVert[i]->size(),&outputInd[i]->at(0),(int)outputInd[i]->size(),nullptr);
                        CShape* shape=App::ct->objCont->getShape(handle);
                        if (shape!=nullptr)
                        {
                            // Following flag is automatically set upon shape creation. Also, it seems that the convex decomposition algo sometimes failes..
                            if (((CGeometric*)shape->geomData->geomInfo)->isConvex())
                                convexRecognizedCount++; // V-REP convex test is more strict than what the convex decomp. algo does
                            _tempHandles.push_back(handle);
                            ((CGeometric*)shape->geomData->geomInfo)->setConvexVisualAttributes();
                            // Set some visual parameters:
                            ((CGeometric*)shape->geomData->geomInfo)->color.colors[0]=0.7f;
                            ((CGeometric*)shape->geomData->geomInfo)->color.colors[1]=1.0f;
                            ((CGeometric*)shape->geomData->geomInfo)->color.colors[2]=0.7f;
                            ((CGeometric*)shape->geomData->geomInfo)->setEdgeThresholdAngle(0.0f);
                            ((CGeometric*)shape->geomData->geomInfo)->setGouraudShadingAngle(0.0f);
                            ((CGeometric*)shape->geomData->geomInfo)->setVisibleEdges(true);
                        }
                        delete outputVert[i];
                        delete outputInd[i];
                    }
                    // we check if all shapes are recognized as convex shapes by V-REP
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
                            CShape* as=App::ct->objCont->getShape(_tempHandles[i]);
                            if (as!=nullptr)
                            {
                                C7Vector tr2(as->getCumulativeTransformation());
                                CGeometric* geom=(CGeometric*)as->geomData->geomInfo;
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
            it->geomData->geomInfo->getCumulativeMeshes(vert,&ind,nullptr);
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
            { // the convex decomposition routine sometimes fails producing good convectivity (i.e. there are slightly non-convex items that V-REP doesn't want to recognize as convex)
                // For those situations, we try several times to convex decompose:
                outputVert.clear();
                outputInd.clear();
                std::vector<int> _tempHandles;
                CMeshRoutines::convexDecompose(&vert[0],(int)vert.size(),&ind[0],(int)ind.size(),outputVert,outputInd,
                        nClusters,maxConcavity,addExtraDistPoints,addFacesPoints,maxConnectDist,
                        maxTrianglesInDecimatedMesh,maxHullVertices,smallClusterThreshold,
                        useHACD,resolution_VHACD,depth_VHACD,concavity_VHACD,planeDownsampling_VHACD,
                        convexHullDownsampling_VHACD,alpha_VHACD,beta_VHACD,gamma_VHACD,pca_VHACD,
                        voxelBased_VHACD,maxVerticesPerCH_VHACD,minVolumePerCH_VHACD);
                int convexRecognizedCount=0;
                for (int i=0;i<int(outputVert.size());i++)
                {
                    int handle=simCreateMeshShape_internal(2,20.0f*piValue_f/180.0f,&outputVert[i]->at(0),(int)outputVert[i]->size(),&outputInd[i]->at(0),(int)outputInd[i]->size(),nullptr);
                    CShape* shape=App::ct->objCont->getShape(handle);
                    if (shape!=nullptr)
                    {
                        // Following flag is automatically set upon shape creation. Also, it seems that the convex decomposition algo sometimes failes..
                        if (((CGeometric*)shape->geomData->geomInfo)->isConvex())
                            convexRecognizedCount++; // V-REP convex test is more strict than what the convex decomp. algo does
                        _tempHandles.push_back(handle);

                        ((CGeometric*)shape->geomData->geomInfo)->setConvexVisualAttributes();
                        // Set some visual parameters:
                        ((CGeometric*)shape->geomData->geomInfo)->color.colors[0]=0.7f;
                        ((CGeometric*)shape->geomData->geomInfo)->color.colors[1]=1.0f;
                        ((CGeometric*)shape->geomData->geomInfo)->color.colors[2]=0.7f;
                        ((CGeometric*)shape->geomData->geomInfo)->setEdgeThresholdAngle(0.0f);
                        ((CGeometric*)shape->geomData->geomInfo)->setGouraudShadingAngle(0.0f);
                        ((CGeometric*)shape->geomData->geomInfo)->setVisibleEdges(true);
                    }
                    delete outputVert[i];
                    delete outputInd[i];
                }
                // we check if all shapes are recognized as convex shapes by V-REP
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
                        CShape* as=App::ct->objCont->getShape(_tempHandles[i]);
                        if (as!=nullptr)
                        {
                            C7Vector tr2(as->getCumulativeTransformation());
                            CGeometric* geom=(CGeometric*)as->geomData->geomInfo;
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
    FUNCTION_DEBUG;
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
    static int depth=20;
    static float concavity=0.0025f;
    static int planeDownsampling=4;
    static int convexHullDownsampling=4;
    static float alpha=0.05f;
    static float beta=0.05f;
    static float gamma=0.00125f;
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
        if (options&128)
        { // we have more parameters than usual (i.e. the V-HACD parameters):
            useHACD=false;
            resolution=intParams[5];
            depth=intParams[6];
            concavity=floatParams[5];
            planeDownsampling=intParams[7];
            convexHullDownsampling=intParams[8];
            alpha=floatParams[6];
            beta=floatParams[7];
            gamma=floatParams[8];
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
        cmdIn.intParams.push_back(depth);
        cmdIn.floatParams.push_back(concavity);
        cmdIn.intParams.push_back(planeDownsampling);
        cmdIn.intParams.push_back(convexHullDownsampling);
        cmdIn.floatParams.push_back(alpha);
        cmdIn.floatParams.push_back(beta);
        cmdIn.floatParams.push_back(gamma);
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
            depth=cmdOut.intParams[5];
            concavity=cmdOut.floatParams[3];
            planeDownsampling=cmdOut.intParams[6];
            convexHullDownsampling=cmdOut.intParams[7];
            alpha=cmdOut.floatParams[4];
            beta=cmdOut.floatParams[5];
            gamma=cmdOut.floatParams[6];
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
                                                        maxIterations,useHACD,resolution,depth,concavity,planeDownsampling,
                                                        convexHullDownsampling,alpha,beta,gamma,pca,voxelBasedMode,
                                                        maxVerticesPerCH,minVolumePerCH);
    else
        retVal=-1;

    if (retVal!=-1)
    { // transfer the inertia and mass:
        // Get the mass and inertia info from the old shape:
        CShape* oldShape=App::ct->objCont->getShape(shapeHandle);
        C7Vector absCOM(oldShape->getCumulativeTransformation());
        absCOM=absCOM*oldShape->geomData->geomInfo->getLocalInertiaFrame();
        float mass=oldShape->geomData->geomInfo->getMass();
        C7Vector absCOMNoShift(absCOM);
        absCOMNoShift.X.clear(); // we just wanna get the orientation of the inertia matrix, no shift info!
        C3X3Matrix tensor(CGeomWrap::getNewTensor(oldShape->geomData->geomInfo->getPrincipalMomentsOfInertia(),absCOMNoShift));

        // Transfer the mass and inertia info to the new shape:
        CShape* newShape=App::ct->objCont->getShape(retVal);
        newShape->geomData->geomInfo->setMass(mass);
        C4Vector rot;
        C3Vector pmoi;
        CGeomWrap::findPrincipalMomentOfInertia(tensor,rot,pmoi);
        newShape->geomData->geomInfo->setPrincipalMomentsOfInertia(pmoi);
        absCOM.Q=rot;
        C7Vector relCOM(newShape->getCumulativeTransformation().getInverse()*absCOM);
        newShape->geomData->geomInfo->setLocalInertiaFrame(relCOM);
    }

    if ( (retVal!=-1)&&((options&1)!=0) )
    { // we wanted a morph!!
        CShape* newShape=App::ct->objCont->getShape(retVal);
        CShape* it=App::ct->objCont->getShape(shapeHandle);
        C7Vector newLocal(it->getParentCumulativeTransformation().getInverse()*newShape->getCumulativeTransformation());
        C7Vector oldLocal(it->getLocalTransformation());
        delete it->geomData;
        it->geomData=newShape->geomData; // we exchange the geomData object
        it->setLocalTransformation(newLocal); // The shape's frame was changed!
        it->incrementMemorizedConfigurationValidCounter(); // so if we are running in a simulation, the shape doesn't get reset at its initial config
        newShape->geomData=nullptr;
        App::ct->objCont->eraseObject(newShape,true);
        // We need to correct all its children for this change of frame:
        for (size_t i=0;i<it->childList.size();i++)
        {
            it->childList[i]->setLocalTransformation(newLocal.getInverse()*oldLocal*it->childList[i]->getLocalTransformationPart1());
            it->childList[i]->incrementMemorizedConfigurationValidCounter(); // so if we are running in a simulation, the shape doesn't get reset at its initial config
        }
        retVal=shapeHandle;
    }
    return(retVal);
}

#ifdef SIM_WITH_GUI
void CSceneObjectOperations::addMenu(VMenu* menu)
{
    handleVerSpec_sceneObjectOperations_addMenu1(menu);
}
#endif
